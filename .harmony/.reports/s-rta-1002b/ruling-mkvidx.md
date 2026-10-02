# RULING lane mkvidx (s-rta-1002b) -- the blind council's attacks

Author: Architect. Plan: `.harmony/.reports/s-rta-1002b/plan-mkvidx.md` (not edited). Seat papers, verbatim:
`.harmony/.reports/s-rta-1002b/attack-mkvidx-papers.md`. Ids here: seat decode-correctness = A1-A11, seat
mkvidx-gates-attack = GA1-GA8 (the final gate list is G1-G9 -- no clash).

Base: main fa9604d (post-hyg). The evidence base 5e47d17 is equivalent for this lane: `git diff 5e47d17 fa9604d --
src/media tests/test_gop_cache_store.cpp tests/test_video_decode_trace.cpp tests/test_gop_cache.cpp
tests/test_video_player_open.cpp` is EMPTY (VERIFIED), so every RED value of the plan and of this ruling holds on fa9604d.

Method: every disputed claim was re-measured on this machine (brew FFmpeg 8.0, libavformat 62.3.100, libavcodec
62.11.100) with the REAL VideoPlayer under the plan's Emulated harness (K decodes per 120 Hz render frame, decode ms
pinned; deterministic counts). Arms: HEAD (= fa9604d src/media), proto3 (the plan's prototype) and m7 (= proto3 with
the half-frame aim at runStep ONLY -- the design AM4 adopts). Scratch rig (rebuildable, not committed):
`$R = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/ruling`
-- idx3.c / idx4.c (index probes), rcost.cpp (the plan's harness + a forced-not-intra switch, forward-skip count, max
resident), rc_head / rc_proto3 / rc_m4 / rc_m6 / rc_m7, t2e.cpp / t2b.cpp, e*.sh + e3x.log / e6.log / e12.log, fxv1/ (the
new fixtures) -- plus the plan's `$S` rig.
Arm labels: rows run on proto3 were re-run on m7 and compared whole-line (late, shown, decodes, seeks, mismatches,
resident, keys): E2 / E4 rev / PingPong / flip 45 rows, 0 differ (e17.sh); E8 / E9 1800f PingPong 24 rows, 0 differ
(e16.log); the 300-frame E9 rows and the E5 sweep ran all arms. E8's 1800f Loop-reverse rows ran on proto3 only --
identical on m7 by construction (m7 differs only at the forward repositioning seeks, never reached in a Loop reverse
from the wrap).

VERDICT: plan AMENDED -- 18 amendments (AM1-AM18) override the plan body. ready_to_build: YES. The FIX verdict stands.
Every Matroska parity claim was re-measured; the plan over-claimed in two places, both corrected: X3 PingPong at shares
>= 164 is not "count for count" (a one-time window split, AM18 / F7), and item 3's two forward call sites are dropped
(AM4: zero measured cost, removes the only unguarded new landing).

## 0. What this ruling measured (VERIFIED on this machine unless marked)

- E1 all-intra Matroska, Cues at the END (A1). The open-time index of an all-intra stream is NOT 1 entry: find_stream_info's
  read-ahead (plus open()'s first decode) indexes every packet it reads -- all keyframes, one frame apart -- so B1 calls the
  file intra AT OPEN. Entries after open(): 64x64 H.264 8; 1080p H.264 keyint=1 8; 4K H.264 8; 4K lossless 4:4:4
  (~3 MB / frame) 2; 1080p SVT-AV1 -g 1: 1 after find_stream_info, 3 after the first decode (libdav1d's frame delay; the
  app's thread_count is 2, VideoPlayer.cpp:129). Reverse CAP 21 K 3, 9 s: HEAD allintra_300.mkv (end Cues) late 68 /
  shown 163 / decodes 3240 -- the plan's X5 row "all-intra H.264 in MKV (1 ms)" exactly, i.e. the plan DID measure this
  case; proto3 = m7 0 / 270 / 273 = the MP4. FX4e (the committed all-intra fixture remuxed, end Cues) 0.9 s: HEAD 4 / 17 /
  324, m7 0 / 27 / 30 = MP4.
  Residual priced with a forced flag (intraOnly_ = false after open, proto3, allintra_300.mp4): CAP 21 K 3 late 0, 281 vs
  273 decodes, 21 frames held; CAP 164 late 29 (one hold = the F1 "bigwin" first window), 374 decodes, 164 frames held;
  PingPong CAP 21 25 s 937 vs 753 decodes; CAP 8 K 1 276 vs 273.
- E2 AV1 / VP9 in Matroska (A2). After the first seek the index == the decoder's AV_FRAME_FLAG_KEY frames {0, 60, 120, 180,
  240} for libaom-av1 MKV, SVT-AV1 MKV and VP9 WebM (as in their MP4s). MKV decodes, HEAD -> m7, vs MP4: rev CAP 21
  2212 -> 1044 = 1044; rev CAP 164 535 -> 548 = 548 (late 3 = 3); PingPong CAP 21 25 s 2702 -> 1482 = 1482; flip CAP 21
  1748 -> 720 = 720 (late 2 -> 0); mismatches 0 everywhere; identical for the three codecs.
- E3 cue-less Matroska (A4). g250 1800f piped to MKV (no Cues, no duration -> totalFrames 0, the F3 still frame): open {0};
  seek 20 s -> {0, 250, 500, 750}; then seek 55 s -> {0 .. 1750} (all 8); seek 40 s alone -> {0 .. 1250}. Always a
  CONTIGUOUS prefix: a seek past the last entry parses every cluster from it (behaviour VERIFIED; the matroskadec
  read_seek mechanism INFERRED -- the libavformat source is not on this machine).
- E4 Matroska timestamps vs the decoder (A9). Open-GOP B-frame streams (the committed opengop fixture: has_b_frames 2, a key
  every 10; an x264 bframes=4 b-pyramid open-GOP GOP-30 encode): the Matroska index rels == the decoded key-flagged frames'
  rels ({0, 10, ..., 50}; {0, 30, ..., 270}); the forward-read prefix and the Cues-loaded index agree. It is the MP4 DTS
  index that is skewed (bf4: {0, 30, 59, 88, 118, 147, 178, 207, 238, 266}) -- pre-existing, untouched by this lane.
  Parity, MKV decodes HEAD -> m7 vs MP4: bf4 rev CAP 21 1881 -> 587 = 587; CAP 164 412 -> 410 = 410; PingPong CAP 21 2365
  -> 1113 vs 1141; flip 1639 -> 491 = 491; opengop (60 f, 1.8 s) 108 -> 106 vs 107. Residual (INFO): a VFR + B-frame stream
  (1 in 7 dropped, bframes 3, GOP 30) rev CAP 21 1336 -> 472 vs 454 (+4 %), CAP 164 337 -> 322 vs 317, late 12 = 12, shown
  193 = 193, seeks / runs identical -- the same with no aim at all (rc_m4 472 / 322), so it is the container's post-seek
  packet delivery, not the model or the aim (cause INFERRED).
- E5 the aim at the forward sites (A7, GA6). m7 (runStep only) vs proto3 (all three sites): identical on the plan's scratch
  T1-T3 lines; on the committed suites' printed lines (test_gop_cache_store 26 / 662 incl. every gop2 line,
  test_video_decode_trace 510 / 5, test_video_player_open 10 / 2 -- only the Catch2 seed and FFmpeg pointer addresses
  differ); on X5 (HAP 1/600 270 / 273; VFR MJPEG 154 / 157; MJPEG MKV 270 / 273; ProRes 270 / 273; the ms-timestamp g250
  MP4 late 682, longest hold 73); on every PingPong / flip row of E9; and on a FLIP SWEEP -- 61 flip points (every frame
  across 2 s) x 7 files (VFR+B-frames MP4 / MKV, bf4 open-GOP MP4 / MKV, committed vfr / vfrgap / mpeg4_bf2 MP4) x CAP 21
  / 164 x 3 arms (2,562 runs): forward frames skipped after the flip HEAD = proto3 = m7 = 0 on every MP4 row (vfrgap
  1 / 1 / 1, pre-existing), Matroska rows HEAD 1-3, proto3 = m7 = 0. The skip A7 predicts did not occur; the forward-site
  aim also bought nothing measurable.
