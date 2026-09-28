STATUS: DONE
RESULT: I re-ran the Tier-1 residual on current main (app 233eae7). It is the same 30 lines, byte-for-byte. Every line is now binned with pixel evidence. Of the 20 "goes black at an extreme" lines, 4 are legitimate vanishing (exception candidates) and 16 are range or math defects. 10 of the 16 have a one-line fix that keeps the default render byte-identical (6 Cross Section factors, 2 Iterations remaps, the mandelbrot Power NaN guard, the newton Power cap). kifs Fold Type is byte-identical by REMOVE, the 3 zoom lines need a design call, and 2 have no one-liner. Of the 6 "no visible effect" lines, only 3 are really dead (astral_grid Warp: implement; band_tower Reflection and spectrum_landscape Smoothing: remove). The other 3 are gated params (harness entries). Each of the 4 harness gaps has a working gate entry, proven with the exact Tier-1 logic. That makes 7 lines fixable by a tier1_exceptions.py entry. test_fractals.py: 113 failed / 161 passed, identical across 5 runs (no flakes). 81 of the failures come from its mean<=5 brightness metric, 10 are gate gaps, 3 are stale/test-value/weak, and 19 are app defects (20 once the metric is fixed). Most of it duplicates Tier-1. I recommend retiring 2 of its classes and porting its sweeps onto the Tier-1 helpers.
FACTS: (1) D1: `1 failed in 207.17s (0:03:27)`. `cmp` against the prior lane's `source_param_failures.txt` = BYTE-IDENTICAL, sha256 `0524d335...88e7` (tier1diag/runs/d1/). (2) Cross Section is black on 32-52 % of the knob on 5 of the 7 3D fractals and on 70 % for kifs. sierpinski_tetra is black at all 50 values, max p99.5 7 (runs/b1/rows.jsonl, slice_summary.txt). (3) mandelbrot Power is black at every value >= 0.26 in Mandelbrot mode and lit at the same powers in Julia mode, so z0 = 0 is the trigger (b1 mb_power / mb_power_julia). (4) newton_fractal Power 1.0: a float32 numpy port reproduces the app's lit fractions: n=7 port 12.98 % vs app 12.64 %; n=8 at zoom 0 port 85.42 % vs app 84.39 %; n=8 at zoom 0.3: 0 %, all non-finite (newton_port32.log). (5) julia_set Zoom black is caused by the fixture: rms 0.6 moves c to (-0.6973, 0.2818). The numpy port matches the app's lit % to 0.01 at zoom 0.1/0.3/0.35. In silence, zoom 1.0 is 90.22 % lit in the app (runs/f1) (julia_port.log). (6) The 7 gate entries pass the exact Tier-1 logic (runs/b2, b3 tier1_logic.txt). (7) test_fractals: `113 failed, 161 passed in 96.21s` x2, `96.59s`, `96.49s`, `96.29s`. Outcomes are identical in all 5 runs, and the multiset of PNG pixel hashes is identical, `f933ffda9e5122c6` x5 (runs/f1..f5).
METHOD: I read the named reports, the exceptions module, conftest and every cited shader and registry block. I ran 4 lock acquisitions of Eyes renders under Tier-1 conditions (composition 256, FEATURES_ACTIVE, t 1.13, load_source without params), 943 renders plus 1080p checks. I decoded every PNG (p99.5, mean, lit %, sha), looked at 5 contact sheets, and replicated the Tier-1 per-param logic in code for each proposed gate. I wrote float32/float64 numpy ports of the Newton and Julia shaders as an independent check. I ran test_fractals 5 times, each on a fresh app, and re-derived every failure from its PNGs.
CONFIDENCE+VERIFY: High on all measured numbers (deterministic, pixel-decoded) and on the D3/D4 verdicts (render-verified with controls). Medium on 3 mechanisms marked INFERRED below: mandelbrot NaN source, the kifs rotating-fold cause, and the zoom-glide fix. Every "byte-identical default" claim for a candidate fix is INFERRED, because no build was allowed. To verify: `bash tier1diag/run-exp.sh <label> tier1diag/exp_b1.py` (or b2/b3); `bash tier1diag/run-pytest.sh <label> <tests>/test_fractals.py`.
UNKNOWNS/NOT-DONE: No candidate fix was built or rendered (diagnosis only). The Cross Section factors are mapped from the measured ladder, not rendered. The kifs rotating-fold (0.33-0.67) failure mechanism is not derived. Whether a thicker slab rescues sierpinski_tetra's slice is not tested. test_fractals was not ported or edited.
NUANCE: 3 of F1.C's bins change. (i) Lines 24-26 are not dead: wireframe Density is live once Shape selects a grid shape, and moire Offset Y is live in Dots and Rings. (ii) Line 16 (julia Zoom) is fixture-dependent. (iii) Line 13 (sierpinski_tetra Cross Section) is black at every value, not only at the extreme. The 4 legitimate-vanishing lines do not meet tier1_exceptions' current admission rule (a) ("provably zero-output"). Two are drawn dots under the 0.5 % coverage floor and two are bounded near-black, so admitting them needs a policy call or a second admission class.
HANDOFF-NEEDS: Planner or Boris: (1) rule on admitting the 4 (a) lines, (2) pick the design option for the 2D zoom targets (mandelbrot / burning_ship / julia), (3) choose REMOVE or reimplement for kifs Fold Type.

