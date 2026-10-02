# PLAN lane mkvidx (s-rta-1002b) -- Matroska index, intra verdict, frame-seek aim

Author: Architect (Fable). Base: main 5e47d17 (src identical to 5b507b1: `git diff --stat 5b507b1 5e47d17` touches only
.harmony/). Evidence rigs (scratch, rebuildable): `$S = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/
73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/mkvidx` -- idx.c / idx2.c (index probes), mkvcost.cpp (the REAL VideoPlayer
under gop2's emulated decoder), proto3/ + proto3.diff (the full prototype), test_mkvidx_scratch.cpp (the proposed tests),
fxb1/ (the bitexact fixtures), m1/ m4/ m5/ proto2 (mutants), run*.log, tr_ms.txt / tr_pp.txt (traces).

VERDICT: FIX (not do-nothing). The ledger's premise is right but its cost estimate ("extra decodes bounded, no stall") is
wrong in both directions: the same mechanism FREEZES reverse for every Matroska file whose Cues sit at the front, and the
1-entry index costs 1.3x-7x the decodes of the same stream in MP4 at multi-layer shares. The ledger's own fix candidate
("refresh after the first seek") is a trap: done naively it re-runs the intra-only test on a keyframes-only index and
freezes EVERY Matroska file (mutant m5). A third, pre-existing bug in the same seek path (a truncated frame seek lands one
GOP low) hurts HAP / ProRes / MJPEG reverse today and would surface on Matroska once the index is fixed; it is item 3.

## 1. GOAL

A Matroska (.mkv / .webm) file reverses and ping-pongs exactly like the same stream in an MP4 -- its keyframe model is the
demuxer's live index, its intra verdict cannot mistake a keyframes-only index for an all-intra stream -- and a seek for
"the keyframe at or before frame k" lands at k's keyframe whatever the container's timestamp rounding.

## 2. ESTABLISHED FACTS

Code (all VERIFIED by reading at 5e47d17):
- C1 open() reads the index ONCE: VideoPlayer.cpp:277-304 (loop :281-292; `entries >= 2 && keys == entries` ->
  intraOnly_ :294-295; keyRels_ relative to the first keyframe :298-300; gopFramesEst_ = longest interval :301-302, or
  totalFrames_ when one keyframe :303-304). It runs AFTER open()'s first decodeNextFrame (:250). Nothing re-reads it.
- C2 every reader of intraOnly_ / gopFramesEst_ / keyRels_ is decode-thread code (or open() before start()): :842, :896,
  :1063-1070 (forwardRetain), :1253, :1290, :1306, :1314, :1318, :1327, :1440, :1475, :1508, :1541; VideoPlayer.h:279-284
  says so. A decode-thread rewrite needs no atomic.
- C3 the uses a wrong index distorts: planPrefetchRun's Lead (GopCache.h:393-399, used at :436-437 -- keyAtOrBefore
  :382-387 returns keys[0] = 0 for every r when keys = {0}, so the lead-in = the whole file and avail = the whole Future
  share), prefetchAt (:328-332) and the "urgent" line (VideoPlayer.cpp:1327: gop x decode / frame x 1.2 -- huge, so the
  GC9 one-PREFETCH token is never waited for), PingPong retention (VideoPlayer.cpp:1070 -> GopCache.h:460-465: the cap),
  the decode-through thresholds after hits (:896, :1541: never seek), the overshoot seek-back (:1440: a whole-file back).
- C4 the intra DEMAND run: startDemand sets windowLo = rel + 1 (:1290-1293); reverseStep keeps a run only while `next >=
  r.windowLo` (:1265-1266) -- so an intra run that does not land ON its frame is restarted (re-sought) every step.
- C5 seekToTimestamp truncates: `static_cast<int64_t>(timeSec / timeBase_)` (:1602), BACKWARD (:1605-1606). Frame-index
  callers pass the nominal time ptsOfRel(rel) (:1018-1021): runStep :1352, forwardStep :898, forwardIdle :1543. Clock-time
  callers pass the clock: decodeStep :853, forwardStep :882 / :887.
- C6 advanceTransport returns at once when duration_ <= 0 (:442-443): a duration-less file's clock never runs.
- C7 open() runs on the message thread or MediaOpener's pool (VideoPlayer.h:40-43; call sites MainComponent.cpp:5271,
  :7008, MediaOpener.cpp:34).

Mechanism (VERIFIED by experiment, FFmpeg 8.0 / libavformat 62.3.100; the libavformat source is not on this machine, so
the internal cause -- matroskadec defers Cues parsing to its first read_seek and adds each keyframe it reads to the
index -- is INFERRED from FFmpeg knowledge and consistent with every observation):
- M1 `$S/idx.c` (replicates open()): ffmpeg-muxed MKV / WebM (Cues at the end, the default), 600 frames GOP 250: 1 index
  entry after find_stream_info AND after open()'s decode -> keyRels {0}, gop = totalFrames (600); after ONE av_seek_frame:
  3 entries {0, 250, 500}. Same for VP9 WebM, GOP 30 / 60 MKV (1 -> 20 / 30 entries), GOP 5 / 10 / 15 / 30 at 360p (1 at
  open). MP4 / MOV / fragmented MP4 (seekable) / AVI: the full per-sample index at open, unchanged by a seek.
- M2 `$S/idx2.c`: plain forward reading (no seek) grows a Matroska index by one entry per keyframe read (g250 1800-frame:
  1 -> 2 / 3 / 5 / 8 at 300 / 600 / 1200 / EOF packets). Cue-less (piped, -live) files: the same, and a seek reads ahead to
  the keyframe after its target.
