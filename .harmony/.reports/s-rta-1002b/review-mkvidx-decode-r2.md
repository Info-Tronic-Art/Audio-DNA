# Reviewer Verdict -- mkvidx, lens decode, round 2
STATUS: DONE
VERDICT: PASS_WITH_NITS (0 MUST, 0 SHOULD, 1 NIT)
REVIEWED: lane/mkvidx @ 4a41ddc0d3cd8d862c781594dbe3c79df5c6ee4c (fix-round base 79dd452). Read via git show / git diff 79dd452..4a41ddc only.

## Answer
R1 and R2 are implemented as ruled; no regression to the non-intra keyframe model or the RUN seek aim. Both round-1 decode SHOULDs (S1, S2) and the N1 NIT are fixed/filed as ruled.

## R1 (VERIFIED by running)
- src/media/VideoPlayer.cpp:1583 `|| (!atOpen && intraOnly_)` -- open() still reads the index once (verdict); no rebuild after. Non-intra path (:1584-1623) byte-unchanged except the counter (:1594-1595) and the witness reset (:1615).
- Counter: src/media/VideoStats.h:33 `keyIndexRebuilds`, incremented only on a real post-open rebuild, after the unchanged-triple early return.
- Test: tests/test_gop_cache_store.cpp:2063-2092 "mkvidx T2f" (FX4e forward, 132 steps, whole file + EOF + Loop wrap): non-vacuity guard `entriesNow > entriesAtOpen + 2` (:2088), shown >= 30, `rebuilds <= 2` (:2090).
- Teeth, my own relink rig (HEAD src copy in scratchpad, only VideoPlayer.cpp.o replaced in the lane's test_gop_cache_store link; deliverable untouched):
  * m0 clean HEAD: T2f "rebuilds 0, shown 34", `mkvidx*` 389 assertions / 7 cases pass; whole test_gop_cache_store 1051 / 33 pass.
  * m1 (guard removed == 79dd452 behaviour + counter): "index entries 8 -> 30, rebuilds 22" -> test_gop_cache_store.cpp:2090 FAILED, 1 case failed. RED confirmed (22 = 30 - 8, one per frame read).
  * m3 (never rebuild after open, `|| !atOpen`): T2 (:2007-2009, :2032-2033, :2057-2058) and :1866 FAIL -- the non-intra live-index model is pinned, so R1 cannot silently have broken it.
  * m4 (`|| intraOnly_` without !atOpen) passes: equivalent for h264 (intraOnly_ is false until the verdict) -- not a gap; the T1 guard covers the verdict.
- Existing lane binaries (built 16:57, after the NIT commit): test_video_decode_trace 517 / 6, test_gop_cache 164 / 14, test_video_player_open 10 / 2 all pass.
- Reader audit of what an intra stream no longer refreshes (keyRels_, gopFramesEst_): keyRels_ readers planPrefetchRun (:1281) / forwardRetain (:1038/:1045) return on intraOnly_ first; gopFramesEst_ readers :871 and :1518 sit behind servedFromCache_ / hitsArmed_ (false for intra, :817), :1302 behind the planPrefetchRun return, :1417 only for a pts-less overshoot (not reachable when the DEMAND run lands on its frame, AM4), :297 is the open log. VERIFIED by reading.
- intraOnly_ is re-derived in open() (:276) so the skip can never leak across a re-open; witness reset (:1615) closes N1.

## R2 (VERIFIED by diff + grep)
- No code change: the 79dd452..HEAD diff of src touches GopCache.h comment lines only (:470-476 states F8 and F6), VideoPlayer.cpp only readKeyIndex, so forwardStep / forwardIdle / runStep's `ptsOfRel(r.seekFrom) + 0.5*frameDur_` aim are untouched.
- docs/claude/pitfalls.md Pitfall 64 (3): "The rule is NOT safe in every case ... F8 ... F6"; rendering.md intra-only clause names both; 0 remaining "conservative" in pitfalls / rendering / GopCache.h / VideoPlayer / tests / CLAUDE.md. F8 text matches my round-1 S2 (adjacent first two keys, 2-entry index, freeze like X1, not a regression). CLAUDE.md line 64 unchanged, 23,976 B.
- The F8 / F6 filing is in the lane report ("R2 ... FILED (beside F6, not this lane)") with trigger and candidate fixes; honest about the freeze being measured on the proto3 binary only.

## Findings
N1 NIT (INFO, verified harmless) -- for a CODEC-intra stream (HAP / MJPEG / ProRes in Matroska with Cues at the end, open index of 1 entry) gopFramesEst_ now stays at open()'s value (totalFrames) instead of drifting to 1 after the first index change; every reader is unreachable for intra (audit above), so no behavioural change. Consider one comment line in readKeyIndex saying gopFramesEst_ is also frozen for intra streams (the current comment says only keyRels_ is unused). No action required.

## SLIM
No excess code: keyIndexRebuilds is referenced (VideoPlayer.cpp:1595, T2f); the guard is live.

## Not reviewed (other lens)
.harmony/probe-vupload-ab.py R3 / R4 selftest and rule binding (gates lens); live A/B and TSan are Harmony's (lane reports TSan 0 on three targets; not re-run).

CONFIDENCE: High -- R1 behaviour, RED and tooth VERIFIED by executed relink mutants; reader audit VERIFIED by reading; R2 VERIFIED by diff.
