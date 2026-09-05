# S166-GFX — The Global Effects stack is fully built and never rendered. Render it.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s166/builder-global-effects.md

## THE LANE IN ONE LINE
The Composition Inspector's "Global Effects" stack is a fully interactive effect
editor — add, reorder, bypass and undo all work, using the SAME widget as the Clip
and Layer effect stacks that DO render — but no renderer ever composites the global
tier, so nothing you put in it has any visible result.

## EVIDENCE FROM THIS SESSION'S RECON (re-verify; do not trust it blind)
- Full recon: `.harmony/.reports/s166/recon-timing-and-compinspector.md` — read it first.
- The Clip and Layer tiers of the identical mechanism DO render; the recon cites
  `src/render/CompositorEngine.cpp` around the apply-effects calls (it named lines 237
  and 756 — LINE NUMBERS DRIFT, re-grep by anchor text).
- The recon reports an existing code comment noting this exact seam was left open
  deliberately. **FIND THAT COMMENT AND READ IT BEFORE YOU BUILD.**

## THE FIRST THING YOU DO — AND YOU MAY STOP HERE
Establish WHY the global tier was left uncomposited. If there is a real reason —
ordering, a resolution/framebuffer mismatch, a per-frame cost, a missing render
target, an unresolved question about what "global" even means relative to layers —
then **report BLOCKED with that reason and do not build.** A seam left open on
purpose is a design decision, and overriding one silently is worse than leaving the
feature dead for another day. Only proceed if the reason is absent or clearly stale.

## WHAT DONE MEANS (if you proceed)
1. Effects added to the Global Effects stack visibly affect the FINAL composited
   output — after layers are composited, not per-layer.
2. Bypass works. Reordering works. An empty global stack costs nothing measurable
   and changes the output not at all (prove this: an empty stack must be a no-op).
3. Undo/redo behaves like the Clip and Layer tiers.
4. No new per-frame allocation on the GL thread, and no new cross-thread reads of
   model state that are not already made safely by the Clip/Layer paths.

## FENCE — you may edit ONLY these
- `src/render/CompositorEngine.cpp` / `.h`
- `src/render/Renderer.cpp` / `.h` **only if** the composite seam genuinely lives there
- a test file under `tests/`
Anything else: STOP and report. In particular do NOT touch the inspector UI — it is
already complete, and the owner has ruled the current UI will be replaced wholesale.

## BUILD AND TEST RULES — this repo's documented false-green traps
- **Capture the build exit code and READ it before quoting any test number.**
- ctest baseline is **222/222**, re-run on disk at s166 boot. Re-run; never inherit.
  Build dir `build`, Release.
- Do NOT launch the app; I run the behavioral gate myself. Build + ctest only.
- If you add a test, prove it load-bearing: neutralize the fix, rebuild, confirm the
  test FAILS, restore byte-identically (md5), confirm it passes. Report both numbers.

## REPORTING
STATUS (DONE / DONE_WITH_CONCERNS / BLOCKED), what you changed and why, the reason the
seam was open and whether it was still valid, build exit code, before/after ctest,
PACKET QUALITY, and anything outside the fence you noticed.
