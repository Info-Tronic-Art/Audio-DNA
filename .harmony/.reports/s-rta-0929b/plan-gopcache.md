# plan-gopcache -- reverse / ping-pong video served from a decode-thread GOP cache (s-rta-0929b, lane "gopcache")

Architect (Fable), 2026-09-29. Main HEAD d88d2ea. Fence: `src/media/VideoPlayer.*`, NEW `src/media/GopCache.h` +
`src/media/GopCacheStore.{h,cpp}`, their ctests, `.harmony/probe-vupload.*`, `docs/claude/*.md`. Two named deviations
(section 3.0). Every `file:line` below was read at d88d2ea; anything not re-derived is marked INFERRED / ASSUMED.

QUESTION: make reverse and ping-pong playback of long-GOP video files as smooth as forward playback (the source frame
rate, late 0), inside a bounded memory budget, with forward playback unchanged -- decide the mechanism, storage, budgets,
threads, transport edge cases, gates, and the exact code changes.

APPROACH (stated first): (1) a per-player, decode-thread-owned **sliding GOP cache** of decoded frames in the decoder's
NATIVE pixel format (AVFrame copies, e.g. yuv420p = 3.11 MB at 1080p), filled by **runs** (one keyframe seek + a forward
decode that STORES the frames the window wants and DROPS the rest) and served backwards into the existing ring through the
UNCHANGED `convertInto` (sws) -> IOSurface slot -> blit path; the window size is what a shared 2 GiB budget allows (a full
GOP cache when it fits, a bounded reverse-decode window when it does not -- one mechanism, no mode switch); (2) the ring's
`pick`/`peek` get a `reverse` argument that MIRRORS their pts comparisons, because today's pick (VideoRing.h:113-121) frees
every Ready frame at or below the chosen one -- in reverse that is the writer's next frame, which is why a re-seek buys
~1 shown frame today; (3) a direction change is a discontinuity (a request-generation bump), so the ring is never mixed
across a flip; (4) PingPong forward play retains the last frames it publishes so the top turn is served from the cache
while the first reverse run decodes. Target: reverse at the source frame rate for BOTH GOP lengths (Harmony's reading is
right: 11.3/s on GOP-30 is a third of a 30 fps source, the mechanism is the same for both).

## 0. ESTABLISHED (cited) and RE-DERIVED (file:line at d88d2ea)

Established (prior reports, cited by the dispatch):
- vupload.md:30 -- u7 on main after the look-ahead fix: g30 reverse 11.3 uploads/s (late 212), g250 reverse 4.5/s (late
  456.5), ping-pong 30.2-30.4 late 0; vupload.md:35 -- 50-52 seeks per 5 s in g30 reverse. plan-vupload.md:157-160 (R-12)
  and :715 (Q5): the GOP cache is its own lane; fence decodeLoop / seekToTimestamp. vupload.md:95: kWriterLookAhead.

Re-derived from source (VERIFIED unless marked):
- The transport: `advanceTransport` VideoPlayer.cpp:366-428. Direction = reverse XOR the ping-pong leg (:379-382);
  `currentTime_ += dt * speed * direction` (:384); a Loop wrap in either direction sets `discontinuity_` (:393, :411) which
  `advanceFrame` (:326-329) / `advanceClock` (:359-363) turn into a generation bump; a PingPong reflection (:395-398,
  :413-415) flips `pingPongForward_` with NO discontinuity; `setReverse` (VideoPlayer.h:79) is a plain atomic store, NO
  discontinuity. Renderer::syncMedia sets reverse only outside PingPong (Renderer.cpp:1793-1794), the loop mode (:1796-1801),
  the speed every frame incl. BPMSync (:1803-1816), and seeks to the in-point at an active out-point in Loop AND PingPong
  (:1836-1840: no reflection at an out point).
- The reader: `uploadToTexture` VideoPlayer.cpp:430-538. `ring_.peek(currentTime_, gen, 0.5 fd)` (:448) and
  `ring_.pick(currentTime_, gen, 0.5 fd, (kWriterLookAhead + 1) fd)` (:467). `pick` (VideoRing.h:73-123): the candidate set
  is Ready same-gen with pts <= clock + tol (:89), the chosen is the NEWEST of them (:91), frames above clock + tol +
  dropAhead are freed (:98-102), and after the pick every Ready same-gen frame with pts <= the chosen pts is freed as
  "skipped" (:113-121). `peek` mirrors the choice with no state change (:129-147). Neither knows the direction.
- The writer: `decodeLoop` VideoPlayer.cpp:668-727. Reseek when the generation changed or `decide()` says so (:693-700);
  `decide` (VideoRing.h:253-258): Reseek iff want < newest - reseekBehind (max(0.1 s, 3 fd) = 0.1 s at 30 fps, :676) or want
  > newest + 2 s; NONREF skipping while > 10 frames from the target (:702-704, VideoRing.h:268-271); `onDecoded` (:729-770)
  drops a frame more than 1.5 fd behind `want` (:740-744) and otherwise waits for a ring slot in a 20 ms loop that gives up
  on a gen change or a Reseek verdict (:747-760); `seekToTimestamp` (:780-801) = av_seek_frame BACKWARD + flush;
  `decodeNextFrame` (:803-840); `convertInto(slot)` (:842-861) sws_scales `decodedFrame_` bottom-up into the slot under the
  IOSurface lock. The trim request purges Free slots (:683-684); the thread frees FFmpeg at exit (:725).
- Why reverse is slow TODAY (the mechanism, from the lines above): in reverse the clock runs down, but the writer only
  produces frames UPWARD from a keyframe. Per cycle it seeks to the keyframe at or before `want` (:698), decodes and drops
  every frame below want - 1.5 fd (:740), publishes the first frame at or above that line, then keeps decoding upward and
  publishing frames ABOVE the clock until the ring is full (:747). The reader picks the newest frame <= clock + 0.5 fd and
  FREES the one below it as "skipped" (VideoRing.h:113-121) -- the very frame it needs next -- and the frames above the
  clock sit in the ring until they are 3.5 fd above it (:98-102), keeping the writer out. When the clock has fallen 0.1 s
  below the newest published frame, `decide` says Reseek (:756-758, :695) and the cycle repeats. Result: about one shown
  frame per (seek + catch-up) cycle. Consistent with the measurements: g30 = 50-52 seeks and 56 uploads per 5 s (1.1
  uploads per seek, ~98 ms per cycle); g250 = 4.5 uploads/s (~220 ms per cycle).
- Where the time goes, per GOP length -- decode cost MEASURED by the architect with the CLI ffmpeg 8.0 (the same
  libavcodec the app links, the app's `thread_count` 2 (VideoPlayer.cpp:124), the u7 fixtures from the vupload
  scratchpad; `$SBX/decode-rig.sh`): 1080p H.264 yuv420p (has_b_frames 2): 300 frames in 0.59-0.81 s = **1.7-2.4 ms per
  frame**; with `-skip_frame noref` 198-202 of 300 frames come out, 0.52-0.60 s (a 13 % saving only: most B-frames are
  reference frames in these encodes); 4K: 2.14 s / 300 = **6.8 ms per frame**; `-ss 8.0` then one frame on a1080_g250 =
  0.50 s (240 frames from keyframe 0 = 1.7 ms each); a4k 1.77 s. Fixtures: g30_1080 keyframes every 1.0 s (GOP 30, 10 s,
  300 frames); a1080_g250 keyframes at 0 and 8.333 s (GOP 250). So per cycle on main: g30 = ~15 decodes (avg half a GOP)
  = ~30 ms of decode in a ~98 ms cycle (the rest: the 20 ms ring-full waits, the 3.5 fd ahead-drop delay, thread wakeups);
  g250 = ~125 decodes = ~210 ms of ~220 ms (decode-bound). INFERRED for the app (the CLI drives the same decoder
  differently); the diagnosis row in section 1 measures it in the app.
