# Reviewer Verdict -- nudge S1, lens real-time, round 1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) -- 0 MUST, 2 SHOULD, 4 NIT
PINNED: lane/nudge head c894851, base 8b464a6. Read only through git objects. Nothing built or run by me.
Spec read: plan-nudge.md adoption blocks (2), ruling-nudge.md A1-A4, A8, A9, section 4 row S1, section 5; ruling-nudge-row.md
section 4 row S1 ("as RN, with kNudgePlusMeansLater = false and NR5-A's literals"): S1 is NOT amended; plan-nudge-row.md
adoption blocks (3): the newest holds S1r..VG, never touches S1.

## REAL-TIME LENS (VERIFIED by reading src/analysis/BeatShift.cpp:107-283 and BeatShift.h at c894851)
- apply() and everything it calls: stack scalars and std::clamp/min/max/floor only; no new/malloc/vector/string/mutex/
  atomic/printf/juce (git grep of the nine T-N14 tokens over BeatShift.h/.cpp: no hit). The class is plain value state;
  apply is noexcept. nonBeatCrc (384-byte stack copy) is only called from tests, never from apply.
- BPMTracker.h:126/128 are two inline const getters; BPMTracker.cpp:580-585 adoptCounters is three scalar stores
  (totalBeatCount_, totalBarCount_, resyncBarOrigin_ = totalBars), noexcept; nothing else in the tracker changed
  (diff: +7 lines cpp, +12 lines h, all additive). appliedResyncs() = resyncRequestsApplied_, written only at
  BPMTracker.cpp:360 (analysis thread) -- the comment "changes exactly when a manual Resync is applied" is true.
