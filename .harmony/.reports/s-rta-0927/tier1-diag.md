STATUS: DONE
RESULT: Tier-1 is red because of bugs in the test harness and outdated test expectations. No recent lane caused it. The five test files have not changed since 2026-03-22. The same 8 tests already failed before the canvas lane, and the item-level output is identical on the pre-change app, the C1 lane and a third run at 16:23 (516/516 source-param lines match). Six clusters explain every failing test. Four are harness bugs: (H1) sources keep their parameter values between loads, (H2) each 256x256 capture resizes the canvas, which resets stateful sources, (H3) a mean-brightness threshold that calls sparse line art "black", (H4) test values that land on no-op points (periodic params at 1.0, the integer time t=1.0). One is outdated expectations (E): the audio features and effects chosen for the reactivity tests, plus sources that are black in silence. One is real app bugs (A): 73 source params plus Dot Field "depth" that no shader reads, julia_set / burning_ship / newton_3d black at their defaults, and Dot Field near-black at its default. Harness fixes alone will not make Tier-1 green. The A items and some policy calls (exceptions list, zero-width params) remain.
FACTS: (1) Stale-parameter chain: in the params test, the "default" render of param k is byte-identical to the "test" render of param k-1 in 651 of 651 pairs. The time-sweep t=1 frame is byte-identical to the LAST test_no_discontinuities frame (every param left at 1.0) for 105 of 105 sources. Live: torus_hole fresh mean 88.19; after all params = 1.0 and a reload without params, mean 0.01 (PSNR vs all-1.0 = inf); after /api/reset it is still 0.01; a reload with explicit defaults gives PSNR inf vs fresh. Source: `src/render/Renderer.cpp` line 616 (no params -> nullptr -> the cached instance keeps its values) and `src/test/TestServer.cpp` line 763. Evidence: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tier1/B1.log`. (2) Resize reset: with the composition at 1920x1080 every 256x256 capture is frame 1 of a freshly reset simulation. Evidence: reaction_diffusion feed default->1.0 PSNR inf. With the composition set to 256x256 it is PSNR 46.7. projectM goes from black to mean 19.9, and fluid_dynamics plus injected features goes from 0 to 13.2 (`src/sources/ProceduralSource.cpp` lines 64-71; B1.log E2, `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tier1/B2.log` (c)). (3) Metric: 15 of the 24 "all black" sources have p99.5 brightness of 98-255 and are visibly drawn. The 1920x1080 capture is dimmer, not brighter (E4). (4) Audio: none of the 6 listed sources reads u_beatPhase (live PSNR inf). The test injects bandEnergies[0], but u_bass = bandEnergies[1] (`src/sources/ProceduralSource.cpp` lines 160-161). Ripple, Hue Shift and Chromatic Aberration do not read u_rms (PSNR inf); Density Wave does (PSNR 9.9).
METHOD: I started from the logs. BASE, LANE and the surviving pytest-45 PNG artifacts of a later identical run gave per-item messages. I classified all 516 source-param lines, the 41 effect-param lines, 24 black sources and 81 time-sweep entries by message and by static shader-uniform scans of `src/render/EmbeddedShaders.h`. I then decoded the PNGs themselves: PSNR chains, brightness percentiles and a contact sheet. Finally I ran two short live batches on the main app (test mode, `open -g`, live lock held, 967 renders): E1-E7, then (a)-(e). They confirmed each cluster's mechanism and measured what is left after the test-side fixes.
CONFIDENCE+VERIFY: High on H1, H2, H3, H4 and E, and on the A items marked VERIFIED below: each was reproduced live, from a different starting point than the logs. Medium on the per-item split of the 56 "goes black" lines and the 23 "other no-effect" lines, because the H1 chain compounds them; they need one re-run after H1 is fixed. To re-check: `bash .../scratchpad/tier1/run_batch.sh <label> .../scratchpad/tier1/exp1.py` (exp2.py, exp3.py).
UNKNOWNS/NOT-DONE: No source, test or build edits (diagnosis only). I did not run the fixed tests; section 4 is predicted from the live residual counts. Not individually verified: the 56 goes-black items, the 23 "other" no-effect items, the gravity_well Scatter/Trail Length items, why crystal_cavern goes black at t=5/10, and whether burning_ship's registry defaults (0.55/0.6) drifted from the shader's designed default (0.5/0.5).
NUANCE: The runs are deterministic: three separate runs gave byte-identical failure text. The stale-parameter chain ALSO decides which items appear in the other failures (for example, "burning_ship Center X no effect" is only because the Location preset set earlier in the chain overrides the manual center). That is why the H1 fix must land first, before anyone triages the remaining lines. Test order matters: the older whole-directory run has a different item list for the same reason, so item lists cannot be compared across run shapes.
HANDOFF-NEEDS: none

INBOX-RECHECK: none

# Tier-1 diagnosis (tests/visual, 5 files) -- s-rta-0927 lane tier1-diag

## 0. Verdict in one table

| Failing test | Cause cluster(s) | Kind | Smallest fix |
|---|---|---|---|
| test_sources::test_all_sources_non_black (24) | H3 x15, H2 x2, E-silence x1, E-router x1, H4-time x1, A-defaults x3, A/H-aliasing x1 | mostly test | H3 metric + H2 composition 256 + exceptions; A: 3 source defaults |
| test_sources::test_all_params_have_effect (516) | H1 chain (drives 329 default-black lines + compounding), A-phantom x73, H2 x11, H4-periodic x24, goes-black x56 (policy), other x23 | harness + app | H1 first, then H2/H4; A: 73 dead params |
| test_effects::test_all_effects_non_black (1) | Dot Field default near-black | app | EffectLibrary default size (or exception) |
| test_effects::test_all_params_have_effect (41) | H4-periodic x13, H4-time x5, E-silence x12, frame-history path x8, quantized x1, A-phantom x1, fixture x1 | mostly test | t=1.13, periodic test value, exceptions/animated input |
| test_audio_reactivity[Beat Phase] | E: listed sources never read u_beatPhase | test (stale expectation) | change the source list |
| test_audio_reactivity[Bass] | E: wrong band index + only 1/6 sources read u_bass | test | band index 1 + source list |
| test_audio_reactivity::test_rms_affects_effects | E: those 3 effects never read u_rms | test | effect list -> Density Wave |
| test_time_sweep::test_sources_change_over_time (81) | H1 (all params stuck at 1.0), then H3/H2/E/A | harness + app | H1 + H2 + H3 + exceptions; A remainder |

The 5 passing ids (test_no_discontinuities, test_no_destructive_effects, [RMS], both test_performance) are unaffected. Nothing here was introduced by the canvas lane (s-rta-0926b), outputs C1/C2 or routines: the test files are unchanged since `0eae756` (2026-03-22). The pre-canvas whole-directory base run already failed the same 8 tests (plus discontinuities, which the canvas lane FIXED) (`.harmony/.reports/s-rta-0926b/canvas-evidence/tests-visual-BASE-outcomes.txt`). I found no evidence that the suite was ever green.

## 1. Evidence base

- Logs: `.../scratchpad/outputs/tier1-BASE/pytest.txt`, `tier1-LANE/pytest.txt`. `diff` of all `^E ` lines: they differ only in the pytest tmp dir name.
- Artifacts: `/private/var/folders/xk/.../pytest-of-boriskarpman/pytest-45/` holds a later run (16:23) of the same five files with every PNG kept. Its `source_param_failures.txt` is byte-identical to BASE's 516 lines, and its `effect_param_failures.txt` has the same 41 lines. All the offline decoding below uses those PNGs.
- Live batches: `.../scratchpad/tier1/B1.log` (E1-E7, 17:21:34-17:23:48) and `.../scratchpad/tier1/B2.log` ((a)-(e), 17:33:22-17:35:35). Scripts: `exp1.py`, `exp2.py`, `exp3.py`, runner `run_batch.sh`. PNGs are in `runs/B1/png`, `runs/B2/png`. Class tables: `src_fail_classes.json`, `src_params_static.json`. Contact sheets: `dim_sources_sheet.png`, `fractal_full.png` (all under `.../scratchpad/tier1/`).

## 2. Clusters: mechanism, evidence, count, fix

### H1 -- sources keep their parameter values between loads (harness; the biggest cause)

**Mechanism (VERIFIED).** `POST /api/load_source` with no `params` calls `Renderer::setActiveSource(type, {})` (`src/test/TestServer.cpp:763`). Each frame, `Renderer.cpp:616` passes `nullptr` when the list is empty, so `renderSource` applies nothing (`Renderer.cpp:1250-1262`). Source instances are cached by id (`getOrCreateSource`, `Renderer.cpp:1054`), and `/api/reset` only clears the active-source selection. So every value ever set by `update_source_params` stays in the instance for the rest of the session. The real UI never hits this: a UI clip always carries the full default list (`src/MainComponent.cpp:1119-1125`).

**Evidence.**
- Offline, params test: default(k) == test(k-1) in 651 of 651 pairs (PSNR inf), and the first param's default equals the non_black default in 108 of 108 sources.
- Offline, time sweep: every t=1 frame equals the last `test_no_discontinuities` frame, where each param was left at 1.0, for 105 of 105 sources. The time sweep therefore renders every source with all parameters at 1.0, not at defaults.
- Live E1 (torus_hole):

  | step | mean | PSNR |
  |---|---|---|
  | fresh defaults | 88.19 | -- |
  | all params = 1.0 | 0.01 | -- |
  | reload without params | 0.01 | vs all-1.0 = inf |
  | after `/api/reset` | 0.01 | vs fresh = 6.2 |
  | reload with explicit defaults | 88.19 | vs fresh = inf |

**What it explains.**
- All 170 "default is black" lines on the 42 sources that are NOT black at fresh defaults. Every one of them comes after a "goes black" line for the same source.
- The time-sweep list (81 entries). For example, torus_hole renders black in the time sweep while its default mean is 88.19.
- Fake "no effect" lines. burning_ship / mandelbrot "Center X/Y" do nothing only because Location was already set to 1.0 earlier in the chain, and the preset overrides the manual center (the shader's `else` branch; `EmbeddedShaders.h` source_burning_ship / source_mandelbrot).
- It also compounds many "goes black" lines. solid_color "Blue goes black at 0.0" happens because Red and Green were already 0.

**Fix options.**
- Harness, preferred (1 site): in `TestServer::handleLoadSource` (`src/test/TestServer.cpp:720-769`), when `params` is absent, seed the full default list from `sourceRegistry_.createSource(id)`, the same way `MainComponent.cpp:1119-1125` does. This fixes every caller at once, including test_range_quality and test_fractals.
- Test-side alternative: every `app.load_source(id)` in the Tier-1 files passes `params={p["uniform"]: p["default"] for p in src["params"]}`. Live E1 proved this restores the fresh image exactly (PSNR inf). It touches about 8 call sites in 4 files.

### H2 -- each 256x256 capture resizes the canvas, which resets stateful sources (harness)

**Mechanism (VERIFIED).** `render_frame` sets the TEST canvas lock to 256x256, then clears it again (`TestServer.cpp:586,592`). With the composition at its default 1920x1080, every capture therefore flips the canvas to 256x256 and back. `ProceduralSource::resize` releases and re-creates the ping-pong FBOs, which wipes the simulation (`src/sources/ProceduralSource.cpp:64-71`). The capture waits for the first frame at the locked size (`Renderer.cpp:2085`), so it always sees frame 1 of a fresh simulation. projectM behaves the same way.

**Evidence.**
- E2: at composition 1920x1080, capture 1 and a capture 1 s later are identical. At composition 256x256, strange_attractor goes from 0.86 to 1.65, reaction_diffusion from 16.4 to 28.0, and cellular_automata from 100 to 198.
- reaction_diffusion "feed" default->1.0 goes from PSNR inf to 46.7.
- (c): projectm_visualizer goes from mean 0 to 19.9 in silence (48.3 with features). fluid_dynamics goes from 0 to 13.2 with features after 2 s. Pitfall 22 already says it needs audio.

**What it explains.** projectm_visualizer and fluid_dynamics in non_black. The 11 "no visible effect" lines on reaction_diffusion x4, cellular_automata x5 and gravity_well x2 (reaction_diffusion verified; gravity_well did not change under composition 256, so it is UNVERIFIED). Their time-sweep entries.

**Pre-existing:** the canvas lane report already noted "Stateful procedural sources reset on any canvas-size change, including each TestServer width/height capture" (`.harmony/.reports/s-rta-0926b/canvas.md:174`).

**Fix (test-side, 1 site).** In the `app` fixture in `tests/visual/conftest.py`, call `POST /api/set_composition_params {"outputWidth":256,"outputHeight":256}` once and restore 1920x1080 at teardown. With the composition and the lock the same size, the canvas never resizes (`resolveCanvas` gives 256 either way). App-side alternative: keep simulation state across a resize (rescale-blit, like `CompositorEngine::rescaleHistory`). That is a larger change.

### H3 -- mean-brightness threshold is wrong for sparse sources (test)

**Mechanism (VERIFIED).** `brightness() = mean(pixels) < 5.0` (`tests/visual/test_sources.py:19-21,48`). The time sweep uses `< 3.0`. Line art, point clouds and small objects cover 1-7% of the frame, so their mean is between 0.86 and 4.8 even though they are clearly drawn (`dim_sources_sheet.png`). The canvas size is NOT the cause: E4 shows the 1920x1080 capture is dimmer (for example, mandelbulb 4.80 at 256x256 vs 2.70 at 1920x1080).

**Affected 15 of the 24:**
strange_attractor, band_tower, sierpinski_tetra, spirograph, timbral_nebula, lissajous, laser_scan, laser_scanner, polygon_lines, sine_oscillator, electric_arc, julia_set_3d, wire_wolf, burning_ship_3d, mandelbulb.

**Fix (test-side).** Call a frame black when `np.percentile(img.max(axis=2), 99.5) < 16`. On the 108 default frames this flags exactly the 9 truly black ones; the dimmest non-black frame scores 34, the others 98-255. With H1 and H2 applied, (d) shows 22 flagged by the old metric vs 7 by this one. Apply it to `test_sources.py` and `test_time_sweep.py`.

### H4 -- test values that do nothing (test)

**Periodic parameters.** The test value is 1.0 whenever the default is <= 0.3, and 1.0 equals 0.0 for anything scaled by 2π or wrapped with `fract`.
- VERIFIED live (E7): PSNR(0,1) vs PSNR(0,0.5) was Chromatic Aberration 95.0 vs 11.3, RGB Split 98.1 vs 11.0, Emboss inf vs 12.2, Prism inf vs 13.7, Palette Remap inf vs 7.8, Radial Cloner 80.7 vs 14.7, Selective Color hue inf vs 44.4.
- Motion Blur angle repeats every π: 0.5 gives 78.9 too, so it needs 0.25.
- The other 5 effect items (Parallax, Voxel, Bendoscope, Density Wave direction, Brush Strokes) are the same by static read (`* 6.28318`).
- The 24 source "Color Shift / Hue / Color 1 / Rotation 0->1" lines (PSNR 74-inf) are the same class, INFERRED from the shader idiom plus the high PSNR values.
- Fix: use 0.5 (0.25 for angles) when default <= 0.3, or pass if either 0.5 or 1.0 changes the output.

**The time t=1.0 lands on a no-op.** All captures use `time_val=1.0`.
- Pulse: sin(3·2π·1) = sin(0.5·2π·1) = 0.
- Color Flash: fract(6) = fract(1) = 0.
- Strobe: phase 0.5 vs 0 with a duty cycle >= 0.5.
- Tunnel: fract(t·speed) with speed 1 vs 0.
- VERIFIED E7: at t=1.0 PSNR is inf, at t=1.13 it is 11.4 / 13.8 / 8.8. Tunnel was checked statically only.
- strobe_light (source) is the same class: black at t=0.5/1/3/5, 255 at t=0/2/10 (E3).
- Fix: `time_val=1.13` for the param tests; exclude strobe_light from the single-time non-black check, or sample 2 times.

**Quantized parameter.** Oil Paint radius uses `2+int(r*2)`, so 0.3 and 0.0 are both radius 2 (E7 PSNR inf; 0.3->1.0 gives 17.5). This is a test-value issue. Minor app note: the default sits inside the dead zone.

### E -- outdated expectations about which audio feature drives what (test)

**Beat Phase 0/6 (VERIFIED).**
- Static scan (declarations excluded): mandelbrot, julia_set, burning_ship, audio_waveform, mandelbulb and kifs use u_beatPhase 0 times.
- Live E6: all 6 give PSNR inf.
- Sources that do use it respond: twisted_torus PSNR 2.3, fermat_spiral 19.9, text_animator 36.7. sacred_geometry (92.6) and astral_grid (inf) do not respond at their defaults.
- Fix: `AUDIO_RESPONSIVE_SOURCES` for this feature = twisted_torus, fermat_spiral, text_animator.

**Bass 0/6 (VERIFIED).**
- `FEATURE_PAIRS` "Bass" sets bandEnergies[0] (sub-bass), but `u_bass = bandEnergies[1]` (`ProceduralSource.cpp:160-161`, `docs/claude/rendering.md` P18 table). Of the 6 listed sources, only audio_waveform reads u_bass.
- Live: band[0] gives inf everywhere. band[1] gives audio_waveform 15.5, perlin_noise 20.9, torus_hole 6.7, geometric_tunnel 21.4, crystal_cavern 29.6.
- Fix: `[0,1,0,0,0,0,0]`, plus a bass-reading source list.

**RMS effects 0/3 (VERIFIED).**
- Of all 153 non-source shaders, only density_wave and structural_morph read u_rms.
- Live: Ripple, Hue Shift and Chromatic Aberration give inf; Density Wave gives 9.9. Structural Morph gives inf because its RMS term is gated by structuralState.
- Fix: `effects_to_test = ["Density Wave", ...]`. Alternatively, test RMS->effect through `add_mapping`, which is how effects actually follow audio.

**Audio-driven sources and effects in silence.** No features are injected in the non-black and params tests.
- chromatic_ring renders 0 in silence and 114.5 with features (VERIFIED E5); band_tower 1.3 -> 74.4.
- 12 effect items only act on audio. Static reads: Key Palette returns early when `u_detectedKey < 0.0`, and inject_features defaults it to -1. Chroma Dissolve and Harmonic Displacement read u_chromagram. Transient Flash reads u_onsetStrength. Timbral Mosaic reads u_mfccs. Structural Morph's drop style reads u_structuralState. These are INFERRED per item, not run live.
- spectrum_landscape x3 (bandEnergies) is INFERRED.
- Fix: inject a non-zero feature set for the audio categories, or add them to the spec's "known exceptions".

**Layer Router** is black by design with no deck layer (Pitfall 21; E3: 0 at every t). Fix: exclude it.

### Frame-history effects through `/api/set_effect` (harness path)

**Screen Split and Frame Stutter (VERIFIED).** These only work inside the deck compositor (`applyClipEffects`, Pitfall 19). `/api/set_effect` drives the legacy `effectChain_`, which runs them as ordinary shaders. (a) shows PSNR inf even on an animated input at composition 256 after 1 s. Same result for Directional Feedback speed and Point Zoom saturation; that their mechanism is the deck-only feedback buffer is INFERRED.

**Channel Delay red and Point Zoom hue shift DO respond on an animated input** (24.2 and 36.0) but not on the static test card (71.9 and 61.4). Their failure is caused by the test fixture (VERIFIED).

**Fix.** Use an animated source (for example `load_source("plasma")`) as the input for "time"-category effects, and add Screen Split / Frame Stutter / Directional Feedback / Point Zoom saturation to the known exceptions for the legacy path, or test them via a deck clip.

### A -- real app defects (app-side)

**A1. Black at defaults (VERIFIED at every t and at both sizes).**
- julia_set, newton_3d: 0 at t = 0, 0.5, 1, 1.5, 2, 3, 5, 10 (E3) and at 1920x1080 (E4).
- burning_ship: 0 at 256x256 and 0.40 mean / 0.37% lit at 1920x1080. Only a corner of the ship is on screen (`fractal_full.png`).
- A user adding these as clips gets the same defaults (`MainComponent.cpp:1119-1125`), so this is product-visible (INFERRED; the deck path was not rendered).
- Sites: `src/sources/SourceRegistry.cpp:415-425` (julia_set: c = (-0.475,-0.24) at zoom e^1.5 frames only interior), `:429-439` (burning_ship: Center 0.55/0.6, while the shader comment says the designed default is 0.5/0.5 -- UNVERIFIED drift), `:583-597` (newton_3d; Angle X from 0.55 to 0.0 made it visible in the chain).
- Fix: pick defaults that frame the structure (Pitfall 27).

**A2. Registered parameters that no shader reads, 74 items (VERIFIED).**
- `addTorusControls` (`SourceRegistry.cpp:644-662`) adds 12 controls to 8 sources (`:668-715`). Only torus_hole's shader implements them. striped_torus, spiral_vortex, checker_torus, ribbed_vortex, wormhole_tunnel, twisted_torus and wormhole do not use u_src_orbit / tilt / zoom / lens_shape / lens_rotate / depth_fade / pinch / heart / shading, and some also ignore tube_radius / warp.
- Live E1: twisted_torus orbit 0->1 and 0->0.5 give PSNR inf; torus_hole orbit 0->0.5 gives 3.0.
- Plus mandelbulb "Iterations", structural_landscape "History Length"/"Camera Height", and effect Dot Field "depth" (`u_dotfield_depth` is declared at `EmbeddedShaders.h:3643` and never used).
- Total 70 + 1 + 2 + 1 = 74. These are dead UI controls.
- Fix: implement them in the 7 shaders, or register the torus controls only for torus_hole.

**A3. Dot Field (the only failing effect in non_black), VERIFIED E7.** Mean 1.80 and 1.2% lit at default size 0.3; size=1.0 gives mean 24.0. Dots are about 1 px in radius (`luma*0.3*0.03*0.6` UV). Fix: raise the default in `src/effects/EffectLibrary.cpp:514-518`, or list it as an exception.

**A4. sierpinski disappears at small sizes (VERIFIED (b)).** Mean 0 at 256x256, 7.76 at 512 and 1024. Its lines are about 1 px at 1920x1080 and vanish at 256. Fix: capture sources at >= 512 (test-side), or antialias/widen the lines (app-side).

**A5. crystal_cavern** is black at t=5 and 10 at defaults ((e)); the mechanism is UNVERIFIED.

### Policy item -- "goes black at the extreme" (56 lines)

Parameters like Thickness, Count, Size, Width, Rays, Branches, Glow or Height at 0.0 legitimately draw nothing. The spec requires "not black at every position" but allows "known exceptions" (`tests/visual/SHADER_VERIFICATION.md:28,167`). The Tier-1 source tests implement no exceptions list. Many of the 56 are also H1-compounded, so they are not individually verified. Needs a ruling: add an exceptions list (test) or clamp minimums (app).

## 3. Accounting per failing test (every item placed)

**test_all_sources_non_black, 24 items:**

| Items | Cluster |
|---|---|
| 15 | H3 |
| projectm_visualizer, fluid_dynamics | H2 (+ fluid needs audio) |
| layer_router | E-router |
| chromatic_ring | E-silence |
| strobe_light | H4-time |
| julia_set, burning_ship, newton_3d | A1 |
| sierpinski | A4 |

**test_all_params_have_effect (sources), 516 items:**

| Lines | Kind | Placement |
|---|---|---|
| 329 | default-black | 170 = H1 on 42 fine sources; 159 on the 24 fresh-black sources above |
| 73 | no-effect | A2 |
| 11 | no-effect | H2 |
| 24 | no-effect | H4 periodic |
| 23 | no-effect | "other": 4 H1 center/location (VERIFIED statically), 3 spectrum_landscape E-silence (INFERRED), 16 UNEXPLAINED until re-run after H1 |
| 56 | goes-black | policy + H1 compounding |

The 16 unexplained "other" lines: astral_grid Warp, spectral_ring Glow, fire_wall Wind, strobe_light Fade, ribbed_vortex Speed, scroll_plane Speed Y, torus_hole Phi/Theta Offset/Pinch/Heart, mandelbrot Max Iterations, cymatics Damping, water_caustics Distortion/Scale, structural_landscape Drama, kaleido_fractal Iterations.

**test_all_effects_non_black, 1 item:** A3.

**test_all_params_have_effect (effects), 41 items:**

| Items | Cluster |
|---|---|
| 13 | H4 periodic |
| 5 | H4 t=1.0 |
| 12 | E-silence |
| 8 | frame-history / static-card |
| 1 | Oil Paint quantized |
| 1 | Dot Field depth (A2) |
| 1 | Selective Color range (the test card has little content near hue 0, INFERRED) |

**Audio [Beat Phase], [Bass], rms-effects:** E (VERIFIED).

**test_sources_change_over_time, 81 items:** H1 (VERIFIED 105/105). What remains after the fixes is in section 4.

## 4. What stays red after the test-side fixes (measured live, B2 (d)/(e))

Settings: explicit defaults (H1), composition 256x256 (H2), p99.5 metric (H3), silence.

**non_black** leaves 7 sources: chromatic_ring, strobe_light, newton_3d, layer_router, burning_ship, julia_set, sierpinski. With features injected, 6 remain; chromatic_ring recovers. After the exceptions (layer_router, strobe_light) and capturing sierpinski at 512, **julia_set, burning_ship and newton_3d remain. These need the A1 app fix.**

**time sweep** leaves 28 entries: chromatic_ring x4, layer_router x4, strobe x2, crystal_cavern x2, newton_3d x4, burning_ship x4, julia_set x4, sierpinski x4. After the exceptions, features and sierpinski at 512, 14 entries remain, which is still >= 5. **Green needs A1 (and crystal_cavern A5).**

**audio reactivity:** all three tests pass with the E list changes (live PSNRs above clear the thresholds: >= 2 sources below 50 for beat and bass; >= 1 effect for RMS).

**param tests:** the true residual is unknown until H1 is fixed; A2's 74 items fail regardless.

## 5. Recommended order (smallest first)

1. H1 in `TestServer::handleLoadSource`: seed the defaults. One harness site; it changes the meaning of most other lines, so do it first.
2. H2 in `conftest.py`: composition 256x256 for the session.
3. H3/H4 in the test files: the p99.5 metric, t=1.13, the periodic test value, the audio lists and band index 1, and an exceptions set (layer_router, strobe_light, audio-native sources get injected features, the legacy-path frame-history effects).
4. Re-run Tier-1 once, then triage the real residual.
5. App (Boris's call on each): A1 source defaults (3 sources), A2 dead controls (74), A3 Dot Field default, A4 sierpinski line width, A5 crystal_cavern.

## ISSUES

- **Launch form.** Test mode has no environment switch; it is only `--test-mode` on the command line (`src/Main.cpp:17-22`). I launched with `open -g --stdout <f> --stderr <f> <App> --args --test-mode`, the same form the earlier lanes used. It is `open -g`, in the background, never opened the Output window, sent no synthetic input, and did not use pytest on the directory.
- **Lock.** Held twice: 17:21:34-17:23:48 and 17:33:22-17:35:35, with more than 45 s between. My apps (pids 65459 and 72762) quit through osascript. After release the lock was taken by renderperf (owner timestamp 17:35:39). The Audio-DNA running now (pid 73457, started 17:35:40) is theirs.
- **Note for the notebook (Harmony to decide; I did not write it):** "Eyes load_source without params keeps the cached source instance's param values across load_source and /api/reset; Tier-1 source tests therefore never render true defaults after the first param."

## SKILL_PROPOSALS
none

## RISKS
- The per-item split of the 56 goes-black lines and the 16 unexplained lines is only settled after H1 is fixed.
- Setting the composition to 256 in the session fixture changes the canvas for any other test in the same pytest session that assumes 1920x1080. Scope it to the Tier-1 modules if other files share the session.
- A1 default changes are visible to users (Boris check).

## METRICS
2 live batches, 967 renders (B1 345+48, B2 574), about 4.5 min of app time. Offline: BASE and LANE logs plus the pytest-45 PNG set decoded.

## PACKET QUALITY
- Clarity: CLEAR.
- Missing context: the allowed launch form (`open -g [--env]`) cannot enable test mode; `--args --test-mode` was needed (see ISSUES).
- Unused context: the routines-display change (irrelevant; nothing routes through it).
- Self-brief files: CLAUDE.md (pitfalls 9/19/21/22/27 useful), `.harmony/gotchas.md` (conftest autouse: noted, not triggered), `docs/claude/testing-eyes.md` and `rendering.md` P18 (useful), `tests/visual/SHADER_VERIFICATION.md` (useful: exceptions clause), earlier lane reports `canvas.md` / `outputs-c1.md` (useful for history).

### STATUS
DONE

### NEXT ACTION
Harmony: dispatch the H1 (TestServer) and H2/H3/H4/E (tests) fixes as one test-harness lane, re-run Tier-1 once, and bring A1-A5 to Boris as app decisions.
