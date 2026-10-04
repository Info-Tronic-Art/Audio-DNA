# Reviewer Verdict - bf2 S4b, lens gates, round 2
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) -- 0 MUST, 0 SHOULD, 3 NIT
PINNED: worktree .claude/worktrees/bf2keys, lane/bf2-keys, base 68abc16, head 9eab9bdbb7156e9c1a54bb6b5ef62ab3c56847dd. Read through git objects only.
FILES: .harmony/probe-sync-selftest.py (+183), .harmony/.reports/s-rta-1003b/bf2-s4b-selftest-mutants.py, bf2-s4b-unit-mutants.py, bf2-s4b.md;
 probe-sync.py / probe-sync.sh re-read at the head (row_r13 1244-1300, row_r14 1303-1368).

## Answer
Round-1 had 0 MUST; its two SHOULDs: gates SHOULD-1 (no offline R13 / R14 cases) is FIXED as asked (15 cases, 42 total); SHOULD-2 (the ML-1 bar) is
a Harmony stop item, correctly not touched by a builder. The fix round changed NO src / tests / docs / probe-sync.py / probe-sync.sh
(git diff d4653bd..head --stat on those paths is empty: VERIFIED), so rows R13 / R14 and their round-1 verdict are unchanged.
I did not build, run a test, run the self-test, launch an app or run a probe (instruction); the new cases are verified by hand-tracing
every stated count against the probe's real row code (VERIFIED by reading); the builder's live self-test / mutant runs are INFERRED from the report.

## Round-1 findings, as found
- SHOULD-1 FIXED, not documented away: probe-sync-selftest.py:112-290 runs the probe's REAL row_r13 / row_r14 (and set_sync / get_sync / wait_settled
  under them); only http() and the clock (KeysClock, a counted clock, restored in a finally) are replaced. 42 `^case(` = 27 + 15 VERIFIED (grep).
- Hand-trace of the stated counts against row_r13 / row_r14 (VERIFIED by reading, each matches): R13 flip (0,8, only step 3 passes: -1 then +1 = 0);
  nocall (0,6: steps 3, 8, 9 pass); release (0,6); late (0,6: the still step reads 200 ms after, the model nudge is due at +100 ms; a read at once
  would pass -> mutant K1 is real); noclamp (0,2); 404 (0,9). R14 dead (0,7: b 4 fields, d / e "not evaluated", f 1); binds (0,10, pass 5);
  strips (0,3); leaves (0,5); nolast (0,2); title (0,1); 404 (0,8, d / e not evaluated). The control arm is therefore exercised: a dead
  injection FAILS b and d / e print "not evaluated" with no PASS line.
- Each case can fail: `case()` demands exact PASS / FAIL counter deltas, text on a line of the stated kind, and a check on the returned row dict.
- bf2-s4b-selftest-mutants.py: 8 copies written to a temp dir; the committed probe is only read (sha256 re-checked); each old text must occur
  exactly once or the script exits 1 ("NOT APPLIED"); a copy must exit 1 AND DIFF the named case (names are prefixes of real case names: VERIFIED).
  K1 (no still wait), K2 (wrong targetMs not a failure), K3 (non-200 ignored), K4 (control arm gone), K5 (title not read), K6 (PASS printed always),
  K7 / K8 (other binding / Last not read) each map to a case I traced to differ. It spawns only the python self-test: it cannot quit an app.
- RED-before: the new cases against probe 68abc16 raise AttributeError (no row_r13) -- a stub RED only; acceptable, the rows are new in this stage.
- Real-app claims (R13 RED on the sign-flipped mutant app, R14 RED on the refused-learn mutant; rows GREEN on the lane app, R6 sha256 af6fc014... before ==
  after, 0 Output windows) are the builder's live lines (INFERRED); round 1 read the same code and the code is unchanged.

## Hygiene
- No MUTANT marker, no .venv link in the tree at the head (git grep / ls-tree: none; .venv appears only as the probe's default interpreter path). The
  worktree `git status --short` is clean. probe-sync.sh teardown is still quit_ours (own pid only); the Output-window and R6 settings.json hash checks unchanged.
- No on-screen text added (no src change this round). Docs: unchanged this round; round 1 verified A11.

## Findings
NIT-1 bf2-s4b-unit-mutants.py:84-111 edits tracked src (Binding.h, BindingManager.cpp) IN PLACE and reverts in a try/finally plus a sha256 re-check.
 An uncatchable kill (SIGKILL, power loss) would leave a mutant in the lane tree; the script's own MUTANT-marker count only reports it afterwards.
 Acceptable: the ruling's G1-RED is a builder-run mutation of the builder's own lane worktree; no fix needed beyond the existing marker check.
NIT-2 The self-test models are the builder's, not the app (stated by the builder as LIMIT); "ok" means "the row reads an app that behaves as ruled as PASS".
 The live row remains the only claim about the real app.
NIT-3 Merge note (builder's stop item 4): probe-sync.py conflicts in two both-added regions when lane/bf2-keys merges into lane/bf2; the self-test auto-merges
 (builder's trial merge, INFERRED). Harmony's merge must keep both blocks and re-run SELFTEST (expect D0's 65 + 15 = 80 ok lines).