- No CAS loop exists in S1 (the word's setters are S2). git grep for atomic/compare_exchange in BeatShift: none.
- NOTHING WIRED (VERIFIED): git grep at c894851 over src for BeatShift::apply, beatShift_, adoptCounters, shiftBeats,
  kNudge*, clampNudge, nudgeAfterTempoCommand, appliedResyncs, predictedBeatRegime() outside BeatShift.h/.cpp shows only
  definitions + comments (BPMTracker.h:131 comment, BeatShift.h:18/34 comments). AnalysisThread, FeatureSnapshot,
  MainComponent, ApiServer untouched (diffstat). No on-screen text added.
- BPMTracker bit-identity: src is identical between 185147b and 8b464a6 (git diff --stat empty), the golden commit 3ba5ca1
  holds only tests/test_analysis_nudge_golden.cpp + tests/CMakeLists.txt (+48), and git diff 3ba5ca1 c894851 on the golden
  file is EMPTY (the four constants at test_analysis_nudge_golden.cpp:129-134 never move). "Golden green at the head, three
  runs" is the builder's MEASURED claim in nudge-S1.md; INFERRED by me (I ran nothing).

## RULE FIDELITY (A1/A2/A3 read line by line against BeatShift.cpp) -- no deviation found
Step 0 anchor/lag (:119-122), step 1 identity writes nothing (:129-136), step 2 restart + zero-request consumed on the first
applied Resync whether it restarts or not (:148-173), glide (:176-180), position exact in double with the 1.0f carry
(:190-207), HOLD with F's sequence (:211-218, A3), CAP (:220-224), step 6 re-seat at edge / first engaged hop / relabelDue
(:233-268), step 7 (:281). Sign: kNudgePlusMeansLater=false (BeatNudge.h:13); T-N1 writes the sign out independently
(test_beat_shift.cpp:37-38) so M1 can bite.

## TESTS / MUTANTS (VERIFIED by reading; counts INFERRED from the report)
- 22 TEST_CASEs in test_beat_shift.cpp + 2 golden = 24 (counted). Rig drives a REAL BPMTracker hop by hop and copies its
  nine fields into a snapshot with moving non-beat content (test_beat_shift.cpp:82-165): real code, not a model.
- The five places the builder says it could not build a row literally (S-1..S-5) are honestly named and each is built as
  the closest thing that can fail; none adds a rule. S-3 (M27 survived the ruled arm; extra arm added) shows the harness
  genuinely reports SURVIVED.
- nudge-mutants-s1.py works on copies under build-lane/mutants-s1/<M> only (rmtree at :149 is that dir); no kill, no
  write to the worktree sources or Boris's folders.
- Nothing stray: git status of the worktree is clean; no .venv, no mutant or instrumentation left in src.

## FINDINGS
SHOULD-1 (VERIFIED by reading, consequence INFERRED) BeatShift.cpp:137,216-217,239 -- a HOLD on the first engaged hop
  returns at :217 before step 6 and sets engaged_, so the next hop's firstEngagedHop is false and A1's "re-seated on the
  first engaged hop" never happens; the bar label stays the tracker's old one until the next count' edge (builder F-2). It
  is the ruling's own paper rule (the hold returns before step 6 there too), so not a MUST; one beat of a stale bar
  position after a LATER nudge in Auto. Route to the architect with FM-2 / S2's live rows; do not re-design in S1.
SHOULD-2 (INFERRED from the builder's MEASURED line, report F-3) in Auto with onsets trailing the wrap a small LATER nudge
  (D=-3) holds one hop on 199 of 200 beats: a 10.7 ms freeze of every beat field once per beat (render reads a repeated
  snapshot). Inside the ruled bar (e) (<= 4 hops), so allowed; record it for Boris's B-checks / FM-2.
NIT-1 tests/test_analysis_nudge_golden.cpp:185-190 -- an env-gated dump hook (ADNA_GOLDEN_DUMP, std::fopen) stays in a
  committed test; recording aid, writes only where the env points. Harmless; could be removed after G-N0 passes.
NIT-2 .harmony/nudge-mutants-s1.py:28-33,170-190 -- outside the stage's named file list; hard-codes /usr/bin/c++, -arch
  arm64 and /opt/homebrew paths. Needed to show "each mutant RED", so fine; machine-specific.
NIT-3 BeatShift.h:67 default argument reads kNudgePlusMeansLater in the header, while BeatNudge.h:10-11 says the sign is
  "read in exactly three places" (shiftBeats call, tooltips, labels). Comment is forward-looking; reword at S3/S5.
NIT-4 T-N14's forbidden list (test_beat_shift.cpp:1230-1233) is the ruling's list; it misses "throw", "std::function",
  "std::map", "std::atomic" (an atomic is legitimate in S2's thread, not in BeatShift). Optional hardening only.
STOP-ITEM STATUS (builder S-1..S-8): all are builder-side substitutions for rows that could not be literal in an unwired
  stage; I agree none is a re-design. S-1 (T-N14 counts 0 call sites, S2 must set kWiredCallSites=1 and name the member
  beatShift_) and F-6 (golden's ADNA_ANALYSIS_THREAD_SOURCES lacks BeatShift.cpp; S2 must add it) are carry-overs for S2's packet.

## SLIM
- BeatShift::apply, BPMTracker::adoptCounters, appliedResyncs/predictedBeatRegime views, BeatNudge.h clampNudge /
  nudgeAfterTempoCommand / kNudgeMin|Max: JUSTIFIED_KEEP reason="named by ruling-nudge.md A1/A2/A4/A8 and S1's row; dormant
  by design until S2 wires the one call (T-N14 pins it at 0 now); no hooks/ or config-referenced surface involved (harness
  wiring set not applicable)".
- BeatShift::nonBeatCrc + kCrc table, barPhaseOf/phrasePhaseOf, beatOf/writeBeat public: JUSTIFIED_KEEP reason="ruling A1
  'carried from BeatLead'; nonBeatCrc is the T-N9 'only the beat is written' oracle used by T-N4/T-N5/T-N8c; the two
  phase formulas are checked bit for bit by tests". nonBeatCrc is test-only code in the app binary (~1 KB table).
- No test-file deletion proposed.

SUMMARY: 11 files, 6 issues (0 blocking, 6 suggestions). Confidence: real-time claims and nothing-wired VERIFIED by
git grep / reading at c894851; green/red results are the builder's MEASURED lines, INFERRED by me.
METADATA: reviewer=sonnet-5-5, builder_packet=nudge-S1, date=2026-10-04
