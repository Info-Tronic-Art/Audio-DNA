# Reviewer Verdict — gopcache-gates-r2 (lens: gates)
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES (FAIL)
REVIEWED: lane/gopcache 98994c6 vs base a02c93c (git show/diff only); ctest TUs rebuilt in $TMPDIR against src at git revs (rev.py rig), never the working tree.

## Verified by execution
- F1 RED on a02c93c: HEAD's "GC3 holes" case -> 1 failed / 36 assertions (seeks 130<=1, runs 126<=1, misses 4<=1; mpeg4 missing := "118 ", seeks 236<=1). GREEN on 98994c6: test_gop_cache_store 499/20 pass.
- F2 RED on a02c93c: HEAD's AVI golden cases (threaded+stepped) fail, "decoded 130 dropped 54 seeks 3" vs golden "decoded 128 dropped 14 seeks 3". GREEN on 98994c6: test_video_decode_trace 510/5 pass, 3/3 runs.
- GC12 golden (AVI) INDEPENDENTLY re-recorded on e13dccb's writer (git diff d88d2ea..e13dccb touches VideoPlayer.cpp only in GL-side counters): 3/3 runs sha b26ed02ca55eceae; published/picks/counters == the literals in HEAD's test byte for byte. Golden really equals the pre-refactor writer.
- Threaded reverse case: 6 parallel copies all pass (no timing flake seen).
- Nothing stray: no .venv / instrumentation committed; no new mutex/lock/syscall in the src diff; CLAUDE.md 24,703 B; probe-vupload.py compiles.

## MUST
1. NO live gate has been run on the HEAD code. Every live number in gopcache.md (u7/u8-u12, GC9, GC11 u12, GC12 u7 forward scenes) is from the a02c93c app; F1 (overshoot re-seek lengthens a DEMAND run) and F2 (forward writer guard) changed VideoPlayer.cpp after them. Open plan items: forward battery (probe-video all rows, w10-all, u2/u4a/u4b/u6/u12, crossfade/media-open/async-load/seq-vram, w1c/w2c/w6b 5x2 interleaved), GC13 (n=4 lane launches, needs >=5 per arm, medians), GC9 u11 (261 vs 248 ms, n 4 vs 5), GC11 u12 client/malloc + w10-all, GC7 256 MB x5, u4b (f)/(g) RED+GREEN, Tier-1. Blocked by the TCC dialog (Boris's to answer) -> not a builder fault, still an unmet plan item. Fix: Harmony runs `live.sh fwd ab7 ab256 abw tier1` on the FIX app after the dialog is cleared; no re-thresholding.
## SHOULD
- GC9 lever (kDemandWindow) unapplied; F1's overshoot re-seek can lengthen the u11 DEMAND -- u11 x5 on HEAD decides; do not tune on n=4.
- Builder-admitted: reverse on a pts-less AVI shows keyframe pictures; VFR file shows fewer distinct reverse frames than main (~25 of 85/lap). File as debt with a repro, not just an ISSUES line.
- The target-keyed prefetch block has no tooth (a served-keyed mutant passes) -- defensive dead weight (EXCESS_VESTIGIAL candidate); DEBT_FILED or remove.
## NIT
- tests/test_video_decode_trace.cpp:190 getenv("GOLDEN_TRACE_OUT") still in the committed test (r1 nit unfixed; recorder should live in scratch only).
- test_video_ring "reverse = false is today's pick" tautology (r1 nit) unfixed.
- APP-INVENTORY (+6 ctests, 1011) outside the fence: Harmony.
