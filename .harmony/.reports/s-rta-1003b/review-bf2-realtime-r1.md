# Reviewer Verdict — bf2 realtime r1 (lens: realtime; head 101dcab)
STATUS: DONE
VERDICT: APPROVE (no MUST; 1 SHOULD, 3 NIT)
FILES: src/analysis/{AnalysisThread.cpp,BeatLead.cpp,BeatLead.h,SyncWitness.h}, src/api/ApiServer.cpp, tests/{CMakeLists.txt,test_beat_lead.cpp,test_recorder_host.cpp,test_sync_witness.cpp,test_analysis_sync_thread.cpp}, docs/claude/{analysis,integration,recording}.md, .harmony/{probe-sync.py (G6 only),probe-sync.sh,probe-tsan-unit.sh,APP-INVENTORY.md}
METADATA: reviewer=reviewer, packet=review-bf2-realtime-r1, base=c45b579, head=101dcab, read via git objects only; nothing built or run

## Answer
No realtime defect. The diff adds nothing to the audio callback, no allocation / mutex / syscall to the analysis steady
state, and the D2 witness field costs one float store inside a TEST-SERVER-only block. D5 is built as Appendix A rule 2
(VERIFIED by induction below); T-L6b pins it and can fail. Everything changed sits inside D2, D5, D6, D7, D8, D22.