- Memory arithmetic: a 1080p yuv420p AVFrame = 1920 x 1080 x 1.5 = 3,110,400 B (+ alignment; `av_image_get_buffer_size`
  gives the exact number -- the builder logs it); 4K = 12.44 MB; BGRA (the ring's format) would be 8.29 / 33.2 MB. A 250-
  frame GOP = 778 MB (1080p) / 3.11 GB (4K) in native format; the whole 300-frame u7 fixture = 933 MB.
- Pitfalls read in full: 53 (pitfalls.md:115), 54 (:117), 55 (:119), 56 (:126), 60 (:129). The invariants this lane must
  keep: never 0 / no media for a shown player (53/56/60), the shown slot is the reader's (60 rule 2), fenced slots return
  only when signaled (60 rule 3), purge only Free slots from the writer (60 rule 5), no FFmpeg on the GL thread (56), a
  per-frame object store needs a slot table + a budget (54), no new mutex (Sacred Rule 2).
- The u7 ping-pong scenes on main measure FORWARD play only (VERIFIED): probe-vupload.py:277-295 loads the clip at
  in-point 0 (`vclip` default ip=0.0, probe-video.py:326-329), triggers, waits 1 s and polls 5 s -- the 10 s fixture never
  reaches its end inside the window, so "ping-pong 30.2-30.4, late 0" is forward play. The lane adds turn scenes (4.3).
- BORIS_DECISIONS.md "Playback Behaviour" (:320-360): no ruling on reverse or ping-pong (grep 0). Nothing to quote.

## 1. DIAGNOSIS STEP (inside the lane, first; pre-registered)

No TEMPORARY instrumentation is needed: the counters exist (`video_frames_decoded`, `video_frames_dropped`, `video_seeks`,
`video_uploads` in both /api/state blocks, TestServer.cpp:703-707 / ApiServer.cpp:1434-1438) and are not yet printed by u7.
Step D1 (commit c0): u7's DATA line gains `decoded`, `dropped`, `seeks` per scene (permanent, cheap). Run
`probe-vupload-ab.sh <main app> <main app> 5 probe-vupload.sh u7_reverse_pingpong` (both arms = main; 5 launches; the
quiet lock) and compute per scene decoded/uploads and seeks/uploads.
Pre-registered expectation (branch A): g30 reverse decoded/upload in [10, 30], seeks/upload in [0.8, 1.2]; g250 reverse
decoded/upload in [60, 140], seeks/upload in [0.8, 1.2]; the (forward-only) ping-pong scenes decoded/upload in [1.0, 1.3],
seeks 0-2 per 5 s. Action: build the plan as written.
Branch B (both reverse scenes show decoded/upload < 5): the catch-up is NOT the cost -- the ring / wait mechanics are.
Action: build commits c1-c1b only (the direction-aware pick + the flip discontinuity + the step refactor), re-run u7 x 5
interleaved vs main; if both reverse scenes reach the section-4 bar (>= 28.5/s, late <= 10) STOP after c1b and report --
Harmony re-rules whether the cache (c2-c4) is still wanted. If not, continue with c2-c4 as planned.
Why the design does not fork on the decode/wait split otherwise: both mechanisms (one seek per shown frame; the pick freeing
the next frame) are proven from source above, and g250's 125 decodes per cycle cannot reach 30 fps without amortizing the
catch-up over many shown frames -- a cache -- whatever the wait share is.

## 2. TRADEOFFS CONSIDERED

- **Direction-aware pick + a bigger ring, no cache** (rejected as the whole answer): with kSlots 8 the writer still
  re-seeks every ~6 frames; g250 stays at 125 decodes per 6 frames = ~5-6 fps. It IS part of the answer (3.2) -- without it
  the cache's frames are freed by the reader before they are shown.
- **Full-GOP cache, always** (rejected as the only mode): 3.11 GB per 4K player at GOP 250, 778 MB at 1080p; a column of
  four does not fit any sane budget. Adopted as the LIMIT of the sliding window: when the budget allows N >= GOP + look-ahead
  frames, the window IS the GOP cache (one decode per frame, then none for a resident file).
- **Bounded reverse-decode window ("chunked reverse") only, fixed W** (rejected as fixed): decodes per shown frame =
  G / (2 W) + 1/2 (each window of W costs a decode from the keyframe: the sum over a GOP is G^2 / 2W); at W = 16 on 4K
  g250 that is 8.3 decodes x 6.8 ms = 56 ms per 33 ms frame -- not smooth; the window must be budget-sized, i.e. the
  sliding cache above with N derived from the budget. Same mechanism, W = whatever the slots allow.
- **Storage in the ring's BGRA (IOSurfaces) instead of native YUV** (rejected): 2.7 x the memory for yuv420p sources, and
  a cached BGRA frame would enter the slot by memcpy instead of sws_scale -- a second write path into the IOSurface slots.
  Native-format AVFrame copies keep `convertInto` (the ONLY slot writer) unchanged: the served frame is the same decoded
  planes, so the uploaded pixels are identical to forward play by construction (gate: ctest identity, section 4.1).
- **`av_frame_ref` (zero-copy) instead of copies** (rejected): a retained ref pins a buffer of the decoder's frame pool;
  the pool grows to the high-water mark and NEVER shrinks until the codec context closes (INFERRED from libavcodec's
  `FramePool` / `av_buffer_pool` semantics), so "the cache is dropped" would free nothing. A copy costs ~0.3 ms (1080p)
  / ~1 ms (4K) per STORED frame -- 5-15 % of one decode -- and our buffers really return to the OS on free (INFERRED:
  macOS large-allocation free; u9 prints phys_footprint as INFO, the byte counter is the gate).
- **A second decoder / prefetch thread per player** (rejected): a second AVCodecContext per player (memory, another
  thread, FFmpeg threads x2); the decode itself is the bottleneck, and the single decode thread has idle time whenever
  the ring is full (~2 frames ahead = 66 ms at 30 fps) in which to step a run. A run is stepped one decode at a time
  from the ring-full wait ("idle work") -- no new thread.
- **Reusing `SeqVram::distances` for the eviction order** (rejected): SeqVram's trajectory walk is O(2n) per plan over
  the FRAME COUNT of the file (a 1-hour 60 fps file = 216,000 steps per run) and models the sequence's in/out jumps; the
  video cache's distances are closed-form (3.3) and the video in/out quirk (Renderer.cpp:1836: a jump, never a
  reflection) only affects eviction efficiency, never correctness. Same PRINCIPLE as Pitfall 54 (farthest-next-use, never
  LRU; the current and shown frames protected; a result outside a full window is dropped, not stored).
- **VideoToolbox** (out of scope, filed): would cut the decode cost 5-10 x but not the mechanism (one seek per shown
  frame); the cache is needed regardless and stays valid under a hardware decoder (it caches the decoder's output).
- **A gen bump on a direction change vs. handling the flip inside the pick** (chosen: the bump): without it the ring holds
  the OLD direction's look-ahead frames on the new direction's future side (e.g. after forward -> reverse, Ready W+1, W+2
  above the held W), and the mirrored pick would choose W+1 -- a forward step while reversing. The bump costs <= 2 wasted
  ring frames per flip (two per ping-pong lap) and is exactly what a Loop wrap already does (:393).

## 3. DECISION / SPEC

### 3.0 Rulings (R-1 .. R-16) and the two deviations

- R-1 The cache is decode-thread-owned: the slot table, the run state, the trajectory math -- read or written by the decode
  thread only (plus `freeFfmpeg` at thread exit / close / destruction, which the thread has left). The GL thread's rules
  (Pitfall 56 pick/hold, Pitfall 60 held slot / fences / budget / purge) are untouched. No new mutex. The audio thread is
  untouched. New shared state: ONE atomic (`wantReverse_`, GL -> decode), the budget's two atomics (decode threads among
  themselves), counters (relaxed atomics). Deviation D1 (VideoRing.h, the "upload path" in the dispatch's sense): `pick`
  and `peek` gain a trailing `bool reverse = false` that mirrors their pts comparisons (3.2). PROOF it must change:
  VideoRing.h:113-121 frees every same-gen Ready frame with pts <= the chosen one -- in reverse those are the frames the
  writer queued for the next picks; and :89-97 choose the NEWEST candidate, which in reverse picks a queued frame up to a
  full frame early. No writer-side ordering can avoid both (a frame published below the clock is picked at once). With
  reverse = false the function is byte-for-byte today's; forward ctests unchanged. `uploadSlot` / `blitSlot` /
  `clientUploadSurface` / `pollFences` / `releaseGL` / `trimIfIdle` / `purgeFreeSlots` / `unpurge` / VideoUploadBudget.h /
  Retire / the slot state protocol: unchanged.
- R-2 A direction change is a discontinuity: `advanceTransport` tracks the effective direction (`reverseNow_`, GL thread)
  and sets `discontinuity_` when it changes -> a generation bump in both `advanceFrame` and `advanceClock` (the existing
  wrap mechanism, :326-329 / :359-363). The reader's stale-generation drop (VideoRing.h:83-87) cleans the ring; the held
  frame stays (Retire). The writer sees the new generation + `wantReverse_` and re-plans (3.5).
- R-3 Storage = AVFrame copies in the decoder's native pixel format and size (`av_frame_get_buffer` once per slot,
  `av_frame_copy` per stored frame; pts kept beside the slot). A slot is allocated on demand up to the budget and reused
  (never per-frame malloc / free after warm-up); `clear()` frees every slot and returns its bytes. A decoded frame whose
  format / size differs from the slots' is dropped (never stored).
- R-4 Budget: `GopCache::kBudgetBytes = 2 GiB` for ALL players' caches together (ASSUMED for a 32 GiB M1 Pro; SeqVram's
  1 GiB is VRAM for sequences; open question Q1: scale by hw.memsize). Shared through a process-wide `GopCache::Budget`
  (`GopCache::sharedBudget()`, a function-local static of two atomics; ctests inject their own via a setter). Per-player
  cap = kBudgetBytes / active caches ("active" = holds >= 1 frame). Floors win (SeqVram H10): a cache may always hold
  `kMinFrames = 8` frames even over the budget, and `video_gopcache_over_budget` says so. A cache above its cap (another
  reverser joined) evicts down to the cap at its next store decision -- lazily, farthest-next-use first.
- R-5 Eviction = farthest next use along the clip's trajectory, never LRU (Pitfall 54), closed-form (3.3); the wanted
  frame and the frame being served are never evicted; a decoded frame outside a full window is DROPPED, not stored; two
  sub-pools per cache -- FUTURE (the trajectory ahead of the clock; capacity slots - behindCap) and BEHIND (the last
  `kBehindFrames = 16` frames just passed, Loop / OneShot reverse only; PingPong puts passed frames in FUTURE with their
  real after-the-bounce distance) so a manual reverse -> forward flip is served for 16 frames while the decoder is re-
  positioned (3.5 Reposition run).
- R-6 Runs (decode-thread state machine, one decode per step, stepped from the ring-full wait): DEMAND (the wanted frame
  is not resident: seek to the keyframe <= target, decode, store what the window wants, PUBLISH the target directly from
  `decodedFrame_` -- latency first, today's catch-up shape), PREFETCH (reverse only: the unserved resident frames ahead
  are fewer than `prefetchAt`; target = the first non-resident trajectory frame; seek to `target - (available - 1) fd`
  so one keyframe seek covers the whole next window; store everything the window wants; publish nothing), REPOSITION
  (forward only: cache hits are being served, the decoder's next output is not the frame after the cache's top; seek
  if needed and decode-and-drop toward it; publish nothing, store nothing but PingPong retention). A generation change,
  a direction change or EOF ends a run; a run's stored frames stay resident.
- R-7 NONREF skipping inside a run only BELOW the storage window minus `kFullDecodeFrames = 10` (VideoRing::Policy's
  value): a skipped non-reference frame is never output, so a hole would otherwise appear in the window and cost a demand
  run (a seek + catch-up) per hole. Reference chains are intact, so stored frames equal a full decode (the ctest proves it).
