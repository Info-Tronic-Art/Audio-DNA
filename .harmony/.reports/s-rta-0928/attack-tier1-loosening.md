VERDICT: SOUND_WITH_FIXES

## SHOULD 1: BLACK_AT_EXTREME excuses by (source,uniform) key, not by the diagnosed value
tests/visual/test_sources.py:107-114: "if key in BLACK_AT_EXTREME: tried.append(...); continue". key = (src["id"], uniform)
(test_sources.py ~:74). The evidence backing every entry (plan section 2, DIAG D2) is about ONE specific value (kifs
Offset 0.0, infinite_corridor Light 0.0). Once the key is listed, ANY ladder candidate that renders black for that
param is silently excused, not just the evidenced one. The plan's own section 8 "Found, not fixed" names this exact gap
("BLACK_AT_EXTREME admits black at ANY candidate of a listed key... a value-keyed table would be stricter") and then
adds 2 more entries onto the same unfixed mechanism (plan section 5c, plan-tier1.md:290-300) instead of tightening it,
even though the plan invents a stricter, value-checked pattern for its OWN new tables in the same commit
(SPARSE_AT_EXTREME requires lit_fraction(test_path) is greater or equal to SPARSE_MIN_LIT on the actual render,
test_sources.py patch S5g; AWAITING_RULING re-renders the exact recorded value and fails if it stops being black).
The cheap fix is already modeled two ways in this same plan: key BLACK_AT_EXTREME by (source, uniform, value)
instead of (source, uniform), or fold it into AWAITING_RULING's stricter "must still render black" pattern.
Bounded risk today (candidate_values() only emits about 5 deterministic candidates per param), but it is the
clearest exception-broader-than-evidence pattern in the packet, and the plan documents the gap without closing it.

## SHOULD 2: the sierpinski_tetra gate override depends on silent file-order, no enforcement
tests/visual/tier1_exceptions.py:137-142 (current) sets GATED_SOURCE_PARAMS entry for sierpinski_tetra slice_count
to a gate of Cross Section 0.55 only, inside a loop shared by 7 fractals. DIAG D4 number 27 proves this exact gate
FAILS today ("gate is Cross Section 0.55 -- FAIL default is black", tier1-residual-diag.md:89). The plan's fix
(plan-tier1.md:333-338) re-assigns the same dict key later in the file with a wider gate (Zoom 0.7, Iterations 1.0
added), relying on Python's last-assignment-wins and a comment ("these override the loop's tetra entries",
plan-tier1.md:317) as the only enforcement. If a future edit reorders the file, or a merge or rebase (the plan
itself flags concurrent-lane conflicts on this file, RISKS, plan-tier1.md:584-585) puts the loop after the override,
the dict silently reverts to the pre-existing broken gate, and nothing catches it -- it is a plain Python dict, not
something the lint or ctest verifies structurally. Cheap fix: delete the tetra entries from the loop's source list
rather than shadow them, or assert the expected gate value in a harness self-test.

## NIT 1: mandelbrot NaN guard is imprecise about which mode it touches
EmbeddedShaders.h about lines 2957-2969 (verified): Mandelbrot mode sets z to (0,0) and c to uv; Julia mode sets z
to uv and c to juliaC. The proposed guard (return c when dot(z,z) equals 0, else the polar step; plan-tier1.md:255)
also fires for the Julia-mode ray whose uv lands exactly at the screen center (z0 = uv = (0,0)), not only for
Mandelbrot mode as the comment and DIAG D2 number 18 ("Julia mode never met z=0") claim. Single pixel, does not
move p99.5 or lit percentage -- cosmetic, but the claim is stated as mode-scoped when the guard is actually
value-scoped.

## NIT 2: test_shader_param_lint.cpp's DEBT_FILED comment loses its only worked example
tests/test_shader_param_lint.cpp:12-15 cites spectrum_landscape's smoothing mix (a no-op mix of energy with itself)
as ITS illustrative example of a lint-clean-but-dead param. Plan S4 removes that exact param (SourceRegistry.cpp:987)
and only updates the comment text (plan-tier1.md:272-273) rather than swapping in a still-existing example -- the
lint's own doc becomes purely historical with no live illustration. No test assertion depends on it; pure
documentation debt.

## Verified NOT a problem (checked, cleared)
- The three removal comments (plan-tier1.md:267-269) contain neither the literal addParam-with-paren text nor the
  literal source-underscore text, which matters: registryParams() in tests/test_source_defaults_gl.cpp:50-67 does a
  raw substring count of that addParam call text over the WHOLE span including comments and REQUIREs it equal the
  regex-parsed count (lines 63-65) -- a careless comment would have broken the harness. The plan's chosen wording is
  safe (verified by direct inspection of the three proposed comment strings).
- compload::reconcileSourceParams is real (src/core/CompositionLoad.h:100 and :142; called from
  src/MainComponent.cpp:3005 and :3151), supporting the byte-identity-by-construction claim for the 3 removals.
- AWAITING_RULING replacing the full ladder walk with one negative probe (test_sources.py patch S5g) does not
  reduce existing coverage: the CURRENT loop (test_sources.py:95-120) already breaks on the first candidate that
  renders black (sets passed to None and breaks at line 113, then the outer loop breaks too), and candidate_values()
  (tier1_exceptions.py:21-31) puts each AWAITING_RULING entry's recorded value first in ladder order for every entry
  checked -- so today's test already stops there too. Formalizing it loses nothing new.

## Strongest counterargument to my own position
BLACK_AT_EXTREME's broad key has existed since s-rta-0927 and every entry it excuses is still required to prove a
passing candidate (PSNR 55 or under) elsewhere in the same fixed 5-value ladder for the param to count as "has
effect" -- it is not a gate that can never fail, only one that could excuse a regression landing on an
already-evidenced key at an un-evidenced value. Given the ladder is small and deterministic, the practical blast
radius is narrow. I hold the finding anyway because the plan explicitly names the gap in its own words and chooses
not to spend the one-line fix it demonstrates elsewhere in the same diff.