## Realtime read (sacred rules 1-4; Pitfalls 29 30 32 42 48 63)
- Audio callback: untouched (no file under src/audio in the diff). VERIFIED (git diff --stat).
- D2 pipelineUs. VERIFIED AnalysisThread.cpp:468-469,570: no NEW clock read. `elapsedUs` is the existing
  high_resolution_clock delta pipelineStart(:211)..after publishWrite(:468) that already feeds cpuLoad_; the new line
  is `w.pipelineUs = static_cast<float>(elapsedUs)` inside the existing `#if AUDIODNA_TEST_SERVER` witness block
  (:526-573), written AFTER the timed window, so it cannot inflate its own measurement. Stored in SyncHopEntry
  (SyncWitness.h:134-137) -- the house way (explicit pad2_, sizeof 152 pinned by STATIC_REQUIRE in test_sync_witness,
  WitnessRing static_asserts trivially-copyable and whole-32-bit-word still hold). Production build byte-for-byte
  unaffected (inside #if). Per-hop cost: one 4-byte store + 8 more bytes in the existing seqlock memcpy. Negligible.
  No new FeatureSnapshot field (FeatureSnapshot.h static_asserts untouched).
- D5 BeatLead origin. VERIFIED BeatLead.cpp:194-199. Counters: beatX_/barX_/originX_ only grow; published
  totalBeat/BarCount: the absorb-and-clamp at :183-193 is untouched, so never decrease. Invariant originX == barX on every
  hop, by induction: both 0 at start / after disengage (engaged_ needs both 0, :230); barX_ += absorbed and originX_ is
  either `+= absorbed` or `= barX_` -- both give barX_new. Hence published O - tracker O == barX, so
  barsSinceResync() = T' - O is the led view's own count (consumers fold barsSinceResync(): OscillatorSignal.h:58,
  EnvelopeSignal.h:57, ConnectionEngine.cpp:102 -- VERIFIED by grep). Shape enumeration 1/2/4/8 bar: all fold
  4*bsr + beatInBar + beatPhase; at the un-crossing re-base bsr drops by exactly 1 and the fold steps by the tracker's own
  step (-0.296), not +3.7; no bar is left ahead of the Resync origin. Resync is always a re-base (BPMTracker.cpp:573
  ++frameEpoch_), so "never decreases on a non-re-base, non-Resync hop" holds (hold republishes F; origin fixed between
  re-bases).
- Pitfall 32: downbeatDetected handling untouched (simulate() and writeBeat unchanged; T-L7 stays in the suite).
  Pitfalls 29/30/42/48 and 63: no code in the diff touches them; no shared model field added (the witness is a lock-free
  seqlock ring, not a model field).
- D7 / D8 / D6: test + CMake only (no src). D8: tag [timing] is on the case (test_analysis_sync_thread.cpp:553), spec
  "~[tsan]~[timing]" + "[timing]" RUN_SERIAL is a partition, no case is dropped or duplicated; pair margins printed
  on every run (:678); bars 0.7/1.3 untouched (H-8 conformant).

## Findings
1. [SHOULD] EXCESS_VESTIGIAL + stale claim, src/analysis/BeatLead.cpp:194-199 and BeatLead.h:35 (VERIFIED by reading).
   After D5 the branch `if (flags.resyncs != lastResyncs_) originX_ = barX_; else originX_ += absorbed;` yields the same
   state in both arms (originX == barX before, so `+= absorbed` == `= barX_` after `barX_ += absorbed`). originX_, Diag.originX,
   lastResyncs_ and Flags::resyncs (header comment ":35 a change = a MANUAL Resync (the origin moves)") therefore carry
   no information; the code comment ":194 a manual Resync re-anchors the origin" tells a reader the Resync path differs
   when it does not. D5 text says the origin "grows by the absorbed bars and nothing else"; the Resync arm contradicts
   that in words only. Not a behaviour bug. Disposition: DEBT_FILED (fix: delete the branch, set originX_ = barX_ or drop
   originX_/Flags::resyncs/lastResyncs_ and the AnalysisThread.cpp:457 argument together; keep Diag.originX only if tests
   still assert it). Harness-wiring check for the EXCESS claim: Flags::resyncs is read only at BeatLead.cpp:194 and set at
   AnalysisThread.cpp:457 and test_beat_lead.cpp:138; no config / hook surface involved. Minimum now: reword the two comments.
2. [NIT] test coverage shape, tests/test_beat_lead.cpp T-L6b: one scenario (manual 120 BPM, 0.4 s lead, Tap un-crossing a
   bar). It pins the step for every multi-bar shape only because they all fold 4*bsr+b+p (VERIFIED, test's own shapeFold).
   It would also fail if the origin shift were deleted (bsr would not drop: out.bsr+1==K) and on the capped code (lane report:
   +3.704 RED, INFERRED from the report, not re-run). No assertion bounds a forward leap on the OTHER re-base kinds (regime
   change, lock lost); T-L3 (v) covers the origin identity there, not the step. Acceptable for this ruling.
3. [NIT] G6 wording, docs/claude/integration.md witness paragraph: "the value behind the DSP-load readout" is exact, but in a
   TEST-SERVER build that window also contains two BeatLead::nonBeatCrc passes over the 384-byte snapshot
   (AnalysisThread.cpp:450-462), so G6's absolute mean slightly over-states production. Conservative for bar (2), and the
   -500 minus 0 INFO difference cancels it. No action beyond a clause in the G6 report.
4. [NIT] stray: lane report (S3f FOUND) says build-mut-r7/ is untracked and not in .gitignore; it is not in the diff
   (VERIFIED: diff --stat lists no build dir) but a `git add -A` would sweep it. Add to .gitignore or delete after Harmony's R7 run.

## Ruling conformance, my lens (each VERIFIED by reading unless marked)
- D2: field + store + REST property (ApiServer.cpp:2954) + 5th test_sync_witness case + EXPECTED_TSAN_CASES 12; probe G6 reads
  pipelineUs, bar (1) leadApplied >= 99 % of -500 window, bar (2) mean <= 2,000 per window, fallback relative bars as ruled,
  windows >= 150 hops, absent-field => FAIL (probe-sync.py g6_verdict). No perf script written (correct per D2).
- D5: cap and helper removed; T-L3 (v) and T-L6 re-stated; T-L6b new, RED-first per lane report (INFERRED, not re-run).
- D6: T-L3 prints qualifying count and CHECKs >= half of non-re-base hops (test_beat_lead.cpp ~:568); lane figure 50728/51161.
- D7: new [host][sync][end] case asserts one early fire, then exactly three tail fires in stamp/seq order, finished once, a
  second end tick fires nothing; recording.md carries D7's words + FU-1. Mutant RED is the lane report's claim (INFERRED).
- D8: as above. D22 sentences present in analysis.md / recording.md / integration.md; CLAUDE.md untouched (not in diff).
- Outside D1..D8: nothing found. APP-INVENTORY / probe-tsan-unit / probe-sync.sh edits are bookkeeping for D2/D3/D4/D6.
- Scripts that could quit a foreign app: probe-sync.sh diff touches only the header comment, the row default list and the
  --rows default; no quit/kill path changed (grep of probe-sync.py for kill/quit/pkill/osascript: no hit).

## Not verified (cannot run, per instructions)
No build, test or probe executed; every RED/GREEN, mutant and live-probe number above is the lane report's, not mine.
Confidence: VERIFIED for source reading and the origin-invariant induction; INFERRED for all executed results.