INBOX-RECHECK: none

# LANE tier1diag: Tier-1 residual, binned (s-rta-0928)

- App under test: `/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app` (main build = code 233eae7), launched only through `start_app ... test` (`open -g --args --test-mode`).
- Tests: the scratch worktree `.claude/worktrees/rta0928-w3` (detached at 6db8d67). The repo was not touched.
- Evidence root (below: `SP/`): `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/tier1diag/`
- Runs:
  - `runs/d1` (D1)
  - `runs/b1` (D2/D3/D4 ladders, 740 renders)
  - `runs/b2` (gates, tetra grid, kifs fold, 1080p, 191 renders)
  - `runs/b3` (tetra gates, 12 renders)
  - `runs/f1`..`runs/f5` (test_fractals)
- Each run has `rows.jsonl` (per render: params, t, size, p99.5, mean, lit %, sha) and `png/`.
- Scripts: `eyes.py` (client with `Connection: close`), `exp_b1.py`, `exp_b2.py`, `exp_b3.py`, `run-exp.sh`, `run-pytest.sh`, `run-fractals-x4.sh`, `analyze_fractals.py`, `bin_fractals.py`, `newton_port32.py`, `newton_port32b.py`, `julia_port.py`, `stale_defaults.py`.
- Contact sheets: `sheet_slice.png`, `sheet_lines.png`, `sheet_hd.png`, `runs/f1/sheet_fractals.png`.

Metric everywhere: black = p99.5 of per-pixel max channel < 16 (`tier1_exceptions.is_black`). The line numbers are `src/render/EmbeddedShaders.h` at 6db8d67 unless stated.

## D1: residual on current main
| run | verbatim summary | lines | vs prior lane4 list |
|---|---|---|---|
| `runs/d1` 08:25:35 to 08:29:02 | `FAILED .claude/worktrees/rta0928-w3/tests/visual/test_sources.py::TestAllSourceParams::test_all_params_have_effect` / `1 failed in 207.17s (0:03:27)` | 30 | `cmp` BYTE-IDENTICAL (same order); sha256 `0524d3351d425df59647adaf49297deae0c5f43be7ed0f6bb89fd006a08188e7` |

No difference to report.

## D2: the 20 "goes black at an extreme" lines
Conditions: Tier-1 exactly (composition 256x256, `FEATURES_ACTIVE` injected, `load_source` without params so H1 seeds defaults, t = 1.13); where stated, also at 1920x1080. "Black band" = ladder values with p99.5 < 16. The "byte-identical" column is INFERRED unless it says VERIFIED-by-reading: no candidate was built.

### (a) LEGITIMATE VANISHING: exception candidates (4)
| F1.C # | line | where / why (evidence) | numbers | one-line reason |
|---|---|---|---|---|
| 9 | spirograph Thickness @0 | :6012 `thick = 0.004 + t*0.02` | ladder p99.5 0/0/2/29/63/95/142 at t = 0/.02/.05/.1/.15/.2/.3; lit 0.28 % at 0. At 1080p p99.5 0 but lit 0.15 %: the dots are clearly drawn (`sheet_hd.png` crop) | "thinnest line (r 0.004) is drawn but covers <0.5 % of pixels, below the p99.5 floor" |
| 10 | lissajous Thickness @0 | :5978 `thick = 0.005 + t*0.03` | p99.5 0/0/8/48/88/121/166 at the same ladder; lit 0.29 % (256) / 0.16 % (1080), drawn (crop) | same |
| 14 | kifs Offset @0 | :7025 `offset = vec3(1)*off*2`. At 0 every map is p → sc·fold(p), which fixes the origin, so the attractor is one point | black 0.00-0.10 (p99.5 0,0,0,1,2,11.3); lit from 0.12 (80); default 0.5 is 238; 1080 @0 p99.5 0 | "Offset 0 collapses the IFS attractor to the origin (zero size)" |
| 20 | infinite_corridor Light Intensity @0 | :9254 `lightMask *= intensity`, and the diffuse term `* intensity`; only :9258 `ambient 0.05` is left, so max channel <= 0.2·0.05·1.04·255 = 2.65 | p99.5 2/5/11/22/42 at 0/.02/.05/.1/.2; lit from 0.1; 1080 @0 p99.5 2 | "zero light: ambient-only, provably <= 2.7/255" |

