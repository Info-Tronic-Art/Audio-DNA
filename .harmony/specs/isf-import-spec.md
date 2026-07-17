# ISF Import — Implementation Plan (Audio-DNA)

<!-- Authored by Plan agent (plan-isf), 2026-07-17. Persisted verbatim by Harmony.
     Status: SPEC — Wave 2 build item (Boris-ratified: implement real ISF support).
     Line numbers verified against working tree 2026-07-17 pre-Wave-0-merge; cite-by-symbol
     fallbacks included where drift expected. -->

**TLDR:** Real ISF import is ~6–9 builder-days across 8 steps. The current path parses and converts the shader but literally discards the converted GLSL (local `glsl` at MainComponent.cpp:2426 is never used), registers a def the compositor then silently skips at the null-program check (CompositorEngine.cpp:312-314), and shows a phantom success dialog (MainComponent.cpp:2450-2455). Two additional independent bugs mean even a compiled shader couldn't work: (a) the converter declares `in vec2 v_uv` (ISFShaderLoader.cpp:129) but the shared vertex shader outputs `v_texCoord` (EmbeddedShaders.h `vertex`) — a statically-used unmatched input, guaranteed link failure; (b) dropped INPUTS (point2D/color/image/event, ISFShaderLoader.cpp:100-104) leave undeclared identifiers in the GLSL body, and float min/max are normalized away with no denormalization anywhere — the shader body receives [0,1] where it expects native ranges (upload path CompositorEngine.cpp:372-377 sends raw slot values).

**Line-drift caveat:** wave0-builder edited concurrently. ISFShaderLoader.cpp cites shift after the `registerISFEffect` stub (ISFShaderLoader.cpp:210-243) deletion — symbols cited alongside.

## Verified architecture facts the plan builds on

- All 275 shader compiles happen on the GL thread at context creation: `newOpenGLContextCreated` (Renderer.cpp:81-88) → `initShaders` (Renderer.cpp:990) → `compileAllShaders` (Renderer.cpp:995+). `ShaderManager::compileProgram` (ShaderManager.cpp:13-41) needs the active context; errors are DBG-only (stripped in release).
- The render loop already has the queue pattern to copy: `pendingImageMutex_` + flags drained at the top of `renderOpenGL` (Renderer.cpp:106-135), with `MessageManager::callAsync` for replies (Renderer.cpp:220).
- **Second GL context:** OutputWindow owns its own ShaderManager and compiles its own copy of every shader (OutputWindow.h:49, OutputWindow.cpp:160-176), passed to `effectChain_.render` (OutputWindow.cpp:143-147). Dynamic shaders must compile in BOTH managers or the projector output silently drops the effect while preview looks fine — worst live-failure mode.
- Param UI + audio modulation are FREE once an EffectDef exists: EffectStackView auto-builds a UniversalParamControl per def param (EffectStackView.cpp:321-327) and drives params from Signal/Oscillator/Envelope/Macro sources generically by index (EffectStackView.cpp:157-215). `fx:` drops create slots with defaults (EffectStackView.cpp:409-447; ClipCell.cpp:400).
- All three stack scopes share one EffectSlot path: per-clip (CompositorEngine.cpp:252+), per-layer (wrapped at CompositorEngine.cpp:748-752), global (`Composition::globalEffects`, Composition.h:21, wired at CompositionInspector.cpp:380). One compile serves all scopes.
- MappingEngine targets the LEGACY global EffectChain by index (MappingTypes.h:127-128), built once from a category list excluding "isf" (`Renderer::initEffectChain`). Cut from v1 (below).
- Persistence: EffectSlot serializes by effectName + ordered paramValues (Clip.h:48-56, Clip.cpp:55-67 and 148-162); unresolvable names silently skip (CompositorEngine.cpp:258-260). Restart survival = re-register same name + same param order before use.
- `registerDynamic` → plain `push_back`, no dedupe (EffectLibrary.h:53, EffectLibrary.cpp:3-6). **Pre-existing data race:** message thread mutates `defs_` (MainComponent.cpp:2443) while the GL thread reads defs every frame (CompositorEngine.cpp:245, 258).
- FXBrowser hard-codes categories without "isf" (FXBrowser.cpp:400-411); unmatched category defaults to index 0 (FXBrowser.h:45) so ISF imports appear misfiled under "Warp"; no refresh fires after import.
- REST probes available: /api/render_frame → `captureFrame` with time override (ApiServer.cpp:769-786), /api/load_image, /api/inject_features, /api/effects (global chain only). Python harness spawns the app (tests/visual/conftest.py).

## Steps

