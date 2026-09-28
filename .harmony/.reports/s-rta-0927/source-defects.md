STATUS: DONE_WITH_CONCERNS
RESULT: all six fix commits of plan-source-defects.md landed on `lane/source-defects-0927` (base 2ee1013), each independently green, each RED first. julia_set, burning_ship, newton_3d (also at t = 30 / 45), sierpinski (256x256 and 1024 @ Iterations 0.75) and crystal_cavern (t >= 2.5 s) now draw a picture at their registered defaults. So does Dot Field on first add. They were black or near-black before. 78 source controls that no shader read are gone (759 -> 681 source params). mandelbulb Iterations, julia_set_3d Iterations and Dot Field depth now work and are identical at their defaults. A new static lint (`test_shader_param_lint`) and an offscreen GL gate (`test_source_defaults_gl`) guard both defect classes. Old compositions still load; their clips are reconciled to the current param list on load. The must-not-change sweep (T4) shows only the intended set changed, both live and offline. Concerns: two plan deviations the evidence forced (crystal_cavern near-clip, sierpinski +0.001), two harness findings, and seven Boris calls (below).
FACTS:
- Commits: 47da170 (C1 julia/burning), 7a99935 (C2 newton), 3809415 (C3 sierpinski), a68b1fd (C4 crystal_cavern), 9482d3e (C5 Dot Field), 0715cb7 (C6 A2 removals + implements + lint + reconcile + docs); C7 = this report + evidence. Source: `git log 2ee1013..HEAD`.
- ctest serial: C1 744/744, C2 745/745, C3 746/746, C4 747/747, C5 748/748, C6 751/751, final rebuild of HEAD 751/751. Source: `source-defects-evidence/ctest-c*.summary`, scratch `green/ctest-final.txt`.
- T2 `test_source_defaults_gl` on the pre-change tree: "test cases: 7 | 0 passed | 7 failed / assertions: 1530 | 1461 passed | 69 failed". On HEAD: "All tests passed (1528 assertions in 7 test cases)". T1 `test_shader_param_lint` pre-change: "test cases: 1 | 0 passed | 1 failed / assertions: 1401 | 1399 passed | 2 failed". HEAD: "All tests passed (1389 assertions in 1 test case)". Source: `source-defects-evidence/T2-base.txt`, `T1-base.txt`, `green-T2-c6.txt`, `green-T1-c6.txt`.
- Live T3 (8080, explicit default params, decoded PNGs), main app -> lane app, % lit: julia_set 0 / 0 -> 43.8 / 50.1 (256 / 1080); burning_ship 0 / 0.37 -> 60.5 / 73.9; newton_3d t=0 0 / 0 -> 65.5 / 65.8; sierpinski 256 0 -> 10.0; sierpinski 1024 @ 0.75 0 -> 5.6; crystal_cavern t=5 0 -> 87.1; crystal_cavern t=10 0 -> 83.4; Dot Field mean 1.80 -> 11.44 (256). Source: `source-defects-evidence/live-before.log`, `live-after.log`.
- T4 live: 461 of 486 captures are byte-identical across two runs of the main app. Of those 461, 14 changed on the lane build: 12 are the intended set, julia_set_3d 256 changed at LSB level (PSNR 83.5), and 2 (line_pattern / waveform_lines 1080p) are time-phase shifts of an unchanged image (byte-identical offline). T4 offline, deterministic GPU: 968 renders; changed = only the intended set, julia_set_3d (PSNR 83-94), and fluid_dynamics / line_generator, which are nondeterministic base-vs-base too. Source: `source-defects-evidence/t4-live-compare.txt`, `t4-offline-full-compare.txt`.
- Existing probes on the lane app: effects-parity "PY 46 PASS / 0 FAIL", crossfade "PY 35 PASS / 0 FAIL", fitmode "PY 10 PASS / 0 FAIL", deck-tabs "6 PASS / 0 FAIL". Source: scratch `battery/*.log`, `source-defects-evidence/probes.log`.
METHOD: I built T1, T2 and the reconcile test first and ran them on the unmodified tree to get RED. I then applied each plan item in order, rebuilt, and ran T2, full serial ctest and commit per item. Live 8080 batches (T3 rows, the A5 diagnosis, and T4 x2 on the main app; T3 + T4 on a frozen copy of the lane app) each ran under /tmp/audiodna-live.lock as `source-defects`, one app at a time, launched with `open -g ... --args --test-mode` and quit via osascript. A deterministic offline T4 (every shader block + registered defaults, pre-change vs lane, rendered in a private CGL context) backs up the live one, whose 1080p captures are noisy.
CONFIDENCE+VERIFY: high. Every changed picture is proven RED -> GREEN in both the offscreen gate and the live app, and live numbers equal the offscreen numbers to the digit (e.g. julia 43.81 / 50.14% lit in both). VERIFY: `ctest --test-dir build-lane -j1`; `build-lane/tests/test_source_defaults_gl -s`; `build-lane/tests/test_shader_param_lint`; live `bash <scratch>/srcdef/live.sh <label> <app> <scratch>/srcdef/t3.py` under the lock.
UNKNOWNS/NOT-DONE: Tier-1 pytest was not run; the harness lane owns it and it is known red for harness reasons (tier1-diag.md). I did not build a Syphon / REST-off variant (not touched). The POLICY item (56 "goes black at 0" lines) ships as-is by design; the exceptions list is the harness lane's job after H1.
NUANCE: crystal_cavern changes at EVERY time, not only t >= 2.5 as the plan expected: rays that used to leave the single cave now meet the next cell (t = 0 PSNR 37-40, lit 80.8% vs 79.7%). The near-clip I added only acts while the camera is within 0.08 of a crystal; in those frames the grazed crystal shows as a flat dark cut disc near the frame edge (visible at t = 10 in `pair_crystal_cavern_1920x1080_t10.png`), instead of the whole frame flashing one colour.
HANDOFF-NEEDS: Harmony: behavioral gate + reviewer; Boris: the 7 calls under BORIS CHECKS; harness lane: the two render_frame findings under ISSUES and the plan-named `tests/visual` edits (merge note).

