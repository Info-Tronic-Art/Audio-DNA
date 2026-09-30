# Reviewer Verdict -- gopcache, lens decode, round 1
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES (FAIL)
REVIEWED: lane/gopcache head a02c93c vs base d88d2ea (read via git show / git archive; probes built from $TMPDIR copies)
METADATA: reviewer, date=2026-09-30

## MUST
1. GC3 "every run has a negative result" is implemented for DEMAND runs only. A PREFETCH run that passes its target
   without output stores nothing, marks no hole, and only sets prefetchBlockedAt_ = servedRel_ (VideoPlayer.cpp endRun /
   onRunFrame). planPrefetch / unservedAhead count only index>=0 as resident, so the un-fetchable index stays the prefetch
   target forever (Loop mode: through the wrap). Result: one wasted seek+catch-up run per served frame, indefinitely.
   VERIFIED (stepped harness, unlimited CPU, real VideoPlayer.cpp at a02c93c):
   - VFR file with dropped frames (ffmpeg select='gte(random(7),0.3)' -fps_mode passthrough, avg 21.25 fps): runs 57/lap
     and rising (215 runs, 236 seeks, 3909 decodes over 4 laps of an 85-frame file); nonresident set never shrinks.
     Threaded real time: lane 3265 decodes / 1300 frames vs main 1686 (2x main's CPU); shows more frames than main.
   - CFR MPEG-4 in MP4 with B-frames (120 real frames): rel 118 never stored -> one seek per served frame forever
     (325 seeks / 322 shown vs 2 seeks on an h264 control).
   The lane's VFR fixture is a setts jitter (no index holes), so test_gop_cache_store GC3 (seeks <= 12 over one lap)
   cannot fail on this hazard.
   Fix: in onRunFrame mark every rel skipped between consecutive run outputs (and the target when passed) as a hole for ALL
   run kinds; count holes as resident in resident()/planPrefetch (mutation with only that change cut runs 215 -> 126, so it
   is necessary but not sufficient); key prefetchBlockedAt_ to the target, not the served frame. Add ctests: VFR-gap fixture
   (recipe above) and an MPEG-4 B-frame fixture asserting gopCacheRuns / seeks stop growing after one lap.
2. Forward play is NOT unchanged for streams whose frames carry no pts. The new onDecoded guard
   `haveNewest_ && pts <= newestPts_ + 0.5 fd -> drop` is not inert for plain forward Loop/OneShot (plan R-15 says it is
   "inert"; c1b must keep forward line by line). pts-less frames take pts = wantTime_, so consecutive frames decoded in one
   clock tick are dropped. VERIFIED, 3 threaded real-time runs each, 4 s DivX-style AVI (mpeg4, bf 2, N/A pts on every third
   frame), 480 render frames forward: main shown 196 / late 27 / seeks 22 / decoded 420; lane shown 155 / late 44 / seeks 39 /
   decoded 670. Same lane source with the guard clause removed (mutation on a $TMPDIR copy): 184 / 31 / 21 / 395 (= main).
   The golden trace (h264 only) cannot see it.
   Fix: apply the guard only when hits are in play (hitsArmed_ || servedFromCache_), as the plan intends; add a pts-less
   fixture (ffmpeg -c:v mpeg4 -g 30 -bf 2 to .avi) forward case to the golden-trace family.

## SHOULD
3. GC9 is only nominally staggered and the gate is failing. planPrefetchRun lets a player without the token start its PREFETCH
   run when unservedAhead < urgent (~19 for 1080p g250 at 30 fps); after a deck return every player holds <= kDemandWindow-1
   = 15 frames, so all four bypass the token. Gate u11 measured 261 ms vs main 248 ms (n 4 vs 5). docs/claude/rendering.md and
   the Pitfall text say "one at a time across players: the budget's token" -- an overclaim. Either make the bypass narrower (e.g.
   only when the player would otherwise be dry within one demand latency) or state the bypass in the docs, and re-run u11 x5.
4. No automated threaded reverse test. Every reverse / flip / turn ctest steps the writer single-threaded (stepUntilIdle); the
   TSan runs the lane reports cover the ring, forward golden trace and stepped cases. I ran a TSan build (VideoPlayer.cpp + harness
   instrumented; FFmpeg / JUCE not) with a real decode thread through PingPong + reverse flips + seeks + speed changes + a Loop
   switch on CFR and VFR files: zero reports during play (reports only at teardown, an artifact of my not joining a JUCE
   thread). So this is a coverage gap, not a found race. Add a threaded reverse case to test_video_decode_trace or a TSan ctest.
5. Measurements are stale after fixes 1-2 and were never complete: docs/claude/rendering.md states "5 x 2 interleaved launches" but
   the lane arm had 4 (probe-vupload-ab.py prints FAIL, GC13 needs >= 5); GC7 256 MB row, GC11 client/malloc, the forward battery
   (c1b was held only to the golden trace), w1c/w2c and Tier-1 not run. Harmony must run all of them on the fixed binary.

## NIT
6. tests/test_video_decode_trace.cpp keeps a `GOLDEN_TRACE_OUT` getenv recording hook; its threaded case decides quiescence by a
   60 ms quiet window (a starved decode thread on the shared rig can fake it). Prefer a step-only recorder + a stepped case.
7. retainForward() copies each forward frame (0.3 ms 1080p, ~1-3 ms 4K) BEFORE ring_.publish in tryPublishPending; publish first,
   retain after, to keep forward publish latency unchanged (INFERRED: not measured).
8. CLAUDE.md index line and pitfalls entry still say "NN" (Harmony assigns at merge); APP-INVENTORY (1005 tests / 110 targets, 16
   /api/state fields) is outside the fence and not yet updated. CLAUDE.md is 24,703 bytes (<= 25,000: OK).
9. AV_FRAME_FLAG_KEY / AV_FRAME_FLAG_CORRUPT (FFmpeg >= 6.1) are used unguarded while the index API is version-guarded; the build
   docs list Linux/Windows FFmpeg packages that may be older (INFERRED).
10. The memory-pressure dispatch source's handler (level -> pressure mapping) has no test; only the atomic path is covered.
    pendingFrame_ keeps a decoder buffer while a player is parked (the flag is cleared, the frame is not unref'd).

## VERIFIED OK (decode lens)
- GC1: pending frame moves into pendingFrame_ (unref before move_ref), never shares decodedFrame_; ctest drives it.
- GC2: lag test signed correctly (next > wantRel -> jump to the clock's frame, counted dropped; next < wantRel-3 -> idle work);
  ctests at 2x / 4x / after a stall; reverse pick / peek mirrors and forward (reverse=false) path is arithmetically today's.
- GC5: intra-only from codec id or an index of >= 2 all-keyframe entries; checked mkv / ts / avi / mov indexes: no false
  positive (mkv and ts expose < 2 entries).
- Ownership: cache_, run_, store, holes, tokens are decode-thread only; freeFfmpeg/clearCache/endRun release budget, active
  count and the PREFETCH token at thread exit, park, trim and destroy. Budget give/take, freeSlot compaction, index remap,
  real-byte slot sizes (page-rounded mmap) and settle-on-first-allocation checked by reading; ctests pass here (413/253/111
  assertions).
- GL side: only pick/peek args, reverseNow_ / wantReverse_ (stored before the generation bump), counters; blit / fences / purge /
  Retire untouched (hunk list). No new mutex, no audio-thread change, no allocation added to the audio callback.
- Memory-pressure hook: dispatch source on the main queue writes one atomic; decode threads read it lazily (GC6 path).
- ADNA_GOPCACHE_BUDGET_MB is gated by AUDIODNA_TEST_SERVER and documented (GC7 gate lever); no .venv, no stray instrumentation
  in src.
- Identity vs a forward decode: h264 / hevc / vp9(mkv) / h264 10-bit reverse laps mism 0, one seek pair, zero steady-state runs.
