# PLAN lane bf10 -- MilkDrop draws into its own composition-canvas-sized framebuffer (BF10)

Architect, s-rta-1002b, 2026-10-02. Base: main fa9604d. The code this plan touches is identical at 5e47d17 and
fa9604d (`git diff --stat 5e47d17 fa9604d` shows no change under src/sources, cmake/, CMakeLists.txt or
tests/CMakeLists.txt), so "RED on 5e47d17" and "RED on fa9604d" are the same claim. Docs base: fa9604d
(CLAUDE.md = 24,002 bytes, measured).

VERDICT IN ONE LINE: rebuild the SAME libprojectM 4.1.1 commit with a 3-part patch that adds the upstream-shaped
`projectm_opengl_render_frame_fbo(handle, fbo)`, install it into a new versioned prefix, make configure refuse any
libprojectM without that function, and have `ProjectMSource::render` call it with its own canvas-sized
`outputFBO_`. Then the window framebuffer is never drawn into and the framebuffer-0 blit is deleted. Static
disassembly of the installed library shows ONE constant decides where projectM draws (E8), so the patch is tiny
and changes nothing else.

---

## (1) GOAL

MilkDrop draws its final picture straight into its own canvas-sized framebuffer, never the window's. It fills
100 % of the picture at every composition size and after every size change, and it leaves the Preview panel's
letter-box bars plain.

Boris's words it serves (verbatim, `.harmony/boris-feedback-backlog.md:69-70`): "milkdrop output has not been
adjusted since we changed output to be composition size. Please add to fix list".

The relayed request ("when I switch between decks, do not change the clips playing in the layers ...") is BF9
(lane bf9b). This lane neither serves nor conflicts with it. A MilkDrop clip that keeps playing across deck
switches under bf9b renders through the same `ProjectMSource::render`, so this fix holds for it unchanged.

---

## (2) ESTABLISHED

V = VERIFIED (I read or ran it). I = INFERRED.

- E1 (V) The bug site. `ProjectMSource::render` binds framebuffer 0, sets `glViewport(0,0,canvasW,canvasH)`,
  calls `projectm_opengl_render_frame`, then blits READ framebuffer 0 over (0,0)-(canvas) into `outputFBO_`
  (`src/sources/ProjectMSource.cpp:167-179`).
- E2 (V) Resizing already follows the canvas. `resize()` re-creates `outputFBO_` and calls
  `projectm_set_window_size(canvas)` (`ProjectMSource.cpp:89-105`). The size is not the bug; the target is.
- E3 (V) Framebuffer 0 is the Preview panel's drawable. The GL context is attached to the panel with component
  painting off (`src/render/Renderer.cpp:59-60`). The window is cleared every frame to 0xff0a0a14
  (`Renderer.cpp:371`). `presentCanvas` draws the canvas only inside the fitted rect (`Renderer.cpp:1085-1112`),
  so anything else drawn into framebuffer 0 outside that rect stays visible. Those are the "garbage strips"; the
  diag's window captures show them (V by diag-bf10, `run2/win_*.png`).
- E4 (V) Every source renders at the canvas size:
  - the compositor calls `sourceRenderFn_(..., width, height)` (`src/render/CompositorEngine.cpp:1100-1103`,
    `1208-1211`, `1367-1370`, `1607-1610`; wired at `Renderer.cpp:264-267`);
  - the legacy path uses `renderW/renderH` (`Renderer.cpp:816-823`);
  - the canvas is `resolveCanvas(test lock, Composition::outputWidth/Height)`, debounced
    (`Renderer.cpp:450-460`, `502`);
  - a Source gets no clip fit: "a Source renders AT the canvas size", fit stays {1,1}
    (`CompositorEngine.cpp:650-656`).
- E5 (V) There is ONE ProjectMSource per app. `activeSources_` is keyed by source id (`Renderer.cpp:1314-1325`),
  so every MilkDrop clip on every layer renders the same projectM instance.
- E6 (V) The symptom, re-decoded by me from the diag's PNGs (`scratchpad/diag10/run2`):

  | capture | alpha 255 | alpha 0 |
  |---|---|---|
  | `c_1920x1080_a` | 30.63 % | 69.38 % |
  | `c_3840x2160_a` | 7.66 % | 92.34 % |
  | `c_640x360_a` | 100 % | -- |

  Inside the covered block alpha is always 255. The covered block is min(panel 756x840, canvas), anchored
  bottom-left (diag-bf10.md table).
- E7 (V) The library and how the app links it:
  - libprojectM 4.1.1 at VCS commit 03aa8a7ffdf81165136ee64643c6a781f5c6a391
    (`~/.local/include/projectM-4/version.h`). Its only render export is `projectm_opengl_render_frame`
    (`render_opengl.h:42`).
  - The app links `@rpath/libprojectM-4.4.dylib` (current version 4.1.1) with LC_RPATH
    /Users/boriskarpman/.local/lib (`otool -L` / `otool -l` on the built app).
  - ~/.local contains ONLY projectM, and `-isystem ~/.local/include` reaches ProjectMSource.cpp only through
    projectM's imported target (`build/compile_commands.json`).
  - The dylib is ad-hoc, linker-signed, minos 15.0, built with SDK 15.2. Its dependencies are OpenGL,
    CoreFoundation, libc++ and libSystem only.
  - The app is signed "Audio-DNA Dev" with flags 0x0 (no hardened runtime), so there is no library validation.
  - The stock install has a Release CMake config: `~/.local/lib/cmake/projectM4/projectM4Targets-release.cmake`.
- E8 (V) Where 4.1.1 decides its target. This is static disassembly (`otool -tV` of
  `libprojectM-4.4.1.1.dylib`); nothing was run under a debugger.
  - `ProjectM::RenderFrame` (0x891a4) first renders the transitioning and active presets into their own
    framebuffers.
  - It then executes `glBindFramebuffer(GL_DRAW_FRAMEBUFFER (0x8CA9), 0)` at 0x89518-0x89520. This is the ONLY
    framebuffer bind in RenderFrame.
  - Next it calls `PresetTransition::Draw` (0x89540) while a soft cut runs, else
    `CopyTexture::Draw(activePreset->OutputTexture(), false, false)` (0x89570).
  - Neither Draw binds a framebuffer or sets a viewport.
  - Outside the `Framebuffer` class, the only other `glBindFramebuffer` calls are `BlurTexture::Update`'s
    save/restore pair (it queries 0x8CA6 and restores READ/DRAW at the end).
  - `_projectm_opengl_render_frame` is a tail call to `RenderFrame` (0x8a55c).
  - Consequence: one constant picks the target, so passing the caller's framebuffer there changes nothing else.
