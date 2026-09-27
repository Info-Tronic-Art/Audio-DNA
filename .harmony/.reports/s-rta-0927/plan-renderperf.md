# plan-renderperf -- two render-side performance fixes (s-rta-0927)

Architect plan. Base: main @ 1636785 (HEAD is 8826fea = one docs-only commit on top; `git diff --stat 1636785..HEAD` over
every source/test/probe file cited here is EMPTY -- all line numbers below hold on both). Every claim is disk-cited and
binned VERIFIED (read in the source) / INFERRED (derived) / ASSUMED (not checked -- the builder checks it).

QUESTION: (1) Which of the lane's options A-G removes the one-time 35.5-38.5 ms hitch when a layer's first crossfade
(or its first Screen Split / Frame Stutter use) creates a 480-cell frame ring inside one frame, and exactly how is it
built? (2) How is the capture path (render_frame / snapshot: synchronous glReadPixels + per-pixel setPixelColour,
98-159 ms round trip at 1080p) made cheaper without changing what a capture returns, and is async PBO readback in scope?

APPROACH (stated first):
(1) Option C, lazy per-cell allocation: a ring cell (texture + FBO) is created the first time it is WRITTEN, inside
    `pushFrameToRing`, never in bulk. Both hitches (first use and first crossfade) go from ~36 ms to ~0.08 ms; the
    steady-state VRAM ceiling is unchanged (it is reached after 480 pushed frames instead of at once). The ring's
    read-index math moves into a pure header so a ctest pins the invariant lazy allocation rests on: a cell is never
    read before it has been written.
