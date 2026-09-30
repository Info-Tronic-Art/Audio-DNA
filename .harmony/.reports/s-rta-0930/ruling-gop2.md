# ruling-gop2 -- ARCHITECT RULING on the blind council's attacks, lane gop2 (s-rta-0930)

Architect, 2026-09-30. Plan: `.harmony/.reports/s-rta-0930/plan-gop2.md` (main 655d232). No app was launched. Every number
below comes from offline rigs linking the REAL `src/media/VideoPlayer.cpp` + FFmpeg 8.0 (scratch; index at the end).
VERIFIED = measured or read here; INFERRED = reasoned and labelled.

**VERDICT: plan-gop2 is SOUND WITH AMENDMENTS -- ready to build as amended in section 3.** Two attacks change the design:
- **decode-4:** a window's lead-in is measured from the container index's REAL keyframe, not from the grid of multiples of
  the longest GOP. On scene-cut files the grid made the in-place window cost up to +17 % decodes over main; the real
  keyframe makes it -4 to -9 %.
- **vj-4:** the landing gate admits only frames at or after the landing packet's pts. An open-GOP leading frame then can
  never be stored, by construction.

Everything else is tests, gate validity and docs. All 26 attacks are ruled: 23 accepted in whole or in part, 3 rejected
with measurements (decode-5, decode-6, gates-9). The amended patch was built offline and passes every committed suite
(store 523/22, golden forward traces 510/5 in 3 of 3 runs, pure 111/11) and every new pre-registered bar below.

**Provenance (read first).** The three papers the dispatch names (`attack-gop2-{decode,gates,vj}.md`) do NOT exist on
disk: the seats returned them in-band (structured output) and wrote no file. The papers were recovered verbatim from the
workflow journal `~/.claude/projects/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/
subagents/workflows/wf_d5f2da66-bc5/journal.jsonl` (type "result"; agents a389c8b156b72f05c decode, addd822c041e5813b
gates, ada5ad9900913252e vj; all three AMEND). Each attack is quoted in one line below. The same holds for lane bt2's
seats (no `attack-bt2-*.md` on disk), and `plan-tsan.md` is absent (the journal records plan:tsan as failed).

## 1. RULINGS, attack by attack

### decode seat (9)

- **decode-1 [MUST]** "The new store gate is validated only where it cannot fail; MPEG-4-in-MP4's 4/20 first-output
  mismatch is waved off." -> **REJECT the premise; ACCEPT the hardening (A2).**
  - The 4/20 is a best-effort-timestamp artefact (VERIFIED). Scratch `landing3` compares full Y+U+V checksums by pts: in
    `mpeg4_bf2.mp4` all 240 post-seek outputs equal the forward frame at the same pts. The 4 flagged outputs are the
    GOP-0 keyframe (pts 0) reporting best-effort 512 after a seek against 0 going forward. The player indexes by pts when
    present (VideoPlayer.cpp:1021).
  - Item 3 changes nothing on that file (VERIFIED). Its first output after every seek is itself key-flagged
    (landing2.log: key 20/20), so HEAD's gate opens on the same frame (VideoPlayer.cpp:1435-1436).
  - The fixtures the attack asks for already exist and run identity (VERIFIED): open-GOP H.264
    (tests/test_gop_cache_store.cpp:625, :646), the MPEG-4 part 2 B-frame MP4 (:662, :702) and the pts-less MPEG-4 AVI,
    shown AND resident (:718, :747-754). All pass on the AMENDED patch (523/22). The scratch identity harness on the same
    files (12-frame cache and whole file, K 5 and K 50) is identical across main's gate, the plan's gate and the amended
    gate: 0 mismatches, 0 bad residents.
  - A wider probe (VERIFIED): 11 files, 2,640 post-seek outputs. The files are x264 closed / open-GOP / B-pyramid
    open-GOP MOV, HEVC open-GOP, MPEG-2 open-GOP MOV, MPEG-4 part 2 MP4 and AVI, VP9, and intra-refresh MP4 (320x180
    and 64x64). No output has content found nowhere in the forward decode, and no output has a pts below the landing
    packet's. A2 makes that last property a rule instead of an observation.
- **decode-2 [MUST]** "The window is planned once; the share shrinks mid-lead-in (a player joins, memory pressure) --
  untested." -> **ACCEPT the test (A5, T1c "shrink"); REJECT re-planning or truncating windowLo in onRunFrame.**
  - VERIFIED: halving `budget.total` (21 -> 10 slots) during a lead-in at three moments gives late 428 / 428 / 251 on
    the patch against 482 / 469 / 322 on main, with identity 0/0 (0 bad residents of 10).
  - The farthest-next-use verdict already drops or replaces the window's own bottom (GopCache.h:255-260). A re-plan
    path would be new code with no measured benefit.
- **decode-3 [SHOULD]** "T1a(ii)'s prefetchAt 19 is not the live value." -> **ACCEPT (A3).** VERIFIED:
  `prefetchAt(21, 250, 1.8, 1000/30)` = 17 (16 at the live 1.76 ms). At the plan's u = 17 the Lead returns None. At
  u = pAt - 1 = 16 it returns Prefetch with target 232, windowLo 218, seekFrom 218, and None without a Lead.