- E9 (V disasm, I for duration) `ProjectM::SetWindowSize` (0x89774) only stores width/height; the preset
  framebuffers resize lazily on the next frame. After a canvas change, MilkDrop's internal buffers (its trails)
  are re-created. The diag saw the first frame after a size change come out opaque black
  (`lock_new_640x360`: mean RGB 0.0, alpha 255, re-decoded). How long a real preset takes to build back up is I.
- E10 (V, disasm) Aspect. `ProjectM::GetRenderContext` (0x896b0-0x89770) sets, every frame, from the window size
  (= the canvas):
  - viewport = window size;
  - aspectX = (h > w) ? w/h : 1;
  - aspectY = (w > h) ? h/w : 1;
  - inverse aspects = 1/aspect.

  So MilkDrop is generated natively at the canvas's shape. Aspect correction is ON
  (`ProjectMSource.cpp:56`). The mesh is a normalized 48x36 grid (`:55`).
- E11 (I, high) Output alpha is 1. Evidence is the shader strings in the dylib:
  - CopyTexture's fragment shader is `color = texture(texture_sampler, fragment_tex_coord);`;
  - the preset (HLSL) wrapper writes `_return_value = float4(ret.xyz, 1.0);`;
  - the transition shaders write `vec4(col, 1.0)`.

  T1 and T6 assert alpha 255.
- E12 (V by diag + code) Why it broke "when output became composition size". Before c9c27c8 (2026-09-27),
  sources rendered at the panel's physical size (`c9c27c8^ Renderer.cpp:456`, renderW = compW). Framebuffer 0
  and the source were then the same size and the blit was exact. Today `compW/compH` is only the present size
  (`Renderer.cpp:446-448`).
