# Scout dossier — Syphon output server integration (Renderer)

**Scout:** repo recon, read-only. **Date:** 2026-08-02. **HEAD:** `d4ac223` (branch `main`).
**Scope:** what a builder needs to publish the final composited frame as a Syphon GL server.

---

## HEADLINE — this is not greenfield

**A complete Syphon output path already exists at HEAD and is wired end to end.** VERIFIED:

| Piece | Location |
|---|---|
| Obj-C++ server wrapper | `src/output/SyphonOutput.h:21-54`, `src/output/SyphonOutput.mm:1-154` |
| `SyphonServer` create (CGL) | `src/output/SyphonOutput.mm:40-42` |
| `publishFrameTexture` call | `src/output/SyphonOutput.mm:55-59` |
| Renderer blit-FBO + publish fn | `src/render/Renderer.h:266-271`; `src/render/Renderer.cpp:1784-1835` |
| Per-frame hook | `src/render/Renderer.cpp:658-662` |
| Init on GL thread | `src/render/Renderer.cpp:103-108` |
| Teardown on context close | `src/render/Renderer.cpp:706-711` |
| Owner + wiring | `src/MainComponent.h:360`; `src/MainComponent.cpp:420` |
| Menu toggle | `src/ui/MenuBarModel.h:98`; `src/ui/MenuBarModel.cpp:151-152`; `src/MainComponent.cpp:4409-4415` |
| CMake option / sources / framework | `CMakeLists.txt:71-74, 340-345, 387-396` |

The builder's job is therefore **verify + fix + enable**, not **write**. The single blocking
defect is in section 4A below (`-F` framework search path — silent no-op).

---

## 1. Composited output

**Where the final frame lives.** There is no dedicated "final composite FBO member." The final
image is rendered into **JUCE's own default framebuffer**, whose id is read each frame at
`src/render/Renderer.cpp:373-374`:

```cpp
GLint defaultFBO = 0;
glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
```

VERIFIED — **this is not guaranteed to be 0.** JUCE may render into its own FBO, so the builder
must use this captured `defaultFBO` value, never a hardcoded `0`, as the blit read source.

**The image occupies a letterboxed sub-rect, not the whole framebuffer.** Computed at
`src/render/Renderer.cpp:400-406`:

```cpp
float fitScale = std::min(scaleX, scaleY);
float vpW = aspectW * fitScale;   float vpH = aspectH * fitScale;
float vpX = (compW - vpW) * 0.5f; float vpY = (compH - vpH) * 0.5f;
```

Publishing the full framebuffer would include the black letterbox bars. The existing code
correctly publishes only `[vpX, vpY, vpW, vpH]` (`src/render/Renderer.cpp:1823-1827`).

**Composite completion point.** VERIFIED, in order within `renderOpenGL()`:

1. `src/render/Renderer.cpp:475` — `applyCompTransform(defaultFBO, vpX, vpY, vpW, vpH)`; the last
   *compositing* stage (composition-level position/scale/rotation).
2. `src/render/Renderer.cpp:576-597` — the master-output-level dim pass; viewport set at
   `:576-578`, final `quad_.draw()` at `:594`. **This is the last draw that touches final pixels.**
3. `src/render/Renderer.cpp:661-662` — the Syphon publish gate. Correctly placed after both.

**Texture target: `GL_TEXTURE_2D`** (not `GL_TEXTURE_RECTANGLE`). VERIFIED — the publish texture is
allocated `GL_TEXTURE_2D` / `GL_RGBA8` at `src/render/Renderer.cpp:1792-1802`, and the Obj-C side
passes `textureTarget:GL_TEXTURE_2D` at `src/output/SyphonOutput.mm:56`. Consistent.

**Dimensions source.** VERIFIED, `src/render/Renderer.cpp:361-370`:

- Base: target component size × `glContext_.getRenderingScale()` → **physical (retina backing-store)
  pixels**, `:362-364`.
- Overridden by locked resolution when `lockedWidth_/lockedHeight_ > 0`, `:367-370`
  (set via `Renderer::setLockedResolution`, `src/render/Renderer.h:207-211`).
- The published size is `vpW × vpH` (the fitted rect), **not** `renderW × renderH`
  (`src/render/Renderer.cpp:1813-1818`).

