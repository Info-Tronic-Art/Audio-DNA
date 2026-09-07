# Scout — OutputWindow second-GL-context crash family
2026-08-02, read-only recon. HEAD = 375f3c6. Scout: outputwindow-glcrash.
Queued follow-up from 2026-07-30 (that day's MAIN-context family was closed;
this sibling had never been examined).

## VERDICT (one line)
The second context is **confirmed unshared**, and `EffectChain` — shared by
reference between the two contexts — carries **per-GL-context state**
(`uniformLocationCache_`, `prevFrameTexture_/FBO_`) plus a **stateful
`MappingEngine::processFrame` that both GL threads drive every frame**. The
top risk is a lock-free `std::unordered_map` mutated by two GL threads
(crash, UB) that ALSO silently poisons uniform locations across contexts and
**stays poisoned after the output window closes**. All four existing .ips
reports show exactly ONE GL thread ⇒ the closed family really was
main-context-only; none of this has ever been exercised in a recorded crash.

---

## 1. TOPOLOGY — second GL context/thread lifecycle

- `OutputRenderer : public juce::OpenGLRenderer` with its **own**
  `juce::OpenGLContext glContext_` — OutputWindow.h:17, :42. **VERIFIED.**
- Attach: `attachTo()` OutputWindow.cpp:20-27 — sets GL 4.1, `setRenderer(this)`,
  `setContinuousRepainting(true)`, `setComponentPaintingEnabled(false)`,
  `attachTo(component)`. Each `attachTo` spawns its own JUCE render thread
  (thread name "OpenGL Renderer" — same name as the main one). **VERIFIED.**
- Called from the `OutputWindow` ctor, OutputWindow.cpp:276, on
  `outputComponent_` (a plain child `Component`, OutputWindow.h:94-104).
- Detach: `~OutputWindow` → `renderer_.detach()` OutputWindow.cpp:279-282.
- Callbacks on the second GL thread: `newOpenGLContextCreated()` (:49-64,
  `quad_.init()` + `initShaders()` = ~80 program compiles), `renderOpenGL()`
  (:66-148), `openGLContextClosing()` (:150-155, releases only its OWN
  shaderMgr_/texMgr_/quad_).
- **UNSHARED — VERIFIED at HEAD.** `grep setNativeSharedContext|NativeShared|
  shareWith` over `src/` + `tests/` returns nothing; the only `getRawContext()`
  uses are Syphon output on the MAIN context (Renderer.cpp:126,130). Prior
  recon confirmed. Consequence: **GL object names (textures, FBOs, programs)
  from one context are meaningless in the other** — load-bearing for §3.
- Own-vs-shared split: `FullscreenQuad quad_`, `ShaderManager shaderMgr_`,
  `TextureManager texMgr_` are per-renderer members (OutputWindow.h:48-50) —
  **correctly independent.** Everything in §2 is the shared part.
- **Hidden lifecycle edge (VERIFIED):** JUCE auto-detaches on hide.
  `canBeAttached` → `isShowingOrMinimised` walks the **parent** chain
  (juce_OpenGLContext.cpp:1163-1177); `componentVisibilityChanged` → `detach()`
  (:1129-1144); the watcher registers with ancestors
  (juce_ComponentMovementWatcher.cpp:44-45). So `setVisible(false)` on the
  window tears the second context down **synchronously on the message thread**
  — the same mechanism as the closed MilkDrop UAF (.harmony/scout-milkdrop-uaf.md).
  Re-showing re-attaches → full 80-shader recompile + fresh cache-insert burst.

## 2. SHARED STATE MAP (both contexts touch)

Shared by reference via ctor (OutputWindow.h:20-22, wired MainComponent.cpp:2474-2477
from `analysisThread_.getFeatureBus()`, `previewPanel_.getMappingEngine()`,
`previewPanel_.getEffectChain()`).

