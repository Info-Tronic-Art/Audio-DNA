# Reviewer Verdict - nudge S1 (THE ARITHMETIC), lens arithmetic-tests, round 1
STATUS: DONE
VERDICT: APPROVE (0 MUST, 2 SHOULD, 5 NIT) -- structured verdict PASS_WITH_NITS
REVIEWED: lane/nudge head c894851, base 8b464a6 (read through git objects only; nothing built, run or launched)
FILES: src/analysis/BeatShift.h/.cpp, src/model/BeatNudge.h, src/analysis/BPMTracker.h/.cpp (+19), tests/test_beat_shift.cpp,
  tests/test_analysis_nudge_golden.cpp, CMakeLists.txt, tests/CMakeLists.txt, .harmony/nudge-mutants-s1.py, report nudge-S1.md
SPEC READ: plan-nudge.md adoption blocks (2), plan-nudge-row.md adoption blocks (3) + NR5-A, ruling-nudge.md A1-A24 / s4 S1 row /
  s5, ruling-nudge-row.md s4 (S1 = "as RN, with kNudgePlusMeansLater = false and NR5-A's literals": NOT amended otherwise).
  Latest hold block (21:33) names S1r..VG, not S1.

## What I verified (VERIFIED = read / grepped at c894851)
- A1 steps 0-7 re-derived line by line against BeatShift.cpp:107-283: anchor / lag (:119-122), identity (:129-136), restart
  and its consume-whether-or-not rule (:148-173), glide 0.25 ms/ms = 2.6667 (BeatShift.h:77, :177-180), exact position and the
  1.0f rounding rule (:195-207), hold + cap on the count against F with A3's sequence (:211-224), bar fields seated at edges /
  first engaged hop / relabel (:233-267), origin clamp and the two float formulas (:268-270), return-to-identity copy (:272-274),
  step 7 (:281). No rule is added; none is missing. Both regimes (barsAdvance = predicted || locked, :247) match A1.
- A2 adoptCounters (BPMTracker.cpp:580-585): totalBeatCount_, totalBarCount_, resyncBarOrigin_ only; one writer, no caller yet.
  A4 views: BPMTracker.h:126,128 read-only, return the existing members (predictedBeatRegime_ :249, resyncRequestsApplied_ :301).
- A8 / NR5-A sign: BeatNudge.h:13 kNudgePlusMeansLater = false; shiftBeats = +ms x bpm / 60000 when plus = earlier (:28-34);
  T-N1 runs both values AND pins the build's sign (test_beat_shift.cpp:357,381-387) with an oracle written out independently
  (expectedShiftBeats, :46).
- NR5-A literals applied: T-N2 (:412-413 "+D"), T-N3 (:450-458 "-D"), T-N7 (D = -500, :696-776), T-N8 (+500 at phase 0.2, :867;
  bar-counter arm D = +40, :883), T-N8b (-250, :972), T-N8c (-40, :1008), T-N13 (-500, :1191), T-N16 (:1302 "+target").
- Cases present under their pre-registered names: T-N1,2,3,4,5,5b,6,7,8,8b,8c,8d,11,12,13,14,15,16 (22 TEST_CASEs with the word
  case); T-N9 is a check inside T-N4/T-N5/T-N8c (crcMismatch, :493,:569,:1043) as the ruling words it. Each drives a REAL
  BPMTracker (Rig, :82-210) except the T-N5b trigger rule (NIT 3) and T-N14 (text) / T-N15 (bytes).
- Golden: 3ba5ca1 is first, parent 8b464a6, holds ONLY tests/test_analysis_nudge_golden.cpp + a 48-line CMake block (A9's
  "two lines" was an underestimate; nothing else). tests/CMakeLists.txt blob at 8b464a6 == at 185147b (9993275), src and tests
  unchanged 185147b..8b464a6, so the constants are valid on the lane base. File is byte-identical at c894851 (git diff
  3ba5ca1 c894851 on it is empty): never re-recorded. Field list (:83-90) covers every FeatureSnapshot field of 8b464a6
  (FeatureSnapshot.h:9-144); commands schedule (:170-173) = hops 300/700/1000/1400 as A9; tail 332..383 and raw hash checked;
  names no lane-added field. Two "[golden] <case> publishes=1965 hash=0x.. raw=0x.." lines as G-N0 words them (:235).
