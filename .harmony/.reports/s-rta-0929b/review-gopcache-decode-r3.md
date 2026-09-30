# Reviewer Verdict — gopcache decode r3
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS)
REVIEWED: lane/gopcache head 9cce17d (base 98994c6) -- src/media/VideoPlayer.cpp/.h, tests/test_gop_cache_store.cpp, docs (rendering.md, pitfalls.md)
ISSUES: 0 blocking / 5 suggestions

VERIFIED (executed by me):
- test_gop_cache_store (523 assertions / 22 cases) and test_video_decode_trace (510 / 5, golden forward traces, file untouched by the diff) pass on the lane's build; R1+R2 cases pass 7 of 7 runs.
- R1 premise: independent FFmpeg 8.0 probe (probe.c, thread_count 1 and 2) on the bf2 fixture AND on two H.264-in-AVI files (bf2, bf3 b-pyramid): best_effort_timestamp = output position + constant, identical from file start and after av_seek_frame+flush at every keyframe (0 differences in 32 / 40 / 40 compared outputs); only the drained last frame(s) have no time -> covered by the runLastOutRel_+1 chain (H.264-AVI has 2 such frames; the chain gives both an index).
- R2: VideoRing::pick reverse frees pts < clock - tol(0.5fd) - dropAhead((LA+1)fd) (VideoRing.h:66-125); the new writer line (want - (LA+1)fd, VideoPlayer.cpp:1251) is inside it by half a frame; the reverse clock falls, so a stale `want` only makes the writer stricter. Doc claim matches the code.
- Forward play: onDecoded publishes with the same `pts` as before; only lastDecodedRel_ (read in forward only under servedFromCache_) and the retainForward index changed. Golden trace green.

NITS (none blocking):
1. NIT (DRY/coupling): the ahead line (kWriterLookAhead + 1) * frameDur_ now appears in VideoPlayer.cpp:162, :552, :1251 and mirrored by hand in the test's pick(); R2 was exactly a writer/pick line mismatch. A named helper (aheadLineSec()) would make the coupling structural, and the test's pick() copy could silently drift from :552.
2. NIT (R2 residual, INFERRED not run): for a MISS the check uses the nominal ptsOfRel(next), not the frame's real pts; with a bounded budget on a VFR file a decoded frame could still land below the pick line. R2 gate only covers the whole-file-resident case.
3. NIT (EXCESS_VESTIGIAL-ish): forwardIdle's new no-index branch (VideoPlayer.cpp ~1542) counts a decode as framesDropped + gopCacheRunDecodes and returns true without updating lastDecodedRel_/haveDecoded_; only reachable for the EOF-drained frame, so harmless, but it duplicates stat lines and leaves the decoder position stale.
4. NIT (robustness, INFERRED): a stream where EVERY frame lacks pts and best-effort (raw .h264 ES: ffprobe shows N/A on 90 of 90) would never be indexed by a run: no store/publish, 3 overshoot seeks, then a decode-to-EOF per DEMAND, repeated. Not reachable through the accepted extensions (.mov/.avi/.mp4/.mkv/.webm/.m4v all carry dts/pts); main's behaviour on it was also wrong (garbage frames). A "cannot index -> hold the shown frame" guard is cheaper than a spin.
5. NIT (test): R1 asserts the frames SHOWN and CACHED equal the forward decode by position, not that frame k is shown at the right CLOCK time (index scale = be = position + constant; frame 0 sits at firstPts 0.0333 s). A one-frame time offset would pass. Acceptable for the picture-identity goal.

R3 (MPEG-TS pre-key parity) filed, not reviewed as a fix. R4 live gates BLOCKED by the TCC prompt: reported, not a FAIL. u7 must be re-run on this build (R2 touches live reverse).
Not re-run by me: TSan, the RED/teeth mutants (the lane's mutant table is INFERRED from its report; no C++ rebuild possible read-only).
METADATA: reviewer=claude-sonnet-5-5, packet=gopcache lens decode r3, date=2026-09-30