- R-8 Intra-only files (every frame a keyframe: HAP, ProRes, MJPEG, DNxHD, raw): no prefetch, no retention -- learned:
  when two consecutive DEMAND runs decode exactly one frame (the seek landed on the target) `intraOnly_` = true; reverse
  then is one seek + one decode + a direct publish per frame (today's path with the direction-aware pick queuing them),
  no copy. Cleared on a new open only.
- R-9 PingPong forward retention: in PingPong mode a forward-playing (non-intra) player stores a copy of every frame it
  publishes into the BEHIND pool, capacity `retainFrames = clamp(ceil(gopFramesEst * decodeMsEma / frameMsEff *
  kRetainSafety(2.0)), kMinRetainFrames(16), capFrames)`, `decodeMsEma` measured by the decode thread (seed
  `kDecodeMsSeedPerMpix = 1.0` ms per megapixel: 2.1 ms at 1080p vs 2.0 measured, 8.3 at 4K vs 6.8), `gopFramesEst`
  from the container index at open() (`avformat_index_get_entries_count` / `avformat_index_get_entry`, VERIFIED present
  in /opt/homebrew/include/libavformat/avformat.h:2740/2753, guarded `#if LIBAVFORMAT_VERSION_MAJOR >= 59`; no index ->
  `kDefaultGopFrames = 250`; refined to max(est, a run's decoded count) when no index). Loop / OneShot forward play stores
  NOTHING -- forward playback of the common case is unchanged in memory and CPU.
- R-10 The cache is dropped: on the idle trim (`trimRequested_`, the R-14 owner pair: the GL thread asks after 1 s without
  a draw, the decode thread purges its Free slots AND clears the cache -- one more line at VideoPlayer.cpp:683-684; the
  parked thread is notified for exactly this), in `freeFfmpeg` (thread exit / close of a never-started player /
  destruction; the cache's bytes go back to the budget there; the destructor runs after the thread exited --
  drainRetiredMedia's gate, Renderer.cpp:1636-1646). NOT dropped on a seek, a wrap or a flip (its frames may be hits; stale
  ones are evicted by distance as slots are needed). A clip switch retires the player -> `freeFfmpeg`.
- R-11 The reverse publish cadence is the ring's: the writer queues up to kWriterLookAhead (2) frames BELOW the clock
  (the mirrored pick serves them at the right clock crossings), waits for a slot exactly like forward (the reader's
  `releasedThisFrame_` notify, :494), and steps a run in the wait. Uploads/s in reverse therefore equal forward's.
- R-12 Graceful degradation, never a stall or a black frame: a run that cannot keep up (4 x 4K, a tiny cap) means the
  reader HOLDS the shown frame (`video_late_frames`), never pending (a shown player), never texture 0 (Pitfall 60 rule 2).
  A GOP that does not fit = a window smaller than the GOP = more decodes per shown frame; the counters
  (`video_gopcache_run_decodes` / `video_gopcache_hits`) show the ratio.
- R-13 The frame-index mapping f = round(pts / fd) is used for the trajectory math only; lookups are by pts within 0.5 fd
  (VFR files: a served frame may be off by one, as forward's pick tolerance already allows).
- R-14 Counters (VideoStats, `video_gopcache_*` in BOTH state blocks -- the video lane's precedent, plan-vupload R-15):
  bytes, frames, active, hits, misses (demand runs), runs (prefetch + reposition), run_decodes, evictions, drops,
  over_budget, cap_bytes (INFO). Deviation D2: ApiServer.cpp (:1447-1453 block) and TestServer.cpp (:716-722 block) each
  gain these lines (outside the fence; the same edit the vupload lane made for its 7 fields).
- R-15 `decodeLoop` is refactored into an outer loop (exit / trim / park / wait) around `decodeStep()` = one unit of
  progress that never blocks (3.5): the ctests drive the REAL writer without a thread (the vupload lane's
  `VideoPlayerTestAccess` shape), and the ring-full wait becomes the place where runs are stepped. Forward semantics are
  preserved line by line (the equivalence list in 3.5); commit c1b lands the refactor ALONE and runs the whole forward
  battery before any cache code exists.
- R-16 Bars are absolute where the number is a frame rate (uploads/s of a 30 fps source is render-rate independent) and
  interleaved A/B where they are not (4.3); nothing existing is re-thresholded; `u7UploadsRatioMin` (0.9) stays for the
  cross-app rule.

### 3.1 Files

- NEW `src/media/GopCache.h` (pure: no FFmpeg, no GL, no JUCE; `tests/test_gop_cache.cpp` drives it): constants, `Budget`,
  `distance()`, `Pools`, `judgeStore()`, `planRun()`, `retainFrames()`, `prefetchAt()`.
- NEW `src/media/GopCacheStore.{h,cpp}` (FFmpeg-facing; decode thread only): the AVFrame slot table + budget accounting.
- `src/media/VideoRing.h`: `pick` / `peek` `reverse` argument (D1). `Policy` / `decide` / `shouldPublish` /
  `useSkipNonRef` / `judge` / `Retire` / `ShownState` untouched.
- `src/media/VideoPlayer.h/.cpp`: the GL-thread direction + discontinuity, the pick/peek call sites, the decode-thread
  step machine, runs, the cache, retention, trim / free hooks, counters, `convertInto(const AVFrame*, int slot)`.
- `src/media/VideoStats.h`: the gopcache counters.
- `src/api/ApiServer.cpp`, `src/test/TestServer.cpp`: the counter lines (D2).
- `tests/CMakeLists.txt`: `test_gop_cache` (pure, registered like test_video_ring :2747-2751), `test_gop_cache_store`
  (the REAL VideoPlayer + FFmpeg, no GL, registered like test_video_player_open :2764-2792 with TEST_FIXTURES_DIR),
  `test_video_ring` (+ cases). NEW fixture `tests/fixtures/video_h264_gop10_64x64.mp4` (recipe in 4.1; libx264 is in the
  brew ffmpeg -- VERIFIED `ffmpeg -encoders | grep libx264`).
- `.harmony/probe-vupload.{py,json}`: u7 DATA fields + two turn scenes + the reverse bars + a mid-window capture; NEW rows
  u8 / u9 / u10; `.harmony/probe-vupload-ab.py`: the u7 rule keeps its shape (per scene, 0.9 x A, late <= A).
- Docs (section 8): `docs/claude/rendering.md` (:75, :77 edits + a new paragraph), `docs/claude/pitfalls.md` (NN),
  `docs/claude/testing-eyes.md` (the probe-vupload paragraph), `CLAUDE.md` (index line NN), `.harmony/APP-INVENTORY.md`.

### 3.2 VideoRing.h -- the direction-aware pick / peek (D1)

Signature: `Pick pick(double clock, uint32_t gen, double tolSec, double dropAheadSec = inf, bool reverse = false)`;
`Pick peek(double clock, uint32_t gen, double tolSec, bool reverse = false) const`. Implementation = today's loop over
`key = sgn * pts` with `sgn = reverse ? -1 : 1` and `limit = sgn * clock + tolSec`: candidate iff key <= limit; the chosen
is the largest key (ties: the newest seq, as today); ahead-drop iff key > limit + dropAheadSec; readyAhead otherwise;
after the pick, skipped iff same-gen Ready with key <= the chosen key. Semantics in reverse: the candidates are the frames
with pts >= clock - tol, the chosen is the SMALLEST pts (the frame whose interval the falling clock has just entered; never
a frame below the held one before the clock crosses held - 0.5 fd), the passed-over frames are those ABOVE the chosen, the
ahead-drop line is 3.5 fd BELOW the clock (never reached by a 2-frame look-ahead, as in forward), readyAhead counts the
queued frames below the clock. `reverse = false`: byte-for-byte today's code paths (the ctests of forward stay identical).
Call sites: VideoPlayer.cpp:448 (`peek(..., reverseNow_)`) and :467 (`pick(..., (kWriterLookAhead + 1) * frameDur_,
reverseNow_)`). `judge()` (VideoRing.h:285-293) is direction-agnostic (|clock - lastShown|) and stays.

### 3.3 GopCache.h -- the pure policy

```
namespace GopCache {
constexpr size_t kBudgetBytes      = size_t{ 2 } << 30;  // every player's cache together (R-4)
constexpr int    kMinFrames        = 8;     // floor per active cache (floors win)
constexpr int    kBehindFrames     = 16;    // the BEHIND pool in Loop / OneShot reverse (a flip's cover)
constexpr int    kMinRetainFrames  = 16;    // PingPong forward retention floor
constexpr double kRetainSafety     = 2.0;
constexpr int    kFullDecodeFrames = 10;    // no NONREF skip within this many frames below the storage window
constexpr int    kDefaultGopFrames = 250;   // no container index
constexpr double kDecodeMsSeedPerMpix = 1.0;
constexpr int    kFar = 1 << 30;

struct Budget {                            // process-wide (sharedBudget()), decode threads only; ctests inject their own
    std::atomic<int64_t> bytes{ 0 }; std::atomic<int> active{ 0 };
    size_t capBytes() const;               // kBudgetBytes / max(1, active)
    bool tryTake(size_t n, size_t mineBytes, size_t floorBytes);   // CAS on bytes: allowed iff mine + n <= floor (floors
                                           // win, over_budget flagged) OR (mine + n <= capBytes() AND bytes + n <= kBudgetBytes)
    void give(size_t n);                   // never below 0
    void activate(); void deactivate();    // the first frame stored / clear()
};
Budget& sharedBudget();

enum class Mode : uint8_t { Loop, PingPong, OneShot };
// Steps until frame f is shown again, from cur moving in `forward` direction; closed-form (no walk):
//   forward Loop: f >= cur ? f - cur : n - cur + f;      forward PingPong: f >= cur ? f - cur : 2(n-1) - cur - f
//   forward OneShot: f >= cur ? f - cur : kFar;           reverse Loop: f <= cur ? cur - f : cur + n - f
//   reverse PingPong: f <= cur ? cur - f : cur + f;       reverse OneShot: f <= cur ? cur - f : kFar
int distance(Mode m, bool forward, int n, int cur, int f);
enum class Pool : uint8_t { Future, Behind, None };
// Loop / OneShot reverse: 0 < f - cur <= kBehindFrames -> Behind; PingPong: everything with a finite distance -> Future
// (its real after-the-bounce distance); forward PingPong (retention): f < cur -> Behind (key = distance), f >= cur -> None
// (the decoder produces the future anyway); forward Loop / OneShot: None (nothing is stored).
Pool poolOf(Mode m, bool forward, int n, int cur, int f);
// behindCap per mode (the player picks it before each decision): Loop / OneShot reverse = min(kBehindFrames, capSlots / 4);
// PingPong reverse = 0 (passed frames are Future with their after-the-bounce distance); PingPong forward = retainFrames
// (the retention pool, capped at capSlots / 2); Loop / OneShot forward = 0 (nothing is stored).
int behindCapFor(Mode m, bool forward, int capSlots, int retainFrames);

struct Slot { int frame = -1; int key = kFar; Pool pool = Pool::None; };   // the store's per-slot policy view
enum class Keep : uint8_t { Store, Replace, Drop };
struct Verdict { Keep keep = Keep::Drop; int slot = -1; };
// f decoded: Store into a free slot (or a new one while slots < capSlots, and the pool has room: Future count <
// capSlots - behindCap, Behind count < behindCap); else Replace the resident of the SAME pool with the largest key if that
// key > key(f) (never `protectA` / `protectB`: the wanted frame and the one being served); else Drop.
Verdict judgeStore(const std::vector<Slot>& slots, int capSlots, int behindCap, int f, int key, Pool pool,
                   int protectA, int protectB);
// Over the cap (another cache joined): the slot to evict now (largest key over both pools, protected excluded), or -1.
int evictOne(const std::vector<Slot>& slots, int capSlots, int protectA, int protectB);
enum class RunKind : uint8_t { None, Demand, Prefetch, Reposition };
struct Run { RunKind kind = RunKind::None; int target = -1; int seekFrom = -1; int windowLo = -1; bool seekPending = true;
             int decoded = 0; };
// Reverse: the unserved resident trajectory frames after `served` (served-1, served-2, ... resident and contiguous).
int unservedAhead(const std::vector<Slot>& slots, Mode m, int n, int served);
// prefetchAt = max(capSlots / 2, min(capSlots - 2, ceil(gopFrames * decodeMs / frameMsEff * 1.2)))
int prefetchAt(int capSlots, int gopFrames, double decodeMs, double frameMsEff);
// Reverse: a Prefetch run when unservedAhead < prefetchAt and the trajectory continues (Loop wraps to n-1; PingPong
// reflects -> None: the forward leg needs no run; OneShot stops at 0): target = the first non-resident frame after the
// resident run; seekFrom = max(0, target - (available - 1)) where available = free slots + Future residents with key >
// distance(target) (the ones a store would replace) -- capped at capSlots.
Run planPrefetch(const std::vector<Slot>& slots, Mode m, int n, int cur, int served, int capSlots, int behindCap,
                 int prefetchAt, bool intraOnly);
// PingPong forward retention size (R-9).
int retainFrames(int gopFrames, double decodeMs, double frameMsEff, int capFrames);
}
```
Everything above is a pure function of ints / doubles; `tests/test_gop_cache.cpp` proves each (4.1).

### 3.4 GopCacheStore (the AVFrame slot table; decode thread only)

```
class GopCacheStore {           // src/media/GopCacheStore.h/.cpp
public:
    void configure(int pixFmt, int w, int h, double frameDur, GopCache::Budget* budget, VideoStats* stats);
    // Copy `src` (its pts) into a free slot / the slot named by the verdict; false = no slot could be taken (budget) or
    // the frame differs in format/size. Allocates a new AVFrame + buffer only while slots() < cap and the budget allows.
    bool store(const AVFrame* src, double pts, const GopCache::Verdict& v);
    const AVFrame* get(double pts) const;      // resident frame within 0.5 fd, else nullptr (linear scan; N <= ~700)
    bool has(double pts) const;
    void evict(int slot);                      // the slot becomes free (memory kept for reuse)
    void freeSlot(int slot);                   // av_frame_free + budget->give (the trim to a smaller cap)
    void clear();                              // every slot freed, bytes given back, deactivate(); counters
    int slots() const; int resident() const; size_t bytes() const; size_t frameBytes() const;
    std::vector<GopCache::Slot>& policyView(); // frame / pool / key per slot, refreshed by the player before a decision
};
```
frameBytes = `av_image_get_buffer_size(fmt, w, h, 32)` (logged once per player in the "Opened" line as `cacheFrame=`);
the store is `configure`d in `open()` after the codec is open (message thread / MediaOpener pool -- touches only this
object, Pitfall 58's rule) and used only by the decode thread afterwards. `active` flips in store() (0 -> 1 frames) and
clear().

### 3.5 VideoPlayer changes

GL thread (uploadToTexture / advanceTransport; the hot path gains two loads and one compare):
- `bool reverseNow_ = false;` (GL thread), `std::atomic<bool> wantReverse_{ false };` (GL -> decode), `bool everAdvanced_`.
- `advanceTransport` (:366-428): after `direction` is final (:382): `const bool rev = direction < 0.0; if (rev !=
  reverseNow_) { reverseNow_ = rev; discontinuity_ = true; }` (R-2). Nothing else changes in the transport math.
- `advanceFrame` (:325): `wantReverse_.store(reverseNow_, release)` beside the `wantTime_` store (before the gen bump);
  `advanceClock` (:358): the same store before its gen bump (and in its seek branch).
- `uploadToTexture`: :448 `peek(..., reverseNow_)`; :467 `pick(..., reverseNow_)`. Nothing else.

Decode thread -- the step machine (R-15). New members: `GopCacheStore cache_; GopCache::Budget* cacheBudget_ =
&GopCache::sharedBudget(); GopCache::Run run_; double servedPts_ = -1; bool haveServed_ = false; bool pendingPublish_ =
false; bool intraOnly_ = false; int intraRuns_ = 0; double decodeMsEma_; int gopFramesEst_; bool haveIndex_;`.
`decodeLoop()` becomes:
```
while (!threadShouldExit()) {
    if (trimRequested_.exchange(false)) { purgeFreeSlots(); cache_.clear(); }        // R-10 (today's :683-684 + one call)
    if (idleStep(nowMs(), lastDrawMs_, pol) == Park) { park(); continue; }           // today's :687-691
    if (!decodeStep()) thread_.wait(20);                                             // today's 20 ms waits (:759, :719)
}
freeFfmpeg();   // frees the cache too (R-10)
```
`bool decodeStep()` (true = made progress; false = nothing can be done now):
```
g, want, rev = the atomics (acquire)
if (g != myGen_) { myGen_ = g; run_.kind = None; haveServed_ = false; pendingPublish_ = false;
    if (rev || cache_.has(want-rounded)) { haveNewest_ = false; }             // served from the cache / a demand run seeks
    else { seekToTimestamp(want); haveNewest_ = haveDecoded_ = false; } }    // today's :697-699 verbatim
return rev ? reverseStep(want) : forwardStep(want);
```
`forwardStep(want)`:
```
1. if (pendingPublish_) return tryPublishPending();           // today's ring-full loop body (:747-760) as ONE attempt:
   acquireWrite ok -> unpurge / convertInto(decodedFrame_) / publish / newestPts_ (true); a gen change or
   decide()==Reseek -> drop the pending frame (true); else -> idleWork() || false.
2. next = haveNewest_ ? newestPts_ + fd : want; if (const AVFrame* f = cache_.get(next)) -> publishFrame(f, ptsOf(f))
   (a HIT: acquireWrite or (idleWork() || false); on publish newestPts_ = pts, haveNewest_ = true, ++hits). A Loop /
   OneShot clip's cache is always empty: this line is a `has` miss and costs one scan of 0 slots.
3. today's path verbatim (:695-722 minus the gen part): decide() -> maybe seek; skip_frame; decodeNextFrame (timed into
   decodeMsEma_); EOF/error/drain as today; onDecoded(): the drop rule (:740-744) + NEW guard `haveNewest_ && pts <=
   newestPts_ + 0.5 fd -> drop` (a no-op today: after any seek haveNewest_ is false and a sequential decoder is monotonic;
   needed after cache hits) + NEW: PingPong && !intraOnly_ -> cache_.store(decodedFrame_, pts, judgeStore(Behind...))
   (R-9, before convertInto) + the ring-full case sets pendingPublish_ = true instead of looping.
   Extra seek rule for step 3 after hits (R-6 Reposition): if haveDecoded_ and the decoder's next output
   (lastDecodedPts_ + fd) is above next + 0.5 fd, or more than gopFramesEst_ frames below it -> seekToTimestamp(next).
   Within a GOP below: decode forward and let the guard drop (cheaper than a seek back to the keyframe).
```
`reverseStep(want)`:
```
1. if (pendingPublish_) return tryPublishPending();           // a DEMAND run's target waiting for a slot
2. next = haveServed_ ? servedPts_ - fd : round(want / fd) * fd
   if (next < -0.5 fd) return idleWork();                    // the GL thread's wrap / reflection brings a new generation
   if (haveServed_ && servedPts_ - want > (kWriterLookAhead + 1) fd) return idleWork();   // far enough ahead: run work
3. if (const AVFrame* f = cache_.get(next)) return publishFrame(f, next);   // a HIT (acquireWrite or idleWork() || false)
4. miss: if (run_.kind != Demand || run_.target != frame(next)) startRun(Demand, next);   // aborts a prefetch (its frames stay)
   return runStep();
```
`publishFrame(src, pts)`: acquireWrite -> unpurge (P4b) -> `convertInto(src, slot)` -> `ring_.publish(slot, pts, myGen_,
++seq_)` -> servedPts_ = newestPts_ = pts, haveServed_ = haveNewest_ = true, ++gopCacheHits for a cache hit (`uploads`
stays the reader's counter; the writer counts `framesDecoded` for run decodes and `gopCacheRunDecodes`).
`idleWork()`: `if (run_.kind == None) planRun(); if (run_.kind == None) return false; return runStep();` where planRun()
in reverse = `GopCache::planPrefetch(...)` unless intraOnly_, and in forward = a Reposition run toward (the cache's
contiguous top above `next`) + fd when hits are ahead and the decoder is not there.
`runStep()` (one decode):
```
if (run_.seekPending) { seekToTimestamp(run_.seekFrom * fd); run_.seekPending = false; haveDecoded_ = false; ++runs/misses; }
skip_frame = (run_.kind != Reposition && haveDecoded_ && run_.windowLo * fd - lastDecodedPts_ > kFullDecodeFrames fd)
             ? AVDISCARD_NONREF : AVDISCARD_DEFAULT;                                       // R-7
timed decodeNextFrame(); on false: EOF -> drain (each drained frame through onRunFrame), run_.kind = None; error -> as today
onRunFrame(pts): lastDecodedPts_ = pts; haveDecoded_ = true; ++framesDecoded; ++run_.decoded; f = frame(pts)
   Demand && f == target: intraRuns_ bookkeeping (R-8); publish decodedFrame_ directly (acquireWrite or pendingPublish_ =
      true); PingPong: also store a copy per judgeStore; run_.kind = None; return true
   kind != Reposition && f <= target: cur = frame(want) fresh; pool / key / judgeStore -> store / replace / drop (counters)
   f >= target: run_.kind = None
return true
```
`startRun(Demand, next)`: target = frame(next), seekFrom = target (the keyframe at or before it), windowLo = target -
(available - 1) with `available` as in planPrefetch. Every run checks `gen_` and `wantReverse_` per step and ends on a
change (today's :749 rule). `convertInto(const AVFrame* src, int slot)` replaces `convertInto(int slot)` (:842-861: only
the source pointer changes; the IOSurface lock, the negative stride, the sws context are the same).

Equivalence of the refactor for FORWARD Loop / OneShot clips (what commit c1b must keep): the gen-change branch is :697-
699 verbatim (the `has` check is a miss on an empty cache); step 2 is a miss; step 3 is :702-722; the ring-full loop
(:747-760) is the same predicates in the same order, evaluated once per step with the outer loop's `wait(20)` between
(today's `thread_.wait(20)` at :759), the idle park checked at the loop top (today: at :687 AND inside the wait :751 -- the
same test, now once per iteration, the V2 rule kept); the EOF 20 ms wait (:719) is the outer loop's wait on a `false`.
The new guard and the retention branch are inert for Loop / OneShot.

### 3.6 Threads and invariants (checked against Pitfalls 53 / 54 / 56 / 60)

- GL thread: still never calls FFmpeg, never waits, never touches a slot outside `uploadToTexture` (56); the pick's
  direction is its own state; the held slot / fences / budget / purge protocol unchanged (60); a shown player never returns
  texture 0 (60 rule 2 unchanged); pending only before the first frame (53/56 unchanged: `open()` still publishes frame 0).
- Decode thread: the only writer of ring slots and of the cache; allocation only on this thread (slots on demand) --
  Sacred Rule 3 is the analysis thread's, the decode thread already allocates through FFmpeg.
- Two-owner idle trim (60 rule 5): unchanged; the cache clear rides on the writer's half.
- Pitfall 54's shape for a per-frame store: slot table + budget + evict-before-store + drop-outside-a-full-window +
  protected current / served + releaseAll on retire -- all present (3.3 / 3.4).
- Pitfall 35 (two live chains in a crossfade): each player has its own cache and budget share; the outgoing reversing
  chain keeps reversing during the fade; when it is no longer drawn it parks (250 ms) and is trimmed (1 s) -> its cache
  clears and its budget share returns; the incoming chain's first frame is exempt from the upload budget as before.
- Pitfall 58: `open()` configures the store but allocates nothing (the first store allocates, on the decode thread).

### 3.7 Transport cases

- Reverse Loop steady state: hits from the cache; prefetch runs keep >= prefetchAt frames ahead; each run = one seek +
  (target - keyframe) decodes for `available` stored frames; the wrap at 0 is on the trajectory (distance cur + n - f), so
  the last frames of the file are prefetched before the wrap and the wrap's generation bump is served from the cache.
- Start of a reversed clip (u7's scenes): `open()` shows frame 0; the first advanceFrame wraps (-dt -> duration - dt,
  gen 1, direction flip in the same bump); the writer: miss -> DEMAND run: seek to the keyframe <= the last frame (g250:
  8.333 s, 50 frames = ~100 ms at 1080p; g30: 30 frames = 60 ms), the last frame published directly, 49 / 29 frames
  stored; then hits while the first PREFETCH run decodes from keyframe 0 (g250: 250 frames = ~500 ms, storing up to the
  cap: at cap 2 GiB / 1 player all 250 fit -> the whole 300-frame fixture is resident after two runs = one lap of decode,
  933 MB, zero decodes afterwards). This is why the u7 window (1-6 s after the trigger) sees steady 30/s.
- PingPong: the top turn = the reflection (:395-398) flips the direction -> gen bump; the writer serves the retained
  BEHIND frames (R-9: 1080p g250 ~30 frames = 1 s of play) while the first prefetch run (~500 ms) decodes; the bottom
  turn (frame 0) = a gen bump; forward hits from the window (in PingPong reverse the passed frames stay in FUTURE with
  distance cur + f, so the frames just above 0 are resident); the Reposition run brings the decoder to the cache's top
  during the hits (keyframe 0 is right there: cheap); after the hits the sequential forward decode continues with no seek.
- Speed changes / BPMSync: the clock only; `prefetchAt` and `retainFrames` use frameMsEff = fd / |speed| (read per plan);
  a speed the decoder cannot sustain -> late frames (hold), never a stall (R-12). Speed 0: no wants, nothing happens.
- Seeks / scrubbing: each seek is a gen bump; the wanted frame resident -> a hit (no FFmpeg seek); else a DEMAND run
  (one keyframe seek + catch-up, today's cost) which a newer generation aborts. The cache is kept (R-10).
- A manual reverse -> forward flip mid-file (Loop): forward hits from the BEHIND pool (16 frames = 533 ms at 30 fps)
  while a Reposition run seeks to the keyframe <= the pool's top and catches up (<= one GOP: ~500 ms at 1080p g250 --
  covered; ~1.7 s at 4K g250 -- a hold of up to ~1.2 s once; today's flip holds for the remainder of the running
  catch-up, up to a GOP, so 4K is no worse than today's worst case and 1080p is better). Forward -> reverse: a DEMAND run
  (<= one GOP of catch-up: today's cost), then smooth.
- Load / clip switch mid-reverse: the player is retired (`closeMediaForClip`), its thread exits and frees the cache in
  `freeFfmpeg`; the new player starts cold (frame 0 from open()). A staged load (Pitfall 58) opens players off-thread;
  the store is configured there, allocation happens on the decode thread later.
- Off-screen deck (rule 15): the thread parks at 250 ms; the trim at 1 s clears the cache; on return a DEMAND run rebuilds
  it (w4's bounded hold, <= one GOP of decode -- today's return cost).
- Context loss: nothing of the cache is GL; the held-slot re-upload (60) is unchanged.

### 3.8 Counters / REST

VideoStats: `std::atomic<int64_t> gopCacheHits, gopCacheMisses, gopCacheRuns, gopCacheRunDecodes, gopCacheEvictions,
gopCacheDrops; std::atomic<int> gopCacheOverBudget;` -- bytes / frames / active / cap come from `GopCache::sharedBudget()`
(bytes, active) and a per-store frame count summed into `VideoStats::gopCacheFrames` (the store adds / subtracts on
store / free). /api/state (both servers): `video_gopcache_bytes`, `_frames`, `_active`, `_cap_bytes`, `_hits`, `_misses`,
`_runs`, `_run_decodes`, `_evictions`, `_drops`, `_over_budget`. Test mode already has `phys_footprint_mb`
(TestServer.cpp:729-734). No new endpoint; no env hook.

## 4. GATES (RED first; each drives real code)

### 4.1 ctests (all serial in `ctest --test-dir build-lane -j1`; TSan build of the pure + ring tests as the vupload lane did)

- `tests/test_gop_cache.cpp` (pure, RED = does not compile on main -> written with the header in c2):
  (1) Budget: cap = kBudgetBytes / active; tryTake refuses over the cap and over the total; floors win (a cache under
  kMinFrames x frameBytes takes even over the total; over_budget observable); give never below 0; activate / deactivate
  round-trip. (2) distance: every closed form vs a brute-force trajectory walk for n in {1, 2, 7, 30}, all modes, both
  directions, every (cur, f) -- the walk is the SeqVram::step semantics without in/out. (3) poolOf: Loop reverse: the 16
  frames above cur are Behind, farther above -> None; PingPong: above cur -> Future with distance cur + f; forward
  PingPong: below cur -> Behind; forward Loop -> None. (4) judgeStore: free slot -> Store; full Future pool -> Replace the
  largest key when the new key is smaller, Drop when larger; never the protected frames; a Behind frame never takes a
  Future slot and vice versa; growth stops at capSlots. (5) planPrefetch: none while unservedAhead >= prefetchAt; a run
  whose target is the first non-resident frame below the resident run, seekFrom = target - (available - 1) >= 0; the Loop
  wrap targets n-1 after frame 0; OneShot plans nothing below 0; PingPong plans nothing at the reflection; intraOnly ->
  none. (6) the amortization property: simulate a reverse Loop over G = 250 with capSlots in {16, 32, 64, 128, 300}
  (decode from the keyframe, greedy store, serve down, prefetch at slots/2) and assert decodes per shown frame <=
  G / (2 * floor(slots/2)) + 1.0 and, at slots >= G + 1, exactly 1.0 after the first lap (then 0 for a resident file).
  (7) retainFrames / prefetchAt arithmetic at the four (res, GOP) points of this plan.
- `tests/test_video_ring.cpp` (+8 cases; RED on main: `pick` has no 5th parameter -> the new cases fail to compile until
  c1): reverse pick with held W (Reading) and Ready W-1, W-2 -- at clock W - 0.3 fd picks nothing (readyAhead 2), at
  W - 0.6 fd picks W-1 and keeps W-2; reverse skipped = Ready frames ABOVE the chosen are freed; the mirrored ahead-drop
  (a frame 4 fd below the clock is freed, one 3 fd below is kept); reverse peek == the reverse pick's choice with no state
  change; a two-thread race (VU10's shape) in reverse: a publish between peek and pick only ever yields a frame at or
  below the peeked one, never above, never none; `reverse = false` cases = the existing cases unchanged (they stay).
- `tests/test_gop_cache_store.cpp` (the REAL VideoPlayer + FFmpeg, no GL, no decode thread; `VideoPlayerTestAccess` extended
  with `decodeStep`, `ring state`, `cache view`, `setBudget`, `setReverse/gen` seams; fixture
  `video_h264_gop10_64x64.mp4` = `ffmpeg -f lavfi -i testsrc2=s=64x64:r=30:d=2 -c:v libx264 -g 10 -bf 2 -pix_fmt yuv420p
  -preset veryfast -crf 28 -threads 1` -> 60 frames, GOP 10, ~20 KB, committed; the test asserts its keyframes with
  the decoder (AV_FRAME_FLAG_KEY) once). RED on main: does not compile. Cases: (1) reverse serve order and identity:
  gen 1, wantReverse, want = frame 45 (1.5 s): stepping the writer publishes 45, 44, 43 (the reader releases between
  picks) with `video_seeks` +1 for the DEMAND run and +1 for the prefetch, and every published slot's bytes equal the
  same frame decoded forward from the file start and converted by a test-side sws with the player's dst format (RGBA
  on the malloc path forced via `forcePath_` so no IOSurface is needed) -- byte-identical; frames 44 and 45 differ from
  each other (a sanity tooth). (2) the budget: a local Budget with kBudgetBytes replaced by a small cap through the test
  seam (cap = 6 frames): resident <= 6, evictions counted, the served frame never evicted, decodes per served frame over
  30 served frames <= 10 / (2 * 3) + 1. (3) intra-only detection: the rawrgba / HAP fixtures (every frame a keyframe):
  after two demand runs `intraOnly_` is true and no prefetch is planned. (4) a direction change is a discontinuity: a
  real player after open(); `setReverse(true); advanceFrame(dt)` -> `gen_` advanced by exactly 1 (the wrap and the flip
  share the bump); a plain forward advanceFrame does not advance it; `setReverse(false); advanceFrame` -> +1. (5) the idle
  trim clears the cache: after frames are resident, `trimRequested_` set + one outer-loop step (a seam that runs the
  loop's trim branch) -> resident 0, bytes 0, active 0. (6) PingPong forward retention: PingPong mode, forward stepping
  from frame 0 for 40 frames -> the BEHIND pool holds the last min(retainFrames, 40) published frames; Loop mode -> 0
  stored. (7) forward Loop untouched: a Loop clip stepped forward for 30 frames stores nothing, allocates nothing
  (bytes 0), and publishes 30 frames in order (the refactor's tooth).
- `tests/test_video_player_open.cpp`, `test_video_player_gl.cpp`, `test_video_upload_budget.cpp`: unchanged and GREEN.
- Teeth (mutated copies, not committed): a forward-only pick (reverse ignored) fails ring cases 1-2; the guard removed
  fails store case 1 (an older frame published after hits -- built into case 1 by a forward flip after 3 reverse serves);
  NONREF skipping inside the window fails case 1's identity for a B-frame index; the cache not cleared on trim fails 5.

### 4.2 Live rows -- RED on main, predicted numbers

All through `.harmony/probe-vupload.sh` (test mode, the quiet lock, open -g, no Output window, no synthetic input); the
fixtures are the existing ones (g30_1080, a1080_g250, a4k_g250, warm.png). >= 5 launches per arm for any verdict, arms
INTERLEAVED via `probe-vupload-ab.sh` (A = main's app copy, B = the lane app).

| row / scene | bar | main (RED, predicted / measured) | lane (predicted) |
|---|---|---|---|
| u7 g30_1080_reverse | (a) uploads/s >= 28.5; (b) late <= 10 per 5 s; (c) hold_no_texture 0; (d) the mid-window capture's code within the playhead bracket (reverse: `in_bracket(c, pa, pb)`, the arguments swapped -- the helper's wrap branch handles a wrap at 0) | 11.3/s, late 212 (measured); (d) likely FAIL (a stale frame) | 29.5-30.4/s, late 0-3, (d) PASS |
| u7 a1080_g250_reverse | same | 4.5/s, late 456.5 | 29.5-30.4/s, late 0-3 |
| u7 g30_1080_pingpong, a1080_g250_pingpong (existing, forward-only window) | INFO in one run; A/B: B >= 0.9 A, late B <= A | 30.2-30.4/s, late 0 | 30.2-30.4/s, late 0 (forward unchanged) |
| u7 g30_1080_pingpong_turn, a1080_g250_pingpong_turn (NEW: `ip=0.7`, trig, then ONE retrigger to land at the in-point -- probe-video's "prime" rule, :29-30 of its docstring; the window covers ~2 s forward, the turn, ~3 s reverse) | (a)-(d) as the reverse scenes | (2 x 30 + 3 x 11.3) / 5 = 18.8/s, late ~125; g250: 14.7/s, late ~275 | 29.5-30.4/s, late 0-4 (the turn: <= 2 wasted ring frames) |
| u8_reverse_column_1080x4 (NEW; 4 layers x a1080_g250 reverse, trigger_column, 1 s, s0, 5 s window, s1) | (a) pooled uploads/s >= 114 (4 x 28.5); (b) late <= 40; (c) `video_gopcache_bytes` <= kBudgetBytes + 4 x kMinFrames x frameBytes and `_over_budget` == 0; (d) hold 0, pending 0; INFO: phys_footprint delta, median fps, per-player share (bytes / active), decodes per upload | counters absent -> FAIL as absent (VU16's shape); uploads ~18/s pooled | 118-121/s; late 0-8; bytes ~2.0-2.1 GB (4 x ~510 MB: cap 536 MB each); decodes/upload ~1.3-2; fps INFO (the fill burst: 4 x 2 FFmpeg threads for ~3 s) |
| u9_reverse_cache_drop (NEW; 1 x a1080_g250 reverse; 3 s; s0; switch to deck 1 (warm.png); trimWaitS 2.5 s; s1; switch back; settle; cap; 3 s; s2) | (a) at s0 bytes >= 25 MB and <= kBudgetBytes, frames >= 8; (b) at s1 bytes == 0, frames == 0, active == 0 (the idle trim); (c) after the return the top layer's code within its (swapped) bracket, hold 0; (d) late over the 3 s after settle <= 10; INFO: phys_footprint s0 -> s1 drop (expected ~900 MB; INFERRED that av_free returns the pages) | absent -> FAIL | (a) 933 MB / 300 frames (the whole fixture: cap 2 GiB / 1); (b) 0 / 0 / 0; (c) PASS; (d) 0-3 |
| u10_reverse_4k (NEW, INFO only) | prints uploads/s, late, bytes, frames, decodes/upload, fps for 1 x a4k_g250 reverse | ~1.2/s (INFERRED: 125 x 6.8 ms per cycle) | ~29-30/s at 173 frames resident (2 GiB / 12.44 MB), decodes/upload ~1.3; no bar (the 4K budget arithmetic is the report's) |

Bars that would equal the rig's drift are INFO: median fps in u8 / u10 (drift 5-8 fps/h); uploads/s of a 30 fps source
and late counts are drift-robust (render-rate independent; a slower render rate only lowers late counts). Every threshold
is new; none existing is re-thresholded (`u7UploadsRatioMin` 0.9 stays; the A/B rule is applied to all six u7 scenes).

### 4.3 Forward unchanged (the must-stay-GREEN battery, on the final app)

`probe-video.sh` all rows (102 / 0 at the s-rta-0929 close; w6b (a) is the known load-sensitive flake: report its 5-run
distribution on both apps, as VU13 did); `probe-video-w10-all.sh <pre-lane w10 refs>` GREEN (30 / 30 captures at max |diff| 0
on blit / client / malloc x 5 formats); `probe-vupload.sh` u2 / u4a / u4b / u6 unchanged; u7's two forward-only ping-pong
scenes B >= 0.9 A, late B <= A (interleaved, 5 x 2); w1c / w2c via the interleaved driver (5 x 2) with B's launch medians
within main's spread (INFO: no new bar); `probe-crossfade`, `probe-media-open`, `probe-async-load`, `probe-seq-vram` as at
the last close; Tier-1 test_effects / test_audio_reactivity / test_time_sweep / test_performance / test_sources (the
reaction_diffusion pre-existing item excluded); ctest full serial; TSan of test_video_ring + test_gop_cache.
Memory of forward play: u4b's phys_footprint numbers must not move (a Loop clip allocates no cache: `video_gopcache_bytes`
== 0 during every forward row -- add that assertion to w1c's DATA (INFO print) and to u4b (PASS bytes == 0)).

### 4.4 The A/B protocol (exact)
`LANE=gopcache LOCK_LIB=<copy of .harmony/.reports/s-rta-0929/wf/lock.sh with SPL fixed> VIDEO_FIXTURES=<shared dir>
.harmony/probe-vupload-ab.sh <main app copy> <lane app> 5 .harmony/probe-vupload.sh u7_reverse_pingpong,u8_reverse_column_1080x4 <out>`
-- one live app at a time, the quiet lock (no compiler before locking), a launch that saw a compiler is tainted and
re-run, UserNotificationCenter windows 0, Output-named windows 0 after every launch. The main app is a COPY made before
the first lane build (`apps/main-d88d2ea.app`, sha logged).

## 5. MUST-NOT-CHANGE

- The upload path: `uploadSlot`, `blitSlot`, `clientUploadSurface`, `ensureTexture`, `fallBack`, `pollFences`,
  `releaseGL`, `trimIfIdle`, `purgeFreeSlots`, `unpurge`, `createSurfaces`, the IOSurface / BGRA / negative-stride
  conversion (`convertInto`'s body: only its source pointer becomes a parameter), `VideoUploadBudget.h`, `Retire`,
  `ShownState`, `judge`, `firstFrameFailed`, the slot state protocol, kSlots 3, kWriterLookAhead, the reseek geometry.
- `open()`: frame 0 into slot 0 + the thumbnail (:229-248); `thread_count` 2; the sws context; the "Opened" log line's
  existing fields (a `cacheFrame=` field is appended).
- Forward Loop / OneShot behaviour: no allocation, no copy, the same seeks / drops / publishes (4.1 case 7, 4.3).
- The transport math (`advanceTransport`) except the direction-change discontinuity; `seekTo`; the playhead store.
- Renderer.cpp's syncMedia call sites, `installVideoPlayer`, `drainRetiredMedia`, `scanVideoIdle`; the audio thread;
  the analysis thread; CompositorEngine; SeqVram / ImageSequence.
- Every existing threshold in probe-video.json / probe-vupload.json; the existing u7 scene definitions (the two forward-
  only ping-pong scenes stay as they are, the reverse scenes gain checks, never lose one).

## 6. RISKS (strongest counterargument first)

1. **"The step refactor of the forward path (R-15) is the real risk; the cache could have been bolted on with a
   thread-driven test."** True that c1b touches the forward path's control flow. Why it still wins: without it the reverse
   machinery is untestable deterministically (the ring-full wait is exactly where runs are stepped), and a thread-driven
   test would be a flake. Mitigation: c1b lands ALONE with the equivalence list (3.5) and the whole forward battery
   (4.3) before any cache line exists; a forward regression bisects to one commit. What to watch: w1c (b) fps
   (118.5 bar, thin margin per vupload.md), late 0 on every forward row, `gl_video_decode_calls` 0.
2. **The ping-pong turn may show 1-2 late frames**: the flip's gen bump wastes the 2 queued ring frames, and the first
   reverse frame after the bump must be published, picked and blitted within a frame or two. Predicted late <= 2 per turn;
   the bar (<= 10 per 5 s) tolerates it; if a turn costs more, the fix is to keep the two Ready frames when they are ALSO
   the new direction's future (only true in PingPong at the reflection: they are the frames just shown) -- a rule in the
   stale drop, not a redesign. Report the per-turn number.
3. **phys_footprint may not fall when the cache clears** (macOS malloc keeping freed large blocks; INFERRED that
   av_free's 3 MB blocks are returned): the counter is the gate (u9 (b)); the footprint is INFO, as SeqVram's rss was.
4. **CPU burst during the fill**: a fresh reverse episode decodes a whole lap at full tilt (g250 1080p: ~300 decodes in
   ~0.6-0.8 s on 2-3 threads per player; u8: four at once). The render thread is at DEFAULT QoS (VU15) and shares the
   P-cores. u8 prints median fps (INFO). If the burst dents fps, the lever is a per-step yield in `runStep` for PREFETCH
   runs (`thread_.wait(1)` every k decodes) -- named, off by default.
5. **4K long-GOP flips hold up to ~1.2 s** (3.7) and four 4K reversers within 2 GiB (43 frames each) run at ~20 fps
   reverse (6.5 decodes x 6.8 ms per frame) -- graceful, not smooth. Stated, not hidden; u10 is INFO; VideoToolbox is the
   real fix for 4K CPU cost (filed).
6. **The Reposition seek heuristic** (seek iff the decoder passed `next` or is more than a GOP behind) can mis-guess on a
   file with wildly varying GOPs (an estimate from the index's max interval); the cost of a wrong guess is a longer
   catch-up, never a wrong frame.
7. **VFR / odd time bases**: the pts <-> frame index rounding (R-13) could make `get(next)` miss a resident frame by one
   -> an unnecessary demand run. The fixtures are CFR; a VFR file is not in any gate (open question Q3).
8. **Budget fairness is first-come**: a new reverser joining gets only what is left until the others trim (lazily, at
   their next store decision -- they are stepping runs, so within a few decodes); it holds its floor meanwhile.
9. **Memory of PingPong forward play is new** (R-9: up to retainFrames copies -- 1080p g250 ~93 MB; 4K ~1.27 GB within
   the cap): the price of a smooth top turn. If Harmony prefers a hold at the turn over the memory, `kRetainSafety = 0`
   disables retention (the turn then costs one demand run of <= one GOP: 500 ms at 1080p).

## 7. COMMIT SEQUENCE (each builds, passes `ctest -j1`, keeps every existing probe GREEN; conventional titles as
`feat(s-rta-0929b gopcache): cN -- ...`)

- c0 `test(... c0 -- counters, u7 DATA fields + turn scenes + reverse bars + capture, rows u8 / u9 / u10 (RED harness))`:
  VideoStats + both state blocks (zeros), probe-vupload.{py,json}, probe-vupload-ab.py (six scenes); run u7 x 5 (A = B =
  main) for the D1 diagnosis; record the RED lines verbatim (main FAILs u7 reverse (a)(b)(d), u8 / u9 absent).
- c1 `feat(... c1 -- VideoRing pick / peek take the direction (mirrored); a direction change is a discontinuity)`:
  VideoRing.h, the two call sites, advanceTransport / advanceFrame / advanceClock, test_video_ring (+8), store test case 4
  (the gen bump) -- forward battery GREEN; u7 x 5 interleaved: B >= 0.9 A on all six scenes (expected: reverse scenes
  improve to ~2 uploads per seek; the diagnosis branch B early exit is decided HERE if it fired).
- c1b `refactor(... c1b -- decodeLoop = an outer loop around decodeStep(); the ring-full wait is one attempt per step)`:
  no cache code; store test case 7 (forward Loop untouched) + `VideoPlayerTestAccess::decodeStep`; the FULL forward battery
  (4.3) on this app, interleaved w1c / w2c / u7 vs main.
- c2 `feat(... c2 -- GopCache.h (pure policy + Budget) and GopCacheStore (AVFrame slots), ctests)`: no wiring; test_gop_cache
  GREEN; the fixture committed; test_gop_cache_store cases 2-3 compile against the store directly.
- c3 `feat(... c3 -- reverse play served from the cache: demand / prefetch runs stepped in the ring-full wait, intra-only
  detection, the idle trim clears it, freeFfmpeg frees it, counters)`: u7 reverse scenes GREEN (interleaved 5 x 2), u8 / u9
  / u10, store test cases 1-3 / 5; TSan of the pure tests.
- c4 `feat(... c4 -- PingPong: forward retention for the top turn, forward hits + the Reposition run for the bottom turn
  and manual flips)`: the turn scenes GREEN; store test case 6; u7 x 5 interleaved on all six scenes; the full 4.3 battery
  on the final app.
- c5 `docs(... c5 -- rendering.md, pitfalls NN, testing-eyes.md, CLAUDE.md index, APP-INVENTORY, the lane report)`.

## 8. DOCS TEXT

**docs/claude/rendering.md** -- line 75, replace "-- reverse or ping-pong on a long-GOP file therefore shows correct
frames at the rate one seek + catch-up allows (a GOP cache is filed)" with "-- reverse and ping-pong are served from the
decode thread's GOP cache (the paragraph below)"; line 77, replace "; the GOP cache for reverse / ping-pong on long-GOP
files is its own lane (plan-vupload R-12)" with "; reverse / ping-pong: the GOP cache paragraph below". New paragraph after
line 77:

"**Reverse and ping-pong video (s-rta-0929b gopcache)**: a reversing player is served from a decode-thread GOP cache
(`src/media/GopCache.h`: the pure policy and the shared `Budget`; `src/media/GopCacheStore`: a slot table of AVFrame copies
in the decoder's native pixel format -- 3.11 MB per 1080p yuv420p frame, never the ring's BGRA -- that `convertInto` turns
into a ring slot exactly as it turns a freshly decoded frame, so the uploaded pixels equal forward play's). The decode
thread runs a step machine (`decodeStep`: one unit of progress, never blocking; the ring-full wait steps a RUN instead of
sleeping): a DEMAND run seeks to the keyframe at or before the wanted frame, stores the frames the window wants and
publishes the target straight from the decoder (today's catch-up latency); a PREFETCH run starts when fewer than
`prefetchAt` unserved frames are resident ahead of the clock, seeks one keyframe below the whole next window and stores it
(one seek per window instead of one per shown frame); a REPOSITION run brings the decoder behind the cache's top while
forward hits are served after a flip. The window is what the budget allows: `GopCache::kBudgetBytes` (2 GiB, every cache
together; per-player cap = budget / active caches; floors of 8 frames win, `video_gopcache_over_budget`), so a GOP that
fits is cached whole (one decode per frame, then none for a resident file) and one that does not is a sliding window
(decodes per shown frame = GOP / (2 x half the slots) + 1/2). Eviction is farthest-next-use along the clip's trajectory
(closed-form; Loop wraps, PingPong bounces, OneShot ends), never LRU; the wanted and the served frame are protected; a
frame outside a full window is dropped, not stored; the 16 frames just passed stay (a manual flip's cover); non-reference
frames are skipped only below the window minus 10 frames (a skipped frame would be a hole = a seek). The ring's
`pick`/`peek` take the direction and mirror their comparisons in reverse (the candidates are the frames at or above clock -
half a frame, the chosen is the lowest, the passed-over frames are those above it) -- the forward pick freed every frame
below the chosen one, which in reverse was the writer's next frame; a direction change (the reverse flag, a ping-pong
reflection) is a request-generation bump like a Loop wrap, so the ring never mixes directions. PingPong forward play
retains the frames it publishes (`retainFrames` = GOP x decode ms / frame ms x 2, floor 16, from the container index and a
measured decode-time EMA) so the top turn is served while the first run decodes; the bottom turn is served from the
window. Intra-only files (HAP, ProRes, MJPEG) skip the cache: one seek + one decode + a direct publish per frame. The
cache is cleared by the idle trim (the R-14 pair, 1 s off screen) and freed with the FFmpeg contexts; a seek, wrap or flip
keeps it. Loop / OneShot forward play allocates nothing. Measured (u7, 1080p, 5 x 2 interleaved): g30 reverse 11.3 ->
~30 uploads/s (late 212 -> ~0), g250 reverse 4.5 -> ~30 (late 456 -> ~0) [the builder fills the numbers]; four 1080p g250
reversers ~2.0 GB resident, one 4K g250 reverser ~2.1 GB. `/api/state` (both servers): `video_gopcache_bytes` / `_frames` /
`_active` / `_cap_bytes` / `_hits` / `_misses` / `_runs` / `_run_decodes` / `_evictions` / `_drops` / `_over_budget`. Levers:
`GopCache::kBudgetBytes`, `kMinFrames`, `kBehindFrames`, `kMinRetainFrames`, `kRetainSafety` (0 = no retention),
`kFullDecodeFrames`, `kDefaultGopFrames`, `kDecodeMsSeedPerMpix`. Guards: `tests/test_gop_cache.cpp`,
`tests/test_gop_cache_store.cpp`, `tests/test_video_ring.cpp` (reverse pick / peek); live: `.harmony/probe-vupload.sh` u7
(six scenes) / u8 / u9 / u10, the interleaved `probe-vupload-ab.sh`."

**docs/claude/pitfalls.md** NN (Harmony assigns the number; after 60):
"NN. **Reverse video is served from a decode-thread GOP cache and the ring's pick is direction-aware -- never re-seek per
shown frame, never publish reverse frames into a forward pick, never cache converted pixels**: `VideoPlayer` used to serve
reverse play by seeking to the keyframe before the clock and decoding upward, publishing frames above the clock; the
forward-only `VideoRing::pick` then freed the frame below the chosen one -- the next frame needed -- so every seek + catch-up
bought about one shown frame (1080p: GOP 30 11.3 uploads/s, GOP 250 4.5/s against a 30 fps source; ping-pong turned at
the same rate). Rules: (1) reverse frames come from the cache (`GopCacheStore`, AVFrame copies in the decoder's native
format) through the SAME `convertInto` as a decoded frame -- never a second write path into the ring slots, never BGRA
copies (2.7 x the memory); (2) `pick` / `peek` get the direction (`reverseNow_`) and mirror every comparison; a direction
change is a discontinuity (generation bump) -- a ring must never hold both directions' look-ahead; (3) a run stores what
the window wants and drops the rest; the wanted and served frames are never evicted; farthest-next-use, never LRU
(Pitfall 54); non-reference frames are skipped only below the window minus `kFullDecodeFrames` (a skipped frame is a hole
= a seek); (4) the store lives on the decode thread and dies with the FFmpeg contexts; the GL thread never sees it; the
idle trim (the R-14 pair) clears it; (5) one shared budget for every cache (`GopCache::kBudgetBytes`), per-player cap =
budget / active, floors win -- a GOP that does not fit is a sliding window, never a stall or a black frame (the reader
holds, `video_late_frames`); (6) forward Loop / OneShot play allocates nothing -- only PingPong retains frames for its
turn; (7) `decodeLoop` is an outer wait loop around `decodeStep()` (one non-blocking unit of progress): a ring-full wait
steps a run; the ctests drive the real writer without a thread. (s-rta-0929b gopcache.) Guards: `tests/test_gop_cache.cpp`,
`tests/test_gop_cache_store.cpp`, `tests/test_video_ring.cpp`; live: `.harmony/probe-vupload.sh` u7 (six scenes) / u8 / u9 /
u10, the interleaved `probe-vupload-ab.sh`."

**docs/claude/testing-eyes.md** -- the "GL context cycle, test mode only" paragraph (:15) gains: "`.harmony/probe-vupload.sh`
u7 runs six scenes (g30 / g250 x reverse / ping-pong (forward window) / ping-pong turn at in-point 0.7) with absolute bars
on the reverse scenes (uploads/s >= 28.5, late <= 10, the mid-window capture's code within its playhead bracket); u8 (a
column of four 1080p reversers: pooled uploads/s, `video_gopcache_bytes` under `GopCache::kBudgetBytes`), u9 (the idle
trim clears a reversing player's cache: bytes 0, phys_footprint INFO), u10 (4K reverse, INFO). Cross-app verdicts through
`probe-vupload-ab.sh` (>= 5 interleaved launches per arm)."

**CLAUDE.md** -- the Common Pitfalls Index gains one line after 60 (~190 B; 24,522 + 190 = 24,712 B < 25,000 -- no payment
needed; if the builder's `wc -c` exceeds 25,000, pay by collapsing index lines 9-12 into one: "9-12. Fractal zoom, center
/ location / dive, 2D power range, 3D camera range -- before touching any fractal zoom, dive, center or camera control."):
"NN. Reverse / ping-pong video is served from the decode thread's GOP cache and the ring's pick is direction-aware --
before touching `decodeLoop`'s reverse path, `VideoRing::pick`, or a direction change."

**.harmony/APP-INVENTORY.md** -- the "967 unit tests / 107 Catch2 targets" line (:31): re-count after the lane (+2 targets;
the builder runs `ctest -N`); the vupload note (:226-229) gains a sibling: "s-rta-0929b gopcache: `/api/state` +11
`video_gopcache_*` fields; ctests `test_gop_cache`, `test_gop_cache_store` (+8 in `test_video_ring`); probe-vupload u7 six
scenes + u8 / u9 / u10."

## 9. OPEN QUESTIONS FOR HARMONY

- Q1 `GopCache::kBudgetBytes` 2 GiB (ASSUMED): accept, or scale by `hw.memsize` (1/16 of RAM: 2 GiB on 32 GiB, 1 GiB on
  16 GiB)? The u8 / u10 arithmetic (four 1080p reversers whole-window; one 4K reverser at 173 frames) assumes 2 GiB.
- Q2 R-9 PingPong forward retention (memory during FORWARD play of a PingPong clip: ~93 MB at 1080p g250, up to ~1.27 GB
  at 4K within the cap) vs a hold of <= one GOP of decode at every top turn: accept as planned (`kRetainSafety` 2.0)?
- Q3 No VFR fixture exists; R-13's tolerance is INFERRED adequate. Add a VFR row later (a separate item) or accept.
- Q4 The diagnosis branch B early exit (section 1): accept the pre-registered rule as written?
- Q5 D2 (ApiServer / TestServer counter lines) and the tests/CMakeLists.txt + fixture additions are outside the literal
  fence: accept as the vupload precedent?

## COMPACT

Reverse is slow because the writer seeks and decodes UPWARD to the clock every cycle (VideoPlayer.cpp:693-760) and the
forward-only pick frees the frame below the chosen one (VideoRing.h:113-121): ~1 shown frame per seek + catch-up (g30
11.3/s, g250 4.5/s; decode 2.0 ms/frame at 1080p, 6.8 at 4K, NONREF -13 %). Fix: (1) `pick`/`peek` take the direction
and mirror (D1, proof above); (2) a direction change bumps the generation; (3) a decode-thread GOP cache of native-format
AVFrame copies fed by DEMAND / PREFETCH / REPOSITION runs stepped in the ring-full wait, served through the unchanged
`convertInto` -> IOSurface -> blit path; window = a shared 2 GiB budget (cap = budget / active, floors 8 win), farthest-
next-use eviction, drop outside a full window, NONREF skip only below the window; (4) PingPong forward retention for the
top turn; intra-only files bypass. Gates: ctests (pure policy incl. the amortization property; ring reverse cases; the REAL
player served backwards byte-identical to a forward decode on a committed GOP-10 fixture; flip = gen bump; trim clears;
Loop forward allocates nothing), live u7 (six scenes: reverse >= 28.5/s, late <= 10, capture in bracket; main 11.3 / 4.5),
u8 column (>= 114/s pooled, bytes <= 2 GiB), u9 trim (bytes 0), u10 4K INFO; forward: probe-video 102/0, w10-all identity,
u2/u4a/u4b/u6, interleaved 5 x 2 everywhere a verdict is taken. Commits c0 (RED harness + diagnosis) c1 (pick + flip) c1b
(step refactor alone, full battery) c2 (pure + store) c3 (reverse) c4 (ping-pong) c5 (docs). Strongest risk: the c1b
refactor of the forward path -- isolated in its own commit with the whole forward battery.

STATUS: PLAN COMPLETE -- ready for blind attackers; five open questions (section 9), none blocking the RED harness (c0).

## HARMONY ADOPTION (s-rta-0929b, 2026-09-30) — OVERRIDES the plan body where they differ
Plan authored on Fable (tier verified in the workflow result). Attacked by 3 blind seats: attack-gopcache-decode.md (5 MUST),
attack-gopcache-vj.md (5 MUST), attack-gopcache-gates.md (6 MUST). Every MUST below is ADOPTED; SHOULDs: the builder adopts each
unless it conflicts with a ruling here or the fence, and records "adopted / declined + reason" per SHOULD in the lane report.

GC1 (decode M1) The pending publish never shares decodedFrame_ with idle work: move it into a dedicated pendingFrame_
    (av_frame_move_ref, pts kept beside it). ctest: fill the ring, decode a Demand target, run 3 idle decode steps, free a slot ->
    published bytes == the target's.
GC2 (decode M2 + vj M1) reverseStep's lag / ahead test is re-derived with the correct sign: writer LAGS when servedPts_ > want
    (frames still above the clock) -> jump next to the clock's frame (skip, count `dropped`), never idle; writer AHEAD when
    servedPts_ < want - 3 fd -> idleWork. next = min(servedPts_ - fd, clock frame). ctests: decodeStep reverse at speed 2x and 4x
    (playhead never frozen, late <= 3) and after a 300 ms stall; u7 gains a speed-2 reverse scene.
GC3 (decode M3 + gates M4) Every run has a negative result: a run that passes its target without output publishes the nearest
    decoded frame <= target (else the first >= target) and marks the target a hole (small set, cleared on open); end-of-stream uses
    the stream's first pts (not 0); at most one consecutive Demand run per target (no seek storm). Frames flagged corrupt
    (AV_FRAME_FLAG_CORRUPT / decode_error_flags) are never stored. Fixtures: open-GOP x264 (open-gop=1, keyint 10, bframes 2),
    a non-zero start_time file, and a VFR file (Q3 closed here, not deferred); identity vs a forward decode from file start
    for every frame on each.
GC4 (decode M4) Reverse correctness is gated beyond rates: GL-thread counter `video_reverse_nonmonotonic` (shown pts > last
    shown pts at the same generation while reversing) == 0 in every reverse scene; >= 20 captures of a burnt-in frame-number
    fixture per reverse scene read monotone decreasing; exactly one direction change per ping-pong turn.
GC5 (decode M5) intraOnly_ is derived from the container / codec (all index entries keyframes, or HAP / ProRes / MJPEG / DNxHD /
    rawvideo), never learned from run history.
GC6 (vj M2 + gates M1) Cap enforcement is per step, not per store decision: every writer step checks bytes > capBytes() ->
    evictOne (farthest next use). ctest: player A resident alone, B joins -> A shrinks to cap within N steps. u8 asserts the
    per-player minimum uploads/s >= 28.5 as well as the pooled sum.
GC7 (Q1 + vj M3 + decode S2) Budget = min(2 GiB, hw.memsize / 16) (2 GiB on this rig), counting 10-bit / 4:2:2 frame sizes by
    their real bytes; a DISPATCH_SOURCE_TYPE_MEMORYPRESSURE hook on the message thread lowers the cap atomically (warn: halve;
    critical: floors only), decode threads evict lazily via GC6. Gate: u8 with a TEST_SERVER / test-mode cap of 256 MB stays
    >= 20 uploads/s per player, late <= 40, bytes <= cap + floors.
GC8 (vj M4) Forward -> reverse flip on a Loop / OneShot clip: while a clip plays FORWARD it keeps the last kBehindFrames (16) of
    published frames (counted in the budget) so a flip serves instantly while the Demand run decodes. Gate: u7 scene
    flip_reverse (trigger, 2 s forward, set reverse via REST, 3 s): max inter-upload gap <= 150 ms at 1080p g30; g250 gap
    recorded (bar: lane < main, interleaved).
GC9 (vj M5) Deck switch-back: PREFETCH bursts are staggered across players (one PREFETCH run at a time through the shared
    budget struct; DEMAND runs unrestricted). Gate: the returned layer's max inter-upload gap after a deck switch back to a
    column of four reversers, lane <= main (interleaved x5), absolute value recorded.
GC10 (gates M2) Leak gates see real memory: test_gop_cache_store adds a phys_footprint (task_info) case — fill 100 frames,
    clear(), open / close a player x20 -> footprint delta < 15 % of the peak; u9 asserts phys_footprint drops >= 50 % of the
    counted bytes on trim, x5.
GC11 (gates M3) Reverse identity on the REAL upload paths: a reverse w10 row (the same frame index captured in reverse vs
    forward, max |diff| 0 on blit / client / malloc); u7 (d) bracket uses CT <= 1 for reverse scenes; run it on main in c0 and
    record main's actual result (no "likely FAIL" guesses).
GC12 (gates M5 + decode S7) The c1b forward refactor is held to a tight bar: forward scenes B median >= 0.985 A and late B == 0 per
    launch; plus a golden-trace ctest — the REAL decodeStep against a fixed-clock schedule must publish exactly the pts
    sequence and seek / decode counters the pre-refactor writer produced (trace recorded on main via a test seam in c0).
GC13 (gates M6) Every absolute live bar (u7 28.5/s + late 10, u8, u9, u10) is judged on the MEDIAN of >= 5 interleaved launches
    per arm; per-launch minimum is INFO. u8 / u9 / u10 run x5 like u7.
Q2 accepted (PingPong forward retention, kRetainSafety 2.0) WITH an absolute per-player cap (decode S4): <= 1/4 of the budget.
Q4 accepted as written; branch-B gap [5, 10] (decode N2): treat as branch A.
Q5 accepted (vupload precedent): ApiServer / TestServer counter lines, tests/CMakeLists.txt, fixtures are in the fence.
Fence (Harmony constraint): src/media/VideoPlayer.*, src/media/VideoRing.h, new src/media/GopCache* files, the counter lines in
    ApiServer / TestServer, tests/, fixtures, .harmony/probe-vupload.* and probe-video-w10-all.sh, docs/claude/*.md.
Commit sequence stays c0 (RED harness + diagnosis + golden trace) -> c1 -> c1b (forward battery alone) -> c2 -> c3 -> c4 -> c5;
    GC rulings land in the commit that owns the code they touch.
