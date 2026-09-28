# plan-renderleft -- the render leftovers R1-R5 (s-rta-0928)

Architect plan. Base: main @ 6db8d67 (code = 233eae7; `git diff --stat 233eae7 6db8d67 -- src tests
.harmony/probe-render-state.*` is EMPTY, VERIFIED) -- every file:line below is at HEAD. Bins: VERIFIED (read at the
line, or measured in a cited report) / INFERRED (derived) / ASSUMED (not checked -- the builder checks it).

QUESTION: R1 take image decode + pixel conversion off the GL thread on every image path, and rule on what a layer
shows meanwhile, preload vs lazy, the cache, the callers, shutdown and Eyes capture determinism; R2 concurrent
captureFrame callers; R3 the PNG encode in the capture round trip; R4 the 4K warm first-use / first-fade leftover;
R5 C4 (frame_ring_cells + r1_cells; output_probe through PixelConvert). One commit group per item, RED-first rows.

APPROACH (stated first):
- R1 A header-only `ImageDecode::Decoder` (juce::ThreadPool, 3 low-priority threads, never a GL call) decodes every
  image file and converts it to GL-ready RGBA rows with a PixelConvert row loop byte-identical to today's per-pixel
  loops. The GL thread only uploads (glTexImage2D), before any pass, one image-sized upload per frame. A texture that
  is not resident yet is PENDING, never "no media". While pending, an active-deck layer HOLDS its last composited
  picture (the Layer Router output compositeDeck already saves every frame). A Mask holds its previous mask image. A
  persistent layer draws nothing. A transition's outgoing image shows the incoming clip alone (today's prevTex == 0
  path). A capture is answered only by a frame with nothing pending, so Eyes and probe captures stay deterministic.
  Paths covered:
  - the compositor's clip images (all 4 getKeyTexture sites);
  - the legacy single image (Eyes load_image, every UI/REST clip trigger or selection, the slideshow). It is decoded
    and uploaded only when a frame actually needs it; the slideshow prefetches its next image;
  - image sequences (look-ahead 3 frames; the last frame repeats while the current one is late).
  Every composition swap prefetches the composition's image clips (one prefetch in flight, 1 GiB budget) and releases
  the textures of images no longer in it (today they are never released). A new whole-callback metric,
  peak_callback_ms, gates the rows, because one of the decodes runs outside the frame timer (next bullet).
- FOUND while reading (code path VERIFIED, magnitude INFERRED). Every UI/REST trigger of an image clip, and every
  cell selection, ALSO decodes that image on the GL thread at the top of the next frame. This is the legacy
  single-image path, and it runs outside the renderStart..renderEnd window that peak_frame_time_ms measures. The
  renderperf hitch rows could not see it (F2).
- R2 One capture at a time, from arm to read. A caller-side std::timed_mutex in captureFrame (never taken on the GL
  thread) also owns the time override and the TEST-ONLY canvas lock, which moves in from TestServer. It is released
  before convert + encode, so concurrent callers still overlap their PNG work. RED rows: 4 concurrent 7070 captures
  (3 time out today), and 2 concurrent 8080 captures at two sizes.
- R3 The encode already runs off the GL thread (C3, VERIFIED F9).
  - Measure the split on main.
  - Then give render_frame (7070 + 8080) a fast PNG writer: zlib level 1 through juce::GZIPCompressorOutputStream,
    filter 0, the same straight RGBA JUCE's writer emits. Decoded pixels are identical; file bytes differ.
  - Snapshots keep JUCE's writer. Delete-first, like writeReplacing.
  - Ship it only if the 1080p encode gets >= 2x faster.
- R4 Measurement first: 4 t2warm variants at 4K attribute the ~2.6 ms excess (transition target, effect pool, or the
  fresh layer's router FBO). Fix only if resize-created targets are VERIFIED as the cause: clear them once at
  creation. Otherwise file it with the table.
- R5 frame_ring_cells in both /api/state + probe row r1_cells; output_probe -> PixelConvert::rgbaBottomUpToARGB(..., true).

## FACTS VERIFIED IN THE SOURCE

F1 (VERIFIED) `CompositorEngine::loadKeyImage` (`src/render/CompositorEngine.cpp:243-297`):
   - path-keyed cache hit (:245-248);
   - `juce::ImageFileFormat::loadFrom` (:250) and `convertedToFormat(ARGB)` (:255);
   - a per-pixel `bmp.getPixelColour` loop (:265-280) and a flip copy (:282-287);
   - `glTexImage2D` + 4 `glTexParameteri` (:258-293), all on the GL thread, inside the frame that first shows the image.
   Callers:
   - its only caller is `getKeyTexture` (:299-306, call at :305);
   - `getKeyTexture` is called by compositeDeck Opaque/Transparent (:985), Mask (:1075),
     compositePersistentLayers (:1211) and `getClipTexture` (:1440, used by applyTransition for the OUTGOING clip at
     :1531).
   Failure and cache:
   - A failed decode returns 0, uncached, so it is retried every frame (:251-252).
   - `textureCache_` (`CompositorEngine.h:199`) is keyed by path only and never evicted; it is freed only in
     `releaseGL` (:37-42).
F2 (VERIFIED path) The legacy single image is re-decoded on every trigger or selection.
   Path:
   - `MainComponent::handleClipTrigger` (the REST `/api/trigger_clip` target, `MainComponent.cpp:1888`), the column
     trigger, the cell selection and `refreshPreviewFromActiveClip` all call `previewPanel_.loadImage(clip->mediaFile)`
     for Image clips (`MainComponent.cpp:721, 4326, 4529, 4584`).
   - That goes through `PreviewPanel::loadImage` (`src/ui/PreviewPanel.cpp:55-61`) to `Renderer::loadImage` (flag
     under `pendingImageMutex_`, `Renderer.cpp:62-68`).
   - The next frame's top (`Renderer.cpp:175-192`) then runs `TextureManager::loadImage`: decode + row swizzle +
     upload (`TextureManager.cpp:13-25, 27-94`), with NO cache.
   Consequences:
   - It runs for every trigger or selection, warm or cold, and in deck mode, where that texture is shown only when
     the compositor draws nothing (`Renderer.cpp:628-633`).
   - It runs BEFORE `renderStart` (`Renderer.cpp:533`), so peak_frame_time_ms (`:779-787`) never sees it.
   - The slideshow drives the same path on every advance (`MainComponent.cpp:3890-3953`: `previewPanel_.loadImage`),
     and so does Eyes `load_image` (`TestServer.cpp:298`, `ApiServer.cpp:959`, each followed by `sleep(100)`).
   - Magnitude INFERRED: about the decode + convert + upload of that image; R1.0 measures it.
F3 (VERIFIED) What the frame timer misses. Window: `renderStart` :533 to `renderEnd` :779. Outside it:
   - before: the pending legacy image (:175-192), the camera upload (:194-203), autopilot, genre, playlist (:352-491);
   - after: the recorder, Syphon and `processPendingCapture` (:835-848).
   A whole-callback measure needs an RAII timer at the first line of `renderOpenGL` (:164), because there are three
   returns (:327-334, :635-642, end).
F4 (VERIFIED) The renderperf image numbers are for small images. Its fixtures are 756x878 (0.66 MP;
   `sips media/P16_01_baseline.png`, `P16_02_Screen_Split_2x2.png`). renderperf's "9-14 ms per new image path"
   (renderperf.md:42-45) is for that size. INFERRED: decode, per-pixel copy and upload are all O(pixels), so
   1920x1080 (2.07 MP) and 3840x2160 (8.29 MP) stills cost ~3x and ~12x. R1.0 measures them.
F5 (VERIFIED) On macOS, JUCE decodes PNG/JPEG through CoreImage/ImageIO:
   - `JUCE_USE_COREIMAGE_LOADER` defaults to 1 (`juce_graphics.h:77-79`), used by `juce_graphics.cpp:131-135` and
     `PNGImageFormat::decodeImage` (`juce_PNGLoader.cpp:541-548`);
   - the decoded image is always `Image::ARGB` NativeImageType (`juce_CoreGraphicsContext_mac.mm:1027`).
   There is an in-repo precedent for off-thread decode:
   - `FilesBrowser` decodes with `ImageFileFormat::loadFrom` on a `juce::ThreadPool` (`src/ui/FilesBrowser.cpp:565-590`,
     pool `FilesBrowser.h:92-94`);
   - it reads the file's mtime on the decode thread, next to the decode (:575-580);
   - it drains the pool FIRST in the destructor with `removeAllJobs(true, 5000)` (:333-345).
   JUCE 8.0.4 API: `ThreadPool(const ThreadPoolOptions&)`, `addJob(std::function<void()>)`, `removeAllJobs`, and
   `withDesiredThreadPriority` (`juce_ThreadPool.h:166-198, 223, 301, 335`).
F6 (VERIFIED) The three conversion loops, and what each writes:
   - loadKeyImage and `ImageSequence::loadImageToTexture` (`ImageSequence.cpp:249-266`) write
     `getPixelColour(x, y)`, which is `Colour(PixelARGB::getUnpremultiplied())` (`juce_Image.cpp:466-474`;
     unpremultiply `juce_PixelFormats.h:262-281`): STRAIGHT RGBA, bottom-up.
   - `TextureManager::uploadImage` (`TextureManager.cpp:37-57`) writes the raw premultiplied bytes swizzled
     B,G,R,A -> R,G,B,A, flipped: PREMULTIPLIED RGBA.
   The two layouts differ where alpha < 255; each consumer must keep its own.
F7 (VERIFIED) No code changes pixel-store state: `grep GL_UNPACK|glPixelStorei|GL_PIXEL_UNPACK_BUFFER src` = none. So
   default unpack alignment 4 and row length 0 hold for every upload, as today.
F8 (VERIFIED) The capture path at HEAD (C3 + fix round 7500718 + followups D2):
   - `Renderer::captureFrame` (`Renderer.cpp:2035-2123`):
     - stores the time override BEFORE arming (:2045-2048);
     - arms under `captureMutex_`, with `capturePromise_ = &promise` and `pendingCaptureSeq_` (:2050-2057);
     - waits 5 s (:2060), then restores the override to the value it read (:2063);
     - on timeout, clears `pendingCapture_` / `capturePromise_` (:2065-2072);
     - otherwise takes its own `CaptureRead` by value (:2080), then converts + PNGs on the caller (:2098-2118).
   - `processPendingCapture` (:2125-2179): lock-size gate (:2131-2138), arm-seq gate (:2143-2145), glReadPixels only
     (:2161-2178).
   The R2 race is still open (renderperf.md:323 "#5b"):
   - a second caller overwrites `capturePromise_` (:2054);
   - the first then times out after 5 s, and its timeout path clears whatever capture is pending then.
   Two more consequences of the same overlap, found here (INFERRED from the code):
   - The time override is restored LIFO-wrong. A reads -1 and stores tA; B reads tA and stores tB. B is answered and
     restores tA, so live frames render at the frozen time tA until A times out.
   - TestServer sets and clears the TEST-ONLY lock OUTSIDE captureFrame (`TestServer.cpp:598-605`). Two 8080
     captures at different sizes therefore race on the canvas size.
   Also: the 1920x1080 clamp (:2037-2038) only feeds `captureWidth_/captureHeight_` (:2052-2053), which nothing reads
   (grep: `Renderer.h:619-620` + those writes only). The capture reads the canvas.
   Callers:
   - 8080 render_frame (`TestServer.cpp:601`);
   - 7070 render_frame (`ApiServer.cpp:1176`);
   - `takeSnapshot` (`Renderer.cpp:2198`), from REST `ApiServer.cpp:720`, OSC, and detached threads
     `MainComponent.cpp:1894, 2199, 6624, 7360`.
   Renderer.cpp is linked into no ctest (renderperf.md:228).
F9 (VERIFIED) PNG:
   - `PngWrite::writeReplacing` deletes first (`src/render/PngWrite.h:11-18`). It is used by captureFrame (:2104) and
     output_probe (`TestServer.cpp`).
   - JUCE 8.0.4's `PNGImageFormat::writeImageToStream` (`juce_PNGLoader.cpp:550-632`) never sets a compression level
     or filter: libpng defaults. It writes RGBA (color type 6) for ARGB images and un-premultiplies each pixel
     (:598-609). So the file's straight RGBA equals `unpremultiply(premultiply(gl))`.
   - `juce::GZIPCompressorOutputStream(dest, level 0..9, windowBits 0)` writes the zlib (RFC 1950) stream a PNG IDAT
     needs (`juce_GZIPCompressorOutputStream.h:57-60`, `.cpp:41-50`: `windowBits != 0 ? windowBits : MAX_WBITS`).
   - The encode is ALREADY off the GL thread (:2074-2118, C3).
   Measured (renderperf.md:94-113, 1080p fixture): read 2.3-5.0, convert 3.9-7.8, png 79.3-92.3 ms, round trip
   93-116 ms; at 4K png 305.6, round trip 350.5.
F10 (VERIFIED) The hold source.
   - `compositeDeck` saves every ACTIVE-deck Opaque/Transparent layer's post-stage picture (after renderLayerStages,
     before keying/opacity) into `layerOutputTexStorage_[layer.id]` every frame (`saveLayerOutput`, call :1016,
     def :216-241, FBO :203-214; maps `CompositorEngine.h:443-447`).
   - `resize` deletes those textures (:108-117); `releaseGL` too (:60-68).
   - Persistent layers save nothing ("the router addresses the active deck only", :1238-1246).
