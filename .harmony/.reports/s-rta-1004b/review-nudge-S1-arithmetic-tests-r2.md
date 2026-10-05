# Reviewer Verdict - nudge S1 (THE ARITHMETIC), lens arithmetic-tests, round 2
STATUS: DONE
VERDICT: APPROVE (0 MUST, 0 SHOULD, 2 NIT) -- structured verdict PASS_WITH_NITS
REVIEWED: lane/nudge head a8afcfb (fix round c894851..a8afcfb), base 8b464a6; read through git objects only; nothing built or run.

## Round-1 findings (r1 had 0 MUST, 2 SHOULD, 5 NIT)
- SHOULD-1 (lag -1 side unreached): FIXED as found, not documented away. VERIFIED: Rig::Arm::EarlySoft (test_beat_shift.cpp, Rig::autoHop):
  conf 0.3 onset fed when beatPhase + 4 hops >= 1, below realign threshold 0.5; truth indexed count+1; softFed reset on the wrap. Added to T-N5
  and T-N5b (all D values, assertions (a)-(e) unchanged) + a precondition leadHops >= 190 so the arm must really put the bar position ahead
  of the count. Harness mutant R1a replaces "- anchor_, -1, 1);" by "- anchor_, 0, 1);" (BeatShift.cpp:122; anchor occurs once, VERIFIED by
  git grep). Builder's raw lines: SURVIVED all 22 before, RED on T-N5 and T-N5b after (builder-measured, not re-run by me).
- SHOULD-2 (formulas only checked against BeatShift's own functions): FIXED. VERIFIED: Rig::publish compares, on every hop with S.bpm > 0,
  BeatShift::barPhaseOf / phrasePhaseOf fed the tracker's own S fields with the tracker's own S.barPhase / S.phrasePhase by memcmp; counters
  asserted == 0 in T-N4 (Manual) and T-N5 (Auto, three arms). Mutants R1b (divisor kBeatsPerBar+1, BeatShift.cpp:71) and R1c (bar index +1,
  :83): anchors occur once each (git grep). Builder's lines: SURVIVED before, RED on T-N4 and T-N5 after. Builder also measured 0 mismatches
  over 867445 hops in a scratch copy (INFERRED for me: not run).
- NIT-1 (harness outside file list; hand-copied compile line): compile line now READ from the build dir's flags.make + link.txt
  (compile_line(), harness ~:152-178); docstring "six source files". VERIFIED by reading: objects precede "-o" in link.txt, so link[-o+2:]
  are libraries only (no duplicate-symbol risk); one -I<worktree>/src swapped for the copy, else "DID NOT COMPILE". No kill/pkill line;
  cache_value / catch_src removed with no dangling use (grep). The harness's home stays Harmony's call (builder's R1-S3).
- NIT-2..NIT-5: untouched, still as in r1 (no change asked). NIT-4 (hold on first engaged hop skips the re-seat) is routed by the
  builder as stop item R1-S1: it is A1 as written; a SHOULD/NIT, not a MUST, so routing it is correct.

## Fix introduced no new defect (VERIFIED by reading)
- Diff c894851..a8afcfb touches only tests/test_beat_shift.cpp, .harmony/nudge-mutants-s1.py and the stage report. src, both CMake files and
  the golden are byte-unchanged (git diff --stat empty): the reviewed rule and golden are intact; no .venv, no instrumentation, no UI text.
- EarlySoft cannot confuse the other arms: softFed only used in that arm; the LATE and EARLY branches are unchanged apart from the arm
  dispatch. Oracle uses only the tracker's own fields and sits before apply(), so it cannot mask a BeatShift defect.
- Fence: nothing outside the S1 file list besides the harness (r1 NIT-1) and the report.

## NITs (new)
[NIT-6] `CHECK(r.oracleHops >= w.hops)` (T-N4 / T-N5) is a weak sanity bound (true even if some hops skip the oracle); the real guard is
  `trackerBarPhaseBad == 0` with oracleHops > 0. No change needed.
[NIT-7] T-N5b's EarlySoft arm has no leadHops precondition (T-N5's has one); R1a is nonetheless RED on both, per the builder's lines.

## SLIM: no new excess. nameOf replaced three inline ternaries; cache_value was removed with its only caller. No test content cut.
## Not re-run by me (not permitted): RED/GREEN, mutant, golden and ctest lines are the builder's (MEASURED by him).
SUMMARY: 3 files in the fix diff, 0 blocking; confidence VERIFIED for fix-vs-finding match and fence, INFERRED for the green/RED lines.
METADATA: reviewer=reviewer, packet=nudge S1 arithmetic-tests r2, date=2026-10-04
