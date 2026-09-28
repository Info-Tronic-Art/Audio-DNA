# Logic critic — source defects, round 1 (s-rta-0927)

Worktree: `.claude/worktrees/rta0927-w10`, branch `lane/source-defects-0927`,
base `2ee1013`, head `465443809a66121aa8864d5fac21228f0e255d55`. Read via
`git diff`/`git show` only, plus the 22 listed PNGs (Read). Judged against the
report's own claims (`.harmony/.reports/s-rta-0927/source-defects.md`) and the
shader diff for `julia_set`, `burning_ship`, `newton_3d`, `sierpinski`,
`crystal_cavern` and Dot Field.

VERDICT: PASS. Every before/after pair shows the claimed defect gone (black →
a picture with visible structure), the sierpinski depth cap removes the
subdivision aliasing it targets, the crystal_cavern sweep never goes black or
flat across t = 1/5/10 (256 and 1080), and the T4 contact sheet + timephase
crop show only the intended source set changed.

## Per-defect check (before/after, non-black + structure)

- **julia_set** (`pair_julia_set_1920x1080_t0.png`, `..._256x256_t0.png`):
  before is flat black (mean 0.0, lit 0.0%) at both sizes; after is a full
  classic-spiral Julia set, black interior lobes with a colored escape-time
  gradient, lit 50.1% / 43.8%. Matches the registry diff (`C Real` 0.35→0.2,
  `C Imaginary` 0.38→0.635, `Zoom` 0.25→0.1) and the report's live numbers to
  the digit (54.2/50.1 and 47.3/43.8 shown in-image vs. 54.19/50.14 and
  47.34/43.81 in `live-after.log`).
- **burning_ship** (`pair_burning_ship_1920x1080_t0.png`,
  `..._256x256_t0.png`): before near-black (0.0% / 0.4% lit); after shows the
  full ship silhouette (hull + mast) against a magenta/blue gradient, lit
  73.9% / 60.5%, matching the registry diff (`Center` 0.55/0.6→0.5/0.5, `Zoom`
  0.2→0.0) and the report's numbers. The speckled noise along the fractal
  boundary in the 256×256 shot is ordinary low-resolution escape-time dither
  at the fractal edge (present in any Mandelbrot-family render at 256px), not
  the row/column subdivision aliasing the sierpinski fix targets — the plan
  never claims burning_ship had that defect class.
- **newton_3d** (`pair_newton_3d_1920x1080_t0.png`, `..._256x256_t0.png`,
  `..._1920x1080_t30.png`, `..._256x256_t30.png`): t=0 before is flat black
  both sizes; after is a lit three-color Newton-fractal heightfield (mean
  ~46, lit ~65.5-65.8%), matching the diff (`Angle X` 0.55→0.0). t=30 before
  and after are both already-lit, structurally similar three-quarter views
  (73.7%→64.2% and 74.9%→67.6% lit) — consistent with the shader diff, which
  only removes the pitch (`rx`) time term and keeps yaw-only orbit
  (`ry += u_time*autoSpeed`), so t=30 was never black pre-change (matches
  `live-before.log`'s 73.7-74.9% figures) and stays a coherent, non-inverted
  view post-change. No sign of the camera tumbling under the terrain.
- **sierpinski** (`pair_sierpinski_256x256_t1.13.png`): before flat black
  (0.0%); after is a clean, sharp Sierpinski triangle at 10.0% lit with no
  moiré/aliasing artifacts — consistent with the depth-cap diff
  (`maxIter = min(maxIter, int(log2(u_resolution.y*zoomPow)+0.001))`), which
  exists specifically to stop the canvas-resolution aliasing described in
  Pitfall 44's comment in the diff.
- **crystal_cavern, "never flies out over time"**
  (`pair_crystal_cavern_256x256_t1/t5/t10.png`,
  `pair_crystal_cavern_1920x1080_t1/t5/t10.png`): before is a lit cave at
  t=1 (78.8-82.3% lit, matches design — defect only manifests after ~2.3s)
  and flat black at t=5/t=10 (0.0% both sizes, matching the diagnosed onset).
  After is non-black and structured at all three times and both sizes
  (lit range 71.1-87.3%), consistent with the z-repeat diff
  (`p.z = mod(p.z+1.2, 2.4)-1.2`) plus the conditional near-clip
  (`if (caveDE(ro,...) < 0.08) totalDist = 0.08`). No frame goes to a flat
  single color or off-scale white in any of the six shots — the picture gets
  denser/closer over time (camera flying past nearer crystals) but never
  loses structure, matching the report's own "0 black / 0 flat frames in a
  401-frame sweep" claim. The one visible artifact — a flatter, darker sphere
  near the frame edge in `..._1920x1080_t10.png` — is the near-clip's
  disclosed grazing cut-disc (Boris check 5, NUANCE section), not an
  undisclosed regression.
- **Dot Field** (`pair_fx_Dot_Field_default_256x256.png`,
  `..._1920x1080.png`): before is a sparse, near-black dot grid (1.2-1.5%
  lit); after is a dense, clearly visible dot pattern (7.2-8.5% lit),
  matching the `size` 0.3→0.7 default change in `EffectLibrary.cpp`.

## Inspector evidence (dead-control removal + reconcile)

`inspector_before_twisted_torus_window.png` shows 14 source rows for a new
twisted_torus clip on the pre-change code (Orbit, Tilt, Lens Shape, Lens
Rotate, Depth Fade, Pinch, Heart, Shading among them — all confirmed dead by
the `SourceRegistry.cpp` diff, which removes the shared `addTorusControls`
lambda and gives `twisted_torus` only 5 params: Twist, Stripe Count, Speed,
Tube Radius, Color Shift).
`inspector_after_twisted_torus_new_clip_window.png` shows exactly those 5
rows for a new clip on the lane build.
`inspector_after_twisted_torus_old_file_window.png` loads the OLD
14-parameter composition file (saved by the pre-change app) on the lane
build: the preview renders a visible black-and-white spiral (not black, not
a crash), and the inspector shows the reconciled 5-row set — direct evidence
the reconcile path (`CompositionLoad.h`) neither breaks old files nor leaves
stale/dead rows behind.

## T4 must-not-change sweep

`t4_changed_contact_sheet.png` (14 before|after tiles) contains exactly the
14 items the report's live T4 table names as changed: Dot Field, burning_ship,
crystal_cavern, julia_set (256/1080 each), julia_set_3d (256 only),
newton_3d (256/1080), sierpinski (256 only), line_pattern (1080),
waveform_lines (1080) — no unlisted source or effect appears in the sheet.
The `julia_set_3d_256x256` tile shows two visually indistinguishable purple
spheres, consistent with the claimed PSNR-83.5 LSB-level codegen shift from
rewriting the iteration loop bound (an expected side effect of the
Iterations-range fix applied to the sibling fractal, not a new defect).
`t4_timephase_line_pattern_waveform_lines_1080.png` shows the striped/
waveform patterns in near-identical phase between before and after (a small
sub-frame time offset, not a shape/color change) — consistent with the
report's claim that these two are a capture-timing artifact (byte-identical
offline) rather than a real change. No tile in either image shows an
uninvolved source or effect with a different picture, which is what the T4
claim ("only the intended set changed") requires the screenshots to show.

## Findings

None at MUST. No SHOULD or NIT findings raised — the two artifacts noted
above (burning_ship 256 boundary dither, crystal_cavern near-clip cut-disc)
are both either inherent to the render technique or already disclosed in the
report's own NUANCE/Boris-checks sections, not undisclosed defects.