F11 (VERIFIED) A late first observation still starts the fade. `CrossfadeStartDetector::observe`
   (`src/render/CrossfadeHistory.h:37-48`) returns start on the first observed frame with `progress < 1 && prev >= 0`
   when it was not running before, or when the (prev, active) pair changed. So skipping `renderLayerStages` while an
   image is pending still hands the history over on the first ready frame (Pitfall 35 holds).
F12 (VERIFIED) GL lifecycle.
   - `~Renderer` calls `detach()` in its body (`Renderer.cpp:21-24`), before members die.
   - `openGLContextClosing` (:965-1054) releases the compositor (`compositor_.releaseGL()`, :1018), the image
     sequences (:999-1003) and `texMgr_` (:1054).
   - Contexts can be re-created during a run (comment :973-981).
   - Media retire: `closeMediaForClip` closes under `imageSeqMutex_` and hands the object to a retire list
     (:1334-1366); `drainRetiredMedia` releases it on the GL thread (:1368-1392).
F13 (VERIFIED) Image sequences decode on the GL thread.
   - `ImageSequence::getCurrentTexture` (`ImageSequence.cpp:190-202`) decodes + per-pixel copies + uploads the
     current frame on its first access, on the GL thread, under `imageSeqMutex_` (`Renderer.cpp:1504-1573`).
   - That happens inside compositeDeck, via `videoFrameFn_` (:997, :1086, :1224, :1450).
   - Every frame's texture is kept until releaseGL.
F14 (VERIFIED) The FX-only trap. A 0 clip texture on an Opaque/Transparent layer runs the clip's effects as FX Only
   over the layers below: compositeDeck :1001-1007, persistent :1229-1235. So a pending image must NEVER be returned
   as a plain 0.
F15 (VERIFIED, parallel lane) Thumbnails decode on the MESSAGE thread: `ClipCell.cpp:408`, `LayerStrip.cpp:959`.
   `.harmony/.reports/s-rta-0928/restore-diag.md` measured it as 99.6-99.9% of the routine-restore hold. It is not a
   GL-thread path and it belongs to that lane.
F16 (VERIFIED) Video decodes on the GL thread too: `VideoPlayer::advanceFrame` (`VideoPlayer.cpp:225`) reaches
   `avcodec_receive_frame` (:521) and `sws_scale` (:537). Not an image path -- filed.
F17 (VERIFIED) What probes rely on in captures.
   - Render probes grep the err.log prefix `Captured frame: [^ ]*` (probe-render-state.sh:74 and the other 4).
   - They decode captures with PIL; none hashes a capture: the one sha256 in probe-outputs.py:506-566 hashes a
     settings file.
   - probe-canvas `c_capture_deterministic` compares decoded arrays (probe-canvas.py:36-37, 375).
   - `/api/state` fields live in `ApiServer.cpp:1296-1310` and `TestServer.cpp:628-637`.
   - `GET /api/composition` reports `layers[].activeClipColumn` (`ApiServer.cpp:382`).
   - 8080 listens on `localhost` only (IPv6, followups.md:195).
F18 (VERIFIED) Pixel-store defaults (F7), JUCE ARGB byte order B,G,R,A (`juce_PixelFormats.h`), and the
   PixelConvert.h / PngWrite.h / FrameRing.h / ScratchPool.h header-only precedents with ctests
   (tests/CMakeLists.txt:2333-2380).
F19 (VERIFIED) CLAUDE.md is 23,752 bytes against its 25,000-byte cap (notebook.md:1697), so ~1.2 KB is free; the
   pitfall index ends at 47.
F20 (VERIFIED) No slack on ports or peers. Test mode runs both 7070 and 8080 (`MainComponent.cpp:1846-1868,
   1877-2118`). A lane under the live lock is running right now (owner `tier1diag`); nothing in this plan was run
   live.

## RULINGS (every design fork)

R1-a While an image decodes, a layer holds its last picture.
   - An active-deck Opaque/Transparent layer holds the previous frame's post-stage picture (F10). The layer's CURRENT
     keying/opacity/blend still apply, and renderLayerStages is skipped: no effect, ring, feedback or transition runs
     on a stand-in.
   - A Mask holds its last IMAGE mask texture.
   - A persistent layer, and any layer with nothing to hold (a layer's first picture, a fresh deck, the frame after a
     composition swap or a canvas resize), draws NOTHING for those frames. Never black-over and never FX-only (F14).
   - A pending OUTGOING image shows the incoming clip alone (the existing `prevTex == 0` return, :1532).
   Rejected: "nothing" as the rule (a black or through-flash on a cut, worse than today's freeze); "wait" (the render
   thread may not wait on I/O); "old image with the new clip's stages" (a wrong picture for 2-8 frames).
R1-b The hold lasts until the texture is resident: decode time plus at most 1 frame of upload queue. INFERRED: 1-3
   frames for a 1080p PNG, 3-8 frames at 4K. R1.0 reports decode=, and rows i1/i4 print the frames held
   (image_hold_frames delta). No cap. A failed decode ends the hold as "no media", today's semantics.
R1-c Both: lazy + hold always, PLUS prefetch at each composition swap.
   - The hook is `MainComponent::swapCompositionModel` after its fence and media close (`MainComponent.cpp:2834-2870`),
     which covers load, New and deck-append.
   - Order: the active deck first, its active clips first. One prefetch in flight (P = 1), so a demand decode always
     finds a free thread.
   - Prefetch stops at 1 GiB of resident image textures (ASSUMED budget, named constant). Demand loads are never
     refused.
   Rejected: preloading every clip at clip-assign time (many hooks: drop, relink, replace, undo -- lazy + hold covers
   them).
R1-d The cache.
   - Key: the full path, as today. Each entry stores the Stamp {mtime ms, size} read on the decode thread
     (FilesBrowser rule).
   - Invalidation: every composition swap re-validates resident images; the decoder compares the stamp first and
     skips the decode when it is unchanged. A changed file is re-decoded and replaces the texture at the next frame
     start. Today a changed file is never picked up until restart or context loss.
   - Eviction: at each swap, entries NOT in the new composition's image set are released. Today they are never
     released, which is a leak across composition loads.
   - Memory (INFERRED arithmetic):
     - a 3840x2160 RGBA8 texture is 33.2 MB, 1080p 8.3 MB;
     - CPU: at most 3 in-flight decodes x 2 buffers + at most 1 queued prefetch result + queued demand results --
       about 270 MB peak at 4K, freed after upload;
     - the VRAM ceiling = the composition's images, the same set today reaches once every clip has shown.
R1-e Every caller and path:
   - compositor: all 4 getKeyTexture sites (F1).
   - legacy single image (F2): lazy -- decoded and uploaded only when a frame reaches the fallback; the slideshow
     prefetches its next image.
   - image sequences (F13): async with look-ahead.
   - thumbnails (F15): OUT, the restore lane's follow-up. Recommended shape for that lane: a (path, stamp) cache fed
     by this plan's `ImageDecode::Decoder`.
   - video (F16): OUT, filed.
   - camera frames (`Renderer.cpp:194-203`): already decoded by the camera thread; unchanged.
R1-f Shutdown and context loss.
   - The decoder is a Renderer member. Its destructor calls `pool_.removeAllJobs(true, 5000)` first (F5 precedent):
     queued jobs are dropped and at most 3 running decodes finish (~0.1-0.3 s INFERRED).
   - Jobs capture no `this`. They deliver into consumer-owned `std::shared_ptr<Mailbox>` through a `std::weak_ptr`,
     so a dead consumer (a retired sequence, a destroyed compositor) drops the result.
   - `~Renderer` detaches the GL context before members die (F12), so no upload can run after that.
   - Context loss: `releaseGL` deletes every image texture and clears the cache, the ready queue and the hold maps.
     Late results for cleared entries are dropped, and the next frame's lookup re-requests.