| Object / field | Writer thread(s) | Reader thread(s) | Guard at HEAD |
|---|---|---|---|
| `EffectChain::effects_` (EffectChain.h:126) | GL(main) `initEffectChain`; msg thread | **both GL**, msg, HTTP | `effectsMutex_` (container only) |
| `Effect::enabled_` (Effect.h:65) | msg (UI), HTTP | **both GL** (EffectChain.cpp:53) | **none** |
| `EffectParam::value` (Effect.h:12) | msg, HTTP, **both GL** via MappingEngine + RoutingEngine (Renderer.cpp:232) | **both GL** (EffectChain.cpp:365-367) | **none** |
| `EffectChain::uniformLocationCache_` (EffectChain.h:141) | **both GL** (EffectChain.cpp:275) | **both GL** (:270) | **none** ← §3 R1 |
| `EffectChain::prevFrameTexture_/prevFrameFBO_/prevFrameWidth_/Height_` (EffectChain.h:130-133) | **both GL** (EffectChain.cpp:379-403) | **both GL** (:136-139,:413-419) | **none** ← §3 R3 |
| `EffectChain::latestSnapshot_` (EffectChain.h:92) | GL(main) only (Renderer.cpp:489) | **both GL** (EffectChain.cpp:294-296) | **none** |
| `MappingEngine::mappings_` / `smoothers_` (MappingEngine.h:53-54) | msg thread (add/remove/clear) | **both GL** (`processFrame`) | **none** ← §3 R2 |
| `Smoother::value_/initialized_` (Smoother.h:41-43) | **both GL** (MappingEngine.cpp:197) | **both GL** | **none** ← §3 R2 |
| `FeatureBus` slots (FeatureBus.h:70) | analysis thread | **both GL** + msg + HTTP | atomic state, **single-reader contract** ← §3 R4 |

- Textures/programs that cross: **none by design, but ids leak via the cache
  and prevFrame quartet** — see §3 R1/R3. That is the whole bug family.
- Composition/model reads: OutputRenderer touches **none** — it does not read
  `composition_`, decks, or clips. Its `renderOpenGL()` reads only FeatureBus,
  MappingEngine, EffectChain, and its own GL objects. **VERIFIED** by reading
  OutputWindow.cpp:66-148 in full. (Good news: undo/composition teardown at
  shutdown cannot be reached from the second thread.)
- **STALE PREMISE (VERIFIED):** the DEFERRED BOUNDARY note at EffectChain.h:108-124
  reasons about `Effect::enabled_`/`EffectParam::value` being "read every frame
  on **the** GL thread" (singular) with writers on msg/HTTP. OutputWindow adds a
  **second GL reader AND a second GL writer** (via `processFrame`). The note's
  severity call ("scalar tearing, not container corruption") no longer covers
  the write/write case. Re-assess that note when this lane is worked.

## 3. RACE / UAF WINDOWS (ranked)

### R1 — `uniformLocationCache_`: two GL threads mutating one `unordered_map`. **CRASH. NEW with OutputWindow.**
- `std::unordered_map<std::string,GLint>` (EffectChain.h:141), **no mutex**.
  `find()` then `uniformLocationCache_[key] = loc;` — EffectChain.cpp:270-276.
- Reached from `render()` (:131,:140) and `uploadEffectUniforms()` (:285-367),
  i.e. from BOTH `Renderer.cpp:490` and `OutputWindow.cpp:143`. **VERIFIED.**
- Concurrent insert → rehash while the other thread walks a bucket chain = UB
  (segfault or infinite loop). Widest window is **immediately on output-window
  open**: ~80 programs × up to ~35 uniform names, all cold-miss inserts on the
  new GL thread, against a main thread doing the same lookups at 60Hz.
- **Second, quieter bug (INFERRED, strong):** the key is
  `to_string(program->getProgramID()) + ":" + uniformName` (:267-268). Program
  IDs are per-share-group; two unshared contexts both allocate from 1 and both
  compile the same ~80 programs, so the ID sets **collide almost entirely**.
  A location cached for output-context program 5 is then returned to the main
  context's *different* program 5 → `glUniform1f` writes the wrong uniform.
  Entries are never evicted and `ShaderManager::releaseAll()` does not touch
  this cache ⇒ **the main preview stays poisoned after the output window
  closes, until app restart.** (GL guarantees per-share-group namespaces —
  VERIFIED by spec; the sequential-from-1 numbering is implementation
  behaviour — INFERRED. Cheap check: log `getProgramID()` from both renderers.)
- **Cheapest structural close:** this cache is *per-context state living in a
  shared object* — move it out. Give `render()` a `EffectChainGLState&`
  parameter (owned by each Renderer, alongside the shaderMgr/texMgr/quad it
  already passes). Fixes the race **and** the ID collision, adds no hot-path
  lock. A mutex alone would fix the crash but NOT the wrong-location bug.