(2) Three semantics-preserving steps: (C0) timing on the existing `[Eyes] Captured frame:` log line so the RED numbers
    are measured, not inferred; (C2) replace the per-pixel loop with a row loop over `juce::PixelARGB` that is
    byte-identical by construction (ctest compares against the real `setPixelColour` loop and the resulting PNG bytes);
    (C3) leave ONLY `glReadPixels` on the GL thread -- conversion, PNG encode and the file write move to the thread
    that is already blocked waiting in `captureFrame`. The frame a capture returns is unchanged (same synchronous read
    of the same frame's canvas), the file is complete before the call returns (today it is not quite -- see F12), and
    the GL thread's stall shrinks from read+convert+encode to read alone. Async PBO readback: OUT of scope (ruled below).

## FACTS VERIFIED IN THE SOURCE

F1 (VERIFIED) Ring creation: `CompositorEngine::getOrCreateRingBuffer` (`src/render/CompositorEngine.cpp:1671-1713`)
   sizes `fbos`/`textures` to `kMaxRingFrames` (=480, `CompositorEngine.h:252`) and creates + clears all 480 cells in the
   loop at `:1700-1707`. Callers: Frame Stutter `:377`, Screen Split `:1823` -- both immediately `pushFrameToRing`
   (`:378`, `:1824`), i.e. the ring is written before it is read in the same call.
F2 (VERIFIED) The crossfade hand-over `handOverClipHistory` (`:1715-1760`, called from `renderLayerStages` at `:887`)
   SWAPS the two map entries (`:1753`) and resets the incoming ring's `frameCount`/`writeIndex` to 0 (`:1754-1759`).
   The incoming clip's next Split/Stutter use finds either an uninitialised entry (first fade: spare created = the
   hitch) or the old spare (later fades: no creation -- the lane's 2nd-fade peaks 1.2-5.9 ms).
F3 (VERIFIED) Read path `getFrameFromRing` (`:1785-1796`): returns 0 when `!initialized || frameCount == 0`, clamps
   `framesAgo` to `[0, frameCount-1]`, index `(writeIndex - 1 - framesAgo + 2*480) % 480`. Write path
   `pushFrameToRing` (`:1762-1783`): draws the passthrough program over the full cell viewport (`glViewport(0,0,
   ringWidth, ringHeight)`, blend off, `FullscreenQuad` spans -1..1: `src/render/FullscreenQuad.cpp:20-23`), then
   `writeIndex = (writeIndex+1) % 480; if (frameCount < 480) ++frameCount`. Nothing in the render path enables a
   scissor test or a colour mask (`grep glScissor|GL_SCISSOR_TEST|glColorMask src/render` = none; the one hit is
   `src/output/OutputPresenter.cpp:81`, a `glDisable` in the output windows' own context). So a push overwrites every
   byte of the cell -- the creation-time `glClear` is dead work once creation happens at first write.
F4 (VERIFIED) Consumers tolerate a 0 texture: Frame Stutter `if (delayedTex != 0)` (`:402-406`), Screen Split
   `if (cellTex == 0) cellTex = clipTex` (`:1868-1869`). With F3, indices returned by the read path lie in the written
   set {writeIndex-1, ..., writeIndex-frameCount} mod 480, so a lazily-created ring never hands out an unwritten cell;
   the fallbacks stay as a belt.
F5 (VERIFIED) Release paths already guard on non-zero handles: destructor `:71-80`, `rescaleHistory` `:149-160` (drops
   every ring on a canvas-size change; `frameRingCount_` decremented), and the "Release old" branch of
   `getOrCreateRingBuffer` `:1683-1691`. All three are compatible with zero (never-created) cells as written.
F6 (VERIFIED) `createFBO` (`:167-179`) binds `GL_TEXTURE_2D` on the active unit and leaves framebuffer 0 bound -- the
   R5 lesson (`CompositorEngine.h:236-238`, `:1717-1720`). Today's bulk creation already runs at exactly the point in
   the pass sequence where the lazy create will run (inside `applyClipEffects`/`applyScreenSplit`, right before
   `pushFrameToRing` binds its own FBO and texture) -- lazy creation adds no new R5 exposure.
F7 (VERIFIED) `kMaxRingFrames` is 480 because Screen Split's reach is `cellIndex * framesPerCell` with up to 8x8 cells
   and 60 frames/cell (`:1810-1815`, `:1866`) -- far past 480 (clamped). Frame Stutter reaches <= 30 (`:381`). Frame
   Stutter hands the ring cell ITSELF downstream as `currentInput` (`:402-406`): later effects sample it as a plain
   `sampler2D`.
F8 (VERIFIED) VRAM per cell = `(w/ds) * (h/ds) * 4` with `ds = RenderGeometry::ringDownscale(w)` = max(4, ceil(w/480))
   (`src/render/RenderGeometry.h:51-55`): 1080p 480x270 = 518,400 B/cell -> 237.3 MiB/ring; 4K (ds 8) the same; 1440p
   426x240 -> 187.2 MiB; 720p (ds 4) 320x180 -> 105.5 MiB. Pitfall 20 (`docs/claude/pitfalls.md:49`) still says
   "1/4 resolution via kRingDownscale = 4" -- the 480-px cap (plan4 1E) is documented at `CompositorEngine.h:247-251`
   and `docs/claude/rendering.md:69`, not in the pitfall.
F9 (VERIFIED) Frame timer window: `renderStart` `src/render/Renderer.cpp:529` -> `renderEnd` `:775`; `peak_frame_time_ms`
   is the un-smoothed max of that span (`:782-783`, reset on read: `Renderer.h:371`). Compositing (ring creation) is
   inside the window -- the lane's 36 ms spikes were seen there. The recorder / Syphon / capture block (`:830-844`) runs
   AFTER `renderEnd`: a capture's cost is INVISIBLE to `peak_frame_time_ms`/`frame_time_ms` (it delays the next frame
   instead). Hence C0's log-line timing: the frame timer cannot be the capture witness, and its window must not move
   (probe tolerances read it).
F10 (VERIFIED) Capture path: `Renderer::captureFrame` (`Renderer.cpp:2031-2071`) arms `pendingCapture_` under
   `captureMutex_` and blocks the CALLER (HTTP thread: TestServer `handleRenderFrame` `src/test/TestServer.cpp:588`;
   ApiServer `handleSnapshot` `src/api/ApiServer.cpp:711-719` via `takeSnapshot` `Renderer.cpp:2152`; OSC
   `/audiodna/snapshot` `src/osc/OscHandler.cpp:185` reaches the same) on a `std::future` for up to 5 s.
   `processPendingCapture` (`:2073-2149`, called at `:327`, `:636`, `:844` -- the two early paths and the normal end of
   frame) does, ON THE GL THREAD: `glReadPixels` of the whole canvas (`:2107-2109`), a per-pixel
   `bmp.setPixelColour(x, y, juce::Colour(r,g,b,a))` loop with the vertical flip (`:2112-2127`), PNG encode + file
   write (`:2130-2138`), then `set_value(ok)` (`:2147`). `takeSnapshot` has no UI caller (grep of `src/ui` = none).
F11 (VERIFIED) What `setPixelColour` writes (JUCE at `build/_deps/juce-src`): `Image::BitmapData::setPixelColour`
   (`modules/juce_graphics/images/juce_Image.cpp:484-499`) = `((PixelARGB*) getPixelPointer(x,y))->set
   (colour.getPixelARGB())`; `Colour(r,g,b,a)` stores non-premultiplied ARGB (`colour/juce_Colour.cpp:242-245`);
   `getPixelARGB()` = `PixelARGB(argb).premultiply()` (`juce_Colour.cpp:303-308`); `premultiply()` (`colour/
   juce_PixelFormats.h:240-259`): a==255 -> unchanged, a==0 -> RGB zeroed, else `c = (c*a + 0x7f) >> 8`; memory
   layout on little-endian macOS is B,G,R,A (`juce_PixelFormats.h:323-336`); `PixelARGB(a,r,g,b)` ctor
   (`:73-79`) sets the same components; `set()` copies the native uint32 (`:127-129`);
   `getPixelPointer(x,y) = data + y*lineStride + x*pixelStride` (`images/juce_Image.h:343`), `getLinePointer(y)`
   (`:337`). Both backings have `lineStride = (pixelStride*w + 3) & ~3` (software `juce_Image.cpp:187`; CoreGraphics
   `native/juce_CoreGraphicsContext_mac.mm:49`, layout `kCGImageAlphaPremultipliedFirst|kCGBitmapByteOrder32Little`
   `:174` = the same B,G,R,A). The 4-arg `juce::Image` ctor uses `NativeImageType` (`juce_Image.cpp:280`). The PNG
   writer (`image_formats/juce_PNGLoader.cpp:556-632`) sets IHDR/sBIT only -- no tIME chunk, no time-dependent state;
   it un-premultiplies each `PixelARGB` (`:604-605`): the PNG bytes are a pure function of the ARGB bytes.
F12 (VERIFIED) Today `juce::FileOutputStream fos` is declared at function scope (`Renderer.cpp:2131`) and the promise
   resolves at `:2147` BEFORE `fos` is destroyed at `:2149`; `FileOutputStream` flushes its tail buffer only in
   `flush()` or its destructor (`modules/juce_core/files/juce_FileOutputStream.cpp:47-49, 69, 82`; the PNG writer
   never flushes, `juce_PNGLoader.cpp:626-632`). So the waiting caller can wake -- and answer HTTP -- while up to the
   buffer's tail of the PNG is unflushed. C3 closes this by writing on the caller's thread inside a scope that ends
   before the return.
F13 (VERIFIED) Other readers: `VideoRecorder::submitFrame` (`src/recording/VideoRecorder.cpp:101-139`) does a
   `glReadPixels` into a triple buffer and the encoder thread converts with `sws_scale` (negative stride flip,
   `encodeFrame`) -- NO per-pixel loop, not touched. Syphon `publishSyphonFrame` (`Renderer.cpp:837-840`) blits and
   publishes a texture -- no readback, not touched (plan5 s14: the Syphon path must not change). TestServer
   `handleOutputProbe` (`TestServer.cpp:1630-1745`) runs the SAME per-pixel loop with alpha forced to 255 (`:1716-
   1722`) -- on the HTTP thread with a private CGL context: zero render-thread cost; and `TestServer.cpp` is inside the
   concurrent C2 lane's fence (plan5-final.md s11 C2 lists `src/test/TestServer.h/.cpp` + `src/api/ApiServer.h/.cpp`;
   worktree `lane/outputs-c2-0927` is live). Deferred (C4).
