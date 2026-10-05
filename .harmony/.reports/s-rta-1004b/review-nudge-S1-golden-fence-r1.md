# Reviewer Verdict — nudge S1 golden-fence r1
STATUS: DONE
VERDICT: PASS_WITH_NITS (APPROVE; 0 MUST, 1 SHOULD, 3 NIT)
PINNED: lane/nudge, base 8b464a6, head c894851 (read through git objects only; nothing built or run)
FILES: tests/test_analysis_nudge_golden.cpp, tests/CMakeLists.txt, CMakeLists.txt, src/analysis/BeatShift.h/.cpp,
  src/model/BeatNudge.h, src/analysis/BPMTracker.h/.cpp, tests/test_beat_shift.cpp, .harmony/nudge-mutants-s1.py,
  .harmony/.reports/s-rta-1004b/nudge-S1.md

## STEP 0 / FENCE (VERIFIED, git log --first-parent --reverse + git show --stat)
- First commit 3ba5ca1 holds exactly tests/test_analysis_nudge_golden.cpp (270) + tests/CMakeLists.txt (+48). The
  "two CMake lines" are a 48-line block (own source list + target); S-4 explains it (no test target links
  AnalysisThread.cpp on main). Acceptable: the block is only that target. 185147b -> 8b464a6 has NO src/tests diff, so the
  base the golden was recorded on is the lane's base.
- Golden file names only fields that exist at 185147b: the 47 fields of ADNA_GOLDEN_FIELDS match FeatureSnapshot.h at
  8b464a6 in declaration order (checked by reading); raw hash covers padding, and FeatureSnapshot::clear() memsets, so the
  raw hash is deterministic. Constants/printed lines are in the report (3 runs, raw stable, tailNonZero=0). The
  auto field hash 0xd731eb3d8ada4fe7 equals bf2's kGoldenHash (grepped: 740b6d6:tests/test_analysis_sync_thread.cpp:130).
- Scratch worktree nudge-golden is gone (ls of .claude/worktrees: bf2 bf2keys keying nudge onesave outputs-a).
- Files changed 8b464a6..c894851 = exactly the S1 row + the report + .harmony/nudge-mutants-s1.py. No AnalysisThread,
  FeatureSnapshot, MainComponent, ApiServer, TopBar, binding or recording file; no UI text. BPMTracker diff is +19 lines:
  predictedBeatRegime(), appliedResyncs() (returns resyncRequestsApplied_), adoptCounters (3 stores). No frameEpoch_/excess/
  fold/Diag came along (grepped). Nothing calls apply()/adoptCounters (T-N14 asserts 0 call sites).

## RULE vs A1/A2/A3 (VERIFIED by reading BeatShift.cpp:107-283 against ruling-nudge.md A1-A3)
Steps 0-7, restart (adopt iff P.count != S.count or P.totalBar != S.totalBar), hold keeps F's seq (:213-214), cap, bar
re-seat at edge / first engaged hop / relabelDue, origin clamp, "copy S whole when delta==0 and fields equal", step-7
disengage all match. No rule added (A24 respected). Sign: kNudgePlusMeansLater=false (BeatNudge.h:12), shiftBeats gives
+D x bpm/60000; tests state the expected sign independently (test_beat_shift.cpp:46) and NR5-A's literals are applied
(T-N2/3/7/8/8b/8c/13/16 checked line by line). Real-time: no allocation/lock/juce in BeatShift (T-N14 text gate, and read).

## TESTS / MUTANTS (VERIFIED by reading; INFERRED that they run as the report says -- I did not execute)
22 TEST_CASEs in test_beat_shift (+2 golden = 24 = N_new). Every case drives a real BPMTracker + the real BeatShift.
The harness (.harmony/nudge-mutants-s1.py) builds mutants on COPIES under build-lane/mutants-s1, requires exactly one anchor
match (else NOT APPLIED), runs the named cases, has a stub arm and a self-test. All 18 mutations match their ruling text
(M23 = Da snaps to 0 then ordinary guards; M19 = uint32 count). It only compiles and runs test binaries: no kill, no
Boris folders.

## FINDINGS
[SHOULD] .harmony/nudge-mutants-s1.py is outside the S1 row's file list (the row names no harness; G-N1 says mutants are
  built in build-mut-<n>). It is needed for "each mutant RED" and is disclosed in the report, so not a MUST -- Harmony should
  accept it explicitly or amend the row. It hard-codes /usr/bin/c++, -arch arm64, /opt/homebrew/lib/libaubio.dylib and a
  copy of flags.make by hand (:176-189) -- drifts from the CMake target silently (INFERRED).
[NIT] Provenance: the ruling says BeatShift carries (re-typed) BeatLead's Beat/beatOf/writeBeat/kBeatFields/nonBeatCrc/
  recompute; neither BeatShift.h/.cpp nor the appliedResyncs() comment (BPMTracker.h:127-128) nor the commit names lane/bf2
  as origin; only the golden does (test_analysis_nudge_golden.cpp:11). The report lists BeatLead only under "files read".
  Add one line "carried from BF2 5a46c07/4ab00bd" to the report.
[NIT] BeatShift.h:57 reads kNudgePlusMeansLater as a constructor default (S-7), a second textual reader of the sign in
  analysis code beside A8's "one call of shiftBeats". Justified (T-N1 must run both values through apply; M1). S2 must
  construct BeatShift with no argument. SLIM: JUSTIFIED_KEEP reason="T-N1 both-sign run through apply is the ruled proof".
[NIT] T-N14 call-site half is a hand-edited constant (test_beat_shift.cpp:1242 kWiredCallSites=0) and a hard-coded member
  token "beatShift_.apply(" (:1255). S2 must flip it to 1 AND name the member beatShift_, else red (loud, not silent).
  Also S2 comments must not contain the text "adoptCounters(" outside BPMTracker files.
[INFO, agree with builder] F-2 (first engaged hop held: no re-seat) and F-3 (Auto LATE, D=-3 holds one hop every beat,
  10.7 ms phase freeze, inside bar (e) <=4) are rule-level gaps for the architect / FM-2, not S1 defects. S-2, S-3, S-5,
  S-6 deviations all build a superset or a closer-to-failing test and are honest.

## SLIM
BeatNudge.h clampNudge / kNudgeMinMs / kNudgeMaxMs / nudgeAfterTempoCommand: no S1 caller (kNudgeAfterResync used by
T-N8). Disposition: JUSTIFIED_KEEP reason="named by A8 for this file; consumed by S2/S3 (T-M1-3, T-U3); harness-wiring
check: none (new, model header, no hooks/config surface)". BeatShift::held/appliedMs/packWord/wordMs/wordZeroRequests:
used by tests and S2's setters (S-8). No EXCESS_DEAD found.

SUMMARY: 11 files, 4 issues (0 blocking, 1 SHOULD, 3 NIT). Confidence: VERIFIED for fence, step-0 contents, rule-vs-ruling
and test-literal checks (read); INFERRED for run results (RED/GREEN, 24 of 24, 18 of 18) -- Harmony's G-N0/G-N1 prove them.
METADATA: reviewer=independent, lane=nudge S1, date=2026-10-04