Caveat for the planner: `tier1_exceptions.BLACK_AT_EXTREME` admission (a) says "shader term provably zero-output".
- #20 and #14 fit in spirit: #20 is a provable bound under 16, #14 is a zero-size attractor.
- #9 and #10 do NOT fit: they draw something, but it is under the 0.5 % coverage floor.

Admitting them needs Boris's policy call or a second admission class ("drawn, lit % > 0, coverage < 0.5 %"). Unrelated to the black verdict: every "Lines" source here is drawn as point samples (300 / 200 dots, :6015 / :5980), not as segments. The default spirograph and lissajous are visibly dotted (`sheet_lines.png`).

### (b) RANGE / MATH defects (16)
| F1.C # | line(s) | where it goes black, and why | numbers | candidate one-line fix | default byte-identical? |
|---|---|---|---|---|---|
| 1,2,4,5,6,7 | mandelbulb, apollonian_3d, julia_set_3d, kifs, menger_sponge, burning_ship_3d Cross Section @0 | RANGE. `sliceZ = (slice-0.5)*3.0` (:6757, :6918, :7258, :7409) or `*4.0` (kifs :7077, apollonian_3d :7853) sweeps a 0.04-thick slab to ±1.5 / ±2.0. The objects span roughly ±0.66 to ±1.02, so both ends of the knob are empty space | Lit band / black share of the 50-value ladder (`slice_summary.txt`):<br>- mandelbulb 0.24-0.82 / 42 %<br>- apollonian_3d 0.26-0.74 / 52 %<br>- julia_set_3d 0.20-0.82 / 38 %<br>- kifs 0.26-0.74 / 70 % (plus 9 internal dropouts, e.g. 0.36 p99.5 4)<br>- menger 0.16-0.84 / 32 %<br>- burning_ship_3d 0.28-0.82 / 46 %<br>1080 @0: mandelbulb 0, kifs 0 | Per-source factor so both ends land inside the lit band (derived from the ladder): mandelbulb `*1.5`, apollonian_3d `*1.8`, julia_set_3d `*1.7`, kifs `*1.8` (the dropouts stay), menger `*2.0`, burning_ship_3d `*1.3` | YES (VERIFIED-by-reading): `sliceZ` is only read inside `if (useSlice)`, and `useSlice = abs(slice-0.5) > 0.01` is false at the default 0.5 |
| 3 | sierpinski_tetra Cross Section @0 | MATH, and not an extreme. The point-cloud DE (:7675 `length(z)*pow(Scale,-iters)`, 4^iters specks of radius 0.001) inside a 0.04 slab never shows at the source's defaults (Zoom 0.3, Iterations 0.5) | Black at all 50 values (max p99.5 7) and with Slice Count 0.5 (max 13). 1080: 0.4 → 2, 0.6 → 3. Visible only with Zoom >= 0.7 and Iterations 1.0 (e.g. 0.55 → 138; `runs/b2` tetra_grid, 96 renders) | Not a one-liner (needs a thicker slab or a solid DE for this source; not tested) | n/a |
| 8 | spirograph Inner Radius @0 | MATH (sampling alias, not an extreme). :6015 samples 300 points over 10 revolutions. When (R-r)/r = p/q with q <= 10 (inner 0 → r 0.1 → ratio 5) the samples coincide, giving only 30q distinct dots | Black at 0, 0.05, 0.125, 0.25, 0.35, 0.5, 0.65, 0.75 (8 of 25 values, mid-travel too); 1080 @0 and @0.35 p99.5 0; lit at 0.02 / 0.1 / 0.4 (`sheet_lines.png`) | Distance to segments between samples (like `lineDist` in sourceWireframe3D) | NO: the default is itself a dotted 300-point curve |
| 11 | sierpinski_tetra Iterations @0 | RANGE (DE floor). :7690 `int(it*12)+3` gives 3-5 iterations for 0-0.25, i.e. 64-1024 specks | p99.5 2/4/9 at iters 3/4/5; 16.0/25/39 at 6/7/8; 143 at default 9; plateau about 199 from 10 on. 1080: iters 3 → 2, iters 5 → 8 | `int(u_src_iterations * 4.0) + 7` (0 → 7, 0.5 → 9, 1 → 11; 11+ is the plateau) | YES (INFERRED: same integer 9 at 0.5) |
| 12 | kifs Iterations @0 | RANGE (DE floor). :7063 `int(it*10)+3` | iters 3 p99.5 7 (1080: 6); iters 4 → 22, 5 → 56, 7 (default) → 238, plateau after | `int(u_src_iterations * 5.0) + 5` (0 → 5, 0.4 → 7, 1 → 10) | YES (INFERRED: 0.4·5 = 2 → 7) |
| 13 | kifs Fold Type @0.5 | MATH, broken mode. Branch 2 (0.33-0.67, :7034, a rotating fold) never renders. Branch 3 (>= 0.67) is dead: after :7029 `p = abs(p)`, `p.x + p.y < 0.0` (:7041) is never true, and with a permutation-symmetric offset that makes it identical to branch 1 | 0.35-0.65 p99.5 <= 2 (one hash); 0.0-0.30 and 0.70-1.0 share hash `ae10edc1be14`. Branch 2 stays black at t 0/1.13/2/5/10, Iterations 0-1, Offset .25-1, Zoom 0-.9 (15 renders); 1080 p99.5 2. The mechanism is UNKNOWN | REMOVE Fold Type (unregistered uniform = 0 → branch 1) | YES for REMOVE (INFERRED: the GL uniform default is 0 = the stored default) |
| 15 | burning_ship Zoom @1.0 | RANGE / design. The zoom target is the manual center (:6385), default (-0.75, -0.5) = inside the ship body; `zoomExp = zoom*7` (:6391) | Black from 0.30 to 1.0 (70 % of travel); 0.25 is lit (p99.5 255, lit 2.49 %). At Zoom 1.0 with a Location: 4 of 5 lit; preset 2 "Main body center" (:6372) is black | (A) pull the manual-mode target toward a boundary point as zoom deepens, `center = mix(center, B, 1.0 - exp(-zoomExp))`; (B) set a boundary Location as the default | (A) YES (INFERRED: weight 0 at zoomExp 0), but it bends Center X/Y at depth (Pitfall 10), so Boris's call. (B) NO |
| 16 | julia_set Zoom @1.0 | FIXTURE-DEPENDENT design. The zoom target is fixed at the origin = the critical point (:6258), so at depth the frame is all interior iff c ∈ M. The default c (-0.7, 0.27) is 0.012 from the M boundary, and :6304 `cx += sin(t*.2)*.02*rms` (and cy) with the Tier-1 rms 0.6 moves it to (-0.6973, 0.2818), inside M | App Tier-1: black from 0.40 (60 %); lit 38.75 / 5.55 / 0.91 % at 0.1 / 0.3 / 0.35. The numpy port gives interior 61.25 / 94.45 / 99.09 / 100 %, an exact match. In silence: app zoom 1.0 lit 90.22 % (`runs/f1`), port 9.58 % interior. Location presets 4 / 6 / 9 black at zoom 1 (c ∈ M) | (A) `zoomExp = min(zoomExp, 2.1)` (the knob goes flat above 0.35); (B) a boundary zoom target (not a one-liner) | (A) YES (INFERRED: the default zoomExp 0.6 < 2.1) |
| 17 | mandelbrot Zoom @1.0 | RANGE / design. Default manual center (-0.5, 0) (:2934) is inside the main cardioid; `zoomExp = zoom*7` (:2940) | Black from 0.25 to 1.0 (75 %); 0.2 is lit (lit 1.61 %). Zoom 1.0 + Location: 8 of 9 lit; preset 8 "Mini Julia island" (:2919) is black | Same as #15 (A)/(B) | Same as #15 |
| 18 | mandelbrot Power @1.0 | MATH. In Mandelbrot mode z0 = 0 (:2957). For power >= 2.5 the polar branch runs `atan(0,0)` (:2967; undefined in GLSL) → NaN → no pixel ever escapes. INFERRED: which of atan / pow(0,p) yields the NaN | Black at every Power >= 0.26 (75 % of travel), in Mandelbrot mode only. Julia mode (z0 = uv) is lit at every power (b1 `mb_power_julia`). Power 0-0.24 is one hash (the quadratic branch: a dead zone). 1080: 0.3 → 0 | In the polar branch: `z = (dot(z,z) == 0.0) ? c : rn * vec2(cos(power*theta), sin(power*theta)) + c;` | YES (VERIFIED-by-reading: the default Power 0 → power 2.0 < 2.5, so the polar branch is not executed) |
| 19 | newton_fractal Power @1.0 | MATH / RANGE. With n = 8 (:6466) near the pole z = 0 (the default Zoom 0.3 frames \|z\| <= 0.22), the first Newton step throws z to about 1e20, and `cdiv` (:6448) `dot(b,b)` of n·r^(n-1) overflows float32 → NaN. The iteration budget (:6469, 59) is also too small to come back | App 256: n=8 black, n=7 (0.9) lit 12.64 %, n=8 at Zoom 0 lit 84.39 %. 1080: n=8 lit 22.23 %, the centre disk black (`sheet_hd.png`). The float32 port matches (12.98 % / 85.42 % / 0 % with 100 % non-finite); +50 iterations does not help in float32 (`newton_port32b.log`) | Cap the power range: `int(min(rawPow * rawPow * 5.0, 3.0)) + 3` (n <= 6; n=6 is lit 63 %) | YES (INFERRED: n = 3 at 0.2) |