F14 (VERIFIED) Probes: every render probe greps the err.log for `Captured frame: [^ ]*` (path = first token:
   `.harmony/probe-canvas.sh:64`, `probe-render-state.sh:74`, `probe-fitmode.sh:64`, `probe-deck-clock.sh:66`,
   `probe-outputs.sh:61`) -- the prefix `[Eyes] Captured frame: <path> (WxH)` must stay verbatim; anything APPENDED
   after the size is safe. `probe-render-state.py` `r1_counts` (`:425-451`) reads `peak_frame_time_ms` once after the
   first use (`s1`, printed only) and once 0.6 s after the first fade (`s2`, asserted <= `peakMaxMs` = 50.0 in
   `.harmony/probe-render-state.json` r1.counts) and asserts `frame_rings` +2 / `temporal_buffers` <= +2 over 5 fades.
   Perf rows run under `wait_no_compiler` (`probe-canvas.py`; pgrep clang / clang\+\+). The lane's polling script is
   `.harmony/.reports/s-rta-0927/routines-timing-evidence/scripts/t2.py` (7070, 15 ms cadence).
F15 (VERIFIED) `/api/state` emits `frame_rings` / `temporal_buffers` / `peak_frame_time_ms` in BOTH `ApiServer.cpp:
   1288-1296` and `TestServer.cpp:612-622` -- both files are in C2's fence (F13). ctest precedent for a headless
   `juce::Image`: `tests/test_lookandfeel_square.cpp:23-26` (`ScopedJuceInitialiser_GUI`, links `juce::juce_gui_basics`,
   block at `tests/CMakeLists.txt` "test_lookandfeel_square"); pure-header precedent `test_render_geometry`.
   ctest on main: 720 passes in `build/Testing/Temporary/LastTest.log`; live suite on main per 1636785's message:
   canvas 15/0, fitmode 10/0, outputs 10/0, render-state 31/0, manual-bpm 22/0.

## TRADEOFFS CONSIDERED

Ring (lane table re-checked against F1-F8):
- A status quo -- rejected: 36 ms twice per chain (r1_counts flaked at 51.86 ms under load; HANDOFF item 2).
- B create the spare with the primary ring -- rejected: first use doubles to ~72 ms (INFERRED 2x36) and +237 MiB for
  every Split/Stutter chain that never fades. HANDOFF.md:82-83 proposes exactly this; it is the wrong fix.
- C lazy per-cell allocation -- ACCEPTED: removes BOTH hitches (each push creates <= 1 cell, ~0.075 ms = 36 ms/480),
  no visible change (F3/F4: a cell is written before any read can return it), no key/handover change (Pitfall 35
  untouched), VRAM ceiling unchanged, ~30 lines in CompositorEngine plus a 15-line pure header. Cost: +~0.075 ms per
  frame for the first 480 pushes of each ring (8 s at 60 fps, 4 s at 120) -- below the frame timer's noise floor
  (steady 0.6-0.9 ms) and far below the adaptive-quality trigger (12 ms sustained 30 frames, `Renderer.h:478-479`).
- D amortised pre-warm -- rejected: first use still 36 ms, VRAM as B, more state (a per-ring warm cursor).
- E recycled pool -- rejected: the first-ever creation is still 36 ms; ownership across chains/decks/resize is the
  hardest option for the smallest gain.
- F one GL_TEXTURE_2D_ARRAY per ring -- INFEASIBLE, not merely costly: Frame Stutter hands the ring cell itself
  downstream as `currentInput` (F7) and Screen Split binds each cell as `sampler2D` (`:1868-1878`); a texture-array
  layer cannot be bound as a 2D texture in GL 4.1 (`glTextureView` is 4.3; CLAUDE.md rule 11), so F needs an extra copy
  pass per read plus shader work in `EmbeddedShaders.h`.
- G size the ring to the effect's reach -- rejected as THIS fix: Split's reach legitimately exceeds 480 (F7), so only
  Stutter-only chains shrink (31 cells, 15 MiB), and a live param increase must grow the ring = a new hitch at the
  worst moment. It is a real VRAM item for later and composes with C (a reach-bounded ring is just a smaller
  capacity); it is not a hitch fix.

Capture:
- Per-pixel `setPixelColour` -> row loop over `PixelARGB` (C2) -- accepted; byte-identical by construction (F11). It is
  not a literal `memcpy` (R and B swap, and alpha < 255 premultiplies); for opaque rows it is a pure swizzle.
- Encode on the GL thread (today) vs on the waiting caller (C3) -- C3 accepted: the caller is already blocked for the
  whole duration (F10), so its wait cannot get longer, while the GL thread stops paying convert + PNG (the PNG encode
  is INFERRED to be the largest single term at 1080p; C0 measures it). Same pixels, same frame, same file, same
  return value; the file is now complete before the return (F12). JUCE image + PNG work on an HTTP thread has a
  precedent in `handleOutputProbe` (F13).