- E13 (V / not verified) The FBO call was the intended API from day one:
  - `ProjectMSource.h:19-24` says "Calls projectm_opengl_render_frame_fbo()";
  - `research/UNIFIED_BUILD_PLAN.md:762-763` says "Renders to any FBO via projectm_opengl_render_frame_fbo(handle,
    fboId)". That doc's "brew install libprojectm" is wrong: Homebrew's `projectm` is 3.1.12 (`brew info
    projectm`).

  Whether an upstream TAGGED release ships the function is NOT VERIFIED here (no network, by design). This plan
  does not depend on it.
- E14 (V) Same-pattern scan: NO other source or effect has this bug.
  - The only `GL_READ_FRAMEBUFFER, 0` in src/ is `ProjectMSource.cpp:175`.
  - ProjectMSource is the only ProceduralSource subclass (grep `public ProceduralSource`).
  - Every other source draws in `ProceduralSource::render` into its own FBO, re-created at the passed size
    (`src/sources/ProceduralSource.cpp:64-71`, `88-140`).
  - The legacy effect chain writes `canvasFBO_` (`Renderer.cpp:858-864`).
  - Every other `glBindFramebuffer(..., 0)` in src/ is an unbind after setup or a clear.
- E15 (V) Two adjacent issues exist. Both predate this lane, are NOT this bug, and should be filed separately:
  - (a) Stateful sources (Strange Attractor / Gravity Well / Fluid Dynamics) reset to black on a canvas change:
    `ProceduralSource::resize` = `releaseGL` + `initGL`, which clears to alpha 0 (`ProceduralSource.cpp:64-71`,
    `19-47`). Compositor histories, by contrast, are rescaled (Pitfall 37).
  - (b) The shared projectM instance (E5): two MilkDrop layers show one preset and advance it twice per frame.
- E16 (V) Main's GL-state contract breaks on init and resize frames. `initGL`/`resize` run BEFORE
  `saveGLState` (`ProjectMSource.cpp:124-127` vs `164-165`). `createFBO` ends by binding 0
  (`ProceduralSource.cpp:273-287`). So on an init or canvas-change frame, the caller's framebuffer binding is
  replaced by 0.
- E17 (V) Existing test and probe infrastructure:
  - Unit tests: a private CGL 4.1 context + `juce::gl::loadFunctions()` + a loud SKIP
    (`tests/test_source_defaults_gl.cpp:99-183`, `261`). GL test targets are at `tests/CMakeLists.txt:2501-2530`
    and `2954-2988`. `ShaderManager.cpp`/`FullscreenQuad.cpp` are already built in tests (`:286-296`). The
    `test_projectm_preset_manager` block is at `:802-831`.
  - Live, test server: 8080 `/api/load_milkdrop_preset {preset_path}` loads with smooth=false
    (`src/test/TestServer.cpp:172`, `1746`). Also `set_composition_params` (`:229`), `set_output_tap` (`:243`),
    `output_probe` (no window, `:247`), `debug/gl_context_cycle` (`:255`).
  - Live, production server: 7070 `/api/trigger_clip` (`src/api/ApiServer.cpp:178`) and `/api/load_image`
    (`:243`). `/api/state` reports fps / frame_time_ms / gpu_time_ms (`.harmony/probe-canvas.py:406-434`).
  - A/B driver: `.harmony/probe-vupload-ab.sh` interleaves launches with VIDEO_APP per arm, collects DATA lines
    and taints launches that saw a compiler or a CPU burner.
- E18 (V) CI never installs projectM (`.github/workflows/build.yml`), so `ProjectM_FOUND` is false there and
  nothing in this plan runs or breaks on CI.
- E19 (V) `Clip::MediaType::Source` = 4 (`src/model/Clip.h:23`).
- E20 (V version, I need) This machine has CMake 4.2.3. A vendored subproject with an old
  `cmake_minimum_required` may need `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`.

---

## (3) DESIGN FORKS

**F1 -- How projectM's final picture reaches a canvas-sized target.**
- **(a) CHOSEN: backport the upstream-shaped `projectm_opengl_render_frame_fbo(projectm_handle, uint32_t)`.**
  - Apply it onto the exact commit this machine's 4.1.1 was built from (03aa8a7).
  - `RenderFrame(uint32_t target = 0)`, and the single DRAW bind (E8) uses `target`.
  - Rendering stays byte-for-byte the engine Boris has today; only the destination changes.
  - The app calls the same symbol an upstream build exports, so a later upgrade drops the patch with zero app
    change.
- (b) Upgrade to an upstream release that exports it. Loses:
  - its release status cannot be verified here (E13);
  - a version jump changes the engine under Boris's whole preset library (I: shader-translation, audio and
    timing changes between minors), which is a taste risk nobody asked for;
  - the rebuild cost is the same as (a).

  It stays a separate, later decision.
- (c) A separate GL context with a canvas-sized default framebuffer. Loses: a core-profile CGL context has no
  drawable or pbuffer (diag probe); an offscreen window is window-server state; cross-context sharing needs
  fences.
- (d) Rebind libprojectM's `glBindFramebuffer` import at runtime (fishhook-style). Loses: it rewrites a signed
  image's pointers, also catches `Framebuffer::Unbind`, and is undebuggable.
- (e) No library change: draw projectM at panel size inside framebuffer 0 and upscale it. Loses:
  - resolution is tied to the window (the Pitfall 37 class);
  - an Output at 4K would show at most a 0.6 MP upscale;
  - it still draws into the panel.
- (f) Binary-patch the dylib's immediate value. Loses: no parameter, needs a re-sign, unreviewable.

**F2 -- Where the patched library lives.**
- **(a) CHOSEN: a NEW versioned prefix `$HOME/.local/opt/projectm-4.1.1-fbo1`.**
  - `cmake/projectm/build-projectm.sh` installs it; `cmake/FindProjectM.cmake` searches it FIRST.
  - No existing file on the machine changes. Old binaries and other worktrees keep the stock dylib until they
    rebase. No live lock is needed (no running app maps these new files).
- (b) Overwrite ~/.local in place. Loses:
  - it mutates shared state mid-session (every worktree and main's app load the same rpath, E7);
  - `cmake --install` rewrites a mapped dylib in place, so a running app can be killed;
  - a rollback needs backups.
- (c) FetchContent/ExternalProject inside the app build. Loses:
  - every worktree's clean configure clones and builds projectM (network plus minutes);
  - the house pattern for heavy native deps is `find_package` of a pre-installed library (`CMakeLists.txt:15-22`:
    Aubio, FFmpeg, ProjectM).

**F3 -- What configure does when projectM is found WITHOUT the function.**
- **(a) CHOSEN: `FATAL_ERROR` naming `cmake/projectm/build-projectm.sh`.** A build can never silently ship BF10
  again. CI is unaffected (E18).
- (b) Fall back to the framebuffer-0 path. Loses: it silently ships a known bug.
- (c) Build without MilkDrop and warn. Loses: Boris silently loses MilkDrop.

**F4 -- MilkDrop's internal resolution.**
- **(a) CHOSEN: the full canvas.** Main already renders projectM's internal passes at the canvas size (E2/E10),
  and every other source does the same.
- (b) Cap it (e.g. at most 1080 lines) and upscale: cheaper at 4K, softer. Taken ONLY if the 4K perf rows show
  BOTH arms below 58 fps; then it becomes Boris question Q1.

**F5 -- Mesh vs aspect.**
- **(a) CHOSEN: keep 48x36.**
  - Nothing stretches (E10): the mesh is a normalized sampling grid and aspect comes from the window size.
  - Landscape is unchanged, including Boris's default 1920x1080.
  - On a portrait canvas the warp cells are taller (22.5 x 53 px at 1080x1920): smoothness only, not shape.
- (b) Orientation-following 48x36 / 36x48: squarer cells in portrait. Not requested; a follow-up only if Boris
  sees coarse warps on portrait (W5).
- (c) Square-cell proportional (48x27 at 16:9): changes today's landscape look. Rejected.

**F6 -- Keeping MilkDrop opaque.**
- **(a) CHOSEN: rely on projectM writing alpha 1 (E11), asserted by T1/T6 with both fixture kinds.**
  Pre-registered: if any alpha < 255 is seen, add an alpha-only clear (`glColorMask(0,0,0,1)` plus a clear to
  alpha 1) right after the render call, then re-run.
- (b) Always clear alpha. Loses: an extra full-canvas pass at 4K for no proven need.
- (c) An RGB8 target. Loses: GL 4.1 does not require RGB8 to be colour-renderable.

**F7 -- Live gate vehicle.**
- **(a) CHOSEN: a new `.harmony/probe-milkdrop.{sh,py,json}`.** Its refuse / launch / quit / lock-gate code is
  cloned from `probe-canvas.sh`. It honours VIDEO_APP, so `.harmony/probe-vupload-ab.sh` drives its perf rows
  unchanged.
- (b) Add rows to `probe-canvas.py`. Loses: MilkDrop fixtures would leak into the canvas witness, and
  `probe-canvas.sh` reads CANVAS_APP, not VIDEO_APP.

**F8 -- A canvas change while MilkDrop plays.**
- **(a) CHOSEN: accept that projectM re-creates its internal buffers (the trails restart, E9).** There is no API
  to rescale them, and a size change is a setup action.
- (b) Hold the last picture for N frames. Loses: any N is arbitrary, and it hides nothing for long-trail presets.

---

## (4) ITEMS + BUILD STAGES

### I1 (stage S1) -- The patched libprojectM in its own prefix

**New files:**
- `cmake/projectm/0001-render-frame-fbo.patch`
- `cmake/projectm/build-projectm.sh`

**The patch** applies to 03aa8a7. The builder locates each change by content; these paths are ASSUMED:
`src/libprojectM/ProjectM.hpp/.cpp`, the C wrapper beside `projectm_opengl_render_frame`, and
`src/api/include/projectM-4/render_opengl.h`.
1. `ProjectM::RenderFrame()` becomes `RenderFrame(uint32_t targetFramebufferObject = 0)`. The one
   `glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0)` (E8) becomes `glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
   targetFramebufferObject)`. Nothing else in the library changes.
2. `render_opengl.h` declares
   `PROJECTM_EXPORT void projectm_opengl_render_frame_fbo(projectm_handle instance, uint32_t framebuffer_object_id);`
   and adds `#include <stdint.h>` (types.h has only stdbool/stddef, V). If the cloned repo's upstream master
   already has this function, copy its declaration and doc comment VERBATIM instead.
3. The C wrapper forwards the new function to `RenderFrame(framebuffer_object_id)`.
   `projectm_opengl_render_frame` is unchanged and still targets 0.