- **decode-4 [SHOULD]** "The keyframe below a window is guessed as a multiple of the LONGEST GOP; use the real index."
  -> **ACCEPT (A1 + T1d).** VERIFIED on two scene-cut 64x64 fixtures: forced keys at 0 37 150 190 213 262, and
  x264-default cuts at 0 69 162 240. Six configurations each, CAP 13-21, K 2-5:
  - the plan's grid spends -0.2 % to +17.4 % decodes against main, and +55 % to +77 % with a 2x-overestimated decode
    time;
  - the real keyframe spends -4.3 % to -8.8 % against main, and +21 % to +29 % at 2x;
  - late is 0 and identity 0/0 in every arm;
  - on fixed-GOP files the two are identical (u8's fixture has keys 0 and 250).
- **decode-5 [SHOULD]** "Token starvation and CPU cost are unmodelled; pre-register a fallback; add a rig row with a
  busy render thread." -> **REJECT (measured).**
  - CPU: in the offline u8 at 256 MB x 4 (real decode threads, 2 rounds), decodes per upload fall from 12.2 to 10.9.
    Four 4K reversers decode ~450/s in both arms (443 vs 455) while the patch uploads 5.6x as many frames.
  - Busy render thread: with the rig's `LOADMS` 4 and 8 (a GL thread burning 4 or 8 ms per render frame) the patch
    stays at late 0 in 2 of 2 rounds each; main is at 38-57.
  - GC9 token: in the u8 shape `prefetchAt` equals the `urgent` threshold. Both are
    ceil(gop x decodeMs / frameMsEff x 1.2) whenever cap/2 <= that <= cap - 2 (GopCache.h:320-321, VideoPlayer.cpp:1317).
    So every PREFETCH plan there is urgent and never waits for the token, on main too (main plans only at u <= 11 < 16).
  - The fallback is already pre-registered: G4's PARTIAL / FAIL branches.
- **decode-6 [SHOULD]** "landedOnKeyPacket_ may go stale across re-seeks; reset it at :1345; add mutant m5." -> **REJECT.**
  - VERIFIED: every run starts through runStep's seek (VideoPlayer.cpp:1340-1343). `Run{}` defaults
    `seekPending = true` (GopCache.h:331), startDemand sets it (:1280) and the overshoot re-seek sets it (:1431).
  - The flags are reset inside `seekToTimestamp` after the flush, which covers its fallback seek too (:1590-1600). A
    run can never read another seek's flag, and :1345 runs right after that seek.
  - No fixture could tell a stale-flag mutant apart: MPEG-TS landings are key 0/20 and MP4 landings key 20/20
    (landing2.log).
- **decode-7 [SHOULD]** "E8 ran on scratch p6.py, not the clean patch; audit which threads call what." -> **ACCEPT (A16).**
  - G1 / G2 run on the committed diff.
  - Caller audit (VERIFIED). `seekToTimestamp` is called at :845 :874 :879 :890 (decodeStep / forwardStep), :1342
    (runStep) and :1528 (forwardIdle): all on the decode thread. `decodeNextFrame` is called at :899 :1357 :1535 on the
    decode thread, plus :249 inside `open()`. `open()` runs on MediaOpener's pool against an unpublished player, with no
    decode thread running (Pitfall 58).
  - A1's `keyRels_` follows `gopFramesEst_`'s pattern: written in `open()`, read on the decode thread.
  - This ruling re-ran all three committed suites on the amended patch (N1).
- **decode-8 [NIT]** "The VFR percentages are simulated." -> **ACCEPT the label (A14); REJECT a new test** (this lane
  does not change VFR behaviour).
- **decode-9 [NIT]** "The 192 -> 160 MB cliff: run G4 at 192 MB too." -> **ACCEPT (A11, G6b INFO).** VERIFIED offline
  under 4 ms of emulated GL work per frame: 192 MB gives late 0 / 0 (main 482 / 536); 160 MB gives 156 / 171 (main
  963 / 947).

### gates seat (9)

- **gates-1 [MUST]** "G5 omits u12 pixel identity and any capped identity check." -> **ACCEPT (A10: G5b runs u12 on B
  uncapped, at 256 MB and at 128 MB; G7 adds u6).** u12 is one 1080p reverser, so under a cap its window IS the
  in-place path (82 slots at 256 MB, gain > 0).
- **gates-2 [MUST]** "G4 can pass vacuously: no control validity, no per-launch ceiling." -> **ACCEPT validity (A7);
  REJECT the per-launch ceiling and a new live row.**
  - A per-launch ceiling contradicts GC13 (plan-gopcache.md:809-810: absolute bars are judged on the median of >= 5
    launches, per-launch values are INFO) and Harmony's no-re-threshold constraint on GC7.
  - The multi-GOP / Loop-wrap steady state is T1b's deterministic 9 s from the Loop wrap across both GOPs: main 69
    late, patch 0.
- **gates-3 [MUST]** "Burner and load control is one pre-run check." -> **ACCEPT a per-launch burner taint and an
  end-of-launch load log (A9); REJECT a load-average void.** The valid ab256 run's launches all started at load
  3.30-5.29 (evidence-0929b/ab-ab256/meta.txt). That is the app's own threads, so a load1 > 3 rule would void every
  launch.
- **gates-4 [SHOULD]** "The bytes bar has ~95 MB of slack; capped over_budget is never checked." -> **ACCEPT, as
  lane-safety conditions in G4's decision (A8).** VERIFIED: probe-vupload-ab.py:127-128 (floors = 4 x 8 x 3,110,400 B)
  and :142 (the capped rule is just `enough`). GC7's pre-registered rule text stays unchanged.
- **gates-5 [SHOULD]** "T1a's RED is a compile failure; (iv) cannot fail; avail0 == 0 is excluded." -> **ACCEPT the
  avail0 == 0 case (A4); REJECT the rest.**
  - The behavioural REDs are T1b / T1c / T1d by design; (iv) is a mutant guard (window > share).
  - The proposed monotonicity strengthening would fail on a correct design: 32 of 6,449 grid states are not monotone in
    decodeMs (VERIFIED). The two-iteration window always lands at or below the self-consistent fixed point, i.e. on the
    conservative side (INFERRED: the map is non-increasing in the window).
  - With contiguous u < pAt and behindCap 0, avail0 == 0 never occurs (0 of 6,449 states, VERIFIED). It does occur when
    behindCap > 0 (capSlots 40, pAt 38 at 7 ms, u 30), and then the result is None with or without a Lead (VERIFIED).
- **gates-6 [SHOULD]** "T1b pins the EMA, so the cold EMA at the failure is untested." -> **ACCEPT a deterministic
  version (A5: T1b (e) emulates the EMA from a 2x cold seed; (f) pins 2x); REJECT an unpinned config.**
  - The EMA is written only at :298 (seed 1 ms per megapixel = 2.07 ms at 1080p, against 1.76 ms measured) and :1378
    (every run decode). Forward decode never writes it (VERIFIED).
  - Emulated cold seeds of 0.5x / 1.18x / 2x give results identical to the warm case, because the first DEMAND run
    converges the EMA before the first PREFETCH plan.
  - An unpinned 64x64 run would measure ~0.05 ms per decode, making servedDuringLead 0: it would test main's window,
    and not deterministically.
- **gates-7 [SHOULD]** "E7 / E8 used scratch code; the GC9 token margin is one frame." -> **ACCEPT the builder evidence
  (A16); REJECT token counters and a mandatory u8h re-run.** The one-frame margin exists only at T1a(ii)'s unreachable
  pAt 19; with the live pAt every u8 plan is urgent (see decode-5).
- **gates-8 [SHOULD]** "The selftest RED is weak; G3 depends on a file absent from the worktree." -> **ACCEPT exact rule
  counts (A13); REJECT "absent".** The file exists at its absolute path in the main checkout (3,007 B; .gitignore:63),
  and G3 is Harmony's gate, run in main after the merge. G3 also runs the new summarizer on the REAL E10 artefact file.
- **gates-9 [NIT]** "Slowest >= 20 is saturated; gate on late and dec/up instead." -> **REJECT the bar change** (GC7 is
  pre-registered). dec/up is printed as a mechanism indicator with a prediction (A7).

### vj seat (8)

- **vj-1 [MUST]** "No answer for the low-RAM column: 160 / 128 MB still fail after the patch." -> **ACCEPT docs and G6b
  INFO (A11, A14); REJECT a degrade policy or a UI warning in this lane** (a product change: filed F4, Boris-facing).
  - Budget = min(2 GiB, RAM / 16) (GopCache.h:30-35). An 8 GB Mac gets 512 MB, halved under a memory-pressure warning
    (the 256 MB row) and floors only when critical (:47-52).
  - Below the feasible share the performer sees holds, never a wrong frame: identity was 0/0 in every capped and
    halved-share run here (VERIFIED).
- **vj-2 [MUST]** "A direction change, pause or speed change during the lead-in is never tested." -> **ACCEPT (A5, T1c).**
  VERIFIED on the real player, stepped, events at 1.0 / 1.6 / 3.0 s (main -> patch):
  - pause: late 57 / 33 / 58 -> 0 / 0 / 0;
  - 2x speed: 578 / 558 / 456 -> 513 / 337 / 218;
  - a decoder slowing from 5 to 3 decodes per frame: 68 / 38 / 0 -> 0 / 0 / 0;
  - a flip to forward, swept over 40 moments (0.6-8.4 s): post-flip late totals 362 (main) vs 376 (patch), max 43 in
    both; pre-flip late up to 69 on main, 0 on the patch; identity 0/0 in all 80 flip runs.
  - The post-flip holds come from the forward catch-up, pre-existing in both arms: VideoPlayer.cpp:885-892 decodes
    forward from the lead-in position whenever it is within `gopFramesEst_` frames, even across a keyframe. Filed F1.
- **vj-3 [SHOULD]** "A renamed .ts reaches the garbage path; the dismissal relies on file extensions." -> **ACCEPT the
  premise and a doc correction (A14); REJECT a code guard in this lane.**
  - VERIFIED: renamed files are probed as `mpegts` by content. Over 20 seeks each, H.264 and HEVC-open-GOP in TS output
    0 wrong frames; MPEG-4 part 2 in TS outputs 606 (up to 53 in a row).
  - The cache never keeps them. TS landing packets are never key-flagged, so the landing gate adds nothing: on
    `video_mpeg4_64x64.ts` all three gate variants are identical, with 0 bad residents.
  - The garbage that IS shown comes from main's DEMAND publish and forward seek (test_gop_cache_store.cpp:805-806), the
    same before and after this lane. Filed F3.
- **vj-4 [SHOULD]** "The landing gate widens trust in the demuxer; open-GOP leading frames become storable." ->
  **ACCEPT (A2).** Evidence as decode-1. A2 costs nothing: T3's numbers are identical with and without it (N8).
- **vj-5 [SHOULD]** "The 4K column: judge G6, and gate the merge on a Boris eyes check." -> **ACCEPT G6's u10 at 512 MB
  as a judged comparative gate (A12); REJECT the Boris merge gate.**
  - The change improves the column (VERIFIED offline, four 4K reversers in 2 GiB, 43 frames each): slowest player
    2.2 -> 12.6 frames/s, pooled 9 -> 51/s, late 2,200 -> 1,340, at the same decodes per second.
  - Q2 and B1 remain the post-merge check: Boris validates visible changes, merge gates are Harmony's.
- **vj-6 [SHOULD]** "T1a pins the formula's exact numbers; T1b never slews the decode time." -> **REJECT the band; ACCEPT
  the slew and misestimate configs (A5).** A pure planner's exact output on fixed inputs is the right unit contract;
  T1a (iii) and (iv) are the behavioural properties.
- **vj-7 [NIT]** "Mixed GOP lengths are untested." -> **ACCEPT** (T1d plus the offline scene-cut table, N7).
- **vj-8 [NIT]** "Locate the cliff under CPU load before merging." -> **ACCEPT, done here** (N10) plus G6b live.

## 2. NEW EVIDENCE (this ruling; VERIFIED unless labelled)

"Stepped" = T1b's deterministic emulation: the real player, at most K FFmpeg decodes per 120 Hz render frame, the decode
estimate driven by the harness; identity = every picked frame and every resident frame compared with a forward decode;
reruns are identical. "u8h" = the plan's offline u8: 4 real players with their decode threads, one Budget, a 120 Hz
reader. Arms run in one binary via env switches that reproduce main's c2 / c3; the main arm reproduces T1b's HEAD numbers
exactly (69 / 246, 56, 0). One stepped 9 s config takes ~0.12 s.

| # | measurement | main 655d232 | plan as written | AMENDED |
|---|---|---|---|---|
| N1 | committed suites, relinked against the patch | 523/22, 510/5, 111/11 | same (E8) | 523/22; 510/5 in 3 of 3; 111/11 |
| N2 | T1b (a) CAP 21 K 3 / (b) CAP 16 K 4 / (c) CAP 21 K 5: late, shown | 69,246 / 56,250 / 0,269 | 0,268 / 0,268 / 0,269 | = plan; identity 0/0/0 |
| N3 | T1b (d) CAP 21 K 2 (4.17 ms per decode, 2.3x the live 1.8 ms) | 403, 160 | 42, 253 | 42, 253 |
| N4 | T1b (e) EMA emulated from a 2x cold seed / (f) pinned 2x | 69 / 69 | 0 / 0 | 0 / 0 (2x pin: +11 % decodes) |
| N5 | T1c at 1.6 s: pause / speed 2x / shrink / slew / flip (total late) | 33 / 558 / 469 / 38 / 3 | 0 / 337 / 428 / 0 / 31 | = plan; identity 0/0/0 |
| N6 | flip swept over 40 moments: post-flip late sum, max | 362, 43 | 376, 43 | = plan (fixed GOP) |
| N7 | T1d scene-cut CAP 16 K 3 / CAP 13 K 4: decodes (late 0 in all) | 1213 / 1461 | 1388 / 1690 | 1140 / 1398 |
| N8 | T3 intra-refresh 64x64, 12-frame cache, per lap: seeks / misses / decodes | 169 / 77 / 3321 | 23 / 0 / 425 | 23 / 0 / 425 |
| N9 | u8h 1080p x4 @ 256 MB with LOADMS 0 / 4 / 8: late (2 rounds) | 68,68 / 38,44 / 57,53 | 0,0 / 0,0 / 0,0 | = plan (fixed GOP) |
| N10 | u8h @ 192 / @ 160 MB, LOADMS 4: late | 482,536 / 963,947 | 0,0 / 156,171 | = plan |
| N11 | u8h 4K x4 @ 2 GiB: slowest / pooled / late / decodes per upload | 2.2 / 9.0 / 2200 / 49.7 | 12.6 / 50.7 / 1340 / 8.9 | = plan |
| N12 | landing probe, 11 accepted-codec files, 2,640 outputs: below the landing pts / content nowhere in forward | -- | -- | 0 / 0 |
| N13 | renamed MPEG-TS, 20 seeks: wrong frames for H.264 / HEVC / MPEG-4 part 2 | 0 / 0 / 606 | same | same; the cache stays clean |

## ARCHITECT RULING (s-rta-0930)

These amendments OVERRIDE plan-gop2's body where they differ; everything not named here stands as written. Fence
additions: `.harmony/probe-vupload-ab.sh` and `tests/fixtures/video_h264_scenecut_64x64.mp4`.

**A1 (decode-4) -- the lead-in comes from the container index's real keyframe.**
- `src/media/GopCache.h`, NEW (plus `#include <iterator>`):
  ```cpp
  // s-rta-0930 gop2: the keyframe at or before r -- the container index's (sorted relative indices) when it has one at or
  // below r, else the longest-interval grid (no index / cue-less: on a scene-cut file the grid over-states the lead-in)
  inline int keyAtOrBefore(int r, int gopFrames, const std::vector<int>* keys)
  {
      if (keys != nullptr && !keys->empty() && (*keys)[0] <= r)
          return *std::prev(std::upper_bound(keys->begin(), keys->end(), r));
      return gopFrames > 0 ? (r / gopFrames) * gopFrames : r;
  }
  ```
  `struct Lead` gains `const std::vector<int>* keys = nullptr;`. In planPrefetch's two iterations,
  `key = keyAtOrBefore(lo, lead.gopFrames, lead.keys)` replaces `(lo / gopFrames) * gopFrames`.
- `src/media/VideoPlayer.h`, beside `gopFramesEst_` (:280): `std::vector<int> keyRels_;` (the container index's keyframes,
  relative and sorted; written in open(), read by the decode thread like gopFramesEst_).
- `VideoPlayer.cpp` `open()`, the index loop (:276-297): collect each keyframe entry's `timestamp`. After the loop:
  `keyRels_.clear()`, then push `std::max(0, (int) std::lround((ts - firstKeyTs) * timeBase_ / frameDur_))` for each.
  - The base is the FIRST keyframe's timestamp: index timestamps are DTS, and this removes the B-frame delay.
  - `keyRels_` stays empty without keyframe entries (the grid fallback, also under `LIBAVFORMAT_VERSION_MAJOR < 59`).
  - It is allocated once, in open() (MediaOpener pool, unpublished player: Pitfall 58), and never touched again.
- `planPrefetchRun` passes `GopCache::Lead{ decodeMsEma_, frameMsEff(), gopFramesEst_, &keyRels_ }`.
- Cross-lane: this hunk sits inside `open()` (:276-297), within the :83-316 range the plan's CROSS-LANE TOUCHES reserved
  for lane tsan's std::cerr edits. The nearest cerr block is :307-316, ten lines away, so non-overlapping hunks merge.
  Lane tsan has no plan this session. Harmony checks this at merge.

**A2 (vj-4, decode-1) -- the landing gate admits frames at or after the landing packet's pts.**
- `GopCache.h`, NEW:
  ```cpp
  constexpr int64_t kNoPts = INT64_MIN;   // == AV_NOPTS_VALUE (static_assert in VideoPlayer.cpp)
  // GC3 + s-rta-0930 gop2 R3: a run stores frames only after its keyframe -- a decoded frame flagged key, or, after a seek
  // that landed on a packet the demuxer flags key, a frame at or after that packet's pts (a recovery point's recovered
  // frames; an open-GOP leading frame precedes it and never opens the gate). No pts on the frame or the landing: the flag.
  inline bool storeGateOpens(bool frameKey, bool landedOnKeyPacket, int64_t framePts, int64_t landingPts)
  {
      return frameKey || (landedOnKeyPacket && framePts != kNoPts && landingPts != kNoPts && framePts >= landingPts);
  }
  ```
- `VideoPlayer.h`: beside the plan's two flags, `int64_t landingPts_ = GopCache::kNoPts;` (the landing packet's pts;
  decode thread only).
