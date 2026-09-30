# Reviewer Verdict — gopcache-gates-r3
STATUS: DONE
VERDICT: APPROVE (code + tests) -- live gates OPEN for Harmony (not waived)
REVIEWED: lane/gopcache head 9cce17d vs base 98994c6 (src/media/VideoPlayer.{cpp,h}, tests/test_gop_cache_store.cpp, docs)

## Verified (executed, not recalled)
- R1 + R2 ctests RED on 98994c6: built base VideoPlayer.cpp/.h (git show) + HEAD test TU in $TMPDIR via the lane's link recipe:
  R1 `badResident 118 == 0`, `mismatches 368 == 0`; R2 `aheadDropped 39 == 0` and `distinct*10: 250 >= 378` on all 3 laps;
  2 cases failed / 8 assertions failed. Thresholds unchanged (kMainMedianPerLap 42, 0.9x; R1 exact identity).
- Same harness on HEAD 9cce17d: 523 assertions / 22 cases pass. Shipped build-lane binary: R1+R2 pass (24 assertions).
- Tests drive the real VideoPlayer (stepped decodeStep + real FFmpeg decode, forwardDecodeByOrder reference), not a model.
- ctest -j4 full: 1 failure "Deck round-trip: mapping resolution..." (432) -- passes twice alone (parallel flake, unrelated to media).
- Code read: indexTimeOf / runLastOutRel_ chain / overshoot on unknown landing / forwardIdle no-index path / reverseStep time-based
  look-ahead after hole walk -- no defect found. Pathological: last frame unindexable AND previous output outside a 1-frame window
  -> frame not stored, nTraj excludes it (degraded, no loop).

## Live gates (R4) -- state plainly
NONE run on HEAD. TCC microphone prompt unanswered (lane precondition failed 04:48:20). Open for Harmony: forward battery,
u7 re-run (R2 changes live reverse look-ahead), GC13 n>=5, GC9 u11 x5 (b<=a was FAIL 261 vs 248 at f1), GC11 client/malloc,
GC7 256MB, u4b (f)/(g), Tier-1, w10-all.

## Issues
- [OPEN for Harmony, not code] live gates above not run on HEAD.
- [NIT] tests/test_gop_cache_store.cpp R2 comment says "RED on 98994c6: 35 distinct a lap, 28 freed unshown"; measured RED in the test is 25 distinct / 39 freed (report says 25/39). Stale number.
- [NIT] R3 (MPEG-TS pre-key publish: 21 of 23 shown wrong, no test) and R2 residual (20 of 85 VFR frames never shown, one frame/index) are filed, unguarded -- accepted per ruling R3.
- [NIT] R2 gate's main baseline (42) comes from a threaded run on main while the test is stepped; conservative, but not re-derivable from the repo.