**The script** (`set -euo pipefail`):
- It is idempotent: it exits 0 if `$PREFIX/lib/libprojectM-4.dylib` already exports the symbol.
- `PREFIX` defaults to `$HOME/.local/opt/projectm-4.1.1-fbo1`.
- Steps:
  1. Clone https://github.com/projectM-visualizer/projectm.git into `$TMPDIR`.
  2. `checkout --detach 03aa8a7ffdf81165136ee64643c6a781f5c6a391`, then `submodule update --init --recursive`.
  3. `git apply --check`, then apply the patch.
  4. Configure Release, shared libraries, `CMAKE_INSTALL_PREFIX=$PREFIX`. Keep playlist ON (the stock install
     has it), tests and SDL UI OFF. Add `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` only if CMake 4 refuses a vendored
     subproject; record whichever was used. Verify the option names against the checked-out CMakeLists; they
     are ASSUMED.
  5. Build, then install.
- It verifies `nm -gU` shows `_projectm_opengl_render_frame_fbo` and `otool -D` = `@rpath/libprojectM-4.4.dylib`.
- It deletes the work dir on success.

**RED-first (probe row, pre-registered).** The lib-probe is diag10's `pmprobe/probe.cpp` plus a mode flag, built
against `$PREFIX`. It uses an offscreen CGL 4.1 context, its own 320x180 framebuffer bound, the fixture
`tests/fixtures/milkdrop/bf10_solid.milk`, and 90 frames.
- RED: on the stock library the fbo mode does not link. Plain mode on the stock library is 0/57600 lit (diag).

**GREEN bar:**
- Patched library, plain mode: still 0 % lit (old behaviour preserved).
- Patched library, fbo mode: at least 99.9 % of pixels within ±6 of the frame median, median max channel ≥ 64.
- The symbol and install-name checks pass.

**Risk:** network (R1); source shape (R2); CMake 4 policy (R11).

### I2 (stage S2) -- Build wiring + configure guard

**Files:** `cmake/FindProjectM.cmake` (top of file only), and `CMakeLists.txt` (a block right after
`find_package(ProjectM QUIET)`, `:21-22`).

**FindProjectM.cmake:**
- Add a cache PATH `AUDIODNA_PROJECTM_PREFIX` (default `$HOME/.local/opt/projectm-4.1.1-fbo1`).
- If a cached `projectM4_DIR` lies outside that prefix, `unset(projectM4_DIR CACHE)`. Use `string(FIND)`, not a
  regex (paths contain dots).
- Call `find_package(projectM4 CONFIG QUIET PATHS ${prefix}/lib/cmake NO_DEFAULT_PATH)` first. The existing
  searches below it stay unchanged.

**CMakeLists.txt:**
- `if(ProjectM_FOUND)`:
  - `unset(AUDIODNA_PROJECTM_HAS_RENDER_FBO CACHE)`;
  - `check_cxx_source_compiles` with `CMAKE_REQUIRED_LIBRARIES ProjectM::ProjectM` on
    `#include <projectM-4/projectM.h>` + `int main(){ projectm_opengl_render_frame_fbo(nullptr, 0u); }`.
- If the check fails: `FATAL_ERROR "libprojectM at <projectM4_DIR> lacks projectm_opengl_render_frame_fbo (BF10:
  MilkDrop must render into its canvas-sized framebuffer). Run cmake/projectm/build-projectm.sh, then
  re-configure."`
- Otherwise print `message(STATUS "libprojectM: <projectM4_DIR> (render_frame_fbo: yes)")`.

**RED-first (probe rows):**
- In a scratch build dir: `-DAUDIODNA_PROJECTM_PREFIX=/nonexistent -DprojectM4_DIR=$HOME/.local/lib/cmake/projectM4`
  must exit non-zero, and the output must contain "build-projectm.sh". This is the guard's teeth.
- Re-configuring main's EXISTING `build/` (stock `projectM4_DIR` cached) must exit 0, with the STATUS line naming
  the fbo1 prefix. This is the stale-cache path.

**GREEN bar:**
- Both rows behave as stated.
- New app: `otool -l` LC_RPATH contains `$PREFIX/lib` and NOT /Users/boriskarpman/.local/lib; `otool -L` shows
  `@rpath/libprojectM-4.4.dylib`.
- In `compile_commands.json`, ProjectMSource.cpp carries `-isystem $PREFIX/include` and no `.local/include`.

**Risk:** R4.

### I3 (stage S2) -- ProjectMSource draws into its own canvas FBO; the caller's GL state survives init/resize frames

**Files:** `src/sources/ProjectMSource.cpp` `render()` only (`:120-195`), plus one sentence in the header comment
of `ProjectMSource.h` (`:13-24`).

**Change:**
1. `GLState savedState; saveGLState(savedState);` becomes the FIRST statement of `render()`, before
   `initGL`/`resize` (E16). Every return restores it, including the `!pm_` early return and the
   `#else` placeholder path.
2. Lines `:167-179` are replaced by:
   - `glBindFramebuffer(GL_FRAMEBUFFER, outputFBO_);`
   - `glViewport(0, 0, fboWidth_, fboHeight_);`
   - `projectm_opengl_render_frame_fbo(pm_, static_cast<uint32_t>(outputFBO_));`

   The blit and every read of framebuffer 0 are deleted.
3. Header comment: "projectM draws its final picture straight into outputFBO_ (canvas-sized); it never touches
   the window framebuffer (BF10, Pitfall NN)."

No change to `initGL`, `resize`, `applyParams`, the PCM feed or the preset queue. No new mutex, no
steady-state allocation (Sacred Rules 1-4 untouched).

**RED-first test:** `tests/test_projectm_canvas_gl.cpp` (NEW; target `test_projectm_canvas_gl`).

Build setup:
- Built only under `if(APPLE AND ProjectM_FOUND)`, with `AUDIODNA_HAS_PROJECTM=1`.
- Sources: the test, `src/sources/{ProjectMSource,ProceduralSource,PresetSelector,ProjectMPresetManager}.cpp`,
  `src/render/{ShaderManager,FullscreenQuad}.cpp`, plus whatever ShaderManager needs to link
  (TextureManager / LUTLoader, as in `tests/CMakeLists.txt:286-296`).
- Links: Catch2, juce_core, juce_opengl, `ProjectM::ProjectM`, `-framework OpenGL`.
- The block is INSERTED right after `test_projectm_preset_manager` (`tests/CMakeLists.txt:831`), not appended
  at the end of the file.

