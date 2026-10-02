# LANE bf9b -- builder report (s-rta-1002b)

STATUS: DONE (S0 / Stage P complete and gated; S1 next)
Stage in progress: S0 DONE (Stage P = Persistent removed; ruling-bf9.md C0-C4, G0-G7 all PASS at STAGE_P_HEAD)
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