- M3 Cues at the FRONT (-cues_to_front 1; INFERRED: also typical of DASH-style WebM downloads): the full index at open --
  and EVERY entry is a keyframe (Cues list keyframes only), so C1's rule sets intraOnly_ for a GOP-250 stream.
- M4 duration-less Matroska (piped / -live; INFERRED: browser MediaRecorder WebM): totalFrames 0 -> C6: a still frame.
  Not this lane (FILED F3).

Cost today (VERIFIED: the real VideoPlayer, gop2's Emulated harness -- K FFmpeg decodes per 120 Hz render frame, decode ms
pinned to 8.333 / K; live 1080p is 1.76 ms = K 4.7, ruling-gop2 :122; K 3 = 1.6x slower (contention); 64x64 fixtures, the
decode COUNTS are portable -- gop2 notebook; INFERRED: portable across resolution for the same GOP / B-frame layout.
`$S/run1..4.log`, run9 / run11 / g10.sh). Share in frames: 21 = four 1080p players in 256 MB, 164 = four in 2 GiB (Boris's
32 GB M1 Pro budget), 690 = one in 2 GiB.
- X1 Cues at the front, reverse Loop: FROZEN. GOP 250 300-frame, CAP 21 K 3, 9 s: MP4 late 0 / shown 268 / decodes 2186;
  MKV front late 875 / shown 1 / decodes 3240 = seeks 3240 (a seek + decode every writer step). 1800-frame g250 / g60 /
  VP9-WebM, 30 s: shown 3 / 15 / 3 of ~900, 10,800 seeks.
- X2 Cues at the end, reverse Loop, 30 s, decodes MP4 -> MKV (late MP4 / MKV):
  | stream | CAP 21 K3 | CAP 21 K5 | CAP 164 K3 | CAP 164 K5 | CAP 690 K3 | CAP 690 K5 |
  |---|---|---|---|---|---|---|
  | g250 1800f | 7720 -> 10142 (0/0) | 8106 -> 16111 (0/0) | 1873 -> 9479 (17/17) | 1945 -> 3535 (0/0) | 1550 -> 1702 (176/177) | 1575 -> 1725 (82/83) |
  | g60 1800f | 2672 -> 9538 (0/0) | 2784 -> 14657 (0/0) | 1316 -> 9171 (0/0) | 1331 -> 2466 (0/0) | 1390 -> 1470 (161/163) | 1416 -> 1491 (74/75) |
  | scene-cut 300f, 9 s | 910 -> 2141 (0/0) | 929 -> 2094 (0/0) | | | | |
  | g5 / g10 360p, 5 s | 221 -> 1374 / 296 -> 1385 (0/0) | | | | | |
  Per shown reverse frame at CAP 164 K 3: g250 2.1 -> 10.6 decodes, g60 1.5 -> 10.2. Decoder duty (decodes x t_d / wall)
  at CAP 164 K 3: MP4 12-17 % of a decode thread, MKV 85-88 %. Late: equal or LOWER for MKV (its over-eager prefetch).
- X3 PingPong 130 s (one forward leg, the top turn, one reverse leg), decodes MP4 -> MKV: g250 CAP 21 16682 -> 21352,
  CAP 164 5136 -> 12533, CAP 690 3941 -> 4011; g60 7298 -> 20070 / 4182 -> 11594 / 3674 -> 3877. Late: MKV 0 everywhere;
  MP4 49 (g250) / 136 (g60) at CAP 690 K 3, 27 / 53 at K 5 -- ONE hold ~15 s into the reverse leg (trace `$S/tr_pp.txt`:
  the first PREFETCH after the turn is target 1771 lo 1106, 729 decodes bottom-up under a 28-frame cover). HEAD's MKV
  avoids it only because its stale gop makes PingPong retain the cap (GopCache.h:460-465). By clip length (CAP 690, K 3-5,
  `$S/len.sh`): 20 s clips 0, 40 s 15-29, 60 s 27-136 render frames (120 Hz).
- X4 Flip forward after 8 s of reverse: MKV +11-130 % decodes after the flip (g60 CAP 164: 285 vs 124), late 0-2 both.
- X5 Seek precision (C5), HEAD, intra-only files reversed 9 s at CAP 21 K 3 (300 frames): HAP time base 1/600: late 316,
  shown 142 of 270, decodes 2797; HAP or ProRes at FFmpeg's 1/15360: late 16, shown 262, decodes 481; ProRes 29.97 at
  1/30000: late 0, 270, 272; MJPEG / all-intra H.264 in MKV (1 ms): late 68, shown 163, decodes 3240; a VFR all-intra MJPEG
  (1 of 7 dropped): late 275, shown 77 in 6 s. Long GOP: an MP4 remuxed from MKV (ms timestamps) vs the exact-timestamp MP4,
  g250 CAP 43 K 1: late 1222 vs 682, longest hold 697 vs 73 render frames (trace `$S/tr_ms.txt`: seekFrom 1250, the
  keyframe stored at 666,672 ticks, the target truncated to 666,666 -> landed on 1000 -> a DEMAND livelock).
- X6 VP9 (no droppable frames in a lead-in) is later than H.264 at a small share in MP4 already; HEAD's MKV hides part of
  it: VP9 CAP 21 K 3 late WebM 56 vs MP4 156 (decodes 10353 vs 8627); CAP 164 37 = 37 (9588 vs 2407).

DO-NOTHING bar (pre-registered): do nothing iff, against the same stream in MP4, Matroska shows (i) no freeze or extra
hold and (ii) <= 1.25x the decodes at shares 21-690 and K 3-5. X1 fails (i); X2 / X3 fail (ii) by up to 7x. -> FIX.

