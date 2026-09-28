# Plan: close the Tier-1 residual (30 lines) and the test_fractals smoke -- s-rta-0928 lane V

Author: Architect · Base: main 6db8d67 (code 233eae7) · Inputs: DIAG = `.harmony/.reports/s-rta-0928/tier1-residual-diag.md`,
`.harmony/.reports/s-rta-0927/followups.md` F1.C-F1.E, `tests/visual/tier1_exceptions.py`,
`.harmony/.reports/s-rta-0927/plan-source-defects.md` §APPROACH / A2. ES = `src/render/EmbeddedShaders.h`,
SR = `src/sources/SourceRegistry.cpp`, line numbers at 6db8d67. Every ES edit below replaces one line in place (no line
count change), so no ES citation anywhere (tier1_exceptions reasons, pitfalls, docs) shifts.

QUESTION: For each of the 30 residual lines of `test_sources::test_all_params_have_effect` and for
`tests/visual/test_fractals.py` (113 F / 161 P, 5x identical): what does the lane change (app fix / removal / harness
entry / deferred-to-Boris entry), in what order, proven by which RED-first test, with every stored default rendering
byte-identical -- and what goes on Boris's list.

APPROACH (verdict first):
- 10 lines get an app fix: 9 one-line shader fixes whose stored default is provably untouched (5 Cross Section factors,
  2 Iterations floors, the mandelbrot z = 0 NaN guard, the newton power cap) + 1 one-line implement (astral_grid Warp).
  RED-first in the existing headless GL ctest `tests/test_source_defaults_gl.cpp` (5 new `[source-extremes]` cases,
  RED on today's shaders, run in silence = stricter than Tier-1's rms 0.6).
- 3 controls are removed (kifs Fold Type, band_tower Reflection, spectrum_landscape Smoothing): registry-only, the
  shader untouched, the uniform keeps its link-time 0 which equals the stored default or is never observable ->
  byte-identical by construction; old files reconcile through the existing `compload::reconcileSourceParams`.
- 7 harness gaps get exact GATED entries (DIAG D3/D4, each replayed with the exact Tier-1 logic).
- The 4 "legitimate vanishing" lines split: kifs Offset 0 and infinite_corridor Light 0 meet BLACK_AT_EXTREME's
  admission rule AS WRITTEN (rule text unchanged); spirograph / lissajous Thickness 0 do not (they draw) -> a new,
  narrower table SPARSE_AT_EXTREME that replaces the p99.5 check with a positive lit-pixel check (not a skip).