- Async PBO readback -- OUT of scope. It removes only the CPU wait for the GPU drain, not the GPU work; a correct
  implementation defers the result by a frame (fence/poll) or blocks anyway, and `render_frame` must return the frame
  rendered after the request at the lock's size (`processPendingCapture` `:2081-2090`; plan5 s14 protects
  captureFrame/render_frame semantics and determinism). After C3 the GL thread's remaining cost is one 8.3 MB (1080p)
  / 33 MB (4K) readback -- C0/C3 numbers decide whether a PBO is ever worth its complexity for the per-frame RECORDER
  (a separate item); for a once-per-request capture it is not.
- Other loops: recorder (`sws_scale`) and Syphon (texture) share nothing (F13) -- untouched. `output_probe` shares the
  loop but costs the render thread nothing and sits in C2's fence -- C4, deferred.

## DECISION / SPEC

Files this lane touches (all under `src/render`, `tests`, `.harmony/probe-render-state.*`, docs): `src/render/
CompositorEngine.h/.cpp`, NEW `src/render/FrameRing.h`, NEW `src/render/PixelConvert.h`, `src/render/Renderer.h/.cpp`
(members `:617-622`, `captureFrame` `:2031-2071`, `processPendingCapture` `:2073-2149` ONLY), NEW `tests/test_frame_ring
.cpp`, NEW `tests/test_pixel_convert.cpp`, `tests/CMakeLists.txt` (two appended blocks), `.harmony/probe-render-state.py`
(`r1_counts`), `.harmony/probe-render-state.json` (r1.counts), `docs/claude/pitfalls.md:49`, `docs/claude/rendering.md:
69,71`, `.harmony/HANDOFF.md:82-84`. Nothing else.

### C0 -- capture timing on the existing log line (instrumentation only; the RED measurement)
`Renderer.cpp` `processPendingCapture`: take `std::chrono::steady_clock` stamps around (a) `glReadPixels` (`:2108-
2109`), (b) the conversion block (`:2112-2127`), (c) PNG encode + write (`:2130-2138`); change the success line
(`:2141-2142`) to
`[Eyes] Captured frame: <path> (WxH) read=R.R convert=C.C png=P.P ms` -- prefix verbatim, fields appended after the
size (F14). No behaviour change. Gate: build, `ctest` all green, `probe-canvas.sh` 15/0 (its grep still matches).
Record on THIS build, quiet (no compiler; print `uptime` load): 3x `render_frame` at 1920x1080 and 1x at 3840x2160 of
the byte-identity fixture below -> the RED table (read / convert / png / HTTP round trip). Expected (INFERRED):
read 3-10 ms, convert 20-40 ms, png 40-90 ms at 1080p; all ~4x at 4K.

### C1 -- ring: lazy per-cell allocation (option C)
1. NEW `src/render/FrameRing.h` (pure; no GL, no JUCE -- like `RenderGeometry.h`):
   ```cpp
   #pragma once
   // FrameRing: the pure index math of CompositorEngine::FrameRingBuffer (Screen Split / Frame Stutter).
   // writeIndex = the NEXT cell to be written; frameCount = cells written in this lifetime (saturates at capacity).
   // Every index readIndex returns lies in the written set {writeIndex-1, ..., writeIndex-frameCount} mod capacity --
   // the invariant that lets a cell be created on its first write (s-rta-0927 plan-renderperf C1);
   // tests/test_frame_ring.cpp. -1 = the ring holds nothing.
   namespace FrameRing
   {
   constexpr int readIndex(int writeIndex, int frameCount, int framesAgo, int capacity) noexcept
   {
       if (frameCount <= 0 || capacity <= 0)
           return -1;
       const int maxDelay = frameCount - 1;
       if (framesAgo > maxDelay) framesAgo = maxDelay;
       if (framesAgo < 0) framesAgo = 0;
       return (writeIndex - 1 - framesAgo + capacity * 2) % capacity;
   }
   } // namespace FrameRing
   ```
2. `CompositorEngine.h`: `struct FrameRingBuffer` (`:254-262`) gains `int allocatedCells = 0;` (cells created so far,
   for accounting). Add `std::atomic<int> frameRingCellCount_{ 0 };` next to `frameRingCount_` (`:269`) and
   `int getFrameRingCellCount() const { return frameRingCellCount_.load(std::memory_order_relaxed); }` next to
   `getFrameRingCount()` (`:148`). Rewrite the comment block `:247-251`: cells are created on first write, one per
   pushed frame, so a ring reaches its 237 MiB (1080p/4K) only after 480 pushed frames and no frame ever creates more
   than one cell per ring; the 480 cap and `ringDownscale` are unchanged.
3. `CompositorEngine.cpp` `getOrCreateRingBuffer` (`:1671-1713`):
   - "Release old" branch (`:1683-1691`): also `frameRingCellCount_.fetch_sub(ring.allocatedCells); ring.allocatedCells
     = 0;`.
   - Replace the two `resize(kMaxRingFrames, 0)` calls (`:1697-1698`) with `assign(static_cast<size_t>(kMaxRingFrames),
     0)` for BOTH vectors -- on a size-change re-init the vectors already hold the just-deleted handles; `resize` would
     keep them non-zero and the lazy check below would bind a deleted FBO. This line is load-bearing.
   - DELETE the creation loop and its trailing `glBindFramebuffer(GL_FRAMEBUFFER, 0)` (`:1700-1707`). Keep
     `initialized = true` and the `frameRingCount_` increment (`:1709-1710`) -- `frame_rings` keeps counting rings from
     initialisation exactly as today (probe `r1_counts` dr == 2 and `r2_ring` depend on it).
   - Keep the function's header comment honest: it no longer creates GL objects (it may still delete old ones).