INBOX-RECHECK: none

# Lane source-defects (s-rta-0927) -- black-at-default sources, dead controls, Dot Field

Worktree `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w10`, branch `lane/source-defects-0927`, base `2ee1013` (main has since moved to 56d7e6f, docs only; not merged, not pushed).
Plan: `.harmony/.reports/s-rta-0927/plan-source-defects.md` (Harmony-adopted), diagnosis `tier1-diag.md`.

## SUMMARY (one line per plan item)

| Item | Verdict | Commit | RED (pre-change) | GREEN (lane) |
|---|---|---|---|---|
| A1.1 julia_set black at defaults | FIXED: C Real 0.2 / C Imag 0.635 / Zoom 0.1 (classic spiral, 1.8x) | 47da170 | 0% lit at 256 and 1080, t = 0 / 1.13 / 10 (T2 + live) | 43.8% / 50.1% lit |
| A1.2 burning_ship black at defaults | FIXED: Center 0.5 / 0.5, Zoom 0.0 (documented design) | 47da170 | 0% / 0.37% lit | 60.5% / 73.9% lit |
| A1.3 newton_3d black at defaults | FIXED: Angle X 0.0 + yaw-only auto-orbit | 7a99935 | t = 0 / 1.13: 0% lit; with the default fix alone t = 30 / 45: 0% lit | 64-68% lit at t = 0 / 1.13 / 30 / 45, both sizes |
| A4 sierpinski black at 256 | FIXED: depth capped at int(log2(H * zoomPow) + 0.001) | 3809415 | 256: 0% lit; 1024 @ Iter 0.75: 0% | 10.0% / 5.6%; 512 7.51% and 1080p 7.68% unchanged |
| A5 crystal_cavern black after ~2.3 s | FIXED: cave repeats every 2.4 along z + conditional near-clip | a68b1fd | live diagnosis: 20.5% lit at t = 2, 0% from t = 2.5 (Speed 0.3); 41% at t = 4, 0% at 5 (Speed 0), prediction PASS | 0 black / 0 flat frames in a 401-frame 20 s sweep; 36-87% lit at every tested time |
| A3 Dot Field near-black default | FIXED: size 0.3 -> 0.7 | 9482d3e | mean 1.65 (T2 ramp) / 1.80 (live test card) | 11.4 / 11.44 |
| A2 dead controls (81) | FIXED: 78 removed, 3 implemented; lint + reconcile | 0715cb7 (+9482d3e for depth) | T1: 1 structural + 22 dead; depth / Iterations PSNR inf | T1 0 dead; depth PSNR 17.6 / 14.9; mandelbulb Iter PSNR 25.9 / 52.0; julia3d 43.7 |
| POLICY 56 "black at 0" lines | NOT CHANGED (plan default: keep; exceptions list = harness lane) | -- | -- | -- |
| T4 must-not-change | PROVEN (live + offline) | C7 evidence | -- | only the intended set changed (table below) |

## FILES CHANGED
- `src/sources/SourceRegistry.cpp`: julia_set / burning_ship / newton_3d defaults (C1, C2). The `addTorusControls` helper is gone; each of the 7 torus sources registers only the controls its shader reads (C6). mandelbulb Detail, newton_3d Slice Count / Slice Distance, band_tower Shape / Smoothing / 3D Rotation and structural_landscape History Length / Camera Height are removed (C6).
- `src/render/EmbeddedShaders.h`: 6 blocks only (offline check: text changed in exactly dotField, sourceSierpinski, sourceMandelbulb, sourceJuliaSet3D, sourceNewton3D, sourceCrystalCavern).
- `src/effects/EffectLibrary.cpp`: Dot Field size default 0.7.
- `src/core/CompositionLoad.h`: `compload::RegisteredSourceParam`, `SourceParamLookup`, `reconcileSourceParams(Deck&|Composition&, lookup)`.
- `src/MainComponent.cpp`: file-local `sourceParamLookup(const SourceRegistry&)`; step "3b. RECONCILE" in `loadComposition` and `appendDeckFromFile` (after RE-MINT, on the staged copy, before the fenced swap).
- Tests: new `tests/test_source_defaults_gl.cpp` (T2), new `tests/test_shader_param_lint.cpp` (T1), `tests/test_composition.cpp` `[source-params]` case, `tests/CMakeLists.txt` (2 targets appended at the end), `tests/visual/test_fractals.py` (plan-named rows: julia defaults, newton Angle X 0.0 / test 0.2).
- Docs: `docs/claude/pitfalls.md` 43 / 44 / 45 + `CLAUDE.md` index; `docs/claude/fractals.md` (mandelbulb / julia_set_3d Iterations, newton_3d row); `docs/claude/architecture.md` + `.harmony/APP-INVENTORY.md` (759 -> 681 params); `tests/visual/SHADER_VERIFICATION.md` (names the two ctest gates).

