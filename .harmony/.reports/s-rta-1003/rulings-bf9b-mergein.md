# Harmony rulings for the bf9b merge-in lane (s-rta-1003, 2026-10-03 13:36:16) — on top of .harmony/.reports/s-rta-1002b/rulings-bf9b-merge.md (R-S1 / R-S2 / R-S3 / R-N1 / R-N3 stay binding)
H-1 MERGE-IN, not a history rewrite: main is merged INTO lane/bf9b (one merge commit). The 49 reviewed lane commits keep their
    SHAs (the r1 / r2 reviews stay pinned to real commits); Harmony's final merge to main is then conflict-free.
H-2 Stage order in the one worktree: M1 merge-in + R-S3 -> M2 by-name-quit sweep (R-N1) -> M3 MilkDrop deck-switch row (R-S1 / H1-H3)
    + post-merge probe re-runs. The round-2 review items that need a design (use-after-free read left by the M1 fix, Undo of
    Add / Load / Duplicate Deck, the untested wiring line, K5 Link-on, the S2b 4.B omissions, the grid NITs) and Boris's BF14
    (strip badge removed) go through plan -> blind council -> ruling (.harmony/.reports/s-rta-1003/plan-bf9b-merge.md,
    ruling-bf9b-merge.md) and are built in a FIX stage after M3. One pinned review round covers a7491d4..final head.
H-3 state-r2 NIT 6 (probe-quit-ours: "only OURPID running" then quit by name): NO CHANGE. Reason: with exactly one Audio-DNA
    process alive and it is ours, a by-name Apple-event quit can only reach that process; a second instance cannot appear in
    the window by a normal launch (same bundle id activates the running one). A SIGTERM would skip the app's own teardown.
H-4 gates-r2 NIT 6 (record_ourpid after the 60 s health loop) and NIT 4 (k1b_duplicate not a registered row): FIX in M2.
H-5 RED arm for every post-merge row = the PRE-MERGE main app (main 5abdf01's build, read-only), not the old STAGE_P copy.
    A row whose RED pattern differs from the lane report's STAGE_P table is listed with the reason, never re-thresholded.
