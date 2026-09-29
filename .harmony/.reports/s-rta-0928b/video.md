# s-rta-0928b video lane report -- video decodes off the GL render thread

STATUS: PARTIAL
RESULT: All six planned commits are on lane/video, rebased on main 328301d (seqvram merged); 14 new ctests, 883/883.
probe-video goes from main 14 PASS / 39 FAIL to 48 / 2 on the final app. The retrigger freeze (46 polls of ~63 ms at
1080p, 13 of ~235 ms at 4K) is gone: every poll <= 16.7 ms, a held frame instead. No avcodec call is left on the render
thread, and the message thread never waits on a decode (lock wait 222 ms -> 0.0003 ms). Not GREEN by the plan's definition:
w2 (a) 4 x 4K reaches 84-90 fps, not 110, and the ceiling of the same scene with NO video is 108-110 fps (a design finding,
not re-thresholded). w1 (a) (4 x 1080p) is bimodal on the same binary under the probe's 15 ms poller.
FACTS: (every number below is from a run log under scratchpad/video/{runs,reg,diag}, named per section)
METHOD: plan-video.md + HARMONY ADOPTION V1-V7, in the plan's commit order; RED on main / commit-1 / commit-2 apps, GREEN on
the lane app, teeth on header copies and on a temporary env-switched build (never committed; sources restored by blob hash).
CONFIDENCE + VERIFY: high for the protocol (14 cases, TSan x3, 9 teeth), the off-GL decode (gl_video_decode_calls 0 in
every lane run), the hold / no-freeze rows, byte identity and every regression probe; medium for the fps rows (w1 bimodal).
Verify: `ctest --test-dir build-lane -j1` (883); `VIDEO_APP=<app> bash .harmony/probe-video.sh` under the live lock.
UNKNOWNS / NOT DONE: w2 (a) not met (see "w2"); w1 (a) not stable (see "w1"); the context-loss hold has no live row (V1:
the only triggers open or resize an Output window).
NUANCE: this lane changed three things beyond the plan's text, each forced by building it (see "Beyond the plan").
HANDOFF-NEEDS: Harmony -- pitfall number for "NN" (55); a ruling on w2 (a) and on w1's measurement; Boris feel items below.
INBOX-RECHECK: none (no addendum messages arrived in this run)

## Status for the relayed question ("a status on the build, what is left?")
The video lane is built and gated; what is left is Harmony's: review, the two fps rulings below, pitfall number, merge.

