## BUILDER REPORT -- LANE tier1 (s-rta-0928)

STATUS: DONE
RESULT: All 30 Tier-1 residual lines and the test_fractals smoke are closed on `lane/tier1-0928` in 7 commits (C1 RED ctest, C2 12 shader fixes, C3 3 knob removals, C4 harness, S6a fill, C5 docs, this report). Tier-1 is GREEN on the lane app, and so is effects parity: test_sources 4/4 with `test_all_params_have_effect` at 0 lines, effects 3, audio 4, time 1, perf 2. test_fractals `93 passed, 8 xfailed`. ctest 803/803. Every stored default of the 12 touched sources renders byte-identical (M3 64/64; M2 208/208 stable ids).
FACTS: RED `tests/test_source_defaults_gl.cpp` [source-extremes] on today's shaders: `test cases: 6 | 0 passed | 6 failed` / `assertions: 368 | 318 passed | 50 failed`. GREEN after C2: `All tests passed (368 assertions in 6 test cases)`. Full ctest at HEAD a47efde: `100% tests passed, 0 tests failed out of 803` (base 797 + 6). Shader edits in `src/render/EmbeddedShaders.h` (12 one-line in-place edits, line count unchanged). Removals in `src/sources/SourceRegistry.cpp:522`, `:987`, `:1003`. Harness in `tests/visual/tier1_exceptions.py`, `tests/visual/test_sources.py`, `tests/visual/test_fractals.py`, `tests/visual/conftest.py`. Pitfalls 49/50 in `docs/claude/pitfalls.md`.
METHOD: I checked every candidate edit first in the headless GL rig: a scratch Catch2 case string-replaced each line in the shipped shader text, then compared the rendered pixels and checked the stored default for byte-identity before I touched EmbeddedShaders.h. This caught the plan's mandelbrot ternary: it changes the stored default. After that: RED ctest, the edits, GREEN ctest, full serial ctest. Live runs used the lock helper, 6 holds, `open -g --args --test-mode`. BEFORE used the base-app copy (main 734f011); AFTER used build-lane. Frames are pixel-decoded (sha256 of the decoded pixels, p99.5, lit %), and I looked at the contact sheets.
CONFIDENCE+VERIFY: High. Re-prove with: `ctest --test-dir build-lane -j1`; `build-lane/tests/test_source_defaults_gl "[source-extremes]"`; `AUDIODNA_NO_SPAWN=1 PYTHONDONTWRITEBYTECODE=1 pytest -p no:cacheprovider tests/visual/test_fractals.py` against the build-lane app (expect `93 passed, 8 xfailed`); test_sources.py (expect 4 passed, about 11.5 min).
UNKNOWNS/NOT-DONE: (1) Flake status of reaction_diffusion Diffusion A in test_sources: it failed once on the BASE app at PSNR 55.1 (limit 55) and passed on the lane app. That is 1 run per arm, so no flake verdict. The key is untouched by this lane. (2) The 6 AWAITING_RULING defects and the 8 SWEEP xfails wait for Boris (B1 / B3 / B4). They are not fixed by design.
NUANCE: I deviated from the plan three times, each on measured evidence. (a) The mandelbrot guard is on theta, not a ternary on the z update: the plan's ternary AND its else-if fallback both changed the stored power-2 default (488 bytes differ at 256x256, PSNR 43.3). (b) The Cross Section factors for mandelbulb and burning_ship_3d are 1.4 and 1.2, not 1.5 and 1.3. The plan's own lower-by-0.1 rule applied: at the 0.0 end they measured p99.5 10 and 13 at 1920x1080. (c) Adoption E3 worked, so spirograph and lissajous Thickness got an app fix (a floor line) and there is no SPARSE_AT_EXTREME table. E2 did not work (the y/z sort draws p99.5 5), so kifs Fold Type is removed as planned. On M2, cellular_automata 1920x1080 differs between the base and lane apps. It is a stateful simulation: every capture on the same app gives a new hash, and its shader is untouched (see section 6).
HANDOFF-NEEDS: none

INBOX-RECHECK: none

### SUMMARY
All 30 residual lines are closed: 12 app fixes (the 10 planned, plus the 2 Thickness lines via E3), 3 removals, 2 value-keyed BLACK_AT_EXTREME entries, 7 GATED entries and 6 strict AWAITING_RULING entries (Boris). test_fractals is retired-2 / ported onto Tier-1 conditions with 8 strict B1 xfails. Every stored default is byte-identical.