R1-g Eyes / probe determinism: `processPendingCapture` answers only a frame with NO pending image (compositor
   lookups, sequences with nothing to show, a needed legacy image). This works like the lock-size and arm-seq gates
   (F8): the 5 s timeout bounds it, and every decode job always delivers Decoded / Unchanged / Failed, even on an
   exception. So `render_frame` right after `load_image`, a trigger or a composition load shows the image, never the
   placeholder. The `sleep(100)` in both load_image handlers stays; it is no longer load-bearing for render_frame.
R1-h Upload: one `glTexImage2D` per image (U1). Rule by R1.0's measurement: if the 3840x2160 `upload=` median is
   > 8 ms (3 quiet samples), R1.2 builds the chunked upload (U2) from the start:
   - `glTexImage2D(nullptr)`, then `glTexSubImage2D` of <= 8 MiB of rows per frame;
   - Resident only after the last chunk; never shown partial.
   The per-frame upload budget: always one image, and more only while the frame's upload total stays <= 8 MiB. It is
   shared by the compositor, the legacy slot and sequences.
R1-i Metric: `peak_callback_ms` (additive, reset on read, like peak_frame_time_ms) covers the WHOLE renderOpenGL
   (F3). The frame-timer window does NOT move (probe tolerances read it).
R2 Single flight (a queue of one) in captureFrame, not a multi-request queue. Two requests with different time
   overrides or lock sizes cannot share a frame, so any queue is serial service anyway. The lock set/restore moves
   INTO captureFrame (its header comment already promises this, `Renderer.h:385`). The dead clamp and the two dead
   members go (they would otherwise clamp the lock).
R3 A fast writer for render_frame only (7070 + 8080); every snapshot keeps JUCE's writer (the user's files). Ship
   iff the 1080p encode is >= 2x faster on the fixture (R3 GREEN bar); otherwise drop R3 and file the benchmark
   table. Level 0 is rejected: raw-size files (8.3 MB per 1080p capture) multiplied over the probe out dirs.
R4 Measure first. Fix only a VERIFIED resize-created target (a clear at creation, ~8 lines); else file.
R5 As C4 was specified (plan-renderperf.md:367-372), now unfenced.

## TRADEOFFS CONSIDERED (rejected)
- Keeping the GL-thread decode but making it cheaper (a row copy only). This is R1.1, and it ships -- but the
  decode (ImageIO, O(pixels)) stays. At 1080p and 4K the frame still drops (F4), and R1.1's gate re-runs i1/i2 to
  show it.
- A shared GL context on the decoder thread (upload off-thread): not provable on the macOS GL-over-Metal driver
  without a debugger; the dispatch forbids it without proof.
- PBO uploads written from the pool thread: the map/unmap must be on the GL thread and the lifetime is fragile. It
  saves only the upload memcpy, and U2 already bounds that per frame.
- The legacy image reusing the compositor's texture: premultiplied vs straight bytes (F6) would change legacy
  pictures with alpha.
- A dedicated decoder std::thread with a priority queue: juce::ThreadPool is the in-repo precedent, and priority is
  unnecessary because prefetch is capped at 1 in flight on 3 threads.
- Evicting by LRU under a VRAM budget: it would evict images a VJ is about to re-trigger mid-show. The
  composition-scoped retain set is predictable.
- R2 per-request promise queue: see R2. R3 level 0 or ImageIO: see R3 (ImageIO is also macOS-only; the app is
  cross-platform).

## DECISION / SPEC (commit order: R1.0 -> R2 -> R5 -> R1.1 -> R1.2 -> R1.3 -> R1.4 -> R1.5 -> R3 -> R4)

Builder step 0.
- Worktree lane; follow the rig rules in `.harmony/HANDOFF.md:36-44`: `df -h`, the live lock, open -g, quit via
  osascript, 45 s between lock holds, no lldb/dtrace/Instruments, no screencapture, no synthetic input, never the
  Output window.
- Build Release with `AUDIODNA_BUILD_TEST_SERVER=ON`, and keep a signed copy of the app after EVERY commit's build
  (renderperf METHOD).
- Record on main's app: ctest count, and the PASS/FAIL counts of probe-render-state, crossfade, canvas,
  effects-parity, fitmode, outputs and deck-clock.
- Perf rows run only when no clang runs, and print `os.getloadavg()`.

### R1.0 -- instrumentation + the RED harness (no behaviour change)
1. `Renderer.h`: `std::atomic<float> peakCallbackMs_{0.0f}; float takePeakCallbackMs()` (exchange 0, relaxed), next
   to `peakFrameTimeMs_` (:471).
   `Renderer.cpp` renderOpenGL, the FIRST statement (before `frameArmSeq_ = ...` at :166; it reads no timeOverride):
   ```cpp
   // s-rta-0928 R1.0: the WHOLE callback (every return), incl. the work before renderStart (pending legacy image,
   // camera upload, autopilot) and after renderEnd (recorder, Syphon, capture read) that peak_frame_time_ms misses.
   struct CallbackPeak {
       std::atomic<float>& peak; std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
       ~CallbackPeak() {
           const float ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
           if (ms > peak.load(std::memory_order_relaxed)) peak.store(ms, std::memory_order_relaxed);
       }
   } callbackPeak{ peakCallbackMs_ };
   ```
   `ApiServer.cpp` after :1305 and `TestServer.cpp` after :632:
   `obj->setProperty("peak_callback_ms", static_cast<double>(renderer_.takePeakCallbackMs()));`
2. Timing lines, instrumentation of the OLD code on the GL thread; each is replaced when its function is replaced.
   All use steady_clock, 1-decimal ms:
   - loadKeyImage: `[Image] loadKeyImage <path> (WxH) decode=D convert=C upload=U ms`. decode = loadFrom +
     convertedToFormat; convert = the loop + flip; upload = gen/bind/TexImage/params.
   - TextureManager::loadImage/uploadImage: `[Image] legacy <path> (WxH) decode= convert= upload= ms`.
   - ImageSequence::loadImageToTexture: `[Image] sequence <file> (WxH) decode= convert= upload= ms`.
3. The new probes (files and rows in PROBE ROWS below) are committed here: `.harmony/probe-image-load.{sh,py,json}`
   and `.harmony/probe-capture.{sh,py,json}`, with their docstrings' calibration left "TBD by R1.0".
4. RED runs, recorded verbatim:
   - (a) main's app, all new rows. cr3 runs in calibration mode: its medians go into probe-capture.json `cr3.calib`
     with the sha, load avg and date.
   - (b) the R1.0 app, all new rows.
   - (c) R1.0's split lines: 3 quiet triggers each of a cold 756x878, 1920x1080 and 3840x2160 image. Report
     decode/convert/upload per size, which decides R1-h, plus the legacy line per trigger (the F2 magnitude).
   Gate: build; ctest green (count unchanged); probe-canvas and probe-render-state GREEN with their main counts (the
   grep prefix is unchanged).

### R2 -- one capture at a time (the concurrent-capture fix)
1. `Renderer.h`:
   - add `std::timed_mutex captureFlight_;` with the comment: caller side only; one capture from arm to read; never
     taken on the GL thread.
   - delete `captureWidth_` / `captureHeight_` (:619-620, dead: F8).
   - Header comment on `captureFrame` (:380-387): "Concurrent callers are served one at a time; width/height > 0 set
     the TEST-ONLY canvas lock for this capture and restore the previous lock."
2. `Renderer.cpp` captureFrame:
   - delete the clamp (:2037-2038) and the two member writes (:2052-2053);
   - wrap everything from the override store to `future.get()` in
     ```cpp
     std::unique_lock<std::timed_mutex> flight(captureFlight_, std::defer_lock);
     if (!flight.try_lock_for(std::chrono::seconds(5)))
     { std::cerr << "[Eyes] Frame capture timed out after 5s (another capture in flight)" << std::endl; return false; }
     const uint64_t prevLock = lockedSize_.load(std::memory_order_relaxed);
     if (width > 0 && height > 0) setLockedResolution(width, height);   // moved from TestServer (R2)
     ```
   - restore `lockedSize_.store(prevLock)` on BOTH exits (timeout branch and after `future.get()`);
   - move `CaptureRead read = future.get();` inside the flight scope; the scope ends (unlock) right after it;
   - convert / PNG / log run outside it, unchanged.
   - The GL side (`processPendingCapture`) is untouched. No new lock on any GL path.
3. `TestServer.cpp:597-605`: delete both `setLockedResolution` calls. The response still echoes width/height.
4. Rows `cr1_concurrent_7070` and `cr2_concurrent_sizes_8080` (probe-capture): RED on main (R1.0 run 4a), GREEN
   here.
   Identity: the capt.py fixture (renderperf-evidence/scripts/capt.py) sha256 must equal the known
   `1eb43743...339e` at 1080p, `eb1f5191...b8156` at 720p and `650ef667...3ac7` at 4K (renderperf.md:116-120,
   followups.md:134-137).
   Regressions: probe-canvas, render-state, effects-parity, fitmode, outputs.
   Docs:
   - Pitfall 37's lock sentence: "`Renderer::captureFrame` sets and restores it under its capture flight lock (R2)";
   - testing-eyes.md: concurrent render_frame calls are served one at a time.

### R5 -- C4
1. `ApiServer.cpp` after :1304 and `TestServer.cpp` after :631:
   `obj->setProperty("frame_ring_cells", renderer_.getCompositor().getFrameRingCellCount());`
   (the getter exists, `CompositorEngine.h:151`).
2. `TestServer.cpp` handleOutputProbe:
   - replace the per-pixel block (the `for (int y ...) bmp.setPixelColour(... 255)` loop) with
     `PixelConvert::rgbaBottomUpToARGB(pixels.data(), w, h, bmp, true);`;
   - add `#include "render/PixelConvert.h"`.
   - Proof: the existing ctest "forceOpaque matches the output_probe loop" (`tests/test_pixel_convert.cpp:119`); its
     oracle IS that loop, verbatim.
3. Row `r1_cells` in probe-render-state (PROBE ROWS): RED on main (field absent), GREEN here.
   Output probe identity: an output_probe PNG of a static fixture (tap forced, as probe-outputs does -- never a
   window) has the same sha256 before and after (JUCE writer unchanged).
   Gates: probe-outputs GREEN (count as recorded), probe-render-state GREEN with the new row.
   Docs: rendering.md ring paragraph (:69) gains `frame_ring_cells` (cells created so far, over every ring).