## TESTS (RED first, verbatim summary lines)

| Test | Pre-change | Lane |
|---|---|---|
| T2 test_source_defaults_gl (full file) | `test cases: 7 \| 0 passed \| 7 failed` / `assertions: 1530 \| 1461 passed \| 69 failed` | `All tests passed (1528 assertions in 7 test cases)` |
| T2 newton with the default fix ONLY (pins the orbit fix) | `test cases: 1 \| 0 passed \| 1 failed` / `assertions: 44 \| 36 passed \| 8 failed` (t = 30 / 45: 0 lit) | C2: `All tests passed (112 assertions in 3 test cases)` |
| T2 crystal_cavern on a scratch MUTANT (repetition, no near-clip) | `assertions: 1287 \| 1286 passed \| 1 failed` ("0 black, 10 flat frames; t=6.3(flat lit=1 ...") | C4: `All tests passed (1424 assertions in 5 test cases)` |
| T1 test_shader_param_lint | `test cases: 1 \| 0 passed \| 1 failed` / `assertions: 1401 \| 1399 passed \| 2 failed` | `All tests passed (1389 assertions in 1 test case)` |
| test_composition [source-params] | compile error on the pre-change header: `error: no type named 'RegisteredSourceParam' in namespace 'compload'` | `All tests passed (27 assertions in 1 test case)` |

T2 renders the shipped GLSL with the defaults PARSED from SourceRegistry.cpp / EffectLibrary.cpp at test time (deviation D1). It is therefore RED on the pre-change registry by pixels, not by a pin.

### T3 live rows (8080, explicit defaults; main app -> lane app)
Full logs: `source-defects-evidence/live-before.log`, `live-after.log`. Selection (mean / p99.5 / lit):

| id | size | t | main | lane |
|---|---|---|---|---|
| julia_set | 256 / 1080 | 0, 1.13, 10 | 0.00 / 0 / 0% | 47.34 / 217 / 43.81%; 54.19 / 217 / 50.14% |
| burning_ship | 256 / 1080 | 0, 1.13, 10 | 0 / 0 / 0%; 0.40 / 0 / 0.37% | 65.59 / 217 / 60.51%; 80.20 / 217 / 73.89% |
| newton_3d | 256 | 0 / 1 / 1.13 / 30 / 45 | 0% / 0% / 0% / 74.9% / 69.6% | 65.5 / 65.5 / 65.5 / 67.6 / 65.6% |
| newton_3d | 1080 | 0 / 30 / 45 | 0% / 73.7% / 69.2% | 65.8 / 64.2 / 66.1% |
| sierpinski | 256 / 512 / 1024 / 1080 | 1.13 | 0% / 7.51% / 7.51% / 7.68% | 10.01% / 7.51% / 7.51% / 7.68% |
| sierpinski Iter 0.75 | 1024 / 1080 | 1.13 | 0% / 3.40% | 5.63% / 5.74% |
| crystal_cavern Speed 0.3 | 256 | 0, .5, 1, 1.5, 2, 2.5, 3, 4, 5, 10, 30, 60 | 79.7, 76.8, 82.3, 58.1, 20.5, 0, 0, 0, 0, 0, 0, 0 % | 80.8, 78.7, 71.1, 67.4, 63.2, 72.7, 81.8, 84.4, 87.2, 83.4, 35.5, 83.7 % |
| crystal_cavern Speed 0 | 256 | 0, 3, 4, 5, 6, 8 | 79.7, 79.9, 41.0, 0, 0, 0 % | 80.8, 66.9, 65.1, 65.3, 77.3, 83.2 % |
| Dot Field default (test card) | 256 / 1080 | -- | mean 1.80 / 2.11 | 11.44 / 13.64 |
| Dot Field depth 0.4 -> 0 / 1 | 256 | -- | PSNR inf / inf | 18.6 / 17.4 |
| twisted_torus params | -- | -- | 14 (Orbit, Tilt, Zoom, Lens Shape, ...) | 5 (Twist, Stripe Count, Speed, Tube Radius, Color Shift) |
| total source params | -- | -- | 759 | 681 |

A5 diagnosis step: the plan's prediction PASSES live on the pre-change app (onset between 2.0 and 2.5 s at Speed 0.3; between 4 and 5 s at Speed 0). The offscreen GPU sweep agrees (first black frame t = 2.15). Pitfall 28 check: set_effect Invert vs none gives PSNR 7.5 (256) / 6.8 (1080) on both apps, so render_frame DOES apply the legacy chain (see ISSUES).

### T4 must-not-change sweep
**Live** (every source at explicit defaults + every effect on test_card.png, 256x256 and 1920x1080, t = 1.13; main run a, main run b, lane):

