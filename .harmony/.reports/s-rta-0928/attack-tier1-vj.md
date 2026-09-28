# Attack: plan-tier1 (s-rta-0928) -- VJ / product-skeptic seat

VERDICT: SOUND_WITH_FIXES

## MUST -- Zoom is a stage-visible knob shipped broken; the gate is re-engineered to expect the breakage
Verified in code: mandelbrot's manual-mode zoom target is the stored center (default (-0.5,0), inside the
cardioid), zoomExp = u_src_zoom*7, no drift toward an edge (EmbeddedShaders.h:2934-2943); burning_ship same
pattern (:6378-6394, default (-0.75,-0.5) inside the ship body); julia_set's zoom target is fixed at the origin
while c is nudged toward the set by rms (:6257-6261, 6304-6305) -- at Tier-1's injected rms 0.6 the default c
lands inside the Mandelbrot set (DIAG D2 #16). This is exactly "turn the knob to its end, get black": Zoom is a
top-level control a VJ actually rides live, not an edge case. Plan's own numbers: black from 25-40% of the
knob's travel, 60-75% of total range (plan-tier1.md:83-85). Chosen action is AWAITING_RULING (test_sources,
strict -- must still render black) and SWEEP_AWAITING_RULING (test_fractals, xfail strict) --
plan-tier1.md:351-390, default disposition explicitly "(c) keep" (plan-tier1.md:539, HANDOFF item 5). Net
effect: G2 "13/13 passed" and G3 "93 passed, 8 xfailed" (plan-tier1.md:486-488) both go GREEN with the on-stage
behavior unchanged. The tests are re-shaped to certify the defect, not catch it; nothing in the app (clamp,
warning, tooltip) stops a performer from finding this live. This is the "gate that cannot fail" the dispatch
asks me to find.
Fix: ship option (b), or a manual-mode zoom clamp (same trick as the rejected julia clamp, plan line 47, scoped
to manual mode) as the default -- not merely documented as recommended-but-not-taken; if Boris says keep, add a
UI signal so this is not only visible in a pytest xfail table.

## SHOULD -- kifs Fold Type removed on "mechanism UNKNOWN"; code shows a plausible one-line cause, untried
SourceRegistry.cpp:522 addParam for Fold Type is deleted (plan-tier1.md:267, R1). Reading
EmbeddedShaders.h:7025-7040: branches 1 and 3 each fold all three axis pairs (x/y, x/z, y/z); branch 2
(0.33-0.67, the dead middle third) folds only x/y and x/z then rotates y/z without ever folding it first --
DIAG flags this "UNKNOWN" (tier1-residual-diag.md:61) and the plan accepts REMOVE without trying the obvious
candidate (add the missing y/z fold before the rotation). The same lane spends real effort tuning 5 Cross
Section factors by trial-and-error ladder (plan-tier1.md:248-249) but not 10 minutes on this one. The removal
changes an existing show's look ("old shows: a clip set in the middle third now shows the fractal instead of
black," plan-tier1.md:548-549) -- effort should match that.
Fix: try the missing-fold candidate before REMOVE; if it fails, show the rendered evidence, not "unknown."

## SHOULD -- SPARSE_AT_EXTREME legitimizes "reads as black on a projector" without touching what ships
Thickness 0 on spirograph/lissajous draws on 0.28/0.29% of pixels (plan-tier1.md:309-315); plan's own Boris
list: "reads as black on a projector" (plan-tier1.md:529). The fix only redefines what the test calls not-black
(a lit-fraction floor added to tests/visual/tier1_exceptions.py's is_black, :14-19); the visual fix (draw
connected lines) is offered only on the Boris list, default still "keep" (plan-tier1.md:544-546).

## Verified accurate (no attack found)
Every ES/SR citation checked matched exactly at 6db8d67: EmbeddedShaders.h:6757, 2969, 7077/7853, 6466 (sliceZ
factors, mandelbrot z=0 branch, kifs slice, newton power-to-n line), SourceRegistry.cpp:522, 987, 1003 (Fold
Type / Smoothing / Reflection), newton default Power 0.2f (SourceRegistry.cpp:446) confirming n=3 unchanged, and
the "identical below Power 0.894" cap arithmetic holds exactly. ProceduralSource.cpp:238-245 confirms only
registered params get uniform uploads (removed-param byte-identity claim holds). RoutineSlice.cpp:155,181
confirms the fx-index guard, supporting the "routines index-safe" risk note. test_sources.py:178-191 confirms
test_no_discontinuities only warns; :107-109 confirms BLACK_AT_EXTREME admits at ANY ladder candidate (both
match the plan's own "Found, not fixed" list). test_fractals retirement (173 ids) looks genuinely subsumed by
Tier-1's auto-discovery (test_sources.py:37-139) plus ported sweeps; the one real loss (julia C Real 0.8) is
named and rerouted to B1 -- not a hidden coverage hole.

## Strongest counterargument to my own position
Plan's own RISKS (plan-tier1.md:568-571): a permanently-red gate buries a NEW regression among 30 known lines,
and strict xfail mechanics mean a genuine fix makes the entry fail loudly -- not a silent whitewash. The Boris
list is honest about what still goes black. I still hold that "gate green + defect ships as default" is the
wrong resting state for a control this central to live performance -- a test that documents and normalizes a
stage-visible failure is still shipping the failure.
