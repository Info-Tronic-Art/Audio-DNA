# Reviewer Verdict — onsetcount
STATUS: DONE
VERDICT: APPROVE-WITH-NITS
FILES: src/analysis/FeatureSnapshot.h, src/analysis/AnalysisThread.{h,cpp}, src/recording/RecorderHost.{h,cpp}, tests/test_recorder_host.cpp, tests/test_integration_pipeline.cpp
ISSUES:
[OK] RT safety: totalOnsetCount_ increment (AnalysisThread.cpp:178-180) is a plain uint32_t
++ inside the existing hop loop, no alloc/lock; snap->onsetCount is set unconditionally every
hop before any continue/return (verified no early exit between line 172 and end of loop body),
so it is published on every snapshot including silence-recovery/manual-BPM paths (those stages
run later, lines 187+, without skipping the earlier assignment).
[OK] Layout: static_asserts (FeatureSnapshot.h:314-325) correctly pin offsetof==316 and
sizeof==320; matches FeatureBus.h's own sizeof==320 static_assert (not touched, correctly
untouched).
[OK] Wrap-safety: unsigned delta = snap.onsetCount - *onsetCountBaseline_
(RecorderHost.cpp:439) is standard wrap-safe unsigned-subtraction; cap+carryover
(kMaxOnsetMarkersPerTick=8, RecorderHost.cpp:20-22, 441-445) advances baseline by n (emitted
count), not full delta, so excess is genuinely picked up next tick, never lost.
[OK] Arm/disarm/overdub/second-arm: onsetCountBaseline_.reset() at both arm() (line 167,
before overdub branch at 183) and disarm() (line 348) — overdub is a branch inside arm(), so
it gets a fresh baseline too. Second-arm test (test_recorder_host.cpp, "new arm resets the
dedupe baseline") proves a stale-but-still-ahead process-lifetime count from a prior take does
not leak into or flood the new take.
[OK] TestServer / --test-mode: handleInjectFeatures (TestServer.cpp:432-437) builds a fresh
`FeatureSnapshot injected; injected.clear();` per call and never sets onsetCount, so it is 0 on
EVERY publish. RecorderHost's baseline therefore also settles at 0 and stays there (delta
always 0-0=0) — no spurious wrap, no flood. Confirmed by tracing, not just asserted: this is a
latent "count never advances" limitation of test mode (onset markers can never fire from
injected features), not a wrap/flood risk. Low severity, pre-existing scope (onsetDetected
injection was already partially non-functional for markers before this diff).
[ISSUE] Correctness (narrow edge case), RecorderHost.h onsetCountBaseline_ comment / .cpp:428-437:
the FIRST tick observed after arm() unconditionally becomes the baseline with zero markers
emitted for it — but the design cannot distinguish "this snapshot's onsetDetected pulse is a
stale hop from before arm" (correctly swallowed) from "a real onset landed in the very first hop
tick() observes after arm, i.e. during the take" (incorrectly swallowed — no test covers this).
The old timestamp-dedupe had the opposite bug (it WOULD emit for a stale pre-arm pulse on the
first tick). Window is narrow (one tick period, ~8ms) but is a genuine regression direction not
called out in the header comment, which claims only "never emit for onsets before this arm"
without acknowledging the post-arm-same-tick miss case. Not blocking (narrow, and arguably the
safer of two bad choices), but should be filed as a known limitation rather than left implicit.
[ISSUE] Test coverage gap: kMaxOnsetMarkersPerTick=8 cap-with-carryover and uint32 wrap-around
are both explicitly named in the packet's defect description and in code comments
(RecorderHost.cpp:20-22, 445) but neither has a dedicated test — grep of
tests/test_recorder_host.cpp and tests/test_integration_pipeline.cpp for "cap"/"wrap" finds only
comment prose, no assertions exercising delta>8 or a count near UINT32_MAX. Logic is simple and
verified correct by inspection (advance-by-n, unsigned subtraction), but per gate-review practice
this is exactly the kind of untested edge a green suite won't reveal if it regresses later.
Suggest: one RecorderHost test with delta=10 (expect 8 markers this tick, 2 next) and one with
baseline near UINT32_MAX wrapping past 0.
[OK] Test teeth: test_recorder_host.cpp's "jump by 2" case is fail-first against the old
timestamp dedupe (explicit comment ties it to the regression); test_integration_pipeline.cpp's
new case drives PipelineRunner end-to-end on a real synthesized click train (with noise floor to
avoid aubio's silence-suppression artifact) through the real onset detector, not a synthetic
FeatureSnapshot fixture, and asserts countIncrements == detectedOnsets with a >5 sanity floor —
this is a live-data-shaped check on the invariant, not just a mock.
SUMMARY: 7 files reviewed, 2 issues (0 blocking, 2 suggestions/nits — narrow first-tick-after-arm
miss window, and missing cap/wrap-specific tests). Core RT-safety, layout, wrap-math, and
arm/disarm/overdub scoping all verified correct by direct inspection of the diff and tracing
control flow, not by recall.
METADATA: reviewer=reviewer-agent, builder_packet=onsetcount, date=2026-09-24