### R2 — `MappingEngine::processFrame` driven by both GL threads. **CRASH (container) + visible GLITCH (write/write).**
- `OutputWindow.cpp:115` and `Renderer.cpp:239` both call
  `mappingEngine_.processFrame(*snap, effectChain_)` on their own GL threads.
- **The idempotence claim at OutputWindow.cpp:111-114 is FALSE. VERIFIED:**
  - `smoothers_[i].process(scaled)` (MappingEngine.cpp:197) mutates
    `Smoother::value_`/`initialized_` (Smoother.h:15-25) — a stateful EMA.
    Two callers per snapshot **double the EMA update rate** (smoothing time
    constant effectively halves — a visible responsiveness change in BOTH
    windows) and race read-modify-write on a plain float.
  - The param write is a **3-pass RMW on a shared plain float**: reset to 0.0f
    (:162) → `+= smoothed` (:200) → clamp (:214). Thread A's reset pass can
    land inside thread B's accumulate pass ⇒ params momentarily 0 or
    half-summed ⇒ **flicker on preview and output simultaneously.**
- `mappings_`/`smoothers_` are plain `std::vector`, **no mutex**, and unlike
  `effects_` **removal exists**: `removeMapping` erases (MappingEngine.cpp:18-19),
  `clearAll` clears (:39-41). Msg-thread writers: EffectsRackPanel.cpp:36,74,
  262,432,448,527; PresetManager.cpp:200,222; MainComponent.cpp:2928;
  Renderer.cpp:1533. Both GL threads iterate `for (const auto& m : mappings_)`
  (:152,:204) and index `smoothers_[i]` (:195-197).
  - erase/clear during iteration ⇒ iterator UAF ⇒ segfault.
  - `addMapping` pushes `mappings_` **then** `smoothers_` (:8-9): in that gap
    `smoothers_[i]` is an out-of-bounds read.
  - Container race is **pre-existing** (1 GL thread vs msg); OutputWindow
    roughly **doubles** it by adding a second concurrent iterator. The
    write/write race on `Smoother::value_` and `EffectParam::value` is **NEW**.
- **Cheapest structural close:** one deletion — `OutputRenderer` must NOT call
  `processFrame` (drop OutputWindow.cpp:115). The output window renders the
  same `EffectChain` whose params the main renderer has already updated this
  frame, so the second mapping pass buys nothing. Removes the doubled EMA and
  the entire GL/GL write/write class in one line. (The msg-thread-vs-GL
  container race remains a separate, pre-existing work item: mutex or a
  mutation queue drained on the GL thread.)

### R3 — `prevFrame*` GL names shared across UNSHARED contexts. **GLITCH + churn; crash plausible.**
- `prevFrameTexture_/FBO_/Width_/Height_` (EffectChain.h:130-133) are generated
  by `glGenTextures`/`glGenFramebuffers` inside `ensurePrevFrameFBO`
  (EffectChain.cpp:391,400) and consumed at :136-139 and :413-431 — from
  **both** contexts, which share no namespace (§1).
- The two contexts render at **different sizes** (preview panel vs fullscreen
  display), so the early-out `prevFrameWidth_ == width` (:373-374) essentially
  never holds ⇒ **every frame, each thread deletes the other's names and
  regenerates** (:377-386). That is per-frame texture+FBO churn in both
  contexts, `glDeleteTextures` on names owned by the other context, and
  binding a name that resolves to a *different* object locally ⇒ garbage
  sampling. Plus a plain data race on four non-atomic ints.
- Gated by `effect->isTemporal()` (:88,:136) — needs ≥1 temporal effect enabled.
- Sharpest instant: **output-window close.** JUCE detaches synchronously on the
  message thread (§1) while the MAIN GL thread is live and may be mid-`render()`
  holding these fields; `openGLContextClosing` (OutputWindow.cpp:150-155) does
  **not** clear them, so the main context can keep binding names belonging to a
  context that no longer exists.
- Same close as R1: move the quartet into the per-renderer `EffectChainGLState`.

### R4 — FeatureBus single-reader contract. **GLITCH only. PRE-EXISTING, aggravated.**
- Contract: "**The** reader calls acquireRead()… valid until the next
  acquireRead()" (FeatureBus.h:34-43). Already violated before OutputWindow:
  Renderer.cpp:211 (GL), TopBar.cpp:216 / SignalBar.cpp:114 /
  AudioReadoutPanel.cpp:17 / MainComponent.cpp:2823,2992,3120 (msg thread),
  ApiServer.cpp:233,528,568,613 (HTTP, `getLatestRead` only — no CAS).
  OutputWindow.cpp:103 adds a **third CAS-ing reader at 60Hz.**
