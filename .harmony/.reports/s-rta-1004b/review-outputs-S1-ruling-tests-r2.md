# Reviewer Verdict -- outputs S1, lens ruling-tests, round 2
STATUS: DONE
VERDICT: PASS (no MUST; round-1 SHOULD-2 fixed as found; SHOULD-1 is Harmony's to rule, correctly not built)
REVIEWED: lane/outputs-core head aa7bc8e (code head 0f4c73c), base 8b464a6; git objects only; nothing built or run.
FILES: fix round a364bad..aa7bc8e = outputs-S1-mutants.py, outputs-S1-mutants-r1.log (new), outputs-S1.md. Nothing else.

## Round-1 items
- MUST: none in round 1 (VERIFIED, r1 report).
- SHOULD-2 (runner rc 0 with M-P2 not RED): FIXED. outputs-S1-mutants.py:11-14,203-210 -- exit 1 if any arm not as recorded, exit 3 if as recorded but an arm that ran did not bite, exit 0 only when all ran arms are RED. Logic traced: M-P2 in EQUIVALENT, hit=False -> good=True (ok stays) -> not_red -> exit 3; M-P2x not in EQUIVALENT, hit=True -> exit 0. Log outputs-S1-mutants-r1.log has the RED arm (rc=0 before) and GREEN arms (M-P2 alone rc=3, M-P2x alone rc=0, all 22 rc=3) as raw lines (VERIFIED, read). Real fix, not documented away.
- SHOULD-1 (U-P2's ruled arm M-P2 cannot bite): not fixed, correctly: the arm is Harmony's text (ruling section 5). Report stop item 2 states it with the substitute M-P2x and what the runner needs on a ruling (VERIFIED, outputs-S1.md fix-round section). Still open for Harmony; not a MUST (the case can fail: M-P2x turns U-P2 RED).
- NIT-1 ("exactly its named case RED"): stop item 3, Harmony's wording. Open, accepted.
- Other r1 nits unchanged and not asked.

## Fix-round checks
- Code untouched: `git diff 0f4c73c aa7bc8e -- src tests CMakeLists.txt docs` empty (VERIFIED). The fence (8 Owns files) therefore still holds; the three report files are under .harmony/.reports/s-rta-1004b/ only.
- New defect in the fix: none found. The runner still works on a copy (build_and_run rmtree/copies only <scratch>/<name>, line ~118-124); no worktree write, no app start or kill (grep for kill/pkill: none). CLEAN arms and TSAN arm logic unchanged.
- No stray mutant, instrumentation, .venv link; no on-screen text added (code unchanged).
- Report's end checks: 1272 of 1272 (= 1252 + 20), 20 of 20, 10 of 10 TSan, 90 assertions in 8 GL cases -- raw lines in the report; INFERRED consistent (not re-run: read-only brief).
- Hand-over refreshed: row 3 now states the exit-3 line and says read per-mutant lines, not rc alone (VERIFIED).

## Findings
SHOULD-1 (carried, Harmony): rule M-P2x as U-P2's arm in ruling section 5; then delete M-P2 from the runner's MUTANTS/EQUIVALENT (one edit) so the full run exits 0.
NIT-A (carried): stop items 1 (owner of frameClockUs, only a comment at src/output/FrameHistory.h:97) and 4 (.harmony/probe-tsan-unit.sh TARGETS / EXPECTED_TSAN_CASES) are scope rulings for Harmony before S2/oa_tsan; correctly not built in S1.

## SLIM
No change from r1: EnsureResult::operator bool JUSTIFIED_KEEP (bridge for SharedFrameSet.cpp:19, S2's file); S1 pure-half symbols unreferenced by product code JUSTIFIED_KEEP (consumers are S2/S3/S5). No test content removed.

SUMMARY: 3 files in the fix round, 0 blocking, 1 should-fix (Harmony ruling), 1 nit carried. Confidence: VERIFIED by reading the diff, runner, log; INFERRED for the green counts (not re-run).
METADATA: reviewer=Reviewer, builder_packet=outputs-S1, round=2, date=2026-10-04