### R1.1 -- PixelConvert, the reverse direction (still on the GL thread; byte-identical)
1. `src/render/PixelConvert.h`, add:
   ```cpp
   // JUCE ARGB BitmapData (premultiplied B,G,R,A) -> GL RGBA8 rows, bottom-up, tightly packed w*4. unpremultiply = the
   // bytes of the old getPixelColour loops (CompositorEngine::loadKeyImage, ImageSequence::loadImageToTexture);
   // false = TextureManager::uploadImage's raw swizzle (premultiplied). Proof: tests/test_pixel_convert.cpp.
   inline void argbToGlRgbaBottomUp(const juce::Image::BitmapData& src, uint8_t* dst, bool unpremultiply) noexcept
   {
       jassert(src.pixelFormat == juce::Image::ARGB && src.pixelStride == 4);
       const int w = src.width, h = src.height;
       for (int y = 0; y < h; ++y)
       {
           const auto* in = reinterpret_cast<const juce::PixelARGB*>(src.getLinePointer(y));
           uint8_t* out = dst + static_cast<size_t>(h - 1 - y) * static_cast<size_t>(w) * 4;
           for (int x = 0; x < w; ++x, out += 4)
           {
               juce::PixelARGB p = in[x];
               if (unpremultiply) p.unpremultiply();
               out[0] = p.getRed(); out[1] = p.getGreen(); out[2] = p.getBlue(); out[3] = p.getAlpha();
           }
       }
   }
   ```
2. Use it in loadKeyImage (:262-287: straight, drop the flip copy), ImageSequence::loadImageToTexture (:249-266:
   straight) and TextureManager::uploadImage (:37-57: premultiplied). Camera frames use uploadImage too: same bytes.
   The R1.0 timing lines remain and show the smaller convert=.
3. `tests/test_pixel_convert.cpp`, add 3 cases, each running the OLD loop verbatim as the oracle on both backings
   (Native, `SoftwareImageType`), with the every-alpha 256x256 buffer:
   - "straight == loadKeyImage's getPixelColour loop + flip". The same oracle covers ImageSequence: its loop is the
     same (F6).
   - "premultiplied == TextureManager::uploadImage's swizzle".
   - "odd sizes 131x77, 1x1, 1x9, 9x1, both layouts".
   Teeth, run once on mutated copies: drop `p.unpremultiply()` -> case 1 FAILs and case 2 passes; swap
   `getRed`/`getBlue` -> both FAIL.
   Gate: ctest green (+3); capt.py sha256 equal to the known values; abpix scenes S1-S4 array_equal and sha256 equal
   (BYTE-IDENTITY below); probe-render-state, crossfade, canvas, fitmode, effects-parity GREEN. Run i1/i2 and record
   them: they stay RED, which is the evidence that the async commits are needed.

### R1.2 -- the decoder, the compositor's async cache, the hold, the capture gate
1. NEW `src/render/ImageTexCache.h`: pure, no GL, no JUCE. It is named to avoid `juce::ImageCache`.
   ```cpp
   namespace ImageTexCache {
   struct Stamp { int64_t mtimeMs = 0; int64_t size = -1; friend bool operator==(const Stamp&, const Stamp&) = default; };
   enum class Kind : uint8_t { Decoded, Unchanged, Failed };            // what a decode job delivers
   enum class State : uint8_t { Pending, Resident, Failed };
   enum class Act : uint8_t { Drop, Upload, MarkFailed };               // what the GL wrapper must do with a result
   struct Lookup { uint32_t tex = 0; bool pending = false; bool request = false; };
   struct PrefetchJob { std::string path; std::optional<Stamp> known; };
   class Cache {                                                         // GL-thread owned
   public:
       Lookup lookup(const std::string& path);   // miss -> Pending(demand) + request=true ONCE; Pending -> pending;
                                                 // Resident -> tex; Failed -> tex 0 (no media, sticky; see R-7)
       Act onResult(const std::string& path, Kind kind, const Stamp& s);   // only a Pending or re-validating entry
                                                 // takes a result; anything else -> Drop
       uint32_t onUploaded(const std::string& path, uint32_t tex, int w, int h, const Stamp& s, size_t bytes);
                                                 // -> the texture it REPLACED (0 = none)
       bool isDemand(const std::string& path) const;
       std::vector<uint32_t> applyImageSet(const std::vector<std::string>& ordered);  // R1.5: drop non-members
                                                 // (returns textures to delete), queue members for prefetch /
                                                 // re-validation in order
       std::optional<PrefetchJob> nextPrefetch(size_t budgetBytes);    // R1.5: P = 1; no NEW prefetch while
                                                 // residentBytes >= budget (re-validations always allowed)
       std::vector<uint32_t> clearAll();
       int residentCount() const; size_t residentBytes() const; int pendingCount() const;
   };
   struct UploadBudget {                         // GL thread, reset every frame
       size_t cap = 8u << 20; size_t used = 0; bool any = false;
       void reset() { used = 0; any = false; }
       bool take(size_t bytes) { if (any && used + bytes > cap) return false; any = true; used += bytes; return true; }
   };
   }
   ```
2. NEW `src/render/ImageDecode.h`: header-only, juce_graphics; no GL anywhere, and the ctest proves it by running
   without a context.
   - `Layout { StraightRGBA, PremultipliedRGBA }`;
   - `Result { std::string path; ImageTexCache::Stamp stamp; uint64_t tag; ImageTexCache::Kind kind; int w, h;
     std::vector<uint8_t> rgba; double decodeMs, convertMs; }` (rgba = GL-ready, bottom-up);
   - `class Mailbox`: `push(Result&&)` takes a std::mutex; `bool tryDrain(std::vector<Result>&)` uses
     `std::unique_lock(m_, std::try_to_lock)` and returns false when contended or empty (the GL thread NEVER blocks);
     `int pending()`.
   - `Result decodeFile(const juce::File&, Layout, uint64_t tag, std::optional<Stamp> known)`:
     - stamp = {getLastModificationTime().toMilliseconds(), getSize()} read FIRST;
     - `known == stamp` -> Unchanged, no decode;
     - `ImageFileFormat::loadFrom`; invalid -> Failed;
     - `convertedToFormat(ARGB)`, then `PixelConvert::argbToGlRgbaBottomUp(bmp, rgba, layout == StraightRGBA)`;
     - log on the pool thread `[Image] decoded <path> (WxH) decode=D convert=C ms`.
   - `class Decoder`:
     - `explicit Decoder(int threads = 3)` with `ThreadPoolOptions{}.withNumberOfThreads(threads)
       .withThreadName("ImageDecode").withDesiredThreadPriority(juce::Thread::Priority::low)`;
     - `~Decoder() { pool_.removeAllJobs(true, 5000); }`;
     - `void request(juce::File, Layout, uint64_t tag, std::optional<Stamp> known, std::weak_ptr<Mailbox> box)`:
       `pool_.addJob([=]{ Result r; try { r = decodeFile(...); } catch (...) { r = Failed{path, tag}; }
       if (auto b = box.lock()) b->push(std::move(r)); });` -- no `this` captured.