- **Traced state machine — the writer can be handed a slot an in-flight reader
  still holds (VERIFIED by hand-trace of FeatureBus.cpp:19-78):**
  start `(w0,l1,r2)` → reader A acquires → `(w0,l2,r1)`, A holds slot **1**
  → writer publishes → `(w2,l0,r1)` → reader B acquires → `(w2,l1,r0)`; slot 1,
  **still being read by A**, is now marked *latest* → writer publishes →
  `(w1,l2,r0)`: **the writer now writes slot 1 under A.**
- Severity is bounded: `buffers_` is a fixed `std::array` member (FeatureBus.h:70),
  so memory is always valid — torn/stale feature values, **not** a UAF, **not**
  a crash. OutputWindow holds `snap` across `processFrame` (OutputWindow.cpp:103-115),
  which widens the torn-read window.
- Close: give each reader its own slot (per-reader read index), or have the
  single main GL reader publish an immutable copy the others share.
- Note: the 2026-07-30/08-02 `acq_rel` fix (08f7361) hardened the ordering for
  the **single**-reader design; it does not make the bus multi-reader-safe.

### R5 — shutdown ordering: the 2026-07-30 law does NOT cover the second context. **LATENT.**
- The law (MainComponent.cpp:1749-1759) detaches **only**
  `previewPanel_.getRenderer().detach()`. The second context is detached
  **17 lines later** at :1776 (`outputWindow_.reset()` → `~OutputWindow` →
  `renderer_.detach()`, OutputWindow.cpp:281) — i.e. **after**
  `undoManager_.clear()` (:1763), OSC/MIDI/recorder stop (:1766-1768),
  inspector reset (:1771), `closeCamera()` (:1774). **VERIFIED.**
- No crash is provable from this ordering today, because the output thread
  touches none of those objects (§2: no composition/model reads) and
  `analysisThread_.stopThread` (:1778) runs **after** the reset, so FeatureBus
  outlives it. But the law's stated intent — "end GL activity before any UI
  teardown" — is **not met for the second context**, and any future widening of
  `OutputRenderer`'s reads (e.g. compositor/clip access) turns this into the
  exact 07-30 crash again. Close: add `if (outputWindow_) outputWindow_->getRenderer().detach();`
  immediately beside :1759, keeping `.reset()` where it is (detach is idempotent).
- **Close-path asymmetry (VERIFIED):** `OutputWindow::closeButtonPressed`
  (:284-288) and `OutputWindow::keyPressed` Escape (:317-326) only
  `setVisible(false)` — the object survives; `MainComponent::closeOutput`
  (:2487-2494) destroys it. Which Escape handler runs depends on focus
  (`setWantsKeyboardFocus(true)`, :274). Hide-only still detaches GL (§1), so
  it is not a live-thread leak — but each hide/show cycle re-runs the 80-shader
  compile and a fresh cold-cache insert burst, i.e. **re-arms R1 every time.**

## 4. CRASH-SIGNATURE CROSS-CHECK — was the closed family main-context-only? **YES.**
- All four `~/Library/Logs/DiagnosticReports/Audio-DNA-*.ips`
  (2026-07-28-182825, 07-28-190701, 07-30-125725, 07-30-132512) parsed as JSON:
  **each contains exactly ONE thread named "OpenGL Renderer"** (indices 18, 29,
  18, 27; totals 45/31/20/29 threads). **VERIFIED.** With the output window
  open there would be TWO identically-named GL threads (§1), so the output
  window was **not open** in any recorded crash.
  (The raw-text count of 2 in 182825 is one `threads` entry plus one
  `legacyInfo` restatement of the crashed thread — still one thread.)
- **Zero** frames mentioning `OutputWindow`/`OutputRenderer` in any of the four.
- `.harmony/scout-shutdown-sigbus.md:30-33` reasons about "Thread 18 'OpenGL
  Renderer'" in the singular and attributes the live-GL-through-teardown
  problem solely to `~PreviewPanel` ordering — consistent with one context.
