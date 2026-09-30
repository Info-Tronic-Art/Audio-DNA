# BUILDER REPORT -- gopcache-fix (s-rta-0929b, fix round 1 of lane/gopcache)

STATUS: PARTIAL

INBOX-RECHECK: none

## RESULT
All code findings are fixed, each with a RED run on a02c93c and a GREEN run on HEAD. Every live gate is still not run,
because a system dialog was already on screen when this round started. It is a TCC prompt: "Audio-DNA would like to
access the microphone" (UserNotificationCenter pid 98807, ordered front at 02:36:23). The rig rule and the 2026-09-30
notebook entry both say to STOP, and that the prompt is Boris's to answer. I did not touch it. The four live MUSTs
(F3 forward battery, F4 GC13 n >= 5, F5 GC9 u11, F6 GC11 client / malloc + GC7 256 MB) therefore stay open. The
scripts are ready: `scratchpad/gopcache-fix/live.sh fwd ab7 ab256 abw tier1`.

- **F1 (MUST, verified, fixed)**: GC3 holes now apply to every run kind, and a run whose keyframe lands above its
  window re-seeks further back. Laps 2-3 of a reverse Loop went from +130 / +236 seeks to <= 1.
- **F2 (MUST, verified, fixed)**: the forward "never an older frame" guard now applies only while cache hits are in
  play. The pts-less AVI golden trace, recorded on main's writer, now matches: threaded and stepped runs both give
  "decoded 128 dropped 14 seeks 3". a02c93c gave "dropped 54".
- **SHOULDs**: GC9 doc overclaim (fixed in the doc), threaded reverse + TSan, GC10 tooth, keyframe-gate tooth, u4b
  forward-retention gate, doc launch counts -- all done. Nothing is left undone except the live re-runs.
- Full ctest -j1: 1011 / 1011 passed. TSan: 0 reports across 4 test targets.
- **Found while fixing, not fixed**:
  - Reverse on a pts-less AVI shows keyframe pictures. Verified. Present in the lane since c3.
  - A VFR file shows fewer distinct frames in reverse than on main. Measured.
  See ISSUES.

## FACTS
- Worktree `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929b-gopcache`, branch lane/gopcache,
  continued from a02c93c. Fix commits:
  - c5b1496 fix: F1, GC3 holes for every run kind + overshoot re-seek + prefetch block by target (+ ctest, 2 fixtures)
  - 3ac8388 fix: F2, the forward guard only while hits are in play (+ AVI golden trace threaded / stepped, fixture)
  - 4b06acd test: the GC3 keyframe gate's tooth (MPEG-TS fixture)
  - 46a2304 test: GC10's tooth (the slot is its own page mapping, unmapped when freed)
  - 63df7c5 test: the threaded reverse / flip / turn case (TSan-run)
  - 9009d9b test: probe-vupload u4b (f) / (g), forward retention bounded and dropped by the trim
  - 88b3868 docs: rendering.md / pitfalls NN
  - the commit carrying this report (+ "Fix round" appended to gopcache.md)
- FIX app: build-lane/AudioDNA_artefacts/Release/Audio-DNA.app built at HEAD's code, sha256 821670d2bcb4e702
  (codesign Authority "Audio-DNA Dev", the same designated requirement as the main build). A ditto copy (signature
  intact, not re-signed; codesign -v valid) is at scratchpad/gopcache-fix/apps/fix.app for the live batches. An earlier
  03:32 build of the same source was 9d6de0e2575c8113: same __text size 8,888,816; the signature time differs. Main
  (pre-change) = 20b9b48ffc29367d, unchanged.
- ctest --test-dir build-lane -j1 (scratchpad/gopcache-fix/logs/ctest-final.log):
  `100% tests passed, 0 tests failed out of 1011`, `Total Test time (real) =  50.39 sec` (03:47). Was 1005: +6
  cases (GC3 holes, GC3 keyframe gate, GC10 mapping, AVI golden x 2, threaded reverse).
