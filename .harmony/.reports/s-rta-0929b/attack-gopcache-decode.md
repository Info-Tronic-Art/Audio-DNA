# attack-gopcache-decode -- FFmpeg / real-time-thread seat (blind). Plan: plan-gopcache.md @ d88d2ea
Verified = read in plan + VideoPlayer.cpp:668-861 / VideoRing.h:73-123,250-275 / VideoPlayer.h:175-215. Inferred = marked.

## MUST
M1. pendingPublish_ holds a frame in `decodedFrame_`, then the step machine runs idle work over it (plan 3.5 forwardStep 1 / reverseStep 1:
    "tryPublishPending ... else idleWork() || false"; runStep -> decodeNextFrame -> avcodec_receive_frame). receive_frame unrefs and
    overwrites decodedFrame_ (VideoPlayer.cpp:824-831). Today's ring-full loop (:747-760) never decodes while a frame waits; the plan
    does. Result: the pending frame (a Demand run's TARGET, or a forward frame) is published later with a different frame's pixels under
    the old pts = wrong / stale picture, and the gate (uploads/s, late) cannot see it. Fix: `av_frame_move_ref(pendingFrame_, decodedFrame_)`
    into a dedicated AVFrame (pts kept beside) and publish from that; or forbid idleWork while pending. Add a ctest: fill the ring, decode
    a Demand target, step 3 idle decodes, free a slot -> published bytes == the target's.
M2. reverseStep line 2 has the SIGN INVERTED (plan :401): `servedPts_ - want > 3fd -> idleWork()` ("far enough ahead"). In reverse the writer
    is AHEAD when served < want; served - want > 3fd means the writer is BEHIND (frames still above the clock). As written, after any hitch,
    at speed >= 2x or a late frame, the writer stops publishing for good: the clock keeps falling, served - want stays > 3fd, no gen bump
    comes, the reader holds a frozen frame (stall; shown-player hold, not black, but permanent). When ahead (served << want) it keeps
    publishing frames the mirrored pick ahead-drops (VideoRing key > limit + 3.5fd) = waste. And `next = servedPts_ - fd` has no resync to
    `want` (forward has the 1.5 fd drop rule :740 and decide()). Fix: next = min(servedPts_ - fd, round(want/fd)*fd + kWriterLookAhead*0? i.e.
    never behind the clock's frame); lag > 3fd -> jump next to the clock's frame (skip, count `dropped`); ahead > 3fd -> idleWork. Add a
    ring+player ctest at speed 2x and 4x reverse (frames skipped, playhead never frozen) and a u7 scene at speed 2.
M3. No negative result for a run: a target that is never output (open-GOP / RASL frames dropped after a seek, a hole from NONREF, VFR
    rounding R-13, target past the last real frame because n = duration/fd != real count, or start_time > 0 so frame 0 is not at pts 0)
    ends its run (`f >= target`, plan :421-423) with nothing published and nothing recorded, so the next reverseStep misses again ->
    another seek + decode, no wait (runStep returns true) = a hot SEEK STORM (and at the wrap / file start forever). Also the "byte-identical
    to forward decode" claim is false for open-GOP: frames after a keyframe-seek decode differently (I have not run it: INFERRED from H.264
    recovery-point / HEVC CRA semantics; the x264 -g 10 fixture is closed-GOP so no gate sees it). Fix: a run that passes the target
    without hitting it publishes the nearest decoded frame <= target (or the first >= target), marks `hole[frame]` (small set, cleared on
    open) so get()/miss treat it as resident-alias; treat pts < stream first_pts as the end (use first pts, not 0, for `next < -0.5fd`
    and `distance()`'s n); cap consecutive demand runs on one target at 1. Add fixtures: open-GOP x264 (`-x264-params open-gop=1`), a
    non-zero start_time (mpegts / `-output_ts_offset`), and a VFR file (Q3 must not stay open for a gate that claims "any file").
M4. Reverse correctness is gated only by rates (u7 (a)(b)) plus ONE mid-window capture inside a playhead bracket (4.2 row u7 (d)). A
    forward-order serve, a duplicate, a frame 10 off, or a post-flip stale frame all give 30 uploads/s and pass a bracket of playhead
    tolerance. Fix: a counter `video_reverse_nonmonotonic` (GL thread, in uploadToTexture: shown pts > last shown pts with same gen and
    reverseNow_) gate == 0 plus >= 20 captures of a burnt-in frame-number fixture, assert monotone decreasing; the same across the ping-pong
    turn (exactly one direction change per turn).
M5. R-8 intraOnly_ is learned ("two consecutive DEMAND runs decode exactly one frame") and sticky until the next open(). A long-GOP file
    hits this whenever two consecutive misses land on keyframe-aligned targets (bottom turn frame 0 / wrap n-1 / scrubbing / after a
    trim); then the cache is permanently off = today's 4.5/s, silently, and no gate differs (u7 fixtures never sequence it). Fix: derive it
    from the container (all index entries AVINDEX_KEYFRAME, or codec_id in HAP/ProRes/MJPEG/DNxHD/rawvideo), never from run history; drop
    the learning or make it reversible (clear when any run decodes > 1 frame).

## SHOULD
S1. A run step = one decode, but the FIRST decode after `av_seek_frame`+flush at 4K frame-threads 2 can be a full keyframe + EAGAIN
    loop over several packets + cold disk read (decodeNextFrame blocks in av_read_frame, :809-835) = 30-100 ms in one step, during which a
    slot the reader just freed is not refilled (the writer only checks acquireWrite between steps). A late frame per prefetch. Fix: time
    steps; do a seek+first decode only when unservedAhead >= 3 (prefetchAt already >= capSlots/2, assert it) and add a gate: max writer step
    ms (counter) <= 20 at 1080p; INFO at 4K.
S2. Budget counts av_image_get_buffer_size of the 8-bit yuv420p case; the plan's arithmetic (4.2 u8/u10) never covers yuv422p10le (1080p 8.3 MB,
    4K 33 MB, the format the plan itself cites at convertInto) or 4:4:4/alpha: floors "win" = 8 frames x N players x 265 MB at 4K 10-bit
    over the 2 GiB. Also unaccounted: decoder frame-pool (thread_count 2, plan itself notes pools never shrink), decodedFrame_, and the
    IOSurface rings, SeqVram 1 GiB. Fix: cap = min(2 GiB, 1/16 hw.memsize) (Q1: decide it, do not leave 2 GiB fixed), a hard ceiling
    on floors (kMinFrames x frameBytes <= 256 MB or refuse and fall back to today's path), gate u8 on 10-bit 4K too, and read
    os_proc_available_memory / memory-pressure to shrink.
S3. Fill burst: 4 players x 300 frames x 3 MB of first-touch page faults + 8 FFmpeg threads inside ~3 s with the render thread at default
    QoS (Risk 4 names it, leaves it "off by default"). A page-faulting decode thread on the same cores as GL can dent fps in a live
    set. Make the yield (`wait(1)` every k decodes for PREFETCH) ON, gate on u8 median fps vs main (currently INFO) with a bar, not INFO.
S4. Retention (R-9) is capped only at capSlots/2 = up to 1 GiB at 4K per FORWARD ping-pong player, and gopFramesEst uses the max index interval;
    a single-keyframe file (webm, screen capture) makes retainFrames = the cap. Cap retention at ~256 MB absolute (Q2) and gate memory in
    u4b's ping-pong variant (bytes must not grow past it).
S5. `distance()` / frame index f = round(pts/fd) assumes fd from avg_frame_rate and CFR (R-13 admits). Under B-pyramids with mp4 edit
    lists the first frames have pts 0 with dts < 0, seek lands at keyframe whose dts <= target but pts_k may be > target (INFERRED): a
    window bottom unreachable -> M3's hole. Log first_pts / start_time in the "Opened" line and assert it in the store test.
S6. Ordering: reads of (gen, want, wantReverse_) are three independent loads (plan 3.5 decodeStep); the plan's release-before-gen store
    guarantees only new-rev-with-old-gen (harmless: stale frames get dropped) -- state that, and test `wantReverse_` flip with
    a concurrent publish under TSan (only pure tests are TSan'd, 4.1 last line; add the player store test under TSan).
S7. c1b refactor gate: the equivalence list omits the "ring-full wait returns on gen change / Reseek AND parks": once per outer iteration
    changes park latency and the notify semantics (`releasedThisFrame_` notify vs `wait(20)`); w1c fps margin is thin (plan risk 1). Add a
    forward frame-order/publish-cadence ctest driven against the OLD decodeLoop copy (golden trace: pts, gen, seq per publish) for 60 frames incl. a wrap.
S8. Test fixture is 64x64 (test_gop_cache_store): decode is ~free so amortization, step-time and budget-vs-time behavior are untested off the
    pure model; only live probes see them. Add one 640x360 GOP-60 fixture and assert run_decodes/served-frame from counters.

## NIT
N1. Tooth "NONREF inside the window fails identity for a B-frame index" is wrong: a skipped non-ref frame is never output (a hole), it does
    not produce wrong pixels; the tooth should assert `misses` rises.
N2. Diagnosis branch B thresholds (decoded/upload < 5) never fail the plan on main's own numbers (g250 predicted 60-140) -- fine, but say what
    happens if g30 falls in [5,10]: undefined.
N3. `kDecodeMsSeedPerMpix = 1.0` seeds prefetchAt before the EMA exists; on the first run at 4K that under-estimates 6.8 vs 8.3 (fine) but at
    HEVC 10-bit is ~2-3x slower (INFERRED): seed conservatively 2.0.
N4. c0 says main FAILs u7 (d) "likely"; a gate whose RED status is a guess. Run it before committing the bracket.

## Verdict
Plan's diagnosis and the mirrored pick (VideoRing.h:113-121 frees the frame below the pick) are sound. MUST M1 (decodedFrame_ clobber) and M2
(inverted lag test / no resync) are outright bugs in the specified step machine; M3 is a seek-storm / stall class on real files that no listed
fixture can trigger; M4/M5 are gates that pass while the feature is silently wrong or off.