| Set | Count | Result |
|---|---|---|
| captures | 486 | -- |
| byte-identical main-vs-main (deterministic) | 461 | -- |
| ... changed on the lane | 14 | Dot Field 256 / 1080 (PSNR 16.0 / 15.2), burning_ship x2 (7.8 / 7.0), julia_set x2 (9.3 / 8.7), newton_3d x2 (11.4 / 11.3), sierpinski 256 (16.2), crystal_cavern x2 (29.3 / 30.2): all INTENDED. julia_set_3d 256 (83.5): LSB codegen of the Iterations loop bound (plan allows >= 60). line_pattern 1080 (9.2) / waveform_lines 1080 (15.5): the SAME image at a different time phase (`t4_timephase_line_pattern_waveform_lines_1080.png`; byte-identical offline) = 1080p capture timing, see ISSUES |
| ... unchanged | 447 | byte-identical (PSNR inf) |
| non-deterministic on main | 25 | all temporal effects or 1080p captures; 21 have PSNR(main, lane) >= 30. The other 4 (Tunnel 7.8, Wave 10.0, metaballs 27.7, Glitch Displace 14.1) equal their own main-vs-main noise (7.8 / 10.0 / 27.7 / 14.6) |

**Offline** (deterministic; `t4-offline-full-compare.txt`, generator `t4-offline-gen.py`): all 107 registry sources + 135 effects. Each is rendered with the PRE-CHANGE shader text + pre-change registry defaults and with the lane's, at 256 / 1080 and t = 0 / 1.13 (968 renders), plus a pre-change repeat as the GPU noise floor. Output changed ONLY for: julia_set, burning_ship, newton_3d, sierpinski (256 only), crystal_cavern, Dot Field (intended); julia_set_3d (PSNR 83-94, LSB); fluid_dynamics (61-71) and line_generator (93-95), whose pre-change repeat differs by the same amount. Every torus source, mandelbulb, band_tower and structural_landscape: byte-identical (their removed params were never read). Neutrality at the values OLD files hold: Dot Field size 0.3 / depth 0.4, and mandelbulb Iterations 0.4, are byte-identical vs the pre-change shader at both sizes (`green-neutral-c6.txt`).
Contact sheet of the live changed set: `source-defects-shots/t4_changed_contact_sheet.png`.

### Existing probes (re-run on the lane app, never re-thresholded)
effects-parity `PY 46 PASS / 0 FAIL` GREEN; crossfade `PY 35 PASS / 0 FAIL` GREEN (its compositions carry Source clips, so they load through the reconcile); fitmode `PY 10 PASS / 0 FAIL` GREEN (Source clips too); deck-tabs `6 PASS / 0 FAIL` (appendDeckFromFile path). The rest of the battery touches nothing this lane changed and was not re-run.

## UI EVIDENCE (`.harmony/.reports/s-rta-0927/source-defects-shots/`)
- Raw decoded renders `before_*.png` (main app) / `after_*.png` (lane app) plus labelled `pair_*.png`. Covers julia_set, burning_ship, newton_3d (t = 0 and t = 30) at 1920x1080 and 256x256; sierpinski 256 (+1080, 1024 @ 0.75); crystal_cavern t = 1 / 5 / 10 at both sizes; Dot Field default at both sizes.
- `t4_changed_contact_sheet.png`; `t4_timephase_line_pattern_waveform_lines_1080.png`.
- Clip inspector, twisted_torus (window-only Quartz capture by window id; state made by a TEMPORARY env-var hook, see D9): `inspector_twisted_torus_before_after_triptych.png` (inspector column cropped from the three window shots) + the full window shots `inspector_before_twisted_torus_window.png` (pre-change code: 14 source rows -- Orbit, Tilt, Zoom, Lens Shape, Lens Rotate, Depth Fade, Pinch, Heart, Shading do nothing), `inspector_after_twisted_torus_new_clip_window.png` (lane: 5 rows -- Twist, Stripe Count, Speed, Tube Radius, Color Shift), `inspector_after_twisted_torus_old_file_window.png` (lane loading the OLD composition saved by the pre-change app, `source-defects-evidence/old-composition-saved-by-prechange-app.json`, 14 params in the file: it loads, the preview renders it (mean 114.8, 52.9% lit) and the inspector shows the reconciled 5 rows).

## BORIS CHECKS (plain words; the default already ships)
1. Seven tube/torus pictures (Striped Torus, Spiral Vortex, Checker Torus, Ribbed Vortex, Wormhole Tunnel, Twisted Torus, Wormhole) had 9-11 knobs that did nothing. They are deleted. Alternative: bring them back and make them work (a separate multi-day job).
2. Dot Field's "depth" knob now sets how strongly brightness drives dot size; its middle position looks exactly like before. New Dot Fields start with bigger dots (size 0.7). Alternative: delete the knob.
3. New starting looks: Julia Set (classic spiral, 1.8x), Burning Ship (the whole ship, as originally designed), Newton 3D (front three-quarter view). Please look at each once. A "richer" Burning Ship framing (hull + flames) is a one-line change.
4. Newton 3D's auto-rotate now circles around the landscape instead of tumbling under it (it used to go black half the time). Option: also show the landscape's underside instead of black when the angle knob points under it.
5. Crystal Cavern is now an endless fly-through (a repeating field of crystals) instead of black after two seconds. When the camera grazes a crystal, that crystal shows as a flat dark cut disc near the edge for a few frames. Alternative: keep one cave and drift the camera back and forth inside it.
6. About 56 knobs (Thickness, Count, Size, Width, Rays, Branches, Glow, Height) draw nothing when turned fully down. Kept, so one knob can empty a layer; they will be listed as known exceptions in the tests. Alternative: force a small minimum.
7. NEW: Julia Set 3D's "Iterations" now works, but above the middle it barely changes the picture (most detail is already there at the default). Keep as is, or narrow its range so the whole knob does something?
Note (no question): saved shows keep working. Deleted knobs vanish from the inspector the next time a show is opened. Clips already made keep their own values, but right-click reset now goes to the NEW starting value.