- `seekToTimestamp` (the plan's item-3 lines) also sets `landingPts_ = AV_NOPTS_VALUE;`. The landing branch in
  `decodeNextFrame` also sets `landingPts_ = packet_->pts;`.
- `onRunFrame` :1435-1436 becomes `if (GopCache::storeGateOpens((decodedFrame_->flags & AV_FRAME_FLAG_KEY) != 0,
  landedOnKeyPacket_, decodedFrame_->pts, landingPts_)) runSawKey_ = true;`, with `static_assert(AV_NOPTS_VALUE ==
  GopCache::kNoPts);` nearby.
- NEW case in `tests/test_gop_cache.cpp`, "gop2 R3: the store gate opens on a key frame, or at / after a demuxer-key
  landing's pts -- never before it, never without a pts":

  | frameKey, landed, framePts, landingPts | expected |
  |---|---|
  | true, false, kNoPts, kNoPts | true |
  | false, true, 1000, 1000 | true |
  | false, true, 1001, 1000 | true |
  | false, true, 999, 1000 | false |
  | false, true, kNoPts, 1000 | false |
  | false, true, 1000, kNoPts | false |
  | false, false, 1000, 0 | false |

  Tooth m6: dropping the pts clause fails the 999 / 1000 row.

**A3 (decode-3) -- T1a(ii) on reachable inputs.** Replace (ii) with:
- Setup: `const int pAt = GopCache::prefetchAt(21, 250, 1.8, 1000.0 / 30.0); REQUIRE(pAt == 17);`. n 300, Loop,
  cur = served = 249, frames 233..248 resident as Future (key 249 - f), so u = 16; capSlots 21, behindCap 0,
  minWindow 10.
- No Lead -> `RunKind::None` (avail0 5 < 10).
- `Lead{1.8, 1000/30.0, 250}` -> Prefetch, target 232, windowLo 218, seekFrom 218. Iteration 1: 5 + 12 - 1 = 16;
  iteration 2: lo 217, gain 11 -> 15.
- The same Lead with `keys = {0, 250}` -> the identical Run.
- The same Lead with `keys = {0, 37, 150, 190, 213, 262}` -> None (the real lead-in 213 -> 228 is 15 frames: gain 0).
- (iii) runs on these inputs and on the existing case's (tests/test_gop_cache.cpp:262-270).

**A4 (gates-5) -- T1a (iv) and (v).**
- (iv) grid, to reproduce: capSlots 8..64; u = 0..pAt-1 contiguous Future frames below served 249 (n 300); decodeMs
  {0.5, 1.0, 1.8, 3.0, 7.0}; gop 250; keys none and {0, 250}. For every Prefetch result:
  min(avail0, target + 1) <= target - windowLo + 1 <= capSlots - behindCap (VERIFIED: 0 violations in 6,449 states).
- (iv) ADD the avail0 == 0 state: capSlots 40 (behindCap 10 = `behindCapFor(Loop, false, 40, 0)`), pAt =
  `prefetchAt(40, 250, 7.0, 1000/30.0)` = 38 (asserted), u = 30. avail0 is 0, so the result is None with and without
  `Lead{7.0, 1000/30.0, 250}`.
- (v) `keyAtOrBefore`: (228, 250, {0,250}) = 0; (228, 113, {0,37,150,190,213,262}) = 213; (100, 113, same) = 37;
  (262, 113, same) = 262; (228, 113, nullptr) = 226; (228, 113, {}) = 226; (5, 113, {10,40}) = 0.

**A5 (vj-2, decode-2, gates-6, vj-6) -- T1b grows; NEW T1c.**
- T1b gains three configs (same fixture and harness, 9 s from the Loop wrap):

  | config | main (RED) | GREEN bar | amended |
  |---|---|---|---|
  | (d) CAP 21, K 2 | 403 / 160 | late <= 100 AND shown >= 240 | 42 / 253 |
  | (e) CAP 21, K 3, EMA emulated: the harness keeps its own `ema` (start 2 x t_d), pins it before every step, and after each FFmpeg decode sets `ema = 0.9 ema + 0.1 t_d` | 69 / 246 | late <= 4 AND shown >= 265 | 0 / 268 |
  | (f) CAP 21, K 3, pinned at 2 x t_d | 69 / 246 | late <= 4 AND shown >= 265 | 0 / 268 |

  Every config (a)-(f) must also show mismatches 0, cacheMismatches 0 and reverseNonmonotonic 0 (VERIFIED on the amended
  patch).
- NEW T1c, "gop2 (GC7): a pause, a 2x speed change, a halved budget, a slower decoder or a flip DURING a PREFETCH lead-in
  -- identity holds, never later than main, a flip never stalls".
  - Setup: the T1b harness, fixture video_h264_gop250_64x64.mp4, CAP 21, K 3 (slew: K 5 -> 3), 9 s from the Loop wrap.
  - Trigger: the event fires, before that frame's advanceFrame, at the first render frame i >= 192 (1.6 s) where
    `VideoPlayerTestAccess::inPrefetchLeadIn(p)` holds: `run_->kind == Prefetch && !run_->seekPending &&
    (!haveDecoded_ || lastDecodedRel_ < run_->windowLo)`. REQUIRE that it fired; both arms fire at frame 192.
  - Late uses the committed Show's rule and counts only while `isPlaying()`.

  | event | main | GREEN bar | amended |
  |---|---|---|---|
  | pause: setPlaying(false) for 120 render frames, then true | 33 | total late <= 4 | 0 |
  | speed: setSpeed(2.0f) | 558 | total late <= 558 (never later than main) | 337 |
  | shrink: budget.total /= 2 (10 slots) | 469 | total late <= 469 | 428 |
  | slew: K 5 -> 3, the pin follows t_d | 38 | total late <= 4 | 0 |
  | flip: setReverse(false) | 3 (post 0) | post-flip late <= ceil(gopFramesEst_ / K) + 6 = 90 AND pre-flip late <= 4 | 31 (pre 0) |

  - Every event must also show mismatches 0 (the forward frames after the flip included), cacheMismatches 0 and
    reverseNonmonotonic 0.
  - RED on main: pause and slew. Pure guards (main passes): speed, shrink and flip.
  - The flip bound is the forward catch-up's: VideoPlayer.cpp:888 allows at most gopFramesEst_ decodes before the next
    output. A stall exceeds it.
- `VideoPlayerTestAccess` gains `inPrefetchLeadIn(p)`, `gopEst(p)` and `keyRels(p)`, plus the plan's `pinDecodeMs`.
  Prototype: scratch `ruling-gop2/scen.cpp` (its main arm uses env switches; the committed test has none).

**A6 (decode-4, vj-7) -- NEW T1d and its fixture.** "gop2 (GC7): on a scene-cut file the window's lead-in comes from the
index's real keyframes -- never more decodes than main".
- Fixture `tests/fixtures/video_h264_scenecut_64x64.mp4`: `ffmpeg -y -hide_banner -loglevel error -f lavfi -i
  testsrc2=s=64x64:r=30:d=10 -c:v libx264 -g 300 -keyint_min 300 -sc_threshold 0 -force_key_frames
  "expr:eq(n,0)+eq(n,37)+eq(n,150)+eq(n,190)+eq(n,213)+eq(n,262)" -forced-idr 1 -bf 2 -pix_fmt yuv420p -preset veryfast
  -crf 28 -threads 1`.
- Sanity teeth: 300 frames; `keyRels(p) == {0, 37, 150, 190, 213, 262}`; `gopEst(p) == 113`.
- The T1b harness, 9 s from the Loop wrap, t_d pinned:

  | config | main | GREEN bar | plan's grid (m5) | amended |
  |---|---|---|---|---|
  | CAP 16, K 3 | 1213 decodes | late <= 4, shown >= 265, decodes <= 1213 | 1388 (FAIL) | 1140 |
  | CAP 13, K 4 | 1461 decodes | late <= 4, shown >= 265, decodes <= 1461 | 1690 (FAIL) | 1398 |

- Identity 0/0/0. Decodes = `st.framesDecoded` over the run (counted, not timed: deterministic). The RED is m5 (keys
  ignored), recorded on the c2 intermediate.

**A7 (gates-2, gates-9) -- G4 validity conditions and a mechanism indicator.** See the final gate list, G4 steps 1 and
INFO.

**A8 (gates-4) -- G4 lane-safety conditions.** See the final gate list, G4 step 3.

**A9 (gates-3) -- the A/B driver taints burners** (`.harmony/probe-vupload-ab.sh`, commit c1).
- Add a `burners` function beside `compilers` (:25) that counts `pgrep -x yes`, `pgrep -x stress-ng` and
  `pgrep -x ffmpeg`.
- The per-launch sampler (:41) writes the burner count as a third column. After each launch its maximum is taken, and a
  non-zero value TAINTS the launch exactly like a compiler (:49-51), with the reason printed.
- The per-launch result line (:47) also prints the load average at the launch's END.
- Before round 1, a non-zero burner count prints the offending processes and exits 1 ("orphaned burner -- kill it
  first"). This automates the rig rule "a ps burner check first".
- NOT added: a load-average void (the app's own launches run at load 3.3-5.3).

**A10 (gates-1) -- identity gates.** G5b and G7, in the final gate list.

**A11 (vj-1, vj-8, decode-9) -- the floor is measured and documented.** G6b in the final gate list; docs in A14.

**A12 (vj-5) -- G6's u10 at 512 MB is judged.** See the final gate list.

**A13 (gates-8) -- the selftest asserts exact counts.**
- (i) exactly 0 PASS/FAIL lines naming "cap 0.0", exactly 1 INFO line "cap 0.0: pre-lane arm only -- no rule", and
  exactly 5 "u8 cap 256.0" rule lines, all PASS; exit 0.
- (ii) exactly 6 cap-0.0 rule lines (pooled / GC6 / late / bytes / over_budget / hold) and 5 cap-256.0 rule lines, all
  PASS; exit 0.
- (iii) exactly 1 FAIL line "u8: no B launch"; exit 1.

**A14 (vj-3, decode-8, vj-1) -- doc corrections.** See DOCS below.

**A15 -- plan errata.** See ERRATA below.

**A16 (gates-7, decode-7) -- builder evidence.** The lane report pastes, measured on the COMMITTED code:
- every T1a / T1b / T1c / T1d / T3 / predicate value next to its pre-registered bar;
- the RED of each: on 655d232, except T1d's (on m5) and the predicate's (on m6);
- mutants m1-m6;
- `ctest -N` (+6 test cases: T1a, T1b, T1c, T1d, T3, the predicate; 0 new targets; the count itself is Harmony's).

G1 / G2 run on the committed diff, never on scratch p6.py or p8.py.

**Mutants (teeth), recorded by the builder:**
- m1 (gain forced to 0) -> T1b (a) (b) (d) (e) (f) and T1c pause / slew fail.
- m2 (gain bounded by the whole share) -> the committed floor case fails (plan).
- m3 (landing ignored) -> T3 fails (plan).
- m4 (every landing treated as key) -> the MPEG-TS gate case fails (plan).
- m5 (keys ignored) -> T1d fails.
- m6 (pts clause dropped) -> the predicate case fails.

**Commit sequence** (each commit builds and passes `ctest -j1`; the builder runs no app):
- c1 = item 2 + A9 + A13 (probe .py + .sh).
- c2 = item 1 + A1 + A3-A6, with the gop250 and scene-cut fixtures.
- c3 = item 3 + A2 + T3 + the predicate case, with the intra-refresh fixture.
- c4 = docs + the lane report.

### FINAL GATE LIST (Harmony, after the merge; pre-registered)

Rig rules as the plan (the lock helper; `open -g ... --args --test-mode`; one live app; no Output window; the quiet
lock; `Connection: close`), plus A9's automatic burner taint.

- **G1 ctest** `ctest --test-dir build -j1` -> 100 %. The golden forward traces must not move (VERIFIED on the amended
  patch: 510/5 in 3 of 3 runs).
- **G2 TSan** (the plan's recipe) on test_gop_cache_store, test_video_decode_trace and test_gop_cache -> 0 reports.
  New members: `keyRels_` (written at open, read on the decode thread), and `firstPacketSinceSeek_` /
  `landedOnKeyPacket_` / `landingPts_` (decode thread only).
- **G3 probe:** `--selftest` -> SELFTEST PASS with A13's exact counts. The ab256 evidence file -> no "cap 0.0" rule line
  (INFO only), and the five cap-256 verdicts equal summary.txt's.
- **G4 GC7:** 5 x 2 interleaved, `ENV_A = ENV_B = ADNA_GOPCACHE_BUDGET_MB=256`, A = a copy of main 655d232, B = the
  merge. Decide in order:
  1. VALIDITY. If any condition fails the result is INCONCLUSIVE: rerun once; a second INCONCLUSIVE goes to Harmony.
     - V1: A's median late at cap 256 > 40 (the defect reproduced in-session).
     - V2: B's medians cap_bytes_mb 64.0 +- 0.5, active 4, frames 84 (the cap engaged; a build that ignores the env
       would pass vacuously -- E20).
     - V3: no tainted launch remains in ab.tsv (A9).
  2. BAR, unchanged (GC7 on GC13 medians): B median late <= 40 AND B median slowest >= 20 AND the summarizer's GC7 bytes
     and hold / pending rules PASS.
  3. LANE SAFETY: over_budget == 0 in every B launch. A non-zero B launch while some A launch is also non-zero is INFO
     (filed); non-zero in B only is a FAIL. And B median bytes_mb <= 256.0.
  - PASS iff steps 1-3 hold. 40 < B late < A late = PARTIAL (Harmony rules). B late >= A late = FAIL -> revert c2.
  - Predictions: A late 110-145 (E1), dec/up 12.1-12.5. B late 0-15 (offline u8h: 0 in 6 of 6 runs, including 4 and
    8 ms of emulated GL load), dec/up 10.5-11.5, slowest ~30.
  - INFO: per-launch late and dec/up, evictions, drops.
- **G5 uncapped no-regression:** as the plan (u7, u8, u9, u11; 5 x 2; u11's GC9 stays INFO).
- **G5b identity** (B only, once each, under the lock): `VIDEO_APP=<merged app>
  [VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=256 | 128] VIDEO_FIXTURES=<shared> .harmony/probe-vupload.sh <out>
  u12_reverse_pixel_identity`, run uncapped, at 256 and at 128.
  - Each must print PROBE-VUPLOAD GREEN (GC11: worst max|diff| == 0 over 3 captures).
  - A RED is a hard FAIL: revert the commit that owns it (c2 for the capped runs).
- **G6 4K:** u10 uncapped stays INFO (3 x 2). u10 with `ENV_A = ENV_B = ADNA_GOPCACHE_BUDGET_MB=512`, 5 x 2, is JUDGED:
  B median uploads_per_s >= A's AND B median late <= A's. Prediction: A ~2/s, B 10-15/s (E7; offline 4K x4: 12.6/s).
- **G6b INFO:** u8 with `ENV_A = ENV_B` at 192 MB and again at 128 MB, 3 x 2 each; printed, not judged. Offline
  predictions: 192 MB: A ~500, B 0-40; 128 MB: A ~1600, B ~1300.
- **G7 forward smoke:** as the plan (probe-video.sh all rows once; u4b once) + `u6_crossfade_video` once (its own PASS
  lines).
- **Order:** G1, G2, G3, G4 (stop on FAIL), G5, G5b, G6, G6b, G7. About 55 min of rig time.

### DOCS (plan section 6, amended)

- **rendering.md :79**
  - (a) The lead-in clause reads "(from the container index's keyframe at or before the window -- the longest-interval
    grid only without an index -- to the window; `GopCache::servedDuringLead` at the measured decode time)".
  - (b) The gate clause reads "-- a decoded frame flagged key, or, after a seek that landed on a packet the demuxer flags
    key, a frame at or after that packet's pts (an intra-refresh ... [the plan's text]) --".
  - (c) The container sentence becomes: "Index-less containers (MPEG-TS / PS) are not accepted BY EXTENSION (the six);
    FFmpeg probes content, so a renamed .ts still opens as MPEG-TS. After a seek its H.264 / HEVC / MPEG-2 video outputs
    nothing before a keyframe (0 wrong frames in 20 seeks each). MPEG-4 part 2 video in it shows the decoder's output
    until the next keyframe (606 wrong frames in 20 seeks, up to 53 in a row) on forward seeks and reverse DEMAND
    publishes, never in the cache (a TS landing packet is never key-flagged). Accepting .ts needs a published-frame gate
    first ...". The VFR clause gains "(simulated from ffprobe timestamps with the player's index rule; the vfrgap
    fixture's 63-64 of 85 is measured)".
  - (d) The Measured sentence also records, from the offline rig (4 real decode threads, s-rta-0930 ruling):
    - 1080p x4: late 0 at 256 and at 192 MB (also with 4-8 ms of emulated GL work per frame), ~160 at 160 MB, ~1300 at
      128 MB (10 frames each: below the feasible share);
    - below the feasible share the reader holds (the last frame stays), never a wrong frame;
    - an 8 GB Mac's budget is 512 MB (RAM / 16), halved under a memory-pressure warning (the 256 MB row), floors only
      when critical;
    - four 4K reversers in 2 GiB (43 frames each): 2.2 -> 12.6 frames/s each, moving but not smooth (software decode).
  - (e) Levers: `kInPlaceMargin` (plan) and `keyRels_` (A1).
- **pitfalls.md, Pitfall 62:** the plan's rule-(3) insertion reads "... counts the slots the clock frees while the run
  decodes its lead-in (`servedDuringLead`, from the index's real keyframe -- the longest-interval grid only without an
  index: on a scene-cut file the grid over-states the lead-in, up to +17 % decodes) ...; the store gate is the decoded
  frame's key flag OR, after a landing on a demuxer-key packet, a frame at or after its pts (`GopCache::storeGateOpens`)
  -- never the frame flag alone ... and never a forward published-frame gate on it ...". The Guards list gains the T1a,
  T1b, T1c, T1d, T3 and predicate case names.
- **testing-eyes.md :15 and CLAUDE.md :225:** as the plan.

### ERRATA to plan-gop2 (A15)

- **K2 is inverted.** An OVERestimate of `decodeMsEma_` gives a BIGGER window (the lead-in is assumed to free more slots
  than it does), so the store drops or replaces the window's bottom (GopCache.h:255-260). That costs decodes, not stalls:
  a 2x pin gives late 0 and +11 % decodes (2,427 vs 2,186 at K 3). An UNDERestimate gives a smaller window, toward
  main's: a 0.5x pin gives late 0 and 2,173 decodes.
- **E5:** the u8 fixture a1080_g250.mp4 (the rig's copy) has 300 frames with keys at 0 and 250 (ffprobe), not 250 frames.
- **K8:** the decoders are saturated in both arms (4K x4: ~450 decodes/s either way). The patch turns the same decoding
  into 5.6x the uploads; it spends no extra CPU.
- **E12's 4/20:** VERIFIED as an artefact (decode-1).
- **Identity:** T1b's (plan :252-254) and T3's (:335-336) are VERIFIED on the amended patch (0/0 in every config).
- **T1a(ii):** pAt 19 is unreachable at its own inputs (A3). The GC9 "one-frame margin" (gates-7) exists only there.
- **CROSS-LANE TOUCHES:** A1 adds a hunk inside open() (:276-297).

### FILED (not this lane)

- **F1** A flip back to forward catches up across a keyframe: VideoPlayer.cpp:885-892 seeks only beyond `gopFramesEst_`
  frames. Stepped, both arms show post-flip holds of up to 43 render frames at 2.78 ms per decode. With `keyRels_` (A1),
  "seek when a keyframe lies between" becomes a small change. Trigger: a performer reports a hitch when flipping a
  reversing clip back to forward.
- **F2** The in-place branch's `avail0 > 0` guard delays runs when behindCap > 0 and pAt > share (4K: runs wait until
  u < share). Relaxing it risks gopcache-fix F1 (a window that stores nothing blocks its target). Measure on u10 first.
- **F3** A published-frame gate for MPEG-4 part 2 in MPEG-TS (a renamed .ts): 606 wrong frames over 20 seeks, forward and
  reverse publish; the cache stays clean.
- **F4** What the app does when a column cannot reverse within its share (it holds today). A hint or a degrade policy is
  Boris's call; ask only if he reports a frozen reverse under memory pressure.

### BORIS QUESTIONS (each has a default; the build does not block)

- **Q1** as the plan (VFR / screen recordings; DEFAULT: documented now, fixed on request).
- **Q2** updated: "Four 4K clips reversing together: today about 2 frames a second each; after this change about 12-13 each
  (measured offline). They move, but not smoothly (software decoding). OK for now?" DEFAULT: yes; hardware decoding
  (VideoToolbox) stays filed.

### RISKS

- **R1 (the strongest counterargument):** "A1 and A2 grow a measured lane with code the live gate never exercises: u8's
  fixture has fixed GOPs, and no accepted file outputs a frame below the landing pts." Why it loses:
  - A1's effect is measured on the committed-test path: T1d is RED on the plan's own formula, and the change is -4 to
    -9 % decodes against main where the plan was +0 to +17 %.
  - A1 is exactly equal on fixed GOPs (T1a (v); N2-N6), so it cannot move G4. Typical VJ clips (x264-default scene-cut
    encodes) are exactly where the plan as written spends extra decode CPU from a shared budget.
  - A2 is one predicate and one member, unit-tested (m6), costs nothing measurable (N8), and turns decode-1's hazard from
    "absent in 11 files" into "impossible".
  - Total cost: one small vector filled in open() and about 25 lines.
- **R2:** T1c's speed and shrink bars are main's own counts (558, 469). An FFmpeg upgrade can shift deterministic counts
  by a few percent (the same class as the plan's T1b numbers). A failure there after an upgrade is a Harmony
  re-registration, not a lane defect.
- **R3:** `keyRels_` built from DTS is off by up to the reorder delay on variable-delay streams (INFERRED harmless: 2
  frames of lead-in moves servedDuringLead by < 1 frame at 1.8 / 33 ms). An MKV with cues coarser than its keyframes falls
  back to an earlier key, i.e. the plan's overestimate, whose cost N4 / N7 bound.
- **R4:** the live G4 remains the arbiter. The offline margin is INFERRED to transfer: the patch holds late 0 at a
  decoder 1.5x slower than live (K 3), and 42 per 9 s at 2.3x slower (K 2) where main has 403.

### EVIDENCE INDEX (scratch; rebuildable)

`/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling-gop2/`:
- `landing3.c` + `landing3.log` (the pts / content probe); `mkfiles.sh` + `files/` (MPEG-2 open-GOP MOV, B-pyramid
  open-GOP MOV, HEVC-in-TS, renamed TS; landing2 on them).
- `scen.cpp` (the stepped event harness: the T1b / T1c / T1d prototype), `scen.sh` / `scen.log`, `flipsweep.sh` /
  `flipsweep_k3.log`, `ema.sh` / `ema.log`, `final.sh` / `final.log`, `irid.sh` / `irid.log`, `ir.sh`.
- `mkscene.sh`, `scenecut_64x64.mp4`, `x264default_cuts_64x64.mp4`, `keys.sh` / `keys.log`, `t1d.sh`.
- `p7.py` (keys), `mkp8.sh` -> `p8.py` (keys + the pts gate), `rig2.py` (relinks any of the three test targets against a
  patched VideoPlayer.cpp), and the `p8store` / `p8trace` / `p8pure` / `p8scen` / `p8ir` builds.
- `t1a.cpp`, `t1a7.cpp`, `t1a0.cpp` (pure planner values); `u8load.sh` / `u8load.log` (u8h under GL load, the cliff,
  4K x4; binaries `../gc7/v0`, `../gc7/v6`).

### NOTEBOOK NOTES (for Harmony to append)

- **The workflow's attack seats never wrote their REPORT_FILE.** The harness's "do not write report files" note wins
  over the seat spec, so the ruling agent receives paths without contents. Recover the papers from
  `subagents/workflows/<wf>/journal.jsonl` (type "result"). Fix the workflow: pass the seats' results into the ruling
  prompt, or write the files from the script | wf/plans.js:81-85.
- **A stepped event harness makes timing-policy regressions deterministic REDs.** Fire a pause / flip / speed / budget
  change at the first frame where a PREFETCH lead-in is in flight | ruling-gop2/scen.cpp.
- **A committed test target can run against a candidate patch without a worktree.** Relink it with the target's own
  flags.make + link.txt, replacing the objects | ruling-gop2/rig2.py.
- **`best_effort_timestamp` is not an index.** An MPEG-4-in-MP4 keyframe reports 512 after a seek and 0 going forward,
  while its pts is 0 | landing3.log.

STATUS: RULING COMPLETE -- 26 attacks ruled (23 accepted in whole or part, 3 rejected with measurements); 16 amendments (2 design: A1 real keyframes, A2 the landing-pts gate); final gates G1-G7 + G5b / G6b pre-registered; the amended patch verified offline against every committed suite and every new bar; ready to build on Harmony's adoption.