## Commits (lane/video on main 328301d)
| # | commit | what |
|---|---|---|
| 1 | cfa921d perf | VideoStats counters on the OLD code, /api/state fields (7070 + 8080), probe-video.{sh,py,json} (RED harness) |
| 2 | 925a7fa perf | b1: upload only a newly decoded frame |
| 3 | bdeb12b feat | src/media/VideoRing.h (pure) + tests/test_video_ring.cpp (14 cases) |
| 4 | d7f8eea perf | VideoPlayer decode thread + ring; Renderer lookup-only mutex, installVideoPlayer, deferred destroy; C1 provider for video |
| 5 | 29831d1 test | probe-video w4b: the ring-full arm first, clear of the Loop wrap (the first version could not RED) |
| 6 | aec6285 docs | rendering.md "Video playback", pitfall NN, CLAUDE.md index (paid by rule 5's NOTE; 24,731 B), B2, tree, testing-eyes, effects.md |
App copies (for RED arms): scratchpad/video/apps/{c1,c2,c3,final}/Audio-DNA.app (c3 = commit 4 pre-rebase; final = rebased).

## probe-video: RED -> GREEN (final probe version; raw PY lines verbatim)
| app | run | PY line |
|---|---|---|
| main 5b78d43 (main checkout build) | red-main | `PY 14 PASS / 39 FAIL` |
| commit-1 app | red-c1 | `PY 26 PASS / 24 FAIL` |
| commit-2 app | red-c2 | `PY 30 PASS / 20 FAIL` |
| commit-4 app (pre-rebase) | c3-1 / green-1 / green-2 | `PY 49 PASS / 1 FAIL` / `PY 49 PASS / 1 FAIL` / `PY 48 PASS / 2 FAIL` |
| final (rebased) | final-green-1 / final-green-2 | `PY 48 PASS / 2 FAIL` / `PY 48 PASS / 2 FAIL` |

Per row (main -> c1 -> c2 -> final; values from the runs above):
| row | main | commit-1 | commit-2 | final |
|---|---|---|---|---|
| w5 gop30 guard | codes exact, dbox 0.72; late absent | PASS | PASS | PASS (dbox 0.72, late 0) |
| w1 4x1080p | 102.2 fps, p90 9.05 | 96.9 fps, 601 GL decodes, 1962 uploads | 102.7 fps, 601 decodes, uploads 602 | (a) 85.9 / 119.9 (see w1); (b) p90 6.51 / 3.71; (c) 0; (d) 604 / 602; (e) 0 |
| w3 retrigger 1080 | 46 polls ~62-66 ms, code 29 | 45 polls, 30 decodes/call | 46 polls, 30/call | 0 polls over; max 9.4 ms; late 30-31 (~255 ms hold); code in bracket |
| w3b retrigger 4K | 13 polls ~231-243 ms, capHold code 29 | 12 polls (max 286), code 29 | 13 polls, code 29 | 0 over; late 126-130 (~1050-1084 ms); capHold codes 245 / 244 (capPre 240: held or landed) |
| w4 deck return | 22 polls ~61-67 ms, code 29 | 22 polls | 22 polls | 0 over; pending 0; late 33-35; clock ran 5.24-5.25 s; code in bracket |
| w4b idle (V2) | fields absent | vacuous (no threads) FAIL | vacuous FAIL | B and A: awake 0, threads 1 (x2) |
| w6 crossfade | fields absent | uploads 542 | uploads 137 | uploads 138 / 137; blend dbox to A 115.6-117.6, to B 117.8-119.0; pending 0 |
| w6b retrigger mid-fade (V4) | 26 polls ~64-69 ms; code 541 after settle | 27 polls | 25 polls | 0 over; hold caps luma 127-128, codes 109 / 114; fade done 2.02-2.03 s; code has bit 9 |
| w8 ProRes | fields absent | 151 GL decodes | 151 | 120 fps, 0 decodes, late 0, code exact |
| w2 4x4K | 35.9 fps, p90 19.7 | 35.7 fps, 603 decodes, 715 uploads | 36.0, 603 decodes, 604 uploads | (a) 89.3 / 88.2 FAIL; (b) 3.83 / 5.47; (c) 0; (d) 600 / 602; (e) 0 |
| w7 X1 | RTT [10.9, 194.9, 243.4, 33.4, 205.7] | lock wait 221.8 ms, RTT to 227 | 224.3 ms, RTT to 230 | lock wait 0.0003 ms; RTT max 25.5 / 39.9 (load 9.8) |

## w2 (a): the 4 x 4K ceiling is not the render thread (diag w2a, commit-4 app, one hold, 4K canvas)
| scene | fps median | gpu_time_ms median | callback median |
|---|---|---|---|
| 1 / 2 / 4 x 4K STILL images | 120.0 / 120.0 / 110.1 (again: 108.2) | 4.56 / 5.36 / 7.14 | 0.84 / 0.89 / 1.11 |
| 1 / 2 / 4 x 4K video | 120.0 / 119.9 / 84.7 | 4.55 / 5.61 / 8.18 | 1.67 / 2.57 / 3.23 |
| 4 x 1080p video on the 4K canvas | 100.7 | 6.27 | 3.65 |
The same composite with no video sits at the 110 bar; 4 x 4K video adds ~1 ms of GPU time (4 x 30 x 33 MB uploads/s) and
lands at 84-90 fps while the callback p90 is 3.8-6.5 ms (w2 (b) PASS). Lever tried: BGRA upload
(`GL_BGRA / GL_UNSIGNED_INT_8_8_8_8_REV`, sws to BGRA; temporary env build, runs bgra-1/2): 82.2 / 82.8 fps vs 83.9 plain --
no gain (w5 frame accuracy PASSED with it). The plan's INFERRED ">= 110 with one upload per frame" does not hold on this
machine. Not re-thresholded. Remaining levers are outside this plan (PBO / IOSurface: rejected by renderleft; VideoToolbox:
out). Main: 35.9 fps -> lane 84-90 fps.

## w1 (a): bimodal measure on the same binary (runs w1x-1..5 on commit 4, flake-1..5 on final, diag w1ab)
Commit-4 app w1 fps across runs: 115.6, 116.0, 102.5, 120.0, 104.0, 120.0, 120.0, 120.0. Final app: 85.9, 119.9, 96.8,
110.0, 101.2, 94.2, 102.2. In one A/B hold without the probe's 15 ms poller (diag_w1.py, 50 ms polls): commit-4 119.6 /
119.7, final 120.0 / 119.7 (x2 each), main 90.2 / 87.9; 4 x 1080p stills 111.7-117.5 on all three. So the lane (pre- and
post-rebase) renders the w1 scene at 120 fps and main at ~89; the probe's w1 (a) reading swings between ~100 and ~120 run to
run on one binary. w1 (b)-(e) pass in every lane run. A flake verdict on the CODE is not justified from this; the
measurement needs Harmony's call (not re-thresholded).

## Hold lengths (V5: NONREF on vs off, 5 runs each; commit-4 app ON, temporary env build OFF)
| row | NONREF ON (shipped) late frames / hold | NONREF OFF late / hold |
|---|---|---|
| w3 1080p mid-GOP retrigger (150 frames past keyframe) | 30-31 / 252-259 ms | 33-35 / 283-318 ms |
| w3b 4K | 124-127 / 1033-1058 ms | 163-165 / 1358-1401 ms |
| w4 1080p deck return (away 3.5 s) | 33-35 / ~280 ms | -- |
Every correctness check passed in all 10 runs (w3 (d), w3b (d)(f)) -> NONREF stays ON (V5). Main: the whole output at
~15 / ~4 fps for ~3 s with the picture on keyframe + 29.

## Ctests, TSan, teeth
- ctest 848 (main 5b78d43) -> 862 (commit 3) -> 883 after the rebase (main's 869 + 14), `ctest -j1` 100 %.
- test_video_ring: 14 cases -- the plan's 11 + V1 (a GL release keeps "shown before": Held / Late, never Pending) + V2 (ring
  full, last draw 300 ms ago -> Park) + reverse play (frames the clock moved away from are freed; look-ahead-aware decide).
- TSan (separate build dir scratchpad/video/build-tsan, -DADNA_SANITIZE=thread, flags.make carries -fsanitize=thread):
  3 runs `All tests passed (105 assertions in 14 test cases)`, no ThreadSanitizer report.
- Teeth (mutated COPIES of VideoRing.h via -I shadowing; deliverable sha256 41b0051cf6b4f308 before = after):
  stale gen accepted -> `test_video_ring.cpp:305: FAILED: staleGenPicks.load() == 0 for: 217 == 0` (+ :57/:58/:59);
  older not freed -> `:44 FAILED ... 2 == 0`, `:306 orderViolations 74380`; acquire takes a Reading slot -> `:102 w != s0`,
  `:303 ownerViolations 38372`, `:304 tornReads 993`; tolerance sign -> `:122 FAILED: r.pick(clock, 1, tol).slot >= 0`;
  decide swapped -> `:154`, `:157`; judge Late at 0.5 fd -> `:179`; V1 release clears shown -> `:192 ... 3 == 1`;
  V2 never parks -> `:204`; no ahead-free -> `:221 aheadDropped 0 == 2`, `:223 acquireWrite() >= 0 for: -1 >= 0`.
- V2 live teeth (temporary env build, AUDIODNA_VIDEO_TEETH_NOPARK): `FAIL  w4b_idle_mid_catchup: B (ring full): 1.5 s after
  the deck left, video_threads_awake 1 == 0 (video_threads 1 == 1: not vacuous)` twice (teeth-nopark-3/4); lane app PASS x2.

## Regressions (final app, runs under scratchpad/video/reg)
probe-image-load `PY 37 PASS / 0 FAIL` GREEN; probe-crossfade `PY 35 PASS / 0 FAIL` GREEN; probe-deck-clock `PY 10 PASS / 0
FAIL` GREEN (d_video_keeps_time t1 - t0 4.38 s; d_imageseq_keeps_time PASS; d_return_hitch `peak 8.28 ms; REPORT -- target
<= 16`, main ~50 ms class); probe-render-state `PY 35 PASS / 0 FAIL`; probe-canvas `PY 15 PASS / 0 FAIL`; probe-fitmode
`PY 10 PASS / 0 FAIL`; probe-effects-parity `PY 46 PASS / 0 FAIL`; probe-outputs `PY 17 PASS / 0 FAIL` (no Output window:
`audio-dna windows 0, Output-named 0` after every batch).

## Byte identity, quit time, thumbnail, CPU
- BYTE-IDENTITY (test mode, render_frame on 7070): S-vid = a1080 at "speed": 0.0 (frame 0 held) and S-img = the same frame
  as a PNG: main == commit-4 and main == final `array_equal True`, max |diff| 0; array sha S-vid c73fbb3c51a9c9f8, S-img
  930194cb77888ce5 on all three; cap0 == cap1 on each (the negative-stride conversion + upload = the old flip, F14).
- Quit with 4 x 4K playing (osascript quit -> process gone): main 0.48 / 0.40 s, final 0.43 / 0.39 s (bar: lane adds <= 0.5 s).
- Thumbnail at open (the `[VideoPlayer] Opened ... thumb=` line, message thread): 4K 0.85-1.38 ms (diag measured 31.9 ms on
  the old path), 1080p ~0.34 ms.
- %cpu at 4 x 4K: lane 288-314 %, main / commit-1 / commit-2 269-289 %.

## Beyond the plan (found while building it; each has a ctest)
1. decide()'s "behind" (re-seek) threshold is max(0.1 s, (slots + 1) frames) in VideoPlayer: the writer runs up to 3 frames
   ahead, so the old fixed 0.1 s (from a decoder that ran at most 1 frame ahead) would re-seek FORWARD play of any < 30 fps
   clip after every third frame (3 frames of 24 fps = 0.125 s). Test: reverse case, `decide(0.0, 3.5 fd, fixed) == Reseek`.
2. pick() frees same-generation frames more than (slots + 1) frames AHEAD of the clock: with the plan's pick, reverse / ping-pong
   play fills the ring with frames the clock moves away from, the reader never frees them and the writer is locked out
   (reverse video would freeze). Forward play never reaches that distance (ring depth). The ring-full wait also returns
   when decide() says Reseek.
3. releaseGL() on a PAUSED open player bumps the generation (the thread re-decodes the current frame): after a context loss a
   paused clip would otherwise never pick a frame again. Plus: open()'s frame 0 is published with pts clamped to <= 0 (current
   from clock 0 even when a stream's first pts is a frame late); decodeNextFrame distinguishes EOF (drain) from a decode error
   (next packet); the Loop wrap / seek set a flag and the GL thread stores wantTime BEFORE bumping the generation (release),
   so a thread that sees the new generation sees its time.

## Deviations (rig and plan)
- Rig: one scratch compile command used `cd` into the scratch rig dir (step-0 sws check) -- a rule breach, no effect on any
  checkout. One lock hold ran 31 min (23:20-23:51): probe-canvas's perf rows waited out other lanes' compilers while holding.
- Probe design (new probe, not re-thresholding): the code band is drawn with drawbox per bit (the rig-verified decode of
  frames 0 / 5 / 37 / 59 was exact) instead of geq; b1080 is the NEGATED testsrc2 (so a crossfade midpoint is distinguishable
  from both clips); a first trigger plays from 0 (only a retrigger seeks to the in-point), so w3 / w3b trigger + retrigger
  ("prime"); a frame check accepts the code between expected(playhead before) - 2 and expected(playhead after) + 2 (a capture
  takes 50-400 ms); reset-on-read fields are maxed over the poller's reads; w7 interleaves a video retrigger before each image
  trigger (image triggers never take the video lock: main RTT 9 ms, commit-1 lock wait 1.44 ms with the plan's version);
  w4b gained the ring-full arm (the adopted mid-catch-up arm parks at the loop top on either build; it cannot RED) and runs it
  first, clear of the Loop wrap; fixtures can be cached ($VIDEO_FIXTURES).
- Temporary variant build (AUDIODNA_VIDEO_NONREF_OFF / _TEETH_NOPARK / _BGRA env switches) for the V5 OFF arm, the V2 teeth and
  the BGRA lever: built, copied to apps/var, the committed VideoPlayer.h/.cpp restored from saved copies (worktree blob ==
  HEAD blob: bc2b958e / 4c67f2bc), rebuilt, `strings | grep -c AUDIODNA_VIDEO_` = 0. Never committed.

## found_not_fixed
1. w2 (a) 4 x 4K fps 84-90 vs 110 (design finding above).
2. w1 (a) bimodal under the probe's 15 ms poller (above).
3. A video whose first frame never decodes stays PENDING forever: a crossfade onto it waits (C1) and render_frame's gate waits
   (was: a black upload). Edge case, same class as a sequence whose first frame fails.
4. After a GL context loss an on-screen, PLAYING clip returns texture 0 with pending false for the 1-4 frames until the next
   pick (V1 keeps it a hold, but there is no picture to hold): a clip WITH effects runs them FX-only for those frames. No live
   row possible (context loss = Output window open / resize).
5. Reverse / ping-pong on a long-GOP file shows correct frames at a low rate (a seek + catch-up per step); a GOP cache is filed
   (R-12). A Loop wrap costs 1-3 late frames.
6. RSS: 3 slots per drawn player (25 MB 1080p / 100 MB 4K); the idle trim (R-14) is filed, not built.

## Notes for .harmony/notebook.md (Harmony appends)
- A ring that decodes AHEAD needs a look-ahead-aware "behind" threshold and a reader that frees frames the clock moved away
  from; porting a synchronous decoder's thresholds (0.1 s) re-seeks forward play of < 30 fps clips | src/media/VideoPlayer.cpp decodeLoop, VideoRing.h pick
- A Loop wrap's generation bump also ends a ring-full wait: any live row about idle / park behaviour must keep its window clear
  of the clip's wrap or it cannot RED | .harmony/probe-video.py w4b
- A FIRST trigger of a video clip plays from 0; only a retrigger seeks to the in-point (MainComponent handleClipTrigger) --
  mid-clip probe rows must trigger + retrigger | src/MainComponent.cpp:4305-4329
- /api/state *_max_* / peak_* fields reset on read: a 15 ms poller consumes them, so a row must max over its polls |
  .harmony/probe-video.py pmax
- `ps -eo pid=,%cpu=,ucomm=` pads ucomm with trailing spaces on macOS: strip before comparing | .harmony/probe-video.py app_cpu
- Teeth for a pure header: compile the test against a mutated COPY placed first on the include path (-I scratch before -I src);
  the deliverable is never touched | scratchpad/video/teeth.py
- On this M1 Pro a 4K canvas with 4 x 4K layers composites at ~108-110 fps with STILLS; video adds ~1 ms GPU (uploads) -> ~85-90 |
  diag w2a

## Boris feel items
- "After a retrigger to a mid-clip in-point, a long-GOP (x264 default) video holds its last frame ~0.25 s at 1080p / ~1.05 s
  at 4K instead of freezing the whole show for ~3 s on a wrong frame."
- "Retriggering the incoming clip mid-crossfade holds its last shown frame ~0.25 s (1080p) while the fade keeps running."
- "Returning to a deck with a playing 1080p video holds its last frame ~0.3 s while it catches up."

## PACKET QUALITY
- Clarity: HAD_TO_INFER -- the plan's w3/w3b "capPre ~240" assumed a first trigger lands on the in-point; w7 and w4b as
  written could not RED; the decide() threshold and pick() did not account for the look-ahead. Each resolved in the lane with
  evidence (above), none a redesign.
- Missing context: first-trigger semantics (inPoint only on retrigger); main moved twice during the lane (seqvram merge,
  then docs).
- Unused context: plan section 5 (part-M seam) needed no action beyond installVideoPlayer.
- Self-brief files: plan-video.md (+ adoption) and both attack papers read in full; lock.sh; probe-image-load / deck-clock
  scaffolding; gate-scripts/final.sh for the regression list. .harmony/notebook.md not read (size guard; not needed).
- Knowledge tools: none in the packet; grep-only, conservative (no deletions on "no callers").
