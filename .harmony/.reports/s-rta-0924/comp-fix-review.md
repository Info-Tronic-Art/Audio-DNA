# Reviewer Verdict — comp-fix-review (s-rta-0924)
STATUS: DONE
VERDICT: APPROVE-WITH-NITS
NOTE: packet named REPORT_FILE=.harmony/.reports/s-rta-0924/comp-fix-review.md,
but the reviewer write-fence (s159 ADJ ruling) only permits memory/.reports/ or
memory/wip/ for read-only agents; this repo has no memory/ tree, so this verdict
is dropped at the nearest allowed path instead. Full verdict is also returned in
the chat response below.

FILE: src/connect/ScalarParams.h (+21 lines, additive)
  [OK] Math correctness (VERIFIED, re-derived independently): posX3840/posY2160
    give pixel range ±1920/±1080 (half of the 3840x2160 reference canvas). The
    shader applies `uv -= u_comp_position * 0.5` (EmbeddedShaders.h:97-121).
    Dividing by 1920/1080 then *0.5 yields px/3840, px/2160 -- fraction of the
    reference canvas -- the correct proportional UV shift. Confirmed:
    posPxToCompUniformX(1920)=1.0, 1.0*0.5=0.5 UV shift=half the 3840-wide
    canvas.
  [OK] Anchor math (VERIFIED): identical conversion, and the shader applies the
    identical `*0.5` convention to u_comp_anchor (subtract before rotate/scale,
    add back after -- pivot-point pattern) -- consistent with position.
  [OK] Placement/scope: additive helpers beside the existing ScalarMath table,
    matches file's to/normFrom pairing convention, no existing formula touched.

FILE: src/render/Renderer.cpp (~2078-2093)
  [OK] Only call site (VERIFIED via grep). Compiles without a new #include --
    already transitively pulled in via Composition.h -> connect/ScalarParams.h.
  [OK] isDefault gate (Renderer.cpp:2039, VERIFIED): still uses raw pixel-space
    posX/posY/scale/rotation for its 0.001f/0.01f thresholds, unaffected by the
    units fix -- correct.
  [ISSUE-NIT] Y-axis sign cross-checked (not independently ground-truthed): no
    GL context available to render a screenshot, so I compared against the
    pre-existing, presumably-working clip/layer transform path
    (CompositorEngine.cpp:479-488 + EmbeddedShaders.h layer_transform:3325-3362)
    -- both apply `uv -= translate` with the same posY2160-derived sign, so this
    fix does not invert Y relative to the app's established clip/layer
    convention. Labeled INFERRED, not VERIFIED against actual rendered output.
  [NOTE, out of scope] Clip/layer transform normalizes by the ACTUAL render
    w/h (CompositorEngine.cpp:481) and applies translate directly (no extra
    *0.5), vs comp_transform's fixed 1920/1080 reference + *0.5. Different
    formula shape, same sign; pre-existing, not touched by this diff.

FILE: tests/CMakeLists.txt
  [OK] Target wiring mirrors the preceding test_manual_write block, no name
    collisions. BUILT via `cmake --build . --target test_comp_transform_units`
    -- succeeds, no new warnings.

FILE: tests/test_comp_transform_units.cpp (new)
  [ISSUE] Test bypasses the real caller (gotcha T13): every assertion calls
    ScalarMath::posPxToCompUniformX/Y directly, never Renderer::applyCompTransform
    -- the actual bug site (a missing conversion call at the glUniform2f
    upload). If Renderer.cpp:2084/2093 regressed back to passing raw posX/posY
    straight to glUniform2f, this suite stays green. It pins the helper's
    formula, not the fix. GL-context unit testing appears out of reach for this
    tier (matches file's stated "same minimal style as test_smoother"); real but
    bounded gap -- recommend a follow-up Eyes/visual-tier check asserting a
    non-zero comp position doesn't render solid black. Not blocking.
  [ISSUE-NIT] Round-trip block (lines 44-55) asserts
    `posPxToCompUniformX(px) == px/1920.0f` -- tautological with the
    implementation, adds nothing beyond the fixed-point checks at lines 26-42
    (which ARE meaningful: 1920/1080 constants pinned against independently
    derived "half of 3840/2160" reasoning).
  [OK] RAN the suite: 17 assertions, 5 test cases, all pass.

BUILD VERIFICATION (VERIFIED, executed this session):
  - `cmake --build build --target test_comp_transform_units` -- succeeds.
  - `./build/tests/test_comp_transform_units` -- all pass.
  - `touch src/render/Renderer.cpp && cmake --build build --target AudioDNA` --
    recompiles and relinks cleanly; only pre-existing unrelated JUCE
    -Wdouble-promotion warnings, nothing new from the changed files.

SCOPE: Surgical, no unrelated refactor. Comments are dense but accurate and
load-bearing.

SUMMARY: 4 files, 2 issues (0 blocking, 2 suggestions: test bypasses the real
call site it's meant to guard; one tautological sub-assertion). Math
independently re-derived and confirmed correct for position AND anchor.
Y-axis sign cross-checked against the existing clip/layer convention and found
consistent (inferred, not screenshot-verified -- no GL context in this
review). isDefault gate, include wiring, and CMake target verified by
executing a real build and test run.
METADATA: reviewer=reviewer-agent, builder_packet=comp-fix-review, date=2026-09-24T00:00:00Z
