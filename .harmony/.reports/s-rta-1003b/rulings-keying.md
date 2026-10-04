# HARMONY'S RULINGS — keying / blend / transition pixel audit (lane/keying-audit, head e22ef2d; written 2026-10-04 00:02:56)
Audit: .claude/worktrees/keying/.harmony/.reports/s-rta-1003b/keying-audit.md + .json + 10 contact sheets; method review r1
PASS_WITH_NITS (0 MUST, 5 SHOULD) -> fix round -> r2 PASS_WITH_NITS (0 MUST, 1 SHOULD). Verdict counts over 13 keying + 55
blend + 55 transition entries: WORKS 19, NO-OP 55, ALIAS 43, BROKEN 6, NOT-DETERMINED 0.
K-1 Transitions: the measured verdicts are ACCEPTED, labelled "measured on a moving 6 s fade" (a mid-fade instant cannot be
    frozen through REST; the fitted fade position stayed within 0.0063 of the clock over 330 frames). Not downgraded.
K-2 Screen, Multiply, Darken, Lighten: reported to Boris as "right only for an opaque picture at full opacity; they ignore
    the opacity slider and a picture's own transparency" -- proposal: FIX. Whether that is a defect is his call on the page.
K-3 Max RGB is WORKS (its own formula fits); "Luma Is Alpha" is the mislabelled duplicate. The fix round's alias rule
    (the entry whose own formula fits is the original, whatever its menu position) is accepted for both families.
K-4 All 55 transition entries were audited (15 shader programs behind them): accepted.
K-5 The worktree and its raw frames (97 MB, git-ignored) stay until Boris has seen the table; the JSON and sheets are committed.
K-6 The probe launches and quits its own pid through probe-quit-ours.sh: accepted.
NEXT: Harmony re-runs a sample herself and looks at a sheet; a works / fix / remove table for Boris (a page, with the
sheets); then a plan for whatever he keeps. Nothing in src changed.