## DEVIATIONS FROM THE PLAN
- D1 T2 parses the registered defaults from the registry source at test time instead of pinning a table: it renders exactly what ships, so no pin can drift. RED is therefore by pixels on the pre-change registry.
- D2 sierpinski cap is `int(log2(H * zoomPow) + 0.001)`, not bare `int(log2(...))`: log2 of an exact power of two must not floor one level low on the very sizes the fix targets.
- D3 crystal_cavern: added a conditional near-clip (`if (caveDE(ro) < 0.08) totalDist = 0.08`, identical whenever the camera is clear). With the plan's repetition alone the camera passes through crystal walls ~2.7% of the time (numpy) and every ray hits at distance 0, giving flat one-colour frames: 10 in the 20 s GPU sweep (mutant RED above), 0 with the near-clip.
- D4 crystal_cavern changes at all times (t = 0 PSNR 37-40), not only t >= 2.5 as the plan's T4 intended-set said.
- D5 T1 adds a structural rule: a param-adding helper lambda that constructs no ProceduralSource fails. RED count is therefore "1 structural + 22 dead", not "80 + 1": the 70 torus knobs sat behind the lambda, whose 12 params the nearest-key rule attributes to apollonian_3d (9 read as dead). That misattribution is the hiding mechanism Pitfall 45 names.
- D6 reconcile follows the registry ORDER (the plan says erase / refresh / append); the lookup helper is file-local in MainComponent.cpp (no header change).
- D7 T2 Iterations check = "the control changes the picture somewhere in its range". julia_set_3d 0.4 -> 1.0 is PSNR 76.5 (Boris check 7); 0.4 -> 0 is 43.7.
- D8 the dead `uniform` declarations are left in the shaders (plan: optional); they are harmless and keep the diff surgical.
- D9 inspector shots: no REST route selects a clip into the inspector, so a TEMPORARY `AUDIODNA_TMP_SHOT` hook in MainComponent.cpp created the state ("make:<source>:<save.json>" drops a source exactly like the UI drop path, then saves; "load:<file>" loads through `loadComposition`; then the clip viewport is scrolled to the first parameter row). It was never committed. It was built into scratch copies only: base code + hook for "before", lane code + hook for "after". The lane app was rebuilt without it; strings check for `AUDIODNA_TMP_SHOT` in `build-lane/.../Audio-DNA` = 0. The "old composition" used for the after-load shot was written by the pre-change app's own serializer (twisted_torus with its 14 old params).
- D10 T4: added the deterministic offline compare because the live 1080p captures are noisy (see ISSUES).
- D11 `tests/visual/test_fractals.py` (plan-named rows) and `tests/visual/SHADER_VERIFICATION.md` (plan-named doc) were edited, although the packet lists "tests/visual harness files" as do-not-touch. Neither is a harness file (the harness lane may still touch them: merge note).
- D12 launch form `open -g [--env] --stdout --stderr <App> --args --test-mode` (test mode has no env switch; same form as probe-canvas / probe-deck-clock and the diagnosis).

## ISSUES / FOUND-NOT-FIXED
- Pitfall 28 ("Eyes render_frame doesn't apply effect chain") is stale for the legacy set_effect path: Invert vs none PSNR 7.5 / 6.8 on both apps. Not edited (out of plan).
- render_frame at the composition's OWN size (1920x1080) is not reproducible. Across two runs of the same main app, 25 of 486 captures differ (angular_grid PSNR 11.6, Tunnel 7.8, Wave 10.0), and line_pattern / waveform_lines come back at a different time phase. At 256 the canvas-resize wait serialises the capture and it is deterministic. This is a harness finding for the harness lane (INFERRED mechanism: the capture does not wait for a frame rendered at the requested time).
- fluid_dynamics and line_generator render nondeterministically even offline (the same shader twice: PSNR 61-71 / 93-95).
- ProjectMSource's params are C++-consumed and outside SourceRegistry.cpp, so the lint does not cover them (by design).
- ControlPath CAN address a source param by index (`ManualWrite.cpp:130-135`, fx < 0). Today no production path builds one; the only builder is the V1 take import, left unresolved by design (`Take.cpp:382-387`). So removing / reordering params is safe now. A future source-param lane must go through the name-checked resolution.
- My own process slip, no effect on the product: two of my queued wait loops used `pgrep -f <script>`, which matched my own tool-call wrapper shells and waited forever. I killed my two waiting scripts; no app was running and no lock was held by them.

## SKILL_PROPOSALS
none

## RISKS
- The new defaults (C1-C3) and the cavern look are user-visible design calls (Boris checks 3-5).
- `test_source_defaults_gl` SKIPs loudly on a machine with no GL pixel format (CI without a GPU would not gate these pictures).
- The reconcile drops a param's connection if the registry no longer has that param (only the 78 dead ones today).

## METRICS
6 fix commits + C7 evidence; 4 new/extended tests (T1, T2 7 cases, reconcile, 2 CMake targets); live: 3 main-app runs and 2 lane runs (T3 x2, T4 x3), 4 inspector-shot runs (1 v1 before-shot superseded: rows were below the fold), 4 existing probes; offline: 968 + 22 GPU compare renders. No perf claims (none were planned; no perf numbers measured).