- E6 item 3 on the committed B-frame / VFR / AVI / TS fixtures (GA6). 10 fixtures x {rev CAP 21 K 3, rev CAP 8 K 1,
  PingPong CAP 21}: HEAD = m7 = proto3 on 27 of 30 rows; m7 better on the other 3 (vfrgap 327 -> 318 and 441 -> 422,
  mpeg4 .ts 226 -> 214 decodes; late equal).
- E7 the sign mutant m6 (minus half a frame; A11): T3 FAILS (HAP shown 0 / 900 decodes; FX4 shown 0 / 324); the T3b rows
  FAIL (opengop 430 -> 465, vfr 425 -> 465 decodes); T1 / T2 PASS.
- E8 the 4K shares (A6). A 4K cache frame is native yuv420p, ~12.4 MB (not 33 MB RGBA): 4 players in 256 MB -> the floor
  kMinFrames 8 (GopCache.h:18); 4 in 2 GiB -> 41; one in 2 GiB -> ~165. 1800f Loop reverse 30 s, decodes MP4 / HEAD MKV
  -> m7 MKV: g250 CAP 8 K 1 3524 / 3583 -> 3524; CAP 8 K 3 9349 / 10269 -> 9349; CAP 41 K 1 3219 / 3579 -> 3219; CAP 41
  K 3 5995 / 10236 -> 5995; g60 CAP 8 K 3 5492 / 8935 -> 5492; CAP 41 K 1 1956 / 3521 -> 1956; CAP 41 K 3 2328 / 9775 ->
  2328 (late equal in every row). PingPong 130 s K 1: g250 CAP 41 7843 / 8901 -> 7843 (late 712 vs MKV 719); g60 CAP 41
  5245 / 8762 -> 5245; CAP 8 rows equal decodes (late +10 / +58 of ~6100 / ~2620). INFO: at CAP 8 K 1 a GOP-250 reverse is
  broken for EVERY container (113 of ~900 frames shown) -- gop2 F4, pre-existing.
- E9 PingPong parity re-check (GA1, A5): the plan's "count for count" is NOT exact here. 1800f 130 s, m7 MKV vs MP4: CAP 21
  equal (g250 16682, g60 7298); CAP 164 g250 5136 =, g60 4207 vs 4182 (+1 seek); CAP 690 K 3 g250 4023 vs 3941, g60 3684
  vs 3674; K 5 4064 vs 3978, 3684 vs 3663 (+1 seek each; late within 1). 300-frame fixtures, 25 s from frame 0: CAP 21 FX1
  = FX2 = MP4 2634 (late 5 = 5), FX3 = MP4 1413; CAP 164 FX1 795 = 795, FX3 687 vs 668 (seeks 5 vs 4); CAP 690 FX1 690 vs
  554 (+1 seek, +136 decodes, all between 10.5 s and 12 s; late 0 = 0), FX2 (front Cues) 554 =. Mechanism (INFERRED from
  the counts + GopCache.h:446-453): in the first forward leg the end-Cues model is {0} / gop = totalFrames until the second
  keyframe is read, so PingPong retention (VideoPlayer.cpp:1065-1070) is sized from the larger gop (54 vs 45 resident at
  9.9 s); the leftover frames form an island under the first PREFETCH target after the turn and planPrefetch never plans
  below a resident frame -- the window splits once. Flips (Loop reverse 4 s, then forward to 8 s): FX1 = FX2 = MP4 1365
  (late 30 = 30), FX3 443 =, FX5 399 =.
- E10 the T2e oracle (A8, GA2). The threaded reverse case on FX5 (gop30 MKV, end Cues), read after close() + stopThread:
  HEAD keyRels {0}, gop 90 (FAIL); m7 {0, 30, 60}, gop 30 (PASS 3 of 3; shown 159, violations 0, hits 160-161, direction
  changes 7). Race-free: JUCE stores threadHandle = nullptr (std::atomic, juce_Thread.h:596) after run() returns
  (juce_SharedCode_posix.h:990-993) and stopThread waits on isThreadRunning() (juce_Thread.cpp:192-194, :227-243).
- E11 T2b through the forward swap (A5). FX1 9 s forward: HEAD keys {0} gop 300, m7 {0, 250} gop 250; both late 0, shown
  271 (0..270), every step between shown frames 1.
- E12 readers (A8). `grep -rn -e keyRels_ -e gopFramesEst_ -e intraOnly_ src` -> only src/media/VideoPlayer.h:279-284
  (declarations) and VideoPlayer.cpp (open() before start(), and decode-thread functions).

## 1. Seat decode-correctness

