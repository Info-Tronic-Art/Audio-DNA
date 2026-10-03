# LANE bf9b -- builder report (s-rta-1002b)

STATUS: PENDING (S4 in progress; S0 DONE at STAGE_P_HEAD 3dac692; S1 DONE at 475b716; S2a DONE at 2db77eb; S2b DONE at bb5d84e; S2c DONE at 9cce864; S3 DONE at e1cd314 -- B7 critic panel is Harmony's)
Stage in progress: S3 DONE (badge / tab dot / badge click 06e26bd, grid f39ca40, TopBar fade 5cdf218, load notice + undo hint e1cd314; ctest 1156 / 0; TSAN 5 / 5; MS7 bites; B7 live captures taken). S2c DONE (Undo skips deck switches, 9cce864; ctest 1144 / 0; TSAN 5 / 5; B4g; MS1 / MS4 on showDeck). S2b DONE (tests on the shared stack; the sanctioned window closed; ctest 1146 / 0; TSAN 5 / 5; MS1-MS6). S2a DONE, S1 DONE, S0 DONE (Stage P; G0-G7 PASS at STAGE_P_HEAD)
BF9B_BASE: 11820fa (main head at lane start, 2026-10-02 17:06 EDT)
STAGE_P_BASE: 11820fa (parent of C0 c79ea39)
STAGE_P_HEAD: 3dac692 (C4)

## S0 progress log (appended per item)

## S0 commits (sha -- item -- app/test build rc)

## Gates G0-G7 (at STAGE_P_HEAD)

## Resume point (for a successor builder)
- 17:06 STEP 0: worktree clean, branch lane/bf9b at 11820fa (== main head). No src/tests/probe/doc drift fa9604d..11820fa
  (`git diff --stat` empty) -- every ruling-bf9 line cite holds. CLAUDE.md 24,002 B. Disk 292 GiB free.
- 17:07 configured build-lane (Release, TEST_SERVER=ON, SYPHON=ON, FETCHCONTENT_FULLY_DISCONNECTED, deps from main's
  build/_deps read-only); full build started -j3.
- C0 c79ea39: ADNA_INSPECT_LAYER lever in MainComponent ctor tail, inside `#if AUDIODNA_TEST_SERVER`.
- C1 prepared (tests): amendment 5 round trip (test_composition.cpp), 6 (test_undo_commands.cpp:188), 8 (lint pins
  CompositorEngine.cpp {1,1} + site comment), 9 (identifier lint case), 10 (test_deck_clock (b) replaced + (a) title),
  11 (test_layer_inspector_layer_row.cpp, 3 cases; CMake target renamed; :3086 comment), test_compositor.cpp:423-437
  deleted, tool_uitoggle_snapshot one LayerInspector shot, amendment-7 test rewords (test_layer_state_key.cpp,
  tests/CMakeLists.txt:122-126).
- C3 / C4 prepared in the tree (probes, docs); C2 prepared as a script (scratchpad c2_edit.py), applied only after the
  C0 app is built and copied (the BASE arm must be base behaviour + lever).
- C1 bfbe9ea committed. Build (C0 src + C1 tests) rc 0 at 17:38:47. BASE arm = that app, copied by `ditto` (no
  re-sign) to scratchpad/bf9b-S0/apps/base-c0.app at 17:39 (strings: ADNA_INSPECT_LAYER 1, "Keep this layer
  rendering" 1).
- G2 RED record at C1 (`ctest --test-dir build-lane -j3 --output-on-failure`), VERBATIM:
  `99% tests passed, 7 tests failed out of 1117` -- Total Tests 1117 (= BASE 1114 + 3, see G2):
    216 - Layer: a file's persistent key is ignored and never written back; every other field round-trips (bf9 Stage P)
          [test_composition.cpp:651 CHECK_FALSE(hasProperty("persistent")) !true; :652 string compare]
    604 - (b) a layer loaded with "persistent": true is an ordinary layer: ... (bf9 Stage P)
          [Transparent: progress 0.0f == 0.25, ticked 0 == 2; Mask: progress 0.0f == 0.25]
    788 - LayerInspector Layer section: Ignore Column Trigger is there and no Persistent toggle is left (bf9 Stage P)
    790 - LayerInspector Ignore Column Trigger takes the full row under Master (bf9 Stage P) [x 150 == 4, w 146 == 292]
    1091 - render thread: one trigger-tuple load per layer per pass (pinned counts) [CompositorEngine.cpp runtime() 2,
          getActiveClip( 2]
    1092 - no Persistent-feature identifier left in src/ [33 hits, = ruling R-F2]
    848 - AppSettings: a corrupt file reads as empty ... -- NOT a Stage P test: `REQUIRE(dir.createDirectory())` at
          test_app_settings.cpp:19 under parallel ctest (temp dir name from getNonexistentChildFile, not atomic across
          concurrently running processes; another lane's ctest was running). Re-run serially 5x:
          `100% tests passed, 0 tests failed out of 7` x5. Pre-existing; filed as found_not_fixed.
  layer_row (2) "for every layer type ..." PASSES at C1 (as ruled). Exactly the ruled RED set fails.

## S0 RESUME (successor builder, started 18:03:14)
- Predecessor stopped ~17:45 (usage limit). State found: branch lane/bf9b = bfbe9ea (C1) on c79ea39 (C0) on 11820fa;
  28 files uncommitted (C2 src removal + C3 probes + C4 docs); build-c2.log ended "Terminated: 15" with no compile
  error before it (C2 build never completed). Base-arm app scratchpad/bf9b-S0/apps/base-c0.app present.
- STAGE_P_BASE = 11820fa (parent of C0 c79ea39).
- C2/C3/C4 verified against ruling-bf9 before committing (git diff read in full): P4 ApiServer (field + comment),
  P3 LayerInspector (toggle gone, Ignore Column full row), P2 CompositorEngine/Renderer/DeckClock deletions +
  amendment-7 rewords (MainComponent.cpp swap + finishStagedLoad comments, CompositorEngine.h clipHasContent,
  LayerStateKey.h, LayerClock.h), P1 Layer.h/.cpp; probes per amendment 12(a)-(e); docs per plan section 6 +
  amendment 13. One addition by the successor: p_flag_ignored prints /api/state frame_time_ms /
  peak_frame_time_ms (G6 was ruled but not yet in the probe).
- 18:08 incremental build of the C2 tree rc 0 (build-c2b.log; 0 compile errors); app strings "Keep this layer
  rendering" 0, ADNA_INSPECT_LAYER 1. ctest at C2 (`ctest --test-dir build-lane -j3 --output-on-failure`):
  `100% tests passed, 0 tests failed out of 1117`.
- Commits: C2 bb4b767 (src, 13 files), C3 bb1fc83 (probes; probe-quit-ours.sh git add -f), C4 3dac692 (docs).
  STAGE_P_HEAD = 3dac692. Static TEST_CASE count 11820fa -> bfbe9ea over the changed test files: 129 -> 132 (+3),
  consistent with G2's 1114 -> 1117.
- 18:10 clean rebuild at STAGE_P_HEAD started (G1 warnings + rc); then ctest, TSan, live gates.

### G3 TEXT at STAGE_P_HEAD 3dac692 (18:11; script scratchpad/bf9b-S0/g3.sh, output verbatim)
```
HEAD 3dac692  worktree clean of src changes: 0 dirty
== G3a: identifier grep over src (comments included), expect 0 lines
G3a lines: 0
== G3b
model/MainComponent/LayerInspector.h ignoreColumn changed lines (expect 0): 0
LayerInspector.cpp ignoreColumn changed lines (expect 2): 2
-    ignoreColumnToggle_.setBounds(area.getX() + area.getWidth() / 2, y, area.getWidth() / 2, kRowHeight);
+    ignoreColumnToggle_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
comparator line (expect 1): 1
== G3c
not-allowed lines at HEAD (expect 0): 0
allowed count HEAD: 16   @STAGE_P_BASE 11820fa: 16
== G3d
2326-    };
2327-#endif
2328-    setSize(1280, 800);
2329-#if AUDIODNA_TEST_SERVER
2330:    // TEST-ONLY (test-server builds): visual gates of the Layer tab -- ADNA_INSPECT_LAYER=<n> selects layer n of the
2331-    // shown deck and opens the Layer tab once startup is done. Inert unless the variable is set.
2332:    if (const char* e = std::getenv("ADNA_INSPECT_LAYER"))
2333-    {
2334-        const int layerIdx = std::atoi(e);
2335-        juce::MessageManager::callAsync([safe = juce::Component::SafePointer<MainComponent>(this), layerIdx] {
2336-            if (safe == nullptr || safe->deckView_ == nullptr || safe->inspectorPanel_ == nullptr)
2337-                return;
2338-            if (safe->deckView_->onLayerSelected)
other src files naming it: 0
== G3e
CLAUDE.md HEAD 23943 B; STAGE_P_BASE 24002 B (expect base - 59, <= 25000)
== G3f
docs/claude/performance-controls.md:42:**Persistent layers: removed** (2026-10-02, Boris: "I want to remove the persistent"). A layer saved with `persistent: tr
G3f remaining lines without 'removed': 0
```
G3 (a)-(f): PASS.

### G0(a) at STAGE_P_HEAD
```
grep -n 'tell application "Audio-DNA" to quit\|adna_kill' .harmony/probe-render-state.sh .harmony/probe-deck-clock.sh -> 0 hits
probe-quit-ours.sh:37:      osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
```
The only osascript quit is in quit_ours(), on its only-ours branch (`if [ -z "$others" ]`); adna_kill is gone. G0(a) PASS.

### Live batch 1 (18:11-18:12, lock bf9b-S0; clean build of HEAD running at -j3 meanwhile -- the ps top lines show it): G0(b) + G5 Run 1 BASE (scratchpad/bf9b-S0/live/batch1.log, VERBATIM)
```
18:11:45 lock acquired
18:11:45 ps top:
 99.4       00:04 /Library/Developer/CommandLineTools/usr/bin/clang
 98.3       00:01 (clang)
 33.4       00:00 /Library/Developer/CommandLineTools/usr/bin/clang
 32.5 01-06:50:05 /Applications/Firefox.app/Contents/MacOS/plugin-container.app/Contents/MacOS/plugin-container
 12.7    04:23:04 claude
===== G0(b) 18:11:45
decoy pid 48676 ucomm 'Audio-DNA       '
OURPID set to 48688 (ucomm 'sleep           ')
FOREIGN Audio-DNA pid 48676 running -- untouched
quit_ours rc=1
kill -0 decoy: alive (untouched)
G0(b) PASS
/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/g0b.sh: line 20: 48688 Terminated: 15          sleep 300
after cleanup: Audio-DNA pids ''
===== G5 Run 1 BASE 18:11:47
app: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/apps/base-c0.app
out: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/run1-base/rstate.O2yu7p
ours: pid 48799
--- a4_feedback
      a4_feedback: d(cap1, absent)=15.01 noise d(cap1, cap2)=0.00
PASS  a4_feedback: (i) the fixture exercises the stage on the active deck (d(cap1, absent)=15.01 >= 5)
      a4_feedback: (ii) not applied (RSTATE_REF unset -- this is the REF / BASE arm)
--- a4_fxonly
      a4_fxonly: d(cap1, absent)=228.66 noise d(cap1, cap2)=0.00
PASS  a4_fxonly: (i) the fixture exercises the stage on the active deck (d(cap1, absent)=228.66 >= 5)
      a4_fxonly: (ii) not applied (RSTATE_REF unset -- this is the REF / BASE arm)
--- a4_fxonly_medialess
      a4_fxonly_medialess: d(cap1, absent)=228.66 noise d(cap1, cap2)=0.00
PASS  a4_fxonly_medialess: (i) the fixture exercises the stage on the active deck (d(cap1, absent)=228.66 >= 5)
      a4_fxonly_medialess: (ii) not applied (RSTATE_REF unset -- this is the REF / BASE arm)
--- p_flag_ignored
      p_flag_ignored: G6 INFO frame_time_ms=1.292744755744934 peak_frame_time_ms=7.400083065032959
      p_flag_ignored: d(control, A-only)=0.00 d(subject, control)=29.43 d(subject, A-only)=29.43 non-blank=True
FAIL  p_flag_ignored: a file's "persistent": true is ignored -- the other deck's layer never draws (d(subject, control)=29.43, tol 1.5)
--- p_flag_ignored_empty
      p_flag_ignored_empty: control mean RGB 0.00, subject mean RGB 19.46, d(subject, control)=19.46
FAIL  p_flag_ignored_empty: an empty shown deck stays black -- another deck's "persistent" layer never draws (d(subject, control)=19.46, tol 1.5)
--- p_api_no_field
      p_api_no_field: 2 layer objects; with "persistent": [0, 5]; missing id / visible / activeClipColumn: []
FAIL  p_api_no_field: GET /api/composition layers carry no "persistent" and keep id / visible / activeClipColumn (2 layers)
--- p_ignore_column
PASS  p_ignore_column: a column trigger skips the Ignore Column Trigger layer (layer 0 activeClipColumn 1 == 1, layer 1 0 == 0)

PY 4 PASS / 3 FAIL
PASS  no foreign render_frame traffic during the run
PASS  app terminated

PROBE-RENDER-STATE RED
run1 rc=1 18:12:42
Audio-DNA after: ''
18:12:42 lock released
```
G0(b) PASS (quit_ours exits 1, prints FOREIGN, decoy alive). Run 1 BASE as pre-registered: p_flag_ignored FAIL (29.43), p_flag_ignored_empty FAIL (19.46), p_api_no_field FAIL (key on layers 0, 5) = the RED evidence; p_ignore_column PASS; a4_* bar (i) PASS. Looked at pfi_subject.png (BASE): deck 1's 2x2 Screen-Split image B covers deck 0's A -- the persistent layer drawn; pfi_control.png = A alone. REF dir = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/run1-base/rstate.O2yu7p

### Live batch 3b (18:13): G5 Run 4 BASE (live/batch3b.log, VERBATIM)
```
18:13:10 re-acquire cooldown 17 s
18:13:27 lock acquired
18:13:27 ps top:
100.0       00:03 /Library/Developer/CommandLineTools/usr/bin/clang
 97.3       00:01 /Library/Developer/CommandLineTools/usr/bin/clang
 63.2       00:00 /Library/Developer/CommandLineTools/usr/bin/clang
  7.7 06-05:30:17 /System/Library/PrivateFrameworks/SkyLight.framework/Resources/WindowServer
  4.7 06-05:30:12 /Applications/Ghostty.app/Contents/MacOS/ghostty
===== G5 Run 4 BASE 18:13:27
app: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/apps/base-c0.app
out: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/run4-base/dclock.zL42h1
ours: pid 59993
--- d_single_advance
PASS  d_single_advance: a layer's fade advances once per frame while its deck is away (progress 0.50 at T/2 = 2.0 s, expected in [0.35, 0.65])

PY 1 PASS / 0 FAIL
PASS  no foreign render_frame traffic during the run
PASS  app terminated

PROBE-DECK-CLOCK GREEN
run4 rc=0 18:13:37
Audio-DNA after: ''
18:13:37 lock released
```
Run 4 BASE: d_single_advance PASS (guard).

### G1 BUILD at STAGE_P_HEAD 3dac692
- Clean rebuild `cmake --build build-lane --clean-first -j3` (the lane's -j cap; Harmony's B1 string uses -j$(ncpu)): 18:10:11 -> 18:44:09, rc=0, 0 ` error:` lines (build-head-clean.log). Machine load avg reached 67 from the other lanes' builds.
- Per-commit rc: C0 c79ea39 + C1 bfbe9ea: app + tests rc 0 at 17:38:47 (predecessor, build-c1.log); C2 bb4b767: incremental rc 0 at 18:08:20 (build-c2b.log); C3 bb1fc83 / C4 3dac692 touch no build input (probes / docs) -> the clean HEAD build rc 0 is theirs.
- Warnings located in Stage P-touched src files (scratchpad/bf9b-S0/warncount.sh: unique file:line:col of 'warning:' lines whose path is one of the 13 touched src files): STAGE_P_BASE-side log = the predecessor's first full build of build-lane at C0 (build-base.log, the lever adds no warning: MainComponent.cpp:2330-2342 not in the list) = 11; HEAD clean rebuild = 11 (identical lists: ApiServer.cpp 2, MainComponent.cpp 8, CompositorEngine.cpp 1 'totalCells'). HEAD <= BASE: PASS. Whole-log totals 2174 vs 2197 differ only in JUCE headers / juce_gui_basics.cpp (third-party; interleaved -j output + the renamed test target's module set).

### G2 UNIT at STAGE_P_HEAD (`ctest --test-dir build-lane -j3 --output-on-failure`, 18:44:24-18:44:58), VERBATIM
```
100% tests passed, 0 tests failed out of 1117
Total Test time (real) =  33.85 sec
```
count(HEAD) 1117 == count(STAGE_P_BASE) 1114 + 3. Required cases present and Passed (ctest-head.log): #216 P1 round trip; #1091 pinned counts (CompositorEngine.cpp {1,1}); #1092 identifier lint; #604 deck_clock (b); #788 / #789 / #790 layer_row (1) (2) (3); #395 TriggerColumnCmd composite; #404 Deck::triggerColumn forced snap. RED record at C1: see the predecessor's lines above (exactly the ruled set + the pre-existing #848 parallel-ctest flake). G2 PASS.

### G7 supplementary: tool_uitoggle_snapshot (build-lane/tests, headless, 18:45, rc 0, 12 PNGs)
scratchpad/bf9b-S0/live/uitoggle/layerinspector-layer-row-headless.png -- LOOKED: the Layer section shows Master (1.00, slider) and then ONE full-width 'Ignore Column Trigger' row (tick box at the Master row's left edge, unticked), no Persistent toggle, no gap; Video starts right below.
- 18:45 G4 probe-tsan-unit.sh started (fresh build-tsan in the worktree).

### G7 VISUAL WORK GATE captures (batch 4, 18:45-18:46; live/batch4.log VERBATIM)
```
18:45:37 lock acquired
===== G7 BEFORE 18:45:37
18:45:40 app up: pid 85568
win 36715 'Audio-DNA' 1728 x 1079 layer 0
captured window 36715 -> /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/g7/before.png rc 0
app running after quit: no
before rc=0
audio-dna windows 0, Output-named 0
===== G7 AFTER 18:45:51
18:45:55 app up: pid 88209
win 36722 'Audio-DNA' 1728 x 1079 layer 0
captured window 36722 -> /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/g7/after.png rc 0
app running after quit: no
after rc=0
audio-dna windows 0, Output-named 0
Audio-DNA after: ''
18:46:01 lock released
```
Captures (window-only, largest on-screen window of OUR pid, 3456x2158): BEFORE = scratchpad/bf9b-S0/live/g7/before.png (BASE app base-c0.app), AFTER = live/g7/after.png (STAGE_P_HEAD app apps/stagep-head.app, sha256 of its binary == build-lane's). Both launched `open -g --env ADNA_INSPECT_LAYER=0 <app> --args --test-mode` (the lever selected Layer 1 and opened the Layer tab in both).
Builder's observations (decoded and LOOKED; the critic verdicts are Harmony's): BEFORE: Layer section = Master row, then 'Persistent' (left half) + 'Ignore Column Trigger' (right half), both unticked. AFTER: Master row, then ONE 'Ignore Column Trigger' row whose tick box sits at the Master row's left edge, unticked; the 'Video' header is at the same y in both captures (nothing below moved); no other on-screen difference (FPS readout 101 vs 102). UserNotificationCenter windows 18:46:16 (15 s after the last quit): 0. Output-named windows: 0.

### G4 TSAN at STAGE_P_HEAD (`.harmony/probe-tsan-unit.sh`, fresh worktree build-tsan, 18:45:26-18:47:39, exit 0)
Full log scratchpad/bf9b-S0/tsan-head.log (186 lines; the rest is compiler output of the 2 targets). Every non-compiler line, VERBATIM:
```
probe-tsan-unit: configure /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/build-tsan (ADNA_SANITIZE=thread, RelWithDebInfo) 2026-10-02 18:45:26
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 18:46:00
probe-tsan-unit: ctest -L tsan finds 4 [tsan] cases (expected 4)
probe-tsan-unit: ctest -L tsan 2026-10-02 18:47:38
Test project /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/build-tsan
    Start 111: R1 message-thread triggers vs render clock / autopilot on one deck
1/4 Test #111: R1 message-thread triggers vs render clock / autopilot on one deck ......   Passed    0.32 sec
    Start 112: R2 clip runtime fields: trigger writes vs render transport write-back
2/4 Test #112: R2 clip runtime fields: trigger writes vs render transport write-back ...   Passed    0.28 sec
    Start 113: R4 tuple consistency and no lost fade under a paced trigger storm
3/4 Test #113: R4 tuple consistency and no lost fade under a paced trigger storm .......   Passed    0.25 sec
    Start 114: R3 manual scalar writes vs eff() reads
4/4 Test #114: R3 manual scalar writes vs eff() reads ..................................   Passed    0.26 sec
100% tests passed, 0 tests failed out of 4
Label Time Summary:
tsan    =   1.11 sec*proc (4 tests)
Total Test time (real) =   1.12 sec
probe-tsan-unit: ctest rc=0 2026-10-02 18:47:39
```
"WARNING: ThreadSanitizer" count in the log: 0. 4 / 4 PASS (EXPECTED_TSAN_CASES 4). G4 PASS.

### Live batch 2 (18:47-18:54, acquire_quiet_lock): G5 Run 2 BRANCH, RSTATE_REF = Run 1's dir (live/batch2.log, VERBATIM)
```
18:48:00 compiler running -- waiting
18:50:01 compiler running -- waiting
18:50:41 lock acquired
18:50:41 ps top:
 27.4 06-06:07:31 /usr/libexec/mobileassetd
 15.8 06-06:07:31 /System/Library/PrivateFrameworks/SkyLight.framework/Resources/WindowServer
 11.0 06-06:06:36 /usr/libexec/sysmond
  8.2 06-06:07:26 /Applications/Ghostty.app/Contents/MacOS/ghostty
  4.9 06-06:07:32 /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/FSEvents.framework/Versions/A/Support/fseventsd
===== G5 Run 2 BRANCH 18:50:41 REF=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/run1-base/rstate.O2yu7p
app: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/apps/stagep-head.app
out: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/run2-branch/rstate.Bq6UEE
ours: pid 35067
--- r5_hold
      r5_hold: frame 0 mean RGBA = (0.0, 0.0, 0.0, 0.0)
PASS  r5_hold: Freeze 1.0 on a fresh layer holds its cleared history (transparent black), never the pass target's glClear colour - frame 0 mean|RGBA|=0.00 (tol 1.5)
      r5_hold: frame 1 mean RGBA = (0.0, 0.0, 0.0, 0.0)
PASS  r5_hold: Freeze 1.0 on a fresh layer holds its cleared history (transparent black), never the pass target's glClear colour - frame 1 mean|RGBA|=0.00 (tol 1.5)
      r5_hold: frame 2 mean RGBA = (0.0, 0.0, 0.0, 0.0)
PASS  r5_hold: Freeze 1.0 on a fresh layer holds its cleared history (transparent black), never the pass target's glClear colour - frame 2 mean|RGBA|=0.00 (tol 1.5)
--- r5_burst
      r5_burst id=81: 47 frames after the switch
      r5_burst id=82: 47 frames after the switch
      r5_burst id=83: 48 frames after the switch
      r5_burst id=84: 47 frames after the switch
      r5_burst id=85: 47 frames after the switch
PASS  r5_burst: every attempt captured >= 8 frames after the switch (short: [])
PASS  r5_burst: no blank frame when a temporal effect first runs on a layer (5 fresh layers): []
--- r1_temporal
      r1_temporal: t=1.49s p~0.19 IN x<126 ratio 0.00 (d(A,B) 4.35)  OUT x>587 ratio 0.00 (d(A,B) 29.09)  [not counted]
      r1_temporal: t=2.00s p~0.25 IN x<249 ratio 0.00 (d(A,B) 8.79)  OUT x>710 ratio 0.00 (d(A,B) 26.88)
      r1_temporal: t=2.51s p~0.31 IN x<371 ratio 0.00 (d(A,B) 19.06)  OUT x>832 ratio 0.00 (d(A,B) 26.70)
      r1_temporal: t=3.03s p~0.38 IN x<495 ratio 0.00 (d(A,B) 24.90)  OUT x>956 ratio 0.00 (d(A,B) 28.54)
      r1_temporal: t=3.53s p~0.44 IN x<617 ratio 0.00 (d(A,B) 30.31)  OUT x>1078 ratio 0.00 (d(A,B) 30.47)
      r1_temporal: t=4.04s p~0.51 IN x<740 ratio 0.00 (d(A,B) 32.74)  OUT x>1201 ratio 0.00 (d(A,B) 30.95)
      r1_temporal: t=4.57s p~0.57 IN x<865 ratio 0.00 (d(A,B) 31.66)  OUT x>1326 ratio 0.00 (d(A,B) 30.30)
      r1_temporal: t=5.08s p~0.64 IN x<989 ratio 0.00 (d(A,B) 29.06)  OUT x>1450 ratio 0.00 (d(A,B) 29.20)
      r1_temporal: t=5.60s p~0.70 IN x<1113 ratio 0.01 (d(A,B) 27.73)  OUT x>1574 ratio 0.00 (d(A,B) 24.24)
      r1_temporal: t=6.12s p~0.76 IN x<1238 ratio 0.01 (d(A,B) 28.11)  OUT x>1699 ratio 0.00 (d(A,B) 12.19)
PASS  r1_temporal: during a crossfade each clip keeps its own history (IN=B OUT=A; 9 counted frames, ratios <= 0.05): []
--- r1_control
      r1_control: t=1.50s p~0.19 IN x<129 ratio 0.00 (d(A,B) 4.59)  OUT x>590 ratio 0.00 (d(A,B) 29.17)  [not counted]
      r1_control: t=2.03s p~0.25 IN x<256 ratio 0.00 (d(A,B) 9.63)  OUT x>716 ratio 0.00 (d(A,B) 26.98)
      r1_control: t=2.55s p~0.32 IN x<382 ratio 0.00 (d(A,B) 20.26)  OUT x>843 ratio 0.00 (d(A,B) 26.90)
      r1_control: t=3.07s p~0.38 IN x<507 ratio 0.00 (d(A,B) 25.52)  OUT x>967 ratio 0.00 (d(A,B) 28.84)
      r1_control: t=3.59s p~0.45 IN x<630 ratio 0.00 (d(A,B) 31.04)  OUT x>1090 ratio 0.00 (d(A,B) 30.73)
      r1_control: t=4.09s p~0.51 IN x<752 ratio 0.00 (d(A,B) 32.90)  OUT x>1213 ratio 0.00 (d(A,B) 31.03)
      r1_control: t=4.61s p~0.58 IN x<876 ratio 0.00 (d(A,B) 31.63)  OUT x>1337 ratio 0.00 (d(A,B) 30.43)
      r1_control: t=5.13s p~0.64 IN x<1000 ratio 0.00 (d(A,B) 29.09)  OUT x>1461 ratio 0.00 (d(A,B) 29.32)
      r1_control: t=5.64s p~0.71 IN x<1123 ratio 0.00 (d(A,B) 27.86)  OUT x>1584 ratio 0.00 (d(A,B) 23.75)
      r1_control: t=6.14s p~0.77 IN x<1244 ratio 0.00 (d(A,B) 28.30)  OUT x>1704 ratio 0.00 (d(A,B) 11.96)
PASS  r1_control: during a crossfade each clip keeps its own history (IN=B OUT=A; 9 counted frames, ratios <= 0.05): []
--- r1_ring
      r1_ring: t=1.50s p~0.19 IN x<130 ratio 0.00 (d(A,B) 4.56)  OUT x>590 ratio 0.00 (d(A,B) 29.41)  [not counted]
      r1_ring: t=2.03s p~0.25 IN x<256 ratio 0.00 (d(A,B) 9.68)  OUT x>717 ratio 0.00 (d(A,B) 27.21)
      r1_ring: t=2.55s p~0.32 IN x<380 ratio 0.00 (d(A,B) 20.22)  OUT x>841 ratio 0.00 (d(A,B) 27.11)
      r1_ring: t=3.07s p~0.38 IN x<506 ratio 0.00 (d(A,B) 25.66)  OUT x>967 ratio 0.00 (d(A,B) 29.08)
      r1_ring: t=3.59s p~0.45 IN x<630 ratio 0.00 (d(A,B) 31.25)  OUT x>1091 ratio 0.00 (d(A,B) 31.00)
      r1_ring: t=4.09s p~0.51 IN x<751 ratio 0.00 (d(A,B) 33.13)  OUT x>1212 ratio 0.00 (d(A,B) 31.29)
      r1_ring: t=4.60s p~0.58 IN x<874 ratio 0.00 (d(A,B) 31.90)  OUT x>1335 ratio 0.00 (d(A,B) 30.67)
      r1_ring: t=5.12s p~0.64 IN x<997 ratio 0.00 (d(A,B) 29.35)  OUT x>1458 ratio 0.00 (d(A,B) 29.56)
      r1_ring: t=5.64s p~0.71 IN x<1123 ratio 0.00 (d(A,B) 28.07)  OUT x>1584 ratio 0.00 (d(A,B) 23.89)
      r1_ring: t=6.16s p~0.77 IN x<1248 ratio 0.00 (d(A,B) 28.54)  OUT x>1709 ratio 0.00 (d(A,B) 11.48)
PASS  r1_ring: during a crossfade each clip keeps its own history (IN=B OUT=A; 9 counted frames, ratios <= 0.05): []
--- r1_retrigger
      r1_retrigger: t=1.50s p~0.19 IN x<129 ratio 0.00 (d(A,B) 4.28)  OUT x>589 ratio 0.00 (d(A,B) 28.86)  [not counted]
      r1_retrigger: t=2.03s p~0.25 IN x<256 ratio 0.00 (d(A,B) 9.36)  OUT x>716 ratio 0.00 (d(A,B) 26.67)
      r1_retrigger: t=2.55s p~0.32 IN x<381 ratio 0.00 (d(A,B) 19.87)  OUT x>842 ratio 0.00 (d(A,B) 26.57)
      r1_retrigger: t=3.06s p~0.38 IN x<504 ratio 0.00 (d(A,B) 25.11)  OUT x>965 ratio 0.00 (d(A,B) 28.49)
      r1_retrigger: t=3.57s p~0.45 IN x<626 ratio 0.00 (d(A,B) 30.56)  OUT x>1087 ratio 0.00 (d(A,B) 30.38)
      r1_retrigger: t=4.09s p~0.51 IN x<751 ratio 0.00 (d(A,B) 32.56)  OUT x>1212 ratio 0.00 (d(A,B) 30.71)
      r1_retrigger: t=4.61s p~0.58 IN x<877 ratio 0.00 (d(A,B) 31.29)  OUT x>1337 ratio 0.00 (d(A,B) 30.10)
      r1_retrigger: t=5.13s p~0.64 IN x<1000 ratio 0.00 (d(A,B) 28.74)  OUT x>1461 ratio 0.00 (d(A,B) 28.96)
      r1_retrigger: t=5.65s p~0.71 IN x<1125 ratio 0.00 (d(A,B) 27.52)  OUT x>1586 ratio 0.00 (d(A,B) 23.25)
      r1_retrigger: t=6.18s p~0.77 IN x<1253 ratio 0.00 (d(A,B) 28.02)  OUT x>1713 ratio 0.00 (d(A,B) 10.69)
PASS  r1_retrigger: during a crossfade each clip keeps its own history (re-triggered: IN=A OUT=B; 9 counted frames, ratios <= 0.05): []
--- r1_counts
      r1_counts: frame_rings 3 -> 5 (+2), temporal_buffers 8 -> 8 (+0); peak_frame_time_ms first use 6.64, first fade 13.42, second fade (no creation) 10.21; load avg 7.19 10.09 17.24
PASS  r1_counts: 5 fades on one layer (Screen Split on every clip) hold exactly 2 rings (+2) and <= 2 temporal buffers (+0)
PASS  r1_counts: longest frame across the layer's first use (its ring created) 6.64 ms <= 16.7 ms
PASS  r1_counts: longest frame across the first fade (spare ring created) 13.42 ms <= 16.7 ms
--- r1_cells
      r1_cells: cells +32 after 0.3 s, +139 after 1.3 s (fps 107.2, bound 179); rings +1
PASS  r1_cells: a fresh ring creates cells on first write (+32 in 0.3 s, 1..479)
PASS  r1_cells: at most one cell per ring per frame (+32 < +139 <= 179)
PASS  r1_cells: never more than 480 cells per ring (+139 <= 480 x 1)
--- a4_feedback
      a4_feedback: d(cap1, absent)=15.01 noise d(cap1, cap2)=0.00
PASS  a4_feedback: (i) the fixture exercises the stage on the active deck (d(cap1, absent)=15.01 >= 5)
      a4_feedback: (ii) d(cap1, REF cap1)=0.00 REF noise=0.00 floor=1.50
PASS  a4_feedback: (ii) renders as on the BASE arm (d(cap1, REF cap1)=0.00 <= floor 1.50)
--- a4_fxonly
      a4_fxonly: d(cap1, absent)=228.66 noise d(cap1, cap2)=0.00
PASS  a4_fxonly: (i) the fixture exercises the stage on the active deck (d(cap1, absent)=228.66 >= 5)
      a4_fxonly: (ii) d(cap1, REF cap1)=0.00 REF noise=0.00 floor=1.50
PASS  a4_fxonly: (ii) renders as on the BASE arm (d(cap1, REF cap1)=0.00 <= floor 1.50)
--- a4_fxonly_medialess
      a4_fxonly_medialess: d(cap1, absent)=228.66 noise d(cap1, cap2)=0.00
PASS  a4_fxonly_medialess: (i) the fixture exercises the stage on the active deck (d(cap1, absent)=228.66 >= 5)
      a4_fxonly_medialess: (ii) d(cap1, REF cap1)=0.00 REF noise=0.00 floor=1.50
PASS  a4_fxonly_medialess: (ii) renders as on the BASE arm (d(cap1, REF cap1)=0.00 <= floor 1.50)
--- p_flag_ignored
      p_flag_ignored: G6 INFO frame_time_ms=1.142840266227722 peak_frame_time_ms=8.68891716003418
      p_flag_ignored: d(control, A-only)=0.00 d(subject, control)=0.00 d(subject, A-only)=0.00 non-blank=True
PASS  p_flag_ignored: a file's "persistent": true is ignored -- the other deck's layer never draws (d(subject, control)=0.00, tol 1.5)
--- p_flag_ignored_empty
      p_flag_ignored_empty: control mean RGB 0.00, subject mean RGB 0.00, d(subject, control)=0.00
PASS  p_flag_ignored_empty: an empty shown deck stays black -- another deck's "persistent" layer never draws (d(subject, control)=0.00, tol 1.5)
--- p_api_no_field
      p_api_no_field: 2 layer objects; with "persistent": []; missing id / visible / activeClipColumn: []
PASS  p_api_no_field: GET /api/composition layers carry no "persistent" and keep id / visible / activeClipColumn (2 layers)
--- p_ignore_column
PASS  p_ignore_column: a column trigger skips the Ignore Column Trigger layer (layer 0 activeClipColumn 1 == 1, layer 1 0 == 0)

PY 25 PASS / 0 FAIL
PASS  no foreign render_frame traffic during the run
PASS  app terminated

PROBE-RENDER-STATE GREEN
run2 rc=0 18:54:16
Audio-DNA after: ''
18:54:16 lock released
```
Run 2 BRANCH: 25 PASS / 0 FAIL, every row PASS. LOOKED at pfi_subject.png (BRANCH): A alone (the three ellipses), identical to the control -- deck 1's "persistent" layer is not drawn.

### Live batch 3 (18:54-19:01, acquire_quiet_lock; waited for bf2-S1a's lock): G5 Run 3 BRANCH deck-clock (live/batch3.log, VERBATIM)
```
18:54:20 re-acquire cooldown 41 s
18:55:01 lock held by: bf2-S1a 43660 1790981660 -- waiting
18:57:01 lock held by: bf2-S1a 43660 1790981660 -- waiting
18:59:01 lock held by: bf2-S1a 43660 1790981660 -- waiting
19:00:42 lock acquired
19:00:42 ps top:
 37.6       00:12 /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10/build-lane/tests/test_projectm_canvas_gl
 19.3 06-06:17:32 /System/Library/PrivateFrameworks/SkyLight.framework/Resources/WindowServer
  9.9 06-06:16:37 /usr/libexec/sysmond
  6.3 06-06:17:27 /Applications/Ghostty.app/Contents/MacOS/ghostty
  3.7    05:12:01 claude
===== G5 Run 3 BRANCH 19:00:42
app: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/apps/stagep-head.app
out: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf9b-S0/live/run3-branch/dclock.30ThoD
ours: pid 87730
--- d_fade_finishes
      d_fade_finishes: d(A,B)=29.43 FLOOR=1.00
      d_fade_finishes: deck 0 L0 (t, crossfadeProgress, previousClipColumn) while away: [(0.82, 0.20637646317482, 0), (1.34, 0.33470231294632, 0), (1.85, 0.462619453668594, 0), (2.37, 0.591590702533722, 0), (2.89, 0.721239805221558, 0), (3.41, 0.849676609039307, 0), (3.93, 0.981102585792542, 0), (4.46, 1.0, -1), (4.98, 1.0, -1), (5.5, 1.0, -1)]
PASS  d_fade_finishes: the fade keeps advancing while its deck is not shown (progress +0.26 between samples 1 and 3)
PASS  d_fade_finishes: the fade is complete (progress 1.0, previousClipColumn -1) by T + 1 s while away (first complete sample (4.46, 1.0, -1))
PASS  d_fade_finishes: back on deck 0 the incoming clip is showing, fade finished (d(f, refB)=0.00, floor 1.00; d(f, refA)=29.43)
--- d_single_advance
PASS  d_single_advance: a layer's fade advances once per frame while its deck is away (progress 0.51 at T/2 = 2.0 s, expected in [0.35, 0.65])
--- d_pending_trigger_still_cancelled
PASS  d_pending_trigger_still_cancelled: a quantized trigger left waiting on the deck is cancelled when you leave it (L5): deck 0 activeClipColumn 0 after 4 crossings
--- d_video_keeps_time
PASS  d_video_keeps_time: a video keeps playing while its deck is away (t0 1.98 s, t1 6.40 s, t1 - t0 = 4.42 s after 4.0 s away, expected in [3.5, 5.5])
PASS  d_video_keeps_time: /api/composition playheadPosition 0.545 matches the frame (0.533 +- 0.06)
--- d_imageseq_keeps_time
      d_imageseq_keeps_time: d(rA, rB)=29.43 FLOOR=1.00
PASS  d_imageseq_keeps_time: an image sequence keeps its clock while its deck is away (back after 2.0 s: d(f, frame B)=0.00, floor 1.00; d(f, frame A)=29.43)
--- d_autopilot_keeps_time
PASS  d_autopilot_keeps_time: autopilot keeps advancing a deck that is away (6 beat crossings, Beat4 / PlayNext: deck 0 activeClipColumn 1, expected >= 1)
--- d_return_hitch
      d_return_hitch: peak_frame_time_ms while away 17.93, across the return 12.24 (fps 103.2)
PASS  d_return_hitch: the return frame's catch-up decode (1080p, GOP 60, 5.0 s away) stays under 50.0 ms (peak 12.24 ms; REPORT -- target <= 16)

PY 10 PASS / 0 FAIL
PASS  no foreign render_frame traffic during the run
PASS  app terminated

PROBE-DECK-CLOCK GREEN
run3 rc=0 19:01:53
Audio-DNA after: ''
19:01:53 lock released
```
Run 3 BRANCH: 10 PASS / 0 FAIL, every row PASS (keep-time unchanged by Stage P). After all batches: UserNotificationCenter windows 0 (19:02:15, 22 s after the last quit); Audio-DNA windows 0 / Output-named 0; no Audio-DNA running.

### G5 / G6 summary (both arms' numbers)
| row | BASE (C0 app) | BRANCH (STAGE_P_HEAD app) |
|---|---|---|
| p_flag_ignored | FAIL d(subject, control) 29.43 (VALID: d(control, A-only) 0.00) | PASS 0.00 (VALID 0.00), non-blank |
| p_flag_ignored_empty | FAIL d 19.46 (VALID: control mean RGB 0.00) | PASS 0.00 (control 0.00) |
| p_api_no_field | FAIL key on layers [0, 5] | PASS no key; id / visible / activeClipColumn present |
| p_ignore_column | PASS (L0 1, L1 0) | PASS (L0 1, L1 0) |
| a4_feedback | (i) PASS 15.01, noise 0.00 | (i) PASS 15.01; (ii) PASS d(cap1, REF) 0.00 <= floor 1.50 |
| a4_fxonly | (i) PASS 228.66, noise 0.00 | (i) PASS 228.66; (ii) PASS 0.00 <= 1.50 |
| a4_fxonly_medialess | (i) PASS 228.66, noise 0.00 | (i) PASS 228.66; (ii) PASS 0.00 <= 1.50 |
| r1_* / r5_* | -- | all PASS (r1_counts peaks 6.64 / 13.42 ms <= 16.7) |
| deck-clock 7 rows | d_single_advance PASS (p 0.50) | all PASS (d_single_advance p 0.51; d_return_hitch peak 12.24 ms) |
| G6 INFO frame_time_ms / peak_frame_time_ms at end of p_flag_ignored | 1.29 / 7.40 (build at -j3 running; ps top = clang) | 1.14 / 8.69 (quiet lock; ps top = mobileassetd 27 %) |

G5 PASS (Run 1 RED as pre-registered, Runs 2-4 PASS). G6 recorded (INFO, no bar).

## S0 RESULT (successor, 19:03)
Commits (sha -- item -- build rc): c79ea39 C0 lever -- rc 0 (17:38:47, with C1); bfbe9ea C1 RED tests -- rc 0 (17:38:47);
bb4b767 C2 src removal (P4 -> P3 -> P2 -> P1 in one commit, as amendment 3 allows) -- rc 0 (18:08:20); bb1fc83 C3 probes --
no build input; 3dac692 C4 docs -- no build input; clean rebuild at 3dac692 rc 0 (18:44:09).
Gates at STAGE_P_HEAD 3dac692: G0 (a)(b) PASS; G1 PASS (rc 0, touched-file warnings 11 <= 11); G2 PASS (1117 = 1114 + 3,
0 failures, required cases passed); G3 (a)-(f) PASS; G4 PASS (4 / 4, 0 TSan warnings); G5 PASS (BASE RED rows as
registered, BRANCH all rows PASS); G6 recorded (INFO); G7 captures + observations recorded -- the five-critic panel is
Harmony's (not run by the builder).
Stage P statement (ruling-bf9 amendment 2): "Stage P delivers only the two clauses both of Boris's answers share: 'I want
to remove the persistent' and 'the only thing remotely persistent should be to ignore column controls'. It delivers neither
the 14:30 reading (a deck change stops the old deck) nor the 14:44 model (decks are boxes of clips). What plays across a
deck switch is bf9b's, gated by K1-K8."
Arms for bf9b's later K rows: STAGE_P = scratchpad/bf9b-S0/apps/stagep-head.app (ditto copy, not re-signed; binary sha256
prefix fcd75aff1387dd66c2c6 == build-lane's at 3dac692). BASE (C0) = scratchpad/bf9b-S0/apps/base-c0.app.

## Deviations / notes
- Added one thing to the predecessor's C3 work: p_flag_ignored prints /api/state frame_time_ms / peak_frame_time_ms (ruling
  G6 asked for it; it was missing). No bar.
- G1's per-commit build for C0 and C1 is one build (the predecessor built both together at 17:38:47); C0's app = that
  build (C1 touches tests only).
- G1's BASE-side warning log is the C0 full build (build-lane's first build), not a separate 11820fa build; the lever adds
  no warning line (none of the 11 lies in MainComponent.cpp:2329-2342).
- Builds used -j3 (lane cap), not Harmony's B1 -j$(ncpu); ctest used -j3, not -j8.
- Rig slip: one text-gate command used a subshell `cd` into the worktree (read-only grep, 18:10); no other effect.
- The .venv symlink was never created: both probe launchers fall back to the main checkout's .venv (read-only use).

## found_not_fixed
- test_app_settings.cpp:19 `REQUIRE(dir.createDirectory())` fails under parallel ctest from several lanes at once (#848
  at C1; temp dir name from getNonexistentChildFile is not atomic across processes). Passed 5x serially; passed in both
  of this successor's full runs. Pre-existing.

## Notes for .harmony/notebook.md (Harmony appends)
- A Stage P / probe G6 frame-time read sits in probe-render-state.py p_flag_ignored (INFO). | discovered: .harmony/probe-render-state.py p_flag_ignored
- .harmony/* is gitignored: a NEW probe helper (probe-quit-ours.sh) needs `git add -f` or it silently stays untracked. | discovered: .gitignore:64

## Resume point (for the S1 builder)
S0 is complete at 3dac692. Next: S1 (plan-bf9b S1 + ruling-bf9b amendments 1, 2(a)(b)) on lane/bf9b; the S1 first commit
carries the symbol -> file:line table (amendment 1). Do NOT rebase until Harmony says (main has hyg + mkvidx; ui next).
INBOX-RECHECK: none

## S1 (ClipRef in the tuple, inert; plan S1 + ruling-bf9b amendments 1, 2(a)(b)) -- builder started 19:04
STATUS(S1): DONE
### S1 progress log (appended per item)
- 19:04 read the lane report, plan-bf9b (incl. HARMONY ADOPTION) and ruling-bf9b in full. Disk 291 GiB free.
- 19:06 symbol -> file:line table generated (scratchpad/bf9b-S1/symtab.py; 90 rows, 0 missing cells, both 11820fa and
  3dac692) -- goes into the S1.1 commit message (amendment 1).
- 19:08 RED (tests written first, src untouched = STAGE_P_HEAD src; `cmake --build build-lane --target test_layer_runtime
  -j3` rc=2, 20 error lines before the error limit; log scratchpad/bf9b-S1/red-build.log). First lines VERBATIM:
  `tests/test_layer_runtime.cpp:49:46: error: use of undeclared identifier 'ClipRef'`
  `tests/test_layer_runtime.cpp:86:52: error: no member named 'activeDeckId' in 'LayerRuntimeSnapshot'`
  `tests/test_layer_runtime.cpp:93:21: error: no member named 'activeRef' in 'LayerRuntimeSnapshot'`
  = the ruled RED form ("does not compile on STAGE_P_HEAD (no activeDeckId)").
- 19:09 GREEN, target only (`cmake --build build-lane --target test_layer_runtime -j3` rc 0, 0 errors, 0 warnings in
  ClipRef.h / Layer.h / test_layer_runtime.cpp): `test_layer_runtime "[layer_runtime]"` -> `All tests passed (2474
  assertions in 17 test cases)`; `"*bf9b S1*"` -> `All tests passed (1366 assertions in 2 test cases)`.
- 19:09:53-19:11:52 full incremental build `cmake --build build-lane -j3` rc=0, 0 error lines, 152 objects recompiled
  (29 of the app target incl. MainComponent.cpp; app binary 19:10:53). Warnings located in the 3 touched files: 0
  (STAGE_P_HEAD clean-build log: 0).
- 19:12:25-19:12:57 B2 at the S1 tree (`ctest --test-dir build-lane -j3 --output-on-failure`, log
  scratchpad/bf9b-S1/ctest-s1.log), VERBATIM:
  `100% tests passed, 0 tests failed out of 1119`
  count 1119 = STAGE_P_HEAD 1117 + 2 added (#1098 "LayerRuntimeCell packs a ClipRef per slot: deck ids and columns
  round-trip at 0 and the limits, a ref without a deck round-trips, pending + snap still share the last word (bf9b
  S1)", #1099 "ClipRef packing is a bijection on the valid domain (bf9b S1)"); 0 retired. #1097 "LayerRuntimeCell:
  pack / unpack round trip over the whole range" (amendment 2(b) edits) Passed.
- COMMIT 475b716 `feat(s-rta-1002b bf9b): S1.1 -- ClipRef (deck id, column) packed per slot into the 16-byte trigger
  tuple (inert)` -- src/model/ClipRef.h (new), src/model/Layer.h, CMakeLists.txt (ClipRef.h listed beside Layer.h),
  tests/test_layer_runtime.cpp. Its message carries the amendment-1 symbol -> file:line table (90 rows: every plan
  4.C block + amendment 26's additions, at BF9B_BASE 11820fa AND at STAGE_P_HEAD 3dac692, first `grep -n` match).
  Build rc at 475b716: app + tests rc 0 (19:11:52; rebuilt again 19:15:51 after the mutants, rc 0).

### S1 encoding (as built; the bijection test pins it)
- active / previous int32 = (deckField16 << 16) | uint16(column); deckField16 = 0xFFFF for kNoDeck, else the id.
  So no clip and no deck = -1 (as before, plan F2's literal), (deck d, column c >= 0) = d << 16 | c (plan F2's literal),
  and (deck d, column -1) = d << 16 | 0xFFFF.
- pending = low 28 bits (deckField14 << 14) | (column + 1), deckField14 = 0x3FFF for kNoDeck; snap override in the
  high 4 (unchanged). Limit: deck 0x3FFE + column 0x3FFE = 0x0FFFBFFF (plan S1's number; pinned).
- DEVIATION from plan F2's literal pending form `(deck << 14 | column) + 1`: identical for every column >= 0, but that
  form maps (any deck, column -1) to 0 = "no deck", which breaks amendment 2(a)'s bijection over deck x column {-1, ...}
  in the PENDING slot. The built form stores column + 1 in its own 14 bits, so every (deck, -1) round-trips. The
  no-pending default word is therefore 0x0FFFC000, not 0 (never serialized, R-F14; pack is the only writer).
- LayerRuntimeSnapshot: activeDeckId / previousDeckId / pendingDeckId appended AFTER pendingTriggerSnapOverride (every
  positional `{a, p, prog, pend, snap}` and designated initialiser keeps compiling, zero caller edits); operator==
  compares them. ClipRef::valid() = deck <= kMaxDeckId and 0 <= column <= kMaxColumn (a deck-less ref is not valid).
  pack does NOT jassert in S1 (amendment 2(c) adds the refusal + jassert from S2 on).

### S1 teeth (mutants; scratchpad/bf9b-S1/mutants.sh; each = an in-place edit of the committed Layer.h in build-lane,
target test_layer_runtime only, restored by cp from a scratch copy; sha256 prefix 209e48dcec14ed7a before and after
every mutant, `git diff -- src/model/Layer.h` 0 lines after each; build-lane fully rebuilt from the restored source
19:13:51-19:15:51 rc 0)
| mutant | packing case (#1098) | bijection case (#1099) | whole-range case (#1097) |
|---|---|---|---|
| MS3 (ruling amendment 13): active / previous pack the no-deck field whatever the deck | FAILED | FAILED | passed |
| MS3b: pending packs the no-deck field whatever the deck | FAILED | FAILED | passed |
| OFF1 (plan S1 risk, 14-bit field off by one): pending deck shift 13 instead of 14 | FAILED | FAILED | FAILED |
| COLM: active / previous column decoded as 14 unsigned bits (no sign) | FAILED | FAILED | FAILED |
Raw: `test cases:    2 |    0 passed |   2 failed` for each mutant's "*bf9b S1*" run. MS3 is ruling-bf9b's S2b smoke; it is
recorded here early because it is S1's own test teeth (S2b may re-record it).

### B3 TSAN at S1's end (`.harmony/probe-tsan-unit.sh`, worktree build-tsan, 19:15:51-19:16:02, exit 0; log
scratchpad/bf9b-S1/tsan-s1.log; test_layer_runtime_race / test_manual_scalar_race recompiled Layer.cpp + their tests
with the new Layer.h), every non-compiler line VERBATIM:
```
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 19:15:51
probe-tsan-unit: ctest -L tsan finds 4 [tsan] cases (expected 4)
probe-tsan-unit: ctest -L tsan 2026-10-02 19:16:01
Test project /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/build-tsan
    Start 111: R1 message-thread triggers vs render clock / autopilot on one deck
1/4 Test #111: R1 message-thread triggers vs render clock / autopilot on one deck ......   Passed    0.31 sec
    Start 112: R2 clip runtime fields: trigger writes vs render transport write-back
2/4 Test #112: R2 clip runtime fields: trigger writes vs render transport write-back ...   Passed    0.31 sec
    Start 113: R4 tuple consistency and no lost fade under a paced trigger storm
3/4 Test #113: R4 tuple consistency and no lost fade under a paced trigger storm .......   Passed    0.28 sec
    Start 114: R3 manual scalar writes vs eff() reads
4/4 Test #114: R3 manual scalar writes vs eff() reads ..................................   Passed    0.25 sec
100% tests passed, 0 tests failed out of 4
Label Time Summary:
tsan    =   1.15 sec*proc (4 tests)
Total Test time (real) =   1.16 sec
probe-tsan-unit: ctest rc=0 2026-10-02 19:16:02
```
"WARNING: ThreadSanitizer" count: 0. B3 at S1's end: 4 / 4 PASS (the ruled S1 count).

### S1 RESULT (19:17)
S1 DONE: one commit 475b716 on 3dac692 (+ this report commit). B1 (per commit) rc 0; B2 1119 / 0 failed (+2 added, 0
retired); B3 4 / 4, 0 TSan warnings; RED = does not compile on STAGE_P_HEAD (verbatim above); mutants MS3 / MS3b / OFF1 /
COLM each fail both new cases. No live app was launched in S1 (inert; no K row runs before S4) -- no lock taken.
The graphify post-commit hook launched a background graph rebuild on commit (log ~/.cache/graphify-rebuild.log); not
the lane's, nothing in the worktree changed (git status clean apart from this report).

## Resume point (for the S2a builder)
S1 is complete at 475b716 (BF9B tree builds app + tests; ctest 1119 / 0). Next: S2a = plan S2.1-S2.11 (src) +
ruling-bf9b amendments 2(c), 3(a) writers fenced, 4, 5, 6, 7, 8, 9(a)-(c), 12 (padStateFor), 18 (Renderer half), 19,
20, 22, 26 -- the sanctioned non-building window opens (app builds at 2a's end, tests at 2b's end). Resolve every plan
line by SYMBOL from the 475b716 commit message's table (the 3dac692 column = S1's parent; S1 touched only Layer.h's
tuple section, so Layer.h lines after :45 moved by +38: triggerClip :386 -> :424, processPendingTrigger :423 -> :461, clearActiveClip :465 -> :503 at 475b716; re-grep). The deck-less ClipRef form (kNoDeck with a
column) is S1's interim only: S2's tuple-writing entries refuse it (amendment 2(c)) and pack jasserts it. Arms for K
rows: scratchpad/bf9b-S0/apps/stagep-head.app (STAGE_P), apps/base-c0.app (C0); REF dir for a4
scratchpad/bf9b-S0/live/run1-base/rstate.O2yu7p. Do NOT rebase until Harmony says.
INBOX-RECHECK: none

## S2a (the model split + trigger API + C1/C2/C3 + the S2a src items; ruling-bf9b S2a row) -- builder started 19:17
STATUS(S2a): DONE
### S2a progress log (appended per item)
- 19:18 read the lane report, plan-bf9b (incl. HARMONY ADOPTION), ruling-bf9b in full, ruling-bf6 AM-1 / AM-6. Disk 291 GiB free.
- 19:40 model split + trigger API + C1/C2 written (src/model ClipRef.h RowClips, NEW ClipRow.h, NEW ShowMigration.h,
  Layer.h/.cpp, Deck.h, Composition.h), core (ClipCommands / DeckCommands / TriggerCommands / UndoService reap hook /
  EffectCommands / CompositionLoad / MediaPresence), Autopilot (show-wide, F10 source-deck advance), render
  (compositeShow, kShowStackKey, one show autopilot, deck fade + off-screen loop + tickMediaClock deleted; DeckClock.h,
  AutopilotBank.h, EmbeddedShaders::deckTransition, VideoPlayer::advanceClock deleted), ConnectionEngine (C2 walks),
  ManualWrite, MidiOutputHandler::padStateFor, recording (Program deckId pin + v2 preamble, PerfState v2, capture,
  RoutineEngine::stopOnLayer(layer), RoutineDeckView bands on shared layers), ui (DeckView::showDeck + shared strips,
  LayerStrip setLayer(layer, index, show)). App build (-k): every TU compiles except MainComponent.cpp, ApiServer.cpp,
  TestServer.cpp (in progress).
- 19:50 MainComponent.cpp (S2.7 / S2.8 rulebook pass), ApiServer.cpp (F8 top-level "layers" + "retiredDeckCount" +
  the mirror; amendment 4(g) debug routes remove_deck / undo), TestServer.cpp: app target `cmake --build build-lane
  --target AudioDNA -j3` rc=0, 0 error lines (build-app.log). No compile error needed a rule outside R1-R7 (no STOP).
- 19:53 RED-first for S2a's model API: NEW tests/test_show_model.cpp (T2, T3, T9, T10, T15) + CMake target
  test_show_model (builds alone: model sources only). RED = compiled against the S1 head's src (`git archive 475b716
  src`, scratchpad/bf9b-S2a/s1src) with the target's own flags: rc 1, 20 error lines, first VERBATIM:
  `tests/test_show_model.cpp:17:14: error: no member named 'getNumLayers' in 'Composition'`
  `tests/test_show_model.cpp:18:11: error: no member named 'insertLayer' in 'Composition'`
  GREEN (worktree): reconfigure rc 0, `--target test_show_model` rc 0, run: `All tests passed (64 assertions in 5 test cases)`.
- 19:55-19:58 live SMOKE (lock bf9b-S2a; build-lane app; production port): GET /api/composition carries top-level
  "layers" (3 shared layers, activeClip {deck,deckId,column,clipId,retired}), "retiredDeckCount" 0, decks[0].layers
  mirror (3 rows, activeClipColumn -1). The UNCHANGED probe-crossfade.py (an OLD-format show POST -> the converter,
  triggers, crossfades, pixel decode) on the new app, VERBATIM tail: `PY 35 PASS / 0 FAIL`, probe rc=0. LOOKED at
  xf/a_both_effected_mid04.png (a mid-dissolve of two effected pictures: both visible, no black frame). App quit by
  quit_app ("app running after quit: no"); Output-named windows 0; UserNotificationCenter windows 0 at 19:58:32.

### S2a commits (one per S2 sub-item group; the sanctioned non-building window -- only the LAST builds the app)
| sha | item | build |
|---|---|---|
| 8cc24c5 | S2a.1 model split (S2.1-S2.3): ClipRow.h, ShowMigration.h, Layer / Deck / Composition, RowClips, C1 / C2, CompositionLoad / MediaPresence walks, test_show_model (RED-first) | test_show_model target rc 0 (model only) |
| 02952c1 | S2a.2 commands (S2.4) + amendment 4(a)-(c) reap / retire, 22 | (window) |
| 1b1f1ca | S2a.3 render + autopilot (S2.5 / S2.6) + amendments 18 (Renderer half), 26; DeckClock.h, AutopilotBank.h deleted | (window) |
| 3ea6f4b | S2a.4 recording (S2.9) + amendment 6 | (window) |
| e26a78f | S2a.5 connections / pad lights / grid (S2.10 / S2.11) + amendment 12 padStateFor | (window) |
| 2db77eb | S2a.6 MainComponent / ApiServer / TestServer (S2.7 / S2.8) + amendments 4(g), 7(a), 8, 19, 20 | APP rc 0 (`cmake --build build-lane --target AudioDNA -j3`, 20:01:49) |

### S2a evidence (raw lines)
- App build at 2db77eb: rc=0, 0 ` error:` lines. Warnings in the 18 touched .cpp TUs (syntax pass, scratchpad
  warn-touched.txt): 19 lines, every one on a code line that exists verbatim at 475b716 (checked line by line:
  LayerStrip.cpp 617 / 706 / 819, MainComponent.cpp 1505 / 2294 / 4746 / 6396, ApiServer.cpp 1081 / 1904, Autopilot.h
  lastEnergyState_, CompositorEngine.cpp totalCells, TestServer.cpp CGL deprecations) -- no new warning.
- test_show_model: RED vs the S1 head's src = compile errors (above); GREEN `All tests passed (64 assertions in 5 test cases)`
  at 2db77eb (rebuilt 20:01).
- Full tree `cmake --build build-lane -j3 -- -k` (19:58:51-20:00:51, rc=2 by design): 96 of 116 test targets build; the
  20 that do not compile yet (S2b's list): test_autopilot test_composition test_compositor test_connection
  test_deck_clock test_deck_thumbnails test_layer_runtime test_layer_runtime_race test_layer_state_key
  test_layer_strip_follows_model test_layer_strip_transport_view test_manual_scalar_race test_manual_write
  test_media_presence test_program_preamble test_recorder_host test_routine test_routine_engine test_undo_commands
  tool_routine_deck_snapshot. (No ctest / TSan run at S2a: the sanctioned window; B2 / B3 run at S2b's end.)
- Live smoke (above): unchanged probe-crossfade.py on the S2a app `PY 35 PASS / 0 FAIL`.
- Text smokes at 2db77eb: B4a terms (DeckClock|AutopilotBank|deck_transition|deckTransition|prevDeckFBO_|
  cancelPendingTriggers|compositeDeck|tickMediaClock|advanceClock) 0 hits in src, comments included;
  globalTransitionSpeed: TopBar.cpp:197 / :204 (S3.3's half), Composition.h field + initDefault (TopBar's target until
  S3.3; never saved, no reader), ShowMigration.h (the allowed literal). B4b: 0 `getActiveDeck(` / `activeDeckIndex` in
  src/render/* and Autopilot.cpp (Renderer::getActiveDeck renamed getFenceToken; its one caller UndoService.cpp).
  B4c: Renderer.cpp reads activeDeck_ once (`activeDeck_.view()`, :390) and never dereferences .ptr. B4d (smoke): the
  bodies of handleDeckSwitch, onDeckSwitched and DeckView::showDeck contain none of
  triggerClip|clearActiveClip|setRuntime|updateRuntime|cancelPending|refreshPreview.

### Amendment 3(a): FENCE AUDIT TABLE at 2db77eb (B4f regex over code lines, ClipRow.h / Deck.h / Composition.h /
ShowMigration.h excluded; scratchpad/bf9b-S2a/b4f.py; counts per file: MainComponent.cpp 25, ClipCommands.h 2,
CompositionLoad.h 1, DeckCommands.h 18, UndoService.cpp 2, RoutineEngine.cpp 2, ClipCell.cpp 1, ClipCell.h 1,
ClipInspector.cpp 1, ClipInspector.h 1, DeckView.cpp 2, InspectorPanel.cpp 1)
| site | writer | fence |
|---|---|---|
| MainComponent.cpp:800 | onMultiVideoDropped `row.clips.resize` growth | withDeckDetached :792 |
| MainComponent.cpp:887 | onMixedFilesDropped growth | withDeckDetached :880 |
| MainComponent.cpp:986, :1003 | onEffectDropped (empty cells) ensureColumns / setClip | withDeckDetached :981 |
| MainComponent.cpp:1115, :1141 | onSourceDropped | withDeckDetached :1109 |
| MainComponent.cpp:1193, :1212 | onClipMoved ensureColumns (+ the cell writes) | withDeckDetached :1190 |
| MainComponent.cpp:1282, :1284 | onMilkDropDropped | withDeckDetached :1280 |
| MainComponent.cpp:1358, :1360 | onMilkDropPlaylistDropped | withDeckDetached :1356 |
| MainComponent.cpp:1583 | onSourceActivated setClip | withDeckDetached (same line) |
| MainComponent.cpp:5319 | commitDrop deck->setClip | every caller fences: :804 in :792, :890 in :880, :5337, :5474 in :5464, :5503 |
| MainComponent.cpp:5471 | handleMultiFileDrop (2 images) growth | withDeckDetached :5464 |
| MainComponent.cpp:6631, :6632 | kDeckClearClips row clear | withDeckDetached :6622 |
| MainComponent.cpp:6715, :6716 | kLayerClearClips row clear | withDeckDetached :6713 |
| MainComponent.cpp:6825 / :6849 | kColumnNew addColumn / kColumnRemove removeColumn | withDeckDetached (same lines) |
| MainComponent.cpp:6889 | kClipClear clearCell | withDeckDetached :6884 |
| MainComponent.cpp:2945, :3794, :5242 | ClipInspector::setClip (a UI setter, regex over-count, R-3) | n/a (not a model writer) |
| ClipCommands.h:113 / :252 | SetClipCmd / SwapClipsCmd apply | runFenced = DeckFenceHook (MainComponent::makeDeckFence -> withDeckDetached) |
| CompositionLoad.h:46 | validateDeck pad | staged, unpublished composition / deck (Pitfall 58) or headless test |
| DeckCommands.h:75 / :126 / :153 | SetColumnCountCmd / RemoveColumnCmd execute / undo | runFenced |
| DeckCommands.h:251 | clearedLayerClips `s.clips.assign` (a local snapshot, regex over-count) | n/a |
| DeckCommands.h:494 / :500 / :510 | AddLayerCmd redo / do / undo | runFenced |
| DeckCommands.h:568 | RemoveLayerCmd execute eraseLayer | runFenced |
| DeckCommands.h:636 | MoveLayerCmd moveLayer | runFenced |
| DeckCommands.h:702 / :723 | AddDeckCmd appendDeck / undo decks.erase | runFenced |
| DeckCommands.h:783 / :812 / :814 / :830 / :832 | InsertDeckCmd redo insertLayer / do insertLayer + appendDeck / undo decks.erase + eraseLayer | runFenced |
| DeckCommands.h:913 / :941 | RemoveDeckCmd retireOrEraseDeck / undo restoreRetiredDeck | runFenced |
| UndoService.cpp:70 / :112 | reapRetiredDecks | inside withDeckDetached itself (headless pass-through / the fenced scope, before the restore) |
| RoutineEngine.cpp:68 / :77 | Footprint `fp.layers` (a local vector) | n/a (regex over-count) |
| ClipCell.* / ClipInspector.* / InspectorPanel.cpp / DeckView.cpp:190, :286 | UI `setClip` setters | n/a (regex over-count) |
Structure writers the B4f regex does NOT name (new model methods; listed so the audit is complete -- S2b decides whether
B4f's method list gains them): DeckCommands.h:586 `insertLayerWithRows` (RemoveLayerCmd undo, runFenced); :693 / :784 /
:943 `insertDeckKeepingId` (AddDeckCmd redo, InsertDeckCmd redo, RemoveDeckCmd undo; runFenced); Composition::fromVar /
normalizeRows / padRows (CompositionLoad.h:62 validateComposition on the STAGED composition; appendDeck's pad inside the
callers' fences); MainComponent.cpp:3231 `composition_ = std::move(s->comp)` and :6482 `initDefault()` (both inside
swapCompositionModel's withDeckDetached); MainComponent.cpp:514 constructor initDefault (before any setActiveDeck: the
GL thread reads nothing yet).

### Amendment 9(a): STORE TABLE (what an old show's per-deck layer data becomes)
| store | lives on | at conversion |
|---|---|---|
| layer settings incl. layer effects, layer scalar connections, per-layer autopilot settings | the Layer (R-F18) | the winning (first) deck's row keeps its own; a dropped row loses its own and the note names it when it differs |
| ByPosition / Selected bindings | (layer index, column) on the shown deck | no migration |
| ThisItem bindings | a clip id | no migration (now searched in every live deck) |
| routine / take keys | deck index + name, layer index + name | Layer scope resolves the shared layer by position / name; Clip scope as today on rows |
| PerfState v1 (no "layers") | per deck | shared layers restored from its captured active deck only (M5) |
| PerTypeAutopilotConfig | Composition | untouched |

### S2a deviations / decisions (for the reviewer)
1. RowClips gained `cellsFn` + `hasCell()` beside the plan's `fn` / `ctx` / `row` / `at()`: a trigger must still tell
   "no such cell" (unchanged, as before) from "an empty cell" (clears the layer, F12).
2. ClipRef::storable() added: the domain the tuple may hold ("none", a deck with column -1, a valid ref); used by
   pack's jassert and by onlyIfActive refusals; trigger targets must be valid().
3. LayerStrip::setLayer(Layer*, int, Composition*) -- non-const (the strip's transport buttons write the playing clip);
   LayerStrip::transportViewOf(const Clip* playing, bounds) replaces (const Layer*, bounds) (its ctest follows in S2b).
4. The column header {deckId, column} is lit only on the deck it was fired from already in S2a (S2.7 "remembers";
   16(e) defines it) -- the only on-screen grid change before S3.
5. Renderer::getActiveDeck renamed getFenceToken (B4b's zero `getActiveDeck(` in src/render/*).
6. The Layer Router reads composition_->layers only inside the frame's deckActive gate (Renderer::showReadable_; a
   standalone layer_router source on a fenced frame returns 0, as the old null deck did).
7. Composition::globalTransitionSpeed stays as a field until S3.3 (TopBar's Fade slider writes it); never saved, no
   reader.
8. tests/test_show_model.cpp (+ CMake target) lands in S2a with T2, T3, T9, T10, T15 as S2a's RED-first evidence; S2b
   extends the same file (T1, T4-T8, T11-T14, T16, M*).
9. handleClipTrigger / handleColumnTrigger refuse a ref that is not valid() at entry (amendment 2(c) carried to the app
   entries; a column > 16,382 cannot exist in a validated deck).
10. Load Deck / New Deck past the deck-id cap: refused with the 7(a) text in the file label + a logLine (the load-notice
    label is S3.4).
11. migrationNote is logged in finishStagedLoad after the swap (a cancelled staged load logs nothing).
12. RemoveDeckCmd's undo does not restore the triggers cancelPendingInto cancelled (runtime undo stays imperfect,
    DeckCommands.h's spec risk #5 family).
13. A row clear (Clear Deck / Layer Clips) touches the shared tuple only when its ACTIVE or PENDING ref is in that row of
    that deck; a fading-out (previous) ref into it just loses its outgoing clip (the fade cuts).
14. Momentary bindings: the press records (layer, ref) per Binding::id (MainComponent::momentaryRefs_); a release with
    no record does nothing.
15. PerfState capture: a layer playing a removed (retired) deck's clip records activeClipColumn -1 (nothing a take can
    restore).
16. deriveRoutineDeckView: pad.onShownDeck is always true and the "on Deck N" corner note is gone (every routine plays
    on the shared layers, on screen).
17. GET /api/composition: a ref's "retired" = it names a deck not among the live decks (REST never reads the retired
    list's elements); "retiredDeckCount" reads its size (the decks.size()-class read tsan-r5 owns).

## Resume point (for the S2b builder)
S2a is DONE at 2db77eb (app builds; test_show_model GREEN). Next: S2b = plan S2.12 (tests) + T1 (amendment 12),
T6c-e (4), T7 + T7b (21, 22), T11 (7), T13-T16, M1 + M6 + M7, the R-bf9b TSAN case (3(c); EXPECTED_TSAN_CASES 4 -> 5),
B4f (3(b); decide on deviation 3(a)-list: insertLayerWithRows / insertDeckKeepingId), mutation smokes MS1-MS7 (13).
Start with the 20 test targets listed above (they do not compile); extend tests/test_show_model.cpp (5 cases there).
The persistent-key lint (test_render_thread_lint "no Persistent-feature identifier left in src/") must allow-list
src/model/ShowMigration.h (B4e); its pinned CompositorEngine.cpp counts change (compositeShow: one runtime() per layer,
no getActiveClip). ShowMigration's note format: "old show converted: layer settings come from the first deck that has
each row; <Deck> row N: settings dropped[; K layer effect(s), M connection(s) dropped]; 'persistent' ignored on:
<Deck> / <Layer>, ...; deck fade X.XX s dropped" (rows 1-based). Arms: STAGE_P = scratchpad/bf9b-S0/apps/stagep-head.app,
C0 = apps/base-c0.app; REF dir for a4 scratchpad/bf9b-S0/live/run1-base/rstate.O2yu7p. Do not rebase until Harmony says.

## Notes for .harmony/notebook.md (Harmony appends)
- bf9b: a layer's clip is Composition::playing(i) / playingClip(i) (a (deck id, column) ref into ANY live or retired
  deck) -- never getActiveDeck()->rows[i]; the shown deck is only the grid. | discovered: src/model/Composition.h playing()
- bf9b: every fenced edit (UndoService::withDeckDetached) reaps retired decks no ref names and hands them to
  onDecksReaped after the fence; a headless test without a renderer reaps too. | discovered: src/core/UndoService.cpp
- `git commit -m ... -- <paths>` commits ONLY those paths (git rm'd files included) -- path-scoped commits without
  touching the index of other files. | discovered: scratchpad/bf9b-S2a/commit.sh
INBOX-RECHECK: none

## S2b (tests: the 20 non-compiling targets, test_show_model extended, R-bf9b TSAN, B4f, B4e, MS1-MS7) -- builder started 20:06
STATUS(S2b): DONE
### S2b progress log (appended per item)
- 20:06 read the lane report, plan-bf9b (incl. HARMONY ADOPTION) and ruling-bf9b in full. Disk 289 GiB free. Branch
  lane/bf9b at 19b9258 (S2a code 2db77eb). Rig slip: one read-only inspection command began with `cd /dev/null` (it
  failed; no effect) -- not repeated.
- 20:07 full -k build (build0.log): exactly the 20 listed targets fail (195 capped error lines). NEW tests/ShowFixture.h
  (makeShow(decks, layers, columns, fill) / row / fire / ref); test_show_model's local makeShow moved there.
- 20:08 test_layer_state_key (fixture: two stack halves over the show's shared layer ids; case 1's premise -- ids
  repeated across decks -- is gone, the key function is still pinned): `All tests passed (324 assertions in 3 test cases)`.
- 20:08 test_deck_clock RETIRED (git rm): (a)-(d), (f) retired with DeckClock / AutopilotBank (4.B); (e) moved
  unchanged (minus `layer.ensureColumns(2)`, Layer has no clips) to NEW tests/test_layer_clock.cpp, CMake target
  renamed test_layer_clock: `All tests passed (6 assertions in 1 test case)`.
- 20:11 test_layer_runtime: fixture Rig (one-deck show, deck id 0; Rig::S writes a column tuple with deck ids -- deck 0
  for a column, none for -1: the tuple's type now carries the deck, values unchanged); the 4.B case renamed
  "a queued trigger cancelled by cancelPendingInto never fires" (same expectation): `All tests passed (2475 assertions in 17 test cases)`.
- 20:14 test_layer_runtime_race: R1 / R2 / R4 on a one-deck show (R1's GL driver = LayerClock::tick per shared layer
  + the show autopilot, DeckClock gone; cancelPendingInto; the live Layer copy checks id). NEW "R-bf9b fenced box and
  stack edits vs the GL resolve of refs into any deck" [tsan][layer_runtime] (amendment 3(c); emulated fence =
  detachFenced + drain 2 GL loop passes + mutate + reap + set; cells / columns / ClearLayerClipsCmd + undo /
  RemoveDeckCmd retire + undo / retire, re-fire, reap, re-insert / insertLayer / moveLayer / eraseLayer; unfenced
  triggers interleave; the GL checks every resolved Clip* lies in a live or retired deck's row storage).
  FOUND (src, S2a): the normal-build run went RED on `reapsAfterRefire == 166` -> `0 == 166` ("frames 6769, held 94614,
  resolved 40235, outside 0, retires 167, restores 167, reaps after a re-fire 0"): a Cut (transitionSpeed <= 0, the
  layer DEFAULT -1 included) or a clear leaves `previous` naming the old clip at progress 1, so
  Composition::deckIsPlaying kept a retired deck alive forever after its layer was replaced. Fix (amendment 4(b)
  / R-F6 intent: "a fade OUT keeps it alive until the fade completes"): previous counts only while
  crossfadeProgress < 1. Then `All tests passed (1204 assertions in 4 test cases)`.
- 20:16 small fixtures: test_manual_scalar_race / test_manual_write (comp.layers / comp.getLayer), test_media_presence
  (one shared layer + one row), tool_routine_deck_snapshot (setLayer(&layer, i, nullptr)), test_recorder_host
  (makeComposition = Composition::initDefault; the [perfstate] case reads the shared layers' opacity / bypassed from
  PerfState v2 `layers`, the clip from decks[0] row 0), test_routine (rows / comp.layers).
- 20:17 test_routine_engine D3: 4.B OMISSION (for Harmony): stopOnLayer(layer) has no deck any more (plan S2.9 / F9
  "stops every running routine touching that shared layer, whatever deck"), so the step `stopOnLayer(1, 0); // another
  deck: nothing` (+ its two CHECKs) cannot be expressed and was removed; the case is renamed "... stops every routine
  on that shared layer, whole, grips released"; every other assertion unchanged. `All tests passed (1145 assertions in 37 test cases)`.
- 20:18 FOUND (src, S2a): test_routine:509 `CHECK( e.norm == Approx(0.4f) )` -> `1.0f == Approx( 0.4 )`: RoutineSlice
  still read a layer's checkpoint settings from cp0.decks[deck].layers (PerfState v1), but v2 keeps them in
  cp0.layers. Fix: RoutineSlice::checkpointLayerSettings (v2 `layers` by position, else v1 per deck) for Layer scope;
  Clip scope stays per deck. Then `All tests passed (201 assertions in 8 test cases)`.
- 20:19 test_program_preamble: the 4.B row (:196) applied -- Layer scope resolves the shared layer, so only Clip scope
  can miss a deck: deck 9's row gets a clip runtime (the unresolved count stays 2, deck + layer), and "deck 0's
  entries are still present" checks a Layer-scope entry on layer 0 (its target carries no deck). Cases 1 / 2 / 6:
  shared layer + deck-0 rows (+ activeDeckId), case 6 reads checkpoint0.layers (v2). `All tests passed (169 assertions in 6 test cases)`.
- 20:20-20:23 test_connection / test_deck_thumbnails / test_layer_strip_follows_model / test_layer_strip_transport_view /
  test_autopilot / test_compositor: fixture rewrites (comp.layers, deck rows, Composition::fire / playingClip,
  LayerStrip::setLayer(layer, i, show), transportViewOf(playingClip)); test_compositor's deck-transition comment
  reworded (plan addendum). All GREEN (raw: `All tests passed (159 assertions in 39 test cases)` connection, `(62 / 4)`
  thumbnails, `(42 / 8)` follows_model, `(23 / 6)` transport_view, `(49 / 11)` autopilot, `(42 / 8)` compositor).
- COMMITS fbe2adb (S2b.1 deckIsPlaying fix), 740f0b2 (S2b.2 RoutineSlice v2), 0cc35a4 (S2b.3 tests, 18 targets).
- 20:25 test_composition GREEN `All tests passed (369 assertions in 28 test cases)` (4.B :1322 imagePaths applied + a
  NEW SECTION for a non-shown deck; duplicateDeck: rows only -- its layer-id / pending-trigger checks are 4.B
  omissions, plan S2.3 "there is no tuple to clear any more").
- 20:32 test_undo_commands: 81 -> 82 cases. FOUND (src, S2a): `REQUIRE( comp.getNumLayers() == 3 )` -> `4 == 3` in
  "RemoveLayerCmd: stale coordinate is a safe no-op": its undo inserted the layer even when execute had refused. Fix
  (erased_ flag). Then `All tests passed (575 assertions in 82 test cases)`.
- 20:34 FIRST FULL BUILD of the window's end: `cmake --build build-lane -j3 -- -k` rc=0, 0 error lines (build1.log):
  the app + all test targets build. ctest (`ctest --test-dir build-lane -j3 --output-on-failure`, ctest1.log):
  `99% tests passed, 3 tests failed out of 1121` -- #690 RoutineDeckView off-deck (a 4.B omission: plan S2.9 / F9 make
  bands show whatever deck is shown; re-pointed), #1088 tuple-load pins (re-pinned), #1089 Persistent lint
  (allow-list ShowMigration.h). Rig slip: that ctest command line began with `cd <worktree> 2>/dev/null; true;`
  (no effect on the run; not repeated).
- 20:37 test_render_thread_lint: re-pins + B4e + NEW B4f, `All tests passed (656 assertions in 4 test cases)`. RED vs
  STAGE_P_HEAD 3dac692's src (scratchpad red_lint.sh): every new / changed check FAILS; B4f teeth (an extra
  `c.decks.push_back` in a src copy): `core/MediaPresence.h: 1 (pinned 0)`, case FAILED.
- COMMITS 10c80a9 (S2b.4 RemoveLayerCmd undo fix), fd1bc5e (S2b.5 tests + lint).
- 20:39 refactor for testability (no behaviour change): recording Program pinnedDeckIndex (dispatch.fire's pinned
  deck) + NEW binding/BindingTarget.h resolveBindingTarget (handleBindingAction's target) -- so T13 / T16 / MS6 drive
  the code the app runs. App build rc 0 (20:40:16).
- 20:43-20:48 test_show_model: + T1 (two cases: the 6-deck fingerprint over the walk / SwitchDeckCmd / headless
  DeckView::showDeck walk 0 -> 5 -> 0 with same-object strips / Add-Insert-Remove Deck execute-undo-redo, with a
  routine running and queues on two layers; and a 20-deck walk), T4, T5, T6, T6c, T6d, T6e, T7 (+ amendment 22),
  T7b, T8, T11, T12, T13, T14, T16, M1-M7. Link set widened (commands, UndoService headless branch -- its Renderer
  calls are inline, Autopilot, RoutineEngine set, DeckView set, MidiOutputHandler). `All tests passed (16818
  assertions in 28 test cases)`. RED: the file against STAGE_P_HEAD 3dac692's src (red_show.sh, test_show_model's own
  flags, -fsyntax-only): `tests/test_show_model.cpp:12:10: fatal error: 'model/ShowMigration.h' file not found`.
- 20:49-20:50 FULL BUILD `cmake --build build-lane -j3 -- -k` rc=0, 0 error lines, 0 warning lines (build2.log,
  incremental). FULL CTEST (`ctest --test-dir build-lane -j3 --output-on-failure`, ctest2.log), VERBATIM:
  `100% tests passed, 0 tests failed out of 1145` / `Total Test time (real) =  33.78 sec`.
- COMMITS f8b6f3a (S2b.6 refactor), a269971 (S2b.7 test_show_model).
- 20:51 B3 TSAN: .harmony/probe-tsan-unit.sh EXPECTED_TSAN_CASES 4 -> 5 (TARGETS unchanged); build-tsan reconfigured
  (`cmake -S <wt> -B <wt>/build-tsan` rc 0); probe exit 0 (scratchpad tsan-s2b.log), every non-compiler line VERBATIM:
```
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 20:51:27
probe-tsan-unit: ctest -L tsan finds 5 [tsan] cases (expected 5)
probe-tsan-unit: ctest -L tsan 2026-10-02 20:51:37
Test project /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/build-tsan
    Start 111: R1 message-thread triggers vs render clock / autopilot on one deck
1/5 Test #111: R1 message-thread triggers vs render clock / autopilot on one deck ..........   Passed    0.31 sec
    Start 112: R2 clip runtime fields: trigger writes vs render transport write-back
2/5 Test #112: R2 clip runtime fields: trigger writes vs render transport write-back .......   Passed    0.33 sec
    Start 113: R4 tuple consistency and no lost fade under a paced trigger storm
3/5 Test #113: R4 tuple consistency and no lost fade under a paced trigger storm ...........   Passed    0.26 sec
    Start 114: R-bf9b fenced box and stack edits vs the GL resolve of refs into any deck
4/5 Test #114: R-bf9b fenced box and stack edits vs the GL resolve of refs into any deck ...   Passed    0.29 sec
    Start 115: R3 manual scalar writes vs eff() reads
5/5 Test #115: R3 manual scalar writes vs eff() reads ......................................   Passed    0.24 sec

100% tests passed, 0 tests failed out of 5

Label Time Summary:
tsan    =   1.42 sec*proc (5 tests)

Total Test time (real) =   1.43 sec
probe-tsan-unit: ctest rc=0 2026-10-02 20:51:39
```
  "WARNING: ThreadSanitizer" count in the log: 0. The TSan binary is fresh (20:51:32 > the test source 20:14:12); a
  direct rerun of the R-bf9b case under TSan: `All tests passed (15 assertions in 1 test case)`.
- 20:54 NEW lint case "bf9b B4d / H2: a deck switch path touches nothing that plays (smoke, one level)" (ruling-bf10 H2
  via the plan's adoption item 2): GREEN `All tests passed (674 assertions in 5 test cases)`; RED vs STAGE_P_HEAD src:
  handleDeckSwitch / onDeckSwitched / SwitchDeckCmd::apply FAIL. COMMIT c7ddff3 (S2b.8, + EXPECTED_TSAN_CASES 5).

### MUTATION SMOKES (ruling-bf9b amendment 13) -- 20:54:31-20:55:42, scratchpad mutants.py / mutants.log
Each mutant is an edit of a COPY of the tree (scratchpad mut/tree, rsync of src / tests / cmake / resources /
CMakeLists.txt) built by a normal cmake build in its own build dir (mut/build, configured like build-lane); the
worktree is never edited. After the run the copy's files are restored (sha256 before == after: True), `diff -r` of
the copy's src vs the worktree's src is empty, `git diff -- src tests` in the worktree is 0 lines, and the restored
copy rebuilds and passes (`All tests passed (16818 assertions in 28 test cases)`, `All tests passed (2475 assertions in 17 test cases)`).
| mutant | edit (copy) | named tests | result (raw) |
|---|---|---|---|
| MS1 the shown-deck switch cancels the leaving deck's queued triggers | SwitchDeckCmd::apply: cancelPendingInto(leaving deck) | T5, T1 | `MS1 [T5*] rc=42: test cases: 1 \| 1 failed`; `MS1 [T1 every deck-switch*] rc=42: test cases:  1 \|  0 passed \| 1 failed` |
| MS2 clipAt ignores ref.deckId (the shown deck) | both Composition::clipAt overloads resolve in getActiveDeck() | T2, T9 | `MS2 [T2*] rc=42: test cases: 1 \| 1 failed`; `MS2 [T9*] rc=42: test cases:  1 \|  0 passed \| 1 failed` |
| MS3 pack writes the no-deck field in every slot | packSlot 0xFFFF + packPending 0x3FFF whatever the deck | S1 packing case, bijection case | `rc=42: test cases:  1 \|  0 passed \|  1 failed`; `rc=42: test cases:    1 \|    0 passed \|   1 failed` |
| MS4 the switch completes every fade | SwitchDeckCmd::apply sets crossfadeProgress 1 on every layer | T1 | `MS4 [T1 every deck-switch*] rc=42: test cases:  1 \|  0 passed \| 1 failed` |
| MS5 reapRetiredDecks ignores previous refs | reapRetiredDecks keeps a deck only for an ACTIVE ref | T6c | `MS5 [T6c*] rc=42: test cases:  1 \|  0 passed \| 1 failed` |
| MS6 dispatch.fire resolves by target index | pinnedDeckIndex returns target.deck | T13 | `MS6 [T13*] rc=42: test cases:  1 \|  0 passed \| 1 failed` |
| MS7 the strip badge shows the shown deck | -- | the S3 badge case | NOT RUN: the badge (S3.1) does not exist yet -- S3's builder records MS7 |
Every mutation made its named tests FAIL (no STOP).

### B2 bookkeeping vs STAGE_P_HEAD (names diffed: S0's ctest-head.log vs `ctest -N` now, scratchpad added.txt / retired.txt)
count(STAGE_P_HEAD) 1117 + added 47 - retired 18 = 1146 == `ctest -N` "Total Tests: 1146" (1145 at the 20:50 run + the
B4d / H2 lint case added after it).
- RETIRED 18: test_deck_clock (a), (b), (c), (d), (f) (4.B: DeckClock / AutopilotBank deleted); and 13 RENAMED (old
  name retired, new name added, the same case): "AddDeckCmd: cancels ..." / "InsertDeckCmd: cancels ..." /
  "SwitchDeckCmd: cancels ..." / "RemoveDeckCmd: undo cancels ..." x2 (4.B: now "... untouched"); "a queued trigger
  cancelled by cancelPendingTriggers ..." (4.B :156 -> cancelPendingInto); "Deck layer management" -> "Layer
  management on the shared stack"; "Deck::fromVar bumps the layer-id mint ..." -> "Composition::fromVar bumps ...";
  "Deck::triggerColumn: forced snap ..." -> "Composition::triggerColumn: forced snap ..."; "compload::imagePaths ..."
  (4.B :1322); "compload::duplicateDeck ... and no queued trigger" -> "... (a deck holds no tuple)"; "RoutineDeckView
  off-deck ..." -> "RoutineDeckView fired from another deck ..."; "RoutineEngine display D3 ... of that deck ..." ->
  "... on that shared layer ...". (test_deck_clock (e) kept its name in test_layer_clock.)
- ADDED 34 new: S1's 2 (packing, bijection); test_show_model 28 (T1 x2, T2-T16 with T6c / T6d / T6e / T7b, M1-M7);
  "ClearLayerClipsCmd: a layer playing another deck's clip keeps playing (bf9b)"; "R-bf9b fenced box and stack edits
  ..." [tsan]; "bf9b: box / stack structure writers sit only in audited sites (pinned counts)" (B4f); "bf9b B4d / H2:
  a deck switch path touches nothing that plays (smoke, one level)". (+ the 13 renames above = 47.)
Required present and passing: S1 packing + bijection; T1 (amendment 12); T2-T5; T6 incl. T6c-e; T7 incl. T7b; T8-T10;
T11 (amendment 7); T12; T13-T16; M1-M7; test_layer_clock (e); test_render_thread_lint incl. B4f with its pins
re-justified (S2b.5 commit). The S3 badge / tab-dot / grid / snapshot / contrast cases are S3's.

### FENCE AUDIT TABLE at this head (B4f pins; the S2a table at 2db77eb holds, lines now +1 in MainComponent.cpp after
the S2b include; DeckCommands.h +2 after the S2b.4 fix) -- the B4f regex adds insertLayerWithRows / insertDeckKeepingId
(S2a deviation 3(a), decided here); every site and its fence:
| file (count) | lines | fence |
|---|---|---|
| MainComponent.cpp (25) | 801, 888, 987, 1004, 1116, 1142, 1194, 1213, 1283, 1285, 1359, 1361, 1584, 5319, 5471, 6631, 6632, 6715, 6716, 6825, 6849, 6889 | withDeckDetached (the S2a table's sites, +1); 2945 / 3794 / 5242 = ClipInspector::setClip (a UI setter) |
| core/ClipCommands.h (2) | 113, 252 | SetClipCmd / SwapClipsCmd apply: runFenced (DeckFenceHook) |
| core/CompositionLoad.h (1) | 46 | validateDeck: a staged, unpublished deck / composition (Pitfall 58) or a headless test |
| core/DeckCommands.h (22) | 75, 126, 153 (column cmds), 251 (a local snapshot `s.clips.assign`), 494, 500, 510 (AddLayerCmd), 568, 587 (RemoveLayerCmd execute / undo insertLayerWithRows), 638 (MoveLayerCmd), 695 (AddDeckCmd redo insertDeckKeepingId), 704, 725 (AddDeckCmd), 785, 786 (InsertDeckCmd redo insertLayer + insertDeckKeepingId), 814, 816, 832, 834 (InsertDeckCmd), 915, 943, 945 (RemoveDeckCmd execute / undo restoreRetiredDeck + insertDeckKeepingId) | runFenced (DeckFenceHook = UndoService::withDeckDetached in the app); 251 n/a |
| core/UndoService.cpp (2) | 70, 112 | reapRetiredDecks inside withDeckDetached itself |
| recording/RoutineEngine.cpp (2) | 68, 77 | a local Footprint vector `layers` (n/a) |
| ui/ClipCell.cpp / .h, ui/ClipInspector.cpp / .h, ui/InspectorPanel.cpp (1 each), ui/DeckView.cpp (2: 190, 286) | -- | UI setClip setters (n/a, regex over-count, ruling risk R-3) |
Not matched by the regex (listed so the audit is complete): Composition::fromVar / normalizeRows / padRows (a staged
composition: CompositionLoad validateComposition, or appendDeck's pad inside the callers' fences);
MainComponent's `composition_ = std::move(...)` and `initDefault()` inside swapCompositionModel's withDeckDetached; the
constructor's initDefault before any setActiveDeck.
- 20:56 test_tempo_start fixture -> Composition::initDefault (it compiled with a bare deck and NO shared layer; passes
  either way: no layer is read) `All tests passed (311 assertions in 13 test cases)`. COMMIT bb5d84e (S2b.9).

### S2b deviations / decisions (for the reviewer and Harmony)
1. THREE src bugs of S2a found by the S2b tests and fixed, RED-first (each its own commit):
   (a) fbe2adb Composition::deckIsPlaying -- a Cut (transitionSpeed <= 0; the layer DEFAULT is -1) or a clear leaves
       `previous` at progress 1, so a retired deck was never reaped once replaced; previous now counts only while
       crossfadeProgress < 1 (amendment 4(b) / R-F6's intent). Affects K9a/b/c and Boris page 8.5.
   (b) 740f0b2 RoutineSlice read a v2 take's layer settings from checkpoint0.decks (v1 shape): a routine sliced from a
       take recorded since S2a restored the wrong look.
   (c) 10c80a9 RemoveLayerCmd::undo inserted a layer even after its execute refused.
2. Two extractions for testability, no behaviour change (f8b6f3a): pinnedDeckIndex (recording/Program) and
   binding/BindingTarget.h -- so T13 / T16 / MS6 drive the code the app runs (amendments 6 / 20 and 13).
3. 4.B OMISSIONS -- assertions the plan body makes impossible but 4.B does not list. Each was re-pointed to the plan
   body (not left red) and is listed here for Harmony to rule (a revert is per-hunk):
   - test_routine_engine D3 "another deck: nothing" step removed (plan S2.9 / F9: stopOnLayer(layer) has no deck).
   - test_routine_deck_view "off-deck" case -> bands on the shared layers whatever deck is shown, no corner note (plan
     S2.9 / F9; S2a deviation 16).
   - test_composition duplicateDeck: its layer-id and queued-trigger checks removed (plan S2.3: a deck holds no
     layers and no tuple); rows / re-mint checks kept.
   - test_undo_commands: TriggerClipCmd's "stale DECK index" sub-steps -> a stale composition / stale layer index (plan
     S2.4: a trigger addresses the shared layer; plan R9 accepts an undo naming a reaped deck); AddLayerCmd /
     MoveLayerCmd "stale DECK index" -> null composition / stale layer index; RemoveLayerCmd "stale DECK" -> stale
     layer index.
   - test_layer_state_key case 1: its premise (layer ids repeat across decks) is gone; the two-half key function is
     still pinned with two stack halves over the show's layer ids.
   - test_recorder_host [perfstate] / test_program_preamble rr-fix: shared-layer settings read from PerfState v2
     `layers` (plan S2.9 "PerfStateCapture captures the shared layers once and, per deck, only clip runtime").
4. Expected tuples in test_layer_runtime / test_undo_commands carry deck ids (the tuple's type changed in S1/S2;
   every value is the old one plus "deck 0 for a column, none for -1").
5. B4f regex = the ruling's + insertLayerWithRows / insertDeckKeepingId (S2a deviation 3(a), decided: they are
   structure writers of the same class). Pinned map: DeckCommands.h 22 (was 18 + those 4).
6. Tuple-load lint: Composition::playing( / playingClip( counted as the successor spelling of getActiveClip( (one
   runtime() load inside) -- otherwise the GL thread's Autopilot / compositor / playlist loads would go unpinned.
7. T1's DeckView walk and every deck command run headless; the command steps call execute / undo / execute directly
   (not through UndoManager) so the fingerprint's history size is a constant of the walk; UndoService is linked into
   test_show_model (its Renderer calls are inline; no renderer is ever set).
8. T5 / T1 switch through SwitchDeckCmd (the model's switch command) so MS1 / MS4 bite; S2c deletes SwitchDeckCmd ->
   S2c must re-point T5's switch (and T1's SwitchDeckCmd section) to the remaining model-level switch (the index +
   DeckView::showDeck) and re-run MS1 / MS4 against it.
9. Mutation smokes ran on a COPY of the tree in its own build dir (never the deliverable; Iron Law 7).
10. MS7 not run: the strip badge is S3's (S3 records MS7).
11. H1 (m9b_deck_switch_live): bf10 has not merged -> a follow-up row for S4 / bf10's probe-milkdrop.py. H2: the
    B4d / H2 smoke lint (c7ddff3). H3: implemented in S2a (Renderer's playlist loop walks the shared layers'
    playingClip inside the deckActive gate); pinned by the tuple-load lint (Renderer.cpp playingClip 1).
12. Rig slips (no effect): one read-only command began `cd /dev/null`; the 20:34 ctest command began
    `cd <worktree> 2>/dev/null; true;`. Neither repeated.
13. probe-routines.sh was NOT run: it quits by name (osascript quit "Audio-DNA" + adna_kill), which the rig forbids
    (a Boris app could be quit); Harmony's merge gate runs it with its own safety.

### found_not_fixed (S2b)
- .harmony/probe-routines.sh (and other pre-S0 probes) still quit / kill by app name -- outside this lane's fence; a
  probe-safety follow-up (S0's quit_ours covers probe-render-state / probe-deck-clock only).
- K-row note for S4: the Cut-transition reap fix (S2b.1) is what makes K9b's "duplicate_deck reaps the retired deck"
  hold when a layer leaves a deck by a Cut; K9a/b fixtures that use Dissolve are unaffected.

### B1 / B2 at the S2b head bb5d84e (the window CLOSES here: app + every test target build)
- B1: `cmake --build build-lane -j3 -- -k` 20:57:43-20:57:48 rc=0, 0 error lines (build3.log; incremental after the
  20:49 full build of the same tree minus S2b.8 / S2b.9's test files). Per S2b commit: fbe2adb / 740f0b2 / 0cc35a4 /
  10c80a9 / fd1bc5e sit inside the sanctioned window (their trees built every target I had fixed so far; the window
  closed at fd1bc5e: the 20:34 full -k build of that content was rc 0); f8b6f3a app rc 0 (20:40:16); a269971 /
  c7ddff3 / bb5d84e: full builds rc 0 (20:50:14, 20:57:48).
- B2: `ctest --test-dir build-lane -j3 --output-on-failure` 20:57:48-20:58:21 (ctest3.log), VERBATIM:
  `100% tests passed, 0 tests failed out of 1146` / `Total Test time (real) =  33.09 sec`.
- B3: 5 / 5 at c7ddff3's tree (above; the S2b.9 change touches no [tsan] target).
- B4 smokes at bb5d84e (code lines, `//` stripped): B4a hits = TopBar.cpp:197 / :204 + Composition.h:144 / :232
  (`globalTransitionSpeed`: the TopBar Fade slider's field until S3.3 deletes that half -- unchanged since S2a) and
  ShowMigration.h:166 / :168 (the allowed JSON key literals); 0 other. B4b: 0 hits. B4c / B4d: unchanged since S2a (B4d
  now also a ctest smoke). B4e / B4f: ctest PASS.

### LIVE SMOKE at the S2b app (build-lane, bb5d84e's src; scratchpad bf9b-S2b/smoke1.sh -> live/smoke1.log)
Lock bf9b-S2b (waited for bf2-S2's lock 20:55:57-21:01:38), production port, `open -g` via start_app, quit via
quit_app. NOT a gate (the K rows are S4's); it shows the S2b src fixes / extractions did not break the app.
```
21:01:38 lock acquired
21:01:38 ps top:
 59.2       00:01 bash
 23.6 06-08:18:25 /usr/libexec/knowledge-agent
  7.5    07:12:57 claude
  6.6 06-08:18:28 /System/Library/PrivateFrameworks/SkyLight.framework/Resources/WindowServer
  3.1 06-08:18:23 /Applications/Ghostty.app/Contents/MacOS/ghostty
21:01:42 app up: 81771
===== GET /api/composition (default show)
top-level keys: ['activeDeck', 'decks', 'layers', 'numDecks', 'retiredDeckCount']
shared layers: [(0, 0, 'Layer 1', {'deck': -1, 'deckId': -1, 'column': -1, 'clipId': -1, 'retired': False}), (1, 1, 'Layer 2', {'deck': -1, 'deckId': -1, 'column': -1, 'clipId': -1, 'retired': False}), (2, 2, 'Layer 3', {'deck': -1, 'deckId': -1, 'column': -1, 'clipId': -1, 'retired': False})]
retiredDeckCount: 0 numDecks: 1
deck0 mirror layers: 3 [(0, -1), (1, -1), (2, -1)]
===== probe-crossfade.py (an OLD-format show POST: exercises the converter + triggers + crossfades)
...
PY 35 PASS / 0 FAIL
probe rc=0
===== after: GET /api/composition
shared layers: [(0, 0, 'L1', 0, 0)]
decks: [('A', 0, 1)] retired: 0
app running after quit: no
audio-dna windows 0, Output-named 0
21:04:02 lock released
UserNotificationCenter windows 0
21:04:18
smoke exit=0
```
The unchanged probe-crossfade.py (an OLD-format show POSTed -> the converter; triggers; crossfades; pixel decode):
`PY 35 PASS / 0 FAIL`, rc 0. LOOKED at live/smoke1/xf/a_both_effected_mid04.png: a mid-dissolve of the two effected
pictures, both visible, no black frame. Output-named windows 0; UserNotificationCenter windows 0 at 21:04:18 (16 s
after the quit). An Audio-DNA pid 69891 was running at 21:04:25 -- NOT this lane's (ours was 81771, quit at 21:04:02;
the lock had passed on): untouched.

## S2b RESULT
S2b DONE: the sanctioned non-building window is CLOSED -- the app and every test target build (B1 rc 0), ctest
`100% tests passed, 0 tests failed out of 1146` (B2: 1117 + 47 - 18), B3 TSAN 5 / 5 with the new R-bf9b case and 0
TSan warnings, B4e (one allow-listed file) and B4f (pinned structure writers) in ctest, the B4d / H2 smoke lint in
ctest, mutation smokes MS1-MS6 each fail their named tests (MS7 is S3's). Three S2a src bugs found by the new tests and
fixed RED-first (deckIsPlaying after a Cut, RoutineSlice v2 checkpoint, RemoveLayerCmd undo after a refusal). The
4.B omissions in "S2b deviations 3" need Harmony's ruling (applied per the plan body, revertible per hunk).

## Resume point (for the S2c builder)
S2b is complete at bb5d84e (+ this report commit). Next: S2c = ruling-bf9b amendment 10 ONLY (one separable commit):
onDeckSwitched's body becomes exactly `handleDeckSwitch(deckIdx);` (MainComponent.cpp, the `deckView_->onDeckSwitched
= [this](int deckIdx) {` lambda ~:1375); SwitchDeckCmd and its cases are deleted (DeckCommands.h class SwitchDeckCmd
~:980; test_undo_commands "SwitchDeckCmd: switch + undo ..." / "SwitchDeckCmd: double-switch redo chain ..." retired,
the SwitchDeckCmd lines inside "Deck commands no-op on stale coordinates (never crash)" [undo][deck][resolve] deleted
(the case stays), "SwitchDeckCmd: leaves a queued trigger untouched on execute, undo and redo (bf9b)" retired).
ALSO re-point (S2b deviation 8): test_show_model T5 (it switches through SwitchDeckCmd) and T1's "the model-level
shown-deck walk 0 -> 1 -> 0 and SwitchDeckCmd ..." SECTION, and test_render_thread_lint's B4d / H2 site list (drop the
"class SwitchDeckCmd" / "void apply(int index)" site); re-run MS1 / MS4 against the remaining switch path
(DeckView::showDeck -- T1's headless DeckView walk is the net there) and record. B4a gains zero `SwitchDeckCmd`; B4g =
onDeckSwitched's body is exactly `handleDeckSwitch(deckIdx);` (add it to the B4d / H2 lint case or a sibling).
Tools left in the scratchpad: bf9b-S2b/build_all.sh (target build + error count), errs.sh (full error list of one
test TU), red_lint.sh / red_show.sh (RED vs an old sha's src), mutants.py + mut/tree + mut/build (a configured copy:
rsync the worktree's src / tests / CMakeLists.txt into mut/tree before reuse). Arms unchanged: STAGE_P =
scratchpad/bf9b-S0/apps/stagep-head.app, C0 = apps/base-c0.app. Do not rebase until Harmony says (main has hyg +
mkvidx; ui merges next).
INBOX-RECHECK: none

## Notes for .harmony/notebook.md (Harmony appends) -- S2b
- bf9b: a Cut (transitionSpeed <= 0 -- the Layer default -1) or a clear leaves the tuple's `previous` naming the old
  clip at crossfadeProgress 1; "is this deck playing" must ignore a previous ref unless progress < 1. | discovered:
  src/model/Composition.h deckIsPlaying
- bf9b: PerfState v2 keeps the shared layers' settings in `layers`; any checkpoint reader (Program preamble,
  RoutineSlice) must read v2 `layers` first and only fall back to decks[deck].layers for a v1 take. | discovered:
  src/recording/RoutineSlice.cpp checkpointLayerSettings
- tests: UndoService.cpp links headless (its Renderer calls are inline) -- a unit test can drive withDeckDetached's
  reap + onDecksReaped without a renderer. | discovered: tests/CMakeLists.txt test_show_model
- tests: a mutation smoke can run on an rsync'd COPY of src / tests / cmake / resources / CMakeLists.txt configured in
  its own build dir (~1.5 min for test_show_model + test_layer_runtime) -- the deliverable is never edited. |
  discovered: scratchpad bf9b-S2b/mutants.py

## S2c (ruling-bf9b amendment 10: Undo skips deck switches -- ONE separable commit) -- builder started 21:08
STATUS(S2c): DONE
### S2c progress log (appended per item)
- 21:08 read the lane report, plan-bf9b (incl. HARMONY ADOPTION) and ruling-bf9b in full. Branch lane/bf9b at 5532074,
  worktree clean. Disk 289 GiB free.
- 21:10 RED first (tests before src): NEW lint case "bf9b B4g: a deck switch is never an Undo step -- the tab click is
  exactly handleDeckSwitch(deckIdx);" [lint][bf9b] in tests/test_render_thread_lint.cpp (onDeckSwitched's body ==
  `{handleDeckSwitch(deckIdx);}` whitespace-stripped; handleDeckSwitch's body names no pushCommands / undoManager_ /
  undoService_ / `Cmd>`; zero `\bSwitchDeckCmd\b` on src/ code lines = B4a's S2c clause). The B4d / H2 case drops its
  SwitchDeckCmd::apply site. RED = the current lint compiled against 5532074's src (scratchpad bf9b-S2c/red_lint.sh
  5532074), VERBATIM: `test_render_thread_lint.cpp:332: FAILED: CHECK( tab == "{handleDeckSwitch(deckIdx);}" )` and
  `test_render_thread_lint.cpp:359: FAILED: CHECK( hits.empty() )` with `SwitchDeckCmd in src/ code lines:
  core/DeckCommands.h:980 core/DeckCommands.h:983 MainComponent.cpp:1389`; `test cases: 1 | 0 passed | 1 failed`. (The
  handleDeckSwitch no-push check passes on 5532074 too: it was already true -- a guard.)
- 21:11 src: onDeckSwitched's body = `handleDeckSwitch(deckIdx);` (the comment moved above the lambda); SwitchDeckCmd
  deleted (DeckCommands.h) with its now-orphaned DeckActivateHook alias and MainComponent::makeDeckActivateHook (.h
  decl + .cpp def; SwitchDeckCmd was their only user); comments that named SwitchDeckCmd / the switch's undo reworded
  (DeckCommands.h step-6 banner, TriggerCommands.h:29, UndoService.h:20-21, UndoService.cpp:22-25, MainComponent.cpp
  handleDeckSwitch capture comment, MainComponent.h makeCompositionResolver comment).
- tests: test_undo_commands -- "SwitchDeckCmd: switch + undo restores active index, redo re-applies" and "SwitchDeckCmd:
  double-switch redo chain restores each active index" RETIRED (amendment 10: :1821 / :1849 at the ruling's base),
  "SwitchDeckCmd: leaves a queued trigger untouched on execute, undo and redo (bf9b)" RETIRED (:2840), the SwitchDeckCmd
  lines in "Deck commands no-op on stale coordinates (never crash)" deleted (the case stays; its banner "all three" ->
  "the deck commands"). test_show_model -- T1's SECTION "the model-level shown-deck walk 0 -> 1 -> 0 and SwitchDeckCmd
  execute / undo / redo" -> "the model-level shown-deck walk 0 -> 1 -> 0" (the index walk kept; T1's DeckView::showDeck
  walk is the code-driving net); T5's switch = `c.activeDeckIndex = 1; dv.showDeck();` on a headless DeckView (what
  handleDeckSwitch does minus the renderer fence token). Same expectations everywhere.
- 21:11:19-21:11:42 `cmake --build build-lane -j3 -- -k` rc=0, 0 error lines (bf9b-S2c/build1.log; 9 objects incl.
  MainComponent.cpp / UndoService.cpp / the 5 test TUs that include DeckCommands.h). Warnings in the touched files: 8
  lines, every one on a code line present verbatim at 5532074 (MainComponent.cpp 1492 / 2280 / 4732 / 4737 / 6373,
  MainComponent.h:97 keyStateChanged) -- no new warning.
- GREEN: test_render_thread_lint `All tests passed (995 assertions in 6 test cases)` (B4g alone `(325 assertions in 1
  test case)`); test_show_model `All tests passed (16812 assertions in 28 test cases)` (T5 `(8 / 1)`, T1 every
  deck-switch `(54 / 1)`); test_undo_commands `All tests passed (554 assertions in 79 test cases)` (82 - 3 retired).

### MS1 / MS4 re-run against the remaining switch path (DeckView::showDeck) -- 21:13:22-21:13:31
scratchpad bf9b-S2c/mut.sh + mutants.py (log mutants2.log): S2b's mut/tree rsync'd from the worktree first (`diff -rq`
src / tests: identical), each mutant = an edit of the COPY's src/ui/DeckView.cpp inserted at the top of
DeckView::showDeck (after `if (!deck) return;`), built by a normal cmake build in mut/build (target test_show_model), the
named tests run, the copy restored. The worktree is never edited (`git diff -- src/ui/DeckView.cpp` 0 lines; copy src ==
worktree src after; copy file sha256 prefix f540c3a2aaad04da before == after; the restored copy rebuilds rc 0 and
passes `All tests passed (16812 assertions in 28 test cases)`).
| mutant | edit (copy, DeckView::showDeck) | named tests | result (raw) |
|---|---|---|---|
| MS1 the switch cancels the leaving deck's queued triggers | every layer's queue into a deck other than the shown one is cleared (updateRuntime) | T5, T1 | `MS1 [T5*] rc=42: test cases: 1 \| 1 failed` (`test_show_model.cpp:489: FAILED: CHECK( c.layers[0].runtime() == queued )`); `MS1 [T1 every deck-switch*] rc=42: test cases:  1 \|  0 passed \| 1 failed` (`:381 CHECK( fingerprint(c, mgr, eng) == f0 )`, the DeckView walk) |
| MS4 the switch completes every fade | crossfadeProgress = 1 on every layer (updateRuntime) | T1 | `MS4 [T1 every deck-switch*] rc=42: test cases:  1 \|  0 passed \| 1 failed` (`:381`, the DeckView walk) |
Both mutations made their named tests FAIL on the remaining path (no STOP). (S2b's MS1 / MS4 rows mutated
SwitchDeckCmd::apply, which no longer exists.)

### B2 / B3 / B4 at the S2c tree (9cce864's content)
- B2 `ctest --test-dir build-lane -j3 --output-on-failure` 21:13:40-21:14:13 (bf9b-S2c/ctest1.log), VERBATIM:
  `100% tests passed, 0 tests failed out of 1144`. Bookkeeping vs S2b's `ctest -N` list (names diffed, comm):
  1146 - 3 retired + 1 added = 1144 == `Total Tests: 1144`. RETIRED: "SwitchDeckCmd: double-switch redo chain restores
  each active index", "SwitchDeckCmd: leaves a queued trigger untouched on execute, undo and redo (bf9b)", "SwitchDeckCmd:
  switch + undo restores active index, redo re-applies". ADDED: "bf9b B4g: a deck switch is never an Undo step -- the
  tab click is exactly handleDeckSwitch(deckIdx);". vs STAGE_P_HEAD: 1117 + 48 added - 21 retired = 1144.
- B3 `.harmony/probe-tsan-unit.sh` (bf9b-S2c/tsan.log; test_layer_runtime_race recompiled: it includes DeckCommands.h),
  exit 0, every non-compiler line VERBATIM:
```
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 21:14:34
probe-tsan-unit: ctest -L tsan finds 5 [tsan] cases (expected 5)
probe-tsan-unit: ctest -L tsan 2026-10-02 21:14:39
Test project /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/build-tsan
1/5 Test #111: R1 message-thread triggers vs render clock / autopilot on one deck ..........   Passed    0.30 sec
2/5 Test #112: R2 clip runtime fields: trigger writes vs render transport write-back .......   Passed    0.32 sec
3/5 Test #113: R4 tuple consistency and no lost fade under a paced trigger storm ...........   Passed    0.27 sec
4/5 Test #114: R-bf9b fenced box and stack edits vs the GL resolve of refs into any deck ...   Passed    0.30 sec
5/5 Test #115: R3 manual scalar writes vs eff() reads ......................................   Passed    0.26 sec
100% tests passed, 0 tests failed out of 5
tsan    =   1.44 sec*proc (5 tests)
Total Test time (real) =   1.45 sec
probe-tsan-unit: ctest rc=0 2026-10-02 21:14:41
```
  "WARNING: ThreadSanitizer" count: 0.
- B4 (code lines, `//` stripped; bf9b-S2c/b4.py): B4a hits = TopBar.cpp:197 / :204 + Composition.h:144 / :232
  (`globalTransitionSpeed`, S3.3's half, unchanged since S2a) + ShowMigration.h:166 / :168 (the allowed literals);
  ZERO `SwitchDeckCmd` (the S2c clause). B4b 0 hits. B4d (smoke, ctest, now 3 sites: handleDeckSwitch, onDeckSwitched,
  DeckView::showDeck) PASS. B4e / B4f PASS (ctest). B4g PASS (ctest, the new case).
- B4f pins unchanged (counts); the audited sites' LINES moved: DeckCommands.h now 75, 126, 153, 251, 494, 500, 510, 568,
  587, 638, 687, 696, 717, 777, 778, 806, 808, 824, 826, 907, 935, 937 (S2b's table -8 from :695 on); MainComponent.cpp
  801, 888, 987, 1004, 1116, 1142, 1194, 1213, 1283, 1285, 1359, 1361, 1570, 2931, 3780, 5219, 5296, 5448, 6608, 6609,
  6692, 6693, 6802, 6826, 6866 (S2b's table -14 from :1375, -23 from :5113). Same sites, same fences.
- Binary: `strings` of the S2c app (build-lane, 21:11:35) has 0 "Switch Deck" (the undo label); main's pre-change app 1,
  the STAGE_P arm (bf9b-S0/apps/stagep-head.app) 1.
- No live run in S2c: a deck-tab click has no non-synthetic driver (rig: no synthetic input), and REST switch_deck
  (handleDeckSwitch) pushed no Undo step on any arm, so no live row can tell the arms apart; B4g (the tab's body IS
  handleDeckSwitch) is the gate the ruling names, and K1 / K8 (S4) drive handleDeckSwitch live. No lock taken, no app
  launched.

### S2c commit
| sha | item | build |
|---|---|---|
| 9cce864 | S2c amendment 10 -- ONE separable commit: onDeckSwitched = handleDeckSwitch(deckIdx); SwitchDeckCmd + DeckActivateHook + makeDeckActivateHook deleted; 3 cases retired, 1 case trimmed, T1 / T5 re-pointed, B4d / H2 site list, NEW B4g | app + every test target rc 0 (21:11:42, the committed content; the worktree differs only in this report) |

### S2c deviations / decisions (for the reviewer and Harmony)
1. Orphans deleted in the same commit: `using DeckActivateHook` (DeckCommands.h) and MainComponent::makeDeckActivateHook
   (.h + .cpp) -- SwitchDeckCmd was their only user (grep src + tests). A Q4 "keep" revert restores them with it.
2. B4g's case also asserts (a) handleDeckSwitch's body names no pushCommands / undoManager_ / undoService_ / `Cmd>` (true
   before S2c too -- a guard on "never an Undo step from any entry") and (b) zero `SwitchDeckCmd` on src/ code lines
   (B4a's S2c clause as a ctest). One case, so the B2 count moves by +1.
3. Docs ("Docs say it", amendment 10) are NOT in 9cce864: amendment 23 schedules "Undo skips deck switches (10)" for
   performance-controls.md in S4's docs pass, and that paragraph region was just edited on main by ui (e5d81c2: deck
   rename in place), which bf9b rebases over. No doc today says a tab click is an Undo step (grep docs/claude, CLAUDE.md,
   APP-INVENTORY: 0 hits). S4 must add the line as ITS OWN hunk so a Q4 "keep" revert of 9cce864 + that hunk stays
   clean.
4. A consequence to name (inferred from the code, not run live): the Remove Deck "Undo Remove" button is hidden by
   pushCommands on any LATER command; a tab click used to push "Switch Deck" and so hid it -- now a tab click after a
   removal leaves the 10-s button up and working (the removal is still the top of the stack).
5. The S2b 4.B omissions (S2b deviations 3) still await Harmony's ruling; S2c touched none of those hunks.

## S2c RESULT
S2c DONE at 9cce864 (one separable commit; builds alone: app + every test target rc 0). ctest `100% tests passed, 0
tests failed out of 1144` (1146 - 3 + 1); TSAN 5 / 5, 0 warnings; B4g RED on 5532074's src (2 checks) and GREEN;
B4a zero SwitchDeckCmd; MS1 (T5 + T1) and MS4 (T1) bite on the remaining switch path DeckView::showDeck.

## Resume point (for the S3 builder)
S2c is complete at 9cce864 (+ this report commit). Next: S3 = S3.1 badge / tab dot / badge click (amendment 16(a)-(c));
S3.2 grid (16(e)); S3.3 TopBar fade removal -- its own commit (18; then B4a's globalTransitionSpeed hits at TopBar.cpp /
Composition.h go); S3.4 load-notice label (9(d)) + undo hint (16(d)); B7 headless machine checks (M-a..M-f,
tests/test_layer_strip_source_deck.cpp) + live captures for the critics; MS7 (the strip badge shows the shown deck ->
the S3 badge case). The badge click is handleDeckSwitch(findDeckIndexById(id)) -- never an Undo step (S2c). S4's docs
must add "a deck switch is never an Undo step" as its own hunk (S2c deviation 3). Tools: scratchpad bf9b-S2c/mut.sh +
mutants.py (rsync then mutate a copy; edit the MUTANTS list), red_lint.sh <sha> (the current lint vs an old src),
b4.py (B4a / B4b scan). Arms unchanged: STAGE_P = scratchpad/bf9b-S0/apps/stagep-head.app, C0 = apps/base-c0.app. Do
not rebase until Harmony says (main has hyg + mkvidx + ui now).
INBOX-RECHECK: none

## Notes for .harmony/notebook.md (Harmony appends) -- S2c
- bf9b: a deck switch is never an Undo step -- every switch entry (tab click, REST, OSC, bindings, replay, the S3 badge
  click) is MainComponent::handleDeckSwitch, which pushes no command; the lint "bf9b B4g" pins the tab body. |
  discovered: tests/test_render_thread_lint.cpp B4g
- tests: a headless DeckView (ScopedJuceInitialiser_GUI + setComposition + showDeck) is the model-level stand-in for
  handleDeckSwitch (index + showDeck; only the renderer fence token is missing) -- mutate DeckView::showDeck to give a
  switch test teeth. | discovered: tests/test_show_model.cpp T5

## S3 (ruling-bf9b S3 row: 16(a)-(c) badge / tab dot / badge click; 16(e) grid; 18 TopBar fade; 9(d) load notice + 16(d) undo hint; B7 machine checks + live captures; MS7) -- builder started 21:17
STATUS(S3): DONE
### S3 progress log (appended per item)
- 21:17 read the lane report, plan-bf9b (incl. HARMONY ADOPTION) and ruling-bf9b in full. Branch lane/bf9b at a4488b6,
  worktree clean. Disk 289 GiB free.
- 21:24-21:28 S3.1 (06e26bd): LayerStrip source-deck badge (tab number / "x"; dim = shown deck; opaque 0xff111111;
  text + 6 px wide; thumbnail >= 40 px only; bottom-left, clear of the band rows; refresh() + strip timer,
  compare-before-set, badge-rect repaint; click -> onSourceDeckClicked, never selects; tooltip), DeckView tab dot
  (deckIsPlaying; syncTabDots in refresh() + MainComponent's 30 Hz tick), MainComponent badge click =
  `handleDeckSwitch(composition_.findDeckIndexById(deckId));`. NEW tests/test_layer_strip_source_deck.cpp (6 cases).
  RED: compile vs a4488b6's src (scratchpad bf9b-S3/red_strip.sh, the target's own flags): rc 1, 70 errors, first
  `tests/test_layer_strip_source_deck.cpp:179:18: error: no member named 'getSourceBadge' in 'LayerStrip'`; vs
  STAGE_P_HEAD 3dac692: rc 1, 118 errors (`ShowFixture.h:19:14: error: no member named 'getNumLayers' in 'Composition'`).
  GREEN: full build rc 0 (0 errors; warnings only on pre-existing lines), `100% tests passed, 0 tests failed out of 1150`.
  Contrast measured by the test: dim 4.39917:1, normal 14.3044:1.
- 21:28-21:29 S3.2 (f39ca40): column header strict (lit only on the firing deck; a deck-less column lights none).
  RED behavioural (header accessor + the S3.1 condition): `test_layer_strip_source_deck.cpp:470: FAILED: CHECK(
  lit().empty() )` / `test cases: 3 | 2 passed | 1 failed`. GREEN: full build rc 0, `100% tests passed, 0 tests failed
  out of 1153`, `Total Test time (real) =  32.28 sec`.
- 21:30-21:33 S3.3 (5cdf218, its own commit; builds alone): TopBar "Fade:" label + slider + layout +
  fadeSliderBoundsForTest deleted; Composition::globalTransitionSpeed deleted; test_master_signal_link re-anchored to
  the Quantize selector (case 1 renamed) + NEW "bf9b S3.3: the TopBar has no deck Fade control". RED behavioural (seam
  first): `:384: FAILED: CHECK_FALSE( labels.contains("Fade:") )`, `:385: FAILED: CHECK( sliders == 3 )` `4 == 3`,
  `test cases: 15 | 14 passed | 1 failed`. GREEN: full build rc 0, `100% tests passed, 0 tests failed out of 1154`.
  B4a (b4.py): only ShowMigration.h:166 / :168 (the allowed literals) -- the TopBar.cpp / Composition.h hits are gone.
  strings(app) "Fade:" 0 (main's pre-change app 1).
- 21:34-21:37 S3.4 (see the S3 commits table): loadNotice_ (yellow, row-1 right slot beside audioDeviceNotice_;
  LoadNotice::forLoad(migrationNote, routineLoadNote); tooltip = details; cleared by save / load / New / click; never
  focus), the 7(a) refusal moved from the file label to it, /api/debug/ui_text "load_notice", the Remove Deck undo hint
  names the layers still playing (DeckTabRow::undoRemoveHint). RED vs 5cdf218: `fatal error: 'ui/LoadNotice.h' file
  not found`. GREEN: full build rc 0, `100% tests passed, 0 tests failed out of 1156`.

### MS7 (ruling-bf9b amendment 13) -- 21:38:37-21:38:48, scratchpad bf9b-S3/mut.sh + mutants.py -> mutants.log
On S2b's mut/tree COPY (rsync'd from the worktree: `diff -rq` src / tests identical), built by a normal cmake build in
mut/build (reconfigured for the new target); the worktree is never edited (`git diff -- src` 0 lines).
| mutant | edit (copy, LayerStrip::sourceBadgeOf) | named test | result (raw) |
|---|---|---|---|
| MS7 the strip badge shows the shown deck instead of the ref's deck | `b.tab = show->activeDeckIndex + 1;` | the S3 badge case "S3.1 the strip badge names ..." | `MS7 [S3.1 the strip badge names*] rc=42: test cases:  1 \|  0 passed \|  1 failed` (`:181: FAILED: CHECK( s->getSourceBadge().text() == text )` `1 == 2`) |
| MS7b (extra) the badge click goes to the shown deck | `b.deckId = show->getActiveDeck()->id;` | "S3.1 a badge click ..." | `MS7b [S3.1 a badge click*] rc=42: test cases:  1 \|  0 passed \| 1 failed` (`:349` `{ 0 } == { 101 }`) |
Copy restored: sha256 prefix fa66ad03468ddb9f before == after; the restored copy rebuilds rc 0 and passes `All tests
passed (252 assertions in 11 test cases)`. TOOLING FINDING: the first run's "restored" check FAILED (MS7b's case) on a
byte-identical copy -- make compares mtimes at 1-s resolution, so a restore written in the same second as the mutant's
build left the mutant object "up to date". mutants.py now stamps every mutated / restored copy 2-4 s in the future
(re-run above is clean). The S2b / S2c mutant scripts restore the same way (their restored-copy checks passed, so they
were not bitten, but a same-second restore could hide a stale object).

### S3 commits (each builds the app + every test target alone: full `cmake --build build-lane -j3 -- -k` rc 0 + full ctest on that exact tree before the commit)
| sha | item | build / ctest |
|---|---|---|
| 06e26bd | S3.1 badge / tab dot / badge click (16(a)-(c)) + NEW tests/test_layer_strip_source_deck.cpp (6 cases) | rc 0; `100% tests passed, 0 tests failed out of 1150` (21:28:05) |
| f39ca40 | S3.2 grid: column header only on the firing deck (16(e)) + 3 cases (M-a, header, M-c) | rc 0; `... out of 1153` (21:29:25) |
| 5cdf218 | S3.3 TopBar Fade section + Composition::globalTransitionSpeed removed (18), its own commit; test_master_signal_link re-anchored + 1 case | rc 0; `... out of 1154` (21:33:12) |
| e1cd314 | S3.4 load notice (9(d)) + Remove Deck undo hint (16(d)) + 2 cases | rc 0; `... out of 1156` (21:36:39) |

### B2 / B3 / B4 at the S3 head e1cd314
- B2 `ctest --test-dir build-lane -j3 --output-on-failure` (scratch ctest-s34.log), VERBATIM: `100% tests passed, 0 tests
  failed out of 1156` / `Total Test time (real) =  32.81 sec`. Bookkeeping vs S2c (names diffed, `ctest -N`): 1144 + 13
  added - 1 retired = 1156. ADDED: the 11 test_layer_strip_source_deck cases (S3.1 x6, S3.2 x3, S3.4 x2), "bf9b S3.3:
  the TopBar has no deck Fade control", "layout: the Signal fader sits to the right of Quantize with no overlap on
  Master" (RENAMED from the one RETIRED: "... to the right of Fade ..."; 4.B row test_master_signal_link :189). vs
  STAGE_P_HEAD: 1117 + 61 - 22 = 1156.
- B3 `.harmony/probe-tsan-unit.sh` exit 0 (21:46:07-21:46:19; scratch bf9b-S3/tsan-s3.log), VERBATIM:
```
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 21:46:07
probe-tsan-unit: ctest -L tsan finds 5 [tsan] cases (expected 5)
probe-tsan-unit: ctest -L tsan 2026-10-02 21:46:17
1/5 Test #111: R1 message-thread triggers vs render clock / autopilot on one deck ..........   Passed    0.29 sec
2/5 Test #112: R2 clip runtime fields: trigger writes vs render transport write-back .......   Passed    0.27 sec
3/5 Test #113: R4 tuple consistency and no lost fade under a paced trigger storm ...........   Passed    0.26 sec
4/5 Test #114: R-bf9b fenced box and stack edits vs the GL resolve of refs into any deck ...   Passed    0.28 sec
5/5 Test #115: R3 manual scalar writes vs eff() reads ......................................   Passed    0.24 sec
100% tests passed, 0 tests failed out of 5
tsan    =   1.34 sec*proc (5 tests)
Total Test time (real) =   1.35 sec
probe-tsan-unit: ctest rc=0 2026-10-02 21:46:19
```
  "WARNING: ThreadSanitizer" count 0. (Run via S2c's tsan.sh, which writes S2c's tsan.log -- that file now holds THIS
  run; S2c's own run is quoted verbatim above in the S2c section.)
- B4 (b4.py, code lines): B4a = ShowMigration.h:166 / :168 only (the allowed literals) -- the TopBar.cpp / Composition.h
  globalTransitionSpeed hits are GONE; zero SwitchDeckCmd. B4b 0. B4d (smoke) / B4e / B4f / B4g: PASS in ctest (no new
  structure writer: B4f pins unchanged).
- Warnings: every warning line of the S3 builds sits on a code line that existed before S3 (the S2c list shifted by the
  added lines: MainComponent.cpp 1505 / 2296 / 4794 / 4799 / 6435, LayerStrip.cpp 618 / 707 / 708 / 714 / 823) -- no
  new warning.
- strings(build-lane app) "Fade:" 0; main's pre-change app 1.

### B7 VISUAL WORK GATE -- MACHINE checks (headless, ctest, tests/test_layer_strip_source_deck.cpp)
| check | case | GREEN |
|---|---|---|
| M-a lit cells == model refs into the shown deck (same / other / removed deck) | "S3.2 the grid lights a cell iff ..." | PASS |
| M-b badge text == 1 + findDeckIndexById(ref.deckId), "x" retired, none clear; dim iff shown deck | "S3.1 the strip badge names ..." | PASS (MS7 makes it FAIL) |
| M-c strip-column snapshot byte-equal outside the badge rects across showDeck 0 -> 5 -> 0; same LayerStrip objects | "S3.2 a 0 -> 5 -> 0 showDeck walk ..." | PASS (and > 0 px changed inside the badges; byte-equal everywhere back on deck 0) |
| M-d badge inside the thumbnail, clear of the band rows, width >= text("20") + 6; folded row none; clip-name row never holds the deck name; 30-char clip name changes only the name row | "S3.1 a folded row ..." + "S3.1 the clip-name row never names the deck ..." | PASS |
| M-e tab dot iff a layer's active ref / running fade's previous ref names the deck (20 decks) | "S3.1 a deck tab shows a dot ..." | PASS |
| M-f WCAG contrast vs the opaque badge bg: dim 4.39917:1 (>= 3), normal 14.3044:1 (>= 7), normal > dim; painted pixels use those colours | "S3.1 badge contrast ..." | PASS |
| badge click: shows that deck, never selects; removed deck: nothing; tooltip | "S3.1 a badge click ..." | PASS (MS7b makes it FAIL) |
The badge CLICK has no non-synthetic live driver (rig: no synthetic input) -- unit-only, like the tab click (B4g).

### B7 LIVE captures (21:42:54-21:44:36, lock bf9b-S3; scratch bf9b-S3/b7.sh -> live/b7.log; window-only, largest on-screen window of OUR pid, 3456x2158)
Fixtures (scratch bf9b-S3/fix): PIL pictures "D<deck> C<col>" (mkimg.py; one colour per deck) and four composition files
written by the MODEL'S OWN toVar through a scratch tool built only in the mut/tree copy (fixgen.cpp, mut/build target
bf9b_fixgen; the worktree has no such file): b7-four-decks.json (4 decks x 3 layers; Layer 2 Ignore Column),
b7-twenty-decks.json (20 decks; tab 5 "Twenty Char Deck Nam"; deck 5 row 1 col 2 "a_thirty_character_clip_name_x";
Layer 3 folded), b7-old-one-deck.json and b7-old-two-decks.json (pre-bf9b shape; the second with a differing Deck 2 row 3,
"persistent": true on Deck 1 / Layer 2, a 1.2 s deck fade). Driven by REST on 7070 (load_composition, switch_deck,
trigger_clip, trigger_column, debug/remove_deck, debug/ui_text, composition), `--test-mode` (no analysis thread, no
mic), fresh connection per request. Captures in scratch bf9b-S3/live/b7/ (crops in live/b7/crops/):
| file | state | model (GET /api/composition, verbatim in b7.log) | builder's LOOK (decoded; the critic verdicts are Harmony's) |
|---|---|---|---|
| after-plain.png / before-plain.png | plain "deck 0 shown, layer 0 playing" (old 1-deck file on both arms) + state 4 TopBar | L0 deck 0 col 0, badge "1" dim | identical grids except AFTER's dim "1" badge on Layer 1's thumbnail and a dot on the Deck 1 tab (crops/plain-before-left-after-right.png) |
| after-plain.png vs before-plain.png TopBar | (4) TopBar without "Fade:" | -- | BEFORE "Quantize: Off | Fade: (slider) 0.30 | Master Signal ..."; AFTER "Quantize: Off | Master Signal ..." -- the right group unmoved, the free middle 136 px wider (crops/topbar-before-over-after.png) |
| after-s9-load-notice.png | (9) load notice after an old show | ui_text load_notice "Old show converted -- layer looks now come from the first deck (hover for details)"; file_label "Loaded: b7-old-two-decks"; one stderr line `old show converted: layer settings come from the first deck that has each row; Deck 2 row 3: settings dropped; 'persistent' ignored on: Deck 1 / Layer 2; deck fade 1.20 s dropped` (count 1) | yellow, right-aligned in row 1, whole sentence (crops/after-s9-row1.png) |
| after-s1.png | (1) deck 1 shown, Layer 2 playing deck 2's clip | L0 deck 0 col 0 badge 1 dim; L1 deck 1 (id 100) col 1 badge 2 normal | Layer 2's thumbnail "D2 C2" with a WHITE "2" bottom-left; no lit cell in Layer 2's row; Layer 1's "1" grey; dots on Deck 1 and Deck 2 tabs |
| after-s2.png | (2) deck 2 shown | L1 badge 2 dim, L0 badge 1 normal | cell D2C2 lit in Layer 2's row; Layer 2's "2" grey, Layer 1's "1" white; Deck 2 tab highlighted; both dots stay (crops/after-s2-grid.png) |
| after-s5.png | (5) column 3 fired on deck 1 while Layer 2 (Ignore Column) keeps deck 2's clip | L0 / L2 deck 0 col 2; L1 deck 1 col 1 | header "3" lit; Layer 1 / 3 cells D1C3 lit; Layer 2 keeps "D2 C2" badge "2" white, its row unlit (crops/after-s5-grid.png) |
| after-s5b-deck2.png | (5) the same, deck 2 shown | -- | header "3" NOT lit on deck 2; only D2C2 lit (Layer 2) (crops/after-s5b-deck2-grid.png) |
| after-s3.png | (3) deck 2 removed while Layer 2 plays its clip | numDecks 3, retiredDeckCount 1, L1 retired True, badge x | Layer 2 still "D2 C2" with an "x" badge, the X button above it; tabs Deck 1 / Deck 3 / Deck 4 (dot on Deck 1 only); flush right "Undo Remove "Deck 2" -- Layer 2 keeps playing its clip"; file label `Removed deck "Deck 2"` |
| after-s7-s8.png | (7) 20 decks, two dots, 30-char clip name, 20-char deck name; (8) a folded layer playing another deck's clip | L0 deck 4 col 1 badge 5; L2 deck 13 col 0 badge 14 | Layer 1 "D5 C2" badge "5" white, name row "a_thirty_chara..." (ellipsis); folded Layer 3 shows a 22-px "D14 C1" thumbnail and NO badge; dots on tab 5 ("Twenty Char" -- the long name fitted by JUCE) and tab "Deck 14" (crops/after-s7-s8-grid.png, after-s7-tabs-right.png) |
| after-s6-layer-tab.png / before-s6-layer-tab.png | (6) ruling-bf9 G7's Layer tab (ADNA_INSPECT_LAYER=0) | -- | both open the Layer tab of Layer 1 (crops/*-inspector.png) |
After the batch: every app quit by quit_app ("app running after quit: no" x4), Output-named windows 0 after each
session, UserNotificationCenter windows 0 at 21:44:36 (16 s after the last quit), lock released 21:44:20. ps at lock
time: three clang processes (another lane's build) -- no perf number was taken.

### S3 deviations / decisions (for the reviewer and Harmony)
1. Badge source: the badge is drawn only when the layer's playing clip EXISTS (Composition::playing(i).clip): a ref into
   an empty cell or a reaped deck shows none (nothing plays, so nothing "came from" a deck).
2. Tab dot = Composition::deckIsPlaying(id) (active ref, or the previous ref only while its fade runs -- the same rule
   that keeps a retired deck alive, S2b.1); M-e's "active or previous" is read with that definition.
3. Beyond the ruling's "set in refresh()": the badge is ALSO re-read on the strip's own 30 Hz timer and the dots on
   MainComponent's 30 Hz tick, both compare-before-set and repainting only what changed -- a GL-thread fire (autopilot,
   a queued trigger) or a fade completing never refreshes the grid, so refresh() alone would leave a stale badge / dot
   (Pitfall 41). Idle cost: one runtime() load per strip per tick + decks x layers loads per tick, no repaint.
4. Column header (16(e)): a column remembered WITHOUT a deck id now lights on no deck (it lit on every deck); no app
   caller passes none (handleColumnTrigger always passes the deck).
5. Amendment 7(a)'s refusal text moved from the file label (S2a's interim) to the load notice.
6. Both notices visible: the load notice sits LEFT of the audio-device notice in the same right-aligned slot (each
   takes at most half of what is left of row 1).
7. The undo hint names layers by their NAME ("Layer 2" by default), not by index.
8. The pure S3.4 helpers (src/ui/LoadNotice.h, DeckTabRow::undoRemoveHint) are tested in the new
   test_layer_strip_source_deck.cpp (the S3 file) rather than a new target.
9. MS7b (badge click to the shown deck) added beside MS7; both bite.
10. B7 fixtures come from the model's own toVar via a scratch tool in the mutation copy (never in the worktree), so the
    files are exactly what the app writes.

### found_not_fixed (S3)
- PRE-EXISTING: DeckView::rebuildGrid recreates the column triggers unlit and nothing re-applies the highlight until the
  next refresh() -- visible in after-s3.png (header "3", lit in state 5, is unlit after Remove Deck's rebuild while
  deck 1 stays shown). Not S3's change (rebuildGrid / removeDeck's refresh order are unchanged); a one-line
  `refresh()` after rebuild, or setActiveColumn's state applied in setupColumnTriggers, would fix it -- Harmony's call.
- Fixture quirk (tests/ShowFixture.h makeShow): deck 0 keeps initDefault's 12 cells per row while numColumns = 4, so
  b7-*.json shows 12 columns on deck 1 (empty 5-12) and 4 on the others. Test-only; S4's bf9b-check.json should set
  every deck's columns explicitly.
- A 1-deck show always shows a dim "1" badge on every playing layer and a dot on its only tab (by the rule) -- a
  critic may call it noise.
- On a narrow 20-deck tab, JUCE fits a long name ("Twenty Char") and the dot sits in the right indent just after it --
  for the critics.
- Tooling: the S2b / S2c mutants.py restore without bumping the mtime (see MS7 above).

## S3 RESULT
S3 DONE at e1cd314 (4 commits, each builds alone): badge / tab dot / badge click, the grid's column header, the TopBar
Fade removal (own commit; B4a's TopBar / Composition hits gone), the load notice + /api/debug/ui_text "load_notice" and
the Remove Deck undo hint. ctest `100% tests passed, 0 tests failed out of 1156`; TSAN 5 / 5, 0 warnings; MS7 (+ MS7b)
bite; B7 machine checks M-a..M-f PASS in ctest; B7 live captures for states (1)-(9) + BEFORE (4) / (6) / plain grid in
scratch bf9b-S3/live/b7 -- the critic panel is Harmony's (a "no" returns the lane to S3).

## Resume point (for the S4 builder)
S3 is complete at e1cd314 (+ this report commit). Next: S4 = probe-boxes (plan S4.1 rows + k1d_*, k2v_decode,
k4b_empty_cell, k8b_browse_fire, k9a/b/c_remove_playing, k10_fresh_and_resume; per-arm readers; K7 also reads
/api/debug/ui_text "load_notice"), docs (amendment 23 + plan section 6; "a deck switch is never an Undo step" as ITS OWN
hunk in performance-controls.md so a Q4 "keep" revert stays clean; the badge / tab dot / badge click / load notice /
undo hint; TopBar has no Fade; CLAUDE.md <= 25,000 B), the Boris test show bf9b-check.json + page (amendment 24; set
every deck's columns explicitly -- see found_not_fixed), lane report. B7 fixtures + scripts reusable: scratch
bf9b-S3/fix (pictures + 4 composition files), fixgen.cpp (build it in mut/tree: append its target to
mut/tree/tests/CMakeLists.txt as b7.sh's notes say; mut.sh's rsync --delete removes it), b7.sh, rest.py, comp.py,
wincap.py. Arms: STAGE_P = scratchpad/bf9b-S0/apps/stagep-head.app, C0 = apps/base-c0.app. The S2b 4.B omissions (S2b
deviations 3) still await Harmony's ruling. Do not rebase until Harmony says (main has hyg + mkvidx + ui).
INBOX-RECHECK: none

## Notes for .harmony/notebook.md (Harmony appends) -- S3
- bf9b: a widget that shows WHAT A LAYER PLAYS (strip badge, tab dot) must re-read on a timer, compare-before-set: a
  GL-thread fire (autopilot, a queued trigger) or a fade completing changes the tuple without any grid refresh. |
  discovered: src/ui/LayerStrip.cpp updateSourceBadge, src/ui/DeckView.cpp syncTabDots
- tests: mutation smokes on a copied tree must stamp the restored file's mtime into the future -- make compares at 1-s
  resolution, so a restore in the same second as the mutant's build leaves the mutant object "up to date" (a stale
  binary passes or fails for the wrong reason). | discovered: scratchpad bf9b-S3/mutants.py
- tests: live-capture fixtures are safest written by the model's own toVar (a scratch Catch2 tool in a copied tree):
  Layer::fromVar reads opacity / visible with no default, so a hand-written layer JSON missing a key loads invisible. |
  discovered: src/model/Layer.cpp fromVar
- DeckView::rebuildGrid leaves the column header unlit until the next refresh() (pre-existing). | discovered:
  src/ui/DeckView.cpp setupColumnTriggers

## S4 (ruling-bf9b S4 row: probe-boxes K rows with per-arm readers; docs (amendment 23 + plan section 6); the Boris test show bf9b-check.json + page (amendment 24); lane report complete) -- builder started 21:50
STATUS(S4): PENDING
### S4 progress log (appended per item)
- 21:50 read the lane report, plan-bf9b (incl. HARMONY ADOPTION) and ruling-bf9b in full. Branch lane/bf9b at 7a6fce1,
  worktree clean. Disk 292 GiB free.
- 21:55-22:03 S4.1 probe-boxes: explore session on the BF9B app (scratch bf9b-S4/explore.sh, 21:56:59-21:57:20, lock
  bf9b-S4) to check the probe's assumptions before writing it: thirds by layer transform (layerScale 1/3, positionX i,
  positionY 1 -> only the middle band lit), /api/state video_* fields, OSC /audiodna/deck/1 moves activeDeck, a clip
  opacity connection on the signal "Volume" follows inject_features rms LINEARLY (means 20.2 / 12.12 / 4.04 at rms 1 /
  0.5 / 0 -> deterministic, so K1d-iii is not a STOP), /api/perf/record works in --test-mode and writes an activeDeck
  lane. The explore take (bf9b-s4-explore.adna-take, ours) was deleted from ~/Documents/Audio-DNA/Takes.
- 22:03 S4.1a 729a76e: `git mv` probe-deck-clock.{sh,py,json} -> probe-boxes.* (rename-only commit: history kept).
- 22:06-22:17 S4.1b c8e8dd6: the rows (below). Both arms run in 3 batches each, every batch under the lock (bf9b-S4),
  launched and quit by the probe itself (quit_ours), test mode. Probe sha256 at the runs (== c8e8dd6's content):
  probe-boxes.sh 998dfd815571b36f.., .py 37093ca3f50508e6.., .json 82b4217d3f4b0596...

### S4.1 probe-boxes -- K rows RED (STAGE_P_HEAD app) / GREEN (BF9B app at e1cd314's src), raw lines
Arms: STAGE_P = scratchpad/bf9b-S0/apps/stagep-head.app (3dac692), BF9B = build-lane app (built 21:35 from e1cd314; the
S4 commits touch no src). Logs (scratch bf9b-S4/live/): dbg1-STAGE_P-220813.log, dbg2-STAGE_P-221212.log,
dbg3-STAGE_P-221417.log; dbg1-BF9B-220643.log, dbg2-BF9B-220949.log, dbg3-BF9B-221537.log. The BF9B arm's k7_old_take
replays the take the STAGE_P arm recorded (BOXES_OLD_TAKE = live/STAGE_P/boxes.5BVXCQ/k7-old-take.adna-take).
Summary lines VERBATIM: STAGE_P `PY 3 PASS / 9 FAIL / 0 BLOCKED (arm STAGE_P)`, `PY 7 PASS / 10 FAIL / 1 BLOCKED (arm
STAGE_P)`, `PY 5 PASS / 5 FAIL / 1 BLOCKED (arm STAGE_P)` -> `PROBE-BOXES RED` x3; BF9B `PY 14 PASS / 0 FAIL / 0 BLOCKED
(arm BF9B)`, `PY 24 PASS / 0 FAIL / 1 BLOCKED (arm BF9B)`, `PY 23 PASS / 0 FAIL / 1 BLOCKED (arm BF9B)` -> `PROBE-BOXES
GREEN` x3. Every batch: `PASS  no foreign render_frame traffic during the run`, `PASS  app terminated`; after every
batch `audio-dna windows 0, Output-named 0` and `UserNotificationCenter windows: 0` (16 s after the quit).
LOOKED at frames (scratch live/BF9B/look-k1.png): k1a ref "D0 C0", k1c mid-dissolve "D0 C0" -> "D0 C1", the k1t feedback
trails, the k1b ramp frame (olive = t ~ 6.5 s) -- the decoded numbers describe what is on the canvas.
| row | STAGE_P (RED arm) | BF9B (GREEN arm) |
|---|---|---|
| k1a REST switch_deck | FAIL d at +0 / +0.5 / +2 s = 63.84 (floor 1.50) | PASS d 0.0 / 0.0 / 0.0 |
| k1a OSC /audiodna/deck/1 | FAIL d 63.84 x3 | PASS d 0.0 x3 |
| k1a duplicate_deck 0 | PASS d 0.0 x3 (GUARD on STAGE_P: its Duplicate copies the playing column, and a static picture restarted looks the same -- see deviations) | PASS d 0.0 x3 |
| k1a load_deck | FAIL d 63.84 x3 | PASS d 0.0 x3 |
| k1a remove_deck (shown, unplayed deck) / undo | N/A (BF9B-only) | PASS / PASS d 0.0 x3 each |
| k1b_switch_video | FAIL t at +2 s = 0.00 (expected 6.47 +- 0.5); playhead PASS (0.1659; STAGE_P's off-screen DeckClock moved it) | PASS t 4.47 -> 6.45 (exp 6.47); playhead 0.1659 (exp 0.1668 +- 0.06) |
| k1c_switch_midfade | FAIL 0/10 on line (p -1.25, residual 41.83); p 1.0 -> 1.0; final d 82.40 (completion PASS: deck 1's layer reads complete) | PASS 11/11 on line (residual 0.13-0.26); p 0.414 -> 0.789; complete at 4.07 s after the fire; final d 0.00 |
| k1t_history_freeze | FAIL d 63.84 / 63.84 | PASS d 0.0 / 0.0 |
| k1t_history_feedback | FAIL d 80.76 / 80.76 | PASS d 0.0 / 0.0 |
| k1d_ia_speed_half | FAIL t(W) 0.00, delta 2.73; playhead PASS 0.0621 | PASS t 1.98 -> 2.73 (delta 0.75 in [0.25, 1.25]); playhead 0.0621 |
| k1d_ib_pingpong | FAIL t(W) 0.00, delta 9.41; playhead PASS -0.1247 | PASS t 10.96 -> 9.41 (delta -1.55 in [-2, -1]); playhead -0.1236 |
| k1d_ii_opacity_blend | VALID 45.75; FAIL d at W / W+0.5 / W+2 = 109.59 / 109.59 / 0.0 | VALID 45.75; PASS d 0.0 x3 |
| k1d_iii_connection | VALID 51.26; FAIL d(Cmax, Rmax) 63.84, d(Cmin, Rmin) 12.58 | VALID 51.26; PASS 0.00 / 0.00 |
| k2_nothing_unseen | VALID 2 advance; FAIL 1 unseen: (deck 1, row 1, col 0) +0.0849 in 1 s | VALID 2 advance; PASS 0 unseen |
| k2v_decode | N/A (BF9B-only) | VALID players 60; PASS awake [3 x10]; decoded 1795 -> 2244 = 449 in [225, 705]; nothing else moved |
| k3_autopilot | VALID (col 1 at beat 4); FAIL d at beats 4 / 8 / 12 = 82.4 / 102.39 / 0.0; FAIL 2 changes off screen (deck 0 row 0 activeClipColumn 0 -> 1 at beat 4, 1 -> 2 at beat 8) | VALID; PASS d 0.0 x3; PASS 0 changes |
| k4_ignore_column_across | FAIL layer 2 REST None, region d 83.08; others PASS (1, 1) | PASS layer 2 (0, 1) region d 0.00; others (1, 1) |
| k4b_empty_cell | N/A (BF9B-only) | PASS layer 0 {101, 1}; layer 1 empty, region d 0.00; layer 2 {100, 0} region d 0.00 |
| k5_queue_link_off | FAIL before / after the bar None / None (pending cancelled by the switch, L5); frame d 82.40 | PASS (0, 0) -> (0, 1) on the bar; frame d 0.00 |
| k5_queue_link_on | BLOCKED (no Link build, no driver) | BLOCKED (same) |
| k7_old_show | settings PASS (STAGE_P keeps per-deck); FAIL logLine count 0; FAIL ui_text has no load_notice; FAIL (new-format: no load_notice key); save BLOCKED | PASS settings [(0, 1.0, 0), (1, 0.8, 0)]; PASS exactly 1 logLine; PASS load_notice "Old show converted -- layer looks now come from the first deck (hover for details)"; PASS new-format: 0 lines, load_notice ""; save BLOCKED |
| k7_old_take | PASS old take (checkpoint0 without "layers", 3 lane points), PASS replay, PASS lane moves; FAIL capture max d 63.84 | PASS x4; captures max d 0.0 (16) |
| k8_twenty_decks | PASS 1.77 s; FAIL 19 switches d 63.84 | PASS 1.83 s; PASS 20 switches d 0.0, peak 0.7-6.3 ms |
| k8b_browse_fire | N/A (BF9B-only) | PASS left d 0.0 at all 20; right d 0.0 from the fire; layer 1 {107, 2}; peak 0.6-4.3 ms |
| k9a_remove_playing | N/A | PASS t 4.00 -> 5.98 over 2.01 s; numDecks 3 -> 2, retiredDeckCount 1, retired True |
| k9b_remove_midfade | N/A | PASS 10/10 on line; p 0.458 -> 0.834; complete 4.03 s; retired 1 while p < 1; duplicate reaps: video_players 2 -> 0 |
| k9c_remove_undo | N/A | PASS t 5.04 -> 7.06 over 2.00 s; deck 2 back at index 2 id 102, retiredDeckCount 0, layer 1 {102, 0} |
| k10_fresh_and_resume | N/A | PASS (i) t 0.14 at +0.25 s; (ii) t_r 4.28 -> t 4.47 at +0.20 s; playhead 0.35878 -> 0.35878 |

### K6 -- probe-deck-clock's rows (ruling-bf9 K6 dispositions, in probe-boxes.py's docstring)
| old row | disposition | new row / bar |
|---|---|---|
| d_fade_finishes | INVERTED | k1c_switch_midfade (K1c) |
| d_single_advance | RETIRED with DeckClock (S2a deleted it) | K1c's "p rises >= 0.15" |
| d_pending_trigger_still_cancelled | INVERTED | k5_queue_link_off / k5_queue_link_on (K5) |
| d_video_keeps_time, d_imageseq_keeps_time | INVERTED | k1b_switch_video + k2_nothing_unseen (+ k2v_decode, k10) |
| d_autopilot_keeps_time | INVERTED | k3_autopilot (K3) |
| d_return_hitch | RETIRED | its 50 ms bar moved to k8_twenty_decks / k8b_browse_fire |
New rows (ruling-bf9b amendment 14): K1d (4 sub-rows), K2v, K4b, K8b, K9 (a / b / c), K10.