### Commits (branch lane/tier1-0928, base 734f011; not merged, not pushed)
| commit | item |
|---|---|
| 02539fc | C1 test: RED `[source-extremes]` (6 cases) |
| 2317e79 | C2 fix: 12 one-line shader edits |
| 1958751 | C3 fix: remove kifs Fold Type, spectrum_landscape Smoothing, band_tower Reflection; Fold Type tuple; lint comment |
| 272b51f | C4 test: harness (tier1_exceptions, test_sources, test_fractals, conftest) |
| f089a77 | S6a test: SWEEP_AWAITING_RULING = the 8 observed ids; newton_3d Trail Fade gate 1.0 |
| a47efde | C5 docs: Pitfalls 49/50, fractals.md, SHADER_VERIFICATION, counts 681 -> 678 |
| (this) | report |
Main moved during the lane to 450f669 with a docs-only commit (`.harmony/notebook.md`, `.harmony/s-rta-0928-work.md`). It does not overlap this lane's files, so I did not rebase. The merge-base stays 734f011.

### 1. Per-line table (the 30 DIAG D1 lines)
"Base" is the lane harness on the base app (`runs/base_sources/source_param_failures.txt`, 16 lines, verbatim below). "Lane" is the lane app (`runs/lane_sources`: `4 passed in 684.33s (0:11:24)`, 0 lines).
| # | line | action | base | lane |
|---|---|---|---|---|
| 1 | mandelbulb Cross Section @0 | APP factor 3.0 -> 1.4 | `goes black at 0.0` | PASS via 0.0 PSNR 19.5 |
| 2 | apollonian_3d Cross Section | APP 4.0 -> 1.8 | `goes black at 0.0` | PASS via 0.0 PSNR 18.5 |
| 3 | sierpinski_tetra Cross Section | AWAITING_RULING (0.0) B3 | (awaiting: reproduces) | reproduces, p99.5 0 |
| 4 | julia_set_3d Cross Section | APP 3.0 -> 1.7 | `goes black at 0.0` | PASS 19.8 |
| 5 | kifs Cross Section | AWAITING_RULING (0.0) B3 | (awaiting) | reproduces, p99.5 0 |
| 6 | menger_sponge Cross Section | APP 3.0 -> 2.0 | `goes black at 0.0` | PASS 16.2 |
| 7 | burning_ship_3d Cross Section | APP 3.0 -> 1.2 | `goes black at 0.0` | PASS 19.9 |
| 8 | spirograph Inner Radius | AWAITING_RULING (0.0) B4 | (awaiting) | reproduces, p99.5 0 (lit 0.17 %) |
| 9 | spirograph Thickness | APP floor line (E3) | `goes black at 0.0` | PASS via 0.0 PSNR 36.7 |
| 10 | lissajous Thickness | APP floor line (E3) | `goes black at 0.0` | PASS 29.7 |
| 11 | sierpinski_tetra Iterations | APP floor | `goes black at 0.0` | PASS 28.0 |
| 12 | kifs Iterations | APP floor | `goes black at 0.0` | PASS 21.3 |
| 13 | kifs Fold Type | REMOVE (E2 tried, failed) | `goes black at 0.5` | param gone |
| 14 | kifs Offset | BLACK_AT_EXTREME (kifs, offset, 0.0) | -- | `0.0 (black, listed)` then PASS 0.25 PSNR 19.5 |
| 15 | burning_ship Zoom | AWAITING_RULING (1.0) B1 | (awaiting) | reproduces |
| 16 | julia_set Zoom | AWAITING_RULING (1.0) B1 | (awaiting) | reproduces |
| 17 | mandelbrot Zoom | AWAITING_RULING (1.0) B1 | (awaiting) | reproduces |
| 18 | mandelbrot Power | APP theta guard | `goes black at 1.0` | PASS via 1.0 PSNR 9.3 |
| 19 | newton_fractal Power | APP cap n <= 6 | `goes black at 1.0` | PASS 6.8 |
| 20 | infinite_corridor Light Intensity | BLACK_AT_EXTREME (.., 0.0) | -- | `0.0 (black, listed)` then PASS 0.25 PSNR 34.3 |
| 21 | astral_grid Warp | IMPLEMENT | `no visible effect ... PSNR=inf` | PASS via 1.0 PSNR 18.3 |
| 22 | band_tower Reflection | REMOVE | `no visible effect` | param gone |
| 23 | spectrum_landscape Smoothing | REMOVE | `no visible effect` | param gone |
| 24 | wire_wolf Density | GATED Shape 0.072 | -- | PASS 0.0 PSNR 11.7 |
| 25 | wire_icosahedron Density | GATED Shape 0.072 | -- | PASS 11.7 |
| 26 | moire Offset Y | GATED Pattern 0.5 | -- | PASS 13.2 |
| 27 | tetra Slice Count | GATED _TETRA (own entry) | -- | PASS 1.0 PSNR 20.5 |
| 28 | tetra Slice Distance | GATED _TETRA + count 0.5 | -- | PASS 0.0 PSNR 21.2 |
| 29 | newton_3d Trail Fade | GATED Trail Distance 1.0 | -- | PASS 0.0 PSNR 49.6 (0.6: 50.9) |
| 30 | shape_generator Rotation | GATED Shape 0.2 | -- | PASS 0.1 PSNR 13.6 |
Totals: 12 app (10 planned + 2 via E3), 3 removals, 2 BLACK_AT_EXTREME, 7 GATED, 6 AWAITING_RULING = 30. The GATED / BLACK / FIXED / AWAITING PASS values above come from the S6a exact-logic replay (`runs/replay1/tier1_logic.txt`, lane app).