Totals: (a) 4 lines, (b) 16. Among the 16:
- 10 lines have a one-liner candidate that keeps the default byte-identical: 6 Cross Section factors, tetra Iterations, kifs Iterations, mandelbrot Power, newton Power.
- 1 more (kifs Fold Type) is byte-identical by REMOVE.
- 3 more have a byte-identical design option: zoom #15, #16, #17.
- 2 have no one-liner: tetra slice, spirograph sampling.

## D3: the 6 "no visible effect" lines, verified by rendering (3 times each: t = 1.13, 2.0, 5.0)
| # | line | render result | verdict (source-defects rule) | one-liner / entry | default byte-identical |
|---|---|---|---|---|---|
| 21 | astral_grid Warp | warp 0 / 0.5 / 1 give one hash at each t (`d477968c179a`, `27dfaf468272`, `0113866cf5ea`) | DEAD (VERIFIED): :9411 `yWarp` is computed and never used → IMPLEMENT (one-liner) | `vec2 gp = vec2(x + yWarp, z) / gridS;` (sideways sine wobble, bass-scaled) | YES (INFERRED: warp 0 → yWarp = ±0, and x + ±0 == x) |
| 22 | band_tower Reflection | 0 / 0.5 / 1 give one hash at all 3 t (`fa9c71488c07`) | DEAD (VERIFIED): :10723 `uv.y < 0.0` is unreachable (gl_FragCoord uv >= 0) → REMOVE | delete `Reflection` from band_tower in `SourceRegistry.cpp` (`registerSource("band_tower"` block, :1000-1005) | n/a: a working reflection would draw at the stored default 0.3, so implementing is not byte-identical |
| 23 | spectrum_landscape Smoothing | 0 / 0.5 / 1 give one hash at all 3 t (`e84ed89a7ade`) | DEAD (VERIFIED): :10594 `mix(energy, energy, s)`; temporal smoothing needs history state → REMOVE | delete `Smoothing` from the `registerSource("spectrum_landscape"` block (:981-989) | n/a |
| 24 | wire_wolf Density | own Shape (0.93 → shape 6): identical at all t. With Shape 0.072 (sphere): 3 distinct hashes, PSNR 11.7 | NOT DEAD, GATED (VERIFIED): `gridN` (:5386) is read only by shapes 0-4; wolf/icosahedron (:5390 `int(shape*6.99)` = 6 / 5) never read it, and each wireframe source exposes Shape (`SourceRegistry.cpp:286`) | `GATED_SOURCE_PARAMS[("wire_wolf", "u_src_density")] = ({"u_src_shape": 0.072}, "EmbeddedShaders.h:5386-5511: gridN is read only by the grid shapes 0-4 (int(u_src_shape*6.99) < 5); the preset's own shape (wolf 6 / icosahedron 5) is a fixed mesh")`; Tier-1 logic: PASS via 0.0 (PSNR 11.7) | n/a. UX note for Boris: on these two presets the knob does nothing until Shape moves |
| 25 | wire_icosahedron Density | same (`4538656735d4` for own shape; sphere gate changes) | same | same entry with `"wire_icosahedron"`; PASS via 0.0 (PSNR 11.7) | n/a |
| 26 | moire_interference Offset Y (was INFERRED) | Pattern 0 (Lines): 0 / 0.5 / 1 give one hash at all 3 t. Pattern 0.5 (Dots) and 1.0 (Rings): 3 distinct hashes each at every t | NOT DEAD, GATED (VERIFIED): Lines = `sin(p.x*freq)` (:9032) reads only x, and the Y offset is added after rotation (:9059) | `GATED_SOURCE_PARAMS[("moire_interference", "u_src_offset_y")] = ({"u_src_pattern": 0.5}, "EmbeddedShaders.h:9032,9059: Lines mode reads only uv2.x, so the Y offset is invisible; Dots/Rings read y")`; PASS via 0.0 (PSNR 13.2; Rings 11.1) | n/a |