3. `CompositorEngine.h/.cpp`:
   - Delete `loadKeyImage` and `textureCache_`. Add:
     - `ImageTexCache::Cache imageCache_`;
     - `std::shared_ptr<ImageDecode::Mailbox> imageBox_ = std::make_shared<...>()`;
     - `std::vector<ImageDecode::Result> readyImages_`;
     - `ImageDecode::Decoder* decoder_ = nullptr`; `ImageTexCache::UploadBudget* uploadBudget_ = nullptr`;
     - `uint64_t frameSerial_ = 0`; `int pendingImagesThisFrame_ = 0`;
     - `struct LayerOutputOwner { uint32_t deckId; uint64_t frame; }`; `unordered_map<uint32_t, LayerOutputOwner>
       layerOutputOwner_`;
     - `unordered_map<uint64_t, GLuint> lastMaskImageTex_`;
     - relaxed atomics `imageHoldFrames_`, `imageSkipFrames_` (int64), `imagesPending_`, `imageTexCount_`,
       `imageTexBytes_`, `peakImageUploadMs_`;
     - getters for /api/state, and `takePeakImageUploadMs()`.
   - `void setImageDecoder(ImageDecode::Decoder*, ImageTexCache::UploadBudget*)`.
   - `void beginFrame()`: `++frameSerial_; pendingImagesThisFrame_ = 0;`.
   - `void pumpImages()`, GL thread, once per frame before compositeDeck, only if `glInitialized_`:
     - `imageBox_->tryDrain` into `readyImages_`;
     - order demand first (`isDemand`);
     - for each result: `onResult`, then on Upload `if (!uploadBudget_->take(bytes)) break;` (it stays for next frame);
       upload; `onUploaded`; delete the replaced texture;
     - Drop and MarkFailed need no GL;
     - update the atomics and `peakImageUploadMs_`.
     - The upload is loadKeyImage's GL calls exactly: gen, bind `GL_TEXTURE_2D`, `glTexImage2D(RGBA8, w, h, 0, RGBA,
       UNSIGNED_BYTE)`, the same 4 params. If R1-h chose U2: chunk state per path, Resident after the last chunk.
   - `GLuint getKeyTexture(const juce::File&, bool* pending = nullptr)`, which NEVER decodes:
     - `lookup`; on `request` call `decoder_->request(file, StraightRGBA, 0, nullopt, imageBox_)`;
     - on pending, `++pendingImagesThisFrame_`, set `*pending`, return 0.
     - `decoder_ == nullptr` -> return 0, not pending (jassert).
   - `deleteImageTexture(GLuint)`: `glDeleteTextures` and purge that name from `lastMaskImageTex_`.
   - `saveLayerOutput(layerId, deckId, ...)` (one caller, :1016) records `layerOutputOwner_[layerId] = {deckId,
     frameSerial_}`.
   - `GLuint heldLayerOutput(layerId, deckId)`: returns `layerOutputTexStorage_[layerId]` iff the owner's deckId
     matches and `owner.frame + 1 == frameSerial_`, else 0.
   - `touchLayerOutput(layerId, deckId)`: stamps the owner with the current frame.
   - `resize` and `releaseGL` clear `layerOutputOwner_`.
   - `releaseGL` also: `for (tex : imageCache_.clearAll()) glDeleteTextures`; clear `readyImages_` and
     `lastMaskImageTex_`.
4. The hold (compositeDeck Opaque/Transparent, :979-1060). Pending never reaches the FX-only branch (F14):
   ```cpp
   bool pending = false;
   if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
       clipTex = getKeyTexture(clip->mediaFile, &pending);
   ... (source / video unchanged; R1.4 adds &pending to videoFrameFn_) ...
   if (pending)
   {
       // s-rta-0928 R1: the picture is still decoding -- never "no media" (that would run the clip's effects as FX
       // Only, :1001-1007). Hold this layer's LAST picture (its Layer Router output from the previous frame); the
       // layer's current keying/opacity still apply below. Nothing to hold -> the layer draws nothing this frame.
       clipTex = heldLayerOutput(layer.id, deck.id);
       if (clipTex == 0) { imageSkipFrames_.fetch_add(1, std::memory_order_relaxed); continue; }
       imageHoldFrames_.fetch_add(1, std::memory_order_relaxed);
       touchLayerOutput(layer.id, deck.id);
   }
   else
   {
       if (clipTex == 0) { ... FX-only exactly as today ...; continue; }
       clipTex = renderLayerStages(...);                          // unchanged
       saveLayerOutput(layer.id, deck.id, clipTex, ...);          // + owner record
   }
   ... keying / Opaque draw unchanged ...
   ```
   Mask (:1070-1096):
   - pending -> `lastMaskImageTex_[LayerStateKey::clipChain(deck.id, layer.id)]` (hold, count) or skip (count);
   - not pending -> record `clipTex` there only when the clip is an Image (cache-owned texture), else record 0.
   compositePersistentLayers (:1209-1236): pending -> count skip; `continue`.
   getClipTexture (:1437-1452): pass the flag through; applyTransition's existing `prevTex == 0` return handles it.
5. `Renderer.h/.cpp`:
   - Members near `texMgr_` (:273): `ImageDecode::Decoder imageDecoder_{ 3 };` and
     `ImageTexCache::UploadBudget uploadBudget_;`.
   - In `newOpenGLContextCreated` after `compositor_.initGL` (:136): `compositor_.setImageDecoder(&imageDecoder_,
     &uploadBudget_);`.
   - renderOpenGL, right after `frameArmSeq_`: `compositor_.beginFrame(); uploadBudget_.reset();`.
   - The first statement inside `if (deckActive)` (:539, inside the frame window): `compositor_.pumpImages();`.
   - The capture gate: in `processPendingCapture`, after the lock-size check (:2138) and BEFORE `captureMutex_`:
     ```cpp
     // s-rta-0928 R1: a frame that held or skipped a layer (an image still decoding) never answers a capture --
     // render_frame right after a load or trigger shows the picture, never the placeholder.
     if (compositor_.framePendingImages() > 0 || legacyPendingThisFrame_)   // legacy flag: R1.3 (false until then)
         return;
     ```
   - `/api/state` (both servers, after `peak_callback_ms`): `image_hold_frames`, `image_skip_frames`,
     `images_pending`, `image_textures`, `image_texture_mb`, `peak_image_upload_ms`.
6. NEW `tests/test_image_tex_cache.cpp` (pure; registered like `test_frame_ring`, Catch2 only). Cases:
   - (1) miss -> Pending, request exactly once; a second lookup -> pending, no request;
   - (2) Decoded on Pending -> Upload; onUploaded -> Resident; lookup -> that tex;
   - (3) Failed -> MarkFailed; lookup -> tex 0, not pending, no request (sticky);
   - (4) a result for a path with no entry (cleared or evicted) -> Drop;
   - (5) clearAll returns every texture, and each later lookup requests again;
   - (6) isDemand after lookup;
   - (7) UploadBudget: the first take is always granted (even > cap); later takes only within cap; reset.
   R1.5 adds (8)-(11).
   NEW `tests/test_image_decode.cpp` (juce_gui_basics, registered like test_png_write; `ScopedJuceInitialiser_GUI`):
   - (1) a 97x61 PNG with varied alpha written to a temp dir, both layouts. Bytes == the verbatim old loops applied
     to `ImageFileFormat::loadFrom(file)`; tag and path round-trip.
   - (2) known stamp == current -> Unchanged, empty rgba; rewrite the file -> Decoded.
   - (3) a text file -> Failed.
   - (4) request, then reset the only mailbox shared_ptr: no crash; the destructor returns.
   - (5) a 1-thread decoder, 30 requests of a 1024x1024 noise PNG, destroyed at once: elapsed < 50% of 30 x the
     single-decode time measured in the test, and < 30 delivered.
   Teeth, run once:
   - `onResult` accepting an unknown path -> case 4 FAILs;
   - the decoder destructor without `removeAllJobs` -> decode case 5 FAILs;
   - `lookup` requesting on every Pending lookup -> case 1 FAILs.
7. Rows:
   - i3_capture_after_trigger GREEN. Teeth: build ONCE with the gate line commented out, run i3 -> FAIL (it captures
     the held picture); restore; the `shasum` of Renderer.cpp equals the committed one.
   - i4_hold_counters and i4m_mask_hold GREEN.
   - i1/i2 at 1080p and 4K: the frame-window line (peak_frame_time_ms) GREEN. The callback line may stay RED until
     R1.3 (the legacy decode, F2): expected and recorded.
   Gate: ctest green (+2 files); abpix array_equal and sha256 equal; probe-render-state (r1_counts peaks are now
   ring-only), crossfade, canvas (incl. perf rows, quiet), effects-parity, fitmode, outputs, deck-clock -- all GREEN
   with their recorded counts.

### R1.3 -- the legacy single image: decoded and uploaded only when a frame needs it
1. `Renderer.h`, replacing `pendingImageFile_/hasPendingImage_/pendingClearImage_` (:599-602):
   - Written under `pendingImageMutex_` by any thread: `struct LegacyRequest { juce::File file; bool clear = false;
     uint64_t gen = 0; } legacyReq_;` and `juce::File legacyPrefetch_`.
   - GL thread only:
     - `uint64_t legacyHandledGen_`, `legacyInFlightGen_`;
     - `std::optional<ImageDecode::Result> legacyReady_, legacyParked_`;
     - `std::string legacyResidentPath_`; `ImageTexCache::Stamp legacyResidentStamp_`;
     - `std::shared_ptr<ImageDecode::Mailbox> legacyBox_`;
     - `bool legacyPendingThisFrame_ = false`.
   `loadImage(f)` sets `{f, false, ++gen}`; `clearImage()` sets `{{}, true, ++gen}`, so the LAST call wins. Today a
   clear+load pair inside one frame dropped the load (`Renderer.cpp:178-183`); see R-7.
   New `void prefetchLegacyImage(const juce::File&)` sets `legacyPrefetch_`.
2. The frame top (:175-192 becomes O(1)):
   - copy the request and the prefetch path under the lock;
   - try-drain `legacyBox_`: a result with tag == the current gen goes to `legacyReady_`; tag kPrefetchTag goes to
     `legacyParked_`; stale results are dropped;
   - a clear request -> `texMgr_.release()`, drop the ready result, clear the resident path;
   - a load request -> only recorded (lazy);
   - a posted prefetch path that is not resident, ready or parked -> request it now (PremultipliedRGBA,
     kPrefetchTag).
   `legacyPendingThisFrame_ = false` at the frame top.
3. `void resolveLegacy()`, called ONLY where the legacy image is needed:
   - (a) before the early return at :327, when `!sourceActive && !deckActive`;
   - (b) in the fallback at :629, before `texMgr_.getImageTexture()`.
   What it does:
   - A request not yet satisfied: adopt `legacyParked_` if it is the same path; else request (PremultipliedRGBA,
     tag = gen, known = the resident stamp when the path equals `legacyResidentPath_`) when none is in flight. At most
     1 in flight; the newest request is issued when the in-flight one completes.
   - A ready Decoded result: `uploadBudget_.take(bytes)` -> `texMgr_.uploadPixels(rgba, w, h)`; record the resident
     path and stamp.
   - Unchanged -> done. Failed -> done; the old texture stays, as today's `loadImage == false`.
   - Still pending -> `legacyPendingThisFrame_ = true`, and the current texMgr_ texture shows (hold) or black.
4. `TextureManager`: `bool uploadPixels(const uint8_t* rgba, int w, int h)` = `uploadImage`'s GL half, verbatim
   (glTexSubImage2D at the same size, else a new texture; :59-93). `uploadImage` = the convert (R1.1) +
   `uploadPixels`; camera frames keep using it. `loadImage` is deleted (its only caller was :187).
5. `openGLContextClosing` clears `legacyResidentPath_` (the texture dies with `texMgr_.release()` at :1054, as today).
6. Slideshow (`MainComponent.cpp:3890-3953`): after each `previewPanel_.loadImage(img)`, call
   `previewPanel_.getRenderer().prefetchLegacyImage(slideshowImages_[(slideshowIndex_ + 1) % slideshowImages_.size()])`.
   In `openImageFolder`, prefetch index 1.
7. Rows:
   - i5_legacy_retrigger GREEN;
   - i1/i2 callback lines GREEN at 1080p and 4K (all four i1/i2 rows fully GREEN now);
   - i3 still GREEN.
   Tier-1 (exactly the 5 files, test mode): `source_param_failures.txt` byte-identical to main's (sha256
   `0524d335...88e7`, tier1-residual-diag.md FACTS (1)) and the same per-id outcomes.
   Plus the R1.2 gate set. Boris check (manual, no synthetic input): the slideshow advances on the beat with no
   stutter.

### R1.4 -- image sequences
1. `CompositorEngine.h`: `using VideoFrameFn = std::function<GLuint(const Clip*, float, bool* pending)>;`. The 4 call
   sites (:997, :1086, :1224, :1450) pass `&pending`, with the same handling as images.
   `Renderer::getVideoFrameTexture` and `syncMedia` gain `bool* pending` (Video: never set, unchanged semantics).
2. `ImageSequence`:
   - Add a `std::shared_ptr<ImageDecode::Mailbox>` member, `std::vector<uint8_t> requested_`, `int lastShown_ = -1`,
     and `uint32_t openGen_` (bumped by `open`; tag = `(uint64_t)openGen_ << 32 | index`).
   - `GLuint getCurrentTexture(ImageDecode::Decoder&, ImageTexCache::UploadBudget&, bool* pending)`:
     - drain + upload ready frames of this gen, within the budget (loadImageToTexture's GL calls);
     - current idx resident -> `lastShown_ = idx`, request look-ahead;
     - else request idx (if not requested) + look-ahead, and return `textures_[lastShown_]` (the last frame repeats,
       as a late video frame would); set `*pending` ONLY when `lastShown_ < 0` (nothing to show).
   - Look-ahead = the next 3 frames in the current play direction (Loop wraps; PingPong follows `pingPongForward_`;
     OneShot stops at the end), at most 4 requested-not-resident per sequence.
   - A Failed frame is marked and never re-requested; the last frame repeats. Today it retried every frame and showed
     nothing (R-7).
   - `releaseGL` also resets `requested_` and `lastShown_`. `loadImageToTexture` is deleted.
3. Row i6_sequence_1080 GREEN; the R1.2 gate set.

### R1.5 -- prefetch at composition swap + release of dropped images
1. `src/core/CompositionLoad.h`: `inline std::vector<std::string> imagePaths(const Composition& c)`. It visits the
   active deck first, and within each layer the active clip first, then the other columns, then the other decks in
   index order. It takes Image clips with a non-empty `mediaFile` and dedupes, keeping the first occurrence.
   `tests/test_composition.cpp` gets 1 case (order + dedupe).
2. `CompositorEngine`: `void postImageSet(std::vector<std::string>)` (any thread): a mutex-guarded slot that REPLACES
   any unconsumed set. `pumpImages` takes it with try_lock and then:
   - `for (tex : imageCache_.applyImageSet(set)) deleteImageTexture(tex);`
   - `lastMaskImageTex_` purged (deleteImageTexture);
   - each frame, `if (auto j = imageCache_.nextPrefetch(kImagePrefetchBudgetBytes)) decoder_->request(j->path,
     StraightRGBA, 0, j->known, imageBox_);` with `kImagePrefetchBudgetBytes = 1u << 30` (ASSUMED, comment says so).
3. `MainComponent::swapCompositionModel`: after the close-by-set-difference loop (:2858-2861), add
   `renderer.getCompositor().postImageSet(compload::imagePaths(composition_));`. The fence has already made the new
   composition live, so the deletions run at the next frame start, before any pass.
4. `tests/test_image_tex_cache.cpp` (8)-(11):
   - applyImageSet drops non-members (textures returned; their late results Drop);
   - P = 1: two queued paths -> the second only after the first's result;
   - budget: no new prefetch at or over budget, re-validations still issued, demand lookups still request;
   - re-validation: Unchanged keeps the tex; Decoded with a new stamp -> Upload, and onUploaded returns the old tex.
   Teeth: P = 2 -> case 9 FAILs.
5. Row i7_prefetch_retain GREEN; the R1.2 gate set.

### R3 -- a fast PNG writer for render_frame
0. Measure (current main, R1.0 run 4a): cr3 calibration at 1080p x5 and 4K x2, plus the split lines. That is where
   the time goes, as the dispatch requires.
1. `src/render/PixelConvert.h`: `inline void rgbaBottomUpToPngScanlines(const uint8_t* gl, int w, int h, uint8_t* out)`.
   - Per row (top-down from GL row h-1-y): one filter byte 0, then per pixel
     `juce::PixelARGB p(a, r, g, b); p.premultiply(); p.unpremultiply();` and emit R,G,B,A.
   - That is exactly the straight RGBA JUCE's writer emits from the PixelConvert image (F9). a = 255 and a = 0 need no
     special case.
2. `src/render/PngWrite.h`: `inline bool writeScanlinesReplacing(const uint8_t* scanlines, int w, int h,
   const juce::File&, int level)`. It:
   - deletes first (the writeReplacing rule, Pitfall 46);
   - writes the signature, IHDR (8-bit, color type 6, no interlace), one IDAT (the level-`level` zlib stream of the
     scanlines through `GZIPCompressorOutputStream(mem, level, 0)` into a MemoryOutputStream) and IEND;
   - computes a table CRC32 over each chunk's type + data.
   `kFastPngLevel = 1`. `writeReplacing` is unchanged.
3. `Renderer`:
   - `enum class CaptureEncoding { Archive, Fast };` and `captureFrame(..., CaptureEncoding enc =
     CaptureEncoding::Archive)`;
   - Fast = scanlines (timed as convert=) + `writeScanlinesReplacing` (timed as png=);
   - append ` enc=fast|archive` after ` ms` on the `[Eyes] Captured frame:` line. The prefix is verbatim (F17).
   `ApiServer.cpp:1176` and `TestServer.cpp:601` pass `Fast`; `takeSnapshot` keeps Archive.
4. NEW `tests/test_png_fast.cpp` (registered like test_png_write). Cases:
   - (1) the scanline bytes == the JUCE writer's row bytes, computed from a PixelConvert image by its own loop copied
     verbatim from `juce_PNGLoader.cpp:600-609` (every alpha + odd sizes);
   - (2) the file parses: CRCs are checked with a bitwise CRC oracle; IDAT inflates through
     `juce::GZIPDecompressorInputStream` to exactly the scanlines;
   - (3) `PNGImageFormat` decodes the fast file and the JUCE-written file to byte-equal images;
   - (4) replace-not-append (an existing bigger file).
   - `[.bench]` (hidden; run by name, `-s`): JUCE writer vs fast levels 0/1/2, plus level 6 with filter 0, on
     1920x1080 and 3840x2160 smooth and noise frames. Printed only: that is the ruling's evidence, and level 6 with
     filter 0 isolates libpng's filter heuristic.
   Teeth: drop the premultiply/unpremultiply round trip -> case 1 FAILs.
5. Row cr3_round_trip GREEN (bars in PROBE ROWS); abpix decoded arrays equal before and after. sha256 now differs for
   render_frame files by design; a snapshot's sha256 is still equal. Tier-1 as in R1.3.
   If the 1080p png median is not <= 50% of the calibration: revert R3, keep the ctest-free benchmark table in the
   report, and file.

### R4 -- 4K warm leftover: measure, then fix or file
0. Four t2warm variants at 3840x2160, each 3 quiet runs on the R1.5 app (images warm; t2warm.py from
   renderperf-evidence as the base; peak_frame_time_ms, window unchanged):
   - E0 as is;
   - E1: the warm layer W (id LID+100) fades A->B (transitionSpeed 1.0), which writes the transition target first;
   - E2: the subject layer first shows a 7th column (A, no effects) for 0.6 s, which creates its router FBO first;
   - E3: W's two clips carry [Invert 1.0] and W fades, which writes the effect pool A/B/C and the transition first.
   A target is VERIFIED when its variant removes >= 70% of E0's excess (first use or first fade minus the second
   fade) in 3/3 runs.
1. If the verified target is created in initGL/resize (transition, effect pool, scratch): add
   `CompositorEngine::touchCanvasTargets()`, called at the end of `initGL` and `resize`. It binds each of the 7
   canvas FBOs, sets the viewport, `glClearColor(0,0,0,0)` + `glClear`, and ends with FBO 0 bound (R5-safe: it runs
   before any pass).
   - The lazy commit moves into the resize frame, which already reallocates everything.
   - Pictures: every pass that reads these targets writes them first; the transition clears explicitly (:1545-1548).
     Proven by abpix plus every picture probe.
   - Add row i8_warm_first_fade_4k (PROBE ROWS). RED on main by the E0 numbers.
   If it is the router FBO (E2), or nothing is VERIFIED: no code; file with the E0-E3 table.

## PROBE ROWS (exact; every perf row waits for no clang/clang++ and prints the load average)
Common to the new probes:
- `.sh` = probe-render-state.sh's scaffolding verbatim:
  - the live-lock gate, `adna_pids` by ucomm, REFUSE if running or if 7070 is taken;
  - a fresh `mktemp -d` OUT, `open -g`, the health wait;
  - the foreign-traffic check, osascript quit, and pkill only after 30 s.
- probe-image-load: `IMGLOAD_APP` / `IMGLOAD_PY`; production mode; `rm -rf "$OUT/media"` after quit.
- probe-capture: `CAPT_APP` / `CAPT_PY`; `--args --test-mode`; waits for 7070 AND `http://localhost:8080`.
- `requests.Session` with `Connection: close`.
- Metric `peak(s, k)`: the key when present; otherwise the row FAILs with "k absent (app predates ...)".
- Windows are polled every 15 ms, keeping the max (the fields reset on read).
- dbox(X, Y): both arrays box-averaged in 16x16 blocks, then mean |diff| over RGB -- robust to a half-pixel shift,
  and it still separates "image" from "held picture" from "black".