**Step 0 (S) — Converter correctness fixes** (`ISFShaderLoader::convertToGLSL`)
Emit `in vec2 v_texCoord` + `#define isf_FragNormCoord v_texCoord` (fixes link failure); add `#define texture2D texture`, `#define IMG_SIZE(s) vec2(textureSize(s,0))`, `DATE` stub `vec4(0)`. Pure string work, unit-testable without GL.

**Step 1 (M) — GL-thread compile queue + error capture**
- `Renderer::queueDynamicShaderCompile(name, fragSource, onDone(bool ok, String log))`: mutex-protected vector drained at top of `renderOpenGL` (clone the pendingImage pattern, Renderer.cpp:106-135), compile via `shaderMgr_.compileProgram`, marshal result via `MessageManager::callAsync`. Cap drains at ~4/frame to bound stalls.
- Extend `ShaderManager::compileProgram` with an error-string out-param (currently DBG-only, ShaderManager.cpp:21/27/33 — invisible in release; this is the root of the phantom-success UX).
- Fix the EffectLibrary race: guard `defs_` with `std::shared_mutex` (shared reads in `getEffectDef`/`getEffectNames`, exclusive writes). Uncontended shared lock per slot per frame is negligible.
- OutputRenderer: compile all dynamic shaders at its `newOpenGLContextCreated` (extend the hard-coded list at OutputWindow.cpp:160-176 to iterate the store from Step 4) + drain its own pending list for imports that happen while the output window is open (track a store version counter).

**Step 2 (M) — Full scalar INPUT coverage + denormalization**
Keep EffectSlot/UI/serialization fully [0,1]-normalized (invariant of the whole system); bake denormalization into generated GLSL instead of touching ParamDef (which has no min/max — EffectLibrary.h:16-21):
- float: `#define speed (u_isf_speed * (MAX-MIN) + MIN)`
- bool: `#define flag (u_isf_flag > 0.5)`; event: same as bool (momentary param)
- long + VALUES: quantize `int(u * float(N-1) + 0.5)` indexed through a generated const int array (current parsing is a stub — ISFShaderLoader.cpp:92-99)
- point2D → two params `name_x`/`name_y` + `#define name vec2(...)`; color → four params `name_r/g/b/a` + `#define name vec4(...)`. Splitting to scalars keeps the glUniform1f upload path (CompositorEngine.cpp:372-377), UI, mapping, and serialization untouched.
- image inputs: exactly one allowed, aliased to `u_texture`; reject ≥2 at import with a clear message. Generators (zero image inputs) import fine as effects that ignore input.
This step is required for correctness, not polish: today any shader using a skipped input type fails to compile with undeclared identifiers.

**Step 3 (S) — Standard uniforms + audio-reactive extension**
Already handled: TIME, RENDERSIZE, PASSINDEX=0, isf_FragNormCoord, IMG_NORM_PIXEL/IMG_PIXEL/IMG_THIS_PIXEL, inputImage (ISFShaderLoader.cpp:131-142). Add: real `u_timeDelta` (+`#define TIMEDELTA`) and a true frame counter for FRAMEINDEX, uploaded next to u_time in the compositor effect loop (CompositorEngine.cpp:361-366). **Audio extension (the differentiator, near-zero cost):** unconditionally declare the audio uniform block (u_rms, u_bass, u_mid, u_high, u_beatPhase, u_barPhase, u_onsetStrength, u_spectralFlux, u_bpm, u_bandEnergies[7], u_chromagram[12]) in the generated header — `uploadAudioUniforms` (CompositorEngine.cpp:1398+) already feeds any program declaring these; unused ones optimize out and upload skips on -1 locations. Every imported ISF shader becomes audio-reactive by just referencing u_bass.

**Step 4 (M) — ISFEffectStore: import dir, boot re-registration, dedupe**
- New `src/effects/ISFEffectStore` (message-thread-owned): `importFile()` copies the original .fs to `~/Documents/Audio-DNA/ISF Shaders/Imported/` (`getISFDirectory` exists — ISFShaderLoader.cpp:245-250, symbol `getISFDirectory`); `scanAndRegister()` at boot parses + converts every file, registers defs, queues compiles.
- Re-convert from the original source each boot (microseconds of string work) — no stale converted cache, converter fixes apply retroactively.
- **Invariant:** param order derives from the INPUTS array order in the file → stable across boots → serialized `paramValues` stay index-aligned (Clip.cpp:148-162). Unit-test this.
- Add replace-by-name dedupe to `registerDynamic` (re-import updates instead of duplicating; EffectLibrary.cpp:3-6).
- Rewrite `handleImportISF` (MainComponent.cpp:2400-2457) to route through the store; delete the dead conversion inline there.

