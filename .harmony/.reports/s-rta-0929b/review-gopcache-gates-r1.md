# Reviewer Verdict - gopcache gates r1 (lens: gates)
STATUS: PARTIAL
VERDICT: FAIL (REQUEST_CHANGES)
REVIEWED: lane/gopcache head a02c93c vs base d88d2ea (read via git show/diff; ctests re-run from build-lane binaries; mutant + ab.py re-runs in $TMPDIR)

## Verified (by execution)
- test_video_ring 683/31, test_gop_cache 111/11, test_video_decode_trace 253/2 (6 runs), test_gop_cache_store 413/17 (3 runs): all PASS, stable.
- Golden trace (GC12): literals unchanged since c0 (git diff e13dccb..a02c93c shows only the added stepped case); c0's VideoPlayer diff touches only GL-side counters, so the recording is main's writer. Both threaded and stepped variants run the REAL VideoPlayer/decoder.
- RED on main: re-ran probe-vupload-ab.py on the c0 main-vs-main tsv: reverse/turn/speed2/flip (a)(b)(GC4 mono) FAIL, counters absent. New ctests do not compile on main (plan-sanctioned RED).
- Nothing stray: no .venv, no instrumentation; env hooks ADNA_GOPCACHE_BUDGET_MB is under #if AUDIODNA_TEST_SERVER (precedent ADNA_VIDEO_FORCE_FALLBACK); no new mutex/lock/syscall on audio path; CLAUDE.md 24,703 B.

## MUST
1. Forward battery never run (probe-video all rows, w10-all, u2/u4a/u4b/u6, w1c/w2c interleaved, crossfade/media-open/async-load/seq-vram, Tier-1). This lane refactors the forward writer (c1b) AND adds a per-published-frame native-format copy on every non-intra-only forward clip (GC8 retention: 3.1 MB/1080p, 12.4 MB/4K per frame). c1b was not held to its own battery. w2c 4K x4 is the worst case and is unmeasured.
2. GC13 unmet: 4 lane launches (<5); ab.py prints FAIL on every rule. The 5th lane launch never answered /api/health (err.log: CoreAudio 10004003 only) - unattributed (main has a known startup issue) so cannot be dismissed. Re-run 5x5 on f1.
3. GC9 FAIL as adopted: B median first-upload 261.15 ms vs A 247.9. Raw: A 389/284/238/236/248, B 311/263/259/251. Within noise, so re-run n=5 before/after tuning kDemandWindow; do not re-threshold.
4. GC11 client/malloc arms of u12 not run; GC7 256 MB row not run (and the pressure source never exercised live).

## SHOULD
- GC10 ctest cannot tell mmap from malloc: my mutant (mapBuffer -> av_frame_get_buffer) still passes the footprint-return assertion (end-base negative); it fails only on the precondition peak-base>25MB (24.38 MB, 0.6 MB margin). Use >=2 MB frames or a page-count probe.
- GC3 keyframe gate has no tooth (builder-admitted): mutant storing before the run's keyframe passes every case.
- Plan 4.3 "u4b bytes==0" dropped under GC8 with no replacement bound: add a forward retention bytes/footprint assertion (u4b/w1c).
- Docs overclaim: rendering.md says PREFETCH runs are "one at a time across players: the budget's token" but planPrefetchRun (VideoPlayer.cpp ~1277) lets a player run without the token when 'urgent'.
## NIT
- test_video_ring "reverse = false is today's pick" compares pick() to pick(...,false): identical call, tautological.
- GOLDEN_TRACE_OUT getenv left in test_video_decode_trace.cpp:168.
- Pitfall number NN; APP-INVENTORY not updated (outside fence) - Harmony.
- Harmony: UserNotificationCenter dialog still on screen; builder's rig-rule breaches (cd, TERM of UNC).