- TSan (ADNA_SANITIZE=thread, scratch build scratchpad/gopcache/build-tsan-gc rebuilt from this tree;
  TSAN_OPTIONS halt_on_error=0:abort_on_error=0; logs scratchpad/gopcache-fix/tsan/):
  - `test_video_decode_trace rc 0 :: All tests passed (510 assertions in 5 test cases) :: tsan reports: 0`
  - `test_gop_cache_store rc 0 :: All tests passed (499 assertions in 20 test cases) :: tsan reports: 0`
  - `test_gop_cache rc 0 :: All tests passed (111 assertions in 11 test cases) :: tsan reports: 0`
  - `test_video_ring rc 0 :: All tests passed (683 assertions in 31 test cases) :: tsan reports: 0`

### Findings, one by one (verdict -> action -> evidence)

**F1 MUST: GC3 holes only for DEMAND runs -> VERIFIED, FIXED (c5b1496)**
- Verified by execution. On a02c93c the new ctest ("GC3 holes", reverse Loop, whole file in budget, laps 2-3 after one
  lap) fails. RED on a02c93c (scratchpad/gopcache-fix/rev.py: a02c93c's src/media + the new test TU):
  - `test cases:  20 |  19 passed | 1 failed`, `assertions: 499 | 493 passed | 6 failed` (the final test file)
  - video_h264_vfrgap_64x64.mp4: `CHECK( st.seeks.load() - seeks1 <= 1 )  130 <= 1`,
    `CHECK( st.gopCacheRuns.load() - runs1 <= 1 )  126 <= 1`, `CHECK( st.gopCacheMisses.load() - misses1 <= 1 )  4 <= 1`
  - video_mpeg4_bf2_64x64.mp4: `236 <= 1` (seeks), `236 <= 1` (runs), `CHECK( missing.empty() )` with
    `missing := "118 "`
- Two causes, both confirmed by trace (scratch-instrumented copies):
  1. A PREFETCH passing an index the decoder never outputs marked nothing, so the index stayed the target. VFR: target
     63, output 61 -> 64, repeated after every served frame.
  2. The MPEG-4 file's frame 118 was WRONGLY written off as a hole. Seeking to frame 118 lands on keyframe 119: the
     index is DTS, and I119's dts is frame 117. So the DEMAND run "passed" 118 and marked it a hole. That hole then
     became the permanent prefetch target, because holes were not counted as covered. The same landing happens in
     h264 closed-GOP files for the frames just below an IDR (INFERRED from the packet table: K pts 10, dts 8).
- Fix:
  - `onRunFrame` marks every index between two consecutive post-keyframe outputs with a real pts, inside the window, as a
    hole. This covers every run kind. A pts-less frame breaks the chain.
  - `GopCache::covered()`: planPrefetch and unservedAhead walk over holes. kHole moved to GopCache.h.
  - A run whose first output lands above its seekFrom re-seeks further back (up to kMaxRunOvershoots = 3; the EOF drain
    honours it).
  - A PREFETCH that stored nothing blocks its target, not the served frame. clearCache resets the block.
- GREEN: `All tests passed (499 assertions in 20 test cases)` (test_gop_cache_store, final). Laps 2-3: vfrgap seeks
  3 -> 3, mpeg4 no false hole, identity 0 mismatches.
- Teeth (scratch mutants of the final source):
  - overshoot re-seek off -> `CHECK( missing.empty() )` fails (`missing := "118 "`)
  - gap rule off -> `7 <= 1` seeks, `3 <= 1` runs, `4 <= 1` misses
  - the target-keyed block alone has no tooth: a served-keyed mutant passes. The hole fix removes the storm by itself,
    so the block is defensive (ISSUES).

**F2 MUST: the forward guard was not inert for pts-less streams -> VERIFIED, FIXED (3ac8388)**
- Verified: the guard `haveNewest_ && pts <= newestPts_ + 0.5 fd` ran in plain forward play, and a pts-less frame takes
  the clock's time.
- Golden trace, same schedule as the GOP-30 trace, on tests/fixtures/video_mpeg4_bf2_64x64.avi (`ffmpeg -f lavfi -i
  testsrc2=s=64x64:r=30:d=4 -c:v mpeg4 -g 30 -bf 2 -pix_fmt yuv420p -q:v 8 -threads 1`; every third frame N/A pts).
  - RECORDED on e13dccb's writer (main's writer + the c0 counters) via GOLDEN_TRACE_OUT.
  - 6 / 6 runs identical (sha b26ed02ca55eceae).
  - The same harness on the GOP-30 fixture reproduced the committed golden exactly (harness check).