Rig:
- The `test_source_defaults_gl` shape: CGL 4.1 core, `juce::gl::loadFunctions()`, `REQUIRE_GL` SKIP.
- `render()` ignores its ShaderManager and quad arguments. Pass a `ShaderManager` built on an unattached
  `juce::OpenGLContext` plus a default `FullscreenQuad`. If that cannot be constructed, add a
  `ProjectMSourceTestAccess` seam (the `VideoPlayerTestAccess` precedent), never a production signature change.
- `setPresetLocked(true)`.
- Readback: the RETURNED texture is attached to a test FBO and read with `glReadPixels` (RGBA8).

Fixtures (`tests/fixtures/milkdrop/`):
- `bf10_solid.milk`: a comp shader returning a constant colour. Fallback if the comp shader does not translate:
  a filled custom shape with rad ≥ 1.5.
- `bf10_solid_b.milk`: a second constant colour.
- `bf10_time.milk`: colour = f(time), period about 2 s.
- `bf10_circle.milk`: fDecay 0, gamma 1, no waves / borders / echo, one filled 64-sided custom shape with rad
  0.25 at the centre.

Cases. All six are RED on main:
- **T1 fill.**
  - Sizes 1280x720, 1920x1080, 1080x1920, 3840x2160.
  - After the first `render()` the test clears the RETURNED texture to a magenta sentinel, then renders 3 frames.
  - PASS iff:
    - the texture is exactly the requested size;
    - alpha == 255 on 100 % of pixels;
    - at least 99.99 % of pixels lie within ±6 of the median;
    - the median max channel ≥ 64 and the median is not the sentinel.
  - RED on main (I, high): main draws into framebuffer 0, which has no storage in a drawable-less context
    (diag probe: 0/57600). Its blit from framebuffer 0 copies nothing, so the sentinel stays.
- **T2 canvas change.**
  - Sequence: 1920x1080 for 3 frames, then 1080x1920, then 1920x1080.
  - From the FIRST frame at each new size: the texture has the new size and alpha is 255 everywhere.
  - The fixture colour covers at least 99.99 % within ≤ 2 frames (the exact count is printed, INFO).
- **T3 live, no stale region.**
  - `bf10_time` at 1920x1080, frames paced about 16 ms apart, two reads 300 ms apart.
  - PASS iff every 64x64 tile's mean changed by ≥ 2 levels.
- **T4 no stretch.**
  - `bf10_circle` at 1920x1080 and at 1080x1920.
  - PASS iff the lit blob's bbox has |w/h - 1| ≤ 0.04 and its centre is within ±2 % of the canvas centre.
  - If T4 fails on the FIX, STOP and report the numbers (E10 would be wrong). Never retune this bar.
- **T5 caller state.**
  - Before `render()` the test binds its own sentinel FBO (cleared to a known colour) and sets viewport
    (1,2,33,44).
  - After `render()` on a steady frame, on the init frame and on a resize frame, PASS iff:
    - DRAW binding == the sentinel;
    - viewport == (1,2,33,44);
    - the sentinel's pixels are unchanged;
    - `glGetError()` == GL_NO_ERROR.
  - RED on main: binding 0 after init/resize (E16), and the framebuffer-0 blit raises an error.
- **T6 soft cut.**
  - `bf10_solid`, then `getPresetSelector().setBlendSeconds(1.0)`, then `loadPreset(bf10_solid_b, true)`;
    40 paced frames follow.
  - PASS iff:
    - every frame has alpha 255 everywhere;
    - every pixel stays inside the per-channel box [min(A,B)-6, max(A,B)+6];
    - at least one frame's median lies strictly between A and B (this proves `PresetTransition::Draw` also hit
      our FBO);
    - the last frames equal B.

**GREEN bar:** T1-T6 PASS. The RED run is captured in the worktree BEFORE the fix (test, target and fixtures are
committed first) and is quoted in the lane report. The builder iterates the fixtures only against the FIX,
never to make main pass.

**Risks:**
- R5 (alpha): the F6 rule applies.
- If a preset pass leaves a small viewport, T1 fails on coverage. The patch then also calls
  `glViewport(0,0,w,h)` right after its bind.
- R12 (T1 might be GREEN on main).

### I4 (stage S3) -- Live probe + A/B perf rows

**New files:**
- `.harmony/probe-milkdrop.sh`: refuse / launch `--test-mode` / quit / lock-gate cloned from `probe-canvas.sh`.
  The app is MILKDROP_APP, else VIDEO_APP, else `<root>/build/.../Audio-DNA.app`. Main-window Quartz id helper
  as in diag10 `run2.sh`.
- `.harmony/probe-milkdrop.py`.
- `.harmony/probe-milkdrop.json`: fixture and preset paths, plus the bars.
- `.harmony/probe-milkdrop-ab.py`: reads `probe-vupload-ab.sh`'s `ab.tsv` and applies the G3 bars.

**Capture rules:**
- Captures use `render_frame` WITHOUT a width/height override (E9 trap).
- The canvas is set through 8080 `set_composition_params`, then the probe waits ≥ 1.5 s.
- HTTP uses `Connection: close`.
- No Output window, no synthetic input, no full-screen capture.

**Shared bar "uniform"** (used by several rows): alpha 255 on 100.000 % of pixels, at least 99.99 % of pixels
within ±6 of the median, median max channel ≥ 64.

**Rows** ("RED" = expected to FAIL on the pre-lane app):
- **m1_fill_legacy (RED).**
  - `load_milkdrop_preset(bf10_solid)`.
  - Canvases 1280x720, 1920x1080, 3840x2160 and 1080x1920 (all RED), plus 640x360 (a guard, GREEN on both).
  - PASS iff the PNG is exactly the canvas size and meets the "uniform" bar.
- **m2_live_no_stale (RED).**
  - A pinned bundled preset, calibrated on the FIX to change ≥ 20 % of every 64x64 tile in 0.5 s at both sizes.
    If the first candidate fails, pick another bundled preset and record why.
  - At 1920x1080 and 1080x1920: two captures 0.5 s apart.
  - PASS iff alpha is 255 on 100 % and every tile has ≥ 20 % of its pixels changed by more than 8 levels.
- **m3_runtime_change (RED).**
  - `bf10_solid`: 1920x1080, then 2560x1440, then 1920x1080; capture 0.5 s after each change.
  - PASS iff each PNG is the new size and meets the m1 bars.
  - INFO: 8080 `peak_frame_time_ms` across each change, and the luminance of a real preset captured right after
    a change (the re-warm).