Base-app verbatim (lane harness, base app 734f011; `1 failed, 3 passed in 712.98s (0:11:52)`):
```
astral_grid:Warp no visible effect (default=0.0, tried [1.0 (PSNR=inf), 0.5 (PSNR=inf), 0.25 (PSNR=inf), 0.1 (PSNR=inf)])
reaction_diffusion:Diffusion A no visible effect (default=0.5, tried [0.0 (PSNR=55.1), 0.25 (PSNR=61.7), 1.0 (PSNR=55.2), 0.1 (PSNR=55.1)])
mandelbulb:Cross Section goes black at 0.0
band_tower:Reflection no visible effect (default=0.300000011920929, tried [0.0 (PSNR=inf), 0.5 (PSNR=inf), 0.25 (PSNR=inf), 1.0 (PSNR=inf), 0.1 (PSNR=inf)])
spirograph:Thickness goes black at 0.0
apollonian_3d:Cross Section goes black at 0.0
sierpinski_tetra:Iterations goes black at 0.0
julia_set_3d:Cross Section goes black at 0.0
kifs:Iterations goes black at 0.0
kifs:Fold Type goes black at 0.5
menger_sponge:Cross Section goes black at 0.0
burning_ship_3d:Cross Section goes black at 0.0
lissajous:Thickness goes black at 0.0
mandelbrot:Power goes black at 1.0
newton_fractal:Power goes black at 1.0
spectrum_landscape:Smoothing no visible effect (default=0.5, tried [0.0 (PSNR=inf), 0.25 (PSNR=inf), 1.0 (PSNR=inf), 0.1 (PSNR=inf)])
```
The reaction_diffusion line is NOT one of the 30. It is a stateful simulation, at PSNR 55.1 against a limit of 55, and the key is untouched by this lane (no entry). The same line passed on the lane app. I give no flake verdict (1 run per arm), and it is named in found-not-fixed.

