# Scout Dossier — Syphon SDK Integration (C++20 / JUCE / OpenGL / CMake+FetchContent)

**ERRATUM (2026-08-02, Reviewer-caught):** §6.5's "`GIT_SHALLOW TRUE` cannot fetch an arbitrary commit SHA [VERIFIED]" is wrong as a blanket claim — this repo's own `melatonin_inspector` pin (raw SHA `cd25631` + `GIT_SHALLOW TRUE`, CMakeLists.txt) fetches a genuinely shallow checkout on disk (`git rev-parse --is-shallow-repository` → true). It depends on the server supporting `uploadpack.allowReachableSHA1InWant`/`allowAnySHA1InWant` (GitHub does); it is not a hard CMake/git limitation. The Syphon pin keeps `GIT_SHALLOW FALSE` anyway as a conservative choice, not because shallow is impossible.

Scout: web-research scout · Date: 2026-08-02 · Tools: WebSearch + WebFetch only.

**Label key used throughout:**
- **[VERIFIED]** — read directly from primary source (upstream source file, GitHub API, official docs). URL given.
- **[INFERRED]** — reasoned from verified facts, not directly stated by a source. Reasoning given.
- **[ASSUMED]** — not checked; flagged so a builder checks it before relying on it.

---

## 0. RECOMMENDED INTEGRATION (read this first)

### 0.1 The pin — do NOT pin the latest release tag

**Pin `main` at commit `71351d4b484cd2d1917867f7846a5cdca724552d` (2025-10-06).**

The headline finding of this research: **the class you asked about, `SyphonOpenGLServer`, does not exist in any tagged release.** The latest *release* is tag `5` ("Syphon SDK 5"), published **2019-03-02** — over seven years old. At that tag the OpenGL server class is named `SyphonServer`, and `SyphonOpenGLServer.h`, `SyphonMetalServer.h` and `Syphon.docc` are **absent from the tree**. [VERIFIED — GitHub contents API at `ref=5` vs `ref=main`]

So there is a real fork in the road, and it must be a deliberate choice:

| Option | Pin | API you get | Verdict |
|---|---|---|---|
| Latest release tag | `5` = `9f16c62da2e547f30317fc117191fcf52d860387` | legacy `SyphonServer` only | Reproducible but 7 years stale; no Metal-client interop story, no modulemap, no DocC |
| `main` HEAD | `71351d4b484cd2d1917867f7846a5cdca724552d` | `SyphonOpenGLServer` + `SyphonMetalServer` | **Recommended** — the only way to get the requested API |

Pin the **full 40-char SHA**, never `main`, so FetchContent is reproducible. Upstream is low-traffic (267 commits total, last commit 2025-10-06 as of this research), so drift risk is low but the SHA pin still removes it entirely. [VERIFIED — commits API]

```cmake
include(FetchContent)
FetchContent_Declare(
  syphon
  GIT_REPOSITORY https://github.com/Syphon/Syphon-Framework.git
  GIT_TAG        71351d4b484cd2d1917867f7846a5cdca724552d  # main, 2025-10-06
  GIT_SHALLOW    FALSE   # required: GIT_SHALLOW cannot fetch an arbitrary SHA
)
FetchContent_MakeAvailable(syphon)   # no CMakeLists upstream -> Populate-only; see 2.4
```

Note: upstream ships **no `CMakeLists.txt`**, so `FetchContent_MakeAvailable` will populate but add no targets. That is expected and is the crux of section 2. [VERIFIED — root listing at `ref=main` contains no CMakeLists.txt]

### 0.2 The CMake approach — compile the OpenGL subset directly into the app target

**Recommended: approach (a) — add Syphon's OpenGL-path `.m` sources to your own target, with ARC enabled, excluding all Metal files.** This keeps the entire build inside CMake, needs no Xcode.app, and avoids embedding + codesigning a nested `.framework` in your `.app`.

Two facts I verified make this viable, and both are the opposite of what you'd guess:

1. **The codebase is ARC, not MRC.** You need `-fobjc-arc` (ON), *not* `-fno-objc-arc`. See §2.1 — this directly answers the question in the brief.
2. **The OpenGL path has zero Metal coupling at the file level.** `SyphonServerBase.m` imports only `SyphonServerBase.h`, `SyphonServerConnectionManager.h`, `SyphonPrivate.h`, `<os/lock.h>` — no Metal anywhere. So the Metal renderer is cleanly excludable. See §2.2.

Excluding Metal is not optional cosmetics — it is **required** for direct compilation. `SyphonServerRendererMetal.m` loads its shaders via `[device newDefaultLibraryWithBundle:[NSBundle bundleForClass:[self class]]]`. Compiled into your app, `bundleForClass:` resolves to *your app bundle*, so Syphon's Metal shaders would have to live in your app's `default.metallib` — colliding with your own. [VERIFIED — source] Dropping the Metal files sidesteps this entirely.

**Fallback trigger:** if you later need a Syphon *Metal client*, or the GL-only source list proves to have hidden coupling, switch to approach (c) (xcodebuild the real framework). See §2.4 for both, fully specified.

### 0.3 The publish call, one line

```objc
[server publishFrameTexture:texID textureTarget:GL_TEXTURE_2D
                imageRegion:NSMakeRect(0, 0, w, h)
          textureDimensions:NSMakeSize(texW, texH) flipped:NO];
```
`GL_TEXTURE_2D` and `GL_TEXTURE_RECTANGLE_EXT` are the only two legal targets. [VERIFIED — header doc comment]

### 0.4 Top 3 pitfalls

1. **App Sandbox breaks Syphon entirely.** Sandboxed processes cannot receive the `userInfo` payload of distributed notifications, which is exactly how servers are announced. Non-sandboxed distribution only; Mac App Store is effectively out. (§6.1)
2. **Publishing without the CGL context current / unlocked.** Syphon deliberately does *not* lock the context for you; JUCE renders on its own dedicated thread. Get this wrong and you get a server that announces but shows black. (§3.4, §6.2)
3. **Pinning tag `5` and finding `SyphonOpenGLServer` doesn't exist.** (§0.1)

### 0.5 Verification CLI verdict

**Feasible, and genuinely small — but it must spin a run loop.** A headless Obj-C CLI can list announced servers with no GL context, no Metal device, and no `NSApplication`. It cannot, however, read `.servers` and exit immediately: discovery is a request/reply broadcast over `NSDistributedNotificationCenter`, and replies only arrive while a run loop is running. Full working design in §5.

---

## 1. Canonical source, pin, and license

### 1.1 Repository

- Official: <https://github.com/Syphon/Syphon-Framework> — 514 stars, 93 forks, 267 commits, default branch **`main`**. [VERIFIED]
- Org home: <https://github.com/Syphon> · Project site: <https://syphon.info/> (`syphon.github.io` 301-redirects to `syphon.info`). [VERIFIED]
- Beware forks that rank in search: `anome/Syphon-Framework`, `eromanc/Syphon-Framework`. Neither is canonical. [VERIFIED — org ownership]

### 1.2 Tags and SHAs (complete, from the tags API)

