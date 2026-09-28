# Session s-rta-0928 — secondary (MINIMAL, foreign repo), 2026-09-28 08:16 → ~15:45

role: secondary · profile: MINIMAL · repo: ~/projects/RealTimeAudio · main loop: Opus 5.5 (1M) · ultracode (workflows)
boot HEAD: 6db8d67 (code 233eae7) · close HEAD: see git log (all pushed) · running log: .harmony/s-rta-0928-work.md
reports/evidence: .harmony/.reports/s-rta-0928/ (plans + adoptions, attack papers, diag reports, lane reports, Boris page,
restore-tools/, gate-scripts/)

## Rows
| t | kind | item | result |
|---|---|---|---|
| 08:29 | gate | tempo witness x20 baseline on main | RED: start bpm 0 4/20, unknown grid 5/20 |
| 08:45 | deviation | Fable quota exhausted ("You've reached your Fable limit") | all 4 plans re-pinned to OPUS max (s66 precedent); recorded; Boris told |
| 09:05 | diag | tier1 residual + test_fractals | 30 lines binned; test_fractals 113F x5 identical, 81 = its own metric |
| 09:12 | diag | routine restore hold | thumbnail decode per DeckView::refresh (99.6-99.9 %); stall skips a gesture's write |
| 09:35-10:08 | plan | tempo / renderleft / restore / tier1 | each attacked by 2 blind seats; Harmony adoption rulings appended to each plan |
| 10:34 | merge | lane/tempo -> 734f011 | reviews PASS_WITH_NITS x2; Harmony RED+GREEN; ctest 797 |
| 13:13 | merge | lane/tier1 -> 78b2c2a | reviews PASS_WITH_NITS x2 + critic PASS; RED 4F -> GREEN 93P/8xf; test_sources 0 lines; ctest 803 |
| 13:30 | merge | lane/restore -> 80e3e53 | reviews PASS_WITH_NITS x2; conflict = pitfall appends (NN -> 51); RED 105/4 -> GREEN 109/0; ctest 820 |
| 14:39 | merge | lane/renderleft -> 0ab3996 | r1 FAIL (3 MUST incl. lost Pitfalls 48-50) -> fix round -> r2 PASS + PASS_WITH_NITS; RED 11/15 + 3/6 -> GREEN 37/0 + 9/0; ctest 846 |
| 14:40-15:10 | gate | FINAL battery on main (21 probes) | all GREEN, 0 Output windows |
| 15:10- | gate | Tier-1 five files + test_fractals on final main | see handoff session section |

## Harmony errors (no gate would surface them)
1. Lock helper took the lane name from a `VAR=x . lib.sh` prefix (bash drops it after sourcing): blank owner, any lane could release another's lock. Fixed + notebook.
2. A `cd` in a read command (never-cd rule). Restored at once.
3. Rebuilt main after the restore merge BEFORE my own RED of its new rows on the pre-merge binary; recovered with the lane's saved pre-merge app copy. Habit: RED before cmake in every merge sequence (done for renderleft).

## Counts (run, not inherited)
ctest 846/846 on main after the last merge · battery 21/21 probes GREEN · unpushed 0 after the close commit