- **m4_comp_load_clip (RED).** This is the deck/clip path (`CompositorEngine.cpp:1100-1103`) plus a composition
  load's canvas change (Pitfall 58).
  - 7070 `load_composition` of a JSON with outputWidth 1280 / outputHeight 720, where deck 0 L0 col0 =
    `{mediaType 4, sourceType "projectm_visualizer"}`.
  - Then 7070 `trigger_clip`, then `load_milkdrop_preset(bf10_solid)`.
  - Then a second JSON at 1080x1920.
  - PASS iff both canvases meet the m1 bars.
- **m5_preset_switch.**
  - `bf10_solid`, then `load_milkdrop_preset(bf10_solid_b)`. This REST route is a hard cut
    (`TestServer.cpp:1746`); the soft cut is covered by T6.
  - PASS iff 0.5 s later B meets the "uniform" bar.
- **m6_panel_bars (RED).** Window-only captures (`screencapture -x -o -l <Quartz id>`).
  - CONTROL first, for each canvas 640x360, 3840x2160 and 1080x1920, BEFORE MilkDrop is ever loaded:
    1. Use the `c_legacy_image_fit` set-up (`probe-canvas.py:355-366`): a composition with an untriggered layer,
       then 7070 `load_image` of a probe-made solid green PNG of the canvas's exact size.
    2. Capture. The present rect is the green rectangle.
    3. Record the 12-px strips just outside that rect, on its bar sides (inside the panel).
  - Control self-check: the strips must have std ≤ 3 and max channel ≤ 40. Otherwise the row is INVALID (rerun
    once, then report); it never passes.
  - Then `bf10_solid` at each size, and capture again.
  - PASS iff:
    - inside the rect, at least 99.5 % of pixels are within ±10 of the rect's median;
    - the strips have std ≤ 3 and a mean within ±6 of the control's strips.
- **m7_output_tap (RED).** This is what an Output, the recorder and Syphon read (`Renderer.cpp:1117-1122`).
  - `set_output_tap` on (opens no window), `bf10_solid` at 1920x1080.
  - `output_probe` 1920x1080 must meet: at least 99.99 % of pixels within ±6 of the median, median max ≥ 64.
- **m8_context_cycle.**
  - `/api/debug/gl_context_cycle`, then the m1 bars at 1920x1080 (the `releaseGL`/`initGL` path).
- **perf_md_1080 / perf_md_4k (A/B only).**
  - The m4 composition with the pinned HEAVY preset "flexi - grind my glitch up [319].milk" (the most comp code
    in the bundled set, plus 10 GetBlur calls), loaded via `load_milkdrop_preset`.
  - Canvas 1920x1080 or 3840x2160; 3 s warm-up; 7070 `/api/state` sampled 10 x 0.5 s.
  - Output line: `DATA perf_md_<size> fps=<mean> fps_min=<min> frame_ms=<mean> gpu_ms=<mean> gpu_peak_ms=<max>`.

**Calibration:** the builder runs m1-m8 on the pre-lane app (the RED lines) and on the lane app (GREEN), and
quotes the numbers in the lane report.

### I5 (stage S3) -- Docs

See section (6).

### BUILD STAGES

**S1 = I1** (one builder context).
- Machine-level and idempotent; no behaviour visible in the repo.
- CAN land alone: it changes nothing until S2 links it.
- Its projectM compile is several minutes of clang. That taints other lanes' quiet-lock perf runs (they re-run),
  so run it outside their perf windows.

**S2 = I2 + I3.**
- Depends on S1's prefix existing on this machine.
- I2 and I3 MUST ship together: I2 makes I3's symbol mandatory.

**S3 = I4 + I5.**
- Depends on S2 for its GREEN results. Ships in the same merge as S2.

Practical recommendation: one builder runs S1, S2, S3 in a single worktree (this is a small lane).

**Merge sequencing:** bf10 has NO ordering dependency on bf9, bf6, ui, bf7, bf2, bf1, bf45 or tsan-r5. It touches
no core shared file: nothing in MainComponent, ClipInspector / LayerInspector / SignalInspector, DeckView or the
deck tab row, TopBar, CompositorEngine, Layer.h / Clip.h / Composition.h, or serialization. Recommendation: build
it in parallel early, and merge whenever it is GREEN.

Shared hunks to sequence:

| file | bf10's hunk | other lanes / note |
|---|---|---|
| `CMakeLists.txt` | the projectM block right after `:21-22` only | bf1 / bf45 add app sources elsewhere |
| `tests/CMakeLists.txt` | inserted after `:831` | others append at the end |
| `docs/claude/pitfalls.md` | one new entry | Harmony numbers it |
| `CLAUDE.md` | index line + the line-14 payment | see (6) |
| `.harmony/APP-INVENTORY.md` | `:86` and the counts area `:26-27` | -- |
| `docs/claude/architecture.md` | `~:162-176` and `~:191-192` | -- |
| `docs/claude/rendering.md` | `:134-142` | -- |
| `docs/claude/build-other-platforms.md` | after `:45-49` | -- |
| `docs/claude/testing-eyes.md` | `:78-95` | -- |
| `THIRD_PARTY_LICENSES.md` | new entry | -- |

---

## (5) GATES (Harmony, after the merge)

**G0 -- build hygiene.**
- A fresh configure + build of merged main in a scratch dir, AND a re-configure of main's EXISTING `build/`:
  both exit 0, and the STATUS line names `.../projectm-4.1.1-fbo1`.
- `nm -gU $PREFIX/lib/libprojectM-4.dylib | grep projectm_opengl_render_frame_fbo` finds the symbol.
- App `otool -l`: LC_RPATH contains `$PREFIX/lib` and NOT /Users/boriskarpman/.local/lib.
- `compile_commands.json` for ProjectMSource.cpp has no `.local/include`.
- Negative control (I2 row): it must FAIL, naming `build-projectm.sh`.

**G1 -- unit.**
- `ctest -R test_projectm_canvas_gl`: all 6 cases PASS. A SKIP counts as a FAIL for this gate.
- The full ctest suite is green.
- The lane report shows the pre-fix RED output.

**G2 -- live.**
- Hold the live lock, then run `.harmony/probe-milkdrop.sh` rows m1-m8: all PASS.
- If m6 is INVALID, rerun once, then report.

**G3 -- perf, interleaved A/B, at least 5 rounds.**

