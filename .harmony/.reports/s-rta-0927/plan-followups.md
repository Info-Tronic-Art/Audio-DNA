# Plan: s-rta-0927 follow-ups -- F1 Tier-1 harness, F2 capture file overwrite, F3 beat-crossing readers

Lane: s-rta-0927 · Author: Architect (Fable) · Base: main @ 5267a0c (read now) · Inputs: `.harmony/.reports/s-rta-0927/tier1-diag.md` (read in full), `renderperf.md` found_not_fixed #4-#5, `docs/claude/pitfalls.md` Pitfall 42. Three independent items, one commit group each. Every line number below is as of main 5267a0c; VERIFIED = read in source at the cited line, INFERRED = derived from cited evidence, ASSUMED = named as such.

## State of main (VERIFIED, `git log` / `git worktree list` at write time)
- main = 5267a0c (renderperf merge). NEITHER `lane/source-defects-0927` (6 commits 47da170..0715cb7, worktree rta0927-w10) NOR `lane/outputs-c3-0927` (10 commits ..1e5a00c, w9) has merged. The work log says both are DONE + reviewed.
- source-defects footprint vs its merge-base 2ee1013: `SourceRegistry.cpp` (759 -> 681 params: 78 dead controls removed, 3 implemented), `EmbeddedShaders.h` (newton_3d yaw-only orbit, sierpinski depth cap, crystal_cavern endless cave, Dot Field depth), `EffectLibrary.cpp` (Dot Field size 0.3 -> 0.7), `MainComponent.cpp` (+29, `compload::reconcileSourceParams` on load), new ctests `test_shader_param_lint.cpp` + `test_source_defaults_gl.cpp`, `tests/visual/test_fractals.py` defaults, `SHADER_VERIFICATION.md` (+8 lines after line 31), Pitfalls 43/44/45. It does NOT touch TestServer.cpp, Renderer.cpp, Autopilot.cpp, conftest.py or the five Tier-1 files -- so F1/F2/F3 auto-merge with it (different files or hunks; INFERRED from the stat).
- outputs-c3 footprint vs its merge-base dc7adf9: TestServer.cpp hunks at :241, :640, :1759+ (none inside handleLoadSource :721-770 or the output_probe write :1728-1735 -- F1/F2's hunks are >= 25 lines away, git auto-merges; INFERRED), TestServer.h, MainComponent.cpp (+67, output/settings regions), output/, AppSettings, tests/CMakeLists.txt (+41). No overlap with F3's regions.
- ctest on main's build dir: 749 registered (`ctest -N`).

QUESTION: For F1 (Tier-1 harness), F2 (capture overwrite) and F3 (beat-crossing readers): which files/lines change, what is RED first, what gates prove it, what must not change, and what can go wrong -- concrete enough for one Builder per item with zero re-derivation.

APPROACH (all three, stated first):
- **F1**: fix H1 in the SERVER (`TestServer::handleLoadSource` seeds the registry's full default list, then overlays the request's values -- exactly what a UI clip carries, `MainComponent.cpp:1114-1125`), H2 in a module-scoped conftest fixture that sets the composition to 256x256 for the four capture-at-256 modules only, H3/H4/E inside the four test files with a generic candidate ladder instead of per-name heuristics, a `tests/visual/tier1_exceptions.py` module where EVERY entry carries its justification, built from the post-fix re-run -- and no "app defect" suppression list: whatever is still red after the harness is right is named in the report with file:line. F1 should run AFTER source-defects merges (its A1-A5 fixes are the only path to GREEN; without them the residual is 6 named app defects, listed in F1.7).
- **F2**: one header-only PNG writer (`src/render/PngWrite.h`, delete-then-write, sibling of `PixelConvert.h`) used by BOTH PNG sites; `takeSnapshot` takes a non-existent sibling name so two snapshots in one second are two files. RED-first ctest (`tests/test_png_write.cpp`) modelled on `test_pixel_convert`.
- **F3**: each of the four readers owns one `OnsetPulse` (`src/features/OnsetPulse.h` -- the existing generic monotonic-counter delta, already sanctioned for `totalBarCount` by Pitfall 32) fed `snap.totalBeatCount`; counters integrate the delta, edges fire once per tick when the delta > 0. RED ctests in `test_autopilot.cpp` with a synthetic 120-BPM snapshot stream containing a stall; the deck-clock probe and the three existing autopilot test files publish `totalBeatCount` with their phase sawtooth (Pitfall 42's injection rule).

TRADEOFFS CONSIDERED:
- F1/H1 server seed vs test-side explicit defaults -- SERVER. One site fixes all 23 `load_source` call sites in 10 files (incl. test_range_quality, test_fractals, test_signals, test_mapping_tick, test_render_pipeline and the `probe-deck-path.sh` probe); the Eyes path then means what the UI path means; test-side touches ~8 sites in 4 files and leaves the others stale. Counter: "the server change alters range-quality/fractals semantics" -- it gives them true defaults, which is what they assume (neither passes `params` to `load_source`; VERIFIED grep). Loses.
- F1/H2 test-side composition 256 vs app-side state-preserving resize (rescale-blit of the ping-pong FBOs like `CompositorEngine::rescaleHistory`) -- test-side (one fixture; the dispatch prescribes it; the app change is a lane of its own and would still reset projectM).
- F1/H3 metric: p99.5 of per-pixel max channel < 16 (sources + time sweep only). Keep the effects' `mean < 3.0` -- it produced exactly one failure (Dot Field, a real defect fixed by source-defects). Rejected: adding "lit >= 5%" (source-defects plan's T2 rule) for sources -- line art covers 1-7% (diag H3), it would re-flag strange_attractor et al.
- F1/H4 candidate ladder [primary, 0.5, 0.25] vs per-name angle heuristics -- ladder. It is generic, renders extra frames only for params that would otherwise fail, and also absorbs a bug the diag did not name: `default_val > 0.3` compares a float32 default (0.3f = 0.30000001) with a Python double, so Oil Paint radius (registered 0.3f, `EffectLibrary.cpp:382`) took test value 0.0 = the same quantised radius (INFERRED from the diag's "0.3 and 0.0 are both radius 2" + `EffectLibrary.cpp:382`).
- F1 exceptions: a list per cluster with a justification string per entry (the spec's "known exceptions", `SHADER_VERIFICATION.md:165-170`) vs xfail/suppression of app defects -- no suppression: an unfixed app defect keeps the test red and is named with file:line (the dispatch's "GREEN or named").
- F2 delete-first vs `setPosition(0)+truncate()` -- delete-first (precedent `TestServer.cpp:1729`; one fewer branch; `FileOutputStream::truncate()` exists too, either is correct). Snapshot names: `getNonexistentSibling(false)` (JUCE 8.0.4, `juce_File.h:335`) vs millisecond names -- sibling (keeps the documented `snapshot_YYYYMMDD_HHMMSS.png` form; the collision case becomes `..._2.png`).
- F3 reuse `OnsetPulse` vs a new `BeatCrossing` helper -- reuse (Rule: no new pattern where one fits; its backwards-jump re-baseline replaces the old `lastBeatPhase_ = 0` resets exactly). Delta-integration (`+= beats`) vs edge-only (RoutineEngine's `count != last`) for the counters -- integration: a 2.3-beat stall must add 2 beats to `beatsPlayed`, and without a stall delta is 0 or 1 at every reader's cadence, so `+= beats` IS the old `++`.
- F3 probe: derive `totalBeatCount` server-side from an injected phase wrap when absent vs the injector carries it -- the injector carries it (Pitfall 42's rule, `ApiServer.cpp:883-887` comment; a server-side derivation would re-create the wrap reader on the writer).

---------------------------------------------------------------------------------------------------

## F1 -- Tier-1 visual-test harness (tests/visual, five files)

### F1.0 Preconditions and order
- Prefer to start after `lane/source-defects-0927` merges (see F1.7 for the residual if it has not). Build the lane app from the lane's worktree; every live step under `/tmp/audiodna-live.lock` with an owner file, `open -g --stdout <f> --stderr <f> <App> --args --test-mode`, quit via osascript, >= 45 s between lock holds (the form in `renderperf-evidence/scripts/run-tier1.sh` + `lock.sh`; exactly the five files, never `tests/visual` as a directory).
- Mechanisms this plan relies on (all VERIFIED in source): `handleLoadSource` builds `params` only from the request and calls `renderer_.setActiveSource(sourceType, params)` (`TestServer.cpp:749-763`); the GL thread copies the list each frame (`Renderer.cpp:223`) and passes `nullptr` when it is empty (`:616`), so `renderSource` applies nothing (`:1250-1262`) and the cached instance (`getOrCreateSource`, `:1055`) keeps its last values; `/api/reset` only clears the selection (`:682-719`, `clearActiveSource` `:91-97`). `render_frame` sets the test lock, captures, clears it (`TestServer.cpp:586-592`); the canvas is `resolveCanvas(lock, composition)` (`RenderGeometry.h:20-27`), the capture waits for a frame whose canvas equals the lock (`Renderer.cpp:2137-2143`), and a size change re-creates every stateful source's ping-pong FBOs (`ProceduralSource.cpp:64-71`). `u_bass = bandEnergies[1]` (`ProceduralSource.cpp:160-161`). `set_effect` ignores unknown param names silently (`TestServer.cpp:344-353`), so today's `params={"intensity": 0.5}` in the RMS-effects test was a no-op.

### F1.1 H1 -- `src/test/TestServer.cpp` `handleLoadSource` (`:749-763`)
Replace the `params` build with: seed, then overlay.
```cpp
// Seed the registry's full default list -- what a UI clip carries (MainComponent.cpp:1114-1125) -- then overlay the
// request's values. Without the seed the cached instance keeps whatever the previous caller set: Renderer.cpp:616
// passes nullptr for an empty list and renderSource applies nothing (tier1-diag H1). createSource() is GL-free
// (ProceduralSource.cpp:6-12); handleListSources already calls it on this thread.
std::vector<Clip::SourceParam> params;
if (auto src = sourceRegistry_.createSource(sourceType))
    for (int i = 0; i < src->getNumParams(); ++i)
    {
        const auto& p = src->getParam(i);
        Clip::SourceParam sp; sp.name = p.name; sp.uniformName = p.uniformName;
        sp.value = p.defaultValue; sp.defaultValue = p.defaultValue;
        params.push_back(sp);
    }
if (obj->hasProperty("params"))
    if (auto* paramsObj = obj->getProperty("params").getDynamicObject())
        for (auto& prop : paramsObj->getProperties())
        {
            const std::string u = prop.name.toString().toStdString();
            const float v = static_cast<float>(static_cast<double>(prop.value));
            auto it = std::find_if(params.begin(), params.end(), [&](const Clip::SourceParam& s) { return s.uniformName == u; });
            if (it != params.end()) it->value = v;
            else { Clip::SourceParam sp; sp.uniformName = u; sp.value = v; params.push_back(sp); }   // unknown name: appended as today, renderSource ignores it
        }
```
`update_source_params` (`:772-800`) stays a REPLACE of the list with the given subset: the instance already holds the defaults applied on the frames after `load_source`'s 100 ms sleep (`:766`), so a one-param update renders defaults + that param (the sequence every Tier-1 test uses). Do not add a merge there.
RED/GREEN (live, no ctest links TestServer.cpp -- `tests/CMakeLists.txt:167-171`): the diag's E1 row -- load `torus_hole`, `update_source_params` all params = 1.0, render (mean ~0.01), `load_source("torus_hole")` with no params, render: main = mean 0.01 (RED), lane = mean ~88 (PSNR inf vs a fresh-app render; GREEN). Put both numbers in the report.

### F1.2 H2 -- `tests/visual/conftest.py` + `tests/visual/vj_controller.py`
`vj_controller.py`: add `set_composition_params(self, **fields)` -> `self._post("/api/set_composition_params", fields)` and `get_composition_params(self)` -> GET `/api/composition_params` (handlers `TestServer.cpp:1378-1448`, `:1450-1494`; both keys required together, ints in [16, 7680] x [16, 4320]).
`conftest.py`: a module-scoped autouse fixture that requests `app` ONLY for the four modules (gotchas.md 2026-05-18: non-Eyes files in tests/visual override `app`; an unconditional dependency would drag them in):
```python
TIER1_CANVAS_256 = {"test_sources", "test_effects", "test_audio_reactivity", "test_time_sweep"}

@pytest.fixture(scope="module", autouse=True)
def tier1_canvas_256(request, tmp_path_factory):
    """H2: with the composition AND the capture lock both 256x256 the canvas never resizes between captures, so
    stateful sources keep their state (ProceduralSource::resize re-creates the FBOs; Renderer::resolveCanvas)."""
    name = request.module.__name__.rsplit(".", 1)[-1]
    if name not in TIER1_CANVAS_256:
        yield; return
    app = request.getfixturevalue("app")
    before = app.get_composition_params()
    app.set_composition_params(outputWidth=256, outputHeight=256)
    # warm-up: the GL thread applies a size pair one frame after it first sees it (Renderer.cpp:262-274); the capture
    # returns only once the canvas is at the lock size, so this render is the "size applied" barrier.
    app.render_frame(str(tmp_path_factory.mktemp("canvas") / "warmup.png"), time_val=0.0, width=256, height=256)
    yield
    app.set_composition_params(outputWidth=before["outputWidth"], outputHeight=before["outputHeight"])
```
`test_performance.py` is deliberately NOT in the set (it captures at 512, today's 1080p canvas behaviour is kept). `/api/reset` (autouse `reset_between_tests`, `conftest.py:73-77`) never touches outputWidth/Height (`handleReset`, `TestServer.cpp:682-719`), so the setting survives the per-test reset. Module fixtures tear down before the next module's tests run, so performance sees the restored size (standard pytest scoping; INFERRED).

### F1.3 H3/H4/E -- the four test files
Shared helpers (put them in `tests/visual/tier1_exceptions.py` next to the exception tables so the four files import ONE module; keep the files' own `psnr_between`):
```python
def is_black(path):                       # H3: sparse line art has a mean of 1-5 but a bright top 0.5 % (diag: 9/108 true blacks flagged, dimmest live frame 34)
    img = cv2.imread(path)
    return True if img is None else float(np.percentile(img.max(axis=2), 99.5)) < 16.0

def test_values(default):                 # H4: the old rule first (passing params render exactly as before), then the periodic/quantised fallbacks
    primary = 0.0 if default > 0.3 else 1.0
    return [v for v in (primary, 0.5, 0.25) if abs(v - default) > 0.05]

T_PARAM = 1.13                            # H4: t = 1.0 is a no-op for sin(k*2pi*t)/fract(k*t) shaders (Pulse, Color Flash, Strobe, Tunnel; diag E7 verified)

FEATURES_ACTIVE = {                       # E: the audio-native sources/effects are black in silence; constant across a param's default/test renders
    "rms": 0.6, "peak": 0.8, "bpm": 120.0, "beatPhase": 0.25, "totalBeatCount": 1, "barPhase": 0.3,
    "bandEnergies": [0.6] * 7, "spectralCentroid": 2000.0, "spectralFlux": 0.3, "spectralFlatness": 0.3,
    "spectralRolloff": 4000.0, "onsetStrength": 0.5, "chromagram": [0.5] * 12, "mfccs": [0.2] * 13,
    "detectedKey": 0, "keyIsMajor": True, "structuralState": 1,
}
def arm_audio(app): app.inject_features(FEATURES_ACTIVE)   # call after EVERY app.reset() (the autouse reset clears the bus; test_effects also resets inside its loops)
```
`test_sources.py`:
- `brightness(..) < 5.0` (`:44, :74, :78, :122`) -> `is_black(..)`; `time_val=1.0` (`:41, :64, :70, :118`) -> `T_PARAM`.
- `test_all_sources_non_black` (`:34-46`): skip ids in `BLACK_SOURCES`; `arm_audio(app)` once at the top; if black at `T_PARAM`, ONE retry at `time_val=2.0` before failing (strobe_light is 255 at t = 0/2/10 and black at 0.5/1/3/5 -- diag E3; the retry is generic, no name needed).
- `test_all_params_have_effect` (`:52-100`): skip `BLACK_SOURCES`; `arm_audio` once; for each param: default render (load, render), then walk `test_values(default)`: load + `update_source_params({u: v})` + render; a black test render -> if `(src, uniform) in BLACK_AT_EXTREME` proceed, else record `"goes black at v"` and stop this param; else if PSNR(default, test) < 55 the param passes; only if EVERY candidate leaves PSNR >= 55 record `"no visible effect (tried [..])"`. Default black -> `"default is black"` (unchanged).
- `test_no_discontinuities` (`:106-150`): metric swap + `T_PARAM` only (it passes today; print-only).
`test_effects.py`:
- `time_val=1.0` (`:62, :86, :93, :133`) -> `T_PARAM`; `arm_audio(app)` after each `app.reset()` (`:59, :83, :90, :129`).
- `test_all_params_have_effect` (`:74-109`): `test_val` (`:82`) -> the `test_values` ladder (default render once, then candidates until PSNR < 55); skip effects in `LEGACY_CHAIN_EFFECTS`; for names in `ANIMATED_INPUT_EFFECTS` use `app.load_source("plasma")` (params seeded by F1.1) instead of `app.load_image(TEST_IMAGE)` as the input -- the legacy chain renders over a source (`Renderer.cpp:613-620` then `effectChain_.render` `:654`; the diag's live batch (a) used exactly this).
- `test_all_effects_non_black` / `test_no_destructive_effects`: `mean < 3.0` stays; `T_PARAM`; `arm_audio`.
`test_audio_reactivity.py`:
- `FEATURE_PAIRS` (`:28-35`) become 4-tuples carrying the source list: RMS -> today's `AUDIO_RESPONSIVE_SOURCES` (passes today, keep); Beat Phase -> `["twisted_torus", "fermat_spiral", "text_animator"]` (diag E6: PSNR 2.3 / 19.9 / 36.7; the six listed today never read `u_beatPhase`); Bass -> `{"bandEnergies": [0, 1, 0, 0, 0, 0, 0]}` and `["audio_waveform", "perlin_noise", "torus_hole", "geometric_tunnel", "crystal_cavern"]` (diag: 15.5 / 20.9 / 6.7 / 21.4 / 29.6). `changed_count >= 2` stays. `time_val` -> `T_PARAM`.
- `test_rms_affects_effects` (`:83-109`): `effects_to_test = ["Density Wave"]` (of 153 non-source shaders only density_wave and structural_morph read `u_rms`; Structural Morph is gated by structuralState); `set_effect(fx, enabled=True)` with defaults (its params are amount/direction/wavelength, `EffectLibrary.cpp:773-777`; `"intensity"` never existed); `>= 1` stays.
`test_time_sweep.py`:
- `brightness(..) < 3.0` (`:51`) -> `is_black`; skip `BLACK_SOURCES`; `arm_audio` once; times `[0.0, 1.13, 5.13, 10.13]` (integers are sin(2*pi*k*t) no-ops for integer k -- INFERRED from the H4 idiom; the print-only "frozen" list is what it affects); `black_at_time` assertion `< 5` stays; entries whose `(src, t)` matches `BLACK_AT_TIMES` are not counted.

### F1.4 `tests/visual/tier1_exceptions.py` -- tables, every entry with its reason (strings are the documentation)
```python
BLACK_SOURCES = {"layer_router": "Pitfall 21: returns another deck layer's output texture; the Eyes legacy path has no deck layer (Renderer.cpp:1213-1244)"}
BLACK_AT_TIMES = {}            # ("strobe_light", 1.13): "strobe: dark half of each 2 s period" -- ONLY if the t=2.0 retry / sweep still needs it after the re-run
BLACK_AT_EXTREME = {}          # (source_id, uniform): "GLSL: <EmbeddedShaders.h:line> -- <term> is zero at 0.0, nothing is drawn" -- Q6 policy (Harmony ruling default: keep the app, list them)
LEGACY_CHAIN_EFFECTS = {"Screen Split": "Pitfall 19: compositor-only frame history (CompositorEngine applyClipEffects); the legacy /api/set_effect chain runs it as a plain shader",
                        "Frame Stutter": "same as Screen Split"}
ANIMATED_INPUT_EFFECTS = {}    # "Channel Delay": "reads previous frames; a static card gives PSNR 71.9, plasma 24.2 (diag)" etc. -- confirm in the re-run before adding
```
Admission rule for `BLACK_AT_EXTREME` (each entry must satisfy both, and the builder writes the GLSL line into the string): (a) the shader term is provably zero-output at that value (read the source's block in `src/render/EmbeddedShaders.h`; e.g. a thickness/count/size multiplier of 0), AND (b) the ladder proved the param has an effect at another candidate. Anything else is an app defect -> F1.7 list, no entry. Bound: if more than ~40 params qualify after H1-H4/E, stop -- that is a harness problem, not 40 policy items.

### F1.5 Pitfall 28 is stale -- verify, then correct (docs)
Pitfall 28 (`docs/claude/pitfalls.md:65`) says Eyes `render_frame` does not apply the effect chain. `Renderer.cpp:654` runs `effectChain_.render(sourceTexture, ...)` on the legacy path, `test_no_destructive_effects` passes today only because effects DO render, and the diag's E7 measured Dot Field/Chromatic Aberration changing through `set_effect` + `render_frame`. Cheapest refutation (do it in the lane, one capture pair): test card, `set_effect("Invert", enabled=True)`, render vs no effect -> PSNR < 55 means the chain renders. If confirmed, rewrite Pitfall 28 to what is true ("the legacy chain renders over the loaded image or source; the DECK compositor path -- clip/layer/global effects, frame-history effects -- is not what `set_effect` drives") and the CLAUDE.md index line 28; also close `.harmony/HANDOFF.md:146` item 8. If NOT confirmed, stop: the whole effects half of Tier-1 measures nothing, report before continuing.

### F1.6 Re-run and triage (the gate)
1. Build; run the five files once (F1.0 form). Keep `pytest.log`, `outcomes.txt`, `source_param_failures.txt`, `effect_param_failures.txt`.
2. Every remaining line goes in exactly one bin: (a) harness -> fix it and re-run; (b) policy -> `BLACK_AT_EXTREME` etc. with its justification per F1.4; (c) app defect -> F1.7 table row with file:line (no test change). At most two triage rounds; the final run's outcome list + per-bin counts go in the report.
3. Expected on main+source-defects (from diag section 4 and the lane's fixes): non_black GREEN (layer_router excepted, strobe via retry; julia/burning/newton/sierpinski/crystal fixed by the lane); time sweep GREEN; audio x3 GREEN (live PSNRs clear the thresholds); params: the true residual is unknown until H1 lands -- the diag's 16 "unexplained" lines (astral_grid Warp, spectral_ring Glow, fire_wall Wind, strobe_light Fade, ribbed_vortex Speed, scroll_plane Speed Y, torus_hole Phi/Theta Offset/Pinch/Heart, mandelbrot Max Iterations, cymatics Damping, water_caustics Distortion/Scale, structural_landscape Drama, kaleido_fractal Iterations) are the first to look at (several are the periodic class the ladder absorbs).
4. Regression probes (8080 `load_source` consumers): `.harmony/probe-deck-path.sh` (the only probe calling `load_source`/`update_source_params`; VERIFIED grep), plus `probe-render-state.sh` and `probe-canvas.sh` (render_frame consumers) GREEN on the lane app.

### F1.7 If source-defects has NOT merged: the named residual (file:line as of main; the lane's commits move them)
| id | defect | site |
|---|---|---|
| julia_set | black at registered defaults (c inside the cardioid) | `src/sources/SourceRegistry.cpp:419-421` |
| burning_ship | corner of the ship only | `SourceRegistry.cpp:433-435` |
| newton_3d | camera under the heightfield; auto-rotate pitches under it | `SourceRegistry.cpp:586`, `EmbeddedShaders.h:7547-7548` |
| sierpinski | black on power-of-two canvases (dyadic alias) | `EmbeddedShaders.h:6547` |
| crystal_cavern | black after ~2.3 s (bounded cave) | `EmbeddedShaders.h:9074-9134` |
| Dot Field | near-black default; `depth` unread | `EffectLibrary.cpp:515`, `EmbeddedShaders.h:3643` |
| 78 dead controls | registered params no shader reads | `SourceRegistry.cpp:644-662` (+ :487, :595-596, :1019, :1023-1024, :1042, :1046) |

### F1.8 Docs, must-not-change, commits
Docs: `tests/visual/SHADER_VERIFICATION.md:27-31` (metric, `T_PARAM`, ladder, the exceptions module -- note the source-defects lane inserts 8 lines right after :31: rebase onto it, or write below its insertion); `docs/claude/testing-eyes.md:72-78` (load_source seeds the registry defaults; params overlay; the 256 fixture); `.harmony/HANDOFF.md:85-86` item 3 -> status; new Pitfall (number after 45 lands): "Eyes `load_source` must seed the registry defaults -- a cached `ProceduralSource` keeps its last values across `load_source` and `/api/reset`" (Guard: the E1 row; no ctest links TestServer.cpp). Must-not-change: `test_no_discontinuities`, `test_no_destructive_effects`, both `test_performance` cases stay PASSED; `test_performance` still captures at 512 on a 1080p canvas; files outside the five + conftest + vj_controller + tier1_exceptions + TestServer.cpp:749-763 + docs untouched (`git diff --stat` in the report); `test_fractals.py` run once as a smoke (it is not Tier-1; H1 changes its meaning for the better). Commits: (1) TestServer H1 + vj_controller helpers + conftest fixture; (2) the four files + `tier1_exceptions.py` skeleton; (3) exceptions entries from the re-run + Pitfall 28 fix + docs + report evidence.

---------------------------------------------------------------------------------------------------

## F2 -- capture / snapshot file overwrite

### F2.1 Mechanism and every writer (VERIFIED)
`juce::FileOutputStream`'s constructor calls `openHandle()` (`build/_deps/juce-src/modules/juce_core/files/juce_FileOutputStream.cpp:39-44`), which for an EXISTING file opens `O_RDWR` and `lseek(f, 0, SEEK_END)` (`juce_core/native/juce_SharedCode_posix.h:488-496`, JUCE 8.0.4): a write to an existing path appends. Grep of src/ for every JUCE file-writing API (`FileOutputStream`, `PNGImageFormat`, `writeImageToStream`, `replaceWithData/Text`, `createOutputStream`, `TemporaryFile`, `JPEGImageFormat`):
| site | opens an existing path? | verdict |
|---|---|---|
| `src/render/Renderer.cpp:2106-2115` `captureFrame` PNG write -- callers 8080 `render_frame` (`TestServer.cpp:591`), 7070 `render_frame` (`ApiServer.cpp:1176`), `takeSnapshot` (`Renderer.cpp:2196-2199`; triggered from `MainComponent.cpp:6563-6566` menu, `:7299-7302` binding action, `:1853`, `:2158`, REST `/api/snapshot` `ApiServer.cpp:717-724`) | YES -- no delete, so a repeated path = OLD PNG first, decoders return the old picture; `takeSnapshot`'s `%Y%m%d_%H%M%S` name (`:2193-2195`) collides within one second | FIX |
| `src/test/TestServer.cpp:1728-1735` `output_probe` PNG | `outFile.deleteFile()` first (`:1729`) | correct today; switch to the helper (one writer) |
| `src/recording/AudioTap.cpp:115` WAV | path `<root>/<uuid>.asset/audio.wav` (`AudioStore.cpp:94-102`), id minted fresh with an exists check (`:109-121`) -- never pre-exists | not affected, no change |
| every other writer (`Composition::saveToFile` `Composition.h:669-672`, `BindingManager.cpp:297`, `PresetManager.cpp:156/528/581`, `Take.cpp:293`, `AudioStore.cpp:251`, `MainComponent.cpp:2224/3228/6670`, `ProjectMPresetManager.cpp:108`, `FilesBrowser.cpp:634`) | `File::replaceWithText` (deletes/creates, `juce_File.cpp:777-781`) | not affected |
No other image writer exists in src/ (no JPEG, no thumbnail-to-disk; the "thumb" hits are UI drawing).

### F2.2 Fix
NEW `src/render/PngWrite.h` (header-only, like `PixelConvert.h`):
```cpp
#pragma once
#include <juce_graphics/juce_graphics.h>
namespace PngWrite {
// Writes img as a PNG to file, REPLACING any existing file. A juce::FileOutputStream opens an existing file at its
// END (juce_SharedCode_posix.h openHandle: lseek SEEK_END), so a plain stream write APPENDS a second PNG after the
// old one and every decoder returns the OLD picture (s-rta-0927 renderperf found_not_fixed #4; tests/test_png_write.cpp).
inline bool writeReplacing(const juce::Image& img, const juce::File& file)
{
    file.getParentDirectory().createDirectory();
    if (file.existsAsFile() && !file.deleteFile())
        return false;
    juce::FileOutputStream fos(file);
    return fos.openedOk() && juce::PNGImageFormat().writeImageToStream(img, fos);
}
}
```
- `Renderer.cpp:2106-2115`: replace the `createDirectory` + stream block with `ok = PngWrite::writeReplacing(img, outputPath);` inside the existing `tPng` timing; keep the `[Eyes] Captured frame:` / `Failed to write PNG` lines verbatim (probes parse the prefix). Include `"render/PngWrite.h"` next to the PixelConvert include.
- `TestServer.cpp:1728-1735`: replace `createDirectory(); deleteFile(); { FileOutputStream ... writeImageToStream }` with `bool ok = PngWrite::writeReplacing(img, outFile);`.
- `Renderer.cpp:2195`: `auto outputFile = dir.getChildFile(filename).getNonexistentSibling(false);` -- for an existing `snapshot_20260927_201005.png` this yields `snapshot_20260927_201005_2.png` (`juce_File.cpp:673-681` -> `:621-670`: a prefix ending in a digit gets `_` + number). Two snapshots in one second are then two files; the write itself still replaces (belt: two snapshot threads that pick the same sibling within the same ~100 ms replace instead of corrupt).

### F2.3 RED ctest -- `tests/test_png_write.cpp` (register exactly like `test_pixel_convert`, `tests/CMakeLists.txt:2331-2353`: Catch2 + `juce::juce_gui_basics`, `JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0`, `apply_sanitizers`, `catch_discover_tests`)
Temp files under `juce::File::getSpecialLocation(tempDirectory).getChildFile("audiodna-png-write-" + Uuid)`, deleted at the end of each case.
- (a) RED first, `[pngwrite][replace]`: `writeReplacing(red 64x64)` to P, then `writeReplacing(blue 16x16)` to the SAME P; assert `ImageFileFormat::loadFrom(P)` is 16x16 with pixel (0,0) blue, AND `P.loadFileAsData()` equals the bytes of `writeReplacing(blue 16x16)` to a FRESH path Q (byte-identical: nothing appended, nothing left over). RED-first shape: commit 1 lands the test and `PngWrite.h` whose body is the OLD sequence verbatim (`createDirectory` + `FileOutputStream` + `writeImageToStream`, no delete) -- the case fails with the decoded size 64x64 (libpng stops at the first IEND; INFERRED -- any other outcome is RED as well because the sizes differ); commit 2 swaps in the delete and the two call sites -- GREEN with the test text unchanged.
- (b) teeth, `[pngwrite][juce-appends]`: the RAW sequence (stream + `writeImageToStream`) twice to one path -> `File::getSize()` == size(first) + size(second). Documents WHY the helper exists; if a JUCE bump stops appending, this case fails and says the helper is redundant.
- (c) pin, `[pngwrite][fresh]`: a fresh path in a not-yet-existing directory -> written, decodes at its size.
No ctest can reach `takeSnapshot` (Renderer.cpp is not linked into any ctest target, `tests/CMakeLists.txt:43-44`) -- live row below.

### F2.4 Gates, must-not-change, commits
- Build; ctest all GREEN, count = base + 3.
- `grep -rn "FileOutputStream" src/` -> exactly `AudioTap.cpp:115` and `PngWrite.h`; `grep -rn "PNGImageFormat" src/` -> `PngWrite.h` only.
- Live (lane app, lock, `--test-mode`): (1) 7070: `POST /api/snapshot` twice within one second -> two files `snapshot_X.png` and `snapshot_X_2.png` (or a different second: two distinct stamps), each decodes (PIL) at the composition size; (2) 8080: `render_frame` twice to ONE path, the second after `set_composition_params 256x256` from 1280x720 -> decodes 256x256 (main: 1280x720); (3) capture byte-identity: same fixture as `renderperf-evidence/scripts/capt.py`, fresh paths, sha256 main-vs-lane equal at 1080p and 720p (the writer changed, the bytes must not).
- Probes: `probe-render-state.sh`, `probe-canvas.sh`, `probe-effects-parity.sh` GREEN (they decode render_frame / output_probe PNGs; they delete first, so they must see no change).
- Must-not-change: `[Eyes] Captured frame:` line format; `captureFrame`'s C3 shape (read on GL thread, convert+png on the caller); no behaviour change for a fresh path.
- Commits: (1) `test_png_write.cpp` + `PngWrite.h` (old body) + CMake -> RED (a) verbatim in the report; (2) the fix + two call sites + `getNonexistentSibling` + `.harmony/HANDOFF.md`/notebook line ("renderperf found_not_fixed #4 FIXED") + a Pitfall entry (number after 45: "`juce::FileOutputStream` opens an existing file at its END -- write PNGs through `PngWrite::writeReplacing`, never a bare stream").
- Out of scope, named: found_not_fixed #5 first half (a second `captureFrame` while one waits overwrites `capturePromise_`; the first times out safely) -- unchanged.

---------------------------------------------------------------------------------------------------

## F3 -- beat-crossing readers onto the `totalBeatCount` delta (Pitfall 42 follow-up)

### F3.1 Sites (VERIFIED) and the writer's rule
Readers still detecting a beat as `beatPhase < last - 0.5f`:
| reader | detector | baseline | cadence / snapshot |
|---|---|---|---|
| `src/model/Autopilot.cpp:44-45` beat mode | `beatCrossed` | `lastBeatPhase_` `Autopilot.h:55` (`lastOnBeat_` `:56` is unused) | GL thread, once per deck instance (`AutopilotBank`) |
| `src/render/Renderer.cpp:407-409` projectM playlist | `beatCrossing`, INSIDE the per-layer loop (`:398-466`) | `lastPlaylistBeatPhase_` `Renderer.h:554` | GL thread, `snap` = `frameSnap_` (`:243`), inside `if (deckActive)` (`:395`) |
| `src/MainComponent.cpp:3870-3892` `advanceSlideshow` | `:3880` | `lastSlideshowBeatPhase_` `MainComponent.h:284`, reset `:3858` | 30 Hz timer `:3717-3719`, `featureBus.read()` |
| `src/MainComponent.cpp:4034-4057` `beatSyncRandomize` | `:4041` | `lastBeatPhase_` `MainComponent.h:331`, resets `:240` (Sync button), `:5121` (resync action) | 30 Hz timer `:3714-3715` |
Writer (VERIFIED `BPMTracker.cpp:221-251`, `FeatureSnapshot.h:128-137`): `totalBeatCount` += 1 per phase wrap, += 1 for a hard realign from the second half of a beat, += 0 from the first half; never reset; published every hop. So a count delta of 1 corresponds exactly to what the old reader called a crossing (wrap: yes; realign 0.7->0: yes, `0 < 0.7-0.5`; realign 0.3->0: no, `0 < -0.2` false) -- no behaviour change without a stall. Under a stall of g beats the old reader sees at most one crossing and none at all when the gap's start phase minus 0.5 is below the end phase (Pitfall 42); the delta is exact.

### F3.2 Fix -- one `OnsetPulse` per reader (`src/features/OnsetPulse.h`: `consume(count)` = count since my previous look, first call primes and returns 0, a backwards jump re-baselines and returns 0 -- `tests/test_onset_pulse.cpp:10-58` pins all of it)
- `Autopilot.h:55-56`: replace `float lastBeatPhase_ = 0.0f; bool lastOnBeat_ = false;` with `OnsetPulse beatCrossings_;   // whole beats since this instance's previous frame -- FeatureSnapshot::totalBeatCount delta (Pitfall 42), one baseline per instance (Pitfall 38)` (+ `#include "features/OnsetPulse.h"`). `Autopilot.cpp:43-48`:
```cpp
const uint32_t beats = beatCrossings_.consume(snapshot.totalBeatCount);   // 0 or 1 at frame rate; >1 only across a stall
if (beats == 0) return anyAdvanced;
```
  then, in the per-layer loop, `clip->beatsPlayed += static_cast<int>(beats);` (`:75`; was `++`). Pending triggers (`:51-59`) fire once per frame as today. Everything else unchanged.
- `Renderer.h:554`: `OnsetPulse playlistBeatCrossings_;` (replaces `lastPlaylistBeatPhase_`). `Renderer.cpp`: consume ONCE per frame immediately before `if (deckActive)` at `:395` -- `const uint32_t playlistBeats = playlistBeatCrossings_.consume(snap.totalBeatCount);` -- and inside the loop replace `:407-411` with `if (playlistBeats > 0) { clip->presetBeatsPlayed += static_cast<int>(playlistBeats); ...`. INTENDED behaviour change to name in the report: today the baseline is updated per LAYER inside the loop, so with two or more projectM playlist clips only the first layer ever sees a crossing (Pitfall 38's class); the once-per-frame delta fixes that. Consuming outside the fence means beats elapsed while no deck is active are dropped, as the old reader dropped them (it could also fire one spurious crossing from a stale baseline on return; the delta never does).
- `MainComponent.h:284`: `OnsetPulse slideshowBeatCrossings_;` (replaces `lastSlideshowBeatPhase_`); `:331`: `OnsetPulse beatCrossings_;` (replaces `lastBeatPhase_`). `MainComponent.cpp:3876-3892`: `const uint32_t beats = slideshowBeatCrossings_.consume(snap.totalBeatCount);` `if (beats > 0) { slideshowBeatCounter_ += static_cast<int>(beats); if (slideshowBeatCounter_ >= slideshowBeats_) { ... } }`. `:4038-4045`: `const uint32_t beats = beatCrossings_.consume(snap.totalBeatCount); if (beats == 0) return; beatCounter_ += static_cast<int>(beats);` then the existing `>= beatRandomCount_` block. Resets: `:3858` -> `slideshowBeatCrossings_.reset();`, `:240` and `:5121` -> `beatCrossings_.reset();` (with `beatCounter_ = 0` as today) -- `reset()` makes the next read a baseline with no crossing, which is exactly what `lastBeatPhase_ = 0.0f` did.

### F3.3 Tests -- `tests/test_autopilot.cpp` (links `Autopilot.cpp`, `tests/CMakeLists.txt:656-668`; add `target_compile_definitions(test_autopilot PRIVATE AUDIODNA_SRC_DIR="${SRC_DIR}")` for case 8, precedent `:2318`)
Synthetic stream: 120 BPM, GL at 60 fps -> `b(t) = 2t`, 1/30 beat per tick; `snap(b)`: `beatPhase = b - floor(b)`, `totalBeatCount = floor(b)`, `bpm = 120` ("publish what BPMTracker publishes" -- the beatclock lane's rule, commit 062b750). Deck as in `test_deck_clock.cpp:200-212` (layer 0 autopilot, 3 columns Beat4/PlayNext, `triggerClip(0)`). Run ticks b = 0, 1/30, ... up to b0 (beatsPlayed = 3 at b0 = 3.4 for both readers), then ONE tick at b0 + g (the stall), then assert.
1. RED `[autopilot][stall]` "a 1.1-beat GL stall loses no beat": b0 = 3.4 -> 4.5. Old: phase 0.4 -> 0.5, `0.5 < -0.1` false, beatsPlayed stays 3, column 0. New: count 3 -> 4, beatsPlayed 4 >= 4, column 1. CHECK column == 1 right after the stall tick (RED today: 0).
2. RED "a 0.7-beat stall that begins at phase 0.5 and contains the wrap": 3.5 -> 4.2 (`0.2 < 0.0` false today). Expect column 1.
3. RED "a 2.3-beat stall adds two beats": 3.4 -> 5.7 (`0.7 < -0.1` false today: zero crossings). Expect column 1 after the tick (beatsPlayed 5 >= 4, one advance).
4. PIN (GREEN before and after) "without a stall the delta reader advances on exactly the ticks the wrap reader did": 16 beats at 60 fps; compute the expected advance ticks in the test with the OLD formula (`phase < last - 0.5f`) on the same stream and compare with the ticks where `activeClipColumn` changes. This is the "no behaviour change without a stall" proof.
5. PIN "realign semantics match the old reader": a Tap from the second half (count +1, phase -> 0) counts one beat; from the first half (count +0, phase -> 0) counts nothing -- both readers agree (BPMTracker.cpp:244-251's rule).
6. PIN "a writer reset (test-mode /api/reset: count back to 0) is a baseline, not a crossing".
7. Existing pins, edited so they publish the count: `advanceOneBeat` (`test_autopilot.cpp:21-27`: `snap.totalBeatCount++` on the 0.01 step; fix the comment at `:18-19`), `test_deck_clock.cpp:221-223, 236-238, 252-253` and `test_compositor.cpp:91-94, 104-107, 124-127` (same one-line bump in each loop). They stay GREEN in both commits; without the bump they would go RED after the fix -- that is the expected teeth, run it once and quote it.
8. LAW `[autopilot][law]`: read `src/model/Autopilot.cpp`, `src/render/Renderer.cpp`, `src/MainComponent.cpp` from `AUDIODNA_SRC_DIR`; assert no line matches `beatPhase\s*<\s*\w+\s*-\s*0\.5` (RED today: 4 hits; the guard against the pattern coming back). `RoutineEngine.cpp:243-247` uses `lastBeatPhase_` for a time-to-boundary prediction, not a wrap test -- not matched, not scanned.

### F3.4 Probe and injection rule (REQUIRED with the fix commit)
`.harmony/probe-deck-clock.py:140-143 crossings()` and the `inject(beatPhase=0.5)` primes at `:272` and `:371` inject a phase sawtooth with NO `totalBeatCount`; on the fixed app the count stays 0 and `d_autopilot_keeps_time` / `d_pending_trigger_still_cancelled` go RED. Update: a module-level `BEAT = 0`; `crossings(n)` does `inject(beatPhase=0.99, totalBeatCount=BEAT)`, then `BEAT += 1; inject(beatPhase=0.01, totalBeatCount=BEAT)`; the primes pass `totalBeatCount=BEAT`. Docstring lines 26/33: add "(totalBeatCount moves with the phase -- Pitfall 42)". 7070's inject keeps the previous count when the key is absent (`ApiServer.cpp:851, :883-887`), 8080's zeroes it (`TestServer.cpp:489`) -- the probe uses 7070. Teeth run: OLD probe against the NEW app -> autopilot column stays 0 (quote it); NEW probe -> GREEN. Inventory of every other beatPhase injector (VERIFIED grep): `probe-downbeat-level/manual-bpm/routines/mastersignal/resync/tempo-silence.sh`, `tests/visual/test_audio_reactivity.py`, `test_signals.py`, `test_render_pipeline.py` -- none exercises autopilot, slideshow, randomize or a projectM playlist (grep for those words hits only probe-deck-clock), so none needs a change.

### F3.5 Docs, gates, must-not-change, commits
- Docs: Pitfall 42 (`pitfalls.md:93`) sentence "Still reading the wrap (...): Autopilot.cpp beat mode, Renderer.cpp projectM playlist, MainComponent::advanceSlideshow / beatSyncRandomize (30 Hz)" -> "Every beat-CROSSING consumer now takes the totalBeatCount delta through OnsetPulse (Autopilot, projectM playlist, slideshow, beat randomize; s-rta-0927 follow-ups): counters integrate the delta, edges fire once per tick"; Pitfall 38 (`:85`) mechanism "detects a beat as `beatPhase < lastBeatPhase_ - 0.5` with ONE `lastBeatPhase_`" -> "ONE OnsetPulse baseline"; `AutopilotBank.h:10-12` comment; `test_autopilot.cpp:18-19` comment. CLAUDE.md index lines 38/42 stay true as written.
- Gates: build; ctest all GREEN (base + 8 cases; commit 1 shows exactly cases 1-3 and 8 RED with the numbers above, 4-7 GREEN); `git grep -n "beatPhase <" src/` -> 0 hits; probes `probe-deck-clock.sh` (all rows, with the F3.4 probe), `probe-beatclock.sh` (unchanged, GREEN), `probe-routines.sh` (105/0 sanity: RoutineEngine untouched), `probe-render-state.sh`.
- Must-not-change: `RecorderClock.*`, `RoutineEngine.*`, `BPMTracker.*`, `AnalysisThread.cpp`, `FeatureSnapshot.h` untouched (`git diff --stat`); `Layer::processPendingTrigger` semantics; case 4's tick-identical advancement; `snap.beatPhase` is still read by `MainComponent.cpp:4223` (beat position for arming) -- not a crossing reader, untouched.
- Commits: (1) tests -- cases 1-8 + the count bump in the three existing files -> RED for 1-3 and 8; (2) the four readers + headers + probe + docs -> GREEN; report evidence (RED/GREEN verbatim, probe teeth, probe GREEN).

---------------------------------------------------------------------------------------------------

## RISKS (per item; the strongest counter-argument first)

F1
- Strongest counter: "fix the tests, not the server -- `load_source` without params meaning 'keep the cached values' could be a feature someone relies on". Checked: no test or probe passes partial params or depends on persistence (grep of tests/visual and .harmony: `probe-deck-path.sh` is the only probe caller; it must be run). The UI never has this semantic. Loses.
- The exceptions list can become a dumping ground. Mitigated by the two-condition admission rule and the ~40 bound; the report must show the per-bin counts.
- Pitfall 28 may be right in some sub-case (effects on the legacy chain but not over a loaded IMAGE?) -- the Invert refutation settles it before any effects triage; if the chain does not render, stop and report.
- The `FEATURES_ACTIVE` injection is new state for every render: a source whose look depends strongly on audio (chromatic_ring, band_tower) gets a different but stable default frame. Both renders of a param share it, so the comparison is fair; note it in SHADER_VERIFICATION.
- Conflict with source-defects in `SHADER_VERIFICATION.md` (adjacent lines) and `test_fractals.py` (F1 does not edit it). Rebase F1 on the merged main.
- Duration: the params test renders up to 3 candidates for failing params only; the diag's run shape (516 lines) was the worst case, so the run gets shorter, not longer, as failures drop.

F2
- Strongest counter: "delete-first is racy; truncate keeps the inode". A concurrent writer to one path is a caller bug either way (probes and tests use distinct paths; snapshots get sibling names). Delete-first matches the one existing precedent. Loses.
- libpng behaviour on a two-PNG file is INFERRED (first image returned). Case (a) is RED under any behaviour except "decodes the second at its size AND bytes equal a fresh write", so the RED does not depend on the inference.
- `getNonexistentSibling` on a slow disk adds an `exists()` stat per snapshot -- negligible next to the 80 ms PNG encode.
- The 7070 `render_frame` (`ApiServer.cpp:1164-1182`) has no width/height (D2 in renderperf) -- unaffected by F2, just noting the live row (2) uses the composition size.

F3
- Strongest counter: "integrating the delta changes behaviour: after a stall the autopilot jumps ahead instead of losing a beat". That IS the requested fix (Boris's ruling for the beat clock: catch up after a freeze is the default, plan-beatclock 2e / work log 17:59), and case 4 proves nothing changes without a stall. Loses.
- The once-per-frame projectM consume fixes the multi-layer playlist baseline (intended, named). If Harmony wants strict parity instead, keep the consume where the old detector was -- but that preserves a Pitfall-38-class bug; not recommended.
- Test-mode injectors that move `beatPhase` without `totalBeatCount` now drive none of the four readers. Inventory in F3.4 says only probe-deck-clock does, and it is updated in the same commit. A future probe author has Pitfall 42's rule and the 8080/7070 difference stated there.
- `Renderer.cpp:1460/:1532` read the bus directly for other purposes (genre/structural) -- not crossing readers, untouched; the LAW case would catch a new wrap detector only in the three scanned files.
- MainComponent.cpp and Renderer.cpp are not ctest-linked: the three swaps there are covered by the LAW case (pattern gone), the build, and the live probes; the slideshow and randomize paths have no probe -- the report should include one manual check (Beats toggle on, manual 120 BPM via `/api/set_bpm`, randomize fires every 4 beats; folder slideshow advances every 8 beats) or accept the helper's ctest coverage plus identical call shape as sufficient. Named as ASSUMED-sufficient here; Harmony decides.

## What is NOT in this plan (named)
- renderperf found_not_fixed #1 (image first-upload hitch), #2 (4K warm leftover), #3 (PNG encode dominates the round trip), #5 first half (capturePromise_ overwrite -> safe timeout), #6 (C4 `frame_ring_cells`).
- App-side state-preserving canvas resize (H2's app alternative).
- Any change to RecorderClock/RoutineEngine (already count-based).

REPORT_FILE: .harmony/.reports/s-rta-0927/plan-followups.md
STATUS: DONE