<https://api.github.com/repos/Syphon/Syphon-Framework/tags> [VERIFIED]

| Tag | Commit SHA |
|---|---|
| `5` | `9f16c62da2e547f30317fc117191fcf52d860387` |
| `4` | `bece9b58c612b655f124cb4028e9019b5fcb4304` |
| `4-alpha-1` | `c4394b2339cf242138cf97a9889f9d71ece6f3a1` |
| `3` | `bb2d0be68781dd5c62f8d55a93432575f2614466` |
| `public-beta-2` | `dae55cb8721eb10ec646b40b28d610938e16baf4` |
| `public-beta-2-gc` | `677cddb45bb15ba2329ffc7bef1df15a0d1af2b0` |
| `public-beta-1` | `5fc1f2fe05bf711740f88c3281cf4ee3cd657c29` |

**Latest release = tag `5`, published `2019-03-02T22:04:33Z`**, release body: *"The fifth release of the Syphon framework for developers. This release adds support for Core Profile OpenGL… SyphonClient is now associated with a single CGL context on instantiation; SyphonServerDirectory's notifications now present the SyphonServerDirectory instance as the notification object, and provide the server dictionary as the associated user-info."* [VERIFIED — <https://api.github.com/repos/Syphon/Syphon-Framework/releases/latest>]

Release asset: `Syphon.SDK.5.zip`, 1,037,806 bytes, 26,220 downloads — <https://github.com/Syphon/Syphon-Framework/releases/download/5/Syphon.SDK.5.zip>. [VERIFIED — releases API] Whether that zip contains a *prebuilt* `Syphon.framework` binary alongside the sources is **[ASSUMED]** — the zip is binary and could not be inspected with the available tools. Size and the ofxSyphon/Cinder-Syphon convention of shipping a prebuilt `Syphon.framework` in a `lib/` folder both suggest it does, but do not confirm it.

**`main` HEAD:** `71351d4b484cd2d1917867f7846a5cdca724552d`, `2025-10-06T14:51:49Z`, "Merge pull request #103 from gxalpha/pixel-format". Recent commits are maintenance-grade: "Explicitly set pixel format type to 32BGRA" (2025-10-01), type annotations and a client-stall fix (2025-01-03). [VERIFIED — branches + commits API]

### 1.3 The tag-5 vs main API gap (the load-bearing finding)

Root of tree at `ref=5`: `SyphonServer.h` present; `SyphonOpenGLServer.h`, `SyphonMetalServer.h`, `Syphon.docc` **all absent**. [VERIFIED — contents API]

Root of tree at `ref=main` includes all of: `SyphonOpenGLServer.{h,m}`, `SyphonOpenGLClient.{h,m}`, `SyphonOpenGLImage.{h,m}`, `SyphonMetalServer.{h,m}`, `SyphonMetalClient.{h,m}`, `SyphonServerBase.{h,m}`, `SyphonClientBase.{h,m}`, `SyphonServerRendererCoreGL.{h,m}`, `SyphonServerRendererLegacyGL.{h,m}`, `SyphonServerRendererMetal.{h,m}`, `SyphonMetalShaders.metal`, `Syphon.modulemap`, `Syphon.docc/`, plus deprecated `SyphonServer.h` / `SyphonClient.h` / `SyphonImage.h` kept for source compatibility. [VERIFIED — contents API]

So: **code written against the old `SyphonServer` name still compiles on `main`** (deprecated headers retained), but code written against `SyphonOpenGLServer` **will not compile at tag `5`**. Pin `main`.

### 1.4 License — BSD **3-Clause**, with a caveat worth knowing

`License.txt` on `main` is **BSD 3-Clause ("New"/"Revised")**: the two redistribution clauses plus the non-endorsement clause. Copyright "2010 bangnoise (Tom Butterworth) & vade (Anton Marini)." [VERIFIED — <https://raw.githubusercontent.com/Syphon/Syphon-Framework/main/License.txt>]

The third clause verbatim:
> * Neither the name of the Syphon Project nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

**Caveat [VERIFIED]:** individual source files carry a *2-clause* variant of the same header — `SyphonOpenGLServer.h`, `SyphonServerDirectory.h` and others omit the non-endorsement clause, while `License.txt` includes it. Treat the repo as **BSD-3-Clause** (the stricter, project-level statement) and comply with all three clauses.

**Attribution obligations for embedding:**
1. Retain the copyright notice + conditions + disclaimer in any source redistribution.
2. **Reproduce the copyright notice, conditions, and disclaimer in documentation or other materials shipped with the binary** — this one bites for a GUI app: put the Syphon license text in an About box, an acknowledgements panel, or a bundled `licenses/` file. Shipping the binary with no attribution anywhere violates clause 2.
3. Do not use "Syphon Project" or contributor names to endorse/promote your product without written permission. (Descriptive factual use — "supports Syphon" — is not endorsement. [INFERRED] — standard BSD-3 reading, not legal advice.)

BSD-3 is permissive: no copyleft, no source-disclosure obligation, compatible with a closed-source commercial app. Direct compilation into your binary (approach (a)) is fine under it.

---

## 2. CMake integration — the decision that matters most

### 2.1 ARC vs MRC — the brief's assumption is inverted

**The Syphon codebase is ARC. You must compile it with `-fobjc-arc` ENABLED. Do not pass `-fno-objc-arc`.** [VERIFIED — four independent lines of evidence]

1. `Syphon.xcodeproj/project.pbxproj` sets **`CLANG_ENABLE_OBJC_ARC = "YES"`**. [VERIFIED]
2. `SyphonServerBase.m` `dealloc` — no `[super dealloc]`:
   ```objc
   - (void)dealloc
   {
       SYPHONLOG(@"Server deallocing, name: %@, UUID: %@", self.name, [self.serverDescription objectForKey:SyphonServerDescriptionUUIDKey]);
       // Don't call anything in the subclass, it has already been dealloc'd
       [self destroyBaseResources];
   }
   ```
   Under MRC, omitting `[super dealloc]` leaks and warns; under ARC, *calling* it is a hard compile error. Its absence across files is decisive.
3. `SyphonOpenGLClient.m` `dealloc` is `{ [self stop]; }` — no `-release` on any Obj-C object anywhere in the file.
4. The only retain/release calls in the OpenGL server are C-API calls on non-Obj-C types — `CGLRetainContext` / `CGLReleaseContext` on `CGLContextObj`, `CFRetain`/`CFRelease` on `IOSurfaceRef`. Those are required under ARC too and are *not* evidence of MRC.

Corroboration from a major downstream consumer: OBS Studio compiles its own Syphon-consuming sources with `target_compile_options(mac-syphon PRIVATE -fobjc-arc)`. [VERIFIED — <https://raw.githubusercontent.com/obsproject/obs-studio/master/plugins/mac-syphon/CMakeLists.txt>]

**Practical consequence for JUCE:** JUCE's own Obj-C++ is generally built without ARC. Do **not** flip ARC on globally. Apply `-fobjc-arc` per-source-file to the Syphon `.m` files only (`set_source_files_properties(... COMPILE_OPTIONS -fobjc-arc)`), or isolate Syphon in its own static library target. Mixing ARC and non-ARC translation units in one binary is fully supported by clang. [INFERRED — standard clang behaviour; the per-file mechanism is standard CMake]

### 2.2 The two other things that decide whether direct compilation works

**(i) The prefix header is mandatory.** `Syphon_Prefix.pch` defines the `SYPHONLOG(...)` macro used throughout the `.m` files, and Xcode injects it via `GCC_PREFIX_HEADER = "Syphon_Prefix.pch"` / `GCC_PRECOMPILE_PREFIX_HEADER = "YES"`. [VERIFIED — pbxproj + pch contents] Outside Xcode nothing injects it, so `SYPHONLOG` is undefined and every `.m` that logs fails to compile. Fix: pass `-include ${syphon_SOURCE_DIR}/Syphon_Prefix.pch`. The pch also does `#import <Cocoa/Cocoa.h>`. Note `SYPHONLOG` expands to nothing unless `DEBUG` is defined, so this costs you nothing at runtime in release. [VERIFIED — pch source]

**(ii) Public headers use framework-style imports; implementation files do not.** `SyphonOpenGLServer.h` does `#import <Syphon/SyphonServerBase.h>` and the umbrella `Syphon.h` imports all six public headers as `<Syphon/...>`. But `.m` files use quoted local imports (`#import "SyphonServerBase.h"`). [VERIFIED — both files read directly] So `<Syphon/...>` must resolve. Cheapest fix in CMake: stage the public headers into `${CMAKE_BINARY_DIR}/syphon_include/Syphon/` and add `syphon_include` to the include path. Do **not** include the umbrella `Syphon.h` from your code — it imports the Metal headers. Import `SyphonOpenGLServer.h` and `SyphonServerDirectory.h` directly.

**(iii) Metal must be excluded, and cleanly can be.** `SyphonServerRendererMetal.m` does:
```objc
NSBundle *bundle = [NSBundle bundleForClass:[self class]];
id<MTLLibrary> defaultLibrary = [device newDefaultLibraryWithBundle:bundle error:&error];
```
[VERIFIED] Compiled into an app rather than a framework, `bundleForClass:` returns your app bundle, so Syphon's `SyphonMetalShaders.metal` would have to be compiled into *your* `default.metallib`. That is the single strongest argument against naively adding all `.m` files. Excluding the Metal set is safe because `SyphonServerBase.m` has no Metal imports at all. [VERIFIED]

Exclude: `SyphonMetalServer.m`, `SyphonMetalClient.m`, `SyphonServerRendererMetal.m`, `SyphonMetalShaders.metal`, and don't import `Syphon.h`.

### 2.3 How other projects actually do it (real precedents, all verified)

| Project | Approach | Source |
|---|---|---|
| **OBS Studio** (`mac-syphon`) | Links a **prebuilt** framework: `find_library(SYPHON Syphon)` + `$<LINK_LIBRARY:FRAMEWORK,${SYPHON}>`; own sources built `-fobjc-arc`. Historically vendored Syphon as a git submodule, since moved to a prebuilt framework "instead of accessing private headers and sources directly". | [VERIFIED CMakeLists](https://raw.githubusercontent.com/obsproject/obs-studio/master/plugins/mac-syphon/CMakeLists.txt) |
| **ofxSyphon** (openFrameworks) | Ships a **prebuilt** `Syphon.framework` in `lib/`; user adds a Framework Search Path, links it, and adds a Copy Files phase to embed it. | <https://github.com/astellato/ofxSyphon> |
| **Cinder-Syphon** | Same pattern — prebuilt framework in `lib/`, plus a Copy Files build phase into the product's Frameworks folder. | <https://github.com/astellato/Cinder-Syphon> |
| **nozzle-spout-syphon** | Runs **xcodebuild on `Syphon.xcodeproj` before** CMake configure, then configures. | <https://github.com/nozzle-io/nozzle-spout-syphon> |
| **syphon-python** (cansik) | Vendors Syphon as a **git submodule** (`--recurse-submodules`) and builds it during `setup.py build`; wraps via PyObjC. Supports Metal + OpenGL server and client. | <https://github.com/cansik/syphon-python> |

**The candid summary: nobody found a healthy community CMake-native fork of Syphon-Framework.** Targeted searches for a CMake fork / vendored `CMakeLists.txt` returned nothing credible. Every serious consumer either links a prebuilt framework or shells out to `xcodebuild`. Treat "use the community CMake fork" (option (b) in the brief) as **not available** — that option is closed. [VERIFIED by absence across multiple searches; labeled [INFERRED] as a negative result, since absence of search evidence is weaker than presence]

### 2.4 The two viable approaches, fully specified

#### (a) RECOMMENDED — compile the OpenGL subset into your target

```cmake
FetchContent_Declare(syphon
  GIT_REPOSITORY https://github.com/Syphon/Syphon-Framework.git
  GIT_TAG        71351d4b484cd2d1917867f7846a5cdca724552d)
FetchContent_GetProperties(syphon)
if(NOT syphon_POPULATED)
  FetchContent_Populate(syphon)     # Populate only: upstream has no CMakeLists.txt
endif()

# Stage public headers so #import <Syphon/...> resolves.
set(SYPHON_INC ${CMAKE_BINARY_DIR}/syphon_include/Syphon)
file(MAKE_DIRECTORY ${SYPHON_INC})
file(COPY ${syphon_SOURCE_DIR}/ DESTINATION ${SYPHON_INC} FILES_MATCHING PATTERN "*.h")

add_library(syphon_gl STATIC
  ${syphon_SOURCE_DIR}/SyphonOpenGLServer.m
  ${syphon_SOURCE_DIR}/SyphonServerBase.m
  ${syphon_SOURCE_DIR}/SyphonServerConnectionManager.m
  ${syphon_SOURCE_DIR}/SyphonServerDirectory.m
  ${syphon_SOURCE_DIR}/SyphonServerRendererGL.m
  ${syphon_SOURCE_DIR}/SyphonServerRendererCoreGL.m
  ${syphon_SOURCE_DIR}/SyphonServerRendererLegacyGL.m
  ${syphon_SOURCE_DIR}/SyphonServerGLShader.m
  ${syphon_SOURCE_DIR}/SyphonServerGLVertices.m
  ${syphon_SOURCE_DIR}/SyphonGLShader.m
  ${syphon_SOURCE_DIR}/SyphonGLVertices.m
  ${syphon_SOURCE_DIR}/SyphonIOSurfaceImageCore.m
  ${syphon_SOURCE_DIR}/SyphonIOSurfaceImageLegacy.m
  ${syphon_SOURCE_DIR}/SyphonImageBase.m
  ${syphon_SOURCE_DIR}/SyphonOpenGLImage.m
  ${syphon_SOURCE_DIR}/SyphonMessaging.m
  ${syphon_SOURCE_DIR}/SyphonMessageQueue.m
  ${syphon_SOURCE_DIR}/SyphonMessageSender.m
  ${syphon_SOURCE_DIR}/SyphonMessageReceiver.m
  ${syphon_SOURCE_DIR}/SyphonCFMessageSender.m
  ${syphon_SOURCE_DIR}/SyphonCFMessageReceiver.m
  ${syphon_SOURCE_DIR}/SyphonPrivate.m
  ${syphon_SOURCE_DIR}/SyphonCGL.c
  ${syphon_SOURCE_DIR}/SyphonDispatch.c
  ${syphon_SOURCE_DIR}/SyphonOpenGLFunctions.c)

target_include_directories(syphon_gl PUBLIC
  ${CMAKE_BINARY_DIR}/syphon_include PRIVATE ${syphon_SOURCE_DIR})
target_compile_options(syphon_gl PRIVATE
  -fobjc-arc                                              # ARC ON - verified requirement
  -include ${syphon_SOURCE_DIR}/Syphon_Prefix.pch         # provides SYPHONLOG
  -DGL_SILENCE_DEPRECATION)
target_link_libraries(syphon_gl PUBLIC
  "-framework Cocoa" "-framework OpenGL" "-framework IOSurface" "-framework CoreVideo")
target_link_libraries(YourJuceApp PRIVATE syphon_gl)
```

**Caveat, stated plainly:** the source list above is my **[INFERRED]** GL-only set, derived from the verified root file listing and the verified absence of Metal imports in `SyphonServerBase.m`. I could not compile it. Expect **one iteration** of "add the missing `.m` the linker names" — the failure mode is loud (undefined symbols at link time), not silent, so it is cheap to converge. `-DGL_SILENCE_DEPRECATION` mirrors what upstream applies to its GL files. [VERIFIED — pbxproj]

**Why this over (c):** whole build stays in CMake/FetchContent with a hard SHA pin (your stated requirement); no Xcode.app dependency; no nested `.framework` to embed, re-sign, and get past notarization; static linkage so there's no `@rpath` to get wrong.

#### (c) FALLBACK — xcodebuild the real framework

Use this if you need a Syphon **Metal** client, or if (a) fights you.

```cmake
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/syphon-build/Release/Syphon.framework
  COMMAND xcodebuild -project ${syphon_SOURCE_DIR}/Syphon.xcodeproj
          -target Syphon -configuration Release
          CONFIGURATION_BUILD_DIR=${CMAKE_BINARY_DIR}/syphon-build/Release
  WORKING_DIRECTORY ${syphon_SOURCE_DIR}
  COMMENT "Building Syphon.framework via xcodebuild")
```
Then an `IMPORTED` target, `-F` search path, embed into `YourApp.app/Contents/Frameworks/`, and **re-sign** (`codesign -o runtime`) — see §6.4. The single Xcode target is named **`Syphon`**, product name `Syphon`, `SDKROOT = macosx`, `DEFINES_MODULE = YES`, `CLANG_ENABLE_MODULES = YES`, deployment target `$(RECOMMENDED_MACOSX_DEPLOYMENT_TARGET)`. [VERIFIED — pbxproj]

Costs: requires **full Xcode**, not just Command Line Tools; slower configure/build; you inherit the embed + codesign + notarize dance for a nested framework; and a **known upstream sharp edge — the Syphon Xcode build breaks when the path contains spaces** (reported against OBS builds). [VERIFIED — <https://obsproject.com/forum/threads/building-obs-from-source-in-mac-10-13-syphon-framework-cant-deal-with-space.120476/>] `/Users/boriskarpman/projects/RealTimeAudio` has no spaces, so this is not an active problem, but it constrains where the build tree may live.

### 2.5 JUCE-specific: `juce_sharedtexture` — evaluate, then probably reject

<https://github.com/benkuper/juce_sharedtexture> — "A JUCE Module to allow Spout/Syphon texture sharing", by benkuper (author of Chataigne). Module ID `juce_sharedtexture`, dependencies `juce_core, juce_opengl`, vendor benkuper, v1.0.0. It is the only purpose-built JUCE↔Syphon module found. [VERIFIED — module header]

**Two reasons to reject it for this project:**

1. **License: GPLv3.** Declared in the module block and a 35 KB `LICENSE` file. [VERIFIED] For a proprietary/commercial app this is a hard blocker — far more restrictive than Syphon's own BSD-3. Using upstream Syphon directly keeps you on BSD-3.
2. **It doesn't solve the build problem anyway.** `juce_sharedtexture.mm` is 743 bytes and simply does `#import <Syphon/Syphon.h>` (plus `<AppKit/NSOpenGL.h>`, `"juce_sharedtexture.h"`, `"SharedTexture.cpp"`). [VERIFIED] Its `libs/` directory is **empty** and there is no `.gitmodules` (404). [VERIFIED] So it still requires *you* to supply a built `Syphon.framework` on the framework search path — it assumes approach (c) and hands the hard part back to you.

**Verdict: read it as a reference for the JUCE render-thread plumbing, do not depend on it.** Health: 11 stars, 4 forks, 82 commits — small but real, and the author shipped it in Chataigne (macOS Syphon support updated July 2024). [VERIFIED]

Useful JUCE precedent from the same author's forum thread — <https://forum.juce.com/t/openglrenderer-without-a-component-complete-offscreen-render-for-spout-syphon/22231>: JUCE requires a **visible, non-zero-size component** for `renderOpenGL()` to fire, so pure-offscreen rendering needs a workaround. JUCE dev `fr810` recommends attaching one `OpenGLContext` to the top-level component and dispatching rendering through that single context rather than creating multiple contexts. Another user hit severe stalls from continuous repainting locking the MessageManager, fixed with `setComponentPaintingEnabled(false)` + `setContinuousRepainting(false)`. [VERIFIED — thread]

---

## 3. Server API — exact usage

Header: `SyphonOpenGLServer.h` on `main`. Full text verified at <https://raw.githubusercontent.com/Syphon/Syphon-Framework/main/SyphonOpenGLServer.h>. `SyphonOpenGLServer : SyphonServerBase`. All signatures and doc quotes in this section are **[VERIFIED]** from that file.

### 3.1 Construction

```objc
- (instancetype)initWithName:(nullable NSString*)serverName
                     context:(CGLContextObj)context
                     options:(nullable NSDictionary<NSString *, id> *)options;
```
> "Creates a new server with the specified human-readable name (which need not be unique), CGLContext and options. The server will be started immediately. Init may fail and return `nil` if the server could not be started."
> "This method does not lock the CGL context. If there is a chance other threads may use the context during calls to this method, bracket it with calls to `CGLLockContext()` and `CGLUnlockContext()`."

`serverName` may be `nil`; it is "not required… but is usually used by clients in their UI to aid identification." `context` is "The `CGLContextObj` context that textures will be valid and available on for publishing." **Check for `nil`** — init is documented as failable.

**Getting the `CGLContextObj` from JUCE** — the concrete bridge, verified on both sides:
```objc
// juce::OpenGLContext::getRawContext() is declared `void* getRawContext() const noexcept;`
// and on macOS returns an NSOpenGLContext*.
NSOpenGLContext* nsCtx = (NSOpenGLContext*) glContext.getRawContext();
CGLContextObj   cgl   = (CGLContextObj) [nsCtx CGLContextObj];
```
[VERIFIED] Public declaration `void* getRawContext() const noexcept;` in <https://raw.githubusercontent.com/juce-framework/JUCE/master/modules/juce_opengl/opengl/juce_OpenGLContext.h>; the macOS native implementation declares `NSOpenGLContext* getRawContext() const noexcept { return renderContext; }` in `juce_OpenGL_mac.h`. JUCE itself derives the CGL context the same way — its `Locker` does `cglContext ((CGLContextObj) [nc.renderContext CGLContextObj])` then `CGLLockContext (cglContext)`.

Options (all `extern NSString * const`, all verified):
- `SyphonServerOptionIsPrivate` — `NSNumber` BOOL; server invisible to other Syphon users; you must hand `serverDescription` to clients yourself. Default `NO`.
- `SyphonServerOptionAntialiasSampleCount` — MSAA sample count; **only affects the `bindToDrawFrameOfSize:`/`unbindAndPublish` path**. Unsupported counts fall back to nearest; if MSAA is unsupported the key is ignored.
- `SyphonServerOptionDepthBufferResolution` — 16/24/32; again only meaningful for the bind/unbind path.
- `SyphonServerOptionStencilBufferResolution` — 1/4/8/16.

Pass `nil` for a plain GL server publishing an existing texture.

### 3.2 Publishing an existing texture (the primary path)

```objc
- (void)publishFrameTexture:(GLuint)texID
              textureTarget:(GLenum)target
                imageRegion:(NSRect)region
          textureDimensions:(NSSize)size
                    flipped:(BOOL)isFlipped;
```

Parameter semantics, verbatim from the header:
- `texID` — "The name of the texture to publish, which must be a texture valid in the CGL context provided when the server was created."
- `target` — "**`GL_TEXTURE_RECTANGLE_EXT` or `GL_TEXTURE_2D`**." Those two only. Note `GL_TEXTURE_RECTANGLE_EXT` == `GL_TEXTURE_RECTANGLE` == `0x84F5`.
- `region` — "The sub-region of the texture to publish." Publish the whole texture with `NSMakeRect(0, 0, w, h)`. Lets you publish a crop of a larger atlas/FBO.
- `size` — "**The full size of the texture**" — the allocated dimensions, which may exceed `region`.
- `isFlipped` — "Is the texture flipped?" i.e. you *describe* your texture's existing orientation; you are not requesting a flip. Syphon uses it to normalise orientation for clients. GL textures rendered into an FBO are typically bottom-left origin; pass `YES` when your content is vertically inverted relative to Syphon's expected orientation. [INFERRED on the practical polarity — the header states only "Is the texture flipped?"; settle it empirically against Simple Client, it is a one-line flip either way.]

Semantics and guarantees, verbatim:
> "The texture is copied and can be safely disposed of or modified once this method has returned."
> "You should not bracket calls to this method with calls to ``bindToDrawFrameOfSize:`` and ``unbindAndPublish`` - they are provided as an alternative to using this method."
> "This method makes no changes to state in the caller's OpenGL context. In legacy OpenGL contexts other state changed is restored. In Core Profile OpenGL contexts work is conducted in a private shared context."
> "This method does not lock the server's CGL context. If there is a chance of other threads using the context during calls to this method, bracket it with calls to `CGLLockContext()` and `CGLUnlockContext()`, passing in the value of the server's context property as the argument."

Real-world call, from Syphon's own Jitter implementation [VERIFIED — <https://github.com/Syphon/Jitter/blob/master/Server/jit.gl.syphonserver/jit.gl.syphonserver/jit.gl.syphonserver.m>]:
```objc
[jit_gl_syphon_server_instance->syServer publishFrameTexture:texName
                                               textureTarget:texTarget
                                                 imageRegion:NSMakeRect(0.0, 0.0, width, height)
                                           textureDimensions:NSMakeSize(width, height)
                                                     flipped:flip];
```
with the server created as `[[SyphonServer alloc] initWithName:name context:CGLGetCurrentContext() options:nil]` — i.e. built from the *currently current* context. (That file uses the legacy `SyphonServer` name and MRC `[… release]`; it predates the OpenGL/Metal split.)

### 3.3 Alternative path — draw directly into the server's FBO

```objc
- (BOOL)bindToDrawFrameOfSize:(NSSize)size;
- (void)unbindAndPublish;
```
> "Binds an FBO for you to publish a frame of the given dimensions by drawing into the server's context (check it using the context property). **If YES is returned, you must pair this with a call to ``unbindAndPublish``** once you have finished drawing. **If NO is returned you should abandon drawing and not call ``unbindAndPublish``.**"
> `unbindAndPublish`: "Restores any previously bound FBO and publishes the just-drawn frame. **This method will flush the GL context (so you don't have to).**"
> "This method changes only the FBO binding in the caller's OpenGL context… In Core Profile OpenGL contexts the default (0) FBO is restored."

Choose this path if you want Syphon to own the render target (and you want the MSAA/depth/stencil options to apply). Choose `publishFrameTexture:` if you already render into your own FBO/texture — which is the normal JUCE case. **Never mix the two for one frame** (explicit in the header).

Also available: `- (nullable SyphonOpenGLImage *)newFrameImage;` — returns the server's current output as an image valid in the server's CGL context; "YOU ARE RESPONSIBLE FOR RELEASING THIS OBJECT."

### 3.4 Threading and context rules (get this right or you get black frames)

Verbatim class-level contract:
> "It is safe to access instances of this class across threads, except for those calls related to OpenGL: a call to ``bindToDrawFrameOfSize:`` must have returned before a call is made to ``unbindAndPublish``, and these methods must be paired and called in order. **You should not call the ``stop`` method while the FBO is bound.**"

Distilled:
- **Syphon never locks the CGL context for you.** Every GL-touching method repeats this. You own the locking.
- **The context must be current on the calling thread** when you publish, and the texture must be valid in the context you passed at init. [INFERRED — the header says the texture "must be a texture valid in the CGL context provided when the server was created"; it does not literally say "must be current". But GL name resolution requires it, and Syphon's own Jitter server sets the context before publishing. Treat "context current" as mandatory.]
- **In JUCE this means the render thread, not the message thread.** JUCE documents: *"When a native context is created, a thread is started, and will be used to call the OpenGLRenderer methods."* [VERIFIED — `juce_OpenGLContext.h`] So create the server in `newOpenGLContextCreated()`, publish in `renderOpenGL()`, and destroy in `openGLContextClosing()` — all on that thread, where the context is already current.
- If anything else can touch the context concurrently, bracket with `CGLLockContext(server.context)` / `CGLUnlockContext(server.context)` — note: **the server's `context` property**, not necessarily the one you passed in ("This may or may not be the context passed in at init").
- `publishFrameTexture:` does **not** promise a flush (only `unbindAndPublish` does). If a client sees stale or black frames, an explicit `glFlush()` before publishing is the first thing to try. [INFERRED — from the asymmetry in the documented flush guarantees.]

### 3.5 Teardown

```objc
- (void)stop;
```
> "Stops the server instance. Use of this method is optional and releasing all references to the server has the same effect."

But: **do not call `stop` while the FBO is bound** (class contract). Call `stop` explicitly on the GL thread in `openGLContextClosing()` before the context dies, rather than relying on ARC deallocation at an arbitrary time — a Syphon server outliving its CGL context is a use-after-free. [INFERRED — from the documented context ownership; the header does not spell out this ordering.] `SyphonOpenGLServer.m`'s `dealloc` calls `[self destroyResources]` and `CGLReleaseContext(_shareContext)`. [VERIFIED]

### 3.6 How the name appears to clients

`@property (nullable, strong) NSString* name;` — settable at runtime, not just at init. Clients read the server description dictionary:
- `SyphonServerDescriptionNameKey` — your server name.
- `SyphonServerDescriptionAppNameKey` — **the localized application name, supplied by Syphon from your bundle, not by you.**
- `SyphonServerDescriptionUUIDKey` — unique ID, "not for UI display".
- `SyphonServerDescriptionIconKey` — `NSImage` app icon.

So a client UI typically shows "**<AppName> — <ServerName>**". Names need not be unique; run one server per video output and name each one if you have several. [VERIFIED — `SyphonServerDirectory.h` + `SyphonOpenGLServer.h`]

`serverDescription` is `NSDictionary<NSString *, id<NSCoding>>*`; "You should not rely on the presence of any particular keys in this dictionary." Only needed directly if you used `SyphonServerOptionIsPrivate`.

---

## 4. Modern-macOS caveats

### 4.1 OpenGL is deprecated but present, and it still works — including on macOS 26

- OpenGL was deprecated in macOS 10.14 Mojave (2018) in favour of Metal. It has **not been removed**. [VERIFIED — Apple/AppleInsider coverage]
- **macOS 26 "Tahoe": OpenGL remains in the SDK; Apple has announced no removal.** What *was* removed is **AGL** (the Carbon-era OpenGL wrapper) — safe to stop linking. [VERIFIED — <https://developer.apple.com/forums/thread/796543>, <https://forum.xojo.com/t/tahoe-and-opengl-quick-note/85789>]
- **Directly relevant to a JUCE app:** a report of JUCE OpenGL plugins crashing on Tahoe Developer Beta 7 was investigated and **traced to virtualization, not the OS**. JUCE team: *"We've tested OpenGL in the latest macOS Tahoe Developer Beta on an x86_64 machine, and everything works as intended there."* Original reporter confirmed: *"It IS VM related, actual hardware (Apple Silicon) works as intended."* [VERIFIED — <https://forum.juce.com/t/macos-tahoe-and-opengl/66921>] **Do not benchmark or QA this feature in a VM** — you will chase a phantom.

### 4.2 GL server ↔ Metal client interop works by design

Syphon transports frames as **IOSurface**, a kernel-managed shared buffer usable by both Metal and OpenGL; texture data is never copied between processes, both sides read/write the same IOSurface-backed memory. Syphon explicitly "supports interoperability between Metal and OpenGL… it is possible to run a Metal-based Syphon server and receive it in an OpenGL client and vice versa." Syphon is compatible with Legacy OpenGL, Core Profile OpenGL, and Metal back ends. [VERIFIED — <https://syphon.info/>, <https://cansik.github.io/syphon-python/syphon.html>]

**Consequence: a `SyphonOpenGLServer` is fully consumable by modern Metal-based clients.** You are not stranding your output on legacy tech.

### 4.3 Verdict for this app

**`SyphonOpenGLServer` is the right choice for a JUCE/OpenGL app. Do not bridge to `SyphonMetalServer`.** Your frames are already GL textures in a JUCE-managed `NSOpenGLContext`; `SyphonMetalServer` would require an `MTLDevice`, a GL→Metal interop path, and (per §2.2) a compiled metallib — all to reach the same IOSurface that `SyphonOpenGLServer` reaches directly. The IOSurface interop in §4.2 means Metal clients see your output either way. Revisit only if the whole app moves to Metal. [INFERRED — reasoned from the verified interop and metallib facts]

### 4.4 Known upstream issues worth tracking

Open issues on the framework (12 total, none OS-version-specific): [VERIFIED — <https://github.com/Syphon/Syphon-Framework/issues>]
- **#47 (2018-12-12): `'kIOSurfaceIsGlobal' is deprecated, Syphon apps will break upon its removal.`** The one genuine long-term existential risk to Syphon. Still open after 7+ years, and Syphon still works — but it's the thing that would break everything at once if Apple pulls it.
- #67 (2022): rethink CPU-memory-access limits under unified memory.
- Remainder are Metal API refinements (#85–#87) and feature requests. **No open issues report Sonoma/Sequoia/Tahoe breakage, black frames, or Apple Silicon problems** — a meaningfully quiet bug tracker for a project this widely deployed.

Contemporary evidence of health: a commercial VJ engine reports running a 3D raymarched scene with Syphon output on an M3 Max without issues. [VERIFIED — <https://renderwave.io/apple-silicon>]

Ecosystem noise to not misread: reports of "Syphon black screen" in OBS trace to OBS's own client-side handling — *"improper handling of Syphon inputs was fixed, which broke a few sources that were sending bad data"* (OBS 32.0). That's a client bug in a third-party consumer, **not** a Syphon server-side defect. [VERIFIED — <https://obsproject.com/forum/threads/syphon-client-broken-in-32-0.190817/>]

---

## 5. Verification client — automated "server is announced" check

### 5.1 Feasibility: YES

A headless Obj-C CLI can enumerate announced Syphon servers with **no GL context, no `MTLDevice`, no `NSApplication`, and no window**. `SyphonServerDirectory` is pure Foundation + distributed notifications; it has no GL or AppKit dependency in its discovery path. [VERIFIED — `SyphonServerDirectory.m` uses `NSDistributedNotificationCenter` and pthread mutexes only]

### 5.2 The one non-obvious requirement — you must run a run loop

Discovery is **request/reply**, not a persistent registry. On init the directory registers observers and **broadcasts an announcement request**; running servers reply with their descriptions: [VERIFIED — `SyphonServerDirectory.m`]
```objc
[[NSDistributedNotificationCenter defaultCenter] addObserver:self
   selector:@selector(handleServerAnnounce:) name:SyphonServerAnnounce object:nil];
// ... also SyphonServerRetire, SyphonServerUpdate, SyphonServerAnnounceRequest
[[NSDistributedNotificationCenter defaultCenter] postNotificationName:SyphonServerAnnounceRequest
   object:nil userInfo:nil deliverImmediately:YES];
```

**Distributed notifications are delivered via the run loop.** A CLI that constructs the directory and immediately reads `.servers` will print an empty array every time — it exits before any reply is delivered. The main thread's run loop must be running in a common mode; in a CLI you must explicitly run it. [VERIFIED — Apple's Notification Centers documentation and multiple developer-forum threads on `NSDistributedNotificationCenter` in CLI tools]

This is exactly the kind of failure that looks like "Syphon is broken" when in fact the test harness is wrong. Budget a settle window of ~0.5–1.0 s.

### 5.3 Minimal working design

```objc
// clang -fobjc-arc -framework Foundation syphon_probe.m <syphon GL subset> -o syphon_probe
#import <Foundation/Foundation.h>
#import "SyphonServerDirectory.h"

int main(int argc, const char *argv[]) {
  @autoreleasepool {
    SyphonServerDirectory *dir = [SyphonServerDirectory sharedDirectory];  // posts announce-request
    // Spin the run loop so replies can arrive. THIS STEP IS MANDATORY.
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0]];

    NSArray *servers = [dir servers];
    for (NSDictionary *d in servers) {
      printf("%s | %s\n",
        [[d objectForKey:SyphonServerDescriptionAppNameKey] UTF8String],
        [[d objectForKey:SyphonServerDescriptionNameKey]    UTF8String]);
    }
    // Exit non-zero if the expected server is absent -> usable as a behavioural gate.
    NSArray *match = [dir serversMatchingName:@"YourServerName" appName:nil];
    return [match count] > 0 ? 0 : 1;
  }
}
```

API surface used, exact declarations [VERIFIED — `SyphonServerDirectory.h`]:
```objc
+ (SyphonServerDirectory *)sharedDirectory;
@property (readonly) NSArray<NSDictionary<NSString *, id<NSCoding>> *> *servers;
- (NSArray<NSDictionary<NSString *, id<NSCoding>> *> *)serversMatchingName:(nullable NSString *)name
                                                                   appName:(nullable NSString *)appname;
extern NSString * const SyphonServerDescriptionUUIDKey;
extern NSString * const SyphonServerDescriptionNameKey;
extern NSString * const SyphonServerDescriptionAppNameKey;
extern NSString * const SyphonServerDescriptionIconKey;
extern NSString * const SyphonServerAnnounceNotification;
extern NSString * const SyphonServerUpdateNotification;
extern NSString * const SyphonServerRetireNotification;
```
`sharedDirectory` is KVO-compliant on `servers`, so a longer-lived probe can observe instead of poll. Note the release-5 notification semantics change: the notification **object** is the directory instance and the server dictionary arrives in **userInfo** (it used to be the object). [VERIFIED — release body for tag 5]

`serversMatchingName:appName:` gives an exact behavioural assertion with no string parsing. Both arguments are nullable, so you can match on app name alone.

**Scope limit, stated honestly:** this proves the server is *announced and discoverable*. It does **not** prove frames are non-black — that needs a client that actually reads a frame and samples pixels. Announcement and correct imagery fail independently (§6.2), so a green probe plus a black output is a real, expected combination.

### 5.4 Human GUI check

**Syphon Simple Client** — official testbed from the Syphon org.
- Repo: <https://github.com/Syphon/Simple> ("Simple Client and Simple Server demo and testbed applications")
- Download: **<https://github.com/Syphon/Simple/releases/download/5/Syphon.Simple.Apps.zip>** (769,932 bytes, release tag `5`, published 2019-03-02; notes: "Use Core Profile OpenGL", "Simple Server no longer plays Quartz Composer compositions"). [VERIFIED — releases API]

The zip contains both Simple Client and Simple Server. Simple Client is the fastest human check that your server both announces *and* renders correct, correctly-oriented imagery — it is the right tool for settling the `flipped:` polarity from §3.2.

---

## 6. Known pitfalls

### 6.1 App Sandbox is a hard blocker (highest-impact pitfall)

Syphon's discovery rides on `NSDistributedNotificationCenter` **with a `userInfo` payload**, and sandboxed processes cannot receive that payload. Upstream issue #10 "Sandbox Support" states it plainly: the distributed notifications for announce/retire/SurfaceID *"limits Mac App Store capability due to the fact that NSUserInfo dictionaries cannot be populated to Applications running in a sandboxed environment. This limits Syphon Framework to only be included in non Mac App Store binaries."* [VERIFIED — <https://github.com/Syphon/Syphon-Framework/issues/10>]

Independently, macOS stopped allowing `nil` notification names for distributed notifications, another sharp edge in this area. [VERIFIED — <https://mjtsai.com/blog/2019/10/04/nsdistributednotificationcenter-no-longer-supports-nil-names/>]

**Implications:**
- Ship **non-sandboxed**; Direct/Developer-ID distribution. Mac App Store is effectively off the table for Syphon support.
- If your app currently has `com.apple.security.app-sandbox`, Syphon will fail in a way that looks like "no servers ever appear" — and the *server* side can appear to start fine while no client ever sees it.
- App Groups do not rescue this: Syphon uses its own port naming convention, so you cannot bring its CFMessagePort traffic under an App Group. [VERIFIED — developer report in the same issue thread]

### 6.2 Server announced but frames are black

Ranked by likelihood, synthesised from the verified API contract plus field reports:
1. **Wrong / not-current CGL context** — the texture name is only meaningful in the context passed at init. A GL name from another context silently resolves to nothing. Field reports of "Invalid IOSurface Buffer" and `device_texture_create_from_iosurface (GL) failed` are context-mismatch symptoms. [VERIFIED — symptom reports; [INFERRED] on ranking]
2. **Context changed underneath a live server** — if the GL context is recreated (JUCE will do this on some display/component changes), **destroy and recreate the server**. [VERIFIED — this exact remedy is documented in the field]
3. **Not flushed.** `publishFrameTexture:` makes no documented flush guarantee (only `unbindAndPublish` does). Try an explicit `glFlush()` before publishing. [INFERRED — from the header's asymmetric guarantees]
4. **Wrong texture target.** Only `GL_TEXTURE_2D` and `GL_TEXTURE_RECTANGLE_EXT` are legal. Passing anything else is undefined. [VERIFIED — header]
5. **Wrong `imageRegion` / `textureDimensions`.** `textureDimensions` must be the *full allocated* texture size; passing the region size for both silently crops or samples garbage. [INFERRED — from the documented meaning of each parameter]
6. **`flipped:` wrong** — image appears but upside down. Cosmetic, not black; settle against Simple Client.

### 6.3 Threading pitfalls specific to JUCE

- Renderer callbacks run on **JUCE's dedicated GL thread**, not the message thread. [VERIFIED — `juce_OpenGLContext.h`] Create/publish/stop the server there.
- Syphon locks nothing for you; JUCE's own code locks via `CGLLockContext` on the context derived from `NSOpenGLContext`. Match that pattern if any other thread can touch the context. [VERIFIED — `juce_OpenGL_mac.h`]
- JUCE needs a **visible, non-zero-size component** for `renderOpenGL()` to fire — pure-offscreen Syphon output needs the single-top-level-context workaround from §2.5. [VERIFIED — JUCE forum]
- Continuous repainting can lock the MessageManager for long stretches; `setComponentPaintingEnabled(false)` + `setContinuousRepainting(false)` is the documented remedy. Relevant here because a real-time **audio** app must not have its message thread stalled. [VERIFIED — JUCE forum]

### 6.4 Signing, hardened runtime, notarization

- **No entitlement is required to *be* a Syphon server** — no camera/mic/screen-recording entitlement, and Syphon is local-only ("Syphon works on your computer's graphics card to share video and images between applications on the same computer"), so no network entitlement either. [VERIFIED — <https://syphon.info/>; the "no entitlement needed" conclusion is [INFERRED] from the absence of any documented requirement across sources]
- The real constraint is **negative**: do **not** enable App Sandbox (§6.1).
- Hardened runtime + notarization are required for Developer-ID distribution regardless, and are compatible with Syphon. [VERIFIED — Apple notarization docs]
- **If you take approach (c)**, the embedded `Syphon.framework` is a nested bundle: it must be signed with hardened runtime enabled and placed in `YourApp.app/Contents/Frameworks/`. Sign inside-out (framework first, app last) — `codesign --deep` is discouraged by Apple. **Approach (a) avoids this entire class of problem**, since the code links statically into your already-signed binary. [VERIFIED on the signing rules; [INFERRED] on the comparative advantage]
- User-facing: nothing to install. *"Some applications have Syphon support built-in… Nothing needs to be installed to use these applications."* [VERIFIED — <https://syphon.info/faq>]
- macOS only; no network sharing. [VERIFIED — same FAQ]

### 6.5 Build-system pitfalls

- **`-fno-objc-arc` is wrong** — the codebase is ARC (§2.1). This is the single most likely wrong turn given the brief's framing.
- **Missing `-include Syphon_Prefix.pch`** → undefined `SYPHONLOG` (§2.2).
- **`<Syphon/…>` imports unresolved** unless public headers are staged into a `Syphon/` directory (§2.2).
- **Including the umbrella `Syphon.h`** drags in Metal — import the specific headers instead (§2.2).
- **`GIT_SHALLOW TRUE` cannot fetch an arbitrary commit SHA** — leave it off when pinning a SHA rather than a tag. [VERIFIED — documented CMake FetchContent behaviour]
- **Spaces in the build path break the Syphon Xcode build** (approach (c) only) (§2.4).
- Upstream ships **no `CMakeLists.txt`**; `FetchContent_MakeAvailable` populates but adds no targets (§0.1).

---

## 7. Open questions a builder should close

1. **Exact GL-only source list** — §2.4(a) is inferred from the file listing, not compiled. One link-error iteration settles it. Loud failure, low risk.
2. **`flipped:` polarity for JUCE-rendered FBO textures** — settle empirically against Simple Client. One-line change either way.
3. **Contents of `Syphon.SDK.5.zip`** — whether it contains a prebuilt `Syphon.framework`. Only matters if you take approach (c) *and* want to skip xcodebuild; and note that a tag-5 binary would carry the **old** `SyphonServer` API (§1.3), so it is probably a dead end regardless.
4. **Whether `en.lproj` resources are needed by the GL path** — if any user-visible string comes from the framework bundle, direct compilation (approach (a)) would lose it. Not checked. Low impact (worst case a cosmetic default name), but unverified.

---

## 8. Source index

**Upstream Syphon (primary, all read directly)**
- Repo: <https://github.com/Syphon/Syphon-Framework>
- `SyphonOpenGLServer.h`: <https://raw.githubusercontent.com/Syphon/Syphon-Framework/main/SyphonOpenGLServer.h>
- `SyphonServerDirectory.h`: <https://raw.githubusercontent.com/Syphon/Syphon-Framework/main/SyphonServerDirectory.h>
- `SyphonServerBase.{h,m}`, `SyphonOpenGLServer.m`, `SyphonOpenGLClient.m`, `SyphonServerRendererMetal.m`, `Syphon.h`, `Syphon_Prefix.pch`, `Syphon.modulemap`, `Syphon.xcodeproj/project.pbxproj` — all under `raw.githubusercontent.com/Syphon/Syphon-Framework/main/`
- `License.txt`: <https://raw.githubusercontent.com/Syphon/Syphon-Framework/main/License.txt>
- Getting Started: <https://github.com/Syphon/Syphon-Framework/blob/main/Syphon.docc/GettingStarted.md>
- Tags API: <https://api.github.com/repos/Syphon/Syphon-Framework/tags> · Latest release: <https://api.github.com/repos/Syphon/Syphon-Framework/releases/latest> · `main` branch: <https://api.github.com/repos/Syphon/Syphon-Framework/branches/main>
- Issues: <https://github.com/Syphon/Syphon-Framework/issues> · Sandbox issue #10: <https://github.com/syphon/syphon-framework/issues/10>
- Project site / FAQ: <https://syphon.info/> · <https://syphon.info/faq>
- Simple apps: <https://github.com/Syphon/Simple> · <https://github.com/Syphon/Simple/releases/download/5/Syphon.Simple.Apps.zip>
- Jitter server reference: <https://github.com/Syphon/Jitter/blob/master/Server/jit.gl.syphonserver/jit.gl.syphonserver/jit.gl.syphonserver.m>

**JUCE**
- `juce_OpenGLContext.h`: <https://raw.githubusercontent.com/juce-framework/JUCE/master/modules/juce_opengl/opengl/juce_OpenGLContext.h>
- `juce_OpenGL_mac.h`: <https://raw.githubusercontent.com/juce-framework/JUCE/master/modules/juce_opengl/native/juce_OpenGL_mac.h>
- Offscreen render for Spout/Syphon: <https://forum.juce.com/t/openglrenderer-without-a-component-complete-offscreen-render-for-spout-syphon/22231>
- macOS Tahoe + OpenGL: <https://forum.juce.com/t/macos-tahoe-and-opengl/66921>
- `juce_sharedtexture` (GPLv3): <https://github.com/benkuper/juce_sharedtexture>

**Integration precedents**
- OBS `mac-syphon` CMakeLists: <https://raw.githubusercontent.com/obsproject/obs-studio/master/plugins/mac-syphon/CMakeLists.txt>
- ofxSyphon: <https://github.com/astellato/ofxSyphon> · Cinder-Syphon: <https://github.com/astellato/Cinder-Syphon>
- nozzle-spout-syphon: <https://github.com/nozzle-io/nozzle-spout-syphon> · syphon-python: <https://github.com/cansik/syphon-python>
- Xcode-build path-with-spaces bug: <https://obsproject.com/forum/threads/building-obs-from-source-in-mac-10-13-syphon-framework-cant-deal-with-space.120476/>
- OBS Syphon client regression (not a Syphon bug): <https://obsproject.com/forum/threads/syphon-client-broken-in-32-0.190817/>

**macOS platform**
- OpenGL after macOS 26: <https://developer.apple.com/forums/thread/796543> · <https://forum.xojo.com/t/tahoe-and-opengl-quick-note/85789>
- Distributed notifications + run loop: <https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Notifications/Articles/NotificationCenters.html>
- `nil` names no longer supported: <https://mjtsai.com/blog/2019/10/04/nsdistributednotificationcenter-no-longer-supports-nil-names/>
- Syphon on Apple Silicon in production: <https://renderwave.io/apple-silicon>