- A1 [MUST] an all-intra end-Cues Matroska file stays on the cache path -- ACCEPT IN PART.
  Premise REJECTED (E1): such a file opens intra-only under B1 (its open-time index is the find_stream_info read-ahead:
  2-8 entries in every all-intra file built, 64x64 to 4K lossless, H.264 and SVT-AV1); the plan's X5 row is this case and
  the prototype gives 0 / 270 / 273 = MP4. The attack's "X5 only measured codec-intra MJPEG" is wrong: the HEAD numbers
  68 / 163 / 3240 reproduce on the end-Cues H.264 file.
  ACCEPTED: (a) the fixture FX4e (FX4 without -cues_to_front) in T3 and in the shape case (AM5, AM6, AM10); (b) the
  residual FILED with its measured cost (F6, AM17).
  REJECTED: the one-way promotion -- the case it guards could not be constructed, and it is a mid-playback intraOnly_
  switch, the m5 family that froze every Matroska file.
- A2 [MUST] AV1 / VP9 uncovered -- ACCEPT. Committed: a VP9 WebM / MP4 pair (FX7 / FX7m) as a T2 parity row (AM5, AM8):
  RED on the base 2212 vs 1044 decodes and keyRels {0}; GREEN 1044 = 1044 and keyRels == the decoder's keys. AV1 is
  VERIFIED in scratch (E2: libaom and SVT-AV1, index == decoded keys, MKV == MP4 in rev / PingPong / flip), NOT committed:
  an AV1 ctest would depend on the build's AV1 decoder (libdav1d; FFmpeg's native av1 decoder needs a hardware path and the
  M1 Pro has no AV1 decode -- INFERRED). The filed item this lane serves (HANDOFF.md:128) names MKV / WebM, not AV1.