Fixtures: generated per run in `$OUT/media` by the .py BEFORE any row (numpy seed 928, PIL compress_level 1):
- warm flat PNGs;
- cold "noisy gradients": R = 255x/W, G = 255y/H, B = 128, + uniform noise in [-64, 64] per channel, alpha 255 --
  hard to compress (a slow decode, like a photo or worse), yet box-identifiable;
- every path is new in this app instance (fresh OUT), i.e. never seen.
The fields read are new: `peak_callback_ms` (R1.0), `image_*` (R1.2), `frame_ring_cells` (R5).

`.harmony/probe-image-load.py` (7070; json `probe-image-load.json`: peakMaxMs 16.7, boxTol 3.0, sizes, layer ids):
| row | fixture / steps | PASS conditions | RED on main |
|---|---|---|---|
| i1_layer_1080 / i1_layer_4k | canvas = size; layer 61 / 71; col 0 warm flat, col 1 cold noisy gradient at the canvas size, cut. load, 1 s, trig 0, 1.5 s, quiet wait, 0.6 s pre-poll, trig 1, 1.5 s poll, cap. | (a) max peak_frame_time_ms in the window <= 16.7; (b) max peak_callback_ms <= 16.7; (c) dbox(cap, cold image) <= 3.0. Prints pre-max, hold frames, load avg. | (a) FAIL: loadKeyImage in the window; (b) FAIL: absent |
| i2_fade_1080 / i2_fade_4k | as i1 with transitionSpeed 1.0 (dissolve to the cold image); layer 62 / 72. | as i1 (a)(b); (c) after 1.5 s dbox <= 3.0 | as i1 |
| i3_capture_after_trigger (guard + teeth) | canvas 1920x1080; layer 63; col 0 warm flat, col 1 cold 3840x2160. trig 0, 1.5 s, capA; trig 1; poll `GET /api/composition` every 5 ms until decks[0].layers[0].activeClipColumn == 1 (<= 2 s); capN at once; 2 s; capS. | fixture: dbox(capA, capS) >= 20; row: dbox(capN, capS) <= 3.0 | PASS (synchronous decode); teeth = the R1.2 temporary build without the gate: FAIL |
| i4_hold_counters | canvas 3840x2160; layer 64; col 0 warm, col 1 cold 4K; after col 0 settles: s0, trig 1, 1.5 s, s1. | image_hold_frames delta >= 1 AND image_skip_frames delta == 0; images_pending(s1) == 0. Prints the frames held. | FAIL: fields absent |
| i4m_mask_hold | 4K; layer 0 Opaque warm colour; layer 1 id 69 Mask (type 4): col 0 warm mask (left half white), col 1 cold 4K mask. trig(1,0), settle, s0, trig(1,1), 1.5 s, s1. | hold delta >= 1 AND skip delta == 0 | FAIL: absent |
| i5_legacy_retrigger | canvas 1920x1080; layer 65; cols = 2 cold 1080p images; trig 0, 1 s, trig 1, 1 s (both resident); quiet; 6 triggers 0,1,0,1,0,1 every 0.5 s while polling. | max peak_callback_ms <= 16.7 (prints max peak_frame_time_ms: expected low on every build) | FAIL: absent; R1.0: FAIL by value (F2) |
| i6_sequence_1080 | canvas 1920x1080; layer 66; col 0 = mediaType 5, 12 fresh 1080p noisy frames (hue shifted per frame), sequenceFps 10, Loop. load, 1 s, quiet, 0.6 s pre, trig 0, 1.6 s poll, cap. | (a)(b) as i1; (c) the cap is within dbox 3.0 of one of the 12 frames | (a) FAIL; (b) FAIL: absent |
| i7_prefetch_retain | composition P: deck 0 layer 67 cols 0-2 + deck 1 layer 67 cols 0-2 = 6 cold 1080p images, none triggered. load P; poll every 0.1 s <= 15 s until images_pending == 0 and image_textures == 6, keeping max peak_callback_ms; s1; trig(0,1); 0.5 s; s2; load Q (one warm image only); 1.5 s; s3. | (1) image_textures == 6 within 15 s; (2) max peak_callback_ms during the prefetch <= 16.7; (3) image_hold_frames(s2) - (s1) == 0; (4) image_textures(s3) <= 1 | FAIL: absent |
| i8_warm_first_fade_4k (only if R4 fixes) | E0 of R4 as a row: 4K; layer 68, 6 cols alternating A/B with [Screen Split 0.15 0.15 0.25 0], T = 1 s; warm layer 168 shows A then B first. | first use and first fade peak_frame_time_ms <= max(1.5 x second fade, second fade + 1.0) | FAIL (renderperf.md:67: 3.83 / 3.68 vs 1.14) |

