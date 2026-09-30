# plan-gop2 -- close the GOP-cache follow-ups of lane gopcache (s-rta-0930, lane "gop2")

Architect (Fable), 2026-09-30. Main HEAD 655d232 (src / tests / probes identical to the gopcache merge 8015c76:
`git diff --stat 8015c76 HEAD -- src tests .harmony/probe-vupload.py .harmony/probe-vupload-ab.py` is empty). Fence:
`src/media/GopCache.h`, `src/media/VideoPlayer.{h,cpp}`, `tests/test_gop_cache.cpp`, `tests/test_gop_cache_store.cpp`, two
new fixtures in `tests/fixtures/`, `.harmony/probe-vupload-ab.py`, docs (rendering.md :79, pitfalls.md Pitfall 62,
testing-eyes.md :15, CLAUDE.md :225). No app was launched for this plan. Every performance claim below comes from OFFLINE rigs
that link the REAL `src/media/VideoPlayer.cpp` + FFmpeg 8.0 into test binaries built with `test_gop_cache_store`'s own flags and
objects (read-only use of `build/`; scratch index at the end). The live verdicts are Harmony's gates (section 5).

## (1) GOAL
Make four capped 1080p GOP-250 reversers meet the unchanged GC7 bar (late <= 40, slowest >= 20/s at 256 MB). Each PREFETCH window
is sized to the slots its lead-in frees. The probe stops inventing "cap 0.0" rules. R3 is decided with evidence: forward and
reverse-publish stay unchanged, and the reverse STORE gate also accepts a landing on a demuxer-key packet. The VFR residual is
documented as by-design, with numbers.

## (2) ESTABLISHED (VERIFIED = read / run by me; INFERRED = reasoned, labelled)

- E1 VERIFIED -- the GC7 evidence (`.harmony/.reports/s-rta-0930/evidence-0929b/ab-ab256/{ab.tsv,summary.txt,meta.txt}`):
  - B = `build/.../Audio-DNA.app` (abdd9135425bd0d5, main code) with `ADNA_GOPCACHE_BUDGET_MB=256`, 5 launches:
    - late 114 / 112 / 143 / 130 / 123 (median 123 > 40: FAIL);
    - slowest 27.8 / 28.2 / 27.8 / 27.8 / 27.6 (median 27.8 >= 20: PASS);
    - 249.4 MB, 84 frames (4 x 21), cap 64 MB, dec/up 12.24-12.42, holds / pending 0.
  - A = the pre-gopcache app (d49df20b40a6a0cd) without the env: 16.8-19.2 uploads/s, late 1771-1799.
- E2 VERIFIED -- what "late" counts. It is incremented per render frame, per drawn playing player, when `uploadToTexture` picks
  nothing and |clock - lastShown| > 1.5 frame (VideoPlayer.cpp:608-613, VideoRing.h:300-308). u8 takes the global counter's delta
  from 1 s to 6 s after one `trigger_column` (probe-vupload.py:594-605). So 123 = the pooled render-frame holds of 4 players at
  ~115 fps, and the bar 40 is ~10 per player per 5 s.
- E3 VERIFIED + arithmetic -- the share per player:
  - cap = total / active (GopCache.h:54); capSlots = max(kMinFrames, cap / frameBytes) (VideoPlayer.cpp:1045-1049).
  - frameBytes = `av_image_get_buffer_size(..., 64)` rounded up to pages (GopCacheStore.h:89-94): 3,110,400 -> 3,112,960 B.
    So floor(67,108,864 / 3,112,960) = 21 slots.
  - The Behind pool is 0 below `kBehindMinSlots` 32 (GopCache.h:167-175).
- E4 VERIFIED (code) -- today's PREFETCH plan needs two conditions:
  - unservedAhead < prefetchAt (GopCache.h:318-322, :379);
  - avail >= minWindow (GopCache.h:381-383), where minWindow = (capSlots - behindCap) / 2 = 10 at 21 slots
    (VideoPlayer.cpp:1306-1308), and avail = the Future share's slots free or replaceable AT PLANNING (GopCache.h:335-358).
  - At 21 slots, a run can start only with <= ~11 frames of cover and stores ~10 (INFERRED from the rule). The traced 13-slot
    twin shows the pattern: runs appear only at "u 4 avail0 6" (scratch trace, 12:47).
