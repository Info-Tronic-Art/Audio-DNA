# Reviewer Verdict -- outputs S1 history-pool lens, round 2
STATUS: DONE
VERDICT: PASS_WITH_NITS (REVIEW VERDICT: APPROVE; 0 blocking, 0 new findings, 4 round-1 items still open for Harmony)
REVIEWED: lane/outputs-core @ aa7bc8e (code head 0f4c73c) vs a364bad and 8b464a6, git objects only; nothing built or run.

ROUND-1 MUSTs: none existed (r1 = 0 MUST, 3 SHOULD, 3 NIT). Nothing to be "documented away".
FIX ROUND SCOPE (VERIFIED, git diff --stat a364bad..aa7bc8e and 0f4c73c..aa7bc8e): 3 files, all under .harmony/.reports/s-rta-1004b/
 (outputs-S1.md, outputs-S1-mutants.py, outputs-S1-mutants-r1.log). No file under src/ or tests/ changed since the code head 0f4c73c,
 so my whole r1 read of FrameLog / pick / extraSlots / reachMs / DepthPolicy / SurfacePool / OutputLook stands unchanged (no re-read needed).
 Whole-lane fence (git diff --name-only 8b464a6..aa7bc8e): only src/output/{FrameHistory.h,OutputLook.h,SharedFrameSet.h,SurfacePool.cpp},
 the S1 tests + tests/CMakeLists.txt, and .harmony/.reports. No venv link (git ls-files grep), no mutant, no on-screen text. VERIFIED.

THE ONE CHANGE: the mutant runner's exit code (outputs-S1-mutants.py, main(), last lines).
 - Before: `return 0 if ok else 1`, with M-P2 (equivalent) counted as "as recorded" -> rc=0 although an arm did not bite.
 - After: rc 1 = a result the report does not record (CLEAN not green, build error, arm flipped); rc 3 = as recorded but an arm that ran
   was NOT RED (prints "EXIT 3: ..."); rc 0 only when every arm that ran is RED; rc 2 = usage. Logic read: `ok` still folds in
   `good = hit != (name in EQUIVALENT)`, `not_red` collects every non-RED verdict, so M-P2 alone -> 3, M-P2x alone -> 0, all 22 -> 3,
   an EQUIVALENT arm that starts to bite -> `good` False -> 1. Correct. VERIFIED (read).
 - RED -> GREEN evidence is in the r1 log (M-P2 alone rc=3, M-P2x alone rc=0, all 22 rc=3, tail of log read) and the report records the
   RED arm (a364bad runner, M-P2 alone, rc=0). Not re-run (task forbids). The exit-1 path is unedited and not re-driven (the report
   says so; it is the old return). Accepted.
 - Runner safety re-checked: no kill / pkill / launch; rmtree only on `root` under the given scratch dir; reads the shared Catch2 source
   read-only; no path under Boris's folders. VERIFIED (grep of the file at aa7bc8e).

ISSUES
[NIT] The full runner list now can never exit 0 (M-P2 stays in the table, rc=3). Disclosed in the report's gate row 3 ("read the
 per-mutant lines, never the exit code alone"). A gate script that treats non-zero as FAIL will fail the full list until Harmony rules
 stop item 2 (M-P2x as the row's arm). Not a defect: honest, and the report says so. VERIFIED (report :295-310 region).

OPEN FROM r1, correctly NOT built by the builder because they are Harmony's rulings (checked against the tree, all still TRUE):
 1. frameClockUs() has no owner: `git grep -n frameClockUs aa7bc8e -- src tests` = only the comment at src/output/FrameHistory.h:97. Must be
    ruled before S2 (add to S1, or add FrameHistory.h to S2's Owns). S2's brief now says "do not add it outside your Owns without a ruling".
 2. U-P2's RED arm M-P2 is an equivalent mutant; M-P2x is the biting arm.
 3. "Exactly its named case RED" wording vs 9 arms that also redden a second case.
 4. .harmony/.reports probe .harmony/probe-tsan-unit.sh does not list test_frame_history; builder's edit proposal (TARGETS + 5 -> 15) is
    INFERRED, not run. Not in any stage's Owns. (The 10 cases are still under the tsan label via tests/CMakeLists.txt.)
 These are scope/wording rulings, not defects in the stage's code; the builder's refusal to edit outside the fence is right.
 Serial-wrap hint for S2 (r1 NIT) is now in the S2 hand-over text. VERIFIED.

NEW DEFECT FROM THE FIX: none. The fix is confined to a report-side script and prose; the app is unchanged by construction (no src/tests diff).

CONFIDENCE: VERIFIED for the diff scope, the runner logic, the fence and the stray checks (read at aa7bc8e). INFERRED: the unchanged app
 behaviour (no run), and that the r1 log's rc lines are genuine (Harmony re-witnesses by running the runner).
METADATA: reviewer=independent-source-review lens history-pool r2, builder_packet=outputs-S1, date=2026-10-04