Command: `LOCK_LIB=<lock helper> LANE=bf10 bash .harmony/probe-vupload-ab.sh <A = merge-base Release app>
<B = merged-main Release app> 5 .harmony/probe-milkdrop.sh perf_md_1080,perf_md_4k <out>`, then
`.harmony/probe-milkdrop-ab.py <out>/ab.tsv`.

Pre-registered rules. All values are medians over per-launch means.

| row | GATE bar | if not met / report |
|---|---|---|
| 1080 | fps_B ≥ 58 AND gpu_B - gpu_A ≤ 1.0 ms | if fps_A < 58 (the preset is too heavy for this rig even before), the 58 bar becomes fps_B ≥ fps_A - 2 |
| 4K | gpu_B - gpu_A ≤ 2.0 ms AND fps_B ≥ fps_A - 3 | absolute fps / gpu REPORTED; BOTH arms < 58 fps is not a bf10 failure: file "MilkDrop render scale at 4K" with numbers and ask Boris Q1 |

- **Teeth rule:** spread_A = max - min of A's 5 launch means, per metric. A delta bar ≤ 2 x spread_A is INFO,
  not GATE.
- **Discriminator if a delta bar FAILS:**
  1. Build A' = the merge-base source configured with `-DprojectM4_DIR=$PREFIX/lib/cmake/projectM4` (old code,
     rebuilt library).
  2. Run A vs A' for 5 rounds.
  3. If A' ≈ B, the cost is the rebuilt library (compiler / SDK, R3), not the fix. If A' ≈ A, it is the fix's
     final full-canvas pass: investigate.
- Before any verdict, check `ps` for CPU burners; the driver taints compiler and burner launches.

**G4 -- regression.** `.harmony/probe-canvas.sh`, all rows, PASS on merged main.

**G5 -- VISUAL WORK GATE.** The Preview panel's picture is a visible change.
- Captures: window-only, by Quartz window id, of the main window, BEFORE (the pre-lane app) and AFTER.
  - MilkDrop playing two real bundled presets at canvases 1920x1080, 3840x2160, 1080x1920 and 640x360.
  - A 1920x1080 to 2560x1440 change sequence at +0.1 / +0.5 / +2 s.
- Critic panel: visual-design, UX, graphic-design and logic critics, plus a dedicated interaction-logic critic
  for the size-change sequence.
- What the critics judge:
  - the picture runs edge to edge in the panel's picture area;
  - the bars are plain;
  - there are no seams, blocks or stale strips;
  - round shapes stay round;
  - the presets look as before.
- Then an artifact page Boris opens: before/after pairs, one plain sentence each.

---

## (6) DOCS

**`docs/claude/pitfalls.md`** gets a new entry, "Pitfall NN" (Harmony assigns; next free is 64). Draft:

> NN. **A library that renders "to the screen" renders into the Preview PANEL -- MilkDrop draws only through
> `projectm_opengl_render_frame_fbo(pm_, outputFBO_)` into its own canvas-sized FBO, never framebuffer 0**:
> libprojectM 4.1.1's `projectm_opengl_render_frame` binds DRAW framebuffer 0 itself just before its final copy
> (`ProjectM::RenderFrame`), and framebuffer 0 is the panel's drawable (the panel's pixel size). So once the
> canvas became the composition's size (Pitfall 37), MilkDrop covered only a panel-sized block at the canvas's
> bottom-left (transparent elsewhere, on every output) and left its raw drawing in the panel's bars (BF10,
> s-rta-1002b). The app links a PATCHED 4.1.1 (`cmake/projectm/0001-render-frame-fbo.patch`, installed by
> `cmake/projectm/build-projectm.sh` into `$HOME/.local/opt/projectm-4.1.1-fbo1`) that adds the upstream-shaped
> `projectm_opengl_render_frame_fbo`; configure FAILS if the found libprojectM lacks it. A canvas size change
> re-creates projectM's internal buffers (its trails restart; the first frame can be black): capture MilkDrop
> after `set_composition_params` + a wait, never with a `render_frame` width/height override. One
> `ProjectMSource` serves every MilkDrop clip (`Renderer::getOrCreateSource`). Guards:
> `tests/test_projectm_canvas_gl.cpp`; live `.harmony/probe-milkdrop.sh` (m1-m8).

**`CLAUDE.md`** gets a Pitfall index line (about 140 bytes):

> NN. MilkDrop draws only through `projectm_opengl_render_frame_fbo` into its canvas FBO -- before touching
> ProjectMSource or libprojectM.

How the bytes are paid:
- In line 14, replace the 18-category breakdown "(3D 24, Geometric 11, ... Routing 1)" with "(per-category
  counts: `.harmony/APP-INVENTORY.md`)". That saves 150 bytes (measured: 253 -> 103).
- The breakdown moves verbatim into APP-INVENTORY.md; CLAUDE.md:3 already names that file the canonical home for
  counts.
- Net change is ≤ 0 bytes (24,002 -> about 23,990).
- Fallback, if another lane already used that text, or Harmony insists on a `docs/claude` destination: drop the
  "### Latency Budget" stub. The Trigger table already routes the latency budget to `architecture.md`.

**Other docs:**

| file | where | change |
|---|---|---|
| `docs/claude/rendering.md` | "Composition Canvas and the Preview Panel" (`:134-142`) | one paragraph: MilkDrop renders straight into its canvas-sized FBO through the patched libprojectM; only `presentCanvas` draws into the window framebuffer; a canvas change restarts MilkDrop's trails |
| `docs/claude/build-other-platforms.md` | new "### libprojectM (MilkDrop) -- patched 4.1.1" after `:45-49` | why; the script; the prefix; the configure guard; how to upgrade (drop the patch once upstream has the same function, then re-run G1-G3) |
| `docs/claude/architecture.md` | tech-stack table near `:170`; dependencies note `:176`; cmake tree `:191-192` | a libprojectM row (4.1.1 + local patch, LGPL-2.1, optional; `cmake/FindProjectM.cmake`, `cmake/projectm/`); the note mentions optional libprojectM; the tree gains `FindProjectM.cmake` and `projectm/` |
| `docs/claude/testing-eyes.md` | "Source Testing via Eyes" (`:78-95`) | the MilkDrop capture rule (`set_composition_params` + wait; never a size override) |
| `.harmony/APP-INVENTORY.md` | MilkDrop row (`:86`) and counts area (`:26-27`) | "renders at the composition canvas size (BF10, s-rta-1002b); needs the patched libprojectM"; the moved per-category breakdown; the unit-test count (+1 target, 6 cases) |
| `THIRD_PARTY_LICENSES.md` | new entry | libprojectM 4.1.1, LGPL-2.1, dynamically linked, locally patched (`cmake/projectm/0001-render-frame-fbo.patch`; upstream github.com/projectM-visualizer/projectm @ 03aa8a7) |