Prototype (VERIFIED: `$S/proto3.diff`, 82 changed lines): with items 1-3, every Matroska row of X1-X4 equals the same
stream in MP4 count for count (X1 front g250: 2186 decodes = MP4; X2 g60 CAP 164 K 3: 9171 -> 1316 = MP4; scene-cut
2141 -> 910 = MP4); MP4 / MOV rows are identical to HEAD; VP9 WebM = VP9 MP4; X5 intra rows: late 0, shown 270, decodes
273 (VFR: 154 / 157); X5 long-GOP ms row: 682 / 73 = the exact MP4. The committed suites pass unchanged against it with
identical printed values: test_gop_cache_store 26 cases / 662 assertions (gop2 T1b-T3 lines identical),
test_video_decode_trace 510 / 5, test_video_player_open 10 / 2, test_gop_cache 148 / 13.

## 3. DESIGN FORKS

Fork A -- fix or not. DO-NOTHING loses on X1 (a freeze is a defect, not a cost) and X2 (5-7x decode CPU in Boris's
4-layer case). Chosen: fix.

Fork B -- the intra-only verdict from an index.
- B1 [CHOSEN] every entry a keyframe AND the longest keyframe interval one frame (`gopFrames == 1`): duration-free,
  container-free, conservative (a wrong "not intra" only costs cache memory; a wrong "intra" froze reverse, X1).
- B2 density: every entry a keyframe AND keys >= 0.9 x totalFrames. Zero change for MP4 (also keeps VFR all-intra MP4 on
  the intra path) but depends on the duration metadata. Runner-up; loses because B1's only behaviour change outside
  Matroska is a VFR all-intra non-codec-intra MP4 going to the cache path, measured harmless (late 0, 171 vs 157 decodes
  per 6 s, `$S/vfr.sh`), and B1 needs nothing outside the index.
- B3 container name ("matroska,webm" -> skip the all-key rule). Rejected: a name list rots; B1 states the property.
- B4 drop index-derived intra entirely (codec list only). Rejected: x264 keyint=1 / all-intra AV1 lose the intra path.

Fork C -- when the keyframe model is (re)built.
- C1 [CHOSEN] at the top of every decodeStep, rebuilt only when the index entry count changed (one O(1) accessor call per
  step). Catches the Cues load at the first seek AND the per-keyframe growth during forward play (M2), so MKV == MP4 from
  the second keyframe on, PingPong legs included.