Every ungated control gave `FAIL no visible effect` with PSNR inf on all candidates (`runs/b2/tier1_logic.txt`).

## D4: the 4 harness gate gaps (exact Tier-1 per-param logic replayed; `runs/b2`, `runs/b3` `tier1_logic.txt`)
| # | line | ungated / current (repro) | working gate | Tier-1 logic result | exact `tier1_exceptions.py` entry |
|---|---|---|---|---|---|
| 27 | sierpinski_tetra Slice Count | `gate={"u_src_slice": 0.55} -> FAIL default is black` (repro) | slice 0.55 + Zoom 0.7 + Iterations 1.0. No Cross Section value alone works (black at all 50; D2 #3) | `PASS via 1.0 (PSNR=20.5)`; Zoom 0.9 variant `PSNR=19.1` | after the 7-fractal loop: `_TETRA = {"u_src_slice": 0.55, "u_src_zoom": 0.7, "u_src_iterations": 1.0}`<br>`GATED_SOURCE_PARAMS[("sierpinski_tetra", "u_src_slice_count")] = (dict(_TETRA), _SLICE + "; sierpinski_tetra's slice is invisible at its own Zoom 0.3 / Iterations 0.5 (point-cloud DE), so the gate also moves the camera in and deepens the IFS")` |
| 28 | sierpinski_tetra Slice Distance | (same repro) | `_TETRA` + Slice Count 0.5 | `PASS via 0.0 (PSNR=21.2)` (Zoom 0.9: 19.2) | `GATED_SOURCE_PARAMS[("sierpinski_tetra", "u_src_slice_dist")] = ({**_TETRA, "u_src_slice_count": 0.5}, _MULTI + "; see sierpinski_tetra slice_count")` |
| 29 | newton_3d Trail Fade | `FAIL no visible effect` (PSNR inf ×4) | Trail Distance 0.6 (:7572 `numTrails`) | `PASS via 0.0 (PSNR=50.9)`: only 4.1 dB under the 55 limit, the weakest pass here | `GATED_SOURCE_PARAMS[("newton_3d", "u_src_trail_fade")] = ({"u_src_trail_dist": 0.6}, _TRAIL)` |
| 30 | shape_generator Rotation | `FAIL no visible effect` (circle, PSNR inf ×4) | Shape 0.2 (box). 0.3 / 0.5 / 0.65 also pass (via 0.5 / 0.25 / 0.5); Shape 0.8 (ring) is rotation-invariant too | `PASS via 0.1 (1.0 → 83.3, 0.5 → 85.8, 0.25 → 90.3, 0.1 → 13.6)` | `GATED_SOURCE_PARAMS[("shape_generator", "u_src_rotation")] = ({"u_src_shape": 0.2}, "EmbeddedShaders.h:4960: the default Shape 0 is sdCircle, which rotation leaves unchanged; the box (0.143-0.286) shows it")` |

Controls: the current 7-fractal gates still pass (mandelbulb Slice Count `PSNR=22.3`, Slice Distance `22.7`).

## Re-binned residual (30 → where each line goes)
| bin | count | lines |
|---|---|---|
| tier1_exceptions.py entry (harness) | 7 | 24, 25, 26, 27, 28, 29, 30 |
| BLACK_AT_EXTREME candidate (policy; admission rule needs a ruling for 9, 10) | 4 | 9, 10, 14, 20 |
| app, one-liner, default byte-identical | 10 | 1, 2, 4, 5, 6, 7, 11, 12, 18, 19 |
| app, byte-identical by REMOVE | 1 + 2 | 13 (kifs Fold Type); 22, 23 (remove dead) |
| app, implement one-liner (byte-identical) | 1 | 21 |
| app, design call (a byte-identical option exists) | 3 | 15, 16, 17 |
| app, no one-liner | 2 | 3 (tetra slice), 8 (spirograph sampling) |

## D5: tests/visual/test_fractals.py
**Safety read (VERIFIED):**
- The only app calls are `list_sources`, `load_source`, `update_source_params` and `render_frame`, plus the conftest `reset`.
- No Output window path, no ax/input synthesis.
- `tier1_canvas_256` does not apply (the module is not in `TIER1_CANVAS_256`), so captures are 512x512 on the default 1920x1080 composition.
- All 15 sources are stateless: the canvas resize on each capture (H2) is harmless.
- Collected: 274 tests. 158 are TestEveryParamHasEffect.
- Run in full (about 96 s, under the 20-min hold) 5 times, each on a fresh app.

| run | verbatim summary |
|---|---|
| f1 08:49:50 | `113 failed, 161 passed in 96.21s (0:01:36)` |
| f2 08:53:35 | `113 failed, 161 passed in 96.21s (0:01:36)` |
| f3 08:55:19 | `113 failed, 161 passed in 96.59s (0:01:36)` |
| f4 08:57:04 | `113 failed, 161 passed in 96.49s (0:01:36)` |
| f5 08:58:48 | `113 failed, 161 passed in 96.29s (0:01:36)` |

- Outcomes (274 ids) are identical in all 5 runs, and the assertion messages are identical (f1 / f2 / f5 compared).
- The multiset of pixel hashes over every rendered PNG is identical: `f933ffda9e5122c6` ×5.
- **FLAKY: 0** (5 runs per arm).
- The counts equal the prior lane's `113 failed, 161 passed`.

Every failure was re-derived from its PNGs (`runs/f1/fail_metrics.txt`, `bins.txt`). I looked at samples in `runs/f1/sheet_fractals.png`: the mandelbulb, tetra and apollonian "all-black" frames are drawn; mandelbrot power 0.5, julia dive 0.7 and kifs fold 0.5 are really black.

| bin | root cause | count (primary) | still failing after the metric fix |
|---|---|---|---|
| harness: metric | `frame_is_not_black = mean > 5`. Sparse 3D fractals are drawn at p99.5 61-223 with mean 0.6-4.9: 4 non_black, 54 "default is all-black" (every param of mandelbulb 14, julia_set_3d 15, burning_ship_3d 13, sierpinski_tetra 12), 10 "with X is all-black" (menger Angle X/Y and Cross Section 0.8; kifs Scale and Palette; apollonian Scale, Angle X/Y, Speed, Palette), 12 Test3DZoomRange, 1 Z2 burning_ship 0.25 (p99.5 214) | 81 | 68 pass; 12 turn into gate; 1 turns into app (tetra Cross Section) |
| harness: needs a gate | Slice Count / Slice Distance with Cross Section 0.5 (`useSlice` off), and Trail Fade with Trail Distance 0 (PSNR inf): menger 3, kifs 3, apollonian 3, newton_3d 1 (and 12 more hidden behind the metric) | 10 | 22 |
| harness: test value | kaleido_fractal Rotation 0.0→0.5 = a π rotation of an abs()-folded, mirror-symmetric pattern (:2842); PSNR 76.7 | 1 | 1 |
| harness: stale expectation | sierpinski Iterations 0.5→0.9: the Pitfall 44 cap (:6553) = 9 levels at 512 for both; PSNR inf | 1 | 1 |
| harness: weak value | apollonian_3d Trail Distance 0→0.5: PSNR 55.7 (Tier-1's ladder passes it at 1.0) | 1 | 1 |
| app: zoom target inside the set (= D2 #15/#17) | mandelbrot:Zoom 0.3; burning_ship:Zoom 0.4; Z2 mandelbrot 0.25/0.5/0.75/1.0; Z2 burning_ship 0.5/0.75/1.0 (p99.5 0) | 9 | 9 |
| app: z0 = 0 NaN (= D2 #18) | mandelbrot:Power 0.5; TestPowerNotBlack 0.25/0.5/0.75/1.0 | 5 | 5 |
| app: Cross Section range (= D2 #1-7) | kifs and apollonian_3d Cross Section 0.8 (the `*4.0` factor: sliceZ 1.2 > extent 0.96), p99.5 0 | 2 | 2 |
| app: kifs Fold Type (= D2 #13) | Fold Type 0.5, p99.5 2 | 1 | 1 |
| policy / geometry: Julia c inside M | julia_set C Real 0.8 → c = (0.2, 0.27) is in the main cardioid, a solid filled set: numpy 100 % interior at zoom 0.1 (69.7 % at zoom 0). Dive 0.7 at t=5 morphs c to (-0.056, 0.489) ∈ M: 100 % interior. The dive path between presets 1→2 crosses M's interior | 2 | 2 |
| app: tetra slice invisible (= D2 #3) | sierpinski_tetra Cross Section 0.8: changed frame p99.5 5 | (in metric) | 1 |
| **total** | | **113** | **45** (22 gate, 3 test value/stale/weak, 20 app/policy) |

Static tuple check (`stale_defaults.log`): all 158 params are registered. 1 tuple has a stale default in its message (newton_fractal Zoom 0.0 vs registry 0.3); it is cosmetic, because the default render uses registry defaults. No test value equals a registry default. 11 registered params go untested (the 9 Feedback params, julia_set_3d Iterations, sierpinski/apollonian Dive Speed).

**Overlap with Tier-1.**
- Duplicate: TestSourceLoadsAndRenders (15) and TestEveryParamHasEffect (158). Tier-1 `test_sources` auto-discovers all 15 sources and every param. It does this with the better metric, candidate ladders, gates, audio features and a 256 canvas. It also covers the 11 params test_fractals skips.
- Unique: the multi-position not-black sweeps. These are TestZoomLooping 30, Test3DZoomRange 28, TestDiveSpeedNotBlack 20 (at t=5), TestPowerNotBlack 5, plus TestPaletteVariety 15 and TestSourceRegistry 3. Tier-1's `test_no_discontinuities` renders 0/0.25/0.5/0.75/1.0 too, but it only WARNS. So the sweeps are the only failing check that catches mandelbrot/burning zoom at 0.25-0.75, mandelbrot power 0.25-0.75, and the julia dive-path blackout.

**Recommendation.**
1. RETIRE TestSourceLoadsAndRenders and TestEveryParamHasEffect: every check they make is also made by Tier-1 `test_sources`, with better mechanics (metric, ladder, gates, audio, canvas).
2. MERGE the sweep, palette and registry classes into Tier-1 infrastructure. Switch to `is_black`, add the module to `TIER1_CANVAS_256`, and keep silence or `arm_audio` explicit.
3. Predicted residual after the port: exactly the 12 real sweep failures. These are Z2 mandelbrot 0.25-1.0 (4), Z2 burning_ship 0.5-1.0 (3), TestPowerNotBlack 0.25-1.0 (4) and julia dive 0.7 (1); all 12 PNGs are p99.5 0. This is INFERRED from the per-image p99.5 above; the port was not run.

Fixing test_fractals in place is the worse option: it would re-implement Tier-1's ladder, gates and metric a second time.

## Risks
- **Fixture-dependent verdict (#16).** julia_set Zoom is black only because the Tier-1 fixture's rms 0.6 pushes c across the M boundary. A fix that clamps zoom will pass Tier-1, while with music the unclamped app still flashes black at depth. The test and the product agree here, but not for the reason F1.C gave.
- **Admission rule vs the (a) list.** Admitting #9 and #10 under the current "provably zero-output" rule would weaken it silently. Rule on a coverage class first.
- **Byte-identity claims are INFERRED.** The builder lane must prove each one: render default sha before and after at 256 and 1080. The Cross Section one is also VERIFIED by reading.
- **newton_3d Trail Fade gate passes with a 4.1 dB margin (PSNR 50.9).** It could drift red on a shader tweak. Zoom or Trail Distance 1.0 might widen it (untested).

## Rig log (disclosed)
Lock acquisitions (lock.sh, owner tier1diag):
| run | acquired / released | hold | note |
|---|---|---|---|
| D1 | 08:25:30 / 08:29:04 | 3.6 min | |
| b1 | 08:32:42 / 08:34:24 | 1.7 min | waited on harmony and restorediag |
| b2 | app launched 08:42:44 (app-out.log birth time) / released 08:43:19 | under 1 min | |
| b3 | app launched 08:46:13 (app-out.log birth time) / released 08:46:20 | under 1 min | |
| f1 | 08:49:45 / 08:51:28 | 1.7 min | waited on restorediag |
| f2-f5 | 08:53:32 / 09:00:29 | 7 min | |

- Every hold was under 20 min, with at least 45 s cooldowns (helper-enforced).
- Every launch was `open -g ... --test-mode` through `start_app`, and every quit was through `quit_app`. The helper printed `app running after quit: no` and `audio-dna windows 0, Output-named 0` after every batch.
- Final check at 09:02:40: no lock dir, no Audio-DNA process, `audio-dna windows 0, Output-named 0`.
- No Output window path, no test_output_window_level, no pytest on the tests/visual directory, no input synthesis, no debugger or profiler, no screen capture (renders only through 8080 `render_frame`). There were no system dialogs.

Incidents:
- Once, a python heredoc command started with `cd /;`, against the never-cd rule. It was harmless: every path was absolute.
- The 4-run fractal batch was started with a trailing `&` instead of `run_in_background`. It ran detached, I polled its log, and it finished and released the lock at 09:00:29.
- `stale_defaults.py` ran without `PYTHONDONTWRITEBYTECODE`. It rewrote the tracked `tests/visual/__pycache__/vision_check.cpython-314.pyc` and created `test_fractals.cpython-314.pyc` in MY scratch worktree. I restored the first with `git show HEAD:<path> > <path>` (single file, my own write) and deleted the second. The worktree status is now clean, and the main repo is untouched: its status shows only the pre-existing `.harmony-version`, `.claude/` and `AGENTS.md`.

## PACKET QUALITY
- Clarity: CLEAR. Every D-item named its method and its output.
- Missing context: (1) Tier-1 injects rms 0.6, which moves the julia c; I found this by checking the input fixture. (2) The BLACK_AT_EXTREME admission rule's wording excludes "drawn but under the coverage floor". (3) test_fractals runs at composition 1920x1080; it is not in TIER1_CANVAS_256.
- Unused context: the prior helper `run-tier1-r.sh` (I adapted it rather than using it).
- Self-brief files: CLAUDE.md, followups.md F1.C/D/E, tier1-diag.md, tier1_exceptions.py, conftest.py and plan-source-defects.md §APPROACH, A2 and POLICY were all useful. pitfalls.md 9-12, 44 and 45 were useful. I did not read `.harmony/notebook.md`: the packet did not list it, and this lane is diagnosis only. None were stale.

### STATUS
DONE

### NEXT ACTION
Planner:
1. Land the 7 harness entries (D3 24-26, D4 27-30).
2. Rule on the 4 (a) lines and on the admission class.
3. Plan the app lane, in order: the 10 byte-identical one-liners, REMOVE kifs Fold Type / band_tower Reflection / spectrum_landscape Smoothing, implement astral Warp, then Boris's design call on the zoom targets.
4. Retire / port test_fractals as recommended.