4. `pushFrameToRing` (`:1762-1783`): after the `prog` check and `idx` computation, BEFORE `glBindFramebuffer`:
   ```cpp
   if (ring.textures[idx] == 0)   // plan-renderperf C1: a cell is created on its first write, never in bulk.
   {                              // createFBO leaves framebuffer 0 bound and rebinds GL_TEXTURE_2D on the active
       createFBO(ring.fbos[idx], ring.textures[idx], ring.ringWidth, ring.ringHeight);   // unit (R5) -- both are
       ++ring.allocatedCells;                                                             // re-bound right below.
       frameRingCellCount_.fetch_add(1, std::memory_order_relaxed);
   }
   ```
   No `glClear`: the passthrough draw that follows overwrites every byte of the cell (F3), and no read can return this
   cell before this push completes (F4). The rest of the function is unchanged.
5. `getFrameFromRing` (`:1785-1796`):
   ```cpp
   if (!ring.initialized) return 0;
   const int idx = FrameRing::readIndex(ring.writeIndex, ring.frameCount, framesAgo, kMaxRingFrames);
   return idx < 0 ? 0 : ring.textures[static_cast<size_t>(idx)];
   ```
   (`#include "FrameRing.h"`.) Identical results to the inline expression for every input (test case 1 proves it).
6. Release paths: `rescaleHistory` (`:149-160`) subtracts `ring.allocatedCells` from `frameRingCellCount_` before
   `ring = FrameRingBuffer{}`; the destructor (`:71-80`) stores 0. `handOverClipHistory` (`:1715-1760`) is UNCHANGED:
   the swap moves allocated cells with the ring; the incoming ring's reset (`frameCount`/`writeIndex` = 0) keeps its
   cells (reused on the next pushes, no creation). `allocatedCells` travels with the struct in the swap.
7. NEW `tests/test_frame_ring.cpp` (Catch2, pure; register like `test_render_geometry`: `Catch2::Catch2WithMain` +
   `juce::juce_core` is not even needed -- mirror `test_ring_buffer`'s block, `tests/CMakeLists.txt:19-24`):
   - "readIndex equals the historical inline expression" -- brute force capacity 480: writeIndex 0..479 x frameCount
     1..480 x framesAgo in {0..frameCount+3, 3780, -1}: compare with the copied old expression after the copied clamp.
   - "an index readIndex returns has always been written in the current lifetime" -- simulate 1500 pushes with the
     two-line advance copied from `pushFrameToRing` (mark `written[idx]`), a swap-style reset at push 700 (`frameCount
     = writeIndex = 0`, `written` cleared -- the incoming ring starts empty), and at every step assert `written[
     readIndex(...)]` for framesAgo 0..600 and 3780. RED-first: with the reset's `written` NOT cleared the test must
     still pass; with a deliberately wrong formula (e.g. `- framesAgo` -> `+ framesAgo`) it must fail -- the builder
     runs both mutations once to prove the test bites.
   - "empty ring / after reset returns -1 until the next push".
8. Live rows (`.harmony/probe-render-state.py` `r1_counts` `:425-451`, `.harmony/probe-render-state.json` r1.counts):
   - `peakMaxMs`: 50.0 -> 16.7; `_why` gains: "one 60 Hz frame period: a hitch under it cannot drop a frame; RED on
     1636785 35.5-38.5 ms (routines-timing T2), GREEN expected <= 6 ms (the 2nd fade's 1.2-5.9 ms = the same work
     without creation)".
   - Assert `s1` (first use, `:431`) against the same bar -- today it is only printed; the first-use hitch is the same
     defect. Insert a second-fade read for context: after `time.sleep(T)` at `:434`, `trig(0, 2); time.sleep(0.6);
     s2b = state(); time.sleep(T)`, then `for c in range(3, 6)` (still 5 fades, still dr == 2). Print `s2b`'s peak in
     the existing print line ("second fade (no creation) X ms"); no assertion on it.
   - The row runs under the rig's quiet rule (perf rows: `wait_no_compiler`; print `os.getloadavg()` in the row's
     print line as t2.py did). Expected: render-state 31/0 -> 32/0 (one new PASS line).
