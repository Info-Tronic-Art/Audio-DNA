# LANE REPORT mkvidx (s-rta-1002b) -- Stage A (C++) + Stage B (probe + docs)

STATUS: DONE
RESULT: Lane complete as ruled. c1 24f9da3 = items 1-3 + fixtures + 8 ctest cases (Stage A); c2 0126610 = item 4 (probe u13 + ab rules + selftest); c3 = item 5 docs (Pitfall 64, CLAUDE.md index line paid by a move, 23,976 B) + this report.
FACTS:
- Stage A (unchanged): A0 RED == the ruling's RED column; GREEN == the G3 GREEN column; ctest 1122 / 1122 (1114 + 8); TSan 0 reports on the three targets; mutants m1-m6 each fail >= 1 case (sections below).
- c2 0126610: `python3 .harmony/probe-vupload-ab.py --selftest` -> `SELFTEST PASS` (25 checks incl. the three u13 TSVs); the same u13 TSVs on HEAD~ (e3dc697) ab.py print 0 rule lines and exit 0 (RED: the frozen-B TSV is not caught); three scratch-copy rule mutants abm1-abm3 each turn the selftest FAIL.
- u7 / u8 refactor: HEAD~ vs lane probe-vupload.py run against one deterministic fake probe-video module print byte-identical output (71 lines, 9 u7 scenes + u8, `diff` empty).
- Live INFO smoke (lock 16:27:56-16:29:41, lane app build-lane, `probe-vupload.sh ... u13_container_reverse`): `PY 7 PASS / 0 FAIL`, `PROBE-VUPLOAD GREEN`; every scene uploads/s 29.4-30.6 (columns 120.2 / 120.6 pooled), late 0 except mp4 ping-pong turn 31; captures decoded (code 194 / 193 / 194 in bracket, 20 mono captures falling); 6 "Keyframe index:" lines = 1 (mkv end-Cues) + 4 (mkv column) + 1 (mkv turn), 0 for front-Cues / MP4 / HAP. After: no Audio-DNA, 0 Output-named windows, 0 UserNotificationCenter windows (kCGWindowListOptionAll) at +20 s.
- `ctest --test-dir build-lane -N` = `Total Tests: 1122` (delta +8 vs the base's 1114; Stage B touched no src / tests: `git diff --stat 24f9da3 HEAD -- src tests` empty).
- G9: `wc -c CLAUDE.md` = 23,976 <= 24,002; rendering.md has 0 "MKV / WebM hold a 1-entry index after open"; pitfalls.md has 0 "(MKV / WebM keep a 1-entry index after open"; Pitfall 64 + the CLAUDE.md "64." line (AM16 (c) text verbatim) present.
METHOD: Stage B: item 4 code per plan item 4 + AM14 (remux fixtures, helpers, u13, "_u13" bars, ab block, selftest), RED via the pre-change ab.py + scratch-copy mutants, an offline fake-pv equivalence run for the u7 / u8 refactor, fixtures checked with ffprobe + the ruling's idx4, then ONE INFO live smoke under the lock helper; item 5 docs per plan section 6 + AM16, edited by quoted text.
CONFIDENCE+VERIFY: High for the code and the selftest (re-run: `python3 .harmony/probe-vupload-ab.py --selftest`); the live rules are Harmony's G6 (5 x 2 interleaved) -- one smoke on B only proves the row runs and the lane app passes the B bars once. Re-check: `git rev-parse HEAD:.harmony/probe-vupload.json` = e770e5c7a0455402c9c4fda79f7de4c3c8cdfc97 at c2 (unchanged by c3).
UNKNOWNS-NOT-DONE: the A arm (main) was not launched (the adoption allows ONE INFO smoke); [CTRL-A] and every B-vs-A rule are unmeasured live until G6. G7 is Harmony's.
NUANCE: in the smoke the mkv / front-Cues reverse scenes decoded 0 frames in-window (all 300 frames resident: one player's share is 2 GiB), so [FREEZE] / [MKV] discriminate by uploads/s / late / captures, not decodes; the 60 s ping-pong turn showed MP4 late 31 (max gap 313 ms) vs MKV late 0 -- MP4's F1 hold, one launch, INFO.
HANDOFF-NEEDS: Harmony: G1-G9 after merge; G6 records the blob e770e5c before its first launch; VIDEO_FIXTURES=$B/fixtures holds the u13 fixtures (reusable, ~164 MB).
INBOX-RECHECK: none

Worktree /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/mkvidx, branch lane/mkvidx, base 9a832a0 (src / tests
byte-identical to fa9604d: `git diff --stat fa9604d 9a832a0 -- src tests` empty). Started 2026-10-02 15:34:02 EDT.
Scratch: $A = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/mkvidx-A

## Fixtures (AM5) -- generated twice with brew FFmpeg 8.0 ($A/genfx.sh), byte-identical, every size / sha256 prefix == AM5

| fixture | bytes | sha256[:16] |
|---|---|---|
| video_h264_allintra_64x64.mkv (FX4e) | 23485 | 2802512b4d266b2d |
| video_h264_allintra_cuesfront_64x64.mkv (FX4) | 23485 | f310619d1c3af1aa |
| video_h264_gop250_64x64.mkv (FX1) | 47251 | f3efd8c1ef9b8fb1 |
| video_h264_gop250_cuesfront_64x64.mkv (FX2) | 47251 | 2cc9c9f8ec94f3f3 |
| video_h264_gop30_64x64.mkv (FX5) | 16277 | 69d77cbff6420f5b |
| video_h264_opengop_64x64.mkv (FX8) | 14620 | e83125cd21d1ca3f |
| video_h264_scenecut_64x64.mkv (FX3) | 50486 | a9c17605b038c2a3 |
| video_hap_tb600_64x64.mov (FX6) | 100841 | 83cfa97a7a1066d3 |
| video_vp9_gop60_64x64.mp4 (FX7m) | 48924 | 11a157e45ecc9fc6 |
| video_vp9_gop60_64x64.webm (FX7) | 49568 | c1348e3b9ffbcab5 |

## A0 RED -- relink rig on fa9604d's src/media ($A/rig.sh red: `git archive fa9604d src`, the worktree's test TUs, main build's JUCE objects + Catch2 read-only, TEST_FIXTURES_DIR = the worktree's tests/fixtures)

test_gop_cache.cpp (P) -- raw compiler output:
```
tests/test_gop_cache.cpp:593:20: error: use of undeclared identifier 'keyIndexFrom'
tests/test_gop_cache.cpp:598:20: error: use of undeclared identifier 'keyIndexFrom'
tests/test_gop_cache.cpp:606:20: error: use of undeclared identifier 'keyIndexFrom'
tests/test_gop_cache.cpp:610:20: error: use of undeclared identifier 'keyIndexFrom'
tests/test_gop_cache.cpp:615:20: error: use of undeclared identifier 'keyIndexFrom'
tests/test_gop_cache.cpp:623:20: error: use of undeclared identifier 'keyIndexFrom'
6 errors generated.
```

test_gop_cache_store "mkvidx*" + test_video_decode_trace "threaded reverse on a Matroska file*" -- printed lines + Catch2 summaries, raw:
```
red-trace.txt:threaded reverse video_h264_gop30_64x64.mkv: shown 159 violations 0 hits 160 seeks 2 direction changes 7
red-trace.txt:threaded reverse video_h264_gop30_64x64.mkv: keyRels {0 } gop 90
red-trace.txt:test cases: 1 | 1 failed
red-trace.txt:assertions: 7 | 5 passed | 2 failed
red-store.txt:mkvidx T1 CAP 21 K 3 9 s video_h264_gop250_64x64.mp4              intra 0 late 0 shown 268 decodes 2186 seeks 26 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T1 CAP 21 K 3 9 s video_h264_gop250_cuesfront_64x64.mkv    intra 1 late 875 shown 1 decodes 3240 seeks 3240 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T1: a long-GOP Matroska file with its Cues at the FRONT is not intra-
red-store.txt:mkvidx T1 guard video_h264_allintra_cuesfront_64x64.mkv intra 1
red-store.txt:mkvidx T2 (a) FX1 open keyRels {0} gop 300 -> after the reverse start + 4 steps keyRels {0} gop 300 (seeks 1; decoded keys {0, 250})
red-store.txt:mkvidx T2: a Matroska file's keyframe model follows the demuxer's index -- the
red-store.txt:mkvidx T2 (b) FX1 9 s forward: seeks 0 keyRels {0} gop 300 late 0 shown 271 (0..270) steps != 1: 0
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_gop250_64x64.mp4              intra 0 late 0 shown 268 decodes 2186 seeks 26 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_gop250_64x64.mkv              intra 0 late 0 shown 268 decodes 2425 seeks 28 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 268 decodes 910 seeks 27 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_scenecut_64x64.mkv            intra 0 late 0 shown 268 decodes 2141 seeks 61 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_gop30_64x64.mp4               intra 0 late 0 shown 83 decodes 202 seeks 10 mismatches 0 keyRels {0, 30, 60} gop 30
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_gop30_64x64.mkv               intra 0 late 0 shown 83 decodes 197 seeks 10 mismatches 0 keyRels {0} gop 90
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_vp9_gop60_64x64.mp4                intra 0 late 0 shown 266 decodes 1044 seeks 27 mismatches 0 keyRels {0, 60, 120, 180, 240} gop 60
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_vp9_gop60_64x64.webm               intra 0 late 0 shown 266 decodes 2212 seeks 59 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_opengop_64x64.mp4             intra 0 late 0 shown 54 decodes 107 seeks 8 mismatches 0 keyRels {0, 10, 19, 30, 40, 50} gop 11
red-store.txt:mkvidx T2 (c) CAP 21 K 3 video_h264_opengop_64x64.mkv             intra 0 late 0 shown 54 decodes 108 seeks 7 mismatches 0 keyRels {0} gop 60
red-store.txt:mkvidx T3 CAP 21 K 3 video_hap_tb600_64x64.mov                intra 1 late 64 shown 38 decodes 834 seeks 834 mismatches 0 keyRels {0, 1, ..., 89} (90 keys) gop 1
red-store.txt:mkvidx T3: a run's seek aims at the frame's middle -- intra-only files on a
red-store.txt:mkvidx T3 CAP 21 K 3 video_h264_allintra_cuesfront_64x64.mkv  intra 1 late 4 shown 17 decodes 324 seeks 324 mismatches 0 keyRels {0, 1, ..., 29} (30 keys) gop 1
red-store.txt:mkvidx T3 CAP 21 K 3 video_h264_allintra_64x64.mkv            intra 1 late 4 shown 17 decodes 324 seeks 324 mismatches 0 keyRels {0, 1, 2, 3, 4, 5, 6, 7} gop 1
red-store.txt:mkvidx T3b CAP 21 K 3 9 s video_mpeg4_bf2_64x64.mp4                intra 0 late 4 shown 269 decodes 561 seeks 34 mismatches 0 keyRels {0, 28, 58, 88, 118} gop 30
red-store.txt:mkvidx T3b CAP 21 K 3 9 s video_h264_opengop_64x64.mp4             intra 0 late 8 shown 270 decodes 430 seeks 33 mismatches 0 keyRels {0, 10, 19, 30, 40, 50} gop 11
red-store.txt:mkvidx T3b CAP 21 K 3 9 s video_h264_vfr_64x64.mp4                 intra 0 late 8 shown 270 decodes 425 seeks 28 mismatches 0 keyRels {0, 10, 20, 30, 40, 50} gop 10
red-store.txt:mkvidx T3b CAP 21 K 3 9 s video_h264_vfrgap_64x64.mp4              intra 0 late 103 shown 144 decodes 327 seeks 22 mismatches 0 keyRels {0, 14, 24, 33, 42, 52, 62, 72, 80} gop 14
red-store.txt:mkvidx T4 (i) PingPong CAP 21 25 s video_h264_gop250_64x64.mp4              intra 0 late 5 shown 747 decodes 2634 seeks 27 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T4 (ii) flip CAP 21 8 s video_h264_gop250_64x64.mp4              intra 0 late 30 shown 229 decodes 1365 seeks 12 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T4 (i) PingPong CAP 21 25 s video_h264_gop250_64x64.mkv              intra 0 late 0 shown 749 decodes 2891 seeks 30 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T4 (ii) flip CAP 21 8 s video_h264_gop250_64x64.mkv              intra 0 late 14 shown 233 decodes 1592 seeks 14 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T4: a Matroska file ping-pongs and flips like the same stream in MP4
red-store.txt:mkvidx T4 (i) PingPong CAP 21 25 s video_h264_gop250_cuesfront_64x64.mkv    intra 1 late 1180 shown 452 decodes 4048 seeks 3596 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T4 (ii) flip CAP 21 8 s video_h264_gop250_cuesfront_64x64.mkv    intra 1 late 317 shown 110 decodes 1677 seeks 1441 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T4 (i) PingPong CAP 21 25 s video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 749 decodes 1413 seeks 28 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
red-store.txt:mkvidx T4 (ii) flip CAP 21 8 s video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 237 decodes 443 seeks 13 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
red-store.txt:mkvidx T4 (i) PingPong CAP 21 25 s video_h264_scenecut_64x64.mkv            intra 0 late 0 shown 749 decodes 2624 seeks 63 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T4 (ii) flip CAP 21 8 s video_h264_scenecut_64x64.mkv            intra 0 late 21 shown 231 decodes 1414 seeks 43 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_gop250_64x64.mp4              intra 0 late 0 shown 749 decodes 795 seeks 3 mismatches 0 keyRels {0, 250} gop 250
red-store.txt:mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_gop250_64x64.mkv              intra 0 late 0 shown 749 decodes 795 seeks 3 mismatches 0 keyRels {0} gop 300
red-store.txt:mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 749 decodes 668 seeks 4 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
red-store.txt:mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_scenecut_64x64.mkv            intra 0 late 0 shown 749 decodes 745 seeks 3 mismatches 0 keyRels {0} gop 300
red-store.txt:test cases:   6 |   2 passed |  4 failed
red-store.txt:assertions: 384 | 340 passed | 44 failed
```

per case on fa9604d: shape PASS, T1 FAIL, T2 FAIL, T3 FAIL, T3b PASS (a guard by construction), T4 FAIL, T2e FAIL -- exactly the ruling's A0 expectation.

## A1-A4 GREEN (items 1-3 + the ten fixtures + the seven cases) -- the same rig on the worktree's src ($A/rig.sh green)

```
threaded reverse video_h264_gop30_64x64.mkv: shown 159 violations 0 hits 160 seeks 2 direction changes 7
threaded reverse video_h264_gop30_64x64.mkv: keyRels {0 30 60 } gop 30
All tests passed (7 assertions in 1 test case)
All tests passed (16 assertions in 1 test case)
mkvidx T1 CAP 21 K 3 9 s video_h264_gop250_64x64.mp4              intra 0 late 0 shown 268 decodes 2186 seeks 26 mismatches 0 keyRels {0, 250} gop 250
mkvidx T1 CAP 21 K 3 9 s video_h264_gop250_cuesfront_64x64.mkv    intra 0 late 0 shown 268 decodes 2186 seeks 26 mismatches 0 keyRels {0, 250} gop 250
mkvidx T1 guard video_h264_allintra_cuesfront_64x64.mkv intra 1
mkvidx T2 (a) FX1 open keyRels {0} gop 300 -> after the reverse start + 4 steps keyRels {0, 250} gop 250 (seeks 1; decoded keys {0, 250})
mkvidx T2 (b) FX1 9 s forward: seeks 0 keyRels {0, 250} gop 250 late 0 shown 271 (0..270) steps != 1: 0
mkvidx T2 (c) CAP 21 K 3 video_h264_gop250_64x64.mp4              intra 0 late 0 shown 268 decodes 2186 seeks 26 mismatches 0 keyRels {0, 250} gop 250
mkvidx T2 (c) CAP 21 K 3 video_h264_gop250_64x64.mkv              intra 0 late 0 shown 268 decodes 2186 seeks 26 mismatches 0 keyRels {0, 250} gop 250
mkvidx T2 (c) CAP 21 K 3 video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 268 decodes 910 seeks 27 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
mkvidx T2 (c) CAP 21 K 3 video_h264_scenecut_64x64.mkv            intra 0 late 0 shown 268 decodes 910 seeks 27 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
mkvidx T2 (c) CAP 21 K 3 video_h264_gop30_64x64.mp4               intra 0 late 0 shown 83 decodes 202 seeks 10 mismatches 0 keyRels {0, 30, 60} gop 30
mkvidx T2 (c) CAP 21 K 3 video_h264_gop30_64x64.mkv               intra 0 late 0 shown 83 decodes 202 seeks 10 mismatches 0 keyRels {0, 30, 60} gop 30
mkvidx T2 (c) CAP 21 K 3 video_vp9_gop60_64x64.mp4                intra 0 late 0 shown 266 decodes 1044 seeks 27 mismatches 0 keyRels {0, 60, 120, 180, 240} gop 60
mkvidx T2 (c) CAP 21 K 3 video_vp9_gop60_64x64.webm               intra 0 late 0 shown 266 decodes 1044 seeks 27 mismatches 0 keyRels {0, 60, 120, 180, 240} gop 60
mkvidx T2 (c) CAP 21 K 3 video_h264_opengop_64x64.mp4             intra 0 late 0 shown 54 decodes 107 seeks 8 mismatches 0 keyRels {0, 10, 19, 30, 40, 50} gop 11
mkvidx T2 (c) CAP 21 K 3 video_h264_opengop_64x64.mkv             intra 0 late 0 shown 54 decodes 106 seeks 7 mismatches 0 keyRels {0, 10, 20, 30, 40, 50} gop 10
mkvidx T3 CAP 21 K 3 video_hap_tb600_64x64.mov                intra 1 late 0 shown 75 decodes 78 seeks 78 mismatches 0 keyRels {0, 1, ..., 89} (90 keys) gop 1
mkvidx T3 CAP 21 K 3 video_h264_allintra_cuesfront_64x64.mkv  intra 1 late 0 shown 27 decodes 30
mkvidx T3 CAP 21 K 3 video_h264_allintra_64x64.mkv            intra 1 late 0 shown 27 decodes 30 seeks 30 mismatches 0 keyRels {0, 1, ..., 29} (30 keys) gop 1
mkvidx T3b CAP 21 K 3 9 s video_mpeg4_bf2_64x64.mp4                intra 0 late 4 shown 269 decodes 561 seeks 34 mismatches 0 keyRels {0, 28, 58, 88, 118} gop 30
mkvidx T3b CAP 21 K 3 9 s video_h264_opengop_64x64.mp4             intra 0 late 8 shown 270 decodes 430 seeks 33 mismatches 0 keyRels {0, 10, 19, 30, 40, 50} gop 11
mkvidx T3b CAP 21 K 3 9 s video_h264_vfr_64x64.mp4                 intra 0 late 8 shown 270 decodes 425 seeks 28 mismatches 0 keyRels {0, 10, 20, 30, 40, 50} gop 10
mkvidx T3b CAP 21 K 3 9 s video_h264_vfrgap_64x64.mp4              intra 0 late 103 shown 144 decodes 318 seeks 22 mismatches 0 keyRels {0, 14, 24, 33, 42, 52, 62, 72, 80} gop 14
mkvidx T4 (i) PingPong CAP 21 25 s video_h264_gop250_64x64.mp4              intra 0 late 5 shown 747 decodes 2634 seeks 27 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (ii) flip CAP 21 8 s video_h264_gop250_64x64.mp4              intra 0 late 30 shown 229 decodes 1365 seeks 12 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (i) PingPong CAP 21 25 s video_h264_gop250_64x64.mkv              intra 0 late 5 shown 747 decodes 2634 seeks 27 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (ii) flip CAP 21 8 s video_h264_gop250_64x64.mkv              intra 0 late 30 shown 229 decodes 1365 seeks 12 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (i) PingPong CAP 21 25 s video_h264_gop250_cuesfront_64x64.mkv    intra 0 late 5 shown 747 decodes 2634 seeks 27 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (ii) flip CAP 21 8 s video_h264_gop250_cuesfront_64x64.mkv    intra 0 late 30 shown 229 decodes 1365 seeks 12 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (i) PingPong CAP 21 25 s video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 749 decodes 1413 seeks 28 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
mkvidx T4 (ii) flip CAP 21 8 s video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 237 decodes 443 seeks 13 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
mkvidx T4 (i) PingPong CAP 21 25 s video_h264_scenecut_64x64.mkv            intra 0 late 0 shown 749 decodes 1413 seeks 28 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
mkvidx T4 (ii) flip CAP 21 8 s video_h264_scenecut_64x64.mkv            intra 0 late 0 shown 237 decodes 443 seeks 13 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_gop250_64x64.mp4              intra 0 late 0 shown 749 decodes 795 seeks 3 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_gop250_64x64.mkv              intra 0 late 0 shown 749 decodes 795 seeks 3 mismatches 0 keyRels {0, 250} gop 250
mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_scenecut_64x64.mp4            intra 0 late 0 shown 749 decodes 668 seeks 4 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
mkvidx T4 (iii) PingPong CAP 164 25 s video_h264_scenecut_64x64.mkv            intra 0 late 0 shown 749 decodes 687 seeks 5 mismatches 0 keyRels {0, 37, 150, 190, 213, 262} gop 113
All tests passed (384 assertions in 6 test cases)
```

per case on the lane: P PASS (16 assertions), shape PASS, T1 PASS, T2 PASS, T3 PASS, T3b PASS, T4 PASS, T2e PASS. Every printed value == the ruling's G3 GREEN column (table below).

## Self-check: TSan (G2) -- worktree build-tsan (RelWithDebInfo, -DADNA_SANITIZE=thread; libclang_rt.tsan linked), TSAN_OPTIONS=halt_on_error=0:abort_on_error=0

```
test_gop_cache_store rc=0 WARNING-count=0 | All tests passed (1046 assertions in 32 test cases)
test_video_decode_trace rc=0 WARNING-count=0 | All tests passed (517 assertions in 6 test cases)
test_gop_cache rc=0 WARNING-count=0 | All tests passed (164 assertions in 14 test cases)
threaded reverse video_h264_gop30_64x64.mp4: shown 160 violations 0 hits 160 seeks 2 direction changes 7
threaded reverse video_h264_gop30_64x64.mkv: shown 159 violations 0 hits 161 seeks 2 direction changes 7
threaded reverse video_h264_gop30_64x64.mkv: keyRels {0 30 60 } gop 30
```
T2e's oracle (keyRels {0, 30, 60}, gop 30) PASSED inside the TSan run: the decode-thread rebuild ran under TSan with 0 reports (AM9's fallback not needed).

## Self-check: gop2 printed lines (G3) -- `test_gop_cache_store "gop2*"` lines vs gop2.md:68-82 (the GREEN block): `diff` empty on the rig-green binary AND the TSan binary (15 lines, IDENTICAL).

## G3 table -- printed (libavformat 62.3.100, printed by the shape case) vs the ruling's table

| case / row | ruling GREEN | lane printed | ruling RED | fa9604d printed (A0) |
|---|---|---|---|---|
| P | 16 assertions PASS | All tests passed (16 assertions in 1 test case) | does not compile | 6 x "use of undeclared identifier 'keyIndexFrom'" |
| fixture shape | PASS, 62.3.100 | PASS, 62.3.100 | PASS | PASS |
| T1 | MP4 0 / 268 / 2186; FX2 intra 0, 0 / 268 / 2186, mism 0; FX4 intra 1 | MP4 0 / 268 / 2186; FX2 intra 0, 0 / 268 / 2186, mism 0; FX4 intra 1 | FX2 intra 1, 875 / 1 / 3240 | FX2 intra 1, 875 / 1 / 3240 |
| T2 (a) FX1 | {0} 300 -> {0, 250} 250 | {0} 300 -> {0, 250} 250 | stays {0} 300 | stays {0} 300 |
| T2 (b) FX1 9 s fwd | seeks 0, {0, 250}, 250, late 0, shown 271, step 1 | seeks 0, {0, 250}, 250, late 0, shown 271 (0..270), steps != 1: 0 | {0} 300 (late 0, shown 271) | {0} 300, late 0, shown 271 |
| T2 (c) FX1 | 2186 -> 2186; 0 / 0; 268 / 268; gop 250 | 2186 -> 2186; 0 / 0; 268 / 268; {0, 250} 250 | mkv 2425; {0} 300 | 2425; {0} 300 |
| T2 (c) FX3 | 910 -> 910; {0, 37, 150, 190, 213, 262} 113 | 910 -> 910; 0 / 0; 268 / 268; same keys, 113 | 2141; {0} 300 | 2141; {0} 300 |
| T2 (c) FX5 2.8 s | 202 -> 202; 83 / 83; {0, 30, 60} 30 | 202 -> 202; 0 / 0; 83 / 83; {0, 30, 60} 30 | 197; {0} 90 | 197; {0} 90 |
| T2 (c) FX7 | 1044 -> 1044; 266 / 266; {0, 60, ..., 240} 60 | 1044 -> 1044; 0 / 0; 266 / 266; {0, 60, 120, 180, 240} 60 | 2212; {0} 300 | 2212; {0} 300 |
| T2 (c) FX8 1.8 s | 107 -> 106; 54 / 54; {0, 10, ..., 50} 10 | 107 -> 106; 0 / 0; 54 / 54; {0, 10, 20, 30, 40, 50} 10 | 108; {0} 60 | 108; {0} 60 |
| T2e FX5 threaded | {0, 30, 60} 30; shown ~159, viol 0, hits ~160, dir 7 | {0 30 60} 30; shown 159, viol 0, hits 160 (161 under TSan), dir 7 | {0} 90 | {0} 90 (shown 159/160, viol 0) |
| T3 FX6 2.5 s | intra, 0 / 75 / 78 | intra 1, 0 / 75 / 78 | 64 / 38 / 834 | 64 / 38 / 834 |
| T3 FX4 / FX4e 0.9 s | 0 / 27 / 30 each | 0 / 27 / 30 each | 4 / 17 / 324 each | 4 / 17 / 324 each |
| T3b | 561/4/269; 430/8/270; 425/8/270; vfrgap 318/103/144 | 561/4/269; 430/8/270; 425/8/270; 318/103/144 | same, vfrgap 327 (PASS) | same, vfrgap 327 (PASS) |
| T4 (i) PP CAP 21 | MP4 2634/5/747 = FX1 = FX2; FX3 1413/0/749 | MP4 2634/5/747 = FX1 = FX2; FX3 1413/0/749 = MP4 | FX1 2891/0/749; FX2 4048/1180/452; FX3 2624/0/749 | identical to the RED column |
| T4 (ii) flip CAP 21 | MP4 1365/30/229 = FX1 = FX2; FX3 443/0/237 | 1365/30/229 = FX1 = FX2; FX3 443/0/237 = MP4 | FX1 1592/14/233; FX2 1677/317/110; FX3 1414/21/231 | identical to the RED column |
| T4 (iii) PP CAP 164 | FX1 795 seeks 3 = MP4; FX3 687 s5 vs 668 s4; late 0 | FX1 795 s3 = MP4; FX3 687 s5 vs MP4 668 s4; late 0 | FX1 795/3; FX3 745 s3 (fails decodes) | identical to the RED column |

Every printed value equals the ruling's table (decodes / late / shown unless labelled).

## Mutant matrix (AM13) -- $A/mut.sh: a COPY of the lane tree (CMakeLists.txt, cmake, src, tests, resources; its own git repo) built by a normal cmake configure / build in its own build dir ($A/mut/build: test_gop_cache, test_gop_cache_store, test_video_decode_trace), the three binaries run whole; each edit reverted by copying the lane file back -> `git diff` of the copy = 0 lines after every mutant (printed). The deliverable was never mutated. Copy base (no edit): 14 / 32 / 6 cases, 164 / 1046 / 517 assertions, all PASS.

| mutant | diff (inline) | expected (AM13) | result |
|---|---|---|---|
| m1 | GopCache.h `- k.intraOnly = entries >= 2 && keys == entries && k.gopFrames == 1;` `+ k.intraOnly = entries >= 2 && keys == entries;` | P, T1, T4 fail | FAIL: P (test_gop_cache 13/14 passed), T1 (FX2 intra 1, 3240 / 875 / 1), T4 (FX2 4048 / 1180 / 452, flip 1677 / 317 / 110); T2e PASS |
| m2 | VideoPlayer.cpp `- readKeyIndex(false);` (decodeStep's first statement) | T2, T2e, T4 fail | FAIL: T2 (18 assertions), T4 (7), T2e (keyRels {0} gop 90) |
| m3 | `- readKeyIndex(false);` in decodeStep, `+ readKeyIndex(false);   // m3` in seekToTimestamp after av_seek_frame | T2 (b) fails | FAIL: T2 (b) only -- `{ 0 } == { 0, 250 }`, `300 == 250` |
| m4 | `- seekToTimestamp(ptsOfRel(r.seekFrom) + 0.5 * frameDur_ - runSeekBackSec_);` `+ seekToTimestamp(ptsOfRel(r.seekFrom) - runSeekBackSec_);` | T3 fails | FAIL: T3 (HAP 64 / 38 / 834; FX4 / FX4e 4 / 17 / 324) AND T2 (c) FX8 parity (`116.0 <= 109.14`: the open-GOP Matroska file without the aim) -- one tooth more than expected |
| m5 | `+ if (entries >= 2 && static_cast<int>(keyTs.size()) == entries) intraOnly_ = true;   // m5` at every rebuild, `- if (k.intraOnly) intraOnly_ = true;` | T1, T2, T4 fail | FAIL: T1, T2 (15), T4 (24), T2e, and the committed "GC3 keyframe gate: an MPEG-TS seek ..." case (`checked 0 >= 20`) |
| m6 | `- ... + 0.5 * frameDur_ - runSeekBackSec_);` `+ ... - 0.5 * frameDur_ - runSeekBackSec_);` | T3, T3b fail | FAIL: T3 (HAP late 0 shown 0 decodes 900; FX4 / FX4e 0 / 0 / 324), T3b (opengop 465 > 430, vfr 465 > 425); shape / T1 / T2 / T4 / T2e PASS. The whole-suite run FAILED the committed GC5 case and then stalled > 17 min inside it (stepUntilIdle's 200,000-step limit per frame on a run that never lands) -- I killed my own scratch binary (pid 17165, rc 143) and re-ran m6 per mkvidx case under a 600 s alarm ($A/mut6.sh) |
| m8 | `+ 0.5 * frameDur_` added at forwardStep's `seekToTimestamp(ptsOfRel(nextRel))` and forwardIdle's `seekToTimestamp(ptsOfRel(target))` | passes all (inert, recorded) | PASS all: 14 / 32 / 6 cases |
| m9 | `- if (!atOpen && entries == keyIndexEntries_ && firstTs == keyIndexFirstTs_ && lastTs == keyIndexLastTs_)` `+ if (!atOpen && entries == keyIndexEntries_)` | passes all (defensive superset, recorded) | PASS all: 14 / 32 / 6 cases |

Teeth rule: each of m1-m6 fails at least one case -- holds. Raw: $A/mut/results/<m>.{diff,test_*.txt}, $A/mut-all.log, $A/mut6.log.

## G8 greps (lane tree)

- (i) `grep -c "+ 0.5 \* frameDur_ - runSeekBackSec_" src/media/VideoPlayer.cpp` = 1, line 1329, inside VideoPlayer::runStep (1322-1369).
- (ii) `seekToTimestamp(ptsOfRel(nextRel));` = 1, `seekToTimestamp(ptsOfRel(target));` = 1 (byte-identical forward sites).
- (iii) the reader grep outside VideoPlayer.{h,cpp} prints nothing.
- (iv) the first statement of VideoPlayer::decodeStep is `readKeyIndex(false);`.

## Full ctest (G1 self-check)

`ctest --test-dir build-lane -j1 --output-on-failure` under the ctest mutex (/tmp/audiodna-ctest.lock acquired 16:14:12, released 16:16:08):
```
Total Tests: 1122
100% tests passed, 0 tests failed out of 1122
Total Test time (real) = 115.78 sec
```
`ctest -N` delta = +8 (1114 -> 1122): mkvidx P (#986), threaded reverse on a Matroska file (#1001), mkvidx fixture shape (#1028), mkvidx T1 (#1029), T2 (#1030), T3 (#1031), T3b (#1032), T4 (#1033). Build: Release, 0 warnings in the touched files (src/media/VideoPlayer*, GopCache.h, tests/test_gop_cache*, test_video_decode_trace); the app binary carries the witness string (`strings ... | grep -c "Keyframe index: "` = 1). Other lanes (ui, bf2) were compiling during the ctest run; no failure.

## FILES CHANGED (c1 24f9da3)

- src/media/GopCache.h: `struct KeyIndex` + `keyIndexFrom(keyTs, entries, timeBase, frameDur, totalFrames)` (pure; intraOnly needs gopFrames == 1).
- src/media/VideoPlayer.h: `readKeyIndex(bool atOpen)`; members keyIndexEntries_ / keyIndexFirstTs_ / keyIndexLastTs_ (the AM3 trigger), keyIndexWitnessed_ / keyIndexOpenKeys_ (the witness line); intraOnly_ / keyRels_ comments (open-only verdict; live index; decode-thread only).
- src/media/VideoPlayer.cpp: open()'s index loop -> `readKeyIndex(true)`; decodeStep's first statement `readKeyIndex(false);`; runStep's seek `+ 0.5 * frameDur_` (AM4, comment as ruled); readKeyIndex (AM2 guard, AM3 triple, once-per-player `[VideoPlayer] Keyframe index: N keyframes, longest interval G frames (open saw n0): <path>` via logLine).
- tests/test_gop_cache.cpp: "mkvidx P" (16 assertions).
- tests/test_gop_cache_store.cpp: VideoPlayerTestAccess::indexEntries; openCapped gains `bool reverse = true` (T4's PingPong arm opens at frame 0; every existing call unchanged); the AM5 recipe comment; helpers decodedKeyRels / keyShape / videoTimeBase / emulateArm / checkParity; cases "mkvidx fixture shape", T1, T2, T3, T3b, T4.
- tests/test_video_decode_trace.cpp: the threaded reverse body factored into `threadedReverse(file, keyRels*, gop*)` (the MP4 case calls it unchanged, bars unchanged; it now also prints one summary line); the new Matroska case + join / keyRels / gopEst test access.
- tests/fixtures/: the ten AM5 fixtures.

## Deviations / notes

- The rig rules say "one commit per plan item"; the adopted ruling's BUILD STAGES says "Commit c1 = Stage A (C++, fixtures, tests)" and the stage scope says "commit c1" -- I made ONE code commit c1 (+ this report as a separate docs commit).
- openCapped's new defaulted parameter is a change to a committed helper (no existing call changes).
- The witness line fires when the model first holds >= 2 keyframes; during forward play of a cues-at-end scene-cut file that is at frame 37's keyframe ("2 keyframes, longest interval 37") -- intended (plan: "first reaches >= 2 keyframes"), noted for u13's "Keyframe index:" count (INFO).
- m6 whole-suite stall (above): my own scratch test binary, killed by pid after confirming its command line; not the app.

## Notes for .harmony/notebook.md (Harmony appends)

- A copy-tree mutant rig (CMakeLists.txt + cmake + src + tests + resources copied to scratch, `git init`, its own cmake build dir of only the needed test targets) gives "normal cmake build, revert = empty git diff" mutants without touching the deliverable; the Harmony stash-guard hook blocks `git add -A` even in a scratch repo -- stage the paths explicitly. | discovered: $A/muttree.sh
- A mutant that breaks an intra DEMAND landing (m6) can make the committed GC5 case run for many minutes (stepUntilIdle's 200,000-step limit per frame): run such mutants per case under `perl -e 'alarm 600; exec @ARGV'`. | discovered: tests/test_gop_cache_store.cpp GC5 case
- A Release build started before a source edit leaves the app's object stale even when the test targets rebuilt later -- re-run the incremental build after the last edit before any ctest / strings check. | discovered: build-lane AudioDNA VideoPlayer.cpp.o

## Next stage (B) notes

- Base for Stage B = lane/mkvidx at c1 24f9da3 (+ this report commit). Build dirs present: build-lane (Release, test server + Syphon, up to date at c1), build-tsan (RelWithDebInfo TSan, 3 targets only).
- Item 4 (u13) can count the witness line: exact text `[VideoPlayer] Keyframe index: <n> keyframes, longest interval <g> frames (open saw <n0>): <path>` on stderr, once per player whose open() saw <= 1 keyframe.
- Docs (AM16): the run-only aim (forward repositioning seeks keep the exact nominal time); Pitfall **64** (Harmony-assigned); guards list: test_gop_cache.cpp "mkvidx P"; test_gop_cache_store.cpp "mkvidx fixture shape" / T1 / T2 / T3 / T3b / T4; test_video_decode_trace.cpp "threaded reverse on a Matroska file (Cues at the end)"; live probe-vupload u13. VideoPlayer.h's comments already name keyIndexFirstTs_ / keyIndexLastTs_ (AM16 (e) done in c1).
- CLAUDE.md measures 24,002 B on this base (`wc -c`, == fa9604d; the dispatch text said 23,996 -- the measured value is 24,002, = the ruling's G9 cap): Stage B must pay every added byte by moving text.
- G3 values to paste into the docs' measured table: the table above (all equal to the ruling's).
- No .venv symlink, no app launched, no live lock taken in Stage A.

## PACKET QUALITY

- Clarity: CLEAR (plan + ruling + adoption consistent; AM-level detail made every bar unambiguous).
- Missing context: where "the ctest mutex" lives (found in .harmony/s-rta-1002-work.md H12: mkdir /tmp/audiodna-ctest.lock); how mutants should be built without touching the deliverable (I used a cmake-built copy tree).
- Unused context: the live-app / lock-helper rules (no live run in Stage A).
- Self-brief files: plan-mkvidx.md, ruling-mkvidx.md (both read in full, useful); gop2.md (GREEN block, useful); the ruling's scratch rig ($R/t2e.cpp, rcost.cpp, proto3.diff: useful for exact harness parity).

---

# STAGE B (probe + docs)

Stage B: 2026-10-02 16:18:47 -> 16:33:08 EDT (report written) on lane/mkvidx e3dc697 (c1 24f9da3). Scratch: $B = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/mkvidx-B

## B1 item 4 = c2 0126610 (AM14)

Files:
- .harmony/probe-video.py make_fixtures: a spec with "remux" is made by `ffmpeg -y -loglevel error -i <source> -c copy <mux...> -fflags +bitexact <tmp>` + rename; remux sources are added to the fixture set; non-remux specs are made FIRST (sort key (is_remux, name)); the "keys" check applies as before. Docstring: one paragraph.
- .harmony/probe-video.json: "keyframesG250x60" [0, 8.333, ..., 58.333]; fixtures a1080_g250.mkv {remux a1080_g250.mp4}, a1080_g250_cuesfront.mkv {remux, mux ["-cues_to_front","1"]}, a1080_g250_60s.mp4 (a1080_g250.mp4's spec + "dur": 60), a1080_g250_60s.mkv {remux}, hap1080_tb600.mov {hap, -video_track_timescale 600, keys null}; "_mkvidx" doc.
- .harmony/probe-vupload.py: ROW_FIXTURES["u13_container_reverse"] = the five; u7's per-scene body -> `rev_scene(tag, lid, cid, sc, name, extra, kind, speed, ip, window=5.0, captures=True, witness=False)` (u7 calls it with the defaults); u8's window -> `column_window(window)` returning (kv, s0, s1, pol, t0, t1) (u8 calls it with 5.0); `witness_lines()` (count of "Keyframe index: " in OUT/err.log); `U13_SCENES` + `u13(tag)`: (a) mkv_cuesfront_reverse (b) mkv_reverse (c) hap600_reverse via rev_scene (u7 reverse scene); (d) mkv_long_column_1080x4 (e) mp4_long_column_1080x4 = four 60 s reversers by ONE trigger_column, 1 s, a 10 s column_window + max_gap_ms / seeks / nonmono / dirchg; (f) mkv_long_pingpong_turn (g) mp4_long_pingpong_turn = rev_scene kind "turn", in-point 0.95 (prime retrigger), window 12 s, captures off. Every scene's DATA line carries kf_lines (the witness delta). Clip ids 210-283, layers 169-172.
- .harmony/probe-vupload.json: "_u13" doc (the rules verbatim) + u13CtrlAFreezeMax 20, u13CtrlACpuMin 1.3, u13HapDecPerUploadMax 1.25, u13CpuRatioMax 0.7, u13ParityMax 1.15, u13LateSlack 15, u13PerPlayerSlack 1.5, u13PpLateSlack 15; layers "u13_container_reverse": [169, 170, 171, 172]. Blob at c2: e770e5c7a0455402c9c4fda79f7de4c3c8cdfc97 (AM14 (6)).
- .harmony/probe-vupload-ab.py: the u13 block -- per-scene A / B medians; [CTRL-A] (i) / (ii) / (iii) printed FIRST as INFO verdicts (no rc effect); one rule line each: [FREEZE] (a), [MKV] (b), [HAP] (c) (+ decoded_per_upload <= 1.25), [CPU] (d) B <= 0.7 x A (on FAIL with [PARITY] PASS an INFO "recorded verdict: live Matroska cost ratio R = B / A ... not a merge blocker (AM14 (4))"), [PARITY] (d) <= 1.15 x (e) in B, [GUARD] (d) late B <= A + 15 and per_player_min B >= A - 1.5, [PP-PARITY] (f) late <= (g) late + 15 in B; every rule FAILs with < 5 launches per arm of the scenes it reads; INFO big-share turn line incl. decision (iv)'s difference. No re-register path (AM14 (1)). `--selftest` gains three u13 TSVs (AM14 (7)).

### Selftest (G4) -- GREEN on the lane, raw (u13 part; the u8 checks above it all `ok`)
```
   ok    selftest (u13-all-pass): exit code 0
   ok    selftest (u13-all-pass): exactly 7 rule lines, one per u13 tag
   ok    selftest (u13-all-pass): exactly 3 [CTRL-A] INFO lines
   ok    selftest (u13-all-pass): every u13 rule PASS
   ok    selftest (u13-all-pass): [CTRL-A] (i) / (ii) / (iii) read the A arm as reproducing main
   ok    selftest (u13-frozen-B-a): exit code 1
   ok    selftest (u13-frozen-B-a): exactly 7 rule lines, one per u13 tag
   ok    selftest (u13-frozen-B-a): exactly 3 [CTRL-A] INFO lines
   ok    selftest (u13-frozen-B-a): exactly one FAIL and it is [FREEZE]
   ok    selftest (u13-too-few): exit code 1
   ok    selftest (u13-too-few): exactly 7 rule lines, one per u13 tag
   ok    selftest (u13-too-few): exactly 3 [CTRL-A] INFO lines
   ok    selftest (u13-too-few): every u13 rule FAIL, none PASS
SELFTEST PASS
```
rc 0.

### RED -- the same three u13 TSVs on the pre-change summarizer (HEAD~ = e3dc697's probe-vupload-ab.py, scratch copy $B/abred)
```
== HEAD ab.py on u13-all-pass.tsv     rule lines 0   rc=0
== HEAD ab.py on u13-frozen-B-a.tsv   rule lines 0   rc=0
== HEAD ab.py on u13-too-few.tsv      rule lines 0   rc=0
```
(the pre-change tool prints only generic medians for an unknown row and exits 0: a frozen B passes it.) The lane tool on the frozen-B TSV, raw:
```
   INFO  [CTRL-A] (i) A (a) median uploads/s 3.0 < 20: the pre-lane app freezes on the front-Cues file -- [FREEZE] discriminates (launches 5)
   INFO  [CTRL-A] (ii) A (c) median uploads/s 15.0 < 28.5 or decoded_per_upload 2.4 > 1.25: A fails [HAP]'s bars -- [HAP] discriminates (launches 5)
   INFO  [CTRL-A] (iii) A (d) median decoded_per_upload 8.0 >= 1.3 x A (e) 3.0: the pre-lane app pays the Matroska cost -- [CPU] discriminates (launches 5 / 5)
   FAIL  [FREEZE] (a) mkv_cuesfront_reverse: B medians uploads/s 3.0 >= 28.5, late 400.0 <= 10, bracket_ok 0.0 == 1, mono_ok 0.0 == 1; nonmono every B launch [0.0, 0.0, 0.0, 0.0, 0.0] == 0 (A medians uploads/s 3.0, late 400.0); launches 5
   PASS  [MKV] (b) mkv_reverse: ... launches 5
   PASS  [HAP] (c) hap600_reverse: ... decoded_per_upload 1.05 <= 1.25; ... launches 5
   PASS  [CPU] (d) B median decoded_per_upload 2.0 <= 0.7 x A's 8.0 (5.6); launches 5
   PASS  [PARITY] (d) vs (e): B median decoded_per_upload mkv 2.0 <= 1.15 x mp4 2.0 (2.3); launches 5
   PASS  [GUARD] (d) B median late 5.0 <= A 10.0 + 15 and B median per_player_min 29.5 >= A 29.0 - 1.5; launches 5
   PASS  [PP-PARITY] (f) vs (g): B median late mkv 20.0 <= mp4 10.0 + 15; launches 5
```

### Rule mutants (scratch copies $B/abmut, never the deliverable; $B/abmut.sh) -- each turns the selftest FAIL
| mutant | diff | selftest |
|---|---|---|
| abm1 | `- good = (nn(sc) >= 5 and u is not None ...` `+ good = (u is not None ...` | FAIL (u13-too-few: every u13 rule FAIL, none PASS) |
| abm2 | `- (("[FREEZE]", SA, "(a)"), ...` `+ (("[FREEZE]", SB, "(a)"), ...` | FAIL (u13-frozen-B-a: exit code 1; exactly one FAIL and it is [FREEZE]) |
| abm3 | `- nn = lambda *scs: min(len(per[sc][arm]) for sc in scs for arm in ("A", "B"))` `+ nn = lambda *scs: 5` | FAIL (u13-too-few: exit code 1; every u13 rule FAIL, none PASS) |

### u7 / u8 output unchanged (plan item 4: "u7's output unchanged")
$B/eq/run.py loads HEAD~'s and the lane's probe-vupload.py against ONE deterministic fake probe-video module ($B/eq/fakepv.py: counters growing per call, a frozen clock, no threads) and runs u7 (all nine scenes: reverse / forward / turn / speed-2 / flip) + u8: `71 lines each, diff empty -> IDENTICAL` (10 DATA lines).

### Fixtures (VIDEO_FIXTURES=$B/fixtures, made 16:23:49-16:24:04 by `probe-vupload.py --make-fixtures u13_container_reverse`)
```
fixture a1080_g250.mp4: keyframes [0.0, 8.333333] (as designed)
fixture a1080_g250_60s.mp4: keyframes [0.0, 8.333333, 16.666667, 25.0, 33.333333, 41.666667, 50.0, 58.333333] (as designed)
fixture hap1080_tb600.mov: encoded in 0.8 s
fixture a1080_g250.mkv: remuxed from a1080_g250.mp4 [] in 0.1 s -- keyframes [0.0, 8.333] (as designed)
fixture a1080_g250_60s.mkv: remuxed from a1080_g250_60s.mp4 [] in 0.1 s -- keyframes [0.0, 8.333, ..., 58.333] (as designed)
fixture a1080_g250_cuesfront.mkv: remuxed from a1080_g250.mp4 ['-cues_to_front', '1'] in 0.1 s -- keyframes [0.0, 8.333] (as designed)
```
ffprobe hap1080_tb600.mov: `codec_name=hap|width=1920|height=1080|r_frame_rate=30/1|time_base=1/600|nb_frames=300` (nb_read_frames 300). The ruling's index probe ($R/idx4, read-only), open + 3 packets -> after one seek: a1080_g250.mkv `1: 0` -> `2: 0 250`; a1080_g250_cuesfront.mkv `2: 0 250` at open (main's intra-only freeze case); a1080_g250_60s.mkv `1: 0` -> `8: 0 250 ... 1750`; a1080_g250_60s.mp4 1800 entries at open.

### ONE INFO live smoke of u13 (B = the lane app only; $B/smoke.sh, log $B/smoke.log, out $B/live/vupload.oJ6uXv)
- Lane app = build-lane/AudioDNA_artefacts/Release/Audio-DNA.app; `cmake --build build-lane --target AudioDNA` 16:25:56 = `[100%] Built target AudioDNA` (no-op: current at c1); `strings ... | grep -c "Keyframe index: "` = 1.
- Lock (LANE=mkvidx-B helper, acquire_quiet_lock): waited for a foreign compiler 16:26:15-16:27:56 (three clang processes, not this lane's), acquired 16:27:56, burners 0, released 16:29:41. No Audio-DNA was running at acquire. CPU before: three clang ~99 % (the wait), claude 12.8 %, Docker 8.7 %; after: WindowServer 25 %, syspolicyd 25 %, claude 18 % (no ChatGPT / Codex renderer in the top 6 at either sample).
- Run: `VIDEO_APP=<lane app> VIDEO_FIXTURES=$B/fixtures VIDEO_PY=<main .venv python> bash .harmony/probe-vupload.sh $B/live u13_container_reverse` (no .venv symlink was created: VIDEO_PY set). Result lines, raw:
```
DATA u13_container_reverse scene=mkv_cuesfront_reverse uploads_per_s=30.4 late=0 fps=105.08787536621094 hold_no_texture=0 decoded=0 dropped=0 seeks=0 skipped=0 decoded_per_upload=0.0 seeks_per_upload=0.0 max_gap_ms=70.5 gap_counter_ms=47.0 nonmono=0 dirchg=0 hits=152 misses=0 runs=0 cache_mb=890.6 code=194 bracket_ok=1 reversed=1 mono_ok=1 kf_lines=0
DATA u13_container_reverse scene=mkv_reverse uploads_per_s=30.6 late=0 fps=106.94105529785156 hold_no_texture=0 decoded=0 dropped=0 seeks=0 skipped=0 decoded_per_upload=0.0 seeks_per_upload=0.0 max_gap_ms=78.8 gap_counter_ms=39.0 nonmono=0 dirchg=0 hits=154 misses=0 runs=0 cache_mb=890.6 code=193 bracket_ok=1 reversed=1 mono_ok=1 kf_lines=1
DATA u13_container_reverse scene=hap600_reverse uploads_per_s=30.6 late=0 fps=119.99430084228516 hold_no_texture=0 decoded=151 dropped=0 seeks=151 skipped=0 decoded_per_upload=0.99 seeks_per_upload=0.987 max_gap_ms=63.6 gap_counter_ms=44.0 nonmono=0 dirchg=0 hits=0 misses=151 runs=0 cache_mb=0.0 code=194 bracket_ok=1 reversed=1 mono_ok=1 kf_lines=0
DATA u13_container_reverse scene=mkv_long_column_1080x4 cap_mb=0 uploads_per_s=120.6 per_player_min=30.1 per_player=30.1,30.1,30.2,30.2 late=0 hold_no_texture=0 pending=0 bytes_mb=2042.5 frames=688 active=4 over_budget=0 cap_bytes_mb=512.0 decoded_per_upload=1.8 fps=116.1232681274414 footprint_delta_mb=147.1 evictions=1055 max_gap_ms=79.3 seeks=13 nonmono=0 dirchg=0 kf_lines=4
DATA u13_container_reverse scene=mp4_long_column_1080x4 cap_mb=0 uploads_per_s=120.2 per_player_min=30.0 per_player=30.0,30.0,30.1,30.1 late=0 hold_no_texture=0 pending=0 bytes_mb=2042.5 frames=688 active=4 over_budget=0 cap_bytes_mb=512.0 decoded_per_upload=1.81 fps=118.0356216430664 footprint_delta_mb=169.8 evictions=1049 max_gap_ms=75.1 seeks=13 nonmono=0 dirchg=0 kf_lines=0
DATA u13_container_reverse scene=mkv_long_pingpong_turn uploads_per_s=30.08 late=0 fps=110.67517852783203 hold_no_texture=0 decoded=793 dropped=0 seeks=2 skipped=0 decoded_per_upload=2.2 seeks_per_upload=0.006 max_gap_ms=81.0 gap_counter_ms=34.0 nonmono=0 dirchg=1 hits=304 misses=1 runs=2 cache_mb=2045.5 turn_seen=1 kf_lines=1
DATA u13_container_reverse scene=mp4_long_pingpong_turn uploads_per_s=29.42 late=31 fps=109.90834045410156 hold_no_texture=0 decoded=779 dropped=588 seeks=1 skipped=0 decoded_per_upload=2.21 seeks_per_upload=0.003 max_gap_ms=313.1 gap_counter_ms=33.0 nonmono=0 dirchg=1 hits=296 misses=9 runs=1 cache_mb=2036.6 turn_seen=1 kf_lines=0

PY 7 PASS / 0 FAIL
PASS  no foreign render_frame traffic during the run
PASS  app terminated

PROBE-VUPLOAD GREEN
probe rc=0 at Fri Oct  2 16:29:20 EDT 2026
after: adna='' audio-dna windows 0, Output-named 0
UserNotificationCenter windows (all) 0
after +20 s: adna='' audio-dna windows 0, Output-named 0
```
- The 7 PASS lines are the per-scene VU5 hold checks. Captures: (a) mid code 194 in [191-1, 194+1], 20 mono codes 117 -> 56 strictly falling; (b) 193 in [191-1, 193+1], 115 -> 55; (c) 194 in [192-1, 194+1], 117 -> 56. I LOOKED at (a)'s and (c)'s mid captures (1920x1080, downscaled copies $B/look_a.png / look_c.png): testsrc2 bars + the moving line, the overlay clock 00:00:06.467 (= frame 194), the code band white at cells 1, 6, 7 (194 = 2 + 64 + 128).
- err.log: 6 `[VideoPlayer] Keyframe index:` lines -- `2 keyframes, longest interval 250 frames (open saw 1)` (the end-Cues 10 s mkv) and five `8 keyframes, longest interval 250 frames (open saw 1)` (four column players + the turn clip); none for the front-Cues file (open saw 2), the MP4s or the HAP clip -- as item 2 specifies.
- Fed to the ab tool as a single B launch (INFO): every u13 rule prints FAIL on the launch gate only ("launches 0"), [CTRL-A] reads "STOP / non-discriminating" for lack of an A arm -- the B values meet every B bar: (a) 30.4 uploads/s late 0, (b) 30.6 / 0, (c) 30.6 / 0 with 0.99 decodes per upload, (d) vs (e) 1.80 vs 1.81 decodes per upload ([PARITY] 1.8 <= 2.08), (f) late 0 vs (g) 31.
- Observations (one launch, INFO): (a) / (b) decoded 0 in-window: the single player's 2 GiB share holds all 300 frames after the first second (cache_mb 890.6, hits 152 / 154), so on B the reverse scenes measure the cache, and on A (main) (a) is intra-only = seek per step. The 60 s ping-pong turn: MKV late 0 / max gap 81 ms vs MP4 late 31 / max gap 313 ms (588 dropped) -- MP4's pre-existing F1 hold; [PP-PARITY] reads B (f) vs (g) and is not threatened by it.

## B2 item 5 docs = c3 (AM16)

- docs/claude/rendering.md (GOP-cache paragraph): "MKV / WebM hold a 1-entry index after open, ... (extra decodes bounded by the share, no stall)" -> "the keyframes are the demuxer's LIVE index -- re-read at the top of every decodeStep when its (entry count, first, last timestamp) changes (... 1.3-7x the decodes of the same stream in MP4 -- s-rta-1002b mkvidx, Pitfall 64)"; "or an index of keyframes only" -> "or an index whose entries are all keyframes ONE frame apart -- Matroska Cues list keyframes only: a long-GOP file with its Cues at the front was called intra-only and froze in reverse"; + the run-only aim sentence (AM16 (a): forward repositioning seeks keep the exact nominal time, no overshoot re-seek; clock-time seeks keep the clock); Levers + `VideoPlayer::readKeyIndex`, `GopCache::keyIndexFrom`; Guards "(incl. the mkvidx cases)", live u8-u13; NEW paragraph "Matroska index and the run seek's aim, measured" with the X1-X5 / E2 / E4 residual / E8 (4K live: unmeasured) / E9 (F7) table, the G3 ctest values and "[G6 medians, filled in by Harmony]".
- docs/claude/pitfalls.md: Pitfall 62 "(MKV / WebM keep a 1-entry index after open: their window is always the whole share)" -> "(the demuxer's live index -- Pitfall 64)"; 62's guard list + "mkvidx T1" / "mkvidx T2" / "mkvidx T3"; NEW Pitfall 64 (run-only aim; readKeyIndex as decodeStep's first statement, the (count, first, last) trigger and its members, the witness line; the intra rule + open()-only verdict; guards per AM16 (d): test_gop_cache.cpp "mkvidx P"; test_gop_cache_store.cpp "mkvidx fixture shape" / T1 / T2 / T3 / T3b / T4; test_video_decode_trace.cpp "threaded reverse on a Matroska file (Cues at the end)"; live probe-vupload u13).
- CLAUDE.md: index line after 63, AM16 (c) verbatim with NN = 64: "64. A run's frame seek aims at the frame's middle; the keyframe index is the demuxer's live one -- before touching `runStep`'s seek, `readKeyIndex` or the intra-only verdict." Paid by MOVING the Key-capabilities source breakdown " (3D 24, Geometric 11, ..., Routing 1)" verbatim into docs/claude/history.md:52 (which pointed back at it; now "(3D 24, ..., Routing 1; moved here from CLAUDE.md's Key Capabilities line, s-rta-1002b mkvidx; also in `.harmony/APP-INVENTORY.md`; SourceRegistry is ground truth)"). `wc -c CLAUDE.md` 24,002 -> 23,976 (cap 24,002). `git grep "Routing 1"`: no test / script parses that text.
- docs/claude/testing-eyes.md "Reverse / ping-pong rows": one sentence for u13 (scenes, the witness count, [CTRL-A] first, [FREEZE] / [MKV] / [HAP] / [CPU] / [PARITY] / [GUARD] / [PP-PARITY], bars frozen in "_u13").
- VideoPlayer.h comments: done in c1 (lines 283-294 name keyIndexEntries_ / keyIndexFirstTs_ / keyIndexLastTs_; AM16 (e)).

## FILES CHANGED (Stage B)
- c2 0126610: .harmony/probe-video.py (+22 / -2), .harmony/probe-video.json (+9), .harmony/probe-vupload.py (+259 / -85 incl. the moved u7 body), .harmony/probe-vupload.json (+12), .harmony/probe-vupload-ab.py (+124).
- c3: CLAUDE.md, docs/claude/history.md, docs/claude/pitfalls.md, docs/claude/rendering.md, docs/claude/testing-eyes.md, .harmony/.reports/s-rta-1002b/mkvidx.md (this report, git add -f).

## Deviations / notes (Stage B)
- [FREEZE] / [MKV] / [HAP] read "bracket_ok 1, mono_ok 1" as MEDIANS (u7's [ABS] instrument) and "nonmono 0" in EVERY B launch -- the plan's sentence "B medians: ..., bracket_ok 1, mono_ok 1, nonmono 0 in every B launch" is ambiguous on whether "in every B launch" binds all three; the rule line prints the per-launch nonmono list. Harmony may tighten before G6 (that changes the frozen blob: record it after).
- The "_u13" bars are top-level keys (u13*) + a "_u13" doc string, the file's convention for "_u8" / "_u9" (not a nested object); [CTRL-A]'s 20 and 1.3 and [PP-PARITY]'s 15 got their own keys (u13CtrlAFreezeMax, u13CtrlACpuMin, u13PpLateSlack).
- [CPU] FAIL still sets the exit code 1 like every rule (the tool's contract: "exit 1 on any FAIL"); the "recorded verdict R ... not a merge blocker" is an INFO line under it.
- RED for the probe row: the adoption allows ONE live smoke, run on B; the live RED (A arm) is G6's [CTRL-A]. The probe's RED is shown offline: the pre-change summarizer passes a frozen B (0 rule lines, rc 0), and three rule mutants fail the selftest.
- The stash-guard hook blocked `git add .harmony/...` (leading-dot relative paths read as a whole-tree add); staged by absolute path (the same five files).
- The smoke waited 1 m 41 s for three foreign clang processes (acquire_quiet_lock); not this lane's.

## Notes for .harmony/notebook.md (Harmony appends) -- Stage B
- A probe refactor (moving a row's body into a helper) can be proven output-identical offline: load the old and new probe module against ONE deterministic fake of the imported helper module (counters that grow per call, a frozen clock, a no-op Thread) and diff stdout -- every scene kind runs, no app. | discovered: $B/eq/run.py
- The Harmony stash-guard hook reads `git add .harmony/<file>` (a relative path starting with ".") as a whole-tree add; stage by absolute path. | discovered: c2 staging
- In u13 a single player's reverse of a 300-frame 1080p clip is served entirely from its 2 GiB share after ~1 s (decoded 0 in the window): decode-count rules for one-player reverse scenes need a longer clip or a smaller budget; u13 puts its cost rules on the four-player 60 s column. | discovered: smoke DATA (a) / (b)

## PACKET QUALITY (Stage B)
- Clarity: CLEAR (plan item 4 + AM14 + AM16 specify every scene, key, bar and doc sentence); one ambiguity recorded above (the "in every B launch" binding).
- Missing context: none blocking; where the "_u13" bars live (nested vs top-level) was inferred from the file's convention.
- Unused context: the TSan / mutant-build instructions (Stage A's).
- Self-brief files: lane report Stage A (useful: witness text, G3 table, CLAUDE.md size), plan-mkvidx.md + ruling-mkvidx.md (read in full, useful), lock.sh (useful), probe-vupload*.{py,json,sh} / probe-video.{py,json} (read).

STATUS: DONE

# FIX ROUND (Harmony rulings) -- lane-name mkvidx-fix, continued on lane/mkvidx from 79dd452

STATUS: DONE (started Fri Oct  2 16:44:57 EDT 2026; finished Fri Oct  2 17:01:30 EDT 2026)

## R1 (decode S1: intra-only Matroska rebuild per step) = d251d04
- Finding VERIFIED against the code before fixing: readKeyIndex (VideoPlayer.cpp ~:1579) rebuilt keyRels_ / gopFramesEst_ on
  every index change for every stream; for an intra-only stream the only readers of keyRels_ (planPrefetchRun :1281,
  forwardRetain :1038) return first on intraOnly_; gopFramesEst_'s readers are unreachable for intra (hitsArmed_ is false for
  intra :817, so forwardStep's after-hits threshold and forwardIdle never run; planPrefetchRun returns) except the pts-less
  overshoot fallback in onRunDecoded (`known ? ... : max(1, gopFramesEst_) * frameDur_`, ~:1417), which keeps open()'s
  value -- the fa9604d behaviour (INFERRED harmless: Matroska frames always carry pts).
- Fix: `if (formatCtx_ == nullptr || videoStreamIndex_ < 0 || (!atOpen && intraOnly_)) return;` (open() still reads the
  index once and decides the verdict); `VideoStats::keyIndexRebuilds` (relaxed atomic, ++ by the decode thread on every
  non-open rebuild, before the members are written). The once-per-player witness is unchanged for non-intra files (an
  intra-only player no longer logs it; Pitfall 64 (2) says "A non-intra player").
- New case "mkvidx T2f: an intra-only Matroska file read forward never rebuilds its keyframe model per frame" (FX4e, big
  budget, Show 1.1 s forward: the whole 30-frame file, EOF, the Loop wrap). Bars: intra 1; index entries grew by > 2
  (non-vacuous); shown >= 30; rebuilds <= 2.
- RED = 79dd452 + the counter only (the counter does not exist on 79dd452; the skip line absent), raw:
```
mkvidx T2f FX4e forward 1.1 s: intra 1 index entries 8 -> 30, rebuilds 22, shown 34 (late 0)
/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/mkvidx/tests/test_gop_cache_store.cpp:2090: FAILED:
  CHECK( st.keyIndexRebuilds.load() <= 2 )
with expansion:
  22 <= 2
test cases: 1 | 1 failed
assertions: 5 | 4 passed | 1 failed
```
  (22 = 30 - 8: one rebuild per frame read.)
- GREEN (the skip added), raw:
```
mkvidx T2f FX4e forward 1.1 s: intra 1 index entries 8 -> 30, rebuilds 0, shown 34 (late 0)
All tests passed (5 assertions in 1 test case)
```
- Whole suites after R1 (build-lane): test_gop_cache_store All tests passed (1051 assertions in 33 test cases);
  test_video_decode_trace 517 / 6; test_gop_cache 164 / 14; test_video_player_open 10 / 2. `"mkvidx*"` 389 / 7.
  gop2 printed lines vs gop2.md:68-82: `diff` empty (15 lines).
- Printed-value drift (expected, INFO): every mkvidx line == the Stage A GREEN block EXCEPT T3's FX4e line, whose keyRels is
  now the open-time read-ahead `{0, 1, 2, 3, 4, 5, 6, 7}` instead of `{0, 1, ..., 29}` (the model is no longer rebuilt for an
  intra stream); its late / shown / decodes 0 / 27 / 30 == the G3 table (G3 compares those only).
- Docs: Pitfall 64 (2) + "never after open() for an intra-only stream ... VideoStats::keyIndexRebuilds", guard list + "mkvidx
  T2f"; VideoPlayer.h keyRels_ comment.
- ctest count: +1 (mkvidx T2f) -> 1123.

## R2 (decode S2: adjacent first keyframes fool the open verdict) -- F8 + docs = 96229f7
- Finding VERIFIED by reading: GopCache::keyIndexFrom sets `intraOnly = entries >= 2 && keys == entries && gopFrames == 1`
  (GopCache.h ~:497); an open-time index of exactly two keyframes one frame apart (keys at frames 0 and 1) gives gopFrames
  = lround(1 frame) = 1 -> intra-only, decided once in open() (readKeyIndex(true)) and never revisited. The freeze
  consequence is the reviewer's run on the plan's proto3 binary (`adj3.mkv` late 879 / shown 1) -- NOT re-run here
  (INFERRED for the lane binary: same rule, same open-time-only verdict).
- Harmony's ruling: NO code change. Docs reworded so the rule is no longer called safe without qualification:
  Pitfall 64 (3) ("The rule is NOT safe in every case -- it sees only the open-time index, and two known exceptions are
  filed: F8 ... F6 ..."), rendering.md (the intra-only clause of the GOP-cache paragraph names F8 / F6), and the
  GopCache::keyIndexFrom comment (comment-only; the B1 rule's definition site).
- FILED (beside F6, not this lane):
  - F6 (AM17, restated) an all-intra stream whose open-time index holds < 2 entries goes the cache path (E1 residual:
    CAP 21 K 3 281 vs 273 decodes, CAP 164 late 29 once). Trigger: an all-intra MKV / WebM whose open log line lacks
    ", intra-only".
  - F8 (NEW, decode review S2) a long-GOP Matroska file (Cues at the end) whose first two keyframes are ADJACENT (e.g.
    `libx264 -g 250 -bf 0 -force_key_frames "0,0.03"`: keys 0, 1, 251) opens with a 2-entry index, both keyframes, gap 1 ->
    intra-only -> reverse freezes like X1 (reviewer, proto3: late 879 / shown 1 / 3240 decodes vs 30 / 258 for ordinary
    keys). Not a regression (fa9604d calls ANY 2-key all-key index intra). Rare shape (min-keyint normally forbids it).
    Candidates: require the gap rule over >= N entries, or a one-way intra -> not-intra demotion at the first rebuild when
    keyRels_ shows a gap > 1 and nothing is cached (note: a demotion is the safe direction; the m5 freeze was a
    PROMOTION; since R1 an intra-only player no longer rebuilds, so such a check needs its own bounded read of the index). Trigger: a performer report of a frozen MKV reverse whose open log line says ", intra-only" on a non-intra
    codec.

## R4 (gates SHOULD 2: bracket_ok / mono_ok / nonmono per launch) = c16f9da
- Finding VERIFIED by reading 79dd452 .harmony/probe-vupload-ab.py: [FREEZE] / [MKV] / [HAP] took bracket_ok / mono_ok as
  MEDIANS (`bo == 1 and mo == 1` on `mm(sc, "B", k)`), nonmono per launch.
- Harmony's ruling (per launch): `bo, mo, nm = ([kv.get(k) for kv in per[sc]["B"]] for k in (...))`, `all(x == 1 for x in
  bo)`, `all(x == 1 for x in mo)`, `all(x == 0 for x in nm)` (each list non-empty); uploads/s and late stay medians. The rule
  line prints the three per-launch lists. Docs: the module docstring, probe-vupload.json "_u13" ("bracket_ok 1, mono_ok 1
  and nonmono 0 in EVERY B launch (per launch, never medians -- fix round R4)"), testing-eyes.md's u13 sentence.
  The "_u13" string changed -> the frozen JSON blob changed: Harmony records `git rev-parse HEAD:.harmony/probe-vupload.json`
  AFTER this round, before the first G6 launch (bars unchanged: u13* keys untouched).
- Commit split: c16f9da = the rule (the 79dd452 selftest still PASSes on it: its three TSVs have identical launches);
  its selftest coverage (one bad launch of five per tag x bracket / mono / nonmono) lands in R3's d256d33.
- RED = the new selftest (d256d33's) run against the 79dd452 rule code (only the selftest replaced), raw FAIL lines:
```
   FAIL  selftest (u13-[FREEZE]-bracket-one-launch): exit code 1
   FAIL  selftest (u13-[FREEZE]-bracket-one-launch): FAIL exactly ['[FREEZE]'], every other u13 rule PASS
   FAIL  selftest (u13-[FREEZE]-mono-one-launch): exit code 1
   FAIL  selftest (u13-[FREEZE]-mono-one-launch): FAIL exactly ['[FREEZE]'], every other u13 rule PASS
   FAIL  selftest (u13-[MKV]-bracket-one-launch): exit code 1
   FAIL  selftest (u13-[MKV]-bracket-one-launch): FAIL exactly ['[MKV]'], every other u13 rule PASS
   FAIL  selftest (u13-[MKV]-mono-one-launch): exit code 1
   FAIL  selftest (u13-[MKV]-mono-one-launch): FAIL exactly ['[MKV]'], every other u13 rule PASS
   FAIL  selftest (u13-[HAP]-bracket-one-launch): exit code 1
   FAIL  selftest (u13-[HAP]-bracket-one-launch): FAIL exactly ['[HAP]'], every other u13 rule PASS
   FAIL  selftest (u13-[HAP]-mono-one-launch): exit code 1
   FAIL  selftest (u13-[HAP]-mono-one-launch): FAIL exactly ['[HAP]'], every other u13 rule PASS
```
  (exactly the six median-read cases; the nonmono one-launch cases already passed -- nonmono was per launch.) GREEN: SELFTEST PASS.

## R3 (gates SHOULD 1: selftest discriminates every u13 rule) = d256d33
- Finding VERIFIED: the 79dd452 selftest had three u13 TSVs (all-pass / frozen-B (a) / too-few); 15 of 16 neutralised
  comparisons still printed SELFTEST PASS on it (left column of the table below; the reviewer's list reproduced).
- Fix: the u13 selftest is table-driven -- 5 launches per arm of the all-pass values, then each case's overrides
  (arm, scene, {k: v}, launch or every launch) -- and each case asserts the exit code, 7 rule lines one per tag, the exact
  set of FAILing tags (every other rule PASS) and the exact set of [CTRL-A] lines reading non-discriminating. 43 u13 cases
  (3 old + 40 new): every comparison once just over its bar (that rule alone FAILs; for [CTRL-A] its line flips to STOP /
  non-discriminating / does not reproduce) and once just under (PASS): [FREEZE] / [MKV] / [HAP] uploads 28.4 / 28.6, late
  11 / 10, bracket / mono / nonmono one bad launch of five (R4); [HAP] decoded_per_upload 1.26 / 1.24; [CPU] A (d) 2.85 /
  2.86 (B 2.0 vs 0.7 x A); [PARITY] B (e) 1.73 / 1.74 (B (d) 2.0 vs 1.15 x B (e)); [GUARD] B (d) late 26 / 25 (A 10 + 15),
  per_player_min 27.4 / 27.6 (A 29.0 - 1.5); [PP-PARITY] B (f) late 26 / 25 (B (g) 10 + 15); [CTRL-A] (i) A (a) uploads
  20.1 / 19.9, (ii) A (c) (uploads 28.6, dpu 1.24) / (28.4, 1.24) / (28.6, 1.26), (iii) A (e) 6.2 / 6.1 (A (d) 8.0 vs 1.3 x).
  One base change: A (e) decoded_per_upload 3.0 -> 2.0 so the [CPU] cases (A (d) 2.85) keep [CTRL-A] (iii) discriminating.
- GREEN: `python3 .harmony/probe-vupload-ab.py --selftest` -> 226 `ok` lines, 0 FAIL, `SELFTEST PASS` (rc 0) (was 25 checks).
- Teeth ($F/abmut.py: each mutant = a scratch copy of the file + probe-vupload.json under $F/abmut/, ONE comparison replaced
  by `True`; the same mutant applied to the 79dd452 file for the left column; the deliverable never mutated -- `git diff` empty
  after the run), raw:
```
ctrl-i ua < fz                             | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 1 failed checks: SELFTEST FAIL (u13-[CTRL-A]-i-over: [CTRL-A] non-discriminating exactly ['(i)'])
ctrl-ii uc < umin                          | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 1 failed checks: SELFTEST FAIL (u13-[CTRL-A]-ii-over: [CTRL-A] non-discriminating exactly ['(ii)'])
ctrl-ii dc > hmax                          | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 1 failed checks: SELFTEST FAIL (u13-[CTRL-A]-ii-over: [CTRL-A] non-discriminating exactly ['(ii)'])
ctrl-iii dd >= cm * de                     | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 1 failed checks: SELFTEST FAIL (u13-[CTRL-A]-iii-over: [CTRL-A] non-discriminating exactly ['(iii)'])
FREEZE/MKV/HAP u >= umin                   | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 6 failed checks: SELFTEST FAIL (u13-[FREEZE]-uploads-over: exit code 1; u13-[FREEZE]-uploads-over: FAIL exactly ['[FREEZE]'], every other u13 rule PASS; u13-[MKV]-uploads-over: 
FREEZE/MKV/HAP lt <= lmax                  | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 6 failed checks: SELFTEST FAIL (u13-[FREEZE]-late-over: exit code 1; u13-[FREEZE]-late-over: FAIL exactly ['[FREEZE]'], every other u13 rule PASS; u13-[MKV]-late-over: exit code
FREEZE/MKV/HAP bracket                     | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 6 failed checks: SELFTEST FAIL (u13-[FREEZE]-bracket-one-launch: exit code 1; u13-[FREEZE]-bracket-one-launch: FAIL exactly ['[FREEZE]'], every other u13 rule PASS; u13-[MKV]-br
FREEZE/MKV/HAP mono                        | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 6 failed checks: SELFTEST FAIL (u13-[FREEZE]-mono-one-launch: exit code 1; u13-[FREEZE]-mono-one-launch: FAIL exactly ['[FREEZE]'], every other u13 rule PASS; u13-[MKV]-mono-one
FREEZE/MKV/HAP nonmono                     | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 6 failed checks: SELFTEST FAIL (u13-[FREEZE]-nonmono-one-launch: exit code 1; u13-[FREEZE]-nonmono-one-launch: FAIL exactly ['[FREEZE]'], every other u13 rule PASS; u13-[MKV]-no
HAP dp <= hmax                             | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 2 failed checks: SELFTEST FAIL (u13-[HAP]-dpu-over: exit code 1; u13-[HAP]-dpu-over: FAIL exactly ['[HAP]'], every other u13 rule PASS)
CPU bd <= cr * ad                          | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 2 failed checks: SELFTEST FAIL (u13-[CPU]-over: exit code 1; u13-[CPU]-over: FAIL exactly ['[CPU]'], every other u13 rule PASS)
PARITY bd <= pm * be                       | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 2 failed checks: SELFTEST FAIL (u13-[PARITY]-over: exit code 1; u13-[PARITY]-over: FAIL exactly ['[PARITY]'], every other u13 rule PASS)
GUARD bl <= al + ls                        | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 2 failed checks: SELFTEST FAIL (u13-[GUARD]-late-over: exit code 1; u13-[GUARD]-late-over: FAIL exactly ['[GUARD]'], every other u13 rule PASS)
GUARD bp >= ap - ps                        | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 2 failed checks: SELFTEST FAIL (u13-[GUARD]-player-over: exit code 1; u13-[GUARD]-player-over: FAIL exactly ['[GUARD]'], every other u13 rule PASS)
PP-PARITY fl <= gl + pp                    | 79dd452 selftest: SELFTEST PASS (rc 0) | fix selftest: rc 1, 2 failed checks: SELFTEST FAIL (u13-[PP-PARITY]-over: exit code 1; u13-[PP-PARITY]-over: FAIL exactly ['[PP-PARITY]'], every other u13 rule PASS)
FREEZE/MKV/HAP launch gate nn(sc) >= 5     | 79dd452 selftest: SELFTEST FAIL  (rc 1) | fix selftest: rc 1, 1 failed checks: SELFTEST FAIL (u13-too-few: every u13 rule FAIL, none PASS)
R4 bracket/mono read as MEDIANS            | fix selftest: rc 1, 12 failed checks: SELFTEST FAIL (u13-[FREEZE]-bracket-one-launch: exit code 1; u13-[FREEZE]-bracket-one-launch: FAIL exactly ['[FREEZE]'], every other u13 rule PASS; u13-[FREEZE]
```
  Every neutralised comparison and the median reading make the fix's selftest FAIL; on 79dd452 only the launch gate did.
  ($F = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/mkvidx-fix)

## NITs
- decode N1 / gates 3 (keyIndexWitnessed_ not reset by open()) -- VERIFIED by reading (open() reset keyIndexOpenKeys_ in
  readKeyIndex's atOpen branch, never keyIndexWitnessed_). FIXED in one line = 0f8c517:
  `keyIndexWitnessed_ = false;` in the atOpen branch. No test (an INFO log line; production opens a new VideoPlayer per file).
- gates 4 (T3b's vfrgap bar is the base's 327, not the lane's 318) -- NOT fixed: AM11 rules the T3b bars ARE the base's
  values ("a GUARD, not a RED ... per file decodes <= base"), and the reviewer recorded it "Ruled by AM11 -- recorded only";
  a one-line change to 318 would override a ruling the adoption adopted. Left for Harmony.

## ctest / TSan (fix round)
- Build: `cmake --build build-lane -j3` rc 0 (only the pre-existing MainComponent.cpp:1554 unused-parameter warning).
  `ctest -N` = 1123 (1122 + mkvidx T2f; the ruling's G1 "before + 8" is now "+ 9").
- Full ctest serial under the cross-lane mutex ($F/ctest.sh: lock acquired 16:57:35, released 16:59:22), verbatim:
```
100% tests passed, 0 tests failed out of 1123

Label Time Summary:
tsan    =   0.05 sec*proc (4 tests)

Total Test time (real) = 106.91 sec
```
- TSan (build-tsan, RelWithDebInfo, ADNA_SANITIZE=thread, libclang_rt.tsan_osx_dynamic.dylib linked;
  TSAN_OPTIONS=halt_on_error=0:abort_on_error=0; the three targets rebuilt from this round's sources), verbatim:
```
test_gop_cache_store rc=0 warnings=0 | All tests passed (1051 assertions in 33 test cases)
test_video_decode_trace rc=0 warnings=0 | All tests passed (517 assertions in 6 test cases)
test_gop_cache rc=0 warnings=0 | All tests passed (164 assertions in 14 test cases)
mkvidx T2f FX4e forward 1.1 s: intra 1 index entries 8 -> 30, rebuilds 0, shown 34 (late 0)
threaded reverse video_h264_gop30_64x64.mkv: keyRels {0 30 60 } gop 30
```
  (probe-tsan-unit.sh not used for this lane, per the task.)
- `python3 .harmony/probe-vupload-ab.py --selftest` -> SELFTEST PASS (226 checks).
- G8 greps on the fix tree: (i) 1, inside runStep; (ii) 1 and 1; (iii) no reader outside VideoPlayer.{h,cpp} (0 lines);
  (iv) decodeStep's first statement `readKeyIndex(false);`. G9: CLAUDE.md 23,976 B (untouched).
- No live app launched in this round (no lock taken; `adna` empty, outwins "audio-dna windows 0, Output-named 0" at 17:00:33).
  R1 changes the live decode path only for intra-only streams after open (fewer rebuilds); G6 / G7 stay Harmony's gates.

## FILES CHANGED (fix round, on 79dd452)
- d251d04 R1: src/media/VideoPlayer.cpp (intra skip + counter ++), src/media/VideoStats.h (keyIndexRebuilds),
  src/media/VideoPlayer.h (keyRels_ comment), tests/test_gop_cache_store.cpp (mkvidx T2f), docs/claude/pitfalls.md (64 (2) +
  guard list).
- 96229f7 R2: docs/claude/pitfalls.md (64 (3)), docs/claude/rendering.md, src/media/GopCache.h (comment only).
- c16f9da R4: .harmony/probe-vupload-ab.py (per-launch binding + docstring), .harmony/probe-vupload.json ("_u13" text only),
  docs/claude/testing-eyes.md.
- d256d33 R3: .harmony/probe-vupload-ab.py (table-driven u13 selftest, 43 cases).
- 0f8c517 NIT: src/media/VideoPlayer.cpp (one line).
- this report (git add -f).

## Deviations / notes (fix round)
- RED for R1 is "79dd452 + the counter" (the counter is new; the test cannot compile on 79dd452 itself).
- R3 and R4 are two commits: R4's rule first (the old selftest stays green on it), R3's selftest second (it carries R4's
  one-bad-launch cases); R4's RED is the new selftest on the old rule code (pasted under R4).
- The "_u13" JSON string changed (R4 wording) -> the frozen blob hash changed; bars (u13* numbers) unchanged.
- VideoStats.h is outside the adoption's file list (VideoPlayer* / GopCache* / VideoRing*) -- edited because Harmony's R1
  ruling names it; a concurrent lane editing VideoStats.h would conflict at merge (one added member; INFERRED low risk).
- Rig-rule slip: one read-only command began with `cd /tmp >/dev/null;` (a `git show | diff` snapshot check, 16:51);
  no effect on any file; every later command used absolute paths only.
- Mutant scratch copies under $F/abmut (never the deliverable; `git diff` of the probe file after the matrix = the committed
  content).

## Notes for .harmony/notebook.md (Harmony appends) -- fix round
- A per-step "did the index change" rebuild is O(N) per FRAME on an all-intra Matroska file (its index grows one entry per
  frame read); gate such rebuilds on whether the model is read at all (intra-only: never). | discovered:
  src/media/VideoPlayer.cpp readKeyIndex, mkvidx T2f
- A selftest only discriminates a threshold rule if it has a case just over AND just under EACH comparison (a table of
  per-case overrides on one all-pass base + "exactly these tags FAIL"); verify by neutralising each comparison to `True` in
  a scratch copy (`$F/abmut.py`). | discovered: .harmony/probe-vupload-ab.py selftest (R3)

## PACKET QUALITY (fix round)
- Clarity: CLEAR (four rulings, each naming the change and its test; NIT rule "one line each").
- Missing context: none blocking. Inferred: "RED on 79dd452" for R1 = 79dd452 + the counter (the counter is part of R1).
- Unused context: the live-app lock / screen-safety rules (no live run needed this round).
- Self-brief files: plan-mkvidx.md + adoption, ruling-mkvidx.md (read in full), both r1 reviews (read in full), the Stage A/B
  lane report, lock.sh (outwins / adna only).

INBOX-RECHECK: none