## PACKET QUALITY
- Clarity: CLEAR (plan + diagnosis precise; line numbers still valid on 2ee1013).
- Missing context: the plan said ControlPath "param" is effect-slot-only; it is not (ManualWrite fx < 0, see ISSUES), though no path uses it. No REST route selects a clip (needed the D9 hook). test_fractals.py vs "tests/visual harness files" is ambiguous (D11).
- Unused context: none.
- Self-brief files: CLAUDE.md (useful: pitfalls 8/17/19/27/28/37), docs/claude/pitfalls.md (format), fractals.md (useful), .harmony/notebook.md headings (render_frame reliability notes, useful), tier1-diag.md + numpy rigs in scratchpad/arch (very useful: every GPU number matched the numpy prediction).

### STATUS
DONE_WITH_CONCERNS

### NEXT ACTION
Harmony: behavioral gate (the pictures via 8080 + the inspector shot) and reviewer on 47da170..HEAD; bring the 7 Boris checks; hand the two render_frame findings to the harness lane.

---

# Fix round (source-defects-fix, 2026-09-27 21:00-21:36)

STATUS: DONE_WITH_CONCERNS
RESULT: I checked all five review findings. Two are fully confirmed, and I fixed the MUST: crystal_cavern no longer draws a flat dark cut disc where the camera grazes a crystal (9ba79b3). The other confirmed finding, the lint's "READ" check being only textual, is now written into the lint and SHADER_VERIFICATION.md as DEBT_FILED (a433f45). SHOULD 3 is half confirmed: striped_torus's camera uniform is live but has no knob. Its spectrum_landscape half is wrong: that source's knob is wired. SHOULD 2 (the look) and SHOULD 4 (dropped connections) are confirmed; they need a decision from Boris, not code. Concerns: the fix costs +1.9 ms GPU per 1080p frame. Also, a close crystal now fades to see-through, which is a new look Boris should see (check 5, replaced below).
FACTS:
- Fix-round commits on `lane/source-defects-0927`: 9ba79b3 (crystal_cavern near fade + T2 case), a433f45 (DEBT_FILED note); this report + evidence = the next commit. Source: `git log 4654438..HEAD`.
- The previous 0.08 near-clip was active for 85.99% of the flight, not "only while grazing". The numpy camera path over 0..120 s at the defaults gives camera DE < 0.08 for 85.99% of the time, < 0 for 1.45%, and DE 0.0117 at t = 10. Source: `source-defects-evidence/fixround-camera-distance.txt`.
- New T2 case "crystal_cavern draws a grazed crystal and never a cut-open dark disc". On the previous shader it fails: "grazed-crystal box: 85.3946% dark" (t = 10) / "90.356% dark" (t = 24), `test cases: 1 | 0 passed | 1 failed`, `assertions: 12 | 10 passed | 2 failed`. On the fix: 10.8096% / 7.89167%, `All tests passed (12 assertions in 1 test case)`. Source: `fixround-RED-graze-prev-shader.txt`, `fixround-GREEN-graze.txt`.
- Whole T2 on HEAD: `All tests passed (1540 assertions in 8 test cases)`. The existing 20 s sweep: `96x54 sweep 0..20 s step 0.05: 0 black, 0 flat frames`. T1 lint: `All tests passed (1389 assertions in 1 test case)`. Full ctest serial: `100% tests passed, 0 tests failed out of 752`. Source: `fixround-T2-final.txt`, `fixround-T1-lint.txt`, `fixround-ctest-final.summary`.
- Live on 8080 (explicit registered defaults, decoded PNGs), grazed-crystal box dark at 1920x1080, previous lane app -> fix-round app: t = 10: 85.4% -> 10.8%; t = 24: 28.6% (INVALID, see NUANCE) -> 7.9%. Live-vs-offscreen PSNR: inf for prev t = 10 and for fix t = 10 / 24. Source: `fixround-live.log`.
- GPU time at 1920x1080 over t = 0..30 (240 frames, GL timer queries, private CGL context, no clang and no Audio-DNA running, load 2.93 -> 2.00): previous shader median 3.257 / 3.259 ms, fix 5.177 / 5.173 ms (two passes each). Source: `fixround-gpu-perf.txt`.
METHOD: I checked each finding against the source and the pixels before changing anything. I ported the camera path and caveDE to numpy to see how often the clip was active. I built a scratch offscreen explorer (the T2 rig plus a shader-from-file loop; `fixround-explore-rig.cpp`) and compared 4 candidates over 0..30 s and one inside-a-crystal episode: the previous shader, no clip, "exit the crystal only when inside", and three near-fade widths. I wrote the T2 case, got RED on the previous shader, edited the shader, then got GREEN. I built the app and froze copies of the previous and the fix-round apps. I ran one live batch per app under `/tmp/audiodna-live.lock` as `source-defects-fix` (launched with `open -g ... --args --test-mode`, quit via osascript), then re-shot the evidence.
CONFIDENCE+VERIFY: high for the MUST. The GREEN numbers are identical live and offscreen (PSNR inf), and the fix block's code equals the prototype I measured (comments aside, checked by script). VERIFY: `build-lane/tests/test_source_defaults_gl "crystal_cavern draws a grazed crystal and never a cut-open dark disc" -s`; `ctest --test-dir build-lane -j1`; look at `source-defects-shots/fixround_pair_crystal_cavern_1920x1080_t10.png`.
UNKNOWNS/NOT-DONE: Tier-1 pytest was not run (same reason as the main round: the harness lane owns it). Existing probes were not re-run: no probe or test other than T2 names crystal_cavern (grep of scripts/ and tests/), and the fix changes only that shader block. The SHOULDs got no code: 2 needs a decision, 3 needs a follow-up, 4 is informational.
NUANCE: The live 1080p capture of the PREVIOUS app at t = 24 came back at another time phase (a different view: PSNR 16.5 vs the offscreen t = 24; shown in `fixround_prev_live_WRONG_TIME_PHASE_crystal_cavern_1920x1080_t24.png`). This is the render_frame capture-timing issue already under ISSUES. So its 28.6% is not a t = 24 measurement. That pair uses the deterministic offscreen render of the previous shader instead, labelled as such. RED at t = 24 rests on the offscreen gate (90.4%); live RED rests on t = 10 (85.4%, byte-identical to offscreen).
HANDOFF-NEEDS: Harmony: behavioral gate + reviewer on 4654438..HEAD. Boris: replaced check 5 and new check 8 (below). Follow-ups: the striped_torus camera knob, and the newton_3d cost (ISSUES).