**Flip: the final image is NOT flipped — standard GL bottom-left origin.** VERIFIED by
triangulation:

- The default FBO is bottom-left origin — proven by `processPendingCapture`, which must invert rows
  to write a PNG: comment at `src/render/Renderer.cpp:1592` ("flip vertically: GL origin is
  bottom-left") and the inverting loop at `:1594-1608`.
- `publishSyphonFrame` blits 1:1 with **no** inversion — src Y range `vpY → vpY+vpH` maps to dst
  `0 → h` in ascending order (`src/render/Renderer.cpp:1823-1827`), so the destination texture keeps
  bottom-left origin.
- Therefore `flipped:NO` at `src/output/SyphonOutput.mm:59` is **correct as written**. Do not "fix" it.

---

## 2. Hook point

**Already implemented** at `src/render/Renderer.cpp:658-662`:

```cpp
if (syphonOutput_ != nullptr && syphonOutput_->isEnabled() && syphonOutput_->isInitialized())
    publishSyphonFrame(static_cast<GLuint>(defaultFBO), vpX, vpY, vpW, vpH);
```

Position VERIFIED: after the video-recorder submit (`:654-656`), before `processPendingCapture`
(`:665`), and it is the last statement group before `renderOpenGL()` returns at `:666`.

- **Context current?** Yes. This is inside `Renderer::renderOpenGL()`, the
  `juce::OpenGLRenderer` override (`src/render/Renderer.h:67`) registered via
  `glContext_.setRenderer(this)` (`src/render/Renderer.cpp:26`). JUCE makes the context current
  before invoking it.
- **Thread:** JUCE's dedicated GL worker thread ("OpenGL Renderer"), not the message thread.
  Continuous repainting is on (`src/render/Renderer.cpp:27`) and component painting is disabled
  (`:28`) — so the GL thread runs **without the MessageManager lock**.
- **Before or after swap?** Before. JUCE swaps buffers after `renderOpenGL()` returns, so the
  publish reads the just-rendered back buffer. Correct for Syphon (publishing post-swap would race
  the next frame).
- `publishSyphonFrame` restores the default framebuffer binding at `src/render/Renderer.cpp:1831`
  before returning, so the subsequent `processPendingCapture` `glReadPixels` still reads the right
  target. VERIFIED — this ordering dependency is real; don't move the publish after the capture.

---

## 3. Context access (CGLContextObj)

VERIFIED chain, no new bridging code needed:

1. `src/render/Renderer.cpp:108` — `syphonOutput_->init(glContext_.getRawContext());`
2. `juce::OpenGLContext::getRawContext()` returns `void*`
   (`build/_deps/juce-src/modules/juce_opengl/opengl/juce_OpenGLContext.cpp:1416-1418`,
   declared `juce_OpenGLContext.h:307`).
3. On macOS the native backend returns an **`NSOpenGLContext*`**:
   `build/_deps/juce-src/modules/juce_opengl/native/juce_OpenGL_mac.h:133` —
   `NSOpenGLContext* getRawContext() const noexcept { return renderContext; }`
4. Bridged back at `src/output/SyphonOutput.mm:99` — `(__bridge NSOpenGLContext*)nsOpenGLContext`.
5. `CGLContextObj` extracted at `src/output/SyphonOutput.mm:41` — `context:[ctx CGLContextObj]`.

**Obj-C++ is already compiled into the app target.** VERIFIED:

- Languages enabled at `CMakeLists.txt:2` — `project(AudioDNA ... LANGUAGES C CXX OBJC OBJCXX)`.
- Source added unconditionally on APPLE at `CMakeLists.txt:340-345`:
  ```cmake
  if(APPLE)
      target_sources(AudioDNA PRIVATE src/output/SyphonOutput.mm)
  endif()
  ```
  Note the `.mm` is added **regardless of `AUDIODNA_BUILD_SYPHON`** — it always compiles, and
  self-disables internally. Confirmed present in `build/compile_commands.json` (one entry, compiled
  `-x objective-c++ -std=c++2a`) even though the current build dir is configured Syphon-OFF.
- The header is listed separately at `CMakeLists.txt:329` under the comment
  `# Output / texture sharing (P22.1-P22.5)`.
- `SyphonOutput.mm` is currently the **only** `.mm` in the target (VERIFIED by source-list scan).

---

## 4. CMake patterns

### 4A. BLOCKING DEFECT — `/Library/Frameworks` is not on the compile-time framework search path

**This is the reason a builder would see "everything is wired but no Syphon server appears."**

The framework block, `CMakeLists.txt:387-396`:

```cmake
if(APPLE AND AUDIODNA_BUILD_SYPHON)
    find_library(SYPHON_FRAMEWORK Syphon PATHS /Library/Frameworks)
    if(SYPHON_FRAMEWORK)
        target_link_libraries(AudioDNA PRIVATE ${SYPHON_FRAMEWORK})
        target_compile_definitions(AudioDNA PRIVATE AUDIODNA_HAS_SYPHON=1)
    else()
        message(WARNING "Syphon.framework not found in /Library/Frameworks — Syphon disabled")
    endif()
endif()
```

It sets a **link** path and a **define**, but never a compile-time framework search path (`-F`).
Meanwhile `src/output/SyphonOutput.mm:10-17` gates the entire implementation on a *compile-time*
probe:

```cpp
#if __has_include(<Syphon/Syphon.h>)
#define AUDIODNA_HAS_SYPHON 1
#import <Syphon/Syphon.h>
...
#else
#define AUDIODNA_HAS_SYPHON 0
#endif
```

**VERIFIED empirically on this machine (2026-08-02):**

- The project's compile line for `SyphonOutput.mm` contains **no `-F` flag and no `-isysroot`**
  (extracted from `build/compile_commands.json`).
- `clang++ -v -E -x objective-c++` prints exactly one framework directory —
  `…/MacOSX.sdk/System/Library/Frameworks`. The real `/Library/Frameworks` is absent.
- Root cause identified: `<sysroot>/Library/Frameworks` **is** a default search dir (proved by
  running the same probe with `-isysroot /`, which then lists both `/System/Library/Frameworks` and
  `/Library/Frameworks`). But with the default SDK sysroot it resolves to
  `…/MacOSX.sdk/Library/Frameworks`, which does not exist — clang reports
  `ignoring nonexistent directory ".../MacOSX.sdk/Library/Frameworks"`.
- Therefore `__has_include(<Syphon/Syphon.h>)` is **false** even when
  `/Library/Frameworks/Syphon.framework` is installed.

**Failure mode (INFERRED from the verified facts above — cannot be executed here because
`/Library/Frameworks/Syphon.framework` is NOT installed on this machine):**

1. `find_library` succeeds (CMake searches `PATHS` explicitly), framework gets linked, and
   `-DAUDIODNA_HAS_SYPHON=1` lands on the command line.
2. Compiling the `.mm`, `__has_include` fails → line 16 does `#define AUDIODNA_HAS_SYPHON 0`,
   **redefining the command-line `=1` with a different value** → `-Wmacro-redefined` warning.
3. No `-Werror` anywhere (`cmake/CompilerWarnings.cmake:12-26` lists no `-Werror`; applied at
   `CMakeLists.txt:425`) → **build succeeds with only a warning.**
4. Every function compiles its disabled branch: `init()` takes `#else` and never sets `impl_`
   (`src/output/SyphonOutput.mm:105-108`), printing
   `"[Syphon] Not available (framework not installed)"` — the sole runtime clue.
5. `isInitialized()` stays false (`src/output/SyphonOutput.h:45`), so the gate at
   `src/render/Renderer.cpp:661` never fires. The menu item still ticks on
   (`isEnabled()` is an independent atomic), so **the UI lies**: toggle reads ON, nothing publishes.

**Fix direction for the builder** (design call is theirs): add the framework search path to
*compile* options, e.g. `target_compile_options(AudioDNA PRIVATE -F/Library/Frameworks)` (or
`-iframework`), derived from the `find_library` result rather than hardcoded. Also resolve the
**double definition** of `AUDIODNA_HAS_SYPHON` — CMake sets it *and* the `.mm` sets it; they should
not both own that symbol. Verify success by the absence of the "[Syphon] Not available" line and by
a client (MadMapper/VDMX/Syphon Recorder) seeing the server.

**Prerequisite:** `Syphon.framework` is **not installed** on this machine — `/Library/Frameworks`
contains only `CoreRepairCore.framework`, `CoreRepairKit.framework`, `iTunesLibrary.framework`
(VERIFIED). The builder must install it before any of this can be tested.

### 4B. FetchContent pin style at HEAD

The gotcha at `.harmony/gotchas.md:16` reads, verbatim:

> **Rule:** Always pin FetchContent GIT_TAG to a specific SHA or release tag, with GIT_SHALLOW TRUE.
> Match existing pattern in CMakeLists.txt.

Established pattern, `CMakeLists.txt:27-33` (release tag):

```cmake
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)
```

Other pins, all conforming: `httplib` → `v0.18.3` (`CMakeLists.txt:48-54`); `AbletonLink` →
`Link-3.1.2` (`:38-44`); `melatonin_inspector` → SHA `cd25631` (`:62-68`). **Zero uses of
`origin/main` at HEAD** — VERIFIED.

Note: Syphon itself is **not** fetched via FetchContent — it is a prebuilt system framework located
with `find_library`. If the builder switches to vendoring Syphon via FetchContent, the pin rule
applies.

### 4C. Target, frameworks, and where sources go

- **Target name: `AudioDNA`** (product name "Audio-DNA"), created with `juce_add_gui_app` at
  `CMakeLists.txt:77-86`.
- **macOS framework linking pattern:** `find_library(... PATHS ...)` → guard on the result →
  `target_link_libraries(AudioDNA PRIVATE ${VAR})` → `target_compile_definitions` feature flag.
  Only example at HEAD is the Syphon block itself (`CMakeLists.txt:387-396`). JUCE links its own
  system frameworks internally; the app's other libs go through `AUDIODNA_LIBS`
  (`CMakeLists.txt:405-423`).
- **Where new sources go:** the single `target_sources(AudioDNA PRIVATE ...)` list at
  `CMakeLists.txt:88-330`, grouped by subsystem with comment headers; the output/texture-sharing
  group is at `:328-329`. Conditional sources use their own `target_sources` block after it
  (test server `:332-338`, Syphon `.mm` `:340-345`).
- **Optional-feature pattern:** `option(AUDIODNA_BUILD_X ... OFF)` + conditional sources +
  `AUDIODNA_HAS_X=1` define. `AUDIODNA_BUILD_SYPHON` is declared at `CMakeLists.txt:71-74` and
  **defaults OFF** — so stock builds have no Syphon.
- Per-file/target warning relaxations for Obj-C++ bridge casts already exist target-wide at
  `CMakeLists.txt:501-508` (`-Wno-old-style-cast -Wno-conversion -Wno-sign-conversion`).

---

## 5. Output-window interplay

**Publish from the MAIN Renderer, not the OutputWindow.** Already correct at HEAD — Syphon is wired
only to the main renderer (`src/MainComponent.cpp:420`, via
`previewPanel_.getRenderer()`).

VERIFIED facts a builder must respect:

- `OutputWindow` owns a **second, independent** `juce::OpenGLContext` +
  `OutputRenderer : juce::OpenGLRenderer` (`src/ui/OutputWindow.h:17, 42`), attached at
  `src/ui/OutputWindow.cpp:276`.
- **There is no context sharing.** `setNativeSharedContext` (available in vendored JUCE at
  `build/_deps/juce-src/modules/juce_opengl/opengl/juce_OpenGLContext.h:132`) is **never called** in
  `src/`; no share-group setup exists in either `attachTo`. A GL texture name from one context is
  therefore meaningless in the other — **do not attempt to publish the output window's texture from
  the main context, or vice versa.**
- The two renderers do **not** produce the same image. `OutputRenderer::renderOpenGL()`
  (`src/ui/OutputWindow.cpp:66-148`) early-outs unless a still image is loaded (`:94-100`) and draws
  only `effectChain_.render(...)` (`:143-147`). It has **no** deck/layer compositing, no procedural
  sources, no comp transform, no deck transitions — all of which are main-Renderer-only
  (`src/render/Renderer.cpp:410-500`). The authoritative composite is the main Renderer's.
- Threading hazard the builder inherits (INFERRED, from directly-read code): `EffectChain` is shared
  by reference across both GL threads (`src/ui/OutputWindow.h:45`; passed at
  `src/MainComponent.cpp:2474-2477`) while holding raw per-context GL names
  (`src/effects/EffectChain.h:130-131`). This is a pre-existing latent bug, **out of scope** for
  Syphon work — but it means a builder must not "simplify" by routing Syphon through the shared
  chain or the output context.

---

## 6. Shutdown

**GL-side teardown hook already exists** — `src/render/Renderer.cpp:706-711`, inside
`openGLContextClosing()`:

```cpp
if (syphonOutput_ != nullptr)
    syphonOutput_->shutdown();
if (syphonFBO_ != 0) { glDeleteFramebuffers(1, &syphonFBO_); syphonFBO_ = 0; }
if (syphonTexture_ != 0) { glDeleteTextures(1, &syphonTexture_); syphonTexture_ = 0; }
```

VERIFIED mechanics:

- Trigger on quit: `src/MainComponent.cpp:1759` — `previewPanel_.getRenderer().detach();`, called
  explicitly and early in `~MainComponent` (`:1729`), with rationale comments at `:1731-1741`
  (servers stop first) and `:1749-1758` (detach before UI teardown).
- `Renderer::detach()` is just `glContext_.detach()` (`src/render/Renderer.cpp:32-35`).
- JUCE makes the **context current before** invoking the closing callback:
  `OpenGLContext::detach()` → `Attachment::detach()` → `CachedImage::stop()` → `pause()`, which does
  `ScopedContextActivator activator; activator.activate(context);`
  (`build/_deps/juce-src/modules/juce_opengl/opengl/juce_OpenGLContext.cpp:201-202`) before
  `context.renderer->openGLContextClosing();` (`:205`). Synchronous, on the caller's thread.
  This is what makes the `[SyphonServer stop]` call at `src/output/SyphonOutput.mm:73` legal — it
  needs a live CGL context.
- **Canonical teardown order** (`.harmony/HANDOFF.md:77-78`): *"servers stop → GL detach →
  everything else (~MainComponent)"*.
- Belt-and-braces: `SyphonOutput::~SyphonOutput()` also calls `shutdown()`
  (`src/output/SyphonOutput.mm:87-90`), and `shutdown()` is idempotent (null-guards `impl_` at
  `:143`). Since `syphonOutput_` is a **by-value member of MainComponent**
  (`src/MainComponent.h:360`) destroyed long after the GL detach at `MainComponent.cpp:1759`, the
  destructor path is a no-op in the normal quit sequence. VERIFIED — no double-stop.

Rules a builder must preserve (from `.harmony/scout-shutdown-sigbus.md:47-53` and the code):
release GL handles in `openGLContextClosing()`, **never** in a destructor; do not reorder
`MainComponent` members or add manual dtor teardown; context close/create cycles happen at runtime
(preview hide/resize), so `newOpenGLContextCreated()` must re-init GL handles idempotently — note
`src/render/Renderer.cpp:103-108` re-inits Syphon on every context creation, and
`SyphonOutput::init()` self-guards by calling `shutdown()` first if `impl_` is already set
(`src/output/SyphonOutput.mm:95-96`). VERIFIED correct for the re-create cycle.

---

## 7. Toggle surface (note only — no design)

**Menu (exists).**
- Enum `kOutputSyphon` in the Output block, `src/ui/MenuBarModel.h:98` (Output range starts
  `kOutputDisabled = 1600` at `:90`; Shortcuts starts `1700` at `:101` — so there is headroom in the
  Output range).
- Item construction with tick state: `src/ui/MenuBarModel.cpp:151-152`.
- Tick-state callback declared `src/ui/MenuBarModel.h:132-134`, wired
  `src/MainComponent.cpp:1525`.
- Handler: `src/MainComponent.cpp:4409-4415` — flips the atomic; comment notes "Default OFF each
  boot."

**REST API (does NOT exist for Syphon).** Registration pattern is a flat list of lambdas in
`ApiServer::setupRoutes`, `src/api/ApiServer.cpp:109-203`, e.g.
`server_.Post("/api/snapshot", [this](const httplib::Request& req, httplib::Response& res) {...});`
(`:147`). A grep for `syphon` across `src/api/` returns nothing — **no endpoint exists.** Note the
house rule at `.harmony/gotchas.md` (2026-05-19): HTTP handlers that touch the renderer need a
`juce::Thread::sleep(100)` so the GL thread can process before the response returns.

**Settings persistence: none.** VERIFIED — no `PropertiesFile`, `ApplicationProperties`, or
`getUserSettings` anywhere in `src/`. Session state persists only via `Composition::toVar/fromVar`
(`src/model/Composition.h:162, 252, 389, 399`), which has no Syphon field. So the toggle is
runtime-only and defaults to **false** each boot (`src/output/SyphonOutput.h:52`,
`std::atomic<bool> enabled_{false}`). Any "remember Syphon on" behaviour is net-new work with no
existing pattern to copy.

---

## 8. Tests

- **188 ctest tests** across **16 executables**, all registered via `catch_discover_tests` in
  `tests/CMakeLists.txt` (one call per target at `:24, 31, 38, 48, 58, 65, 106, 139, 159, 179, 209,
  241, 276, 302, 331, 360`). Baseline corroborated at `.harmony/HANDOFF.md:71-72`.
- **No test target compiles `Renderer.cpp` or `CompositorEngine.cpp`.** VERIFIED — zero hits for
  those filenames in `tests/CMakeLists.txt`. Stated as deliberate in
  `tests/test_renderer_source_confinement.cpp:33-36`: *"no target links CompositorEngine.cpp or
  Renderer.cpp for exactly this reason."*
- **No test creates a GL context; everything is headless.** `tests/test_compositor.cpp:9-11` says
  full GL compositing tests "require a GL context which is not available in unit tests" and tests the
  data model instead. `tests/test_renderer_source_confinement.cpp:31-52` re-implements the
  *confinement pattern* in a stand-in `OwnerThreadMap` (`:54+`) and is explicit at `:48-52` that it
  "does NOT exercise Renderer.cpp's actual code." Only `test_mapping_engine` and `test_routing_engine`
  compile any `render/` sources (`ShaderManager/TextureManager/LUTLoader/FullscreenQuad`,
  `tests/CMakeLists.txt:114-117, 251-254`) and link `juce::juce_opengl` for symbols only.
- **Implication for the builder: `publishSyphonFrame` is not unit-testable at HEAD.** There is no
  headless-GL harness to extend. Following house precedent, a new ctest would have to mirror the
  *mechanism* (e.g. enable/disable gating logic, FBO size-change invalidation in `ensureSyphonFBO`)
  rather than drive real GL.
- **End-to-end verification is manual or via the Eyes harness**, which needs the built app **and a
  display**: `tests/visual/conftest.py:47-74` resolves
  `build/AudioDNA_artefacts/Release/Audio-DNA.app/...` and skips if absent; requires
  `-DAUDIODNA_BUILD_TEST_SERVER=ON` (`conftest.py:67`). `tests/visual/vj_controller.py:65-74` stops
  the app with SIGTERM, so the harness also exercises the graceful-quit path from section 6.
- Practical acceptance test for Syphon: run the app, toggle Output ▸ Syphon Output, and confirm a
  client (Syphon Recorder / MadMapper / VDMX) sees the "Audio-DNA" server with correct orientation
  and no letterbox bars. Also confirm zero `.ips` crash reports after quit with Syphon enabled.

---

## Risks for the builder

1. **(Highest) The `-F` gap makes Syphon a silent no-op** — section 4A. Everything builds, links,
   and toggles ON while nothing is ever published. The only signal is one stderr line. Verify the
   `AUDIODNA_HAS_SYPHON` value that actually reaches the compiler before debugging anything else.
2. **`Syphon.framework` is not installed** on this machine (`/Library/Frameworks` has no
   `Syphon.framework`) and `AUDIODNA_BUILD_SYPHON` defaults OFF (`CMakeLists.txt:73`). Nothing can be
   validated until both are addressed.
3. **`defaultFBO` is not 0** and the composite is a letterboxed sub-rect — hardcoding `0` or
   publishing full-framebuffer are the two easy mistakes (section 1).
4. **Do not "fix" `flipped:NO`** — verified correct (section 1). Changing it inverts output.
5. **Ordering inside `renderOpenGL` is load-bearing**: the publish must stay after the master-level
   pass and before `processPendingCapture`, and must restore the FBO binding
   (`src/render/Renderer.cpp:1831`).
6. **Pre-existing, out-of-scope:** the shared-`EffectChain`-across-two-GL-contexts hazard
   (section 5). Don't let Syphon work get entangled with it.
