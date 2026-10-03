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
H-6 (2026-10-03 13:54:41; M1 stop item) .harmony/probe-ui-files-rename.sh rows R3, R4a, R9a expect "builds +1" (a tab-row rebuild) on a
    deck switch. That clause encodes the PRE-bf9b switch. Boris (verbatim): "when I switch between decks, do not change the
    clips playing in the layers or how they are playing" -- on the lane a switch between same-shape decks is showDeck ->
    refresh, no rebuild. M3: (1) run the probe UNCHANGED on the lane app first and paste the raw lines (expected: only the
    build-count clause of R3 / R4a / R9a differs; anything else that fails is a real finding -> STOP item); (2) change ONLY
    the build-count clause of those rows to "+0" for same-shape decks, keep every other clause (active deck, editor closed /
    open, the box stays on its deck over its tab), put Boris's quote at each changed row; (3) ADD a positive control so the
    counter cannot be dead: one row where an action that legitimately rebuilds the tab row reads "+1" (name the driver that
    exists: a rename commit, add / duplicate / remove deck); (4) re-run GREEN on the lane app, and show the changed rows
    RED on the pre-merge main app (it reads +1). A switch between decks with DIFFERENT column counts still rebuilds the
    grid (live-r2 NIT-1): add no row for it -- it is under plan review (P6).
H-7 (2026-10-03 14:44:55; M2 stop item 1) The 15 ARCHIVED evidence scripts under .harmony/.reports/** that still quit or kill Audio-DNA by
    name (21 lines; pkill / killall / unguarded osascript quit; the lock.sh copies of s-rta-0927 .. s-rta-0930): they are
    records, but handoffs point at those folders as templates, so a copy-and-run would quit Boris's app. NEUTRALISE, do not
    rewrite: each such script gets ONE inserted line right after its first line that prints "ARCHIVED RECORD (R-N1,
    s-rta-1003): this script quits Audio-DNA by name -- never run or source it; use .harmony/probe-quit-ours.sh" to stderr
    and stops (exit 64 for a run script; return 64 2>/dev/null || exit 64 for a sourced helper). Nothing else in them
    changes. NOT neutralised: a helper whose by-name quit sits under the only-ours check (the s-rta-1002b and s-rta-1003
    wf/lock.sh, in use today). Built in FIX-4; proof = the by-name grep over .harmony/.reports lists every remaining hit
    with either the neutraliser above it or the only-ours guard.
H-8 (2026-10-03 14:44:55; M2 stop item 2) CONFIRMED: quit_ours keeps its by-name branch untouched; the added "no pid recorded ... nothing is
    quit" line and the new ask_ours_to_quit (a second by-name line under the same only-ours condition) are within H-3 and
    within gate G-1's allowed class. The gates reviewer reads both blocks.
H-9 (2026-10-03 14:44:55; M2 stop item 3) CONFIRMED: probe-ui-files-rename.sh UIFR_ATTACH=1 refuses unless UIFR_ATTACH_PID, or LOCK_LIB +
    LANE, names the running test-mode app. Harmony's gate runs pass one.