- `.harmony/scout-milkdrop-uaf.md` is entirely main-context
  (`previewPanel_.setVisible(false)` → `Renderer::openGLContextClosing`); no
  second-context involvement.
- ⇒ The 07-30 closure stands. This dossier's family is **unexercised, not
  disproven** — absence of OutputWindow crashes is absence of *evidence*
  (nobody had the output window open during a captured crash), not evidence of
  safety. Note also **zero test coverage**: no file under `tests/` references
  OutputWindow or OutputRenderer (grep-verified).

## 5. EXPOSURE — when is OutputWindow live?
- **Default OFF.** `outputWindow_` is a null `unique_ptr` (MainComponent.h:240)
  until an explicit user action; no startup/preset/autoload path constructs it
  (all call sites enumerated below). **VERIFIED.**
- Entry points, all user-driven: menu *Output → fullscreen on display N*
  (MenuBarModel.cpp:142 → MainComponent.cpp:3746-3749); the display-selector
  combo (:328-330, :502-504); **Cmd+F** (:2272-2284).
- Exits: Escape (:2223-2231 → `closeOutput()` destroy) or Escape with output
  focused (OutputWindow.cpp:319-325 → hide only); Cmd+F toggle (:2276);
  :4370; app quit (:1776).
- Honest read on fraction of sessions: **near-zero in dev/test sessions
  (default off, no tests), but ~100% of real VJ performance sessions** — the
  output window IS the product's primary output path, and a performance runs
  with it open on a projector for the whole set, with effects being toggled
  and knobs turned live (exactly the R1/R2 trigger profile). So: rarely hit
  during the work that produced the existing .ips corpus, routinely hit during
  the use case that matters most. (INFERRED from the feature's purpose +
  keybinding prominence; not measured.)

## 6. RANKED VERDICT — top 3

| # | Risk | Severity | Likelihood | Cheapest structural close |
|---|---|---|---|---|
| 1 | `uniformLocationCache_` mutated by 2 GL threads (EffectChain.h:141, .cpp:270-276) | **CRASH** (unordered_map UB) + silent wrong-uniform corruption that **survives output-window close** | **HIGH** whenever output opens with effects active; window widest at open | Move the cache (and R3's quartet) out of shared `EffectChain` into a per-renderer `EffectChainGLState&` passed to `render()`. Kills race + ID collision, no hot-path lock. |
| 2 | `MappingEngine::processFrame` driven by both GL threads (OutputWindow.cpp:115) — docs claim idempotent, it is **not** | **CRASH** (unguarded vector w/ erase) + **visible flicker** both windows + halved smoothing | **HIGH** for glitch (every frame); MEDIUM for crash (needs msg-thread mapping edit during render — routine in a live set) | **Delete OutputWindow.cpp:115.** Main renderer already updated the shared params this frame. One line. |
| 3 | `prevFrame*` GL names shared across unshared contexts (EffectChain.h:130-133) | **GLITCH** + per-frame FBO/texture churn; cross-context deletes; crash plausible | MEDIUM — needs a temporal effect enabled; then every frame | Same as #1 (same `EffectChainGLState` move). |

Runners-up: R4 FeatureBus multi-reader (glitch, pre-existing, aggravated);
R5 shutdown law not generalized to the second context (latent, one-line close).

**Sequencing note:** #2 is one deletion and removes the largest *behavioural*
misbehaviour; #1/#3 are one shared refactor (`EffectChainGLState`) and remove
the crash. Doing #2 first does **not** fix #1 — the cache race survives it,
because `render()` itself is what touches the cache.

## Residuals / what would refute this
- Program-ID collision (R1's silent half) is INFERRED. Cheapest refutation:
  log `getProgramID()` for the same shader name from both renderers — disjoint
  ID sets would demote that half (the map-mutation crash stands regardless).
- Nothing here is reproduced. Cheapest confirmation, in order: (a) open the
  output window under the existing **TSan** build (`build-tsan/` present, and
  08f7361 shows TSan is already wired and trusted here) with an effect chain
  active — R1/R2/R3 should all report immediately; (b) ASan build
  (`build-asan/`) + open/close the output window in a loop while toggling
  effects, for the R2 container UAF.
- R3's "different sizes ⇒ never early-outs" assumes the preview panel is not
  coincidentally the display size; on a single-monitor setup where the preview
  is nearly fullscreen the churn could be intermittent rather than per-frame.
