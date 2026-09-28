# Reviewer Verdict — review-tier1-loosening-r1
STATUS: DONE
VERDICT: APPROVE
FILES: src/render/EmbeddedShaders.h, src/sources/SourceRegistry.cpp, tests/test_shader_param_lint.cpp,
tests/test_source_defaults_gl.cpp, tests/visual/tier1_exceptions.py, tests/visual/test_sources.py,
tests/visual/test_fractals.py, tests/visual/conftest.py, tests/visual/SHADER_VERIFICATION.md,
docs/claude/{fractals.md,pitfalls.md,architecture.md,build-other-platforms.md}, CLAUDE.md,
.harmony/{APP-INVENTORY.md,FEATURES.md}, .harmony/.reports/s-rta-0928/tier1.md (17 files, lane/tier1-0928
734f011..221d087f)

ISSUES: 0 MUST, 3 NIT (none blocking)

1. [NIT / process disclosure, not a Builder defect] During verification I ran
   `pytest tests/visual/test_sources.py::TestExceptionTables::test_sierpinski_tetra_gates` in the PINNED worktree
   without `PYTHONDONTWRITEBYTECODE=1`, which regenerated `tests/visual/__pycache__/vision_check.cpython-314.pyc`
   (a tracked file). `git status --short` in the worktree now shows
   ` M tests/visual/__pycache__/vision_check.cpython-314.pyc`. I am fenced from `git checkout` (read-only role) so I
   could not revert it myself. This is exactly the pre-existing hygiene issue the Builder already names in their own
   report section 11 ("3 tracked `tests/visual/__pycache__/*.pyc` files get rewritten by any pytest run without
   PYTHONDONTWRITEBYTECODE") — it is not a defect the diff introduced, and the Builder's own G8 gate confirms the
   worktree was pyc-clean at head 221d087f. Harmony should `git checkout -- tests/visual/__pycache__/vision_check.cpython-314.pyc`
   in that worktree before merge (a one-line, zero-risk revert of a build artifact, not of reviewed source).
2. [NIT] E2 (kifs Fold Type candidate fix, "branch 2 y/z abs-fold, p99.5 5 at the 0.5 extreme") is not
   independently reproducible from the diff alone — the tried-and-reverted candidate line is gone, so this claim is
   INFERRED, not VERIFIED, from my side. It is internally consistent with the SR:522 comment ("a y/z sort before its
   rotation did not help") and does not affect correctness either way (Fold Type is removed regardless of whether the
   candidate truly failed at p99.5 5 or some other value).
3. [NIT] `docs/claude/pitfalls.md` #49 / the astral_grid M1 row's "INFERRED for signed zero: x = (uv.x - 0.5) * depth
   is never -0" reasoning is more cautious than necessary — IEEE 754 gives `x + (-0.0) == x` for every finite x
   regardless of sign, so the guard clause isn't actually load-bearing for the byte-identity conclusion. The
   conclusion (byte-identical at Warp = 0) is still correct; this is a precision-of-reasoning nit, not a fix needed.

SUMMARY: 17 files, 3 nits (0 blocking). Independently re-verified on the pinned worktree
(/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w3, head 221d087f): `ctest --test-dir
build-lane -j1` → 100% tests passed, 0 failed out of 803 (VERIFIED, matches report); `test_source_defaults_gl
[source-extremes]` → All tests passed (368 assertions in 6 test cases) (VERIFIED); `test_shader_param_lint` → All
tests passed (1383 assertions) (VERIFIED); `pytest --collect-only test_fractals.py` → 101 tests collected, exactly
the 8 predicted SWEEP_AWAITING_RULING ids marked xfail (mandelbrot-{0.25,0.5,0.75,1.0}, burning_ship-{0.5,0.75,1.0},
julia_set-0.7 dive) (VERIFIED); `pytest --collect-only test_sources.py` → 4 tests incl. the new
TestExceptionTables self-test (VERIFIED); the self-test's dict-membership assertions and its E5 "teeth" (re-adding
sierpinski_tetra to the shared 6-source loop after its own entries breaks the assertion via shadowing) both
reproduced directly against `tier1_exceptions.GATED_SOURCE_PARAMS` (VERIFIED, no live app needed for the logic
itself — the pytest wrapper needs the live app only because of the module's autouse `app` fixture, pre-existing
design). Cross Section final factors (mandelbulb 1.4, apollonian_3d 1.8, julia_set_3d 1.7, menger_sponge 2.0,
burning_ship_3d 1.2) and their documented ±ranges in fractals.md (±0.7/±0.9/±0.85/±1.0/±0.6) check out by direct
arithmetic (VERIFIED); Iterations floor formulas for tetra (7-15, 9 at 0.5) and kifs (5-13, 7 at 0.4) check out by
direct arithmetic (VERIFIED); newton Power cap n<=6 checks out (VERIFIED); byte-identity arguments (M1 table) for
all 9 shader edits + 3 removals hold by reading the guarded branches against each param's stored default (VERIFIED
by reading, not by rendering — same caveat the Builder's own report states for the read-only half of M1).
BLACK_AT_EXTREME is correctly value-keyed (E4), sierpinski_tetra has its own GATED entries out of the shared loop
(E5), AWAITING_RULING/SWEEP_AWAITING_RULING are strict and carry evidence-bearing reasons, SPARSE_AT_EXTREME was
correctly never built once E3's floor fix succeeded (confirmed absent from the diff and from a repo-wide grep — no
EXCESS_SPEC left behind). FENCE E8 respected: diff touches only EmbeddedShaders.h and SourceRegistry.cpp under
src/, no Renderer.*/CompositorEngine.*/ImageDecode.*, no thread/real-time-path files. CLAUDE.md is 23,984 bytes
(<=25,000, VERIFIED via wc -c). No stray `.venv` in the worktree. `kifs u_src_fold_type` is fully deregistered with
no dangling references outside the shader's own uniform declaration + read (expected, matches the byte-identity
argument). test_range_quality.py's `ALL_PARAM_TESTS` import stays intact. Confidence: HIGH on everything listed
above as VERIFIED; INFERRED (not independently re-run) on the live-app PSNR/p99.5/M2/M3 numbers themselves, which
require the app + GL rig the Builder already ran across 6 timestamped, lock-disciplined holds — no reason found to
doubt them given every independently-checkable claim (ctest counts, collection counts, xfail ids, arithmetic, file
fence) matched exactly.
METADATA: reviewer=reviewer-agent, builder_packet=lane/tier1-0928 (round 1 loosening review), date=2026-09-28