INBOX-RECHECK: none

## Findings: verdicts

| # | Sev | Finding | Verdict | Action |
|---|---|---|---|---|
| 1 | SHOULD | lint "READ" is textual; `mix(energy, energy, u_src_smoothing)` passes | VERIFIED. `EmbeddedShaders.h:10582` is in `sourceSpectrumLandscape` (block starts 10561); spectrum_landscape registers Smoothing (`SourceRegistry.cpp:987`); `isRead()` (`tests/test_shader_param_lint.cpp`) accepts any non-declaration use | DEBT_FILED in the lint header + `tests/visual/SHADER_VERIFICATION.md` (a433f45). No code change: pre-existing and outside A2 |
| 2 | MUST | crystal_cavern near-clip renders a flat dark disc in place of a crystal (t = 10, 1080p) | VERIFIED, and worse than stated. The clip was on 86% of the flight (not only while grazing), so every crystal nearer than 0.08 vanished and any crystal the 0.08 sphere cut showed its inside at ambient level. Offscreen and live at t = 10: 85.4% of the crystal's box dark | FIXED 9ba79b3 (below) |
| 3 | SHOULD | crystal_cavern reads as tiled / boxy | VERIFIED as a look. Same sphere field, same camera; the fix does not change the composition | Product decision: existing Boris check 5 alternative stands; no code |
| 4 | SHOULD | striped_torus `u_src_camera` is live but unwired; "check sourceSpectrumLandscape which has the same" | striped_torus VERIFIED (`EmbeddedShaders.h:7943` declared, read at 7960 `camAngle = u_src_camera`; no `u_src_camera` in its registry block, `SourceRegistry.cpp:640-648`; pre-change registry had none either). spectrum_landscape REFUTED: it registers "Camera Angle" -> `u_src_camera` (`SourceRegistry.cpp:984`), read at `EmbeddedShaders.h:10586` | Filed as follow-up (below); not implemented (new control = scope) |
| 5 | SHOULD | a saved connection on one of the 78 removed params is dropped on load | VERIFIED: `compload::reconcileSourceParams` (`src/core/CompositionLoad.h:100-140`) rebuilds `sourceParams` from the registry list, so a removed param's `SourceParam` (with its connection) is discarded. No picture changes (the param never reached the shader) | Informational; no load-notice pattern exists to hook. Kept under RISKS |

## The MUST fix (9ba79b3): near fade instead of near-clip
- `src/render/EmbeddedShaders.h` `sourceCrystalCavern` only. Every ray now marches from the camera. A crystal hit at distance `d` is drawn with weight `smoothstep(0, 0.08, d)`, composited front to back, and the ray continues through the crystal to what lies behind it. It stops once less than 1% of the ray still shows. Every hit 0.08 or more away is drawn exactly as before (weight exactly 1). A crystal the camera brushes past turns see-through as it nears, with no hard edge. A camera inside a crystal sees through it: no flat frame, no cut face.
- Why not the other candidates (`fixround-variants-contact.png`, `fixround-variant-flatness.txt`):
  - No clip at all flashes the whole frame one flat colour whenever the camera is inside a crystal: the frames at 6.3, 9.5, 10.1, 19.9, 20.0 and 23.8 s were 100% flat blocks. `fixround-inside-episode-head-repnoclip-exit.png`, middle column.
  - "Step out only when inside" is cheaper (median about 4.0 ms), but at normal flight a shadowed crystal then fills up to 92% of the frame at ambient level (t = 20.1). That is the same "flat dark" complaint.
- The frame composition away from the camera is unchanged. Offscreen 0..120 s (2401 frames, 96x54): fix 0 black / 0 flat, lit median 80.0% (previous 79.2%).
- Cost (INFERRED cause: rays now also march the first 0.08 and pass through near crystals): GPU median 3.26 -> 5.18 ms per 1920x1080 frame. For scale, same rig: mandelbulb 2.1, julia_set_3d 1.0, newton_3d 56.1 (ISSUES).