- C2 in seekToTimestamp only (the ledger's candidate; prototype proto2). Runner-up; loses: a PingPong first leg (no seek)
  keeps the stale model (mutant m3 fails T2b), so the first top turn behaves unlike every later one.
- C3 force the Cues parse in open() (a seek before the first decode). Rejected: I/O on the message thread for drop-opens
  (C7), a cue-less file reads a GOP of packets, and it changes open()'s first-frame path that test_video_player_open and
  the thumbnail pin.
- C4 treat a < 2-key index as "no index" (grid, kDefaultGopFrames). Rejected: half a fix -- wrong on any GOP != 250 and
  blind to the real keys the demuxer has after one seek.
- In every option intraOnly_ stays an open()-time verdict (GC5 "never learned from runs"); re-deriving it at a refresh
  with the old rule is mutant m5 and freezes every cues-at-end Matroska file (T2c late 1023 / shown 6).

Fork D -- the seek aim.
- D1 [CHOSEN] frame-index seeks aim at the frame's middle: ptsOfRel(rel) + half a frame (relOf rounds at the half too,
  so the keyframe found IS frame rel's or the one before it). Only the three frame-index callers (C5); clock-time seeks
  keep their exact target, so the forward golden traces cannot move (verified: 510 assertions unchanged).
- D2 round instead of truncate. Rejected: the error is the container's rounding (0.33 ms at 1 ms, 0.6 ms at 1/600), far
  above one tick.
- D3 add the half frame inside seekToTimestamp for every seek. Rejected: moves the clock-time forward seeks (the golden
  traces) for no measured gain.

Fork E -- the inherited big-share PingPong turn hold (X3).
- E1 [CHOSEN] accept: Matroska == MP4, the hold is MP4's pre-existing planner behaviour (one layer, clips >= ~40 s); FILE
  it (F1) with the trace. Boris question Q1.
- E2 retain the cap for PingPong (prototype proto4): late 0 for MP4 and MKV, slightly fewer decodes -- but it breaks the
  committed R-9 test (test_gop_cache_store.cpp:1142, "PingPong keeps its retainFrames") and grows forward-play memory to a
  quarter of the budget: an adopted rule (R-9 / Q2) changes -> its own ruling, not this lane.
- E3 ship items 1 + 3 only, defer item 2 until F1. Rejected as the default: keeps 1.3-7x decodes in every multi-layer
  Matroska reverse to protect an accident that only helps one-layer clips >= ~40 s; Boris may choose it (Q1).

## 4. ITEMS

Fixtures (tests/fixtures; every command run with brew FFmpeg 8.0, `-fflags +bitexact`, twice -> byte-identical; sizes
and sha256 prefixes from `$S/gen5.sh`):
- FX1 video_h264_gop250_64x64.mkv = `ffmpeg -i tests/fixtures/video_h264_gop250_64x64.mp4 -c copy -fflags +bitexact <out>`
  (47,251 B, f3efd8c1ef9b8fb1) -- Cues at the end.
- FX2 video_h264_gop250_cuesfront_64x64.mkv = the same + `-cues_to_front 1` (47,251 B, 2cc9c9f8ec94f3f3).
- FX3 video_h264_scenecut_64x64.mkv = remux of video_h264_scenecut_64x64.mp4 (50,486 B, a9c17605b038c2a3).
- FX4 video_h264_allintra_cuesfront_64x64.mkv = remux of video_h264_allintra_64x64.mp4 + `-cues_to_front 1` (23,485 B,
  f310619d1c3af1aa).
- FX5 video_h264_gop30_64x64.mkv = remux of video_h264_gop30_64x64.mp4 (16,277 B, 69d77cbff6420f5b).
- FX6 video_hap_tb600_64x64.mov = `ffmpeg -f lavfi -i testsrc2=size=64x64:rate=30 -t 3 -c:v hap -video_track_timescale 600
  -fflags +bitexact <out>` (100,841 B, 83cfa97a7a1066d3).
Each test asserts its fixture's shape first (gop2 precedent, REQUIRE with a "fixture shape" message): FX1 / FX3 / FX5
open with keyRels {0}; FX2 opens with keyRels {0, 250}; FX4 / FX6 every packet and frame a keyframe (keyCounts); FX6's
stream time base 1/600.

### Item 1 -- the intra verdict and the keyframe model come from one pure reading of the index
Files: src/media/GopCache.h, src/media/VideoPlayer.h, src/media/VideoPlayer.cpp (open()).
- GopCache.h (pure, no FFmpeg): `struct KeyIndex { std::vector<int> rels; int gopFrames = -1; bool intraOnly = false; };`
  and `inline KeyIndex keyIndexFrom(const std::vector<int64_t>& keyTs, int entries, double timeBase, double frameDur,
  int totalFrames)`: keyTs = the index's KEYFRAME entries' timestamps in index order (MP4 / MOV: DTS; Matroska: Cue
  times). rels = max(0, lround((ts - keyTs.front()) x tb / fd)); gopFrames = max(1, lround(maxGap x tb / fd)) when >= 2
  keys and maxGap > 0, totalFrames when exactly 1 key and totalFrames > 0, else -1 (= keep the current estimate);
  intraOnly = entries >= 2 && keys == entries && gopFrames == 1. (The prototype's, `$S/proto3/src/media/GopCache.h`.)
- VideoPlayer: `int keyIndexEntries_ = -1;` and `void readKeyIndex(bool atOpen);` (decode thread after start(); open()
  before). readKeyIndex: (LIBAVFORMAT_VERSION_MAJOR >= 59; no-op without formatCtx_ / a stream) entries =
  avformat_index_get_entries_count; if (!atOpen && entries == keyIndexEntries_) return; collect the keyframe timestamps;
  k = keyIndexFrom(...); keyRels_ = k.rels; if (k.gopFrames > 0) gopFramesEst_ = k.gopFrames; if (atOpen && k.intraOnly)
  intraOnly_ = true. NEVER changes intraOnly_ when !atOpen.
- open(): VideoPlayer.cpp:277-304 becomes `readKeyIndex(true);` after the codec-intra line (:275-276). The open log line
  (:323-324) unchanged.
Behaviour change: a Matroska / any keyframes-only index is intra-only only if its keyframes are one frame apart (FX2:
reverse 1 frame / 9 s -> 268); MP4 / MOV / AVI verdicts unchanged except a VFR all-key index with a > 1-frame gap
(cache path).
RED-first tests:
- P (tests/test_gop_cache.cpp, "mkvidx P: keyIndexFrom ..."): (a) MP4 DTS {-1024, 126976, 254976}, 600 entries, 1/15360
  -> rels {0, 250, 500}, gop 250, not intra; (b) Matroska {0, 8333, 16667}, 3 entries, 1/1000 -> {0, 250, 500}, 250, NOT
  intra; (c) 30 keys at llround(i x 1000 / 30) ms, 30 entries -> gop 1, intra; (d) {0}, 1 entry, totalFrames 300 -> {0},
  300, not intra; (e) none -> {}, -1, not intra; (f) 30 keys at i x 512 (+512 from i = 15: one 2-frame gap), 30 entries,
  1/15360 -> gop 2, NOT intra. RED on 5e47d17: does not compile (no keyIndexFrom) -- the gop2 T1a precedent. GREEN: 16
  assertions (scratch: PASS).
- T1 (tests/test_gop_cache_store.cpp, "mkvidx T1: a long-GOP Matroska file with its Cues at the FRONT is not intra-only;
  reverse = the same stream in MP4"): the T1b harness (openCapped CAP 21, Emulated K 3, 9 s from the Loop wrap) on
  video_h264_gop250_64x64.mp4 and on FX2. Bars: FX2 intraOnly false; late <= 4; shown >= 265; decodes <= mp4 x 1.02;
  mismatches 0; guard: FX4 opens intra-only (B1's positive case). RED on 5e47d17 (scratch `$S/tobj/mkvidx_head`): intra
  1, late 875, shown 1, decodes 3240 vs 2186. GREEN (prototype): intra 0, 0 / 268 / 2186 = mp4 2186.
Risk: the gap rule needs a sane frame rate; an avg_frame_rate far off the stream (F5) would mis-size gop -- not a
regression (the old rule used the same frameDur_).

### Item 2 -- the keyframe model follows the demuxer's live index
Files: src/media/VideoPlayer.cpp (decodeStep, readKeyIndex), VideoPlayer.h (comments :281-284).
- decodeStep() (:807): first statement `readKeyIndex(false);` -- O(1) unless the entry count changed; a rebuild is one
  pass over the entries (the decode thread allocates already; not a sacred thread).
- A once-per-player witness when the model first reaches >= 2 keyframes after open() saw <= 1: `logLine("[VideoPlayer]
  Keyframe index: ", n, " keyframes, longest interval ", gop, " frames (open saw ", n0, "): ", path)` (logLine is the
  sanctioned decode-thread logger, LogLine.h:10-17; VideoPlayer.cpp already uses it on that thread, :758).
Behaviour change: a cues-at-end / cue-less Matroska file's windows, prefetch line, urgency, retention and decode-through
thresholds use the real keyframes from its first seek or second keyframe read on; MP4 / MOV / AVI never rebuild (their
count never changes after open; X2's MP4 rows identical to HEAD).
RED-first tests (tests/test_gop_cache_store.cpp, "mkvidx T2 ..."):
- T2a FX1: REQUIRE open keyRels == {0} (fixture shape: the lazy index); after the reverse start + 4 decodeSteps (the
  DEMAND seek): keyRels == {0, 250}, gopEst == 250. RED: {0}, 300.
- T2b FX1, big budget, 9 s of plain forward play (Show): seeks == 0, keyRels == {0, 250}, gopEst == 250. RED: {0}, 300.
  (Discriminates C1 from C2: mutant m3 fails here.)
- T2c FX3 vs video_h264_scenecut_64x64.mp4 (T1 harness): keyRels {0, 37, 150, 190, 213, 262}, gop 113, decodes <= mp4 x
  1.02, late <= mp4 late, mismatches 0. RED: keys {0}, gop 300, decodes 2141 vs 910. GREEN: 910 = 910.
- T2d FX1 vs the gop250 MP4 (T1 harness): decodes <= mp4 x 1.02, late <= 4. RED: 2425 vs 2186. GREEN: 2186.
- T2e (tests/test_video_decode_trace.cpp): a NEW case "threaded reverse on a Matroska file (Cues at the end)" -- the body of
  the threaded reverse case (:264-311) factored into a function of the fixture, run on FX5 with the same bars (shown >=
  100, violations 0, hits > 0, direction changes >= 6). Not a RED (passes on HEAD): it puts the new decode-thread rebuild
  under TSan (gate G2).
Risk: R4 (rebuild frequency) and R1 (the inherited turn hold).

### Item 3 -- a frame seek aims at the frame's middle
Files: src/media/VideoPlayer.h / .cpp.
- `bool seekToRel(int rel, double backSec = 0.0)` = `seekToTimestamp(ptsOfRel(rel) + 0.5 * frameDur_ - backSec)`, with a
  comment: timestamps are rounded to the container's time base and the conversion truncates; relOf rounds at the half.
- Call sites: runStep :1352 -> `seekToRel(r.seekFrom, runSeekBackSec_)`; forwardStep :898 -> `seekToRel(nextRel)`;
  forwardIdle :1543 -> `seekToRel(target)`. seekToTimestamp itself and its clock-time callers (:853, :882, :887) unchanged.
Behaviour change: an intra-only file's DEMAND run lands on its own frame (one seek + one decode per reverse frame); a run
whose window starts on a keyframe lands on that keyframe, not a GOP below.
RED-first test (tests/test_gop_cache_store.cpp, "mkvidx T3: a frame seek aims at the frame's middle -- intra-only files
on a coarse time base reverse at one decode per frame"): T1 harness, FX6 for 2.5 s and FX4 for 0.9 s. Bars: intraOnly true;
late <= 2; shown >= 72 (FX6) / 26 (FX4); decodes <= 1.1 x shown + 3; mismatches 0. RED (5e47d17): FX6 late 64 / shown 38
/ decodes 834; FX4 4 / 17 / 324. GREEN: 0 / 75 / 78 and 0 / 27 / 30.
Risk: R6 (a VFR keyframe within half a frame above the nominal time is that frame; F1's overshoot re-seek covers the
exact-half edge; the committed VFR / open-GOP / MPEG-4 / AVI cases are unchanged on the prototype).

Mutant matrix (builder; scratch copies only, never the deliverable; reference run `$S/mut.sh`):
| arm | T1 | T2 | T3 |
|---|---|---|---|
| HEAD 5e47d17 | FAIL (875 / 1) | FAIL a b c d | FAIL (834 / 324 decodes) |
| prototype (items 1-3) | PASS | PASS | PASS |
| m1 old intra rule | FAIL | PASS | PASS |
| m2 no refresh (= HEAD's model) | -- | FAIL a b c d | -- |
| m3 refresh in seekToTimestamp only | PASS | FAIL b | PASS |
| m4 no half frame | PASS | PASS | FAIL |
| m5 refresh re-derives intra (old rule) | FAIL | FAIL c d (late 1023 / 1055, shown 6 / 2) | PASS |

### Item 4 -- live probe row u13_container_reverse + its A/B rules
Files: .harmony/probe-video.py (make_fixtures), .harmony/probe-video.json, .harmony/probe-vupload.py, .harmony/
probe-vupload.json, .harmony/probe-vupload-ab.py.
- probe-video.py make_fixtures (:241-296): a spec with `"remux": "<source fixture>", "mux": [...]` is made by `ffmpeg -y
  -loglevel error -i <source> -c copy <mux...> -fflags +bitexact <tmp>` + rename; non-remux specs are made FIRST (sorted()
  puts a1080_g250.mkv before its .mp4); the "keys" check applies as today.
- probe-video.json fixtures: a1080_g250.mkv {remux a1080_g250.mp4, mux [], keys keyframesG250};
  a1080_g250_cuesfront.mkv {remux a1080_g250.mp4, mux ["-cues_to_front","1"], keys keyframesG250}; a1080_g250_60s.mp4
  (a1080_g250.mp4's spec + "dur": 60, keys keyframesG250x60 = [0, 8.333, 16.667, 25.0, 33.333, 41.667, 50.0, 58.333]);
  a1080_g250_60s.mkv {remux a1080_g250_60s.mp4}; hap1080_tb600.mov {1920x1080, band 64, testsrc2, enc ["-c:v","hap",
  "-video_track_timescale","600"], keys null}.
- probe-vupload.py: ROW_FIXTURES["u13_container_reverse"] = those five; probe-vupload.json "layers" adds
  "u13_container_reverse": [169, 170, 171, 172]. u7's per-scene body (:512-582) becomes a helper called by u7 and u13
  (u7's output unchanged); u8's window body (:590-613) becomes a helper with a window length. Scenes:
  (a) mkv_cuesfront_reverse, (b) mkv_reverse, (c) hap600_reverse -- u7 "reverse" scenes (1 s, a 5 s window, the mid
  capture's bracket, the GC4 mono captures); (d) mkv_long_column_1080x4, (e) mp4_long_column_1080x4 -- four 60 s players
  reversing by ONE trigger_column at the default budget, a 10 s window, counters only (no captures: the code band and
  expected() assume 300 frames); (f) mkv_long_pingpong_turn, (g) mp4_long_pingpong_turn -- one 60 s clip, PingPong,
  in-point 0.95 (prime retrigger), 1 s, a 12 s window, counters + turn_seen, no captures. DATA keys as u7 / u8
  (uploads_per_s, late, decoded_per_upload, per_player_min, nonmono, bracket_ok, mono_ok, max_gap_ms) + the count of
  "Keyframe index:" lines in err.log (item 2's witness; INFO).
- probe-vupload-ab.py: a u13 block, bars in probe-vupload.json "_u13" (every rule needs >= 5 launches per arm):
  [FREEZE] (a) B medians: uploads/s >= u7ReverseUploadsMin 28.5, late <= u7ReverseLateMax 10, bracket_ok 1, mono_ok 1,
  nonmono 0 in every B launch; INFO A's medians. [MKV] (b) the same bars. [HAP] (c) the same + B median
  decoded_per_upload <= u13HapDecPerUploadMax 1.25. [CPU] (d) B median decoded_per_upload <= u13CpuRatioMax 0.7 x A's (d).
  [PARITY] B median decoded_per_upload (d) <= u13ParityMax 1.15 x B's (e). [GUARD] B median late (d) <= A's (d) +
  u13LateSlack 15 and B median per_player_min (d) >= A's - 1.5 (a non-regression guard: it catches only > 15 late per
  window). INFO (f) / (g): "big-share turn: A mkv late X, B mkv late Y, B mp4 late Z, max gaps".
- --selftest gains two synthetic u13 TSVs: all-pass (exit 0, the six rule tags PASS) and a frozen-B (a) (exit 1, exactly
  one [FREEZE] FAIL).
Expected RED on main (INFERRED from the emulation; G6's A arm confirms or refutes): (a) uploads/s < 5; (c) uploads/s
~15, decoded_per_upload >= 2; (d) decoded_per_upload >= 2x the lane's.
Risk: R9 (emulation vs live contention); the 60 s fixture encode (~1 min once; VIDEO_FIXTURES reuse).

### Item 5 -- docs (section 6)

### BUILD STAGES (order; one builder context can do both)
- Stage A (C++; base = main AFTER the hyg merge -- no overlap: hyg touches no src/media / tests/test_gop_cache* /
  test_video_decode_trace lines; "Pitfall NN" occurrences in them = 0):
  A1 fixtures FX1-FX6 + tests T1, T2a-e, T3 -- RED shown on the base with the gop2 relink rig (the committed test TU + the
  base's src/media, main build's flags.make / link.txt read-only, TEST_FIXTURES_DIR repointed); A2 item 1 + P; A3 item 2;
  A4 item 3. Builder self-checks: full ctest serial; TSan (test_gop_cache_store, test_video_decode_trace, test_gop_cache);
  the mutant matrix; the gop2 printed lines unchanged.
- Stage B (probe + docs): B1 item 4 (probe-video remux, JSON, u13, ab rules + selftest) -- selftest PASS, one INFO live
  smoke of u13 under the lock; B2 item 5.
Fence: no edit to Clip.h / Layer.h / Composition.h (tsan-r5), AudioEngine.cpp / DeviceGuard.cpp / test_log_line_lint.cpp
(hyg); pitfalls.md / CLAUDE.md edited on the post-hyg tree by quoted text, never by line number.

## 5. GATES (Harmony, after the merge)

- G1 `cmake --build build -j$(sysctl -n hw.ncpu)` then `ctest --test-dir build -j1`: 100 %; count = before + 5 (T1, T2,
  T3, P, T2e).
- G2 TSan build (RelWithDebInfo, -DADNA_SANITIZE=thread), TSAN_OPTIONS=halt_on_error=0:abort_on_error=0:
  test_gop_cache_store, test_video_decode_trace, test_gop_cache -> rc 0 and 0 "WARNING: ThreadSanitizer".
- G3 `build/tests/test_gop_cache_store "mkvidx*"` printed values == section 4's GREEN values (decodes may drift <= 2 % only
  after an FFmpeg upgrade -> re-register, ruling-gop2 R2); `build/tests/test_gop_cache_store "gop2*"` lines identical to
  gop2.md's GREEN block (2186 / 2776 / 2349 / 1698 / 2186 / 2427; T1c totals 0 / 337 / 428 / 0 / 31; T1d 1140 / 1398;
  T3 120 of 120 and 0 0 0; 23 / 0 / 425).
- G4 `python3 .harmony/probe-vupload-ab.py --selftest` -> SELFTEST PASS.
- G5 fixture audit: `shasum -a 256` of FX1-FX6 == section 4's prefixes.
- G6 live A/B, lock held, no Output window, `probe-vupload-ab.sh <main app> <lane app> 5 .harmony/probe-vupload.sh
  u13_container_reverse <out>` (interleaved, burners checked, ~25 min): every u13 rule PASS. Pre-registered decisions:
  (i) A's (a) median uploads/s >= 20 -> the live app does not reproduce X1: STOP, check the fixture's index (`$S/idx`)
  before judging B; (ii) [CPU] FAIL with [PARITY] PASS -> the live decoder is faster than K 5 (emulated ratio 0.55):
  record the ratio, re-register the bar at ratio + 0.05 (INFO), not a lane defect; (iii) [PARITY] FAIL -> lane defect;
  (iv) (f) / (g): B mkv late - A mkv late > 60 per window -> Q1 goes to Boris with the live number before the next release.
- G7 live MP4 non-regression (item 3 moves every container's frame seeks): `probe-vupload-ab.sh <main> <lane> 5
  .harmony/probe-vupload.sh u7_reverse_pingpong,u8_reverse_column_1080x4 <out>` at the default budget, then u8 again with
  VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=256: the existing [VU7] / [ABS] / [GC6] / [GC7] / [GC8] / [GC12] rules PASS for B.
  B-vs-A per-scene medians (uploads/s, late) are INFO (their margins equal the drift).

## 6. DOCS

- docs/claude/rendering.md :79: replace "MKV / WebM hold a 1-entry index after open, so keyRels_ = {0} and their window
  is always the whole Future share (extra decodes bounded by the share, no stall)" with "the keyframes are the demuxer's
  LIVE index -- re-read at the top of every decodeStep when its entry count changes (a Matroska file's Cues load at its
  first seek and every keyframe read adds one; the open-time index had 1 entry: 1.3-7x the decodes of the same stream in
  MP4 -- s-rta-1002b mkvidx)"; replace "or an index of keyframes only" with "or an index whose entries are all keyframes ONE
  frame apart (Matroska Cues list keyframes only: a long-GOP file with its Cues at the front was called intra-only and
  froze in reverse)"; add "a seek for frame k aims at k's middle (`seekToRel`: container timestamps are rounded -- 1 ms,
  1/600 -- and a truncated target landed a whole GOP low; HAP at 1/600 showed 142 of 270 frames)"; Levers: add
  `VideoPlayer::readKeyIndex`, `GopCache::keyIndexFrom`; the measured table (X1-X5) and "[G6 medians, filled in by Harmony]".
- docs/claude/pitfalls.md: Pitfall 62 -- replace "(MKV / WebM keep a 1-entry index after open: their window is always the
  whole share)" with "(the demuxer's live index -- Pitfall NN)"; add the guard names "mkvidx T1 / T2 / T3". New entry
  Pitfall NN (Harmony assigns; next free 64): "**A frame's seek aims at its middle, and the keyframe model is the demuxer's
  LIVE index -- never the open-time reading of it, never 'every entry is a keyframe' alone**: (1) `seekToRel(rel)` =
  ptsOfRel(rel) + half a frame -- container timestamps are rounded (Matroska 1 ms, QuickTime 1/600; double arithmetic can
  truncate k / 30 x 15360 to 512k - 1) and a BACKWARD seek just below a keyframe lands on the one before: a GOP of lead-in,
  and an intra-only DEMAND run that never reaches its frame (HAP 1/600: 142 of 270 shown, 2,797 decodes per 9 s; fixed
  270 / 273). Clock-time seeks keep their exact time (the golden traces). (2) `readKeyIndex` rebuilds keyRels_ /
  gopFramesEst_ whenever the index entry count changes (top of decodeStep): Matroska Cues load at the first seek and each
  keyframe read adds an entry. (3) intra-only from an index = every entry a keyframe AND every interval one frame
  (`GopCache::keyIndexFrom`); intraOnly_ is decided in open() only -- re-deriving it with the all-key rule at a refresh
  freezes every Matroska file. Guards: test_gop_cache.cpp mkvidx P; test_gop_cache_store.cpp mkvidx T1 / T2 / T3;
  test_video_decode_trace.cpp threaded reverse on a Matroska file; live probe-vupload u13."
- CLAUDE.md (24,006 B before hyg; re-measure after): add after the 63 line: "64. A frame seek aims at the frame's middle;
  the keyframe index is the demuxer's live one -- before touching `seekToTimestamp`, `readKeyIndex` or the intra-only
  verdict." (170 B). Paid by MOVING the Key-capabilities source breakdown " (3D 24, Geometric 11, ..., Routing 1)" (201 B,
  CLAUDE.md:14) verbatim into docs/claude/history.md:52, which today points back at it ("full per-category breakdown in
  CLAUDE.md's Key Capabilities line") -- net -31 B. If hyg already moved that text, move the routine parenthetical of the
  same line (205 B) into docs/claude/recording.md "Surfaces" instead. Gate: `wc -c CLAUDE.md` <= its post-hyg size.
- docs/claude/testing-eyes.md "Reverse / ping-pong rows": one sentence for u13 (scenes, the [FREEZE] / [HAP] / [CPU] /
  [PARITY] rules).
- VideoPlayer.h :279-284 comments (intraOnly_: the gap rule, open() only; keyRels_: refreshed by decodeStep).

## 7. RISKS

- R1 (the strongest counterargument) Matroska inherits MP4's big-share PingPong top-turn hold: one layer, clips >= ~40 s,
  emulated: HEAD MKV late 0 -> 15-29 render frames (40 s clips) / 27-137 (60 s) once per reverse leg; ~0.1-0.5 s at live
  decode speed (K 5), up to ~1.1 s contended (K 3); MP4 identical before and after. Why the fix still wins: HEAD's
  smoothness there is an accident of the stale index (it retains a quarter of the budget), paid for with 1.3-7x decodes in
  every multi-layer configuration and a reverse FREEZE for front-Cues files; the hold is the planner's (the first window
  after a turn is the whole free share decoded bottom-up -- trace), fixed for MP4 and MKV together by F1 (E2's one-line
  cure exists but needs an R-9 ruling). Cheapest test: G6 (f) / (g).
- R2 VP9 WebM at a small share gets later (X6: CAP 21 K 3 late 56 -> 156 per 30 s, PingPong 103 -> 326) because it becomes
  VP9-in-MP4, whose lead-ins cannot skip frames; equal at CAP 164 (37 = 37). Only near 4 VP9 1080p layers in 256 MB (an
  8 GB Mac under pressure). FILED F2. Cheapest test: `$S/run6.sh` VP9 rows (done).
- R3 item 1 moves a VFR all-intra non-codec-intra MP4 to the cache path (measured late 0, +9 % decodes, memory in its share).
- R4 the rebuild: one accessor call per decodeStep; a full pass per index growth (Matroska: per keyframe until the Cues
  load; ff_reduce_index can SHRINK a huge cue-less index -> a rebuild from a coarser subset = an over-estimated lead-in,
  bounded). Cheapest test: G2 + INFO video_writer_step_max_ms in G6 / G7.
- R5 an FFmpeg upgrade that parses Matroska Cues at open turns T2a's / T2b's fixture-shape REQUIRE red although the code
  is right -> re-register (the shape check says so in its message).
- R6 the half-frame aim on VFR: a keyframe up to half a frame above the nominal time IS frame rel (relOf rounds the same
  way); exactly at the half -> F1's overshoot re-seek (one seek). The committed VFR / open-GOP / MPEG-4 part 2 / pts-less
  AVI cases pass unchanged on the prototype.
- R7 the intra DEMAND restart (C4) remains for frames more than half a frame off the average grid (heavier VFR than the
  1-in-7 measured, which item 3 fixes: late 275 -> 0). FILED F4 (windowLo = rel for intra runs).
- R8 hyg merges first: pitfalls.md is re-ordered and CLAUDE.md re-sized under this lane -- Stage B starts from post-hyg
  main (`git log --oneline -1 -- docs/claude/pitfalls.md` is hyg's).
- R9 the emulation models one player at a fixed K; four live players contend (main's MKV players saturate their threads),
  so the live [CPU] ratio may be lower (better) than emulated -- or higher if live decode is faster than K 5 (G6 rule ii).
- R10 the bitexact fixtures were made with brew FFmpeg 8.0; another FFmpeg can produce different bytes from the same
  commands -- the committed files are the fixtures; regenerate only to compare.

FILED (not this lane): F1 "bigwin": the first PREFETCH window after a PingPong top turn / a reverse start at a big share
is the whole free share decoded bottom-up (X3; also "reverse from the wrap, CAP 690: late 176 at the start" for every
container). F2 VP9 lead-in cost (no droppable frames). F3 duration-less Matroska shows a still frame (C6 / M4). F4 the
intra DEMAND restart (C4). F5 (INFO) an H.264-in-AVI remux reported avg_frame_rate 600 -> a 36,000-frame trajectory (one
scratch file; real AVIs untested).

## 8. WHAT ONLY BORIS CAN CHECK

- His real MKV / WebM clips (OBS recordings, downloaded WebM) on a layer in reverse and ping-pong: smooth, no freeze (on
  main a front-Cues file froze in reverse; whether his files have front Cues is unknown).
- His HAP / ProRes library in reverse: every frame shown (main may show about half of a QuickTime-1/600 HAP clip; which
  time base his files use is unknown -- ASSUMED common, not measured).
- Four reversing MKV layers: fans / CPU in Activity Monitor lower than before.
- A single long (> 40 s) clip in ping-pong: whether a brief hold at the top turn (shared with MP4) is noticeable to him.

## 9. QUESTIONS FOR BORIS (each with a default)

- Q1 Long single-layer ping-pong clips (> ~40 s) will show the same brief hold at the top turn that MP4 already shows
  (~0.1-0.5 s at normal decode speed, emulated) until a follow-up fixes it for both. Ship mkvidx now, or wait for that
  follow-up? DEFAULT: ship now (it cures a reverse freeze, half-speed HAP reverse and 2-7x CPU on multi-layer MKV); the
  follow-up lane F1 is filed.
- Q2 Browser / streaming recordings with no duration (some WebM) open as a still frame today. Support them? DEFAULT: filed
  as its own lane (F3), not in mkvidx.

STATUS: READY FOR RULING

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 15:33:35) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1002b/ruling-mkvidx.md IN FULL (AM1-AM18, the final BUILD STAGES A + B, the FINAL GATE
   LIST G1-G9). Base = main fa9604d (+ docs-only commits; src / tests identical to fa9604d).
2. PITFALL NUMBER: the new pitfall is **Pitfall 64** (Harmony-assigned; write 64, not NN). Next free after this lane: 65.
3. BORIS QUESTIONS — defaults taken: Q1 ship now (the top-turn hold stays until follow-up F1); Q2 duration-less WebM = its
   own filed lane (F3). Both listed on Boris's page.
4. LIVE ROWS: the builder runs only the ruling's ONE INFO live smoke of u13 under the lock; the G6 / G7 interleaved A/B is
   Harmony's gate after the merge. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not
   start is his (s-rta-1002b incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses,
   stop the batch and release the lock.
5. FENCE: no concurrent BUILD lane at launch. Plans in flight (bf9b decks / layers model, bf6, ui, bf7, bf2, bf1, bf45,
   bf10 MilkDrop) build after or beside you; your files are src/media/VideoPlayer* / GopCache* / VideoRing*, their tests,
   tests/fixtures (new), .harmony/probe-video* / probe-vupload*, docs/claude/rendering.md + pitfalls.md (62, new 64) and the
   CLAUDE.md index line. Never Clip.h / Layer.h / Composition.h / MainComponent.cpp / the Compositor.
6. MERGE: Harmony merges (commit notes -> pre-merge app copy -> merge -> build -> ctest -> G1-G9).