- `BORIS_DECISIONS.md`: nothing. This is a bug fix that restores the 2026-09-26 canvas ruling; no new ruling.
- `.harmony/binding-decisions.md`: Harmony's BF10 closure line.

---

## (7) RISKS (each with the cheapest discriminating test)

- **R1 -- Network or clone blocked.** S1 is then BLOCKED.
  - Test: `git ls-remote https://github.com/projectM-visualizer/projectm.git` (seconds).
  - Fallback: Researcher fetches the v4.1.1 source tarball (with vendored submodules).
- **R2 -- The source at 03aa8a7 is shaped differently from E8.** The I1 lib-probe decides: fbo mode must fill
  ≥ 99.9 % and plain mode must stay at 0 %. Whatever the source layout, the one DRAW bind in RenderFrame is the
  thing to change.
- **R3 -- The rebuilt library is not the stock build** (CLT SDK 26.2 vs the stock's 15.2, plus compiler flags).
  Numerics or speed could drift. Test: the G3 discriminator A'.
- **R4 -- Stale CMake cache, or two rpaths.** The stock dylib could load first. Worst case is a loud dyld
  "symbol not found" at launch, never a silent failure. Test: G0 (`otool -l`, and configuring the existing
  `build/`).
- **R5 -- Alpha < 1 on some path.** A MilkDrop clip would turn see-through on a layer. Tests: the T1/T6/m1 alpha
  bars; the F6 rule applies.
- **R6 -- 4K cost.** Main already runs projectM's internal passes at the canvas size (E2), so the fix adds only
  the final full-canvas copy. If BOTH arms are slow at 4K, that is the follow-up behind Q1, not a bf10 regression.
- **R7 -- The re-warm after a canvas change reads as a glitch to Boris.** Covered by W3; F8(b) stays on file.
- **R8 -- One shared projectM instance (E15b).** Two MilkDrop layers fight over one preset. Under bf9b (layers
  keep playing across decks) that state is easier to reach. File it as "one projectM per playing clip", a
  separate lane.
- **R9 -- LGPL.** If Audio-DNA is ever distributed, the source of a modified LGPL library must be offered. The
  in-repo patch plus the THIRD_PARTY_LICENSES entry cover that. ASSUMED: it is not distributed today.
- **R10 -- Strongest counterargument: "don't carry a fork, upgrade upstream."** It loses today:
  - the release status cannot be verified offline;
  - it is an engine change under Boris's entire preset library;
  - the rebuild cost is identical.

  The backport exports the upstream-shaped symbol, so upgrading later stays a one-line decision: drop the patch
  and re-run G1-G3.
- **R11 -- CMake 4.2.3 refuses a vendored subproject's old minimum version.** Use
  `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` in the script and record it.
- **R12 -- T1 turns out GREEN on main.** That would mean framebuffer 0 has storage in a drawable-less CGL
  context. The builder reports it. T5 stays RED on main (the init/resize binding), and live m1 / m6 / m7 remain
  the RED witnesses for the canvas behaviour.

---

## (8) WHAT ONLY BORIS CAN CHECK

- **W1** On his Output or projector, at his usual composition size, MilkDrop fills the picture edge to edge.
  There is no black block, and the Preview panel's bars are plain.
- **W2** His favourite presets look and move exactly as before. It is the same MilkDrop engine; only where it
  draws has changed.
- **W3** Changing the composition size from the dropdown while MilkDrop plays: the picture refits, and its
  trails start over for a moment. Is that acceptable?
- **W4** At 4K, if he uses it, motion stays smooth with his heaviest presets.
- **W5** On portrait compositions, if he uses them, MilkDrop looks natural and round shapes stay round.

---

## (9) QUESTIONS FOR BORIS (plain words, with defaults; the build never waits)

- **Q1** Asked ONLY if the 4K test shows stutter on both the old and the new app: "On a 4K composition, MilkDrop
  may be too heavy to stay smooth on this Mac. Would you rather it draw a little softer and stay smooth, or keep
  full sharpness even if it stutters?" DEFAULT: keep full sharpness.
- **Q2** "When you change the composition size while MilkDrop is playing, its trails start over for a moment. Is
  that fine?" DEFAULT: yes, fine. Size changes are setup, not live.

STATUS: COMPLETE

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 15:58:22) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1002b/ruling-bf10.md IN FULL (20 amendments, FINAL BUILD STAGES S0-S3, FINAL
   CONSOLIDATED GATE LIST). Base = current main at launch (src identical to fa9604d unless a lane merged first: then
   rebase-free, the base is whatever main is; say it in the report).
2. The patched libprojectM goes ONLY to $HOME/.local/opt/projectm-4.1.1-fbo1 (new folder; never overwrite ~/.local's
   existing projectM — other worktrees and Boris's app link it). Network fetch of the 4.1.1 source (commit 03aa8a7) is
   allowed; no sudo, no brew install / upgrade, no system path writes.
3. BORIS QUESTIONS: Q1 only if both arms < 58 fps at 4K (default keep full sharpness); Q2 filled with measured numbers by
   Harmony after the merge (default fine). F1-F4 filed by Harmony.
4. H1-H4 (handoff to bf9b) are relayed by Harmony into bf9b's adoption — not this lane's work beyond what the ruling says.
5. FENCE (concurrent BUILD lanes): mkvidx (VideoPlayer / GopCache / video probes / rendering.md / pitfalls 62 + 64),
   ui (VideoInfo, DeckTabRow / DeckView / MainComponent rename + Finder, ApiServer test routes, ClipInspector),
   bf2 (analysis / beat clock / FeatureBus snapshot ring / AppSettings venues / ApiServer + OSC sync routes). Your files:
   src/sources/ProjectMSource.*, the CMake projectM lookup, tests/test_projectm_canvas_gl + fixtures, .harmony/probe-milkdrop.py,
   docs. Renderer.cpp only where the ruling names it. Small appended hunks in shared files; whoever merges second rebases.
6. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not start is his (s-rta-1002b
   incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses, stop the batch and release.
   MERGE by Harmony.
