# ATTACK -- GATE + REGRESSION SKEPTIC vs plan-tempo.md (s-rta-0928)

VERDICT: SOUND_WITH_FIXES -- the handshake design and its FeatureSnapshot/BPMTracker citations check
out exactly, but the mandatory "RED on the current app" gate is not reliably reachable as specified,
and two of the ctest "law" cases never execute production code.

## MUST 1 -- tight-pair's modelled RED rate contradicts the plan's own input diagnostic
plan-tempo.md:696-697 defines "tight pair" (stall hook unavailable) as "sub-millisecond gap, natural
timing" and models ~60% (W2) / ~95% (W3) per-cycle failure. Its own cited input,
.harmony/.reports/s-rta-0927/tempo0-diag.md:4,33, measured this identical natural scenario (curl
set_bpm then curl record, no stall) at a median 29 ms gap -- about 3x one hop (10.7ms), not
sub-millisecond -- with only 4/34=12% start-bpm-0 and 9/34=26% unknown-grid, not 60/95%. The plan
never reconciles the 5x gap between its own measured baseline and its own model. Consequence:
Binomial(n=20,p=0.12) gives P(X>=5)=8.2% (1 - sum P(0..4)=1-0.918). If a builder's environment lacks
the stall hook (any Release build without AUDIODNA_BUILD_TEST_SERVER), there is about a 92% chance a
20-cycle W2 run shows fewer than 5 failures and the plan's own rule ("counts as a gate only if FAILED
>=5/20", section 6) reports "no refutation power" even though the defect is real. W3's 95% is worse
than unverified: the diag never measured the stale-grid-after-Resync case at all (plan-tempo.md:54,
"INFERRED"), so it is pure model against zero live data.
Fix: make the stall-assisted path mandatory, not a first-choice-with-silent-fallback -- it is real and
precedented (src/api/ApiServer.cpp:1582-1597; already forces deterministic RED in probe-beatclock.sh
b2/b3 and probe-routines.sh row 7s, 3/3 FAIL on old code per
.harmony/.reports/s-rta-0927/beatclock.md:6-7) -- or re-derive the tight-pair model from the diag's
12%/26% and size the cycle count/bar to match.

## MUST 2 -- L1 and L2 are text scans, not executed code
The new ctest target links BPMTracker.cpp/RecorderHost.cpp etc. but NOT MainComponent.cpp or
AnalysisThread.cpp (confirmed absent from the add_executable(test_tempo_start ...) source list,
plan-tempo.md:406-426). L1/L2 (plan-tempo.md:595-599, 645-649) instead read the raw source TEXT of
those files and check substring presence/offset order. A logically inverted wire-up (e.g. an
if(testMode_) sense flipped, or the wrong tracker) would still contain "postedRequestSeq()"/
"testMode_" and omit "startBeatInBar"/"getFeatureBus().read()" -- L2 stays GREEN. This is the one
link between the new sequence number and the real arm call site (verified current at
src/MainComponent.cpp:5270-5277) that no other case in the suite exercises end-to-end. "ctest 100%"
in the Done checklist (plan-tempo.md:873-874) overstates what is actually gated here; only the live
probe (section 6, which does call the real /api/perf/record) behavior-tests this wire.

## SHOULD
- Onset-marker baseline window (Pitfall 30) grows from <=8ms today to up to the 100ms fallback when
  raced (plan-tempo.md:297-299, R3); named but never bounded (e.g. onsets-at-risk at 120 BPM).
- startWaitSnap_ is a heap unique_ptr<FeatureSnapshot> (384B) purely to survive Stop-before-t=0
  (plan-tempo.md:289,349-351) when startClock reads only 5-6 scalar fields; a small POD avoids the
  allocation and forward-declare workaround -- smaller than proposed, the plan's own cost logic.
- Section 7 gives exact re-run env vars only for probe-routines.sh; the other 4 re-run rows get none.

## Verified clean
No existing probe records with a tempo command in flight: probe-beatclock/downbeat-level/resync.sh
have 0 hits for "perf/record" (grep-confirmed); probe-manual-bpm.sh records at line 117, strictly
before its first tempo command (~203/204, grep-confirmed). probe-step3.sh's T2 is
m['sample'] - firstSample (.harmony/probe-step3.sh:527), sample-domain, immune to a t=0 shift.
FeatureSnapshot layout (offsetof(totalBeatCount)==324, sizeof==384,
src/analysis/FeatureSnapshot.h:178,182) and all cited BPMTracker.cpp lines (69,555,586,625) match
exactly, no drift.

## Strongest counterargument, and why it doesn't fully clear MUST 1
The plan's credibility rule exists precisely to catch a low-power run and report "no refutation
power" rather than silently pass -- so the gate cannot lie. True, and the right instinct. But an
8%-chance-of-firing fallback path is not a reliable mandatory step run once in a five-commit sequence;
the plan names the rule but gives no retry/escalation for the case it trips. The fix is one sentence,
not a redesign.