**Step 5 (S) — Failure UX**
Success dialog fires ONLY from the compile callback after link OK. On failure: dialog with the GLSL error log + path to the converted source dumped at `Imported/.debug/<name>.frag`, then unregister the def and remove the copied file. Reject at import (before compile) with specific messages: multi-pass (PASSES > 1 or PERSISTENT), ≥2 image inputs, audio/audioFFT inputs. Boot-time failures: stderr log + skip, no dialog spam.

**Step 6 (S) — Browser polish**
Add "isf" category with its own color (FXBrowser.cpp:400-411); trigger `fxBrowser_.refresh()` (exists — BrowserPanel.cpp:109) after successful import; fix the dialog text promise ("Find it under the ISF category" — currently lands under Warp).

**Step 7 (M) — Tests**
- Unit `tests/test_isf_loader.cpp` (register alongside existing test_*.cpp in tests/CMakeLists.txt; no GL needed): JSON block extraction variants; INPUTS parsing incl. min/max, long VALUES, point2D/color splitting; converted-source assertions (contains v_texCoord not v_uv, denorm defines correct, audio block present, texture2D alias); rejection cases; param-order stability; EffectLibrary dedupe.
- Behavioral (python harness, tests/visual/conftest.py): add two small endpoints — POST `/api/import_isf {path}` (drives the store path headlessly) and POST `/api/add_clip_effect {layer, col, name}` (nothing today can attach per-clip effects via REST; /api/set_effect targets only the legacy global chain). Probe: load_image fixture → import a trivial known-output ISF (e.g. invert) → add to clip → trigger_clip → `/api/render_frame {time: fixed}` → assert captured PNG differs from no-effect baseline and matches golden within tolerance. Failure probe: import broken .fs → error response, absent from library. Restart probe: relaunch app fixture → effect still listed, render unchanged (proves persistence end-to-end).

## v1 cut lines (with rationale)

1. **Multi-pass / PERSISTENT buffers** — needs per-effect FBO chains with lifetime/resize management inside the compositor; the single largest chunk (L) and most popular filters are single-pass. Reject cleanly at import, don't mis-render.
2. **audio / audioFFT texture inputs** — needs waveform/FFT texture synthesis from FeatureBus (M). The Step-3 audio uniform block covers most reactive use cases better for this app. Reject only when actually used.
3. **Multiple image inputs** — needs a multi-texture binding protocol in the compositor slot loop (M).
4. **Global EffectChain / MappingEngine integration** — targets are stored as chain indices (MappingTypes.h:127-128); dynamic membership makes preset/mapping indices unstable across differing import sets, plus chain mutation mid-session races the GL iteration. Per-clip/layer/global EffectSlot stacks (all the same code path) + UniversalParamControl signal sources already deliver audio mapping.
5. **ISF generators as Sources** (SourceRegistry entries) — natural v2; v1 imports everything as an effect (generators ignore u_texture and work).
6. **Hot-reload/edit of imported shaders** — defer.

## Risk register

1. **GL-thread compile stalls** — a big ISF shader can take tens of ms to compile mid-show. Bounded by the per-frame drain cap; imports are user-initiated; precedent: 275 synchronous compiles already run at context creation.
2. **Dual-context divergence** (preview OK, projector output silently missing the effect) — highest silent-failure risk; mitigated by store-version sync in Step 1 and an explicit restart+output-window manual check in the gate.
3. **GLSL 120→410 coverage** — some real shaders will still fail (varyings, gl_TexCoord[], implicit int→float). Acceptance metric: sample 30-50 shaders from the isf.video corpus, target ≥60% compiling in v1; the rest must hit the error UX, never phantom success.
4. **EffectLibrary race** — pre-existing (MainComponent.cpp:2443 writes vs CompositorEngine.cpp:245/258 reads); the shared_mutex in Step 1 fixes it; watch for perf regression in the per-frame getEffectDef path (expected negligible).
5. **Param-count explosion** — color→4, point2D→2 scalars; a 12-input ISF becomes ~20 UI rows (EffectStackView::getPreferredHeight grows linearly). Cap at 32 params with an import warning. Mapping-target space unaffected (cut #4).
6. **Serialization drift** — if a re-imported file changes its INPUTS order, saved paramValues silently misalign (index-based, Clip.cpp:148-162). Accepted v1 risk; documented; dedupe-replace keeps name stable.
7. **Hostile/pathological shaders** — infinite-loop GLSL can trigger GPU watchdog resets; no host-side defense possible. Accepted (user-initiated import of files they chose).

**Dependency order:** 0 → 1 → 2 → 3 (parallel with 2) → 4 → 5 → 6 → 7. Steps 0+2+3 are pure `ISFShaderLoader` string work testable before any GL wiring lands. Total ≈ 6–9 builder-days (S≈0.5d, M≈1.5-2d).