- Stray check: diff stat is exactly the 11 files above; no instrumentation, printf or vector in src; the forbidden-token text
  gate of T-N14 re-run by me with grep on BeatShift.h/.cpp: only "BeatShift::apply(" at BeatShift.cpp:107 (own file, excluded).
  No UI text, no doc touched (the S1 row names none), no .venv.
- Mutant harness (.harmony/nudge-mutants-s1.py): copies, never edits the tree (build_one :145-195); each edit anchored with
  count == 1 else NOT APPLIED (:168); selftest proves SURVIVED and NOT APPLIED paths (:214-231); 18 mutants named as the ruling
  (M1-M11, M19-M23, M27, M28) + a stub arm, each mapped to a case tag (:47-117). No app, kill or pkill line; writes only under
  <build>/mutants-s1. The report's raw lines are the builder's (MEASURED by him); I did not re-run them (not permitted).
- S-2 (T-N6) re-derived by reasoning: with plus = earlier the beat-skipping Tap is the mirror of the ruled arm (shift grows
  positive), so the ruled arms hold, not cap; the builder's superset + `burstArms >= 1` precondition (:655,:689) is correct.
  S-3 (T-N8b / M27) likewise: an edge re-seats the bar fields whether a relabel is pending or not; the added +250 / beatInBar 2
  arm (:988-1000) is the one where only the relabel can fix the label. Both are correct readings, not re-designs.

## FINDINGS

[SHOULD-1] The lag == -1 branch of A1 step 0 (and the lower clamp) has no committed case that reaches it. VERIFIED: Rig::autoHop
  feeds every onset at confidence 0.9 (test_beat_shift.cpp:191), and BPMTracker::updatePhase realigns on conf >= 0.5
  (BPMTracker.cpp:235, kBeatResetConfidence BPMTracker.h:57) -> realignPhaseToZero increments the count from the second half of the
  beat (BPMTracker.cpp:221-227) on the SAME hop the onset moves the bar position, so arm EARLY never has "bar position names the
  beat about to be counted" (lag -1). The builder says so himself (report F-4) and ran an uncommitted soft-onset experiment
  (conf 0.3, 3 hops before the wrap) that was green. Consequence: a mutant that keeps lag but drops only its negative side
  (e.g. `lag = std::max<int64_t>(lag, 0)`) survives every committed case; M21 (lag removed) is killed only by the +1 side.
  Real music has soft onsets, so this is the branch Harmony's live rows meet first. -> Commit the soft-onset EARLY arm (T-N5 and
  T-N5b, D = +-3/+-30/+-250, assertions (a)-(d) unchanged) and add a harness mutant "lag clamped at 0 from below" that must be RED.
  Not a MUST: the ruled EARLY arm is as the ruling words it (confident onset), and the gap is disclosed.