`.harmony/probe-capture.py` (json `probe-capture.json`):
| row | steps | PASS | RED on main |
|---|---|---|---|
| cr1_concurrent_7070 | the capt.py fixture at 1920x1080 (layer opacity 0.5, clip A), triggered, 1.5 s; ref = one solo capture (time 0.0); then 4 threads behind a barrier POST 7070 render_frame {fresh path, time 0.0}, timeout 15 s. | all 4 ok:true, each call <= 3.0 s, each PNG decodes array_equal to ref | 3 of 4 fail after ~5 s (F8) |
| cr2_concurrent_sizes_8080 | same fixture; solo 8080 refs at 640x360 and 320x180; then both sizes concurrently (barrier). | both ok, each decodes at its own size and array_equal to its ref, each <= 3.0 s | one times out |
| cr3_round_trip (perf) | same fixture; 5 sequential 7070 captures at 1920x1080, then 2 at 3840x2160 (composition size); the client times the round trip; the err.log line for each path gives read/convert/png. | median 1080p png <= 0.5 x calib.png1080_ms AND median 1080p round trip <= 0.75 x calib.rt1080_ms; every PNG non-blank at the canvas size. Prints the full table + enc=. | FAIL by construction (it IS the calibration: ~100%) |

`.harmony/probe-render-state.py` (+ json `r1.cells`: layerId 72, fx [Screen Split 0.15 0.15 0.25 0]):
| row | steps | PASS | RED on main |
|---|---|---|---|
| r1_cells | fresh layer 72, col 0 = A + fx; s0; trig 0; 0.3 s s1; 1.0 s s2 (fps from state) | (1) 1 <= cells(s1)-cells(s0) < 480; (2) cells(s1) - cells(s0) < cells(s2) - cells(s0) <= cells(s1) - cells(s0) + 1.3 x (t2 - t1) x max(fps) + 5; (3) cells(s2) - cells(s0) <= 480 x (frame_rings(s2) - frame_rings(s0)) | FAIL: frame_ring_cells absent |

## BYTE-IDENTITY -- how "pictures unchanged" is checked
- ctest oracles run the OLD loops verbatim:
  - the 3 R1.1 cases;
  - the existing `forceOpaque` case for R5;
  - test_png_fast (1)-(3) for R3;
  - test_image_decode (1) for the whole decode path.
- `abpix.py` (lane evidence scripts; test mode so 7070 and 8080 both run) runs each scene twice per build, keeping
  sha256 and PIL arrays:
  - S1 = capt.py at 1080p / 720p / 4K (known hashes above);
  - S2 = 1080p, layer 0 clip B + layer 1 Transparent clip `alpha.png`, a generated 512x512 with alpha = x ramp and
    colour = y ramp, which exercises the unpremultiply path;
  - S3 = 8080 load_image(alpha.png) + render_frame 640x360 (the legacy path);
  - S4 = an ImageSequence of 3 frames at sequenceFps 0.1 (frame 0 held);
  - S5 = `/api/snapshot` of S1 (Archive writer).
  The rules:
  - every commit: array_equal to main's arrays on all scenes;
  - until R3: sha256 equal too;
  - after R3: render_frame sha256 differs by design, and S5 stays sha256-equal.
- Every picture probe stays GREEN with its recorded counts: render-state, crossfade, canvas, effects-parity, fitmode,
  outputs, deck-clock.

## MUST NOT CHANGE (and how each is kept)
- Pictures: BYTE-IDENTITY above. The only new visible state is the hold during decode, which no capture can observe
  (R1-g).
- The output / IOSurface path: `publishToOutputs`, `SharedFrameSet`, `OutputPresenter` and `OutputWindow` are not
  edited. R5 edits only the test-only reader's conversion, proven byte-identical.
- Pitfall 3: no new FBO anywhere. The hold reads the existing router texture, uploads create textures only, and R4
  clears existing FBOs.
- Pitfalls 13/14: `EffectChain` is untouched; the legacy image still feeds `effectChain_.render`.
- Pitfall 20: ring code is untouched; r1_counts' rows and tolerance are unchanged; R5 only reads the count.
- Pitfall 25: VideoRecorder is untouched.
- Pitfall 35: no history key changes. A held frame skips renderLayerStages, so no chain history is read or written,
  and the hand-over fires on the first ready frame (F11). r1_temporal / r1_ring / r1_retrigger stay GREEN.
- Pitfall 37: canvas sizing is unchanged. R2 moves the TEST-ONLY lock set/restore into captureFrame: same values,
  same span around the capture.
- Pitfall 39: layer_transform is untouched; the fit reads the texture size, and the uploaded texture has the same
  size.
- Pitfall 46: every PNG write deletes first (the new writer too, ctest case 4).
- The render thread never waits on I/O:
  - no decode on the GL thread on any path;
  - the GL side of each new lock is try_lock (Mailbox, image-set slot) or not taken at all (captureFlight_);
  - the one blocking lock the GL thread gains is juce::ThreadPool's internal list lock, an O(1) append on a cache
    miss;
  - no new GL-thread log line: upload timing goes to /api/state. The explicit discussion Sacred Rule 2 asks for is
    R-5.
- The frame-timer window (`:533`/`:779`) and every existing probe tolerance are unchanged; rows are only added.
- 7070/8080 request and response shapes: /api/state fields are added, nothing else.
- Capture semantics (D2 arm-seq, the lock-size gate, the 5 s timeout): unchanged, plus the completeness gate
  (intended).

## FENCE
This lane's files:
- src/render/{CompositorEngine.h,.cpp, Renderer.h,.cpp, TextureManager.h,.cpp, PixelConvert.h, PngWrite.h}
- NEW src/render/{ImageTexCache.h, ImageDecode.h}
- src/media/ImageSequence.h/.cpp, src/core/CompositionLoad.h
- src/MainComponent.cpp: swapCompositionModel +1 line, openImageFolder / advanceSlideshow +1 line each
- src/api/ApiServer.cpp, src/test/TestServer.cpp (state fields, render_frame handlers, output_probe)
- tests (3 new files + test_pixel_convert, test_composition, CMakeLists blocks)
- .harmony/probe-{image-load,capture}.*, probe-render-state.{py,json}
- docs
Parallel s-rta-0928 lanes:
- restore follow-up (DeckView/ClipCell/LayerStrip/Clip.h thumbnails, RoutineEngine/Player; restore-diag.md);
- Tier-1 residual follow-up (tests/visual, EmbeddedShaders.h, SourceRegistry.cpp);
- tempo race (recording/BPM).
Only MainComponent.cpp, TestServer.cpp and tests/CMakeLists.txt can meet, in different functions or as append-only
blocks. Rebase before each commit; resolve CMake per notebook.md:1987-1989. Pitfall numbers: take the next free ones
at commit time (48/49 on today's main); lanes collide (notebook.md:1697).
A valid split, if Harmony wants two lanes:
- A = R2 + R3 + R5: captureFrame, the render_frame handlers, output_probe, PngWrite.h, probe-capture;
- B = R1 + R4: everything image, the one gate line in processPendingCapture, probe-image-load.
They conflict only on adjacent /api/state lines.

## RISKS (strongest counterargument first)
R-1 "Async + hold + gate + prefetch is a subsystem to fix a 9-14 ms hitch." It loses:
   - the 9-14 ms is for a 0.66 MP image (F4); decode is O(pixels) and runs on ImageIO whatever the conversion costs;
   - 1080p and 4K stills, and the invisible legacy decode on EVERY trigger or selection (F2), still drop frames, and
     freeze the whole output for ~100+ ms at 4K (INFERRED; R1.0 measures);
   - R1.1 is the simple path and ships first; its gate re-runs i1/i2 to show they stay RED.
   Second counterargument: "hold is a new visual state." It is local to one layer, the capture gate keeps tests
   exact, prefetch makes it rare, and the alternative is today's whole-output freeze of the same length.
R-2 The hold freezes an ANIMATED previous clip for the decode time (4K: ~3-8 frames INFERRED). A column trigger of
   several cold layers appears staggered by about 1 frame per upload. Prefetch removes both for composition clips;
   what remains is a clip dropped and triggered at once.
R-3 A capture waits up to the decode time. It cannot wait forever: every job delivers a result (try/catch), and
   cleared entries re-request; the 5 s timeout is the backstop.
R-4 A 4K upload could exceed the frame budget. R1.0's measured upload= decides U1/U2 before R1.2 is built; i1_4k and
   i2_4k are the check. A first-draw cost of a new texture that lands in the upload frame is measured by the same
   rows. If it stays RED, report the numbers; no further lever is in scope.
R-5 New locks (Sacred Rule 2, discussed explicitly):
   - Mailbox and image-set slot: the GL side is try_lock only; producers hold them for O(1);
   - captureFlight_: callers only;
   - ThreadPool's internal lock: taken by the GL thread only on a cache miss, O(1);
   - no lock is held across I/O or a decode.
R-6 Memory: in flight about 270 MB peak at 4K (R1-d arithmetic, INFERRED). The VRAM ceiling = the composition's
   images; prefetch capped at 1 GiB (ASSUMED). Retain eviction frees the previous composition's textures (a
   reduction vs today).