- 6 lines are defects with no byte-identical one-line fix (3 zoom-into-the-set design lines, 2 point-cloud slices,
  the spirograph sampling alias) -> a new STRICT table AWAITING_RULING (the listed value must still be black; a fix
  makes the entry stale -> red until removed) + Boris list B1 / B3 / B4. Standing default honoured ("exception entry
  with a reason"), kept apart from the legitimate tables.
- Two deliberate deviations from DIAG, both on evidence: (1) kifs Cross Section is NOT given the 1.8 factor -- DIAG's
  own ladder shows kifs slices are faint (inside the object: 9 of 24 ladder values black, 12 at p99.5 16-22, 3 at
  70-79); the factor would land the end on a p99.5 ~17 slice (~13 in silence: black) -- a metric pass, not a picture.
  Deferred with sierpinski_tetra (same point-cloud DE root, B3). (2) The Iterations fixes are floors on today's
  mapping, not DIAG's linear remaps: identical to today over the whole upper range, and tetra keeps its 15 folds,
  which the tetra slice gate (D4 #27/#28, measured at Iterations 1.0 = 15 folds) depends on (DIAG's
  `int(x*4)+7` caps at 11 and would change that gate's picture).
- test_fractals: retire the 2 classes Tier-1 duplicates (173 ids), port the 6 unique classes (101 ids) onto Tier-1
  conditions (is_black, 256x256 composition, explicit silence); the known-black sweep ids (predicted 8, confirmed on
  the lane's first run before marking) become strict xfails from SWEEP_AWAITING_RULING. Predicted `93 passed, 8 xfailed`.

TRADEOFFS CONSIDERED:
- Zoom lines #15-17: DIAG option (A) `center = mix(center, B, 1 - exp(-zoomExp))` -- REJECTED as a default. It is
  byte-identical at Zoom 0, but it rewrites what Center X/Y and Dive mean at depth (the zoom converges on B, Center
  becomes a screen-relative pan): a documented control contract (docs/claude/pitfalls.md:27-29 Pitfalls 9/10,
  docs/claude/fractals.md "Design rules" 2), not a range/math defect. (B) new default center -- not byte-identical.
  Both go to Boris (B1); the tests list the lines strictly meanwhile.
- julia #16 clamp `zoomExp = min(zoomExp, 2.1)` -- REJECTED: trades a black range for a dead range (the knob flat
  above 0.35).
- Keep the 6 deferred lines red (the module's old rule) vs a strict ledger -- LEDGER: the standing default asks for
  exception entries; a permanently red id buries any NEW regression among 30 known lines; strictness makes a stale
  entry fail, so the ledger cannot outlive its defect.
- One VANISHES_AT_EXTREME table for all four (a) lines -- REJECTED: it would blur "provably nothing drawn" (a skip is
  safe) with "drawn under the floor" (must still be proven drawn). Two tables, two proofs.
- Iterations: DIAG linear remaps vs `max(today, floor line)` -- floor chosen (see APPROACH). A plain `max(today, 7)`
  was rejected: it flattens the bottom third of tetra's knob to one picture.
- mandelbrot guard as an `else if` branch vs DIAG's in-place ternary -- ternary: one line, no ES line shift; GLSL
  evaluates only the selected operand, so the NaN of the other cannot leak (a real branch is the fallback if the
  ctest stays RED).
- test_fractals: fix in place (re-implement ladder/gates/metric) vs retire duplicates + port sweeps -- port (DIAG D5).
- Newton cap n <= 6 (DIAG) vs n <= 7 -- n <= 6: n = 7 still shows the black centre disk (lit 12.64 %, same
  overflow); the cap changes only knob values >= 0.894.

DECISION / SPEC

## 1. Per-line decisions (the 30 DIAG D1 lines; one action each)

| # | line (DIAG) | action | reason / evidence | proof |
|---|---|---|---|---|
| 1 | mandelbulb Cross Section @0.0 | APP ES:6757 `* 3.0` -> `* 1.5` | slab swept +-1.5; object z -0.78..+0.96 (ladder lit 0.24-0.82 at x3) | ctest `3D fractal Cross Section cuts the object...` RED->GREEN; line gone |
| 2 | apollonian_3d same | APP ES:7853 `* 4.0` -> `* 1.8` | object +-0.96 (lit 0.26-0.74 at x4) | same |
| 3 | sierpinski_tetra same | AWAITING_RULING (0.0), B3 | point-cloud DE: black at all 50 values, max p99.5 7; draws only with the camera in and the IFS deep (Zoom 0.7 + Iterations 1.0: 138 at 0.55) | strict probe (still black) |
| 4 | julia_set_3d same | APP ES:7258 `* 3.0` -> `* 1.7` | lit 0.20-0.82 at x3 (-0.90..+0.96) | ctest |
| 5 | kifs same | AWAITING_RULING (0.0), B3 (DIAG deviation) | faint slices (above); the factor end would be p99.5 ~17 (~13 silence) | strict probe |
| 6 | menger_sponge same | APP ES:6918 `* 3.0` -> `* 2.0` | lit 0.16-0.84 at x3 (the cube's faces +-1.02) | ctest |
| 7 | burning_ship_3d same | APP ES:7409 `* 3.0` -> `* 1.3` | lit 0.28-0.82 at x3 (-0.66..+0.96) | ctest |
| 8 | spirograph Inner Radius @0.0 | AWAITING_RULING (0.0), B4 | ES:6015-6019 300 point samples / 10 turns alias to 30q dots at (R-r)/r = p/q, q <= 10: black at 8 of 25 values incl. 0.25/0.5/0.75; the fix (segments) changes the default look | strict probe |
| 9 | spirograph Thickness @0.0 | SPARSE_AT_EXTREME | drawn: lit 0.28 % (256) / 0.15 % (1080), p99.5 0 | Tier-1: lit >= 0.05 % checked; ladder passes at 0.5 |
| 10 | lissajous Thickness @0.0 | SPARSE_AT_EXTREME | lit 0.29 % / 0.16 % | same |
| 11 | sierpinski_tetra Iterations @0.0 | APP ES:7690 floor | 3 folds p99.5 2; 7 folds 25; 8 folds 39 (256, rms 0.6) | ctest `sierpinski_tetra and kifs draw at Iterations 0` |
| 12 | kifs Iterations @0.0 | APP ES:7063 floor | 3 folds 7; 4 -> 22; 5 -> 56 | same |
| 13 | kifs Fold Type @0.5 | REMOVE SR:522 | 0.33-0.67 black at every t / Iterations / Offset / Zoom tried (15 renders); >= 0.67 same hash as 0.0 | `/api/sources`; M2 sha; line gone |
| 14 | kifs Offset @0.0 | BLACK_AT_EXTREME | offset 0 -> kifsDE(p) = length(p) exactly (the fold keeps length): the fractal is one point; lit 0.0 % at 256 and 1080 | Tier-1: listed, passes at 0.25 (p99.5 >= 202) |
| 15 | burning_ship Zoom @1.0 | AWAITING_RULING (1.0), B1 | default center inside the ship body; black from 0.30 (70 %) | strict probe |
| 16 | julia_set Zoom @1.0 | AWAITING_RULING (1.0), B1 | target = origin (critical point); rms 0.6 moves c into M; silence 90 % lit | strict probe |
| 17 | mandelbrot Zoom @1.0 | AWAITING_RULING (1.0), B1 | default center (-0.5, 0) inside the cardioid; black from 0.25 (75 %) | strict probe |
| 18 | mandelbrot Power @1.0 | APP ES:2969 z == 0 guard | Mandelbrot mode z0 = 0: atan(0,0) NaN, never escapes; Julia mode lit at every power | ctest `mandelbrot Power 2.5-4 draws...` |
| 19 | newton_fractal Power @1.0 | APP ES:6466 cap n <= 6 | n = 8 near the pole overflows float32 (port 0 % finite); n = 6 lit 63 % | ctest `newton_fractal Power at the top...` |
| 20 | infinite_corridor Light Intensity @0.0 | BLACK_AT_EXTREME | knob terms 0; remainder <= 2.65/255 (p99.5 2, lit 0.0 %) | Tier-1: listed, passes at 0.25 (p99.5 52) |
| 21 | astral_grid Warp | IMPLEMENT ES:9414 | `yWarp` (ES:9411) computed, never read; 3 t x 3 values one hash each | ctest `astral_grid Warp changes the picture` |
| 22 | band_tower Reflection | REMOVE SR:1003 | ES:10723 `uv.y < 0.0` unreachable; a working reflection would draw at the stored 0.3 (not byte-identical) | `/api/sources`; M2 |
| 23 | spectrum_landscape Smoothing | REMOVE SR:987 | ES:10594 `mix(energy, energy, s)`; smoothing needs history state | `/api/sources`; M2 |
| 24 | wire_wolf Density | GATED {Shape 0.072} | ES:5386 gridN read only by shapes 0-4 (ES:5395-5519) | DIAG replay PASS via 0.0 (PSNR 11.7); Tier-1 |
| 25 | wire_icosahedron Density | GATED {Shape 0.072} | same (own shape 5) | PASS 11.7 |
| 26 | moire_interference Offset Y | GATED {Pattern 0.5} | ES:9032 Lines reads p.x only; ES:9058-9059 Y offset | PASS 13.2 |
| 27 | sierpinski_tetra Slice Count | GATED _TETRA | its slice is B3-invisible at its own camera | PASS via 1.0 (20.5) |
| 28 | sierpinski_tetra Slice Distance | GATED _TETRA + count 0.5 | same | PASS via 0.0 (21.2) |
| 29 | newton_3d Trail Fade | GATED {Trail Distance 0.6 or 1.0} | ES:7572 numTrails | PASS 50.9 at 0.6; S6a picks the wider |
| 30 | shape_generator Rotation | GATED {Shape 0.2} | ES:4960 default circle is rotation-invariant | PASS via 0.1 (13.6) |

Totals: app 10 (9 fixes + 1 implement), remove 3, BLACK_AT_EXTREME 2, SPARSE_AT_EXTREME 2, GATED 7, AWAITING_RULING 6 = 30.

## 2. Exception-table rulings (tests/visual/tier1_exceptions.py)
- BLACK_AT_EXTREME: rules UNCHANGED ((a) the param's shader term provably zero-output at that value, (b) the ladder
  proves an effect elsewhere). #20 meets (a) literally (both knob terms are `* u_src_light_intensity`; the remainder is
  bounded < 16). #14 meets (a): with offset 0 the IFS is provably the single point 0 (kifsDE = length(p)); measured lit
  0.0 % at both sizes. Pre-existing 4 entries untouched.
- SPARSE_AT_EXTREME (new): the frame is NOT black -- a hairline covering < 0.5 % of pixels, under p99.5's floor. The
  params test does not skip it: it requires `lit_fraction >= SPARSE_MIN_LIT` (0.05 %) at the extreme, so an empty frame
  still fails "goes black". Admission: (a) the value's own meaning is the thinnest stroke, (b) lit measured > 0 at 256
  AND 1080, (c) the ladder proves an effect elsewhere.
- AWAITING_RULING / SWEEP_AWAITING_RULING (new): KNOWN DEFECTS waiting for Boris, never legitimate. Strict: the
  recorded value must still render black under the test's own conditions; the moment a fix lands, the test fails
  "stale" until the entry is removed. The module docstring's "an app defect is never listed here" becomes: "never
  admitted as legitimate; a defect with no byte-identical one-line fix, waiting for Boris, sits only in the strict
  AWAITING tables and on the Boris list".
- No VANISHES_AT_EXTREME table (see TRADEOFFS).

## 3. test_fractals.py ruling (DIAG D5)
| class | ids | action | reason / evidence |
|---|---|---|---|
| TestSourceLoadsAndRenders | 15 | RETIRE | Tier-1 `test_all_sources_non_black` renders all 108 sources (these 15 incl.) with is_black, FEATURES_ACTIVE, T_RETRY; D5: its 4 failures were the mean > 5 metric |
| TestEveryParamHasEffect | 158 | RETIRE | Tier-1 `test_all_params_have_effect` covers every registered param of these sources (+ the 11 this list never tested: 9 Feedback, julia_set_3d Iterations, 2 Dive Speed) with ladder, gates, audio, 256; D5: of its failures 81 were the metric, 10 gate gaps, 3 test-value / stale / weak, the app ones are all Tier-1 lines or B1. One check is lost and named: julia C Real 0.8 (c = (0.2, 0.27) inside M's cardioid fills the frame, numpy 100 % interior at zoom 0.1) -> B1 family on the Boris list |
| TestZoomLooping | 30 | PORT | is_black, 256, silence, t 0; strict xfails: mandelbrot 0.25/0.5/0.75/1.0, burning_ship 0.5/0.75/1.0 (B1) |
| TestDiveSpeedNotBlack | 20 | PORT | t 5, silence; strict xfail: julia_set 0.7 (B1: dive path through M's cardioid) |
| TestPowerNotBlack | 5 | PORT | now the regression guard for #18 (4 ids black before the fix) |
| Test3DZoomRange | 28 | PORT | its 12 failures were the metric (D5: pass with is_black) |
| TestPaletteVariety | 15 | PORT | capture size only |
| TestSourceRegistry | 3 | KEEP as is | no render |
No threshold is relaxed silently: the metric change (mean > 5 -> is_black) is Tier-1's H3 (sparse line art has mean
1-5 but a bright top 0.5 %); every retired id and every xfail carries its reason in the file.
`ALL_PARAM_TESTS` and the per-source tuple lists STAY as data: `tests/visual/test_range_quality.py:31` imports
`ALL_PARAM_TESTS` (Tier-2 tool); only the kifs Fold Type tuple (test_fractals.py:152) is deleted (param removed).

## 4. Ordered steps
Rig rules for every live step (DIAG rig log; `.harmony/notebook.md:1662-1663`; docs/claude/testing-eyes.md): lock
`/tmp/audiodna-live.lock` via Harmony's shared lock helper, each hold < 20 min, >= 45 s between holds; launch only
`open -g <app> --args --test-mode`, quit gracefully, after each hold print `Output-named 0`; 8080 is `localhost`
(IPv6); pytest only per FILE as `PYTHONDONTWRITEBYTECODE=1 AUDIODNA_NO_SPAWN=1 pytest -p no:cacheprovider <file>`
(tracked pyc files otherwise get rewritten); never `pytest tests/visual`, never test_output_window_level.py, no
Output window, no input synthesis, no screen capture.

S0 Setup. Worktree + branch `lane/tier1-residual-0928` from 6db8d67; `cmake -B build-lane -DCMAKE_BUILD_TYPE=Release
-DAUDIODNA_BUILD_TEST_SERVER=ON && cmake --build build-lane -j8`; `ctest --test-dir build-lane -j1` at base (record
the count). BEFORE app = this base build (not main's `build/`: other lanes may rebuild it).

S1 BEFORE captures (hold H1, base app). Scratch script (pattern: DIAG `tier1diag/eyes.py` + `exp_b1.py` if still on
disk): (a) M2 sweep TWICE; (b) M3 set; (c) the before-frames of section 5 G6. Save rows.jsonl (params, t, size,
p99.5, mean, lit, pixel sha256) + PNGs.

S2 Commit C1 = RED ctest. Append to `tests/test_source_defaults_gl.cpp` after :454, and add to its header comment
(:1-11) "and (s-rta-0928) the knob extremes whose black was a range / math defect":
```cpp
// ---- s-rta-0928 plan-tier1: knob EXTREMES whose black frame was a range / math defect (tier1-residual-diag D2) ----
// This rig leaves every audio uniform 0 (silence); each source below scales its colour by 0.7-0.8 + rms * k, so silence
// is DARKER than Tier-1's injected rms 0.6 and a pass here implies the Tier-1 line passes. "Not black" is
// tests/visual/tier1_exceptions.is_black's floor; slices and sparse folds light < 5 % of the frame, so these cases do
// not also demand requireVisible's 5 % lit.
namespace
{
void requireNotBlack(Rig& rig, const std::string& what, const char* frag, int w, int h, float t,
                     const std::vector<Param>& ps)
{
    const Stats s = stats(rig.render(frag, w, h, t, ps), w, h);
    INFO(what << " " << w << "x" << h << " t=" << t << ": mean=" << s.mean << " p99.5=" << s.p995 << " lit=" << s.lit);
    CHECK(s.p995 >= 16.0);
}
} // namespace

// Cross Section's 0.04-thick slab swept +-1.5 (+-2.0) of world z, past the objects (z -0.66..+1.02): both ends of the
// knob cut empty space. The per-source factor puts both ends inside the object.
TEST_CASE("3D fractal Cross Section cuts the object at both ends of the knob", "[source-extremes][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const std::pair<const char*, const char*> cases[] = {
        { "mandelbulb", EmbeddedShaders::sourceMandelbulb },   { "apollonian_3d", EmbeddedShaders::sourceApollonian3D },
        { "julia_set_3d", EmbeddedShaders::sourceJuliaSet3D }, { "menger_sponge", EmbeddedShaders::sourceMengerSponge },
        { "burning_ship_3d", EmbeddedShaders::sourceBurningShip3D } };
    for (const auto& [id, frag] : cases)
    {
        const auto ps = registryParams(id);
        for (auto [w, h] : kSizes)
            for (float v : { 0.0f, 0.25f, 0.75f, 1.0f })
                requireNotBlack(rig, std::string(id) + " Cross Section " + std::to_string(v), frag, w, h, 1.13f,
                                with(ps, "u_src_slice", v));
    }
}

// A point-cloud IFS DE (length(z) * scale^-n; a ray draws only within 0.001 of a speck) shows nothing below a few
// folds: Iterations 0 was 3 folds (sierpinski_tetra p99.5 2, kifs 7 at 256x256).
TEST_CASE("sierpinski_tetra and kifs draw at Iterations 0", "[source-extremes][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const std::pair<const char*, const char*> cases[] = { { "sierpinski_tetra", EmbeddedShaders::sourceSierpinskiTetra },
                                                          { "kifs", EmbeddedShaders::sourceKIFS } };
    for (const auto& [id, frag] : cases)
    {
        const auto ps = registryParams(id);
        for (auto [w, h] : kSizes)
            requireNotBlack(rig, std::string(id) + " Iterations 0", frag, w, h, 1.13f, with(ps, "u_src_iterations", 0.0f));
    }
}

// Mandelbrot mode starts at z = 0, where the polar power step's atan(0, 0) is undefined in GLSL (NaN on this GPU): no
// pixel ever escaped, so Power >= 0.26 (power >= 2.5) was black. Julia mode (z = uv) never met z = 0.
TEST_CASE("mandelbrot Power 2.5-4 draws in Mandelbrot mode", "[source-extremes][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("mandelbrot");
    for (auto [w, h] : kSizes)
        for (float v : { 0.25f, 0.5f, 0.75f, 1.0f })
            requireVisible(rig, ("mandelbrot Power " + std::to_string(v)).c_str(), EmbeddedShaders::sourceMandelbrot,
                           w, h, 1.13f, with(ps, "u_src_power", v));
}

// n = 7 / 8 roots: the first Newton step from near the pole z = 0 throws z to ~1e20 and cdiv's dot(b, b) overflows
// float32 (NaN): the default Zoom 0.3 frame was black at n = 8 (256x256). Power now stops at n = 6.
TEST_CASE("newton_fractal Power at the top of the knob draws", "[source-extremes][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("newton_fractal");
    for (auto [w, h] : kSizes)
        for (float v : { 0.9f, 1.0f })
            requireVisible(rig, ("newton_fractal Power " + std::to_string(v)).c_str(),
                           EmbeddedShaders::sourceNewtonFractal, w, h, 1.13f, with(ps, "u_src_power", v));
}

// astral_grid computed a Warp offset (yWarp) that nothing read; Warp now waves the grid lines sideways.
TEST_CASE("astral_grid Warp changes the picture", "[source-extremes][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("astral_grid");
    const Pixels dflt = rig.render(EmbeddedShaders::sourceAstralGrid, 256, 256, 1.13f, ps);
    const double q = psnr(dflt, rig.render(EmbeddedShaders::sourceAstralGrid, 256, 256, 1.13f,
                                           with(ps, "u_src_warp", 1.0f)));
    INFO("astral_grid Warp 0 -> 1 at 256x256 t=1.13: PSNR " << q);
    CHECK(q < 55.0);
}
```
Build; run `build-lane/tests/test_source_defaults_gl "[source-extremes]"` on today's shaders: all 5 cases RED (expect
p99.5 0 at the Cross Section ends and mandelbrot powers, 2 / ~5 for Iterations 0, newton 1.0 black at 256, Warp
PSNR 1e9). Save the output verbatim. Commit.

S3 Commit C2 = GREEN shader edits (ES, by LINE -- ES:6757/6918/7258/7409/7702 carry identical text; tetra :7702 and
kifs :7077 must NOT change):
- :6757 mandelbulb `float sliceZ = (u_src_slice - 0.5) * 3.0;` -> `* 1.5;` (+ end comment `// ends inside the object (s-rta-0928)`)
- :6918 menger `* 3.0` -> `* 2.0`;  :7258 julia_set_3d `* 3.0` -> `* 1.7`;  :7409 burning_ship_3d `* 3.0` -> `* 1.3`;
  :7853 apollonian_3d `* 4.0` -> `* 1.8`.
  Decision rule: if a Cross Section case fails for a source, lower THAT factor by 0.1 and re-run until it passes at
  both sizes; if it still fails at 1.0, stop: that source joins AWAITING_RULING B3 with the measured p99.5 (report it).
- :7690 tetra `int iters = int(u_src_iterations * 12.0) + 3;` ->
  `int iters = max(int(u_src_iterations * 12.0) + 3, int(u_src_iterations * 4.0) + 7);   // >= 7 folds (point-cloud DE); 9 at 0.5 as before`
  Rule: if Iterations 0 fails at 1920x1080, use the floor line `int(u_src_iterations * 2.0) + 8` (0.5 still -> 9).
- :7063 kifs `int iters = int(u_src_iterations * 10.0) + 3;` ->
  `int iters = max(int(u_src_iterations * 10.0) + 3, int(u_src_iterations * 5.0) + 5);   // >= 5 folds; 7 at 0.4 as before`
- :2969 mandelbrot `z = rn * vec2(cos(power * theta), sin(power * theta)) + c;` ->
  `z = (dot(z, z) == 0.0) ? c : rn * vec2(cos(power * theta), sin(power * theta)) + c;   // 0^p = 0: atan(0, 0) is NaN here`
  Rule: if the ctest stays RED, replace the ternary by `} else if (dot(z, z) == 0.0) { z = c; } else {` around the
  polar block (then re-check every ES citation below :2969 for a shift).
- :6466 newton `int n = int(rawPow * rawPow * 5.0) + 3;` -> `int n = int(min(rawPow * rawPow * 5.0, 3.0)) + 3;   // n <= 6: 7-8 overflow near the pole`
- :9414 astral `vec2 gp = vec2(x, z) / gridS;` -> `vec2 gp = vec2(x + yWarp, z) / gridS;   // Warp: the grid lines wave sideways`
- If mandelbrot at power 2.5 draws but lights < 5 % (requireVisible fails on lit only), use requireNotBlack for that
  value and name the lit % in the report.
Run the `[source-extremes]` cases GREEN, then the FULL `ctest --test-dir build-lane -j1` GREEN. Commit.

S4 Commit C3 = removals. SR, each line replaced by a comment (no `addParam(` or `source_` substring in the comment:
`tests/test_source_defaults_gl.cpp:50-71` (registryParams) and the lint count those):
- SR:522 `s->addParam("Fold Type", "u_src_fold_type", 0.0f);` -> `// "Fold Type" removed s-rta-0928: 0.33-0.67 drew black, >= 0.67 equalled the first fold; u_src_fold_type stays 0`
- SR:987 `s->addParam("Smoothing", "u_src_smoothing", 0.5f);` -> `// "Smoothing" removed s-rta-0928: mix(energy, energy, s) changed nothing (smoothing needs history)`
- SR:1003 `s->addParam("Reflection", "u_src_reflection", 0.3f);` -> `// "Reflection" removed s-rta-0928: it drew only where uv.y < 0.0, off screen`
- ES untouched (byte-identity by construction; the lint checks registered params only).
- `tests/visual/test_fractals.py:152` delete the kifs Fold Type tuple.
- `tests/test_shader_param_lint.cpp:12-15` DEBT_FILED comment: spectrum_landscape Smoothing "was the example -- removed
  s-rta-0928 after the Tier-1 sweep caught it"; keep the general DEBT note.
Build; full ctest GREEN (test_shader_param_lint, test_composition `[source-params]`, test_source_defaults_gl). Commit.

S5 Commit C4 = harness (tier1_exceptions.py, test_sources.py, conftest.py, test_fractals.py). SWEEP_AWAITING_RULING
is committed EMPTY here and filled in S6a from the observed run.
(a) tier1_exceptions.py docstring :1-8: "The 256x256 Tier-1 files (test_sources, test_effects, test_audio_reactivity,
test_time_sweep) and the fractal sweeps (test_fractals) import this ONE module ... An app defect is never admitted as
legitimate: a defect with no byte-identical one-line fix that waits for Boris sits only in the strict AWAITING tables
and on the Boris list; any other defect keeps its test red and is named in the lane report with file:line."
(b) after `is_black` (:19):
```python
def lit_fraction(path):
    """Share of pixels whose max channel is over 16 (the is_black floor): sees a thin drawn stroke that p99.5 cannot --
    a stroke on < 0.5 % of the frame leaves p99.5 at the background (SPARSE_AT_EXTREME)."""
    img = cv2.imread(path)
    return 0.0 if img is None else float((img.max(axis=2) > 16).mean())
```
(c) inside BLACK_AT_EXTREME (:82-92), two entries:
```python
    ("kifs", "u_src_offset"): "GLSL: EmbeddedShaders.h:7025 offset = vec3(1.0) * off * 2.0 = 0 at 0.0 -- each fold step "
                              "is then p -> sc * fold(p) (:7045) and the fold (abs + swaps, :7029-7033) keeps length(p), "
                              "so kifsDE (:7047) = length(p) exactly: the fractal is the single point 0 (lit 0.0 % at "
                              "256x256 and 1920x1080; s-rta-0928 tier1-residual-diag D2 #14)",
    ("infinite_corridor", "u_src_light_intensity"): "GLSL: EmbeddedShaders.h:9254 lightMask *= u_src_light_intensity "
                              "and :9274 diff * atten * u_src_light_intensity are 0 at 0.0; what is left, wallCol "
                              "(<= 0.2, :9263) * ambient 0.05 (:9258) * fog (<= 1) * (0.8 + rms 0.6 * 0.4), is <= 2.65 / "
                              "255 < 16 (p99.5 2, lit 0.0 %; diag D2 #20)",
```
(d) after BLACK_AT_EXTREME:
```python
# (source_id, uniform) -> reason: at that value the source still DRAWS, on < 0.5 % of the frame -- under is_black's
# p99.5 floor. Not skipped: the params test requires lit_fraction >= SPARSE_MIN_LIT there (an empty frame still fails
# "goes black"). Admission (all required): (a) the value's own meaning is the thinnest stroke, not a defect; (b)
# lit_fraction > 0 measured at 256x256 AND 1920x1080; (c) the ladder proved an effect at another candidate.
SPARSE_MIN_LIT = 0.0005   # 0.05 % of the pixels (33 at 256x256); the listed strokes measure 0.28-0.29 %
SPARSE_AT_EXTREME = {
    ("spirograph", "u_src_thickness"): "GLSL: EmbeddedShaders.h:6012 thick = 0.004 + u_src_thickness * 0.02 -- at 0.0 "
                                       "the thinnest stroke is drawn: lit 0.28 % at 256x256, 0.15 % at 1920x1080, p99.5 "
                                       "0 at both (s-rta-0928 tier1-residual-diag D2 #9)",
    ("lissajous", "u_src_thickness"): "GLSL: EmbeddedShaders.h:5978 thick = 0.005 + u_src_thickness * 0.03 -- at 0.0 the "
                                      "thinnest stroke is drawn: lit 0.29 % at 256x256, 0.16 % at 1920x1080, p99.5 0 "
                                      "(diag D2 #10)",
}
```
(e) after the torus_hole gate (:143-145), before DECK_ONLY_SOURCE_PARAMS (these override the loop's tetra entries):
```python
# s-rta-0928 (tier1-residual-diag D3 / D4, each replayed with the exact Tier-1 per-param logic):
_WIRE_GRID = ("EmbeddedShaders.h:5386 gridN = int(u_src_density * 20) + 4 is read only by the grid shapes 0-4 "
              "(:5395-5519, shape = int(u_src_shape * 6.99)); this preset's own shape ({}) is a fixed mesh -- the gate "
              "selects the sphere (Shape 0.072)")
GATED_SOURCE_PARAMS[("wire_wolf", "u_src_density")] = ({"u_src_shape": 0.072}, _WIRE_GRID.format("wolf, 6"))
GATED_SOURCE_PARAMS[("wire_icosahedron", "u_src_density")] = ({"u_src_shape": 0.072},
                                                              _WIRE_GRID.format("icosahedron, 5"))
GATED_SOURCE_PARAMS[("moire_interference", "u_src_offset_y")] = (
    {"u_src_pattern": 0.5}, "EmbeddedShaders.h:9032 the default Lines pattern reads only p.x and :9058-9059 add the Y "
                            "offset to layer 2's y -- invisible in Lines; Dots (0.5) and Rings (1.0) read y")
GATED_SOURCE_PARAMS[("shape_generator", "u_src_rotation")] = (
    {"u_src_shape": 0.2}, "EmbeddedShaders.h:4960 the default Shape 0 is sdCircle, which a rotation leaves unchanged; "
                          "the box (Shape 0.143-0.286) shows it")
GATED_SOURCE_PARAMS[("newton_3d", "u_src_trail_fade")] = ({"u_src_trail_dist": 0.6}, _TRAIL)   # S6a: 0.6 or 1.0
_TETRA = {"u_src_slice": 0.55, "u_src_zoom": 0.7, "u_src_iterations": 1.0}
_TETRA_WHY = ("; sierpinski_tetra's slice draws nothing at its own Zoom 0.3 / Iterations 0.5 (AWAITING_RULING Cross "
              "Section, B3), so the gate also moves the camera in (Zoom 0.7) and deepens the IFS (15 folds)")
GATED_SOURCE_PARAMS[("sierpinski_tetra", "u_src_slice_count")] = (dict(_TETRA), _SLICE + _TETRA_WHY)
GATED_SOURCE_PARAMS[("sierpinski_tetra", "u_src_slice_dist")] = ({**_TETRA, "u_src_slice_count": 0.5},
                                                                _MULTI + _TETRA_WHY)
```
(f) end of module:
```python
# KNOWN APP DEFECTS waiting for Boris (s-rta-0928 plan-tier1, Boris list B1 / B3 / B4). Not legitimate behaviour: each
# has no byte-identical one-line fix, so the fix is his call; listed so the gate stays usable while they wait. STRICT:
# the listed value must still render black -- once a fix lands the entry is stale and the test fails until removed.
_B1 = "B1 zoom target inside the set (Boris's call; plan-tier1): "
_B3 = "B3 slicing a point-cloud IFS (Boris's call; plan-tier1): "
_B4 = "B4 spirograph point samples (Boris's call; plan-tier1): "

# (source_id, uniform) -> (value, reason): test_sources renders `value` (defaults + that param, Tier-1 conditions)
# instead of walking the ladder, and fails if it no longer renders black.
AWAITING_RULING = {
    ("mandelbrot", "u_src_zoom"): (1.0, _B1 + "EmbeddedShaders.h:2934 the manual zoom target is Center X/Y, default "
        "(-0.5, 0) inside the main cardioid; :2940 zoomExp = zoom * 7: from Zoom 0.25 the frame is all interior, painted "
        "black (:2980-2985) -- 75 % of the knob (tier1-residual-diag D2 #17)"),
    ("burning_ship", "u_src_zoom"): (1.0, _B1 + "EmbeddedShaders.h:6385 default center (-0.75, -0.5) is inside the "
        "ship's body; :6391 zoomExp = zoom * 7: black (:6409) from Zoom 0.30 -- 70 % of the knob (D2 #15)"),
    ("julia_set", "u_src_zoom"): (1.0, _B1 + "EmbeddedShaders.h:6258-6261 the zoom target is the origin, the critical "
        "point; FEATURES_ACTIVE's rms 0.6 moves c (:6304-6305) to (-0.6973, 0.2818), inside M (the default c is 0.012 "
        "from M's edge), so the deep frame is all interior (:6322): black from Zoom 0.40 here, 90 % lit in silence "
        "(D2 #16)"),
    ("sierpinski_tetra", "u_src_slice"): (0.0, _B3 + "EmbeddedShaders.h:7675 is a point-cloud DE (4^iters specks, a "
        "ray draws within 0.001, :7745): the 0.04 slab (:7717) holds too few to draw at the source's own Zoom 0.3 / "
        "Iterations 0.5 -- black at all 50 Cross Section values, max p99.5 7 (D2 #3)"),
    ("kifs", "u_src_slice"): (0.0, _B3 + "EmbeddedShaders.h:7047 the same point-cloud DE: :7077 (slice - 0.5) * 4 puts "
        "26 of 50 ladder values in empty space, and inside the object 9 of 24 are black and 12 faint (p99.5 16-22); a "
        "range factor would land the end on a p99.5 ~17 slice (~13 in silence) -- not a picture (D2 #5; plan-tier1)"),
    ("spirograph", "u_src_inner"): (0.0, _B4 + "EmbeddedShaders.h:6015-6019 draws 300 point samples over 10 turns; at "
        "(R - r) / r = p/q with q <= 10 they coincide (Inner Radius 0: r 0.1, ratio 5 -> 30 dots): black at 8 of 25 "
        "values incl. 0.25 / 0.5 / 0.75 (D2 #8)"),
}

# (source_id, uniform, value) -> reason: test_fractals.py sweep ids black by a B1 defect at the sweep's own conditions
# (silence, 256x256). Each is marked pytest.mark.xfail(strict=True): it runs, reports XFAIL with this reason, and an
# unexpected pass FAILS the run -- remove the entry then. Filled from the lane's own first ported run (plan-tier1 S6a).
SWEEP_AWAITING_RULING = {}
```
S6a fills it with exactly the observed B1 ids; predicted (DIAG D5):
```python
_SW_MB = (_B1 + "mandelbrot's default center (-0.5, 0) (EmbeddedShaders.h:2934) is inside the main cardioid: from "
          "Zoom 0.25 (:2940) the frame is all interior, painted black (:2980-2985)")
_SW_BS = (_B1 + "burning_ship's default center (-0.75, -0.5) (EmbeddedShaders.h:6385) is inside the ship's body: "
          "black (:6409) from Zoom 0.30 (:6391)")
_SW_JD = (_B1 + "julia_set's Dive morphs c through the presets (EmbeddedShaders.h:6281-6290): at t = 5 / Dive 0.7 "
          "c = (-0.056, 0.489) is inside M's main cardioid while the dive zooms in on the origin (:6258): all "
          "interior (tier1-residual-diag D5, numpy 100 %)")
SWEEP_AWAITING_RULING = {
    **{("mandelbrot", "u_src_zoom", v): _SW_MB for v in (0.25, 0.5, 0.75, 1.0)},
    **{("burning_ship", "u_src_zoom", v): _SW_BS for v in (0.5, 0.75, 1.0)},
    ("julia_set", "u_src_dive_speed", 0.7): _SW_JD,
}
```
(g) `tests/visual/test_sources.py`: import (:18-20) adds `lit_fraction, SPARSE_AT_EXTREME, SPARSE_MIN_LIT,
AWAITING_RULING`. After the `render` closure (:84):
```python
                if key in AWAITING_RULING:
                    # A known defect waiting for Boris: its ladder is not walked, but it must still reproduce --
                    # a fixed defect leaves a stale entry, and a stale entry fails until it is removed.
                    value = AWAITING_RULING[key][0]
                    if not is_black(render({uniform: value}, T_PARAM, f"awaiting_{value:.2f}")):
                        failures.append(f"{src['id']}:{param['name']} AWAITING_RULING entry is stale: {value} no "
                                        f"longer renders black -- remove it (tier1_exceptions.py)")
                    continue
```
after the BLACK_AT_EXTREME `continue` (:109):
```python
                            if key in SPARSE_AT_EXTREME and lit_fraction(test_path) >= SPARSE_MIN_LIT:
                                tried.append(f"{test_val} (sparse {lit_fraction(test_path):.2%}, listed)")
                                continue
```
(h) `tests/visual/conftest.py:77-79`: set gains `"test_fractals"`; comment: "Tier-1 modules that capture at 256x256
(s-rta-0927 F1, tier1-diag H2) and the fractal sweeps ported onto them (s-rta-0928)"; fixture docstring :85-86 "the
four modules" -> "these modules".
(i) `tests/visual/test_fractals.py`: new docstring (retired classes and why -> Tier-1 test_sources; what stays;
conditions: 256x256 composition, is_black, SILENCE by design -- no arm_audio, conftest's reset clears the snapshot;
strict xfails from SWEEP_AWAITING_RULING). Imports: drop numpy and `frame_is_not_black`; add
`from tier1_exceptions import is_black, SWEEP_AWAITING_RULING`. Keep `frames_are_different`, the tuple lists and
`ALL_PARAM_TESTS` (comment: "data for test_range_quality.py (Tier 2); fields 2-3 are historical and unused") and
`ALL_SOURCES`. Delete TestSourceLoadsAndRenders (:263-272) and TestEveryParamHasEffect (:275-299). Add:
```python
SIZE = 256   # Tier-1 capture; conftest.TIER1_CANVAS_256 pins the composition to it for this module


def sweep(source_ids, uniform, values):
    """(source_id, value) params; an id listed in SWEEP_AWAITING_RULING is a strict xfail carrying its reason."""
    out = []
    for s in source_ids:
        for v in values:
            reason = SWEEP_AWAITING_RULING.get((s, uniform, v))
            out.append(pytest.param(s, v, id=f"{s}-{v}",
                                    marks=[pytest.mark.xfail(strict=True, reason=reason)] if reason else []))
    return out
```
and re-parametrize the four sweeps through it (same values and times as today), assert `not is_black(out)`, capture
`width=SIZE, height=SIZE`:
TestZoomLooping `sweep(ZOOM_SOURCES_2D, "u_src_zoom", [0.0, 0.25, 0.5, 0.75, 1.0])`, t 0.0 ·
TestDiveSpeedNotBlack `sweep(DIVE_SOURCES, "u_src_dive_speed", [0.1, 0.3, 0.5, 0.7, 1.0])`, t 5.0 ·
TestPowerNotBlack `sweep(["mandelbrot"], "u_src_power", [0.0, 0.25, 0.5, 0.75, 1.0])`, t 0.0 ·
Test3DZoomRange `sweep(SOURCES_3D, "u_src_zoom", [0.0, 0.3, 0.6, 0.9])`, t 1.0.
TestPaletteVariety: capture at SIZE, otherwise unchanged. TestSourceRegistry unchanged. Commit.

S6 AFTER, live on the lane app (holds H2-H6):
- H2 / S6a: (1) test_fractals with SWEEP_AWAITING_RULING empty -> record the failing ids; expected exactly the 8 above.
  Fill the dict with the observed B1 ids only. Any other failure: bin it with the PNG's p99.5 (harness -> fix; an app
  defect outside B1 -> a new Boris item + an entry with reason and evidence, named in the report; never an unexplained
  xfail). A TestPowerNotBlack failure means the S3 fix is incomplete -> back to S3. (2) scratch replay of the exact
  Tier-1 per-param logic (DIAG `runs/b2/tier1_logic.txt` pattern) for: the 7 GATED keys (print the PASS line); newton_3d
  Trail Fade at trail_dist 0.6 AND 1.0 -> keep the lower PSNR in the entry (both must pass <= 55); the 6 AWAITING probes
  (p99.5 printed, each < 16) plus one teeth row (mandelbrot zoom probed at 0.1 must print "stale"); the 2 SPARSE and 2 new
  BLACK_AT_EXTREME keys ("(sparse x.xx %, listed)" / "(black, listed)" then PASS). Offline teeth: `lit_fraction` of the
  saved spirograph Thickness-0 PNG >= SPARSE_MIN_LIT and of an all-black 256x256 PNG == 0.
- H3: `test_sources.py`. H4: `test_effects.py test_audio_reactivity.py test_time_sweep.py test_performance.py` (split
  further if a hold nears 20 min). H5: `test_fractals.py` (filled dict) + M2 AFTER + M3 AFTER + after-frames (G6) +
  `/api/sources` param listing (G7). H6: `.harmony/probe-effects-parity.sh` with `PARITY_APP=<build-lane app>` and
  `AUDIODNA_LOCK_OWNER` set.
- If main moved meanwhile: rebase, rebuild, re-run ctest + H3-H6 on the rebased head; M2 BEFORE is re-captured from a
  build of the new merge-base (the comparison is always lane head vs its own base).

S7 Commit C5 = docs:
- `tests/visual/SHADER_VERIFICATION.md`: harness-rules paragraph (:46-54) + one sentence on the tables
  (BLACK_AT_EXTREME provable nothing; SPARSE_AT_EXTREME hairline with the >= 0.05 % lit check; GATED; AWAITING_RULING /
  SWEEP_AWAITING_RULING strict, Boris-pending); :38-40 DEBT example updated (Smoothing removed); :243 test_fractals row
  -> "fractal range sweeps (zoom / dive / power / 3D camera), palettes, registry; 256x256, silence".
- `docs/claude/fractals.md`: :45-48 test_fractals description; :55 mandelbrot Notes -> "Power >= 2.5 = polar step, z = 0
  guarded; manual zoom target = Center X/Y (deep zoom at the default center is black: Boris B1)"; :58 newton "Power
  (3-6)"; :71 Cross Section range -> "ends at the object's edge: mandelbulb +-0.75, apollonian_3d +-0.9, julia_set_3d
  +-0.85, menger +-1.0, burning_ship_3d +-0.65 (final factors); kifs +-2.0 / sierpinski_tetra +-1.5 unchanged (dust
  slices, B3)"; :83 kifs "Scale, Iterations (5-13, 7 at 0.4), Offset"; :87 tetra "Iterations (7-15, 9 at 0.5)".
- `docs/claude/pitfalls.md` + CLAUDE.md index (next free numbers at merge; 48/49 if still free -- concurrent lanes):
  "A polar complex power at z = 0 is NaN on this GPU -- guard z == 0 before atan/pow (atan(0, 0) is undefined in GLSL;
  0 * NaN kept z NaN and `dot(z, z) > 4.0` is false for NaN, so mandelbrot Power >= 2.5 never escaped: 75 % of the knob
  black). Guard: test_source_defaults_gl [source-extremes]" and "A point-cloud IFS DE (length(z) * scale^-n, no radius)
  draws sub-pixel specks -- few folds or a thin slice show nothing; a new IFS source needs an iteration floor proven
  p99.5 >= 16 in silence at 256x256 and 1920x1080 (tetra >= 7, kifs >= 5); their slices stay faint (Boris B3)".
- `.harmony/APP-INVENTORY.md:27` and `docs/claude/architecture.md:205`: source param count RECOMPUTED from
  `/api/sources` (expected 681 -> 678; note the three removals). `.harmony/FEATURES.md:1030` test_fractals line.
  (`git add -f` for `.harmony/*`.) design/ and research/ inventories are dated snapshots: not edited.
Commit.

S8 Lane report: per-line table (RED line on base -> result on lane), ctest RED/GREEN verbatim, final factors and
floors, the S6a observed set, G1-G8 with verbatim summaries, M2/M3 tables, contact-sheet paths, drift list.

## 5. Gates (all on the lane head)
- G1 ctest `ctest --test-dir build-lane -j1`: 100 % pass incl. test_shader_param_lint, test_composition
  `[source-params]`, test_source_defaults_gl (the 5 new cases RED on C1 and GREEN on C2, outputs verbatim). Count =
  base + 5.
- G2 the five Tier-1 files: expected 13 / 13 PASSED; `test_all_params_have_effect` 0 lines. Any remaining line is named
  with file:line -- never auto-excepted.
- G3 test_fractals: expected `93 passed, 8 xfailed` (INFERRED from DIAG D5 at 512; S6a is the arbiter); 0 XPASS.
- G4 probe-effects-parity GREEN (no effect or compositor code changes; this proves it).
- G5 = M2 + M3 (section 6): every stored default byte-identical.
- G6 before/after frames + one contact sheet per group (before row over after row), Tier-1 conditions (256 composition,
  FEATURES_ACTIVE, t 1.13) plus 1920x1080 silence for the ends: Cross Section 0.0 / 0.25 / 0.75 / 1.0 on the 5 fixed
  sources; tetra Iterations 0.0 / 0.25 / 0.4; kifs Iterations 0.0 / 0.1 / 0.25; mandelbrot Power 0.25 / 0.5 / 0.75 / 1.0;
  newton Power 0.9 / 1.0; astral Warp 0.25 / 0.5 / 1.0 at t 1.13 and 2.0; kifs Fold Type 0.5 (before) next to kifs
  defaults (after) = what an old show's middle-third kifs clip now shows.
- G7 `/api/sources`: kifs has no Fold Type, band_tower no Reflection, spectrum_landscape no Smoothing; total recomputed.
- G8 screen safety: every hold ends with the app quit and `Output-named 0`; worktree `git status` clean of pyc changes.

## 6. Must-not-change: stored defaults render byte-identical
M1 by reading (VERIFIED unless marked):
| change | why the stored default cannot change |
|---|---|
| 5 Cross Section factors | `sliceZ` is read only inside `if (useSlice)` (ES:6774 / 6932 / 7272 / 7423 / 7866); `useSlice = abs(slice - 0.5) > 0.01` is false at the stored 0.5 |
| tetra Iterations | max(int(0.5 * 12) + 3, int(0.5 * 4) + 7) = max(9, 9); every operand exact; identical to today for x >= 0.4167 |
| kifs Iterations | 0.4f * 10 rounds to 4.0 and 0.4f * 5 to 2.0 in float32 -> max(7, 7); identical to today for x >= 0.3 |
| mandelbrot guard | the polar branch runs only for power >= 2.5; stored Power 0 -> power 2.0 -> the z^2 branch (ES:2963-2964) |
| newton cap | min(0.2^2 * 5, 3.0) returns its first operand -> n = 3; identical to today below Power 0.894 |
| astral Warp | stored Warp 0 -> yWarp = +-0 -> x + 0 == x (INFERRED for signed zero: x = (uv.x - 0.5) * depth is never -0) |
| kifs Fold Type removal | shader unchanged; `ProceduralSource.cpp:238-244` uploads only registered params, so the uniform keeps its link-time 0 = the stored 0.0 |
| band_tower Reflection removal | the uniform becomes 0 (was 0.3); the block it gates (ES:10721-10726) writes only when uv.y < 0.0, never true |
| spectrum_landscape Smoothing removal | mix(e, e, 0.0) and mix(e, e, 0.5) both equal e under either mix lowering (INFERRED; M3 decides) |
M2 dynamic: for every id in `/api/sources`: `/api/reset`, `load_source(id)` (seeds defaults, Pitfall 47), capture
256x256 and 1920x1080 at t 1.13 in silence, sha256 of the DECODED pixels. BEFORE twice (base build) = noise floor; ids
that differ between the two BEFORE runs (stateful sources, projectM, layer_router) are compared by PSNR >= 30 and
listed; every other id must be identical BEFORE vs AFTER.
M3: the 12 touched sources (mandelbulb, apollonian_3d, julia_set_3d, menger_sponge, burning_ship_3d, sierpinski_tetra,
kifs, mandelbrot, newton_fractal, astral_grid, band_tower, spectrum_landscape) at
defaults on a 256x256 composition with FEATURES_ACTIVE at t 0 / 1.13 / 5.13 / 10.13 (the time sweep's times):
identical BEFORE vs AFTER.
Any mismatch: STOP -- no LSB drift is accepted silently; bisect to the change and report PSNR + the differing pixels
to Harmony.

## 7. Boris list (plain words; the default ships if he says nothing)
Settings that go black (or nearly) that a VJ can reach, now listed as known in the tests:
1. Kaleidoscopic IFS "Offset" all the way down: the fractal shrinks to one point -- black. Default: keep (an "implode"
   move).
2. Infinite Corridor "Light Intensity" all the way down: lights off, about 1 % brightness. Default: keep.
3. Spirograph and Lissajous "Thickness" all the way down: a hairline of dots on 0.3 % of the screen -- reads as black on
   a projector. Default: keep (the test still checks it is drawn).
4. Unchanged from the last session: Fire Wall Density 0, Concentric Rings Width 0, Metaballs Size 0, Line Pattern
   Width 0.
Waiting for your call (the tests list them until you decide):
- B1 Deep zoom lands in the black inside of the fractal. Mandelbrot Zoom past 1/4, Burning Ship past 0.3 (at their
  starting centers), Julia Set past 0.4 on loud music (the sound nudges its shape inside); Mandelbrot / Burning Ship
  Dive at the starting center also goes black after a few seconds; Julia's Dive has black stretches (e.g. Dive 0.7
  around 5 s); Julia "C Real" 0.8 fills the screen black. Options: (a) zoom always dives toward a detailed edge point --
  the starting picture stays identical, but Center X/Y becomes a sideways nudge at depth instead of the dive target;
  (b) move the starting center (and Julia's starting shape) onto the edge -- the starting picture shifts a little,
  every knob keeps its meaning; (c) keep -- pick a Location before zooming. Default (c). Recommended (b).
- B3 Cross Section on Sierpinski Tetrahedron shows nothing (unless Zoom >= 0.7 and Iterations at max); on
  Kaleidoscopic IFS it shows faint specks with black gaps. Options: (a) draw the specks bigger inside the slice only
  (the normal picture stays identical; a small shader job); (b) remove the three slice knobs from these two; (c) keep.
  Default (c). Recommended (a).
- B4 Spirograph "Inner Radius" blinks to black at 8 of 25 positions (e.g. 1/4, 1/2, 3/4); Spirograph and Lissajous draw
  dotted curves at every thickness. Options: draw connected lines (the starting look changes: dots become a solid
  curve); keep. Default keep. Recommended: draw lines.
Removed knobs:
- R1 Kaleidoscopic IFS "Fold Type": its middle third was black, its top third looked like the bottom. Old shows: a clip
  set in the middle third now shows the fractal. Option: build real fold types later.
- R2 Band Tower "Reflection": never drew anything (it drew below the screen).
- R3 Spectrum Landscape "Smoothing": never did anything (a placeholder).
Fixed (only when you move the knob; starting pictures unchanged): Cross Section always cuts through the object on
Mandelbulb, Apollonian 3D, Julia Set 3D, Menger Sponge, Burning Ship 3D; Tetrahedron / IFS "Iterations" bottom now shows
the fractal; Mandelbrot "Power" works above 1/4; Newton "Power" top stops at 6 roots (7-8 were black); Astral Grid
"Warp" now bends the grid sideways, with the bass (it did nothing).
Knobs that act only in another mode (tests open that mode): Wireframe Wolf / Icosahedron "Density" (grid shapes only),
Moire "Offset Y" (not in the Lines pattern), Shape Generator "Rotation" (not on the circle).

## 8. Found, not fixed
- mandelbrot Power 0-0.24 is one picture: the z^2 branch runs up to power 2.5 (ES:2963) -- a dead quarter of the knob.
- `test_no_discontinuities` (test_sources.py:145-191) only warns; its black count never fails.
- BLACK_AT_EXTREME admits black at ANY candidate of a listed key (test_sources.py:107); harmless for today's 6 entries
  (each passes at the next candidate); a value-keyed table would be stricter.
- kifs Slice Count / Slice Distance / Trail Fade gates pass at p99.5 ~20 (slice 0.55 at x4 lands between 21 and 19):
  4 over the floor, pre-existing.
- 3 tracked `tests/visual/__pycache__/*.pyc` files get rewritten by pytest: untrack + .gitignore (repo hygiene).

RISKS
- Strongest counterargument: "AWAITING_RULING turns a red gate green while 6 known defects remain." It loses: the
  standing default asks for exception entries; strictness makes an entry fail the moment its defect is fixed; the Boris
  list and HANDOFF carry them; and a permanently red id cannot surface a NEW regression among 30 known lines.
- Second: "DIAG's option (A) fixes the zoom lines byte-identically; the standing default says fix." It loses: (A)
  changes what Center X/Y and Dive mean at depth (Pitfalls 9/10) -- a control contract, Boris's call.
- Byte-identity of the in-place edits is by reading + arithmetic; the GPU compiler could still reschedule the
  unchanged path. M2 / M3 decide; STOP on any mismatch.
- Cross Section factors come from a t = 1.13 ladder; at other times the auto-rotation can show a slab edge-on and dim
  ANY slice position (pre-existing, not introduced).
- test_fractals' xfail set is predicted from 512-px runs; S6a confirms it on the lane before any mark.
- newton_3d Trail Fade gate margin (PSNR 50.9 at 0.6); S6a picks 0.6 or 1.0 by the lower PSNR.
- julia's AWAITING probe depends on FEATURES_ACTIVE's rms 0.6; a fixture change can make it stale (it fails loudly).
- Removed params and recorded takes / routines: source params are not addressed by index there (RoutineSlice.cpp:155,
  :181 handle `param` only for `fx >= 0`; Take.cpp:384 leaves v1 source-param events unresolved on purpose) --
  INFERRED index-safe; reconcile keeps values and connections by uniform name (CompositionLoad.h:100-140).
- Concurrent lanes (tempo, render leftovers, restore): pitfall numbers / CLAUDE.md index / APP-INVENTORY conflicts;
  rebase rule in S6. No other lane is expected to touch ES or SR.

HANDOFF-NEEDS (Harmony: adopt or veto; defaults in force): (1) AWAITING_RULING strict tables + the docstring change;
(2) SPARSE_AT_EXTREME; (3) kifs Cross Section deferred, not factored (DIAG deviation 1); (4) Iterations floor form
(DIAG deviation 2); (5) B1 default (c) keep.

STATUS: DONE

----------------------------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (s-rta-0928, 10:08) — OVERRIDES THE BODY WHERE THEY DIFFER
Authorship: OPUS architect standing in for Fable (Fable quota exhausted; recorded tier deviation).
Blind council: attack-tier1-loosening.md (SOUND_WITH_FIXES), attack-tier1-vj.md (SOUND_WITH_FIXES, 1 MUST). ADOPTED as
written (10 app fixes, 3 removals, 7 gate entries, SPARSE_AT_EXTREME, strict AWAITING_RULING ledger, test_fractals
retire-2-port-6), with these rulings:
- E1 (vj MUST: Zoom goes black over 60-75% of travel on mandelbrot / burning_ship / julia_set at default centre). RULING:
  this changes a documented control contract (docs/claude/fractals.md design rules 1-2; Pitfalls 9/10: what Zoom /
  Center / Dive mean) — a product/feel call, so it is NOT redesigned in this lane. The strict AWAITING_RULING entries stay
  (they fail the moment a fix lands, so the gate cannot hide it) and it goes to the TOP of the Boris list in plain words
  with the architect's options and Harmony's recommendation: "Zoom dives toward edge detail instead of the black
  inside; the picture at Zoom 0 does not change" (option A), because a Zoom knob that is black for most of its travel is
  the worse stage failure and option A keeps every stored show identical at Zoom 0.
- E2 (vj SHOULD) kifs Fold Type: before removing it, TRY the one-line fix the seat found (branch 2, 0.33-0.67, lacks the
  y/z abs-fold its sibling branches do first: EmbeddedShaders.h ~7025-7040). Render it at 0 / 0.5 / 1 and at the stored
  default: if the 0.5 extreme is no longer black AND the stored default renders byte-identical, IMPLEMENT (and list it
  for Boris as "Fold Type 0.5 now draws"); otherwise REMOVE as planned, with the evidence.
- E3 (vj SHOULD) spirograph / lissajous Thickness 0: TRY a floor on today's mapping (identical above the floor, the stored
  default byte-identical — same shape as the Iterations floors) that makes Thickness 0 visibly drawn at 1080p (passes the
  normal p99.5 metric, not only SPARSE). If it works, it is an app fix; if not (the sampling alias B4 dominates), keep
  SPARSE_AT_EXTREME and list it for Boris at the same severity as B1 ("reads as black on a projector").
- E4 (loosening SHOULD) BLACK_AT_EXTREME entries are keyed by (source, uniform, VALUE): the value the evidence covers,
  not any black candidate. Convert the 4 existing entries too (each already names its value); keep the admission rule
  text.
- E5 (loosening SHOULD) sierpinski_tetra: take it OUT of the shared 7-fractal gate loop's source list and give it its own
  entries (no shadowing by dict re-assignment order); add a tiny harness self-test (a pytest that imports
  tier1_exceptions and asserts tetra's gate values) if it costs < 15 lines.
- E6 NITs: the mandelbrot z = 0 guard also covers the Julia-mode ray at the centre — the byte-identity check must include
  julia_set's and mandelbrot's Julia-mode stored defaults; the DEBT_FILED comment in test_shader_param_lint.cpp gets a
  live example or neutral wording.
- E7 Lock discipline: full Tier-1 runs are ~18 min — hold the lock per run, release between runs (three other lanes and
  Harmony share it). test_fractals chunks <= 20 min per hold.
- E8 FENCE: this lane edits src/render/EmbeddedShaders.h (shader lines) and src/sources/SourceRegistry.cpp. The renderleft
  lane edits other src/render files (not EmbeddedShaders.h); do not touch Renderer.* / CompositorEngine.* / ImageDecode.