[SHOULD-2] "barPhase' and phrasePhase' equal the two float formulas bit for bit" (T-N4, ruling A1 step 6) is checked only against
  BeatShift's own functions: Watch::see calls BeatShift::barPhaseOf / phrasePhaseOf (test_beat_shift.cpp:296-297) and nothing
  else in tests or src calls them (grep). It proves P's bar phase is computed from the PUBLISHED beatInBar'/phase' (M3 class) but
  not that the formula equals BPMTracker::updateBarPhase / updatePhrase (BPMTracker.cpp:486-503, :550-557). A drifted clamp or
  divisor in BeatShift would pass. -> add an oracle on the tracker's own bytes: on every Rig hop (Manual and Auto-locked)
  `barPhaseOf(S.beatInBar, S.beatPhase)` == S.barPhase and `phrasePhaseOf(S.barCount, S.barPhase, t.phraseBars())` ==
  S.phrasePhase, bit for bit (S is the tracker's own after feedDownbeatFeatures; applyResync keeps the identity at 0).
  (INFERRED that the identity holds on every hop; it is true by the tracker's code order, but not run by me.)

[NIT-1] .harmony/nudge-mutants-s1.py is a new file outside the file list the S1 row names (src, tests, the two CMake files). It is
  the evidence artefact for the stage's EXIT ("unit mutants each RED") and does nothing unsafe, so not a fence MUST; INFERRED that
  the packet did not name it, so Harmony should ratify its home. It hard-codes /usr/bin/c++, -arch arm64, /opt/homebrew
  (:176-186), its docstring says "five source files" (:4) for six (:32-39), and G-N1's sentence says build-mut-<n> while the
  harness uses build-lane/mutants-s1 (the rig's copy rule; fine per this review's own rule).

[NIT-2] T-N14's call-site half counts only the tokens `BeatShift::apply(` and `beatShift_.apply(` (test_beat_shift.cpp:1255):
  S2 must name its member beatShift_ or a call through another name escapes the lint; the half is asserted == 0 at S1 via
  kWiredCallSites (:1242, report S-1, accepted: S1 wires nothing). S2 must flip the constant to 1 or its own build goes red.

[NIT-3] T-N5b drives a two-line copy of Layer's Bar rule (test_beat_shift.cpp:619) instead of the real
  Layer::processPendingTrigger (src/model/Layer.h:449-475 Bar case; Autopilot.cpp:85 is the real caller), although tests
  already build a Layer (tests/test_layer_runtime.cpp:359, test_undo_commands.cpp:2742). The ruling allows the copy ("else its
  two-line rule, and the report says which") and the report does say; the stated reason (keep the target Catch2 + aubio so the
  harness can compile it) is self-imposed. Drift risk only. A one-line text assertion that Layer.h still holds
  `shouldTrigger = (beatInBar == 0)` would pin it.

[NIT-4] F-2 (INFERRED by reading, not run): `firstEngagedHop = !engaged_` is taken at BeatShift.cpp:137; a first engaged hop that
  HOLDS returns at :211-218 with engaged_ = true, so the A1 "re-seat on the first engaged hop" is never done for it and the
  published bar label stays the tracker's own until the next count' edge (<= 1 beat; Auto only, needs the tracker within
  |delta| of a wrap on the first engaged hop). Same class as A1's "may repeat once"; it is A1 as written (the held hop cannot
  re-seat). Worth one sentence in the architect's reply to S-1..S-8, no code change asked.

[NIT-5] BeatNudge.h's `#include <string>` / `nudgeAfterTempoCommand(const std::string&...)` (:2-3, :38) comes into the analysis TU
  through BeatShift.h:5, while T-N14 forbids "std::string" in BeatShift.* only by text. Harmless (never called on the hop;
  signature is PL:335's, ruled) but it makes the text gate narrower than its intent. No change asked.

## SLIM section (authoritative over the builder's SLIM CHECK)
- EXCESS_DEAD candidates: BeatNudge.h clampNudge, kNudgeMinMs / kNudgeMaxMs, nudgeAfterTempoCommand have no caller in S1.
  Disposition: JUSTIFIED_KEEP reason="ruling-nudge.md A8 names them as the one header of the nudge's numbers; the consumers are
  S2 (setBeatNudgeMs, A2's two statements) and the model tests T-U3 / T-M1..T-M3; deleting them now only re-adds them one stage
  later". Not under hooks/ or a config-referenced surface: no harness-wiring set applies.
- EXCESS_VESTIGIAL: none. BeatShift's public beatOf / writeBeat / sameBits / kBeatFields are all used inside apply / nonBeatCrc.
  The three word helpers (S-8) and the constructor's sign argument (S-7) are used by tests now and by S2 / T-N1.
- EXCESS_DUP: shiftBeats' own bpm guard (BeatNudge.h:30) duplicates apply's hasTempo (BeatShift.cpp:191) -- apply needs its own for
  the "target is S, copied whole" rule, so not removable; DEBT_FILED not needed.
- No test file content was cut or proposed for REMOVED.

## Real-time lens (Sacred Rules 1-4) -- VERIFIED
apply / the helpers are noexcept, plain-value state, std::clamp / bit_cast / floor only; no allocation, lock, system call or juce
include; adoptCounters writes three integers on the analysis thread.

## Not re-run by me (not permitted): the RED / GREEN, mutant and golden lines are the builder's report lines; this review verifies
that the cases and the harness are built so those lines can occur and can fail. MUST count: 0.
SUMMARY: 11 files, 7 findings (0 blocking, 2 SHOULD, 5 NIT); confidence VERIFIED for the rule-vs-code match and the fence,
INFERRED for NIT-4 and the SHOULD-2 identity.
METADATA: reviewer=reviewer, packet=nudge S1 arithmetic-tests r1, date=2026-10-04