### 2. ctest (G1)
- RED, C1 on today's shaders (11:05:49; `/private/tmp/.../scratchpad/tier1/red-c1.log`): `test cases:   6 |   0 passed |  6 failed` / `assertions: 368 | 318 passed | 50 failed`. Examples: Cross Section ends p99.5=0, mandelbulb 0.25 at 1080 p99.5=10, tetra Iterations 0 p99.5=1, kifs 5, mandelbrot Power 0.25-1.0 p99.5=0, newton 1.0 at 256 p99.5=0, `astral_grid Warp 0 -> 1 at 256x256 t=1.13: PSNR 1e+09`, spirograph/lissajous Thickness 0 p99.5=0 (lit 0.27 % / 0.28 %).
- GREEN, C2 (`green-c2.log`): `All tests passed (368 assertions in 6 test cases)`.
- Full serial: C2 `100% tests passed, 0 tests failed out of 803`; C3 the same line; final at HEAD a47efde (12:20:08): the same line. Base count 797 (= tempo's 797/797) + 6 new cases. The plan expected +5; the sixth is the E3 Thickness case.

### 3. Final factors, floors, guards (src/render/EmbeddedShaders.h, one line each, line count unchanged)
- Cross Section `sliceZ = (u_src_slice - 0.5) * F`:
  - mandelbulb :6757 **1.4**: the plan's 1.5 was p99.5 10 at 1080 @0.0, so 0.1 lower by rule; 1.4 gives 61.
  - menger :6918 **2.0**.
  - julia_set_3d :7258 **1.7**.
  - burning_ship_3d :7409 **1.2**: the plan's 1.3 was p99.5 13 at 1080 @0.0; 1.2 gives 55.
  - apollonian_3d :7853 **1.8**.
  - tetra :7702 and kifs :7077 are unchanged.
- tetra :7690 `max(int(x*12)+3, int(x*4)+7)`. Iterations 0 is p99.5 19 at 256 and 17 at 1080 in silence (margin 1 over the floor at 1080; the plan's fallback applies only on a fail).
- kifs :7063 `max(int(x*10)+3, int(x*5)+5)`. Iterations 0 is p99.5 43 / 41.
- mandelbrot :2967 `float theta = (dot(z, z) == 0.0) ? 0.0 : atan(z.y, z.x);` (DEVIATION). Evidence from the headless rig, default renders today vs variant (`scr-mb.log`):
  - plan ternary on :2969: `DIFFERENT bytesdiff=488 maxdiff=178 psnr=43.3` at 256, `bytesdiff=8892 psnr=46.0` at 1080, julia-mode `psnr=35.8`;
  - plan fallback `else if` on :2965: identical numbers;
  - theta guard: `IDENTICAL` at 256 / 1080 x t 0 / 1.13 / 5.13 and julia-mode.
  All three fix the power (p99.5 178, lit 52-76 %).
- newton :6466 `int(min(rawPow * rawPow * 5.0, 3.0)) + 3`.
- astral :9414 `vec2(x + yWarp, z)`.
- spirograph :6012 `max(0.004 + t*0.02, 0.0085 + t*0.002)`; lissajous :5978 `max(0.005 + t*0.03, 0.01 + t*0.01)` (E3). Both are identical to today from Thickness 0.25, and the headless rig shows the stored default IDENTICAL at 256 / 1080 x t 0 / 1.13 / 5.13 / 10.13 and at Thickness 0.3 / 0.5 / 1.0. Thickness 0 in silence:
  - spirograph p99.5 83 (256) / 33 (1080);
  - lissajous p99.5 77 / 30.
  Exception: lissajous at exactly t = 0.0 at 1080 is p99.5 0. That frame is the curve's own degenerate phase, and there the stored default itself drops to p99.5 29 with half its lit area (B4 family, sampling). At t = 0.25-10.13 the Thickness-0 frame is p99.5 27-31.

### 4. E2 / E3 (adoption rulings)
- E2 kifs Fold Type: I tried the seat's candidate (`if (p.y - p.z < 0.0) p.yz = p.zy;` before the rotation) headless. Fold 0.5 gave p99.5 7 / 5 / 4 / 1 / 0 at t 0 / 1.13 / 2 / 5 / 10 (256) and 7 / 5 / 4 / 1 / 0 (1080); 0.34-0.66 all p99.5 5. It is not a picture, so Fold Type is **REMOVED** as planned. Fold 1.0 renders the same as fold 0.0 (p99.5 183, lit 12.5 %, identical stats), which confirms the top third was a copy of the bottom.
- E3 Thickness: the floor worked. It is an **app fix**: SPARSE_AT_EXTREME is not created, and `lit_fraction` is not added.

### 5. test_fractals (G3)
- Collected 101 (30 zoom + 20 dive + 5 power + 28 3D zoom + 15 palettes + 3 registry).
- Base app with the lane harness, SWEEP empty (`runs/base_fractals`): `12 failed, 89 passed in 26.11s`. The failures: mandelbrot Zoom 0.25/0.5/0.75/1.0, burning_ship Zoom 0.5/0.75/1.0, julia_set Dive 0.7, and mandelbrot Power 0.25/0.5/0.75/1.0.
- Lane app, SWEEP empty (S6a, `runs/lane_fractals_empty`): `8 failed, 93 passed in 26.11s`. Exactly the 8 predicted B1 ids, all p99.5 0 (decoded).
- Lane app with SWEEP filled (`runs/lane_fractals`): `93 passed, 8 xfailed in 28.09s`, 0 XPASS.

### 6. Byte-identity (G5 = M2 + M3)
- M3 (14 touched sources plus mandelbrot Julia-mode plus julia_set; 256 composition, FEATURES_ACTIVE, t 0 / 1.13 / 5.13 / 10.13): **64 of 64 identical** BEFORE vs AFTER. The touched sources are the 12 in the plan plus spirograph and lissajous.
- M2 (every source, `load_source` defaults, silence, t 1.13, 256 and 1920x1080; 216 captures; BEFORE x2 on the base app, AFTER x2 on the lane app):
  - 208 identical across all 4 runs.
  - 8 vary. Seven vary between the two BEFORE runs, so they are the noise floor:
    - projectm_visualizer at 256 and 1080;
    - reaction_diffusion at 256 and 1080;
    - strange_attractor at 256 and 1080;
    - cellular_automata at 256.
  - Their b1-vs-after PSNR is at or above the b1-vs-b2 floor, except strange_attractor 256 (51.6 vs 57.3). Both of those pass the plan's >= 30 bar.
  - projectm 1080 has b1-b2 18.8 / b1-after 25.8: the floor itself is under 30.
  - One more: **cellular_automata 1920x1080** was equal in both BEFORE runs and in both AFTER runs, but differs between them (PSNR 8.3).
- Why I did not STOP on cellular_automata. On the lane app, 10 `reset + load_source + render` captures of cellular_automata (5 per size) gave 10 distinct hashes, while burning_ship_3d captured between them repeated its hash exactly (`runs/after_ca/rows.jsonl`). The simulation advances one step per rendered frame, so the frame count since its FBO was created decides the picture. Its 256 capture alternates between the same two hashes (a35a593b / 2f576d75) in BOTH apps. Its shader and ProceduralSource are untouched: `git diff 734f011 HEAD -- src` touches 12 ES lines, none in sourceCellularAutomata (:3248-3360). This is a stateful noise id that happened to agree twice within each app, not a render change. Harmony may want 5 runs per arm before calling it noise.
- Removals: kifs / band_tower / spectrum_landscape defaults are identical in M2 at both sizes and in M3.

### 7. G6 frames (Tier-1 conditions: 256 composition, FEATURES_ACTIVE, t 1.13; plus 1920x1080 silence for the ends)
Contact sheets (BEFORE row over AFTER row): `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/tier1/cmp1/sheet_g6_*.png` and `sheet_g6hd_*.png` (23 sheets). I looked at the mandelbrot, spirograph-hd, mandelbulb, kifs and astral_grid sheets. What they show:
- mandelbrot Power 0.25-1.0: black before; multibrot sets after (p99.5 255, lit 52-62 %).
- mandelbulb Cross Section 0 / 1: black before; a slice ring after.
- kifs Fold Type 0.5: black before; the default fractal after (what an old show's middle-third clip now shows).
- astral Warp: the grid lines bend sideways.
- spirograph at 1080: a faint dot ring before; a visible dotted ring after.

Selected numbers (p99.5 before -> after):
| frame | before | after |
|---|---|---|
| Cross Section 0.0 @256 | 0 for all 5 | mandelbulb 121, apollonian 161, julia3d 80, menger 230, bship3d 81 |
| Cross Section 1.0 @256 | 0 for all 5 | 179 / 187 / 235 / 231 / 189 |
| Cross Section 0.0 @1080 | 0 | 61 / 70 / 59 / 177 / 55 |
| tetra Iterations 0 | 2 (256), 1 (1080) | 25, 17 |
| kifs Iterations 0 | 7, 5 | 56, 41 |
| mandelbrot Power 1.0 | 0 | 255 (256), 178 (1080) |
| newton Power 1.0 | 0 (256), 90 (1080, lit 22 %) | 144 (lit 63 %), 145 (lit 78 %) |
| spirograph Thickness 0 | 0 (lit 0.28 % / 0.14 %) | 108 / 33 |
| lissajous Thickness 0 | 0 (0.29 % / 0.15 %) | 100 / 30 |

### 8. Other gates
- G2 Tier-1, lane app:
  - `test_sources.py` `4 passed in 684.33s (0:11:24)`. This is 3 existing tests plus the new TestExceptionTables self-test, and `test_all_params_have_effect` has 0 lines.
  - `test_effects.py` `3 passed in 175.77s (0:02:55)`.
  - `test_audio_reactivity.py` `4 passed in 4.26s`.
  - `test_time_sweep.py` `1 passed in 55.53s`.
  - `test_performance.py` `2 passed in 71.56s (0:01:11)`, run after `wait_quiet`: no compiler, load `{ 4.67 5.29 5.24 }`.
  - In total 14 passed: the plan's 13 plus the self-test.
- G4 probe-effects-parity (PARITY_APP = build-lane app, lock owner tier1; 12:55:49-12:56:34): `PY 46 PASS / 0 FAIL`, `PASS  app terminated`, `PROBE-EFFECTS-PARITY GREEN` (`runs/parity/probe.log`).
- G7 `/api/sources` (lane, `runs/after1/api_sources.json`): 108 sources, **678 params** (base 681).
  - kifs: Scale, Iterations, Angle X, Angle Y, Zoom, Speed, Offset, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette.
  - band_tower: Spacing, Color Mode.
  - spectrum_landscape: Height Scale, Camera Angle, Color Mode, Glow.
- G8 screen safety:
  - Every hold ended with `app running after quit: no` and `audio-dna windows 0, Output-named 0`.
  - No Output window path, no test_output_window_level, no directory pytest, no synthetic input, no debugger, no screen capture.
  - `git status` is clean of pyc.
- Teeth for the tetra self-test (E5): a scratch copy of tests/visual whose tier1_exceptions re-adds sierpinski_tetra to the shared loop after its own entries. `test_sierpinski_tetra_gates` FAILS there (`1 failed in 0.22s`; `AssertionError: assert {'u_src_slice': 0.55} == {...}`).
- AWAITING teeth: a mandelbrot Zoom probe at 0.1 prints `stale` (p99.5 255, lit 20.05 %). All 6 real entries print `reproduces (black)`.

### 9. Holds (lock helper, owner tier1)
| hold | acquired / released | app | content |
|---|---|---|---|
| before1 | 11:16:28 app up / 11:18:08 | base | M2+M3+G6 |
| holdB2 | 11:26:14 / 11:39:46 (13.5 min) | base | M2 rerun, test_fractals (RED), test_sources |
| holdL1 | 11:41:51 / 11:44:12 | lane | M2+M3+G6 AFTER, S6a replay, test_fractals (empty), tetra teeth |
| holdL2 | 11:51:08 / 12:04:18 (13.2 min) | lane | M2 rerun, test_fractals (filled), test_sources |
| holdL3 | 12:14:24 / 12:19:42 | lane | CA repeatability, effects, audio, time, perf |
| parity | 12:55:49 / 12:56:34 | lane (probe launches it) | probe-effects-parity |
There was a >= 45 s cooldown between holds (helper-enforced), and every hold was under 15 min.

### 10. Boris list (plain words; nothing ships differently unless he says so)
**TOP (Harmony E1): Zoom goes black for most of its travel on three 2D fractals.** Mandelbrot past a quarter of the knob, Burning Ship past 0.3, and Julia Set past 0.4 on loud music: the zoom dives into the black inside of the fractal. Mandelbrot and Burning Ship Dive at the starting center also go black after a few seconds. Julia's Dive has black stretches (e.g. Dive 0.7 around 5 s), and Julia "C Real" 0.8 fills the screen black. Options:
- (a) the zoom always dives toward edge detail. The picture at Zoom 0 does not change, but Center X/Y becomes a sideways nudge at depth.
- (b) move the starting center (and Julia's starting shape) onto the edge. The starting picture shifts a little.
- (c) keep, and pick a Location before zooming.
**Harmony recommends (a)**: a Zoom knob that is black for most of its travel is the worse stage failure, and (a) keeps every stored show identical at Zoom 0. The tests list these strictly until you decide.

Other items waiting for your call:
- B3 Cross Section on Sierpinski Tetrahedron shows nothing (unless Zoom >= 0.7 and Iterations at max). On Kaleidoscopic IFS it shows faint specks with black gaps. Options: (a) draw the specks bigger inside the slice only; (b) remove the slice knobs from these two; (c) keep. Recommended (a).
- B4 Spirograph "Inner Radius" blinks to black at 8 of 25 positions (e.g. 1/4, 1/2, 3/4). Spirograph and Lissajous draw dotted curves. Option: draw connected lines (the starting look changes from dots to a solid curve). Recommended: draw lines.

Settings that go black on purpose (listed as known):
- Kaleidoscopic IFS "Offset" all the way down: the fractal shrinks to one point.
- Infinite Corridor "Light Intensity" all the way down: lights off.
- Unchanged: Fire Wall Density 0, Concentric Rings Width 0, Metaballs Size 0, Line Pattern Width 0.

Removed knobs:
- R1 Kaleidoscopic IFS "Fold Type": its middle third was black (a one-line repair was tried and still drew nothing), and its top third looked like the bottom. An old show with a clip in the middle third now shows the fractal.
- R2 Band Tower "Reflection": never drew anything.
- R3 Spectrum Landscape "Smoothing": never did anything.

Fixed (only when you move the knob; starting pictures are unchanged):
- Cross Section always cuts through the object on Mandelbulb, Apollonian 3D, Julia Set 3D, Menger Sponge and Burning Ship 3D.
- The bottom of Tetrahedron / IFS "Iterations" now shows the fractal.
- Mandelbrot "Power" works above 1/4.
- The top of Newton "Power" stops at 6 roots (7-8 were black).
- Astral Grid "Warp" now bends the grid sideways, with the bass.
- NEW (E3): the bottom of Spirograph and Lissajous "Thickness" now draws a visible hairline (it read as black on a projector).

Knobs that act only in another mode (the tests open that mode): Wireframe Wolf / Icosahedron "Density", Moire "Offset Y", Shape Generator "Rotation".

### 11. Found, not fixed
- `reaction_diffusion` Diffusion A (`tests/visual/test_sources.py:100-130` ladder; `src/sources/SourceRegistry.cpp:116`): 1/1 base run failed at PSNR 55.1 (limit 55) and 1/1 lane run passed. It is a stateful single-step PSNR, so the result depends on the session's history. There is no flake verdict (needs 5 runs per arm, about 1 h of lock). Harmony's call.
- kifs Fold Type is gone, but the upper-third mode (branch 3, `EmbeddedShaders.h:7040-7044`) was dead code: `p.x + p.y < 0.0` is never true after `p = abs(p)`. Now unreachable (uniform 0).
- mandelbrot Power 0-0.24 is one picture (the z^2 branch runs up to power 2.5, `EmbeddedShaders.h:2963`).
- `test_no_discontinuities` (`tests/visual/test_sources.py`) only warns: 190+ warnings on the lane run, pre-existing.
- 3 tracked `tests/visual/__pycache__/*.pyc` files are rewritten by any pytest without PYTHONDONTWRITEBYTECODE (repo hygiene; untouched here).
- lissajous at exactly t = 0 at 1920x1080: the default is at half brightness (p99.5 29) and Thickness 0 at p99.5 0. The curve samples coincide at that phase (B4 family).

### 12. Notes for .harmony/notebook.md (Harmony appends)
- 2026-09-28: test a shader edit HEADLESS before touching EmbeddedShaders.h. In `tests/test_source_defaults_gl.cpp` a temporary case can take `std::string(EmbeddedShaders::sourceX)`, string-replace the candidate line, and compare Rig::render pixels of the stored default (today vs variant) and of the knob extreme. That takes seconds, with no app build or lock. It caught a plan's "unreachable at the default" ternary that moved 488 bytes of the default render (Pitfall 49). | Files: tests/test_source_defaults_gl.cpp, src/render/EmbeddedShaders.h.
- 2026-09-28: stateful sources (cellular_automata, reaction_diffusion, strange_attractor, projectM) are never byte-identity candidates in an M2-style sweep. Each capture on the same app advances the simulation (cellular_automata: 10 distinct hashes over 10 captures), so equality between two runs is chance. Compare them by PSNR against their own run-to-run floor. | Files: src/sources/ProceduralSource.cpp.
- 2026-09-28: the full `tests/visual/test_sources.py` takes about 11.5 min (test_no_discontinuities dominates). Hold the live lock for it alone; test_effects + audio + time + perf together take about 5 min. | Files: tests/visual/test_sources.py.
- 2026-09-28: to commit one hunk of a file that has other uncommitted edits (no interactive add): write the wanted version to a scratch file, then `git hash-object -w` it and `git update-index --cacheinfo 100644,<sha>,<path>`. The working tree is untouched. | Files: any.

### FILES CHANGED
- `src/render/EmbeddedShaders.h`: 12 one-line in-place edits (section 3).
- `src/sources/SourceRegistry.cpp`: 3 addParam lines replaced by comments (the comments contain no "addParam(" or "source_" substring).
- `tests/test_source_defaults_gl.cpp`: header line plus 6 `[source-extremes]` cases and `requireNotBlack`.
- `tests/test_shader_param_lint.cpp`: DEBT_FILED example reworded (E6).
- `tests/visual/tier1_exceptions.py`: docstring; value-keyed BLACK_AT_EXTREME (+2); tetra out of the loop plus own entries (incl. Trail Fade); 5 GATED entries; AWAITING_RULING (6); SWEEP_AWAITING_RULING (8).
- `tests/visual/test_sources.py`: AWAITING probe; value-keyed BLACK_AT_EXTREME lookup; TestExceptionTables.
- `tests/visual/test_fractals.py`: docstring; 2 classes retired; `sweep()` strict xfails; 256 / is_black; Fold Type tuple removed.
- `tests/visual/conftest.py`: TIER1_CANVAS_256 gains test_fractals.
- Docs: `docs/claude/pitfalls.md` (49, 50), `CLAUDE.md` (index 49/50; the Common Build Issues table moved out, 23,995 -> 23,984 bytes), `docs/claude/build-other-platforms.md`, `docs/claude/fractals.md`, `docs/claude/architecture.md`, `tests/visual/SHADER_VERIFICATION.md`, `.harmony/APP-INVENTORY.md`, `.harmony/FEATURES.md`.

### SLIM CHECK
Cut: the planned SPARSE_AT_EXTREME table, SPARSE_MIN_LIT and `lit_fraction`. Once E3 made Thickness 0 an app fix, they had no entries (they would have been EXCESS_SPEC). Nothing else to cut: every entry traces to a D1 line or an adoption ruling.

### RISKS
- The tetra Iterations 0 margin at 1920x1080 in silence is p99.5 17 against a floor of 16. The ctest is deterministic on this GPU; another GPU could land under it.
- The Cross Section factors come from a t = 1.13 ladder, as in the plan. Other auto-rotation times can dim any slice (pre-existing).
- Pitfall numbers 49/50 and the CLAUDE.md index may collide with the renderleft / restore lanes at merge.
- The julia AWAITING probe depends on FEATURES_ACTIVE's rms 0.6 (plan risk; it fails loudly as "stale" if the fixture changes).

### KNOWLEDGE CONTEXT
- Tools used: grep plus headless GL renders; no graph queries.
- Impact authority: grep (not authoritative). The edits are in-shader and registry-only, so I took a conservative posture and proved byte-identity dynamically (M2/M3).
- God nodes in scope: none queried. Risk level: NORMAL. Dependencies discovered: `registryParams` counts the "addParam(" text, so the removal comments avoid it (verified).

### PACKET QUALITY
- Clarity: CLEAR (the plan plus the adoption section).
- Missing context: the plan's ternary guard was not byte-identical (measured, section 3). The plan also assumed the Cross Section factors would pass at 1080 (two needed the 0.1 rule).
- Unused context: the plan's SPARSE_AT_EXTREME machinery (superseded by E3).
- Self-brief files: plan-tier1.md, tier1-residual-diag.md, attack-tier1-*.md, CLAUDE.md, tier1diag scripts (eyes.py reused): all useful. I did not read `.harmony/notebook.md` beyond the plan's citations.

### Rig incidents (disclosed)
- One command ran `cd /private/tmp` before a pytest `--collect-only` (a no-cd rule breach; harmless, since all paths were absolute and no app was involved).
- A graphify post-commit hook launched background rebuilds on each commit (repo hook, not started by me).

### STATUS
DONE

### NEXT ACTION
Harmony: behavioral gate + Reviewer; merge `lane/tier1-0928` (merge-base 734f011; main is at 450f669, docs only); put the Boris list (section 10, E1 on top) in front of Boris.