- RED on a02c93c: `test cases:   5 |   3 passed | 2 failed`; `counters: decoded 130 dropped 54 seeks 3`, against the
  golden `decoded 128 dropped 14 seeks 3`, threaded and stepped.
- GREEN: `All tests passed (510 assertions in 5 test cases)` (test_video_decode_trace), 5 / 5 runs.

**F3 MUST: forward battery never run -> VERIFIED (not run in round 1), NOT FIXED: BLOCKED (dialog)**
- The scripted battery is `live.sh fwd`: w10 refs on main, w10-all, probe-video all rows, vupload u2 / u4a / u4b / u6 /
  u12, u12 client / malloc, crossfade / media-open / async-load / seq-vram. It also needs `live.sh abw` (w1c / w2c /
  w6b 5 x 2 interleaved vs main).
- Offline INFERRED cost of GC8's copy (scratch micro-benchmark, GopCache::Store::store, cache-hot source, 3 runs):
  - steady Replace 0.090-0.095 ms per 1080p frame, 0.281-0.288 ms per 4K frame
  - new slot (first touch) 0.28 / 1.10-1.13 ms
  - 4 x 4K at 30 fps -> ~34 ms/s of decode-thread CPU across 4 threads, about 2 % of their software decode
    (14.2 ms / 4K frame per vupload's measurement)
  - Unmeasured live.

**F4 MUST: GC13 (n = 4 lane launches) -> VERIFIED from ab.tsv, NOT FIXED: BLOCKED (dialog)**
- The earlier hung 5th launch lines up with the TCC mechanism in the notebook entry. Its err.log has "CoreAudio error:
  10004003", and the notebook describes a hang with the same error. That attribution is INFERRED; there is no re-run.
  `live.sh ab7` runs u7 + u8-u12 at 5 x 2.

**F5 MUST: GC9 fails (261 vs 248 ms) -> VERIFIED from the data (n 4 vs 5; inside A's spread), NOT FIXED: BLOCKED**
- The kDemandWindow lever was NOT applied. There is no live measurement to warrant it, and the bar is Harmony's.
- Note: F1's overshoot re-seek can lengthen a DEMAND whose target sits just below a keyframe. It trades one GOP of
  decode for a correct frame, so u11 must be re-measured on this binary in any case.

**F6 MUST: GC11 client / malloc + w10-all, GC7 256 MB -> VERIFIED not run, NOT FIXED: BLOCKED (dialog)**
- Scripted: `live.sh fwd` (u12 client / malloc, w10all) and `live.sh ab256` (u8, ENV_B=ADNA_GOPCACHE_BUDGET_MB=256,
  5 x 2).

**F7 / F13 SHOULD: "one PREFETCH at a time" overclaims the code -> VERIFIED, FIXED in the doc (88b3868); code unchanged**
- rendering.md now states the urgent bypass: after a deck return every player of a column is under the line and all
  prefetch at once.
- I did not narrow the code. The "urgent" line (GOP x decode ms / frame ms x 1.2 resident frames ahead) already IS
  "would run dry within one GOP of decode", which is about one DEMAND latency. A narrower line would starve players
  into DEMAND holds. That is reasoning only (INFERRED); u11 x 5 is blocked.

**F8 SHOULD: no threaded reverse test -> FIXED (63df7c5)**
- The new case runs the real decode thread through flips, turns, seeks, 2x and a mode switch, in real time.
- 160 frames shown in 5 / 5 runs, 0 violations. Tooth: no direction bump -> `violations := 19`.
- TSan: 0 reports.

**F9 SHOULD: stale measurements, doc launch counts -> doc FIXED (88b3868); runs BLOCKED**
- "5 x 2 interleaved" now reads "main 5 / lane 4 ... GC13 not yet met; numbers are the pre-fix lane build".

**F10 SHOULD: GC10 ctest cannot tell mmap from malloc -> VERIFIED, FIXED (46a2304)**
- The new case asserts that each slot is one page-aligned whole-page mapping, and that the mapping is gone after
  freeSlot / clear (msync -1 / ENOMEM).
- The malloc mutant (scratch GopCacheStore.h with av_frame_get_buffer): `test cases:  2 |  0 passed |  2 failed`,
  18 assertions (e.g. `f->buf[0]->size % page == 0` -> `6400 == 0`; `errno == 12` -> `22 == 12`).

**F11 SHOULD: keyframe gate has no tooth -> VERIFIED (dormant on the h264 fixtures), FIXED (4b06acd)**
- An MPEG-TS file (`-c:v mpeg4 -g 30 -bf 0`, .ts, starts at 1.4 s) seeks onto non-keyframes. The decoder outputs
  predicted garbage (key 0, not flagged corrupt).
- HEAD: 0 bad of >= 20 resident frames. m7 (no runSawKey_): `CHECK( bad == 0 )  29 == 0` (checked 60).

**F12 SHOULD: forward retention has no bound in any live row -> FIXED in the probe (9009d9b); run BLOCKED**
- u4b (f): frames <= 4 x fwdRetainMaxFrames (20) at s0, absent = FAIL on main. u4b (g): 0 after the trim.
- `live.sh fwd` runs it on FIX and on main (RED).

## METHOD
- Verified each finding against a02c93c before touching code. F1 was checked by a new ctest run RED on the a02c93c
  tree plus a scratch-instrumented trace; F2 by a golden trace recorded on main's writer and run RED on a02c93c.
- New scratch tools (scratchpad/gopcache-fix/):
  - mut.py: a patched VideoPlayer.cpp linked with the target's other objects
  - rev.py: the test TU + VideoPlayer.cpp compiled against src/media/* at a git revision, optionally patched
  - hmut.py: a patched GopCacheStore.h
- Commits were staged per plan item from saved final files (stage.py); each stage was built and its tests run before
  its commit.
- Live: `live.sh` is written (my lock name gopcache-fix, Bluetooth check, on-screen dialog STOP without dismissal,
  .venv symlink removed at the end) and was never started. The dialog was watched from 03:35 to 04:05
  (`waitdialog.sh`, 30 s polls of on-screen UserNotificationCenter windows). Last line:
  `04:05:14 dialog still on screen after 9 min` -- the same pid 98807 / window 29885 throughout.

## CONFIDENCE / VERIFY
- High: F1 and F2 (RED on a02c93c, GREEN on HEAD, teeth); F8 / F10 / F11 (teeth); ctest 1011 / 1011; TSan 0.
- Not verified: every live gate. Harmony: once Boris has answered the microphone prompt, run
  `bash scratchpad/gopcache-fix/live.sh fwd ab7 ab256 abw tier1` (about 2.5-3 h of rig time), or the equivalent.

## UNKNOWNS / NOT DONE
- Live, all BLOCKED by the dialog: the forward battery (F3), u7 + u8-u12 5 x 2 (F4, GC9 u11 = F5), u8 256 MB x 5 and
  u12 client / malloc + w10-all (F6), w1c / w2c / w6b 5 x 2, u4b (f) / (g) RED + GREEN, Tier-1.
- The kDemandWindow lever (F5): not applied, not measured.
- APP-INVENTORY (outside the fence): +6 ctests, 1011 total; 110 Catch2 targets, unchanged.

## NUANCE
- Deviation, F1 (a strict improvement on the reviewer's recipe): the gap rule plus "holes count as covered" (the
  reviewer's fix) would have kept the MPEG-4 file's frame 118 as a FALSE hole, skipped in every reverse lap. The
  overshoot re-seek recovers it. Tooth: turning it off fails `missing.empty()`.
- The keyframe-gate test asserts identity of the RESIDENT cache only. What a DEMAND run publishes before its keyframe on
  a TS file is the decoder's raw output, and main's forward seek shows the same (INFERRED from main's writer; ISSUES).
- The "GC3 holes" test's shown-count bar is a never-frozen floor (>= 40 over two laps). The VFR file shows ~25 of its
  85 frames per lap in reverse, before and after the fix (ISSUES).
- forwardDecode (test reference) now keeps the FIRST frame at a colliding index, as the cache does, and uses the
  player's frame duration fallback (r_frame_rate for TS).

## HANDOFF-NEEDS
- **The dialog**: UserNotificationCenter pid 98807, window 29885, 260x234: "Audio-DNA would like to access the
  microphone" [Don't Allow] [Allow]. Ordered front 02:36:23, before this round. It is Boris's to answer (notebook
  2026-09-30). A window-only capture by Quartz id is at scratchpad/gopcache-fix/unc-98807.png.
- **Rig-rule breach to report**: one command at ~02:59 began `cd /private/tmp && python3 .../mut.py ...` (my first
  mutant run). No file outside the scratchpad / worktree was touched; later runs have no cd.
- **Boris checks** (after the live gates): reverse / ping-pong on a real long-GOP clip; a DivX-style AVI in reverse
  (ISSUES 1).

## ISSUES (found, not fixed)
1. **Reverse of a pts-less AVI shows keyframe pictures** (VERIFIED by trace, present since c3; not this round's change).
   - Every run's first output after its seek is the pts-less keyframe. onRunFrame gives it
     `ptsOfRel(r.target)`, so it is stored at the run's TARGET index. Trace lines:
     `DBG runframe kind 2 rel 116 hasPts 0 key 1 seekFrom 0 target 116`, then 115, 114, ... one per run.
   - Threaded harness, 2 laps of the 120-frame AVI: FIX 120 seeks / 120 decodes; main 73-77 seeks / 833-916 decodes.
   - A scratch identity run: 372 mismatches. The reference is itself unreliable here: best_effort_timestamp is off by
     one and keyframes have no pts.
   - Direction: index a pts-less run frame by the run's output chain (previous index + 1), and never store or publish
     one whose index is unknown.
2. **VFR reverse shows fewer distinct frames than main** (VERIFIED, threaded harness, 3 runs each, 2 laps of
   video_h264_vfrgap_64x64.mp4):
   - main: distinct 64 / 65 / 62, late 290 / 298 / 289 of 960, decoded 800 / 812 / 777, seeks 91 / 89 / 88
   - FIX: distinct 48 / 41 / 42, late 297 / 352 / 325, decoded 91, seeks 3
   - INFERRED cause: the relative index is the AVERAGE frame duration (47 ms against 33 ms real spacing), so frames
     collide on an index (the cache keeps one) and the step machine walks indices, not pts.
3. **MPEG-TS (no keyframe index)** (VERIFIED in the ctest): seeks land on non-keyframes. A DEMAND publishes the
   decoder's pre-key output, and pre-key frames are never stored, so the file keeps re-running (39 seeks in 3 s of
   reverse). Main's forward seek has the same landing (INFERRED).
4. The prefetch block keyed to the target has no tooth (defensive).
5. Overshoot re-seek cost: a DEMAND just below a keyframe now decodes the previous GOP instead of showing a stand-in.
   The latency effect on u11 is unmeasured.

## PACKET QUALITY
- Clarity: CLEAR (findings with recipes).
- Missing context: the pending TCC dialog on screen at the start, and the notebook rule that it is Boris's to answer.
- Unused context: none.
- Self-brief files: gopcache.md (lane report), the rig lock helper, notebook 2026-09-30 TCC entry -- useful.

## Notebook notes (for Harmony to append to .harmony/notebook.md)
- A keyframe seek (AVSEEK_FLAG_BACKWARD on a DTS index) can land ABOVE the frame asked for in a B-frame stream: an MPEG-4
  part 2 file's last GOP (I119 dts = frame 117), h264 closed-GOP frames just below an IDR. A decode run must compare its
  first output with the frame it sought and seek further back, never write the skipped frames off as "never output" |
  src/media/VideoPlayer.cpp onRunFrame (runLanded_ / kMaxRunOvershoots).
- MPEG-TS has no keyframe index: av_seek_frame lands on the exact non-key frame, and the mpeg4 decoder then outputs
  predicted frames (key 0, no corrupt flag). Only a "stored after the run's keyframe" gate keeps them out of a cache |
  tests/test_gop_cache_store.cpp "GC3 keyframe gate".
- DivX-style AVI (mpeg4 -bf 2): every third frame has pts N/A (keyframes included), and best_effort_timestamp is one
  frame off. A pts-less frame has no reliable index | tests/fixtures/video_mpeg4_bf2_64x64.avi.
- Recording or RED-running a test on another revision: extract src/media/* at the rev into a scratch dir that shadows
  src/, then compile the test TU AND VideoPlayer.cpp against it. Linking an old VideoPlayer.o to a new test object is an
  ABI mismatch (the header grew members) | scratchpad rev.py.
- msync(addr, len, MS_ASYNC) returns -1 / ENOMEM exactly when a page-aligned range is unmapped (EINVAL when unaligned):
  a deterministic "memory went back" assertion where phys_footprint deltas depend on malloc's caching |
  tests/test_gop_cache_store.cpp GC10 mapping case.
