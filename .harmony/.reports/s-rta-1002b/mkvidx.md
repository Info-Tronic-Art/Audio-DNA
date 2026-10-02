# LANE REPORT mkvidx (s-rta-1002b) -- Stage A (C++)

STATUS: DONE
RESULT: Stage A built as ruled (AM1-AM13): items 1-3 in src/media, ten bitexact fixtures, 8 new ctest cases; c1 = 24f9da3.
FACTS:
- c1 24f9da34679fbbbf4a2da3c01c940bceae417fab on lane/mkvidx (parent 9a832a0; src / tests of the parent == fa9604d).
- A0 RED on fa9604d's src (relink rig $A/rig.sh red): P = compile error (6 x "use of undeclared identifier 'keyIndexFrom'"); shape PASS; T1 / T2 / T3 / T4 / T2e FAIL; T3b PASS -- every value == the ruling's RED column (section A0 below, raw).
- GREEN: every mkvidx / T2e printed value == the ruling's G3 GREEN column (table below); `gop2*` lines identical to gop2.md:68-82.
- `ctest --test-dir build-lane -j1` (ctest mutex): `100% tests passed, 0 tests failed out of 1122` (1114 + 8), 115.78 s.
- TSan (build-tsan, RelWithDebInfo, ADNA_SANITIZE=thread, TSAN_OPTIONS=halt_on_error=0:abort_on_error=0): test_gop_cache_store rc 0 / 0 reports (1046 assertions, 32 cases); test_video_decode_trace rc 0 / 0 (517, 6) -- T2e oracle PASS inside it; test_gop_cache rc 0 / 0 (164, 14).
- Mutants m1-m6 each fail >= 1 case; m8 / m9 pass all (inert / defensive, as AM13 expects).
- G5: the committed blobs' sha256 prefixes == AM5 (all ten). G8 (i)-(iv) hold.
METHOD: RED first with the gop2 relink rig (fa9604d src exported by `git archive`, the worktree's test TUs, main build's JUCE / Catch2 objects read-only), then items 1-3 from the plan's prototype with AM2 / AM3 / AM4 applied, then the same rig on the lane src, the lane cmake build, ctest, TSan, and a copy-tree mutant matrix built by cmake.
CONFIDENCE+VERIFY: High -- deterministic emulated counts reproduce the ruling's tables to the unit. Re-check: `build-lane/tests/test_gop_cache_store "mkvidx*"`, `build-lane/tests/test_video_decode_trace "threaded reverse on a Matroska file*"`, `ctest --test-dir build-lane -j1`.
UNKNOWNS-NOT-DONE: Stage B (item 4 probe u13 + its live INFO smoke, item 5 docs incl. Pitfall 64 / CLAUDE.md line) is the next stage's. No live app was launched in Stage A (none needed).
NUANCE: m4 also fails T2 (c)'s open-GOP Matroska parity row (an extra tooth the ruling did not predict); m5 also fails the committed MPEG-TS GC3 case; m6's whole-suite run stalls > 17 min in the committed GC5 case (killed; re-run per mkvidx case).
HANDOFF-NEEDS: none (Harmony's gates G1-G9 after Stage B + merge).
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