- A3 [SHOULD] the refresh keys on the entry count only -- ACCEPT the signature (AM3): rebuild when (entries, entry-0
  timestamp, last-entry timestamp) changed -- two O(1) accessor calls, a strict superset of the count trigger (it can only
  rebuild more often, each time from the live index). REJECTED the shrink test: ff_reduce_index halves the entries, so the
  count already changes (INFERRED); no constant-count content change was observed on FFmpeg 8.0 (the Cues merge keeps the
  read entries' timestamps, E3 / E4); a test would need a max_index_size seam. Mutant m9 (count only) is recorded as
  indistinguishable (AM13).
- A4 [SHOULD] a sparse index fakes a long GOP -- REJECT (E3). The demuxer's index is a contiguous prefix after any seek;
  keyRels_ is by construction the set of landings av_seek_frame can make (a coarser index, e.g. sparse muxer Cues, is
  exactly where the demuxer lands too). No reverse plan is made above the read prefix: a reverse start or a Loop wrap
  begins with a DEMAND seek that completes the index (end Cues) or reads to the end (cue-less) before the next decodeStep
  rebuilds. The cue-less test is moot besides: a piped cue-less file has no duration (the F3 / C6 still frame).
- A5 [SHOULD] the mid-run swap -- ACCEPT IN PART; the predicted one-time retention effect EXISTS (E9).
  VERIFIED harmless: the reverse-start swap (the DEMAND seek loads the Cues; the rebuild runs at the next step with the run
  in flight -- T1 / T2 totals equal the MP4's exactly) and the forward swap at frame 250 (late 0, no step > 1, E11).
  VALIDATED: PingPong on an end-Cues / cue-less Matroska file at shares >= 164 splits its first post-turn window once (+1
  seek; +0.3-2.2 % decodes per 130 s; FX1 CAP 690 +136 decodes over 25 s; late within 1 of MP4).
  Amendments: T2b asserts late 0 and consecutive shown frames (AM8); T4 pins the split to at most one extra seek (AM12);
  the "count for count" claim is corrected (AM16, AM18); FILED F7 (no fix here: eviction-on-rebuild code for a one-time,
  late-neutral cost). REJECTED a separate swap-step trace test: the equal totals and T4's seek bar already fail on a run
  restart or a re-seek storm.
- A6 [SHOULD] 4K unmeasured -- ACCEPT the emulated rows (E8; recorded here and in rendering.md): the 4K shares are 8 / 41 /
  ~165 frames; m7 MKV == MP4 in every row; HEAD MKV 1.1-4.2x decodes, so the do-nothing bar fails at 4K too. REJECTED the
  live 4K u13 row: a 4K 60 s fixture set for a container-level property the emulation shows identical; 4K live stays
  UNMEASURED (said so in the docs).
- A7 [SHOULD] the forward seeks have no overshoot check -- ACCEPT the attack's alternative: the half-frame aim at runStep
  ONLY (AM4). E5: zero measured cost (m7 = proto3 everywhere); every remaining new landing is guarded by F1's overshoot
  re-seek (VideoPlayer.cpp:1426-1443). The VFR forward test it asks for is E5's sweep (VFR+B-frames MP4 / MKV, committed
  vfr / vfrgap); no committed test -- the forward sites are byte-identical to the base, pinned by G8 (ii).
- A8 [SHOULD] T2e has no oracle -- ACCEPT (AM9, E10): after close() + stopThread the case asserts keyRels == {0, 30, 60} and
  gopEst == 30 (RED on the base: {0} / 90). The reader grep is gate G8 (iii) (E12: none outside VideoPlayer.{h,cpp} today).
- A9 [SHOULD] Cue time vs decoded key time -- REJECT the premise, ACCEPT both fixes (E4). Matroska has one timestamp domain
  (block time = PTS) whether an entry comes from the Cues or from reading; on open-GOP B-frame streams its index equals the
  decoder's key frames exactly (the MP4 DTS index is the skewed one, pre-existing). Adopted: every Matroska row of T2 asserts
  keyRels == decodedKeyRels(file) -- the index-base vs firstPts_ check the attack asks for (AM8); an open-GOP B-frame row
  FX8 (the committed opengop fixture remuxed: parity 106 vs 107; RED by keyRels {0}).
- A10 [NIT] the shape REQUIREs couple the suite to the demuxer -- ACCEPT as GA2's split: one "mkvidx fixture shape" case
  (AM6) that prints avformat_version() and says "re-register, not a product defect"; the product cases carry no shape
  REQUIRE. REJECTED the soft skip (a silently skipped shape leaves a vacuous product test green) and the version-in-name.
- A11 [NIT] mutants -- ACCEPT m6 (E7: caught by T3 and T3b). m7 is not a mutant: it is the adopted design (AM4); its inverse
  m8 (the plan's forward-site aim) is recorded inert (E5). REJECTED "T3 PASS is vacuous": T3 runs FX4 / FX4e (Matroska);
  its PASS on m1-m3 / m5 is the intended separability (items 1-2 never act on an intra file whose index is complete).

## 2. Seat mkvidx-gates-attack

- GA1 [MUST] parity guarded only for Loop reverse -- ACCEPT: committed T4 -- PingPong 25 s and a reverse-to-forward flip at
  CAP 21 K 3 on FX1 / FX2 / FX3 vs their MP4s, plus PingPong at CAP 164 on FX1 / FX3 with the one-time-split bar (AM12,
  E9); VP9 is A2's FX7 row; G6 (f) / (g) get the pre-registered bar [PP-PARITY] (AM14). REJECTED a MOV / ProRes row:
  ProRes is codec-intra (VideoPlayer.cpp:275-276), i.e. T3's intra DEMAND path, which T3 runs on a QuickTime 1/600 file
  (FX6); a MOV index is complete at open, so items 1-2 never act on it (E6: HEAD = m7 on every MOV / MP4 row).
- GA2 [MUST] T2e is no RED; the shape checks depend on FFmpeg -- ACCEPT (A8 + A10).
- GA3 [MUST] base inconsistent; RED from a scratch rig -- ACCEPT IN PART. Base pinned to fa9604d (the lane's files are
  byte-identical to the evidence base, VERIFIED). The builder pastes the RAW RED output (the relink rig on fa9604d) and the
  GREEN output of every new case, plus each mutant's diff inline (1-3 lines) with its result -- the gop2 precedent
  (ruling-gop2.md:369-372, gop2.md:6). G3's pass rule: printed values == the G3 table only when the build's libavformat is
  62.3.100 (the shape case prints it); on any other FFmpeg the assertions alone decide and the values are INFO (AM13, G3).
  REJECTED committing the relink script / mutant patch files: the rig is a scratch tool by precedent (gop2.md:200) and the
  inline diffs make the matrix reproducible from the repo.
- GA4 [SHOULD] fixture recipes not in the repo; FX1 / FX2 same size -- ACCEPT IN PART: every fixture's exact command goes
  into a comment above the mkvidx cases (the repo's convention: tests/test_video_player_open.cpp:73; tests/ has no fixture
  script) (AM5); the shape case asserts the structure (keyCounts, open-time index shapes, FX1 {0} vs FX2 {0, 250}) (AM6).
  REJECTED the script file, and "same size is suspicious": -cues_to_front re-orders the same elements; the files differ
  (sha) and behave differently (HEAD: FX2 opens with 2 entries and intra -- the X1 freeze; FX1 with 1).
- GA5 [SHOULD] the probe bars are not truly pre-registered -- ACCEPT (AM14): decision (ii)'s re-register escape is deleted;
  [CPU] FAIL with [PARITY] PASS is a recorded verdict with a pre-stated consequence; the bars are frozen in the committed
  JSON (blob hash recorded before the first launch); the share mapping: (d) = 4 x 1080p at min(2 GiB, RAM / 16) = 2 GiB on
  the 32 GB probe Mac -> ~164 frames per player (the plan's share) -> emulated g250 CAP 164 B / A ratio 0.20 (K 3) / 0.55
  (K 5); live 1080p decode ~K 4.7 (ruling-gop2 :122) -> expect ~0.5 against the 0.7 bar; negative controls on the A arm,
  [CTRL-A] (i)-(iii). The A arm cannot "fail [CPU]" (a B / A ratio): its control is (iii).
- GA6 [SHOULD] item 3's MP4 non-regression is INFO-only -- ACCEPT: item 3 now touches only the run seek (AM4), so the
  forward paths and the golden traces are byte-identical to the base; committed guard T3b pins the run aim on four B-frame /
  VFR / open-GOP MP4s at the base's values (E6; m6 fails it, E7) (AM11); G7 adds a u8 B-vs-A late tripwire (u7 already has
  [VU7] "B late <= A late", probe-vupload-ab.py:158) (AM15).
- GA7 [SHOULD] loose bars, one GOP size -- ACCEPT IN PART: T2's parity table covers GOP 250 / 113 (scene-cut) / 30 / 60
  (VP9) / 10 (open-GOP B-frames) (AM8); every parity bar is relative to the MP4 arm of the same run (shown >= mp4 - 1, late
  <= mp4 + 1, decodes <= mp4 x 1.02) -- tighter than absolute bars and robust to an FFmpeg change (AM7, AM8). REJECTED T1
  on FX5 / FX3: T1 tests the front-Cues intra verdict; FX3 / FX5 are end-Cues files (T2's job) and the gap rule across GOP
  sizes is P's.
- GA8 [NIT] interleaving / launches in prose; synthetic selftest; no content check in (d)-(g) -- ACCEPT IN PART: the
  selftest gains a too-few-launches TSV (AM14 (7)). Already in the tooling (VERIFIED): A B A B rounds
  (probe-vupload-ab.sh:3), burners -> tainted round re-run (:16, :29-61), every rule needs >= 5 launches per arm
  (probe-vupload-ab.py:158, :170). REJECTED content checks in (d)-(g): content identity is the unit tests' byte-for-byte
  mismatches == 0 on every parity row; (a)-(c) keep their captures.

## ARCHITECT RULING (s-rta-1002b)

The amendments OVERRIDE the plan body where they differ; everything else in plan-mkvidx.md stands.

### Amendments

- AM1 Base. Build on main fa9604d. RED = the committed new test TUs compiled against fa9604d's src/media with the gop2
  relink rig (the main build's flags.make / link.txt read-only, TEST_FIXTURES_DIR repointed). P's RED is its compile error
  (no keyIndexFrom) -- the gop2 T1a precedent.
- AM2 Item 1 as the plan, and readKeyIndex starts with `if (formatCtx_ == nullptr || videoStreamIndex_ < 0) return;` (the
  prototype dereferences formatCtx_ unguarded; the plan's text promised the no-op).
- AM3 Item 2's trigger is the triple (entries, timestamp of entry 0, timestamp of entry entries - 1), read with
  avformat_index_get_entries_count / avformat_index_get_entry (a null entry = INT64_MIN): rebuild iff it differs from the
  last build's. Members `int keyIndexEntries_ = -1; int64_t keyIndexFirstTs_ = INT64_MIN, keyIndexLastTs_ = INT64_MIN;`
  (open() before start(), the decode thread after -- the VideoPlayer.h comment says so). `readKeyIndex(false);` stays the
  FIRST statement of decodeStep. The once-per-player witness log line as the plan.
- AM4 Item 3 = the RUN seek only. runStep (VideoPlayer.cpp:1352) becomes
  `seekToTimestamp(ptsOfRel(r.seekFrom) + 0.5 * frameDur_ - runSeekBackSec_);` with the comment "mkvidx: aim at the frame's
  middle -- container times are rounded (Matroska 1 ms, QuickTime 1/600) and the conversion truncates; relOf rounds at the
  half. F1: further back after an overshoot". forwardStep :898 `seekToTimestamp(ptsOfRel(nextRel));` and forwardIdle :1543
  `seekToTimestamp(ptsOfRel(target));` stay byte-identical (they have no overshoot re-seek). No seekToRel helper (one call
  site). Clock-time seeks unchanged. Fork D1 is amended accordingly; D3 stays rejected.
- AM5 Fixtures (tests/fixtures; brew FFmpeg 8.0; every command run twice -> byte-identical; all with
  `-hide_banner -loglevel error -y`):
  - FX1 video_h264_gop250_64x64.mkv, 47,251 B, f3efd8c1ef9b8fb1 = `ffmpeg -i video_h264_gop250_64x64.mp4 -c copy -fflags
    +bitexact <out>` (Cues at the end)
  - FX2 video_h264_gop250_cuesfront_64x64.mkv, 47,251 B, 2cc9c9f8ec94f3f3 = FX1's command + `-cues_to_front 1`
  - FX3 video_h264_scenecut_64x64.mkv, 50,486 B, a9c17605b038c2a3 = the same remux of video_h264_scenecut_64x64.mp4
  - FX4 video_h264_allintra_cuesfront_64x64.mkv, 23,485 B, f310619d1c3af1aa = remux of video_h264_allintra_64x64.mp4 +
    `-cues_to_front 1`
  - FX4e video_h264_allintra_64x64.mkv, 23,485 B, 2802512b4d266b2d = the same remux WITHOUT -cues_to_front (NEW, A1)
  - FX5 video_h264_gop30_64x64.mkv, 16,277 B, 69d77cbff6420f5b = remux of video_h264_gop30_64x64.mp4
  - FX6 video_hap_tb600_64x64.mov, 100,841 B, 83cfa97a7a1066d3 = `ffmpeg -f lavfi -i testsrc2=size=64x64:rate=30 -t 3 -c:v
    hap -video_track_timescale 600 -fflags +bitexact <out>`
  - FX7m video_vp9_gop60_64x64.mp4, 48,924 B, 11a157e45ecc9fc6 = `ffmpeg -f lavfi -i testsrc2=size=64x64:rate=30 -t 10
    -c:v libvpx-vp9 -g 60 -keyint_min 60 -deadline good -cpu-used 4 -crf 45 -b:v 0 -row-mt 0 -threads 1 -fflags +bitexact
    <out>` (NEW, A2)
  - FX7 video_vp9_gop60_64x64.webm, 49,568 B, c1348e3b9ffbcab5 = remux of FX7m (NEW, A2)
  - FX8 video_h264_opengop_64x64.mkv, 14,620 B, e83125cd21d1ca3f = remux of video_h264_opengop_64x64.mp4 (NEW, A9)
  Every command goes verbatim into a comment above the mkvidx cases in tests/test_gop_cache_store.cpp, with "brew FFmpeg
  8.0 / libavformat 62.3.100; the committed bytes are the fixtures (R10)". No generator script.
- AM6 NEW case (test_gop_cache_store.cpp) "mkvidx fixture shape: the Matroska fixtures' lazy / front-Cues index and the
  codec structure the mkvidx cases rely on -- an FFmpeg that changes them turns THIS case red: re-register, not a product
  defect". It prints avformat_version() and REQUIREs: FX1 / FX3 / FX5 / FX7 / FX8 open with keyRels {0}; FX2 opens with
  {0, 250} (so FX1's index differs from FX2's); FX4 opens intra-only with 30 index entries; FX4e opens intra-only with >= 2
  index entries (the probe read-ahead; 8 on 62.3.100); FX4 / FX4e / FX6 every packet and frame a keyframe (keyCounts);
  FX6's stream time base 1/600; FX7 / FX7m keyCounts 5 / 5; decodedKeyRels FX1 {0, 250}, FX3 {0, 37, 150, 190, 213, 262},
  FX5 {0, 30, 60}, FX7 {0, 60, 120, 180, 240}, FX8 {0, 10, 20, 30, 40, 50}. The product cases (T1-T4, T2e) carry NO shape
  REQUIRE (the plan's "each test asserts its fixture's shape first" is withdrawn). Helper `decodedKeyRels(file)` = a
  forward decode keeping lround((pts - first output pts) / fd) of every AV_FRAME_FLAG_KEY frame, fd as forwardDecode's.
- AM7 T1 bars relative to the MP4 arm of the same run: FX2 intraOnly false; late <= mp4.late + 1; shown >= mp4.shown - 1;
  decodes <= mp4.decodes x 1.02; mismatches 0; guard: FX4 opens intra-only.
- AM8 T2 (one case): (a) FX1 reverse start, 4 decodeSteps: keyRels == decodedKeyRels(FX1) == {0, 250}, gopEst 250;
  (b) FX1, big budget, 9 s forward (Show): seeks 0, keyRels {0, 250}, gopEst 250, late 0, every consecutive pair of shown
  frames differs by 1; (c) the parity table -- Loop reverse from the wrap in the T1 harness (openCapped CAP 21, Emulated
  K 3), each Matroska file against its MP4 source in the same run: FX1 9 s, FX3 9 s, FX5 2.8 s, FX7 vs FX7m 9 s, FX8 1.8 s;
  per row: keyRels after the run == decodedKeyRels(mkv); gopEst == the largest gap of decodedKeyRels; decodes <= mp4 x 1.02;
  late <= mp4.late + 1; shown >= mp4.shown - 1; mismatches 0. Replaces the plan's T2a-T2d.
- AM9 T2e (test_video_decode_trace.cpp): the threaded reverse body factored into a function of the fixture (the MP4 case
  unchanged), run on FX5 with the existing bars (shown >= 100, violations 0, hits > 0, direction changes >= 6) PLUS, after
  `p.close()` and a test-access `join(p)` = `p.thread_.stopThread(3000)`: keyRels == {0, 30, 60} and gopEst == 30. RED on
  the base. Only if G2 reports the oracle's read: replace it by a decode-thread-written VideoStats rebuild counter asserted
  >= 1, and say so in the lane report.
- AM10 T3 adds FX4e (0.9 s): intraOnly true; late <= 2; shown >= 26; decodes <= 1.1 x shown + 3; mismatches 0.
- AM11 NEW guard "mkvidx T3b: a run's half-frame aim never costs a B-frame / VFR / open-GOP MP4 more than the base": the T1
  harness, 9 s Loop reverse, CAP 21 K 3, on video_mpeg4_bf2_64x64.mp4, video_h264_opengop_64x64.mp4,
  video_h264_vfr_64x64.mp4, video_h264_vfrgap_64x64.mp4; per file decodes <= base, late <= base, shown >= base - 1,
  mismatches 0, with the base values (fa9604d, E6) in the case's comment: 561 / 4 / 269, 430 / 8 / 270, 425 / 8 / 270,
  327 / 103 / 144 (decodes / late / shown). A GUARD, not a RED: it passes on the base by construction; its teeth = m6.
- AM12 NEW "mkvidx T4: a Matroska file ping-pongs and flips like the same stream in MP4", Emulated K 3: (i) PingPong 25 s
  from frame 0 (setLoopMode(PingPong), no reverse flag) at CAP 21 and (ii) Loop reverse from the wrap for 4 s, then
  setReverse(false), to 8 s, at CAP 21 -- on FX1, FX2, FX3 vs their MP4: decodes <= mp4 x 1.02, late <= mp4.late + 1, shown
  >= mp4.shown - 1, mismatches 0; (iii) PingPong 25 s at CAP 164 on FX1 and FX3: seeks <= mp4.seeks + 1, late <= mp4.late +
  1, decodes <= mp4 x 1.05 (F7's one-time split, explained in the case's comment).
- AM13 Mutants (scratch copies only; each diff pasted inline in the lane report with its actual result; expected below;
  teeth rule: each of m1-m6 fails at least one case, else the gate set is missing a tooth -> report before merge):
  m1 keyIndexFrom without `&& k.gopFrames == 1` -> P, T1, T4 fail; m2 no `readKeyIndex(false)` -> T2, T2e, T4 fail; m3 the
  rebuild in seekToTimestamp instead of decodeStep -> T2 (b) fails; m4 no half frame -> T3 fails; m5 the rebuild re-derives
  intra with the old rule -> T1, T2, T4 fail; m6 minus half a frame -> T3, T3b fail; m8 the aim also at the two forward
  sites (the plan's proto3) -> passes all (inert, E5; recorded); m9 the count-only trigger -> passes all (defensive
  superset; recorded).
- AM14 Item 4 (u13): (1) decision (ii) is DELETED. (2) NEW rule [PP-PARITY]: B (f) median late <= B (g) median late + 15.
  (3) NEW [CTRL-A] lines, evaluated first and printed as INFO verdicts that fix the interpretation (not lane defects): (i) A
  (a) median uploads/s < 20, else STOP (the plan's decision (i)); (ii) A (c) fails the [HAP] bars (median uploads/s < 28.5
  or median decoded_per_upload > 1.25), else [HAP] is non-discriminating in this run -- recorded, check hap1080_tb600.mov's
  time base (1/600) before judging B; (iii) A (d) median decoded_per_upload >= 1.3 x A (e)'s, else the live app does not
  reproduce X2's cost and a [CPU] FAIL is the expected outcome. (4) [CPU] (bar 0.7, frozen; mapping in GA5): FAIL with
  [PARITY] PASS = a recorded verdict "live Matroska cost ratio R", NOT a merge blocker (the fix rests on X1 + parity); the
  docs then carry R in place of the emulated "1.3-7x"; never re-registered. (5) Every u13 rule requires >= 5 launches per
  arm. (6) Bars frozen: "_u13" in probe-vupload.json is committed with item 4; Harmony records `git rev-parse
  HEAD:.harmony/probe-vupload.json` before the first G6 launch; a changed blob voids the run. (7) `--selftest`: three
  synthetic u13 TSVs -- all-pass (exit 0; [FREEZE] [MKV] [HAP] [CPU] [PARITY] [GUARD] [PP-PARITY] PASS), frozen-B (a) (exit
  1; exactly one [FREEZE] FAIL), too-few (4 launches per arm; exit 1; every u13 rule FAIL, none PASS).
- AM15 G7 adds [G7-TRIP]: per u8 cap group (default budget, 256 MB), B median late <= A median late + 15 (from the tool's
  printed A / B medians); FAIL -> one re-run of that pair; a repeat FAIL = lane defect.
- AM16 Docs (plan section 6) amended: (a) rendering.md and the new Pitfall describe item 3 as "a RUN's seek aims at the
  frame's middle (runStep); the forward repositioning seeks (forwardStep after hits, forwardIdle) keep the exact nominal time
  -- they have no overshoot re-seek; clock-time seeks keep the clock"; (b) rendering.md's measured table adds E2 (AV1 / VP9
  in Matroska == MP4), E4's residual (a VFR B-frame Matroska stream +1.6-4 % decodes), E8 (the 4K shares; "4K live:
  unmeasured") and E9 ("PingPong on an end-Cues / cue-less Matroska file at shares >= 164: the first post-turn window splits
  once -- +1 seek, +0.3-2.2 % decodes per 130 s, late = MP4's -- F7"); (c) the CLAUDE.md index line reads "NN. A run's frame
  seek aims at the frame's middle; the keyframe index is the demuxer's live one -- before touching `runStep`'s seek,
  `readKeyIndex` or the intra-only verdict." (NN = Harmony's); (d) the Pitfall's guards: test_gop_cache.cpp "mkvidx P";
  test_gop_cache_store.cpp "mkvidx fixture shape" / T1 / T2 / T3 / T3b / T4; test_video_decode_trace.cpp threaded reverse
  on a Matroska file; live probe-vupload u13; (e) VideoPlayer.h's comments name keyIndexFirstTs_ / keyIndexLastTs_.
- AM17 FILED (not this lane): F6 an all-intra stream whose open-time index holds < 2 entries goes the cache path (E1
  residual; candidate: a one-way promotion at the first rebuild while nothing is cached; trigger: an all-intra MKV / WebM
  whose open log line lacks ", intra-only"). F7 the PingPong first-turn window split (E9; candidate: drop the forward-retained
  frames above the new retention when a rebuild lowers gopFramesEst_, or size provisional retention from
  kDefaultGopFrames; trigger: a u13 (f) vs (g) late difference or a performer report). INFO: a 4K GOP-250 reverse at the
  floor share (CAP 8, K 1) shows 113 of ~900 frames for every container (gop2 F4).
- AM18 ERRATA: (1) "every Matroska row of X1-X4 equals the same stream in MP4 count for count" -> true for X1, X2, X4 and X3
  at CAP 21; X3 at CAP >= 164: +1 seek, +0.3-2.2 % (E9). (2) "T2e ... Not a RED" -> RED (AM9). (3) G1 "+5" -> "+8" (P,
  fixture shape, T1, T2, T3, T3b, T4, T2e). (4) Decision (ii)'s re-register escape -> deleted (AM14). (5) The X5 row "MJPEG /
  all-intra H.264 in MKV" is the end-Cues case (E1). (6) R6's "F1's overshoot re-seek covers the exact-half edge" now holds
  for every new landing (AM4). (7) The mutant matrix is AM13's.

### BUILD STAGES (final; one builder context)

- Stage A -- C++, branch lane/mkvidx from fa9604d:
  - A0 relink rig on fa9604d: compile P / shape / T1 / T2 / T3 / T3b / T4 / T2e against fa9604d's src/media; paste the RAW
    output (expected: P compile error; shape PASS; T3b PASS; T1 / T2 / T3 / T4 / T2e FAIL at the G3 table's RED values).
  - A1 item 1 (keyIndexFrom; readKeyIndex(true) + AM2) + P.
  - A2 item 2 (AM3; the witness line).
  - A3 item 3 (AM4).
  - A4 the ten fixtures + the recipe comment + the seven store / trace cases (AM6-AM12).
  - Self-checks: ctest -j1 100 %; TSan on test_gop_cache_store / test_video_decode_trace / test_gop_cache 0 reports; the
    mutant matrix (AM13); the G8 greps; the gop2 and mkvidx printed lines vs the G3 table.
  - Commit c1 = Stage A (C++, fixtures, tests).
- Stage B -- probe + docs:
  - c2 = item 4 as amended (AM14): probe-video.py remux specs, probe-video.json fixtures, probe-vupload.py u13 scenes +
    helpers, probe-vupload.json "_u13" bars + layers, probe-vupload-ab.py u13 block + [CTRL-A] + [PP-PARITY] + the
    selftest; `--selftest` PASS; one INFO live smoke of u13 under the lock.
  - c3 = docs (plan section 6 as amended by AM16) + the lane report (RED / GREEN raw output, mutant diffs + results, the
    ctest -N delta, the G3 table filled).
- Fence (unchanged): no edit to Clip.h / Layer.h / Composition.h (tsan-r5), AudioEngine.cpp / DeviceGuard.cpp /
  test_log_line_lint.cpp (hyg); pitfalls.md / CLAUDE.md edited by quoted text on fa9604d.

### G3 value table (libavformat 62.3.100; GREEN = the lane design = m7 measured; RED = fa9604d)

| case / row | GREEN | RED |
|---|---|---|
| P | 16 assertions PASS | does not compile (no keyIndexFrom) |
| fixture shape | PASS, prints 62.3.100 | PASS |
| T1 CAP 21 K 3 9 s | MP4 late 0 shown 268 decodes 2186; FX2 intra 0, 0 / 268 / 2186, mism 0; FX4 intra 1 | FX2 intra 1, 875 / 1 / 3240 |
| T2 (a) FX1 | open {0} gop 300 -> {0, 250} gop 250 | stays {0} gop 300 |
| T2 (b) FX1 9 s forward | seeks 0, {0, 250}, gop 250, late 0, shown 271, step 1 | {0} gop 300 (late 0, shown 271) |
| T2 (c) FX1 9 s | mp4 2186 -> mkv 2186; late 0 / 0; shown 268 / 268; gop 250 | mkv 2425; {0} 300 |
| T2 (c) FX3 9 s | 910 -> 910; 0 / 0; 268 / 268; {0, 37, 150, 190, 213, 262} gop 113 | 2141; {0} 300 |
| T2 (c) FX5 2.8 s | 202 -> 202; 0 / 0; 83 / 83; {0, 30, 60} gop 30 | 197; {0} 90 |
| T2 (c) FX7 9 s | 1044 -> 1044; 0 / 0; 266 / 266; {0, 60, 120, 180, 240} gop 60 | 2212; {0} 300 |
| T2 (c) FX8 1.8 s | 107 -> 106; 0 / 0; 54 / 54; {0, 10, 20, 30, 40, 50} gop 10 | 108; {0} 60 |
| T2e FX5 threaded | keyRels {0, 30, 60} gop 30 (exact); shown ~159, violations 0, hits ~160, dir 7 (bars only) | {0} gop 90 |
| T3 FX6 2.5 s | intra, 0 / 75 / 78 | 64 / 38 / 834 |
| T3 FX4 0.9 s / FX4e 0.9 s | 0 / 27 / 30 each | 4 / 17 / 324 each |
| T3b (bars = base) | mpeg4_bf2 561 / 4 / 269; opengop 430 / 8 / 270; vfr 425 / 8 / 270; vfrgap 318 / 103 / 144 | same, vfrgap 327 (PASS) |
| T4 (i) PingPong CAP 21 25 s | gop250 MP4 2634 late 5 shown 747 = FX1 = FX2; scenecut 1413 / 0 / 749 = FX3 | FX1 2891 / 0 / 749; FX2 4048 / 1180 / 452; FX3 2624 / 0 / 749 |
| T4 (ii) flip CAP 21 8 s | gop250 MP4 1365 / 30 / 229 = FX1 = FX2; scenecut 443 / 0 / 237 = FX3 | FX1 1592 / 14 / 233; FX2 1677 / 317 / 110; FX3 1414 / 21 / 231 |
| T4 (iii) PingPong CAP 164 25 s | FX1 795 seeks 3 = MP4; FX3 687 seeks 5 vs MP4 668 seeks 4; late 0 | FX1 795 / 3 (passes); FX3 745 seeks 3 (fails decodes) |

(decodes / late / shown unless labelled; T2e's shown / hits are threaded and judged by their bars only.)

### FINAL GATE LIST (Harmony, after the merge; pre-registered -- copy these strings)

- G1 `cmake --build build -j$(sysctl -n hw.ncpu)` then `ctest --test-dir build -j1` -> 100 %; `ctest -N` count = before + 8
  (mkvidx P, mkvidx fixture shape, mkvidx T1, mkvidx T2, mkvidx T3, mkvidx T3b, mkvidx T4, the threaded reverse case on a
  Matroska file).
- G2 TSan build (RelWithDebInfo, -DADNA_SANITIZE=thread), TSAN_OPTIONS=halt_on_error=0:abort_on_error=0:
  test_gop_cache_store, test_video_decode_trace, test_gop_cache -> rc 0 and 0 "WARNING: ThreadSanitizer"; T2e's oracle
  assertions PASS inside that run (the decode-thread rebuild ran under TSan).
- G3 `build/tests/test_gop_cache_store "mkvidx*"` and T2e's printed values == the G3 table's GREEN column when the shape case
  prints libavformat 62.3.100 (T2e: keyRels / gop exact, shown / hits by bars); on another FFmpeg the assertions alone decide
  (values INFO). `build/tests/test_gop_cache_store "gop2*"` lines identical to gop2.md's GREEN block (T1b 2186 / 2776 /
  2349 / 1698 / 2186 / 2427; T1c totals 0 / 337 / 428 / 0 / 31; T1d 1140 / 1398; T3 (a) 120 of 120 and 0 0 0 0; T3 (b) 23 /
  0 / 425) -- VERIFIED identical on m7.
- G4 `python3 .harmony/probe-vupload-ab.py --selftest` -> SELFTEST PASS, including the three u13 cases (AM14 (7)).
- G5 `shasum -a 256` of the ten fixtures == AM5's prefixes.
- G6 live A/B, lock held, no Output window: record `git rev-parse HEAD:.harmony/probe-vupload.json`, then
  `probe-vupload-ab.sh <main app> <lane app> 5 .harmony/probe-vupload.sh u13_container_reverse <out>` (A B A B, burners
  checked, ~25 min). Read [CTRL-A] first ((i) STOP rule; (ii) / (iii) fix the interpretation). Then [FREEZE] [MKV] [HAP]
  [PARITY] [GUARD] [PP-PARITY] PASS -- a FAIL is a lane defect ([HAP] is INFO-only when [CTRL-A] (ii) said
  non-discriminating). [CPU] PASS, or FAIL with [PARITY] PASS = the recorded verdict "live ratio R" (docs updated; not a
  blocker). Decision (iv) unchanged: (f) B mkv late - A mkv late > 60 per window -> Q1 goes to Boris with the number.
- G7 `probe-vupload-ab.sh <main> <lane> 5 .harmony/probe-vupload.sh u7_reverse_pingpong,u8_reverse_column_1080x4 <out>` at
  the default budget, then u8 again with VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=256: the existing [VU7] / [ABS] / [GC6] /
  [GC7] / [GC8] / [GC12] and "_u8" rules PASS for B; [G7-TRIP] per u8 cap group B median late <= A median late + 15 (FAIL ->
  one re-run of the pair; a repeat FAIL = lane defect).
- G8 greps on the merged tree: (i) `grep -c "+ 0.5 \* frameDur_ - runSeekBackSec_" src/media/VideoPlayer.cpp` = 1 and that
  line lies inside VideoPlayer::runStep; (ii) `grep -c "seekToTimestamp(ptsOfRel(nextRel));" src/media/VideoPlayer.cpp` = 1
  and `grep -c "seekToTimestamp(ptsOfRel(target));" src/media/VideoPlayer.cpp` = 1 (both = 1 on the base, VERIFIED);
  (iii) `grep -rn -e keyRels_ -e gopFramesEst_ -e keyIndexEntries_ -e keyIndexFirstTs_ -e keyIndexLastTs_ src | grep -v
  -e "^src/media/VideoPlayer.cpp:" -e "^src/media/VideoPlayer.h:"` prints nothing; (iv) the first statement of
  VideoPlayer::decodeStep is `readKeyIndex(false);`.
- G9 docs: `wc -c CLAUDE.md` <= 24,002 (fa9604d); docs/claude/rendering.md no longer contains "MKV / WebM hold a 1-entry
  index after open"; docs/claude/pitfalls.md Pitfall 62 no longer contains "(MKV / WebM keep a 1-entry index after open";
  the new Pitfall (Harmony's number) and the CLAUDE.md index line of AM16 (c) are present.

### Risks of this ruling

- RR1 (the strongest counterargument, against AM4): the forward repositioning keeps the truncation -- when its target is
  itself a keyframe stored above its nominal time (Matroska rounding up by < 1 ms, QuickTime 1/600) it lands a GOP low and
  decodes a GOP (a hold if decode-bound), once per flip / bottom turn. It loses because a reposition target is a keyframe
  with probability ~1 / GOP and the cost is decodes, never a wrong or skipped frame; across 2,562 sweep runs and every flip /
  PingPong row the forward-site aim changed no count; and it would be the only new landing without an overshoot re-seek.
  Cheapest test if a Matroska flip-forward hitch is reported: E5's sweep with m8 on that clip.
- RR2 AV1 is scratch-verified only (libaom, SVT-AV1, FFmpeg's matroskaenc); mkvmerge / GStreamer layouts (BlockGroups,
  sparse Cues) are unmeasured. Sparse Cues are landing points too (A4), and T2's keyRels == decodedKeyRels check flags a
  key-flag disagreement on any fixture that has one. Boris's own files (plan section 8) are the live check.
- RR3 T3b pins today's MP4 reverse counts on four fixtures; a later lane that legitimately changes them re-registers
  (precedent: gop2 T1d's "main" constants).
- RR4 F6 / F7 residuals -- measured, bounded, filed.
- RR5 The T2e oracle's race-freedom rests on JUCE's thread-exit atomic (E10); G2 is the check; AM9 names the fallback.
- RR6 FFmpeg coupling is explicit: the shape case and G3's value equality are tied to libavformat 62.3.100; the product
  assertions are relative to the MP4 arm of the same run.

### QUESTIONS FOR BORIS (unchanged from the plan; defaults)

- Q1 Long single-layer ping-pong clips (> ~40 s) keep the brief top-turn hold MP4 already shows until follow-up F1. Ship
  mkvidx now? DEFAULT: ship now.
- Q2 Duration-less WebM (browser / streaming recordings) opens as a still frame today. DEFAULT: its own lane (F3), not mkvidx.

### Notebook (for Harmony)

- matroskadec's index is a contiguous prefix after any seek, and an all-intra Matroska file's open-time index is the
  find_stream_info read-ahead (2-8 entries): probe with a chain-of-seeks index dump ($R/idx4.c).
- A flip-point sweep (every frame across a GOP x files x shares x arms) is the cheap way to catch a forward-reposition seek
  regression that single flips miss ($R/e4.sh).
- A forced-flag arm (a test-access write of a verdict after open) prices a "wrong verdict" residual without constructing
  the input ($R/rcost.cpp, FORCE_NOT_INTRA).

STATUS: RULED -- accepted A2 A3 A6 A7 A8 A10 A11 GA1 GA2 GA5 GA6; in part A1 A5 A9 GA3 GA4 GA7 GA8; rejected A4; 18 amendments; ready_to_build yes
