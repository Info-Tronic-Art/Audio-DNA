# 7-ITEM GESTURE REPLAY — BORIS RESULTS (2026-08-04)

**Replayed live by Boris on a Release binary rebuilt from HEAD `20fc53d` (0 errors), app
running with `--test-mode`, output window never opened.** First hands-on replay since the list
was written 2026-07-30 — it survived 6 handoffs unprocessed because each one referenced the
previous revision instead of carrying it.

| # | Item | Result |
|---|---|---|
| a | Drag effect onto layer channel strip (single + multi), one Cmd+Z restores | **PASS** |
| b | Finder-drop multiple files together, one Cmd+Z removes all | **PASS as written** — but a NEW image-specific bug found, see below |
| c | Cmd+X with cell selected / with nothing selected | **PASS** |
| d | Click a playing video cell -> restarts from in-point | **PASS** |
| e | Header-drag "Energetic (9)" -> MilkDrop Playlist, 3 knobs live | **BLOCKED — untestable** |
| f | Autopilot over a SOURCE cell -> advances off it | **PASS** |
| g | Genre auto-switch to empty deck -> preview blank, no ghost clip | **NOT TESTED — feature not locatable** |

## b — ANOMALY (open, needs disk verification before any fix)
Boris dropped **2 images + 1 video** (note: NOT the 2 videos + 1 image the row specifies — the
row was never run as written). Observed: **the 2 images landed on the SAME cell**, the video on
the next cell. Expected: three files -> three cells.

- **RESOLVED by Boris (2026-08-04): the two images STACK/COEXIST — the second does NOT replace
  the first, so there is NO data loss. And ONE Cmd+Z removed ALL of it — undo granularity is
  correct (single transaction for the whole multi-file drop).**
- **Severity accordingly DOWNGRADED: placement defect, not data loss. Queue it; it does not
  jump the preset lane.**
- **BORIS THEN RAN THE ROW AS WRITTEN (2 videos + 1 image): all three landed on separate
  cells, undo fine. ROW b PASSES AS SPECIFIED.**
- **CONTROLLED COMPARISON — the defect is IMAGE-SPECIFIC.** Boris ran both variants back to
  back, isolating the variable himself:
  | drop | result |
  |---|---|
  | 2 images + 1 video | two images share ONE cell — ANOMALY |
  | 2 videos + 1 image | three separate cells — WORKS |
  Videos advance the destination cursor correctly; a second image does not get its own cell.
  This is a NEW bug, discovered by Boris deviating from the written row — the written row
  would never have found it.
- **OPEN QUESTION, product not engineering:** two images sharing one cell may be INTENDED
  (multi-image-as-sequence/slideshow in a single cell is a normal VJ idiom). Boris reported it
  as merely unexpected, not as broken. Confirm intent BEFORE treating it as a bug — recon will
  say what the code intends, but only Boris can say what it SHOULD do.
- Unknown: whether this is image-specific or a general multi-file cell-allocation bug. The row
  as originally written (2 videos + 1 image) has still never been executed, so a type-dependent
  bug is not ruled out.
- **Do NOT dispatch a fix until the drop path is read on disk.** Standing rule for this repo.

## e — BLOCKED, not a failure
**No MilkDrop presets are loaded by default**, so the "Energetic (9)" header-drag could not be
exercised at all. This is an availability/config problem, not a gesture defect. The row stays
OPEN — it has never been verified either way. Do not close it as PASS.

## g — feature not locatable by its owner
Boris asked "where is genre auto switch function?" — he could not find the affordance in the UI.
Two possibilities, both open: the feature is not reachable from the UI, or it is reachable but
undiscoverable. **Either way the row is untested, and an owner who cannot find his own feature
is itself a finding worth recording.**

## Prediction scorecard (kept honest)
Harmony predicted f would PASS with preview attached but still FREEZE with preview detached.
**Half-tested: f passed as run. The detached condition was NOT exercised**, so the prediction is
neither confirmed nor refuted. It must not be recorded as confirmed.