- E5 VERIFIED (CLI ffmpeg 8.0 = the app's libavcodec; the u8 fixture a1080_g250.mp4, 250 frames; gc7/dec.sh; load 0.9-1.0):
  - threads 2: 0.471 s including 0.062 s process start = 1.64 ms/frame; four concurrent: 1.80 ms/frame each; threads 1: 3.2 ms;
  - `-skip_frame noref`: 0.490 s (no gain on this encode).
  - A run for the top of GOP 0 (keyframe 0 -> ~249) is ~250 decodes = ~440 ms at the app's EMA 1.76 ms, against ~367 ms of cover
    under E4.
- E6 VERIFIED -- where the late frames are. The offline rig `gc7/u8h.cpp`: 4 real players with their real decode threads, one
  `GopCache::Budget` of 256 MB, a 120 Hz reader doing `uploadToTexture`'s pick / Retire / judge without GL.
  - HEAD's late per 0.5 s = 0 45 14 3 0 0 0 0 0 0: all of it 1.5-2.5 s after the trigger = frames ~254..224, the entry into the
    top of GOP 0.
  - The rig reproduces the live row's shape: dec/up 12.2-12.4 (live 12.36), 84 frames, 21 slots, slowest 28.2-29.0 (live 27.8).
  - Its late count (44-101 over 20 HEAD runs; 93-101 at load 6-9, batch5) is below the live 123: it has no compositor or GL load.
- E7 VERIFIED -- the fix works offline. HEAD and the patch run interleaved in one invocation (batch6.log, 12:49-12:52, load 2-7);
  "patch" = items 1 + 3 = scratch `gc7/p6.py`.

  | row | HEAD 655d232 | patch |
  |---|---|---|
  | u8, 256 MB x 4: late | 48 / 63 / 63 / 67 / 75 | 0 / 0 / 0 / 0 / 0 |
  | u8, 256 MB x 4: slowest | 28.4-29.0 | 30.0 every run |
  | u8, 256 MB x 4: dec/up | 12.2 | 10.8-10.9 |
  | u8, 256 MB x 4: DEMAND misses | 48-56 | 0 |
  | 192 MB | late ~500 | 0 |
  | 160 MB | late ~1000 | 152 |
  | 128 MB | late ~1580 | 1283 (10 slots is below feasibility) |
  | uncapped 2048 MB x 4 | 120.0/s, late 0, dec/up 1.12 | 120.0/s, late 0, dec/up 1.01-1.05 |
  | 1 x g30 / 1 x g250 whole file, speed 2 | identical | identical |
  | deck return (u11 shape) | first upload 241.7 ms, late 116 (3/3) | 241.7 ms, late 116 (3/3) |
  | one 4K reverser, 512 MB share (43 slots) | 2.2/s, late 550, 0 PREFETCH runs | 14.4/s, late 298 |

  The 4K row is one of four 4K reversers in 2 GiB: HEAD serves it by DEMAND runs only.
  Earlier formula variants also gave late 0 in 10/10 more runs at 256 MB (batch1, batch5).
- E8 VERIFIED -- the committed suites pass on the patched source (the rig built with the committed test TUs):
  test_gop_cache_store 523/22, test_video_decode_trace 510/5 in 3 of 3 runs (the forward golden traces), test_gop_cache 111/11.
- E9 VERIFIED -- a deterministic stepped RED (`gc7/gc7step.cpp`):
  - Setup: the real player, the writer stepped, <= K FFmpeg decodes per 120 Hz render frame, `decodeMsEma_` pinned to 8.33/K
    before every step. A 64x64 GOP-250 fixture, 9 s of reverse from the Loop wrap.
  - HEAD: 21 slots / K 3: late 69, shown 246. 16 / K 4: 56 / 251. 13 / K 4: 150 / 224. 21 / K 4 and 21 / K 5: 0 / 269.
  - Patch: 0 late in all five, shown 268-269.
  - Reruns are identical (the test counts decodes, not time).
- E10 VERIFIED -- the probe artefact. probe-vupload-ab.py:123 iterates the UNION of both arms' `cap_mb` values. The A arm had no
  env, so its lines read `cap_mb = ENV_CAP_MB or 0` (probe-vupload.py:332-335, :603). The resulting "cap 0.0" group has no B
  line, so six B rules run on empty data and print "None" FAILs (summary.txt lines 3-8). The run exits 1 whatever the cap-256
  verdicts are.
- E11 VERIFIED -- the app accepts only `.mov .avi .mp4 .mkv .webm .m4v` as video:
  MainComponent.cpp:5253-5254 (drop), :6948 (chooser); ClipCell.cpp:257-258, :296-297; FilesBrowser.cpp:549-551 lists
  .avi / .mov / .mp4. A composition file carries `mediaType` verbatim (Clip.cpp:31, :174), so a .ts video can only come from a
  hand-written composition (INFERRED).
- E12 VERIFIED -- seek landings. Probe: scratch `r3/landing2.c`, FFmpeg 8.0, thread_count 2, 20 BACKWARD seeks + flush per file,
  each output compared with a forward decode by timestamp + checksum.
  - Every accepted container / codec lands on a key packet 20/20 with 0 wrong frames out: h264 closed / open GOP in
    mp4 / mov / m4v / mkv / cue-less "live" mkv; HEVC (open GOP) mp4; VP9 webm incl. cue-less; MPEG-4 part 2 in AVI / MP4;
    truncated AVIs without idx1.
  - One exception to check: MPEG-4-in-MP4 had 4/20 first outputs mismatch the reference by timestamp. INFERRED to be a
    best-effort-timestamp artefact of the probe: the output is key-flagged, and the lane's identity ctest on that codec /
    container passes.
  - MPEG-TS: the landing packet is key 0/20 in EVERY codec. MPEG-4 part 2 in TS outputs 606 wrong frames over 20 seeks (up to 53
    in a row). h264 / mpeg2 in TS output nothing before a keyframe (0 wrong).
- E13 VERIFIED -- intra-refresh H.264 (x264 `intra-refresh=1:keyint=60`, 320x180, 10 s):
  - In MP4 / MKV every seek lands on a key packet (20/20), and the first output is correct 20/20;
  - but only 4/20 of those first outputs carry `AV_FRAME_FLAG_KEY`. ffprobe: 5 key packets, 1 key-flagged decoded frame (frame 0).
- E14 VERIFIED -- the GC3 store gate is the DECODED frame's key flag: VideoPlayer.cpp:1435-1436 set `runSawKey_`; :1460-1462 store
  only with it; the hole walk needs it too (:1446). So on intra-refresh files only runs that seek to frame 0 store anything.
  Stepped HEAD, the 300-frame 320x180 file, laps 2+ (the patch = item 3):

  | cache | HEAD | closed-GOP control | patch |
  |---|---|---|---|
  | 32 frames: seeks / DEMAND misses / decodes a lap | 567-601 / 195-204 / ~16,000 | 25-27 / 0-1 / 800-950 | 31-32 / 0-1 / 923-1,036 |
  | whole file: resident | 298 of 300 | 300 | 300 |
  | whole file, each later lap | 3 seeks + 2 misses + 125 decodes | 0 | 0 seeks |

- E15 VERIFIED -- GC5 (VideoPlayer.cpp:291-292): an index whose every entry is a keyframe makes the player intra-only (no cache, no
  runs), so a broken all-key index never reaches the run store gate.
- E16 VERIFIED -- VFR losses. Method: ffprobe timestamps + the player's index rule, relOf = round((t - first) x avg_frame_rate)
  (VideoPlayer.cpp:1005-1008, :148-154); the cache keeps the FIRST frame at an index (:1460 `slotOf(rel) < 0`); scratch
  `vfr/count.py`. Frames never shown in reverse:

  | file shape | frames never shown in reverse |
  |---|---|
  | 30 fps with +-4 ms jitter (phone-like) | 0 % |
  | the lane's setts fixture | 0 % |
  | 30 / 24 / 30 fps (`r_frame_rate` 120/1) | 4.4 % |
  | 60 fps with 10 % drops | 9.1 % |
  | 30 fps with 30 % drops (vfrgap: 20 of 85) | 23.5 % |
  | 60 fps bursts + 4 fps idle (screen-recording-like) | 56.1 % |

  The lane shows the same 63-64 of vfrgap's 85 every lap; main showed a varying 36-52 (gopcache.md fix round 2, R2).
- E17 VERIFIED -- CLAUDE.md = 23,999 B; its Pitfall-62 index line (:225) is 181 B.
- E18 VERIFIED -- no Boris ruling on reverse, ping-pong, GOP, memory or VFR: BORIS_DECISIONS.md "Playback Behaviour" and
  .harmony/binding-decisions.md searched.
- E19 VERIFIED -- `planPrefetch` has one production caller (VideoPlayer.cpp:1307) and 12 test calls (tests/test_gop_cache.cpp),
  all with the default trailing arguments.
- E20 VERIFIED -- the app build honours the budget env: `AUDIODNA_BUILD_TEST_SERVER=ON` (build/CMakeCache.txt:31) ->
  `AUDIODNA_TEST_SERVER` (CMakeLists.txt:462-464) -> GopCacheStore.h:45-49.

## (3) DESIGN FORKS

**F-A. GC7: how a 21-frame share keeps up with a 250-decode lead-in.**
- A1 [CHOSEN] **The in-place window.** A PREFETCH window = the slots free now (`avail0`, today's window) + the slots held by
  nearer frames that the clock will pass while the run decodes its lead-in (keyframe -> window), at the measured decode time:
  `window = min(share, avail0 + max(0, min(share - avail0, servedDuringLead(lead)) - 1))`,
  iterated twice (the window's bottom and its lead depend on each other). Evidence: E7-E9.
  - Safe when the lead-in is shorter than a frame period (every small-frame ctest, fast decoders, big shares):
    `servedDuringLead` = 0 and the window IS today's (E8: every committed case passes; E7: uncapped unchanged).
  - Safe when the estimate is wrong: the store's existing farthest-next-use verdict (GopCache.h:199-261) keeps the frames
    nearest the clock and replaces the window's own bottom. The window shrinks; it never stalls.
- A2 [REJECTED, measured] **Bound the gain by the whole share** (`share - (u - served) - 1`). It had the best tight-cap numbers
  (160 MB late 4) but breaks the committed 8-frame-floor case (perFrame 4.55 > its 2.57 bound, deterministic): it counts slots
  nothing will free when the lead-in is short.
- A3 [REJECTED, measured] **Bound the gain by the contiguous unserved run `u`.** It misses the 2-3 resident frames between the
  clock and the last published frame (the 13-slot trace: "avail0 1 gain 6 avail 6" where 12 fit). 160 MB late 659 vs A1's 152;
  equal at 256 MB.
- A4 [REJECTED, measured] **A2 + clamp every window to its target's GOP.** 192 MB late 0 (vs 16 / 4 / 4 unclamped), but the
  uncapped u8 decodes 1.55 vs 0.97-1.05 per upload (+50 % CPU), and nothing is gained at 256 MB.
- A5 [REJECTED] **More skipping in the lead-in.** NONREF saves nothing measurable on this encode (E5).
  `skip_loop_filter` / `AVDISCARD_BIDIR` change reference pictures, so the stored frames would differ from forward play
  (Pitfall 62 rule 1, GC11 identity).
- A6 [REJECTED] **A second decoder per player as a mid-GOP checkpoint.** plan-gopcache already rejected it (memory from the same
  budget, more threads), and FFmpeg cannot copy a decoder's state: a parked decoder is a single-use checkpoint.
- A7 [REJECTED] **An unequal or time-shared budget split.** A sliding window costs ~G^2 / 2W decodes per GOP, convex in W, so the
  equal split minimises the column's total.
- A8 [REJECTED] **Declare the bar unreachable.** E7 / E9 say it is reachable.
- Runner-up: A2. It loses because it violates a committed amortization bound (that would mean re-thresholding an existing test,
  which is not allowed), and its gain is not grounded in slots that will actually free.

**F-B. The probe artefact.**
- B1 [CHOSEN] Rule the u8 bars per cap group of the LANE arm (B). The pre-lane arm's lines at the same cap are the baseline,
  else all its lines, labelled. A cap present only in A prints INFO. No B u8 line at all -> one explicit FAIL.
- B2 (drop empty groups silently) loses: a B arm that produced no u8 DATA line would pass silently.
- B3 (force ENV_A = ENV_B in the driver) loses: a capped lane vs a pre-cache A is a legitimate comparison that the summarizer
  must read.

**F-C. R3: output before the keyframe.**
- C1 [REJECTED, E13] **A forward gate on the decoded frame's key flag.** Intra-refresh MP4 / MKV would freeze after 16 of 20
  seeks (correct frames, never key-flagged) until the Loop wrap.
- C2 [REJECTED] **A forward gate on the landing packet.** No reachable benefit: every accepted container lands on a key packet
  with 0 wrong frames (E12); the garbage exists only for MPEG-4 part 2 in MPEG-TS, which the app does not accept (E11).
- C3 [REJECTED] **A reverse PUBLISH gate.** Same reach as C2; parity with forward is the consistent behaviour.
- C4 [CHOSEN, E13-E15] **The reverse STORE gate also accepts a landing on a demuxer-key packet.**
  - It is the lane's own gate, not a publish gate.
  - Intra-refresh files then cache like every other file (E14).
  - TS garbage stays out: TS landing packets are never key-flagged (E12).
  - The forward path writes the two new flags and reads neither (E8 golden traces).
- C5 [OUT OF SCOPE] **Accept .ts / .m2ts / .mpg.** Documented as a precondition: a published-frame gate on the landing packet
  must come first.

**F-D. The VFR residual.**
- D1 [CHOSEN] By design: documented with E16's numbers, and filed with a design sketch and a trigger.
- D2 [REJECTED for this lane] **An index grid at the stream's base rate.** `r_frame_rate` is the LCM for mixed rates (120 for
  30 / 24 content = 4.3 indices per real frame). So every window, the DEMAND window, the Behind pool and the forward-hit walk
  (all sized in indices: relOf / ptsOfRel / nFrames_ at 30+ sites) would need rescaling. That is a redesign. CFR files would be
  untouched by construction (the grid equals the frame duration), so it is a clean follow-up lane.
- D3 [REJECTED] **Two frames per index.** Bursts put 3-4 frames in one bucket (the 56 % case).

## (4) ITEMS

### Item 1 -- GC7: the in-place PREFETCH window (commit c2)

**Files / functions:**
- `src/media/GopCache.h`, NEW:
  - `constexpr int kInPlaceMargin = 1;`
  - `struct Lead { double decodeMs = 0.0; double frameMsEff = 0.0; int gopFrames = 0; };` (gopFrames 0 = no model = today's
    window);
  - `inline int servedDuringLead(int leadFrames, double decodeMs, double frameMsEff)` = floor(lead x decodeMs / frameMsEff),
    0 when any input <= 0.
- `planPrefetch` (:366-397) gains a trailing `const Lead& lead = {}`. After `avail0 = availableFor(...)` (:381), and only when
  `lead.gopFrames > 0 && avail0 > 0`, run two iterations of:
  - `lo = max(0, t - (avail - 1))`; `key = (lo / gopFrames) * gopFrames`;
  - `gain = min(share - avail0, servedDuringLead(lo - key, decodeMs, frameMsEff))`;
  - `avail = min(share, avail0 + max(0, gain - kInPlaceMargin))`, with `share = capSlots - behindCap`.

  Then today's gate (:382-383) and window (:384-395) run on the new `avail`. The comment should say: the lead-in's passed frames
  free their slots; the store's farthest-next-use keeps the nearest frames when the estimate is optimistic.
- `src/media/VideoPlayer.cpp` `planPrefetchRun` (:1306-1308): pass `GopCache::Lead{ decodeMsEma_, frameMsEff(), gopFramesEst_ }`.
- Unchanged: `prefetchAt`, `minWindow`, the GC9 token / urgent line (:1314-1320), `startDemand` (:1273-1294), the budget,
  eviction, `enforceCap`.

**Behaviour change:** in a share smaller than a GOP's worth of decode cover, a PREFETCH run is planned when `prefetchAt` fires,
not only once half the share is free. Its window counts the slots the clock frees during its lead-in. With a short lead-in
(small frames, fast decode, a big share) nothing changes.

**RED-first tests:**
- **T1a** -- `tests/test_gop_cache.cpp`, NEW case "gop2 (GC7): the in-place window counts the slots a run's lead-in frees;
  unchanged when the lead-in is shorter than a frame":
  - (i) `servedDuringLead(228, 1.8, 1000/30.0) == 12`; `(0, ...) == 0`; `(100, 0.0, 33.3) == 0`; `(9, 0.05, 33.3) == 0`.
  - (ii) The u8 shape: n 300, Loop; cur = served = 249; frames 232..248 resident as Future (key 249 - f), so u = 17;
    capSlots 21, behindCap 0, prefetchAt 19, minWindow 10.
    - Without a Lead -> `RunKind::None` (avail 4 < 10: today's result).
    - With `Lead{1.8, 1000/30.0, 250}` -> Prefetch, target 231, windowLo 218, seekFrom 218. (Iteration 1: 4 + 12 - 1 = 15;
      iteration 2: lo 217, gain 11 -> 14.)
  - (iii) `Lead{0.05, 1000/30.0, 250}` returns exactly the no-Lead Run (kind / target / windowLo / seekFrom), on these inputs
    and on the existing case's inputs (:262-270).
  - (iv) A grid (avail0 1..20, lead 0..300, decodeMs {0.5, 1.8, 7}): avail0 <= window <= share.
  - RED: does not compile on 655d232 (no `Lead`). The behavioural RED is T1b.
- **T1b** -- `tests/test_gop_cache_store.cpp`, NEW case "gop2 (GC7): a GOP-250 reverse Loop in a small share keeps up with an
  emulated decoder", + NEW fixture.
  - Fixture `tests/fixtures/video_h264_gop250_64x64.mp4`: `ffmpeg -y -hide_banner -loglevel error -f lavfi -i
    testsrc2=s=64x64:r=30:d=10 -c:v libx264 -g 250 -keyint_min 250 -sc_threshold 0 -bf 2 -pix_fmt yuv420p -preset veryfast
    -crf 28 -threads 1`. That gives 48,497 B, 300 frames, key packets at pts 0 and 128000 = 8.333 s. The test asserts 300 frames
    and 2 keyframes once.
  - `VideoPlayerTestAccess` gains `pinDecodeMs(p, ms)` (`p.decodeMsEma_ = ms`).
  - Per 120 Hz render frame: `advanceFrame(1/120)`; up to K FFmpeg decodes (the `st.framesDecoded` delta) of `decodeStep`, with
    `pinDecodeMs(p, 8.333 / K)` before EVERY step (the EMA would otherwise learn the 64x64 file's real ~0.05 ms); then the Show
    pick + late rule.
  - Reverse from the Loop wrap (`setReverse(true); advanceFrame(1/1000)`) for 9 s (1080 render frames), with
    `budget.total = CAP x frameBytes` after open (the S8 pattern, :933).
  - Pre-registered values (deterministic, E9):

    | config | HEAD late / shown | GREEN bar | patch |
    |---|---|---|---|
    | (a) CAP 21, K 3 | 69 / 246 (RED) | late <= 4 AND shown >= 265 | 0 / 268 |
    | (b) CAP 16, K 4 | 56 / 251 (RED) | late <= 4 AND shown >= 265 | 0 / 269 |
    | (c) CAP 21, K 5 (no-regression) | 0 / 269 | late <= 4 AND shown >= 265 | 0 / 269 |

  - In all three: every shown frame equals `forwardDecode` (`s.check`, mismatches 0) and `reverseNonmonotonic` is 0. HEAD
    passes these too; they guard the patch (not yet executed with the check on: INFERRED to hold, because the change alters
    only window sizes).

**GREEN bar:** T1a and T1b pass. Every existing case of test_gop_cache (111/11), test_gop_cache_store (523/22) and the golden
traces (test_video_decode_trace 510/5) passes unchanged. All three were verified offline on this exact formula (E8).

**Teeth** (scratch mutants, reported, not committed):
- m1: gain forced to 0 -> T1b (a) and (b) fail with HEAD's numbers.
- m2: the gain bounded by the whole share (A2) -> the committed "the budget: a cache capped at its floor" case fails
  (perFrame 4.55 > 2.57). This proves the avail0 anchoring matters.

**Live gates:** G4 (the GC7 gate), G5 (no regression).

**Risk:** live contention above the offline rig (K1).

### Item 2 -- the probe artefact (commit c1)

**Files:** `.harmony/probe-vupload-ab.py`: the u8 branch (:122-145), plus a `--selftest` mode.

**Behaviour:**
- `capsB` = the `cap_mb` values of the B arm's u8 lines. For each: `B_` = B lines at that cap; `A_` = A lines at that cap, else
  all A lines, labelled "A baseline, cap X".
- The existing rules are unchanged in text and logic: uncapped = pooled / GC6 slowest / late / bytes / over_budget /
  hold-pending; capped = GC7 slowest / late / bytes / over_budget (enough) / hold-pending.
- A cap present only in A -> one INFO line "cap X: pre-lane arm only -- no rule".
- No B u8 line at all -> FAIL "u8: no B launch".
- For inputs where B covers every cap, the verdict lines are identical to HEAD's.

**`--selftest`:** writes three synthetic TSVs to a mkdtemp dir, runs this file on each (subprocess), checks the results, and
prints SELFTEST PASS / FAIL (exit 0 / 1):
- (i) capped-only: A cap 0 without gopcache fields; B cap 256 passing (slowest 29.5, late 10, 249.4 MB, over_budget 0, holds 0,
  5 rounds). Expect 0 rule lines containing "cap 0.0", five "u8 cap 256.0" rule lines all PASS, exit 0.
- (ii) mixed: A cap 0; B cap 0 and cap 256, both passing. Expect both groups ruled, exit 0.
- (iii) A-only. Expect the FAIL "u8: no B launch", exit 1.

**RED-first:** run HEAD's summarizer on the selftest's TSV (i) (`git show 655d232:.harmony/probe-vupload-ab.py` into a scratch
copy). It prints six "cap 0.0" FAIL lines with None and exits 1, although every B value passes.

**GREEN:**
- The selftest passes.
- The main tree's evidence `.harmony/.reports/s-rta-0930/evidence-0929b/ab-ab256/ab.tsv` (gitignored: `.gitignore:63`, so it
  exists in the main tree only), summarized by the new file, prints no "cap 0.0" rule line. Its five "u8 cap 256.0" verdicts
  equal summary.txt's (late 123 FAIL stays: that data predates item 1).

**Risk:** none to the app. The selftest writes only under mkdtemp.

### Item 3 -- R3: decision + the reverse STORE gate accepts a demuxer-key landing (commit c3)

**Decision (no code):**
- Forward seek: unchanged (C1 / C2).
- Reverse DEMAND publish: unchanged (C3).
- Index-less containers stay unaccepted (C5, documented in section 6).

**Files / functions:**
- `src/media/VideoPlayer.h`, beside `runSawKey_` (:289): `bool firstPacketSinceSeek_ = false;` (the next video packet read is a
  seek's landing) and `bool landedOnKeyPacket_ = false;` (that packet carried the demuxer's key flag). Decode-thread only, like
  `runSawKey_`.
- `VideoPlayer.cpp` `seekToTimestamp` (:1598-1600), after `avcodec_flush_buffers`:
  `firstPacketSinceSeek_ = true; landedOnKeyPacket_ = false;`
- `decodeNextFrame` (:1622-1626), after the non-video filter:
  `if (firstPacketSinceSeek_) { firstPacketSinceSeek_ = false; landedOnKeyPacket_ = (packet_->flags & AV_PKT_FLAG_KEY) != 0; }`
- `onRunFrame` (:1435-1436): `if ((decodedFrame_->flags & AV_FRAME_FLAG_KEY) != 0 || landedOnKeyPacket_) runSawKey_ = true;`
  The comment should say: a recovery point is a key packet whose frames the decoder outputs only once recovered, never
  key-flagged; a TS landing packet is never key-flagged, so its garbage stays out.
- Forward play writes the two flags and reads neither, so the golden traces are unchanged (E8).

**RED-first -- T3:** `tests/test_gop_cache_store.cpp`, NEW case "gop2 R3: a landing on a demuxer-key packet opens the store gate
-- intra-refresh H.264", + NEW fixture.
- Fixture `tests/fixtures/video_h264_intrarefresh_64x64.mp4`: `ffmpeg -y -hide_banner -loglevel error -f lavfi -i
  testsrc2=s=64x64:r=30:d=4 -c:v libx264 -x264-params intra-refresh=1:keyint=30 -bf 0 -pix_fmt yuv420p -preset veryfast -crf 28
  -threads 1`. That gives 25,353 B, 120 frames.
- Sanity tooth in the test: over a forward decode, >= 3 key-flagged packets and exactly 1 key-flagged decoded frame; otherwise
  the fixture no longer exercises the gate.
- Stepped (Show / `stepUntilIdle`), reverse Loop from the wrap:
  - (a) Whole file in budget (`big(budget)`): one lap + 60 frames, then 2 laps.
    - HEAD (E14's 64x64 twin): 118 of 120 resident; every later lap 3 seeks / 1 run / 2 misses / 83 decodes. RED on
      `missing.empty()` and `seeks <= 1`.
    - GREEN: all 120 resident after the first lap; laps 2-3 seeks / runs / misses <= 1 each; mismatches 0; cacheMismatches 0
      (patch: 120, and 0 / 0 / 0).
  - (b) A 12-frame cache (`budget.total = 12 x frameBytes`), lap 2.
    - HEAD: 169 seeks, 77 misses, 3,321 decodes. RED.
    - GREEN: misses <= 2 AND decodes <= 700 AND every shown frame equals the forward decode (patch: 23 / 0 / 425).
  - The identity assertions are pre-registered but not yet executed. INFERRED from E13's correct 20/20 first outputs. If they
    fail, that is a real finding: STOP and report.
- The existing "GC3 keyframe gate" case (MPEG-TS, :807-832) must stay GREEN. It does on the patch (22/22).

**Teeth:**
- m3: `landedOnKeyPacket_` ignored -> T3 fails with HEAD's numbers.
- m4: every landing treated as key -> the TS keyframe-gate case fails (garbage stored; the gopcache-fix m7 precedent,
  `bad 29 == 0`).

**Risk:** a demuxer that flags a non-decodable packet key (a partly broken index) would now let a run store its garbage. An
all-key index is already intra-only (E15); partial mislabels have no fixture (K4).

### Item 4 -- the VFR residual: by design, documented (commit c4, docs only)

The text in section 6 records:
- E16's table;
- the reason: one frame per average-duration index, so frames closer together than the average collide;
- that forward play shows every frame, and that the lane already beats main on the fixture;
- the filed fix (D2), with its trigger: Boris reports choppy reverse on a screen recording / game capture, or answers Q1
  "fix now".

No test change: the R2 case's floor stays.

### Item 5 -- docs (commit c4)

Section 6. APP-INVENTORY.md counts are Harmony's: the builder reports `ctest -N` (+3 cases: T1a, T1b, T3; 0 new targets).

**Commit sequence:** c1 (item 2) -> c2 (item 1 + T1a / T1b + fixture) -> c3 (item 3 + T3 + fixture) -> c4 (docs + the lane
report).
- Each commit builds and passes `ctest -j1`.
- c2 and c3 each record their RED on 655d232. T1b and T3 compile unchanged against HEAD's VideoPlayer: they use only existing
  members through the friend.
- c2 and c3 each record their teeth.
- The builder runs no app. Its evidence is ctest + TSan + the offline rig (optional).

## (5) GATES (Harmony, after the merge)

Rig rules for every live run: through the lock helper; `open -g ... --args --test-mode`; one live app; no Output window; the
quiet lock; `Connection: close`; a `ps` burner check first.

- **G1 ctest:** `ctest --test-dir build -j1` -> 100 % (1049 + the new cases). test_video_decode_trace passes: the forward golden
  traces must not move.
- **G2 TSan:** the prebuild.md recipe (`-DADNA_SANITIZE=thread`; `TSAN_OPTIONS halt_on_error=0:abort_on_error=0`) on
  test_gop_cache_store, test_video_decode_trace (its threaded reverse / flip / turn case) and test_gop_cache -> 0 reports.
  The new members are decode-thread only.
- **G3 probe:**
  - `python3 .harmony/probe-vupload-ab.py --selftest` -> SELFTEST PASS.
  - `python3 .harmony/probe-vupload-ab.py .harmony/.reports/s-rta-0930/evidence-0929b/ab-ab256/ab.tsv` -> no "cap 0.0" rule
    line; the five cap-256 verdicts equal summary.txt's.
- **G4 GC7 (the bar, unchanged):**
  - Command: `LANE=gop2 LOCK_LIB=<lock.sh copy, SPL fixed> ENV_A=ADNA_GOPCACHE_BUDGET_MB=256
    ENV_B=ADNA_GOPCACHE_BUDGET_MB=256 VIDEO_FIXTURES=<shared> .harmony/probe-vupload-ab.sh <main 655d232 app copy (ditto,
    signature intact, never re-signed)> <merged app> 5 .harmony/probe-vupload.sh u8_reverse_column_1080x4 <out>`.
  - Decision rule (pre-registered, on B's medians over 5 interleaved launches): PASS iff late <= 40 AND slowest >= 20 AND max
    bytes <= 256 MB + floors AND hold / pending 0 in every B launch.
  - Prediction (INFERRED): A late ~110-140 (E1); B late 0-15 (E7 x the rig-to-live factor of 1.2-2.8); slowest ~30.
  - 40 < B late < A late: PARTIAL. Harmony rules. Fallbacks measured offline: A2 with the floor test's bound re-derived (a
    re-threshold = a Harmony ruling), or an earlier trigger.
  - B late >= A late: FAIL -> revert c2.
- **G5 no regression, uncapped** (both arms without env): the same driver, rows
  `u7_reverse_pingpong,u8_reverse_column_1080x4,u9_reverse_cache_drop,u11_reverse_column_return`, 5 x 2.
  - Every existing rule of probe-vupload-ab.py must PASS on B: u7's nine scenes (ABS / VU7 / GC12 / GC8); u8 pooled >= 114,
    slowest >= 28.5, late <= 40, bytes, over_budget 0; u9 (a)-(d) + GC10.
  - u11's GC9 rule (B median first gap <= A) is INFO for this lane. Its history is 247.9 vs 261.2 and 257 vs 261 ms (drift
    >= 13 ms), and the DEMAND path is untouched (offline 241.7 ms on both, E7). Run it 10 x 2 if Harmony wants it as a gate.
- **G6 INFO:** u10 (4K), plus u10 under `ENV_B=ADNA_GOPCACHE_BUDGET_MB=512` (the share of four 4K reversers), 3 x 2. Prediction:
  B 10-15 uploads/s vs A ~2 (E7). Printed, not judged.
- **G7 forward smoke:**
  - `probe-video.sh`, all rows once. The load-sensitive w6b (a) / w7 (b) stay INFO per the s-rta-0929b notebook.
  - `probe-vupload.sh u4b_idle_ring_trim` once: (f) the forward retention bound; (g) retention dropped by the trim.

## (6) DOCS

**`docs/claude/rendering.md` :79** (the "Reverse and ping-pong video" paragraph):
- (a) Replace "a PREFETCH run starts in the idle steps when fewer than `prefetchAt` frames are resident ahead and half the Future
  share is free, one keyframe seek for the whole next window (the Loop wrap's window included);"
  with "a PREFETCH run starts in the idle steps when fewer than `prefetchAt` frames are resident ahead and half the Future share
  will be free when its stores begin -- the frames the clock passes while the run decodes its lead-in (keyframe to window,
  `GopCache::servedDuringLead` at the measured decode time) free their slots, so a small share plans its whole window, not what
  is free at planning (s-rta-0930 gop2: four 1080p GOP-250 reversers in a 256 MB budget, 21 frames each, ran late 123 per 5 s;
  a lead-in shorter than a frame changes nothing) -- one keyframe seek for the whole next window (the Loop wrap's window
  included);".
- (b) Replace "Only frames decoded after a run's keyframe and not flagged corrupt are stored (an MPEG-TS seek lands on a
  NON-keyframe: `tests/test_gop_cache_store.cpp` "GC3 keyframe gate");"
  with "Only frames decoded after a run's keyframe -- a decoded frame flagged key, or a seek that landed on a packet the demuxer
  flags key (an intra-refresh H.264 file's recovery points: its decoder flags no frame key after frame 0, so reverse used to
  cache only runs from frame 0 -- 567-601 seeks a lap in a 32-frame cache; s-rta-0930 gop2) -- and not flagged corrupt are
  stored (an MPEG-TS seek lands on a NON-key packet: `tests/test_gop_cache_store.cpp` "GC3 keyframe gate");".
- (c) Insert before " Measured (u7, 1080p": "Index-less containers (MPEG-TS / PS) are not accepted (the six extensions):
  MPEG-4 part 2 in them seeks onto non-keyframes and forward and reverse both show the decoder's output until the next keyframe;
  every accepted container lands every seek on a key packet (17 files, 0 wrong frames: s-rta-0930 gop2 R3) -- accepting one
  needs a published-frame gate on the landing packet first, never on the decoded frame's key flag alone (intra-refresh files
  would freeze). VFR is one frame per average-duration index by design: frames closer together than the average collide and
  one of them is never shown in reverse -- 0 % for phone-like jitter, 4.4 % at 30 / 24 / 30 fps, 9.1 % for 60 fps with 10 %
  drops, 23.5 % for 30 fps with 30 % drops (the vfrgap fixture: the same 63-64 of 85 every lap; main showed a varying 36-52),
  56 % for a screen-recording-like file (60 fps bursts); forward shows every frame. The fix (an index grid at the stream's base
  rate; `r_frame_rate` can be the LCM -- 120 for 30 / 24 content -- so every window, pool and hit walk sized in indices must
  scale) is filed."
- (d) In the Measured sentence, after the u8 numbers, add "; in a 256 MB budget (GC7, 21 frames each) main 655d232 ran late
  123 per 5 s, slowest 27.8 -- gop2: [G4's medians]" (Harmony fills in the live numbers).
- (e) The Levers list gains `kInPlaceMargin`.

**`docs/claude/pitfalls.md` Pitfall 62:**
- In rule (3), after "a run that lands on a keyframe above its window seeks further back first;", insert: "a PREFETCH window
  counts the slots the clock frees while the run decodes its lead-in (`servedDuringLead`), never only what is free at planning
  (four 21-frame shares started each 250-decode run with ~11 frames of cover: late 123 per 5 s -- s-rta-0930 gop2); the store
  gate is the decoded frame's key flag OR a landing on a demuxer-key packet -- never the frame flag alone (an intra-refresh
  file's recovered frames are never key-flagged) and never a forward published-frame gate on it (those files would freeze);".
- Before "(s-rta-0929b gopcache.)", add: "VFR files keep one frame per average-duration index -- by design, numbers in
  rendering.md."
- The Guards list gains the three new case names.
- No new pitfall number: the key-flag rule belongs to Pitfall 62's run gate. Harmony may promote it to 63 if another lane needs
  it.

**`docs/claude/testing-eyes.md` :15**, append: "`probe-vupload-ab.py` rules u8 per cap group of the lane arm (B); the pre-lane
arm's lines are its baseline (a capped-only run no longer invents 'cap 0.0' rules); `probe-vupload-ab.py --selftest` checks the
summarizer offline (s-rta-0930 gop2)."

**`CLAUDE.md` :225**, replaced (181 -> 178 B, net -3; the file stays <= 23,999): "62. Reverse / ping-pong video = the decode
thread's GOP cache + a direction-aware pick -- before touching `decodeStep`, `VideoRing::pick`, a direction change or a keyframe
gate."

## (7) RISKS

- **K1 (the strongest counterargument):** "An offline rig without GL or compositing is not the live app. The live decode threads
  are slower, and the in-place window's margin may not survive."
  - Why it probably holds: under load 6-9 the rig's HEAD late reached 93-101 (live 123) while the patch stayed at 0 in the same
    interleaved batch (batch5).
  - The patch holds 0 down to 192 MB: one budget step of margin.
  - The window grows exactly when decoding is slow (`servedDuringLead` rises with the measured EMA).
  - Cheapest discriminating test: G4 itself (5 x 2 interleaved, ~5 min of rig time).
- **K2 `decodeMsEma_` misestimates.** An overestimate gives a smaller window (today's at worst). An underestimate means the
  window's bottom frames get replaced by nearer ones: a wasted 0.1-0.3 ms copy each at 1080p (gopcache-fix F3 micro-bench).
  Never a stall. Watch `video_gopcache_evictions` / `_drops` (INFO) in G4 / G5.
- **K3 The keyframe below a window is estimated** from `gopFramesEst_`, the index's LONGEST keyframe interval
  (VideoPlayer.cpp:293-296). Exact for fixed-GOP files. With scene-cut GOPs the lead can be over-estimated (a self-trimming
  window) or under-estimated (today's window). T1a (iv) pins the invariant avail0 <= window <= share.
- **K4 The landing-key gate trusts the demuxer.** A partly mislabelled index (a non-key packet flagged key) would let a run store
  garbage. An all-key index is intra-only (E15). No fixture exists for a partial mislabel (INFERRED rare).
- **K5 Forward identity:** two flag writes on the forward decode path. G1's golden traces are the gate (verified offline 510/5,
  3 of 3 runs).
- **K6 T1b pins the decode estimate through the friend seam.** A renamed member breaks the pin loudly (late > 4), never
  silently.
- **K7 The GC9 rule is drift-sized**, so it is INFO in G5; run 10 x 2 if Harmony wants it as a gate.
- **K8 The in-place window also changes 4K / tight-share behaviour** (E7: 4K @ 512 MB 2.2 -> 14.4/s). It spends decode CPU where
  HEAD sat in DEMAND holds. u10 fps is INFO in G6.

## (8) WHAT ONLY BORIS CAN CHECK

- **B1** Four 4K clips in one column, all set to Reverse, then trigger the column (his Mac gives each ~43 cached frames). Before,
  they barely moved (~2 frames/s each, INFERRED from the offline 512 MB share); now they should move, roughly half as smooth as
  forward. Is that acceptable (Q2)?
- **B2** If he uses screen recordings or game captures: reverse one. Fast parts show fewer frames than forward (by design, Q1).
- **B3** Any reversed clip he knows, in a column of four: it should look exactly as before and never stall. The change only
  alters WHEN frames are decoded, never which pixels are shown.

## (9) QUESTIONS (Boris's, each with a default so the build does not block)

- **Q1** Screen recordings, game captures and phone clips with dropped frames: in reverse, their fast parts show fewer frames
  than forward (0 % for normal phone video, up to about half for a screen recording). Fix now (a bigger change to the video
  engine), or only if you notice it? DEFAULT: documented now, fixed on request.
- **Q2** Four 4K clips reversing together will move but not look smooth on software decoding. OK for now? DEFAULT: yes;
  hardware decoding (VideoToolbox) stays filed.

## CROSS-LANE TOUCHES

- **lane tsan** edits VideoPlayer.cpp `open()`'s std::cerr lines (:83-316). This lane's hunks are all outside that range:
  - VideoPlayer.cpp :1306-1308 (`planPrefetchRun`), :1435-1436 (`onRunFrame`), :1598-1600 (`seekToTimestamp`), :1622-1626
    (`decodeNextFrame`);
  - VideoPlayer.h :289-290.

  The hunks are disjoint, so either merge order works.
- **Shared docs:**
  - CLAUDE.md :225 only (net -3 B);
  - pitfalls.md: Pitfall 62's entry only;
  - rendering.md :79 only;
  - testing-eyes.md :15 only.
- **Not touched:** MainComponent.cpp, Renderer.cpp, ApiServer / TestServer, tests/CMakeLists.txt (two fixture files only).
- **lane bt2:** nothing.

## EVIDENCE INDEX (scratch, this session; rebuildable from the descriptions above)

All paths are under `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/`.
- **Rigs:**
  - `gc7/rig.py`: builds a harness TU + `src/media` at HEAD (optionally patched) with `build/tests` test_gop_cache_store's flags
    and link line (objects replaced, output to scratch).
  - `gc7/u8h.cpp`: u8 offline. `gc7/u11h.cpp`: deck return. `gc7/gc7step.cpp`: T1b's prototype. `gc7/irstep2.cpp`: reverse laps,
    capped.
- **The patch:** `gc7/p6.py`, the exact items 1 + 3 with NOINPLACE / NOLANDKEY control switches. The builder writes a clean
  version without them.
- **Logs:** `gc7/dec.sh`; `gc7/batch1.log`, `batch5.log`, `batch6.log`, `batch6b.log`, `stepbatch.log`, `u11batch.log`.
- **R3:** `r3/landing2.c`, `r3/landing2.log`, `r3/files/` (the landing files, incl. the intra-refresh ones).
- **VFR:** `vfr/count.py`.
- **Fixture prototypes:** `gc7/video_h264_gop250_64x64.mp4`, `r3/video_h264_intrarefresh_64x64.mp4`.

## NOTEBOOK NOTES (for Harmony to append)

- **An offline u8.** Link the REAL VideoPlayer.cpp plus a harness TU using `build/tests/CMakeFiles/test_gop_cache_store.dir`'s
  flags and link line (objects replaced, `-o` to scratch). Run 4 players with real decode threads, a shared `GopCache::Budget`,
  and a 120 Hz reader doing `uploadToTexture`'s pick / Retire / judge without GL. It reproduces u8's dec/up and slot counts
  exactly, and its late count at ~0.4-0.8x: a fast A/B for decode-policy changes with no live app | gc7/rig.py, u8h.cpp.
- **A deterministic real-time emulation.** A stepped writer with a per-render-frame decode budget, and the decode-time EMA
  pinned before every step, counts decodes, not time: a RED-first test for a timing policy | gc7step.cpp.
- **`AV_FRAME_FLAG_KEY` is not decodability.** x264 intra-refresh MP4 / MKV land every seek on a key packet and output correct
  frames, but only frame 0 is key-flagged. MPEG-TS landing packets are never key-flagged, in every codec | r3/landing2.c.
- **MPEG-TS + MPEG-4 part 2 is the only garbage source found.** h264 / mpeg2 decoders output nothing before a keyframe instead
  | r3/landing2.log.

## COMPACT

**GC7:** four 21-frame shares (256 MB / 4, 1080p) plan a PREFETCH run only when half the share is free (VideoPlayer.cpp:1308,
GopCache.h:381-383). That gives ~11 frames (367 ms) of cover for a 250-decode (~440 ms) lead-in at the top of GOP 0, so every
late frame sits at the GOP-0 entry.
- Fix (item 1): the window = the slots free now + the slots held by nearer frames that the clock passes during the lead-in, at
  the measured decode time (`servedDuringLead`). With a lead-in shorter than a frame, nothing changes.
- Offline, the real player: late 48-75 -> 0 (5/5), slowest 30.0; 192 MB 0; uncapped identical; 4K @ 512 MB 2.2 -> 14.4/s.
- The committed suites pass, incl. the golden traces. The deterministic stepped RED is 69 -> 0.

**Probe (item 2):** rule u8 per B cap group plus `--selftest`.

**R3 (item 3):**
- Forward and reverse publish: unchanged. Accepted containers land every seek on a key packet with 0 wrong frames (17 files);
  the garbage is MPEG-4-in-TS only, which the app does not accept. A frame-key forward gate would freeze intra-refresh files.
- The GC3 STORE gate also accepts a landing on a demuxer-key packet: intra-refresh reverse goes from 169 seeks / 77 misses -> 23
  / 0 per lap in 12 frames, and the whole file becomes resident. TS garbage stays out.

**VFR (item 4):** by design, with numbers (0-56 % by shape); filed.

**Gates:** G1-G3 offline; G4 = GC7 live 5 x 2 at 256 MB on both arms (PASS iff late <= 40 AND slowest >= 20); G5 uncapped
no-regression 5 x 2; G6 / G7 INFO / smoke.

STATUS: PLAN COMPLETE -- 4 items (GC7 in-place window, probe cap grouping + selftest, R3 decided + the demuxer-key store gate,
VFR documented); every RED pre-registered with HEAD's measured values; 2 Boris questions with defaults; no live gate run (Harmony's).

## HARMONY ADOPTION (s-rta-0930, 2026-09-30 13:51:57)
ADOPTED: the section "## ARCHITECT RULING (s-rta-0930)" of .harmony/.reports/s-rta-0930/ruling-gop2.md IN FULL (amendments A1..,
FINAL GATE LIST, DOCS, ERRATA A15, FILED F1-F4, RISKS). Where it differs from this plan body, the RULING wins. The ruling ruled the
three blind seats' papers (recovered from the workflow journal; now also on disk as attack-gop2-{decode,gates,vj}.md).
Harmony decisions:
- H1 Build items 1-5 as amended (GC7 in-place window anchored on the index's real keyframe, A1; landing-pts gate, A2; probe u8
  per-B-cap rule + --selftest; R3 store gate on demuxer-key landing packets; VFR documented by-design with numbers).
- H2 The G4 GC7 decision rule (validity V1-V3 -> bar -> lane safety, PASS / PARTIAL / FAIL) is pre-registered as written; Harmony
  runs G1-G7 after the merge. The lane itself runs the ctest RED/GREEN, the probe --selftest and any live row it needs under the lock.
- H3 Boris questions Q1 (VFR) and Q2 (four 4K reversers) go on the Boris page with their defaults; the build does not wait.
- H4 Filed F1-F4 go to the loose-ends ledger, not this lane.
- H5 Pitfall numbers: write "Pitfall NN" for anything new; Harmony assigns (next free 63). Pitfall 62 edits are in scope.
- H6 Cross-lane: lane tsan may touch VideoPlayer.cpp open() std::cerr lines only; keep this lane's hunks where the plan says.