9. Hitch table (the dispatch's measurement row, both builds): run t2.py (F14) 3x at 1920x1080 and 1x at 3840x2160 on
   the pre-C1 build and on the C1 build, quiet, load printed. Add `S.headers["Connection"] = "close"` to the copy (the
   cpp-httplib 5 s keep-alive drop, c1-state-fix). Report per run: first use peak, first fade peak, second fade peak,
   steady median/max, `frame_rings` before/after. RED = today's 35.5-38.5 ms; GREEN = both first-use and first-fade
   peaks <= 16.7 ms on all 4 runs AND within 2x of the same run's second-fade peak (the relative check catches a fix
   that merely moves the cost). If a GREEN run shows a first-fade peak between 6 and 16.7 ms, look at the temporal-
   buffer path (F2's spare temporal buffer, canvas-size, 7.9 MiB at 1080p) before accepting -- it is the next
   candidate and is NOT touched by C1.
10. Docs: `docs/claude/pitfalls.md:49` (Pitfall 20) -- append: cells are stored at `ringDownscale` (never wider than
    480 px, 237 MiB per full ring at 1080p and 4K) and CREATED ON FIRST WRITE, one per pushed frame -- never allocate a
    ring in bulk inside a frame (36 ms measured, s-rta-0927); a cell is never read before it is written
    (`FrameRing::readIndex`, `tests/test_frame_ring.cpp`). `docs/claude/rendering.md:69`: same two sentences; `:71`:
    replace "The one-time hitch (ring recreation ~18-24 ms per ring) shows in peak_frame_time_ms" with "ring cells are
    recreated lazily, one per frame -- no hitch". `.harmony/HANDOFF.md:82-83` item 2: fixed (C, not B), cite this plan
    and the GREEN table. CLAUDE.md pitfall index line 20 is unchanged.

### C2 -- capture: byte-identical row conversion
1. NEW `src/render/PixelConvert.h` (pure; JUCE graphics types, no GL):
   ```cpp
   #pragma once
   #include <juce_graphics/juce_graphics.h>
   #include <cstdint>
   // PixelConvert: GL RGBA8 rows (bottom-up, tightly packed w*4) -> a juce::Image::ARGB BitmapData, byte-for-byte what
   // the per-pixel setPixelColour(x, y, Colour(r, g, b, a)) loop produced (premultiplied B,G,R,A; forceOpaque = the
   // output_probe variant, alpha 255). Proof: tests/test_pixel_convert.cpp (s-rta-0927 plan-renderperf C2).
   namespace PixelConvert
   {
   inline void rgbaBottomUpToARGB(const uint8_t* rgba, int w, int h, juce::Image::BitmapData& dst, bool forceOpaque) noexcept
   {
       jassert(dst.pixelFormat == juce::Image::ARGB && dst.pixelStride == 4 && dst.width == w && dst.height == h);
       for (int y = 0; y < h; ++y)
       {
           const uint8_t* src = rgba + static_cast<size_t>(h - 1 - y) * static_cast<size_t>(w) * 4;
           auto* out = reinterpret_cast<juce::PixelARGB*>(dst.getLinePointer(y));
           for (int x = 0; x < w; ++x, src += 4)
           {
               juce::PixelARGB p(forceOpaque ? uint8_t(255) : src[3], src[0], src[1], src[2]);
               p.premultiply();
               out[x].set(p);
           }
       }
   }
   } // namespace PixelConvert
   ```
   Identity argument (F11): same address (`getLinePointer(y) + 4x` == `getPixelPointer(x,y)` at pixelStride 4), same
   components (`PixelARGB(a,r,g,b)` == `Colour(r,g,b,a).argb`), same `premultiply()`, same `set()`. The oracle's
   `Colour` construction, `getPixelARGB()` temporary and per-pixel format switch are what the loop drops.
2. `Renderer.cpp` `processPendingCapture`: replace `:2112-2127` with
   `juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
    PixelConvert::rgbaBottomUpToARGB(pixels.data(), readW, readH, bmp, false);` (still on the GL thread in this
   commit). Timing line from C0 now shows the new `convert=`.
3. NEW `tests/test_pixel_convert.cpp` (Catch2; register by copying `test_lookandfeel_square`'s block: links
   `juce::juce_gui_basics`, same defines/warnings; each case opens with `juce::ScopedJuceInitialiser_GUI gui;` -- the
   known-good headless-image precedent, F15). The oracle `referenceLoop(rgba, w, h, bmp, forceOpaque)` is the OLD loop
   copied verbatim from `Renderer.cpp:2114-2126` (forceOpaque: `TestServer.cpp:1718-1722`). Helper `sameBytes(imgA,
   imgB)`: equal `lineStride`/`pixelStride`, then `memcmp(getLinePointer(y), ..., w*4)` per row. Cases:
   - "every alpha, every backing": a 256x256 buffer with A = x, R = y, G = (7x+3y)&255, B = x^y -- oracle vs
     candidate on `juce::Image(ARGB, w, h, false)` (default = Native) AND on `juce::Image(ARGB, w, h, false,
     juce::SoftwareImageType())`. Covers a==0, a==255 and all 254 premultiply values.
   - "forceOpaque matches the output_probe loop" (same buffer, alpha forced).
   - "odd sizes": 131x77, 1x1, 1x9, 9x1 from a seeded LCG.
   - "PNG bytes identical": encode oracle and candidate images with `juce::PNGImageFormat().writeImageToStream` into
     two `juce::MemoryOutputStream`s -> equal size, `memcmp` equal. (Proves the PNG claim headless: F11's writer is a
     pure function of the bytes.)
   - RED-first proof: run once with the candidate's `premultiply()` line commented out -> the every-alpha case must
     FAIL (alpha < 255 rows differ) and the forceOpaque case must still PASS; restore.
4. Live byte-identity A/B (the probes decode PNGs with PIL, they do not hash them -- F14 -- so this is a separate
   check, run by the builder under the live-lock rig rules of `probe-render-state.sh:1-70`: `open -g`, refuse if
   running, fresh out dir): fixture = one deck, one layer with `"opacity": 0.5` (alpha < 255 over the whole picture ->
   the premultiply branch is exercised live), one clip = `media/P16_01_baseline.png`, no effects, no mappings;
   `render_frame` `{time: 0.0}` at the default 1920x1080 and at `width/height` 1280x720. Steps: (i) pre-C2 build,
   capture twice -> `shasum -a 256` equal (determinism baseline; if THIS fails the live A/B is void and the ctest is
   the proof -- say so); (ii) C2 build, same fixture -> hashes equal to (i); (iii) repeat (ii) after C3. Also decode
   one pair with PIL and assert `np.array_equal` (guards against a hash of an empty file).
5. Probes unchanged: `probe-canvas.sh` 15/0, `probe-effects-parity.sh` 46/0, `probe-render-state.sh` 32/0 (after C1),
   `probe-fitmode.sh` 10/0 -- every one on the C2 build.

### C3 -- capture: only glReadPixels stays on the GL thread
1. `Renderer.h` (`:617-622`): add `std::vector<uint8_t> capturePixels_; int captureReadW_ = 0, captureReadH_ = 0;
   double captureReadMs_ = 0.0;` -- GL thread writes, the waiting caller reads, both under `captureMutex_`.
   `captureOutputPath_` (`:619`) becomes unused by the GL thread (the caller has `outputPath`); remove it only if
   nothing else reads it (grep) -- otherwise leave it.
2. `processPendingCapture` after the invalid-dims branch (`:2105`): resize `capturePixels_`, bind `canvasFBO_` for
   reading, `glReadPixels` into it (timed -> `captureReadMs_`), set `captureReadW_/H_`, `set_value(true)`, clear
   `pendingCapture_`/`capturePromise_`. DELETE the image/PNG/file/log-success code (`:2111-2145`). Keep "[Eyes]
   Processing capture: canvas WxH" and "[Eyes] Invalid capture dimensions" as they are.
3. `captureFrame` after the timeout branch (`:2070`): `if (!future.get()) return false;` then, under `captureMutex_`,
   move `capturePixels_` out with `w/h/readMs`; convert with `PixelConvert::rgbaBottomUpToARGB(..., false)` into
   `juce::Image(juce::Image::ARGB, w, h, false)` (same ctor as today: Native backing); `outputPath.getParentDirectory()
   .createDirectory()`; write the PNG inside a `{ juce::FileOutputStream fos(outputPath); ... }` scope (flushed before
   the return -- F12); then the C0 line `[Eyes] Captured frame: <path> (WxH) read=.. convert=.. png=.. ms` on success
   or `[Eyes] Failed to write PNG: <path>`; return ok. `takeSnapshot` (`:2152`) and both HTTP callers are unchanged:
   they already ran on the caller's thread and already blocked for the whole duration.
4. Semantics preserved, point by point: the pixels are read on the GL thread at the same place in the same frame
   (after `presentCanvas`, before the window FBO rebind), still gated on the lock size (`:2081-2090`); the return
   value is still "file written"; the same PNG bytes (C2's proof + the live A/B (iii)); the 5 s timeout path is
   unchanged; no new mutex (the existing `captureMutex_` is taken once more, by the caller, for a `std::move`).
5. GREEN witness (C0's line, same fixture, quiet, 3x 1080p + 1x 4K): the GL-thread part is now `read` alone. Bar:
   `read` <= 16.7 ms at 1080p (INFERRED achievable: one 8.3 MB readback; if it measures above the bar the residual is
   the GPU drain, which no CPU-side change removes -- report it and file the PBO question for the recorder item; the
   commit still stands on the convert/png removal). Also report the HTTP round trip before/after: it shrinks only by
   the convert saving (the PNG encode still happens before the response) -- state this in the report so nobody
   expects 98-159 ms to become 10 ms.
6. A live snapshot check (Boris-visible, optional): `curl -X POST 127.0.0.1:7070/api/snapshot` while a clip plays at
   1080p -- before C3 the picture freezes for the whole capture, after C3 for `read` only. Not gated (no automated
   frame-gap metric exists: F9).

### C4 -- DEFERRED (fence-gated; not part of this lane's gate)
After `lane/outputs-c2-0927` merges (or Harmony grants the two files): (a) emit `frame_ring_cells` =
`getFrameRingCellCount()` next to `frame_rings` in `ApiServer.cpp:1292` and `TestServer.cpp:618` (additive field);
add probe row `r1_cells`: 0.3 s after the layer's first use `1 <= cells < 480` (RED today: 480 immediately) and after
the 5 fades `cells <= 960`; (b) `handleOutputProbe` (`TestServer.cpp:1716-1722`) -> `PixelConvert::rgbaBottomUpToARGB
(..., true)` (its test case already exists). Until then VRAM accounting is analytic (below).

## VRAM ACCOUNTING (F8; unchanged ceiling, changed timing)
| canvas | ds | cell | B/cell | per full ring | growth per pushed frame | full after |
|---|---|---|---|---|---|---|
| 1280x720 | 4 | 320x180 | 230,400 | 105.5 MiB | 0.22 MiB | 480 frames |
| 1920x1080 | 4 | 480x270 | 518,400 | 237.3 MiB | 0.49 MiB | 8 s @60 / 4 s @120 |
| 2560x1440 | 6 | 426x240 | 408,960 | 187.2 MiB | 0.39 MiB | idem |
| 3840x2160 | 8 | 480x270 | 518,400 | 237.3 MiB | 0.49 MiB | idem |
Per (deck, layer) clip chain that has crossfaded with Split/Stutter on both sides: 2 rings (the outgoing slot,
Pitfall 35) -> 474.6 MiB ceiling at 1080p/4K, as today. A chain that shows Split/Stutter for less than 480 frames now
holds proportionally less. `rescaleHistory` frees all cells (as today) and the new canvas size refills lazily -- the
"18-24 ms per ring" recreation hitch in rendering.md:71 disappears with it. The spare TEMPORAL buffer created at the
first fade (canvas-size, 7.9 MiB at 1080p / 31.6 MiB at 4K, one FBO) is NOT part of this plan.

## MUST NOT CHANGE
`kMaxRingFrames` = 480, `RenderGeometry::ringDownscale`, the `LayerStateKey` scheme and `handOverClipHistory`'s
copy/swap (Pitfall 35), `frame_rings`/`temporal_buffers` semantics (r1_counts dr == 2, r2_ring), the frame timer window
`Renderer.cpp:529-783` (probe tolerances read `peak_frame_time_ms`), what a capture returns (the canvas of the frame
rendered after the request, at the lock's size, synchronously read), the `[Eyes] Captured frame: <path> (WxH)` prefix
and `[Eyes] Processing capture:`, 7070/8080 request/response shapes (C4's field is additive and deferred), the 5 s
capture timeout, no PBO, no new mutex on any GL frame path, `VideoRecorder`, the Syphon path, `EmbeddedShaders.h`,
`OutputPresenter`/`SharedFrameSet`, every existing probe's tolerance except the one this plan tightens (r1.counts
`peakMaxMs`), the ctest set (additive only).

## FENCE
Concurrent lanes: plan5 C2 (`src/output/*`, `OutputWindow`, `MenuBarModel`, `TopBar`, `MainComponent` output code, AND
per plan5 s11 `TestServer.h/.cpp` + `ApiServer.h/.cpp`) -- this lane touches none of them (C4 deferred for that
reason); its `Renderer.cpp` edits are `:617-622` and `:2031-2150`, far from the output tap at `:764`
(`publishToOutputs`) -- a rebase is textual-conflict-free (INFERRED from the regions; the builder rebases before each
commit). Routine beat clock lane (`RecorderClock`, routine engine, BPM) -- no shared file.

## RISKS (strongest counterargument first)
R1 Strongest counterargument to C: "G removes 94 % of the ring's VRAM for Stutter-only chains; C keeps 237 MiB per
   ring forever". It loses because the dispatch's defect is the hitch, not the ceiling: G leaves Screen Split's ring at
   480 (F7), adds a grow-time hitch on a live param change, and composes with C later (a smaller capacity is a one-
   constant change once cells are lazy). Second counterargument: "F is the proper GL design" -- infeasible under GL 4.1
   without a copy pass (F7, TRADEOFFS).
R2 A lazily-created cell read before its first write would show as a black/garbage cell. Guarded three ways: the
   index-set invariant (F3/F4) pinned by `test_frame_ring` on the REAL function, the existing 0-texture fallbacks (F4),
   and the live rows r1_ring/r2_ring/r5_hold (pictures). Residual: a stale non-zero handle after a size-change re-init
   -- closed by the `assign` (C1 step 3); the builder must not "simplify" it back to `resize`.
R3 GL state at the lazy create (R5 class): `createFBO` rebinds `GL_TEXTURE_2D` on the active unit and leaves FBO 0
   bound. `pushFrameToRing` rebinds both immediately after (F6), and the bulk creation already happened at this exact
   point today. Verified by the picture rows, not by reasoning alone.
R4 The tightened 16.7 ms bar can flake under load (the 50 ms bar flaked once at 51.86 ms at load ~15). Mitigation:
   the perf rows run quiet (`wait_no_compiler`, load printed); the relative check (<= 2x the second fade) is the
   diagnostic when a run is loaded. Media upload cost sits inside both windows (first use = image A's first upload +
   ring; first fade = image B's + spare) and measured the same 36 +- 1 ms on both (lane table), so it is inside the
   noise; expected post-fix peaks 1-6 ms.
R5 C3 moves 8-33 MB of allocation and a PNG encode onto an HTTP thread (cpp-httplib worker). The same thread already
   blocked for that long (F10) and `handleOutputProbe` already does image + PNG work there (F13). A snapshot's HTTP
   response time is unchanged; only the GL thread gains.
R6 The `read` bar (16.7 ms at 1080p) is INFERRED. If the drain dominates, C3 still removes convert + png; the report
   states the measured split and leaves PBO for the recorder item. 4K numbers are reported, not gated.
R7 Byte-identity rests on the JUCE version in `build/_deps/juce-src` (F11 lines). The ctest links the same JUCE, so a
   JUCE bump that changed `premultiply` would fail the test rather than silently drift.
R8 `frame_ring_cells` deferral: no live structural witness of lazy growth until C4 -- the peak rows and the VRAM table
   carry the claim; the reviewer should read `pushFrameToRing`'s diff, it is ~8 lines.

## COMMIT SEQUENCE (each independently green, each reverts alone)
1. `perf(capture): time read/convert/png on the [Eyes] Captured frame line` -- C0. Gate: build, ctest green,
   probe-canvas 15/0; RED table recorded (3x 1080p, 1x 4K, load).
2. `perf(ring): create frame-ring cells on first write (option C) + FrameRing.h + test_frame_ring + r1_counts bar 16.7`
   -- C1. Gate: ctest green incl. the mutation proof; probe-render-state 32/0 x3 quiet; hitch table GREEN (4 runs);
   probe-effects-parity 46/0; docs (pitfalls 20, rendering.md, CompositorEngine.h comment, HANDOFF item 2).
3. `perf(capture): PixelConvert row conversion, byte-identical (test_pixel_convert)` -- C2. Gate: ctest green incl.
   the premultiply-mutation proof; live A/B hashes (i)==(ii); probe-canvas 15/0, fitmode 10/0, render-state 32/0.
4. `perf(capture): only glReadPixels on the GL thread; convert+PNG on the waiting caller` -- C3. Gate: live A/B (iii)
   equal; GREEN line table (read / convert / png at 1080p + 4K); probe-canvas 15/0, effects-parity 46/0, render-state
   32/0, outputs 10/0 (output_probe untouched but the err.log grep changed shape -- confirm 10/0); HANDOFF item 4.
5. (deferred, separate dispatch after C2-lane merge) C4.
Commit trailer: `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`.

## DONE LOOKS LIKE
ctest 720 -> 720 + test_frame_ring (3 cases) + test_pixel_convert (4 cases), all green; probe-render-state 32/0 with
`peakMaxMs` 16.7 on both first use and first fade; hitch table: first-use and first-fade peaks <= 16.7 ms on 3x 1080p
+ 1x 4K quiet runs (today 35.5-38.5); live PNG hashes identical across pre-C2 / C2 / C3 builds on the opacity-0.5
fixture at 1080p and 720p; the `[Eyes] Captured frame` line shows the GL-thread cost = `read` only; no file outside
this plan's list touched; docs updated as in C1 step 10 and HANDOFF items 2 and 4 closed.

REPORT_FILE: .harmony/.reports/s-rta-0927/plan-renderperf.md
STATUS: FINAL -- ring: option C specified to the line (lazy cell at first write, FrameRing.h invariant test, r1_counts
bar 50 -> 16.7 on first use AND first fade, VRAM ceiling unchanged); capture: C0 instrumentation, C2 byte-identical
PixelConvert with a real-setPixelColour + PNG-bytes ctest, C3 read-only on the GL thread; PBO OUT of scope; output_probe
+ frame_ring_cells deferred behind the C2 lane's fence.