### Tests (verbatim)
| Test | Previous shader (4654438) | Fix round (HEAD) |
|---|---|---|
| T2 "crystal_cavern draws a grazed crystal and never a cut-open dark disc" | `test cases:  1 \|  0 passed \| 1 failed` / `assertions: 12 \| 10 passed \| 2 failed` (85.3946% / 90.356% dark) | `All tests passed (12 assertions in 1 test case)` (10.8096% / 7.89167%) |
| T2 whole file | -- | `All tests passed (1540 assertions in 8 test cases)` |
| T1 lint (after the comment edit) | -- | `All tests passed (1389 assertions in 1 test case)` |
| ctest serial | -- | `100% tests passed, 0 tests failed out of 752` |

### Live rows (8080, explicit defaults; previous lane app -> fix-round app; `fixround-live.log`)
| size | t | previous | fix round |
|---|---|---|---|
| 256 | 1 / 5 / 10 / 24 | lit 71.1 / 87.2 / 83.4 / 53.0% | lit 74.9 / 86.8 / 87.6 / 56.0% |
| 1920x1080 | 1 / 5 | lit 73.2 / 87.3% | lit 75.3 / 87.0% |
| 1920x1080 | 10 | lit 81.2%, grazed-crystal box 85.4% dark | lit 83.7%, box 10.8% dark |
| 1920x1080 | 24 | (wrong time phase, see NUANCE) | lit 69.6%, box 7.9% dark |
| 96x54 sweep 0..20 s every 0.25 s | -- | 0 black-or-flat of 81 | 0 black-or-flat of 81 |

### Re-shot evidence (`source-defects-shots/`)
- `after_crystal_cavern_{256x256,1920x1080}_t{1,5,10}.png` and their `pair_*.png`: re-shot on the fix-round build. BEFORE is still the main app, black from t = 2.5.
- NEW `fixround_pair_crystal_cavern_1920x1080_t10.png`: previous lane build vs fix round, both live, with the grazed crystal's box drawn. The flat disc becomes a lit crystal with its highlight.
- NEW `fixround_pair_crystal_cavern_1920x1080_t24.png`: the previous shader rendered OFFSCREEN, labelled, vs the fix round live.
- NEW raw captures: `fixround_prev_*`, `fixround_fix_*`, and `fixround_prev_live_WRONG_TIME_PHASE_*` (kept for the record).

## BORIS CHECKS (fix round; the default already ships)
5 (REPLACES check 5 above). Crystal Cavern is an endless fly-through. A crystal the camera brushes past now fades to see-through as it gets close, instead of showing a dark cut-open disc. Very close crystals look like glass ghosts for a moment. Alternatives: keep one cave and drift the camera inside it, or push the camera away from crystals so nothing ever gets that close. Both are a new look, which is your call.
8 (NEW). Crystal Cavern now costs about 5.2 ms of GPU per 1080p frame instead of 3.3 ms. That is the price of drawing close crystals properly. It is fine on its own. If you stack it under heavy effects, a cheaper version is possible: close crystals just vanish, with no fade.

## ISSUES / FOUND-NOT-FIXED (fix round)
- FOLLOW-UP (review SHOULD 4): striped_torus's shader has a working `u_src_camera` (0 = side vortex, 1 = top-down hole) that no knob sets, so it always renders 0. Exposing it means adding a "Camera" param with default 0.0, which leaves today's picture unchanged. Deferred as a new control.
- DEBT_FILED (review SHOULD 1): spectrum_landscape Smoothing is a dead knob that the lint cannot see: `mix(energy, energy, u_src_smoothing)`. Fixing it means implementing temporal smoothing, or removing the knob.
- NEW (measured, pre-existing, not this lane's regression): newton_3d costs about 56 ms of GPU per 1920x1080 frame at the lane defaults, roughly 18 fps. The pre-change shader at its pre-change defaults costs 81.7 ms median (p95 147). Source: `fixround-gpu-perf.txt`. It deserves its own perf pass.
- render_frame at 1920x1080 returned another time phase once more: the previous app at t = 24, PSNR 16.5 vs offscreen. This is the harness finding already listed above.
- Process: every `git commit` in this worktree prints `[graphify hook] launching background rebuild`. That is a repo hook I did not add, and it compiles nothing (no clang during the perf run).

## RISKS (fix round)
- A user-visible look change: close crystals are now translucent (Boris check 5).
- +1.9 ms GPU per 1080p frame for this one source (Boris check 8).
- The T2 grazed-crystal case pins two frames and two boxes. A deliberate change to the camera or the cave will need new frames or boxes. The case says so in its comment.

## METRICS (fix round)
2 commits + this report. 1 new T2 case. Offscreen: about 5,000 explorer renders (7 variants), 3 GPU timing runs. Live: 2 app runs (prev, fix), each holding the lock for about 1 min. ctest serial x2 (752/752 both).

## PACKET QUALITY (fix round)
- Clarity: CLEAR.
- Missing context: finding 4's spectrum_landscape claim was wrong (that knob is wired). Finding 2 understated the clip's reach (86% of the flight, not only grazing).
- Unused context: none.
- Self-brief files: the main-round scratch rigs (cavern_fix.py numpy port, T2 rig, lc.py / live.sh) were all reused.

### STATUS
DONE_WITH_CONCERNS

### NEXT ACTION
Harmony: behavioral gate + reviewer on 4654438..HEAD (look at `fixround_pair_crystal_cavern_1920x1080_t10.png`). Take Boris checks 5 (replaced) and 8 to Boris. File the striped_torus camera knob and the newton_3d cost as follow-ups.
