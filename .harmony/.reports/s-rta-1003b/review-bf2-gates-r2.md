# Reviewer Verdict - bf2 gates r2 (S3f fix round)
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) - no MUST in the gates lens
FILES: .harmony/probe-sync.py, .harmony/probe-sync-selftest.py (new), .gitignore, docs/claude/analysis.md, lane report bf2-delta.md (FIX-R1)
METADATA: reviewer=reviewer, head=68abc16d71f3825cb31fbbb0a8a00939a4ee63e6, fix range db950ab..68abc16, date=2026-10-03. Read through git objects only; nothing built, no probe or self-test run.

## Round-1 gates items (r1 had no MUST; four SHOULDs in my lens)
- r1 #3 .gitignore /build-mut-*/ - VERIFIED (diff). Right pattern; the mutant dir stays on disk.
- r1 #4 synthetic REDs committed as .harmony/probe-sync-selftest.py - VERIFIED tracked (ls-tree, 100644, blob f5041e5; .harmony is gitignored, so it was force-added). It imports the REAL probe by path (import-safe: main() under __main__, only constants at module level), and drives the real r7_trim / r7_verdict / g6_verdict / subset_tag, reading back the probe's own PASS/FAIL counters and printed lines. Not a restated copy. I hand-checked the expected PASS/FAIL counts of the G6 cases and the R7 case arithmetic against g6_verdict (probe-sync.py:~985-1031) and r7_verdict (:937-975): the stated counts match the code (e.g. 7 lines for the 6-window PASS case; INCONCLUSIVE case 2 PASS + 1 FAIL; 277/280 = 98.93 % < 99 % FAIL). Bars checked: G6 >= 150 hops, bar (1) 99 %, bar (2) 2,000 us, relative +300, neighbours < 100 us, R7 1.0 / 0.33 / n >= 100 / drops <= 4 / 21.333 ms all appear as in the ruling.
- r1 #4 "can it fail": INFERRED-from-record, not re-run (instruction: no runs). Lane log lists 9 mutated COPIES (G6_BUDGET_US, bar (1), the <150 test, the relative +300, R7_BAR_MS, R7_SE_LIMIT_MS, the INVALID thresholds, R7_TRIM_MS, subset_tag) each exiting 1 with named DIFF cases, probe sha256 unchanged, and the pre-fix probe failing with AttributeError. The committed probe is unmutated at the head (cadad16 blob; no mutant text).
- r1 #5 subset_tag - VERIFIED. R5 tag = subset_tag("R5", arms != [0,-100,0,-500,0] or seconds != 61.0): same condition as the old test; defaults (arms, 61 s) NOT changed (:770 vs the old :767). Every R5 verdict line (per-arm bars via early_hop_bars, lag bar, three hold lines, free-running count) and R7b's line now carry the tag; `subset` is in scope at :1205 (set :1083). Full-length lines unchanged. Tag is passed as a %s argument, never in a format string. Nothing in the repo greps these labels (git grep at the head, and main's .harmony/*.sh and scripts/: no hit), so no consumer broke.
- r1 #6 analysis.md - VERIFIED: states the observed readings (0.65-0.82, later 0.97-1.01), "cause not established". The claim is now labelled, not asserted.
- No threshold, bar, default arm list or default seconds changed in this round (VERIFIED by diff of probe-sync.py: label/tag edits plus subset_tag only). cb63bc8 -> 68abc16 is the lane report only (git diff --stat).
- Script safety: probe-sync.sh is NOT in the fix range; quit_ours / own-pid rig unchanged. The self-test starts no app. Nothing here can quit an app it did not start. testing-eyes.md now tells readers to quit only their own pid (dc59573), removing the by-name pkill recipe.

## Findings (no MUST)
1. NIT (INFERRED): after the tag change the R7 / R7b mutant-app RED was not re-run (the lane says so, and why: arithmetic unchanged, only the line's first words). The self-test's `tag` mutant covers the label; the live RED line text Harmony will see is "FAIL  R7b SUBSET (development: not a gate line) ..." as the hand-over states. Acceptable.
2. NIT (VERIFIED, lane's own "found not fixed"): .harmony/gotchas.md:122/:136 still carry by-name kill advice; out of this lens and lane, correctly filed.
3. NIT (VERIFIED): the self-test's G6 PASS case uses 3 R1 0 arms (six windows) not the gate's five (eight); it tests the function's logic, and the live row's 8 x G6(2) was already on record in r1. No action.

Out of lens (not judged here): the keys MUST (no door to Relative CC) is left open for Harmony's ruling per the lane report; it is not a gates matter.