R-7 Edge behaviour changes, all intended, each documented:
   - a Failed image is not retried every frame (today it is): a file repaired on disk shows after the next
     composition swap or context loss;
   - legacy clear+load in one frame now loads (last wins);
   - a broken sequence frame repeats the last frame instead of showing nothing.
R-8 Renderer.cpp is in no ctest (F8), so the gate and hold wiring are proven live only. The teeth are the
   temporary-edit build for i3 (reverted, sha checked) and the counters rows i4/i4m.
R-9 Perf rows flake under load: the quiet rule, the load printed, and a lone FAIL under load re-run >= 5 times
   (notebook.md:1983).
R-10 R3 changes render_frame FILE bytes (not pixels). No in-repo consumer hashes captures (F17); an external script
   would see the change.
R-11 Decode threads at low priority: holds lengthen under CPU saturation (printed load avg).
R-12 CLAUDE.md is at 23,752 of 25,000 bytes: two index lines (~350 B) fit; `wc -c CLAUDE.md` is part of the docs
   step.

## COMMIT SEQUENCE (each builds, passes ctest and keeps every existing probe GREEN; each reverts alone)
1. `perf(render): peak_callback_ms + image decode/convert/upload timing lines; probe-image-load + probe-capture (RED
   harness)` -- R1.0.
2. `fix(capture): one capture at a time -- flight lock owns the time override and the test canvas lock` -- R2.
3. `feat(state): frame_ring_cells + r1_cells; output_probe via PixelConvert` -- R5.
4. `perf(image): PixelConvert argb->GL rows for loadKeyImage / ImageSequence / TextureManager (byte-identical)` -- R1.1.
5. `perf(image): decode off the GL thread -- ImageDecode + ImageTexCache, a pending layer holds its last picture,
   captures wait for complete frames` -- R1.2.
6. `perf(image): the legacy single image decodes and uploads only when a frame needs it; slideshow prefetches the
   next` -- R1.3.
7. `perf(image): image sequences decode ahead off the GL thread` -- R1.4.
8. `perf(image): prefetch the composition's images at each swap; release images no longer in it` -- R1.5.
9. `perf(capture): fast PNG writer for render_frame (level 1, same pixels); snapshots unchanged` -- R3 (or its revert
   + table).
10. `perf(render): clear canvas targets at creation` -- R4, only if VERIFIED; else nothing.
11. Docs, in the commit that makes each true (listed here once):
    - rendering.md: new "Image loading (s-rta-0928)" section after the Frame Ring paragraph -- decoder, pending !=
      no media, hold rules, capture gate, budget, prefetch/retain, sequences, the legacy lazy path, the /api/state
      fields -- plus peak_callback_ms next to peak_frame_time_ms at :69.
    - pitfalls.md:
      - 48 "an image is decoded off the GL thread -- a pending image is never 0 / no media (the FX-only trap);
        render_frame waits for a complete frame";
      - 49 "one capture at a time: captureFrame owns the time override and the test canvas lock";
      - 37 and 46 amended.
    - CLAUDE.md index: 2 lines (48, 49).
    - testing-eyes.md: the render_frame completeness gate; load_image's sleep no longer load-bearing; serialized
      captures; enc=fast.
    - .harmony/HANDOFF.md items 3 / ledger 3 closed with the numbers.
12. The report commit.
Trailer: the session's attribution line (the renderperf D8 practice).

## DONE LOOKS LIKE
- ctest green: +test_image_tex_cache (11 cases), +test_image_decode (5), +test_png_fast (4 + hidden bench), +3 in
  test_pixel_convert, +1 in test_composition; each with its teeth run once.
- probe-image-load: i1/i2 (1080p + 4K), i3, i4, i4m, i5, i6, i7 GREEN, twice, quiet, load printed; RED lines on
  main + R1.0 recorded verbatim; i3's teeth recorded.
- probe-capture: cr1 and cr2 GREEN; cr3 GREEN, or R3 reverted with its table.
- probe-render-state GREEN = the recorded count + r1_cells' 3 lines.
- crossfade, canvas, effects-parity, fitmode, outputs and deck-clock GREEN at their recorded counts.
- Tier-1 failures file byte-identical to main's.
- abpix rules held.
- The split tables in the report: decode/convert/upload per size (R1.0); held frames per size (i1/i4); capture
  read/convert/png before and after at 1080p and 4K; R4's E0-E3.
- No file outside the FENCE list touched; CLAUDE.md <= 25,000 bytes.

## FILED (found, not in this lane)
1. Message-thread thumbnail decodes (F15) belong to the restore follow-up; suggested reuse of ImageDecode::Decoder +
   a (path, stamp) cache.
2. Video decodes on the GL thread (F16), the twin of R1 for video.
3. ImageSequence keeps every frame's texture for its lifetime (a 300-frame 1080p sequence = ~2.5 GB of VRAM,
   INFERRED arithmetic).
4. `existsAsFile()` (a stat) per image clip per frame on the GL thread (CompositorEngine.cpp:984, :1074, :1209,
   :1439, clipHasContent :1104): I/O that already exists on the render thread. The decoder's stamp could replace it.
5. MainComponent still posts a legacy load on every image trigger or selection in deck mode (F2). After R1.3 it is
   an O(1) request, but the churn is pointless while the deck draws.
6. `ImageSequence::open` and `openMediaForDeck` decode frame 0 on the message thread (ImageSequence.cpp:51,
   MainComponent.cpp:2937).
7. If R4 attributes the leftover to the per-layer router FBO (created at a layer's first use): pre-creating it costs
   33 MB per layer at 4K.

REPORT_FILE: .harmony/.reports/s-rta-0928/plan-renderleft.md
STATUS: FINAL. Summary:
- R1: async decode (ImageDecode + ImageTexCache); pending = hold, never no-media; the capture gate; legacy lazy;
  sequences; prefetch + retain; peak_callback_ms, because the legacy decode sits outside the frame timer.
- R2: the capture flight lock owns the override and the lock.
- R3: a fast PNG for render_frame, ship iff >= 2x.
- R4: measure E0-E3, fix only resize targets.
- R5: C4.

---------------------------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (s-rta-0928, 09:42) — OVERRIDES THE BODY WHERE THEY DIFFER
Authorship: written by an OPUS architect standing in for Fable (Fable quota exhausted; recorded tier deviation).
Blind council: attack-renderleft-gl.md (SOUND_WITH_FIXES), attack-renderleft-vj.md (SOUND_WITH_FIXES). Both confirmed the
core promises (no GL call off the GL thread, try_lock-only on the GL side, weak_ptr job delivery, removeAllJobs before
teardown, no new FBO, no history-key change). ADOPTED as written, commit order unchanged, with these rulings:
- B1 (gl MUST) Every commit builds and passes ctest alone: declare `legacyPendingThisFrame_ = false` in R1.2 (commit 5,
  unused until R1.3), or move that gate term into R1.3 — builder's choice, stated in the report.
- B2 (gl SHOULD) `compositor_.pumpImages()` (drain / upload / evict / prefetch dispatch) runs EVERY frame near the top of
  renderOpenGL, independent of deckActive and before any early return — no pile-up, no skipped eviction in a no-deck or
  source-only frame. An ImageTexCache/renderer test or a probe counter shows the drain ran in a no-deck state.
- B3 (gl SHOULD) Upload-budget spill: a decoded result that misses the per-frame budget STAYS queued with its decoded
  bytes and is uploaded on a later frame without a re-decode (onResult state must allow it); ImageTexCache test
  "budget exhausted mid-drain, resumed next frame".
- B4 (gl SHOULD) Quit time: measure app quit (osascript quit -> process gone) on main and on the lane with a cold-4K
  prefetch in flight; the lane may add <= 0.5 s. If a running decode cannot be interrupted, say so with the number.
- B5 (gl NIT) Pitfall 37 / TEST-ONLY lock: say the lock SPAN narrows (values unchanged) — no "unchanged" wording.
- C1 (vj MUST 1) A crossfade whose INCOMING image is pending does not advance: pause that layer's crossfade clock
  (advanceCrossfade) while its incoming clip texture is pending, so the dissolve starts, smoothly, when the image lands
  (a 1-8 frame late start, never a freeze-then-jump). Rule 15 (an inactive deck keeps time) is untouched: the pause is
  only for a PENDING incoming image. Add a mid-fade row (i2m: dbox or progress at ~30-60% of a 1 s dissolve onto a cold
  image is monotonic, no jump > one frame's worth) — RED on the current app is not required for this row (the defect
  does not exist before R1.2); prove its teeth by a mutation (remove the pause -> row FAILs).
- C2 (vj MUST 2) The "nothing pending" capture gate applies ONLY to render_frame captures (7070 + 8080 REST/Eyes). A
  user snapshot (takeSnapshot: UI Save Snapshot, OSC, REST snapshot) captures as today — this frame, as-is. A flag on
  the capture request, set only by the render_frame handlers. Row: a snapshot taken while a cold 4K image is pending
  returns within its normal time (no 5 s wait).
- C3 (vj MUST 3) Image sequences with nothing to show bump the SAME framePendingImages counter the gate reads (name it
  in R1.4), and add a zero-wait capture row for a freshly triggered ImageSequence (i3s), RED-first where it applies.
- C4 (vj SHOULD) i4m: add a picture (dbox) check that a held mask still keys a changing layer beneath it, if cheap;
  otherwise state why the counter check suffices.
- C5 Boris list (the lane report's boris_checks): on a never-seen image the layer now holds its last picture for 1-3
  frames (1080p) / 3-8 (4K) instead of the whole output hitching one frame; a dissolve onto such an image starts
  1-8 frames late; composition loads prefetch images (memory up to the 1 GiB budget). These are feel calls for Boris.
- C6 FENCE with the restore lane (plan-restore.md, planned in parallel): this lane owns `ImageDecode::Decoder` (new
  header) and the compositor/renderer image paths; thumbnails (ClipCell / LayerStrip / DeckView) are OUT of this lane
  (the restore lane's). Do not edit src/ui/ClipCell.*, LayerStrip.*, DeckView.*.
