# plan-vupload -- video texture upload: per-frame budget, GL-thread QoS, IOSurface ring + blit, context-loss hold, idle trim (s-rta-0929 START HERE 2 + 4)

Lane "vupload" (rows u1.., commit messages "(s-rta-0929 vupload)"). Base: main adf9b8a. Authored by Fable (architect, Law #11
row 2); an opus builder executes it in a worktree; Harmony adopts or overrides after the blind attackers.
Basis read in full: diag-vfps.md + audit-diag-vfps.md (MUST: the prototype's "byte-identical" is NOT verified -- this plan
gates a true per-pixel diff), diag-vfps-tools (instr.diff = the cap + IOSurface prototype arms; agg/q1-table, q2/q2b-table,
main-reference), plan-video.md + HARMONY ADOPTION + ADDENDUM (V1-V7, W1-W5), video.md (all rounds), probe-video.{sh,py,json},
pitfalls 35 / 53 / 54 / 56, BORIS_DECISIONS "Playback Behaviour", the source named in section 1, JUCE 8's
juce_OpenGLContext.cpp / juce_OpenGL_mac.h / juce_Threads_mac.mm, the codebase's own IOSurface precedent (src/output).
Labels: VERIFIED = re-derived from source / the diag's raw aggregates at adf9b8a; INFERRED = reasoned, not run; ASSUMED = a
number the builder must confirm before it becomes a bar.

QUESTION: keep 4 (and more) videos at the display rate when their frame clocks bunch, lift the 4 x 4K ceiling, and close
the filed video items that live in the same code (context-loss FX-only frames, idle ring-slot RSS, the GOP cache ruling) --
phased, each phase its own gated, revertible commit group, under the rig's rules.

APPROACH (recommended, stated first):
P1 a pure, count-based, demand-adaptive per-render-frame video upload budget (`VideoUpload::Budget`, mirrors the images'
   `UploadBudget` pattern; never shared with it): cap = max(2, ceil(avg requests/frame over 16 frames) + 1); a player over
   the cap HOLDS its shown frame (never pending, never Late), bounded to min(2, K-1) render frames (K = render frames per
   content frame) so no content frame is ever skipped and no layer starves; never-shown players and the first frame after a
   GL release are exempt; both chains of a crossfade count (they share the same call path).
P2 the GL render thread at QoS USER_INTERACTIVE, set once per context creation on the thread itself (JUCE creates it as a
   plain std::thread at DEFAULT): fixes wake-up latency and halves lost display-link ticks; does NOT fix the bunched frame
   cost, the fps tail, or E-core placement of a light GL thread.
P3 IOSurface-backed ring slots (BGRA, the codebase's SharedFrameSet precedent) bound once per slot to a GL_TEXTURE_RECTANGLE
   read FBO; one `glBlitFramebuffer` per NEW frame into the existing GL_RGBA8 sampler2D `texture_` (the effect chain is
   untouched); a per-slot fence polled with timeout 0; a slot returns to the decode thread only after its fence signals;
   OpenGL 4.1 core; 3 rect + 3 read FBO + 1 dst FBO + <= 3 syncs per player (bounded); the decode thread's sws writes into
   the locked IOSurface (negative stride, as today); malloc + glTexSubImage2D stays the non-Apple and allocation-failure path.
P4a the frame ON SCREEN keeps its ring slot (Reading) until a newer frame is shown (`VideoRing::Retire`): after a context
   loss the first `uploadToTexture` re-uploads it, so a playing clip never returns 0 -- gated by a CGL ctest (RED on main)
   and a TEST-ONLY `POST /api/debug/gl_context_cycle` (detach + re-attach the preview context; no window is opened).
P4b R-14: a player not drawn for 1 s drops its Ready slots (reader-owned) and asks its parked decode thread to purge the
   Free slots (`IOSurfaceSetPurgeable(kIOSurfacePurgeableEmpty)`; free()/malloc() on the portable path); the writer
   un-purges a slot when it next acquires it; the held (shown) slot is never purged.
P4c the GOP cache (reverse / ping-pong on long-GOP files) is RULED OUT of this lane: its own lane (section 2, R-12).
OUT: VideoToolbox (filed with the diag's price: software 14.2 ms vs VT 0.87 ms CPU per 4K frame, 333 frames/s for 4
   streams; 0 fps at the measured load -- headroom only; HAP / ProRes 4444 / 10-bit stay software; NV12 needs a YUV shader).

## 1. TODAY'S CODE, RE-DERIVED (file:line at main adf9b8a; VERIFIED unless marked)

Upload path (`src/media/VideoPlayer.cpp`)
- :151-153 (`VideoPlayer.h`) `kSlots = 3`, `slotBytes_` (malloc), `VideoRing::Ring<3> ring_`; :156-163 GL-thread state
  (`texture_`, `textureCreated_`, `shown_`, `lastShownPts_`, `firstDrawMs_`, `firstFrameFailed_`, `releasedThisFrame_`).
- :171-195 open(): sws to `AV_PIX_FMT_RGBA`, `rowBytes_ = width_ * 4`, 3 malloc'd slots (page-lazy); :204-216 frame 0
  converted into slot 0 (gen 0, pts clamped <= 0) and the thumbnail, on the message thread (or asyncload's pool thread).
- :397-483 uploadToTexture: :408-409 `ring_.pick(currentTime_, gen, 0.5 fd, (kSlots + 1) fd)`; :415-446 upload only when
  `shown_.needsUpload(p.seq)`: glTexImage2D once (:421-430, GL_RGBA8 / GL_RGBA / GL_UNSIGNED_BYTE), then glTexSubImage2D
  (:433-437); :447-450 `lastShownPts_`, `ring_.release(p.slot)` AT ONCE ("client-memory glTex*Image2D copies before it
  returns"), `releasedThisFrame_ = true`; :453-462 W3; :464-482 judge -> `return texture_`.
  THE P4a BUG: after `releaseGL()` (:485-500: texture deleted, `texture_ = 0`, `textureCreated_ = false`, `onReleaseGL`)
  a PLAYING clip whose next content frame is 1-4 render frames away picks nothing (its shown slot was released at :448
  and re-used by the writer), judge says Held / Late, and :482 returns `texture_ == 0` with `*pending == false` --
  `CompositorEngine.cpp:1121-1130` (Opaque / Transparent), :1354-1385 (persistent) then run the clip's effects FX-only
  over the layers below (Pitfall 53's trap), until the next pick. V1 keeps it a hold in name only (no picture to hold).
- :555-560 park(); :562-615 decodeLoop (idle check :575-579, generation / reseek :581-588, NONREF :590-592);
  :617-653 onDecoded (ring-full wait :635-648 with the V2 park :639-643); :725-734 convertInto (bottom-up negative stride).
- `VideoRing.h` :29-151 Ring (Free -> Writing -> Ready -> Reading -> Free; only the reader leaves Ready / Reading, only
  the writer leaves Free / Writing; pick :73-123 frees stale / ahead / skipped); :225-232 ShownState (V1).
- `VideoStats.h` :7-29 (relaxed atomics; `take*` reset on read); `/api/state` blocks `src/api/ApiServer.cpp:1374-1393`
  and `src/test/TestServer.cpp:681-701` ("Same fields in ApiServer and TestServer" -- the video lane's precedent).
Renderer (`src/render/Renderer.cpp`)
- :227-283 newOpenGLContextCreated -- runs ON THE RENDER THREAD (JUCE `initialiseOnThread`, juce_OpenGLContext.cpp:621-671);
  :250 `compositor_.setImageDecoder(&imageDecoder_, &uploadBudget_)`; :266-278 media-pending provider (`neverShown()`).
- :285-321 renderOpenGL top: :311 `uploadBudget_.reset()` (the images' per-frame byte budget, `ImageTexCache.h:259-271`:
  first upload always admitted, cap 8 MiB); :321 drainRetiredMedia; :323 scanSequenceVram (the per-frame O(#sequences)
  idle scan under imageSeqMutex_ -- the pattern P4b copies for players, :1645-1697).
- :680-683 `realDt` (clamped 0-0.25 s); :711 compositeDeck(.., realDt, ..) -> the compositor calls `videoFrameFn_`
  (`CompositorEngine.cpp:1103`, :1211, :1370, and :1688 `getClipTexture(*prevClip..)` for the OUTGOING chain of a
  crossfade -> :1607 `videoFrameFn_`) -> `getVideoFrameTexture` :1728-1731 -> `syncMedia` :1733-1830: the lookup under
  `videoPlayerMutex_` (:1751-1758), transport sync, `advanceFrame(dt)` (:1786), :1821-1829 `uploadToTexture(&videoPending)`
  and `notePendingImage()` for the C3 gate. Both crossfade chains therefore reach the SAME upload path (both count).
- :754 `DeckClock::tick(other, realDt, tickMediaClock)` (decode = false: `advanceClock`, no upload) for off-screen decks.
- :1135-1230 openGLContextClosing: `player->releaseGL()` for every player (:1160-1166) and the retired ones (:1186).
  JUCE 8 calls it from `CachedImage::pause()` on the MESSAGE thread with the context activated after the render thread
  has removed this image (juce_OpenGLContext.cpp:176-209 stop/pause; `renderThread->remove` :191 takes callbackMutex, so
  no renderOpenGL runs concurrently) -- releaseGL never races uploadToTexture; the decode thread keeps running.
- :1473-1509 openVideoForClip / installVideoPlayer (asyncload's seam; untouched here); :1571-1635 drainRetiredMedia.
- `Renderer.h` :56-57 attachTo / detach public; :98 `getContext()` public; :304 `uploadBudget_`; :305-316 the seqvram frame
  fields (`seqFrameSerial_`, `seqDeletes_`); :629-631 `videoPlayerMutex_` / `videoPlayers_` / `videoStats_`.
GL thread creation and pacing (JUCE, `build/_deps/juce-src/modules/juce_opengl`)
- `opengl/juce_OpenGLContext.cpp:888-892` `std::thread thread { [this] { Thread::setCurrentThreadName ("OpenGL Renderer");
  while (flags.waitForWork (renderAll() != noWork)) {} } }` -- no priority, no QoS -> QOS_CLASS_DEFAULT (diag: 0x15,
  priority 31 in 218/218 launches). :746-966 one `RenderThread` per process (`SharedResourcePointer`): the Output windows'
  contexts render on the SAME thread. :258-259 / :904-919 `pendingRender` is a FLAG set from the CVDisplayLink callback;
  `native/juce_OpenGL_mac.h:178-210` swapBuffers = `[renderContext flushBuffer]` + the underrun sleep (never fired: diag).
- `juce_core/native/juce_Threads_mac.mm:49-58` Priority::normal -> QOS_CLASS_DEFAULT (the decode threads,
  `VideoPlayer.h:207`); highest -> USER_INTERACTIVE.
IOSurface precedent (this codebase, macOS)
- `src/output/SurfacePool.cpp:33-50` createSurface: `kIOSurfaceWidth/Height/BytesPerElement 4/PixelFormat 'BGRA'`
  (plain C++ .cpp under `#if defined(__APPLE__)`, no .mm); `SharedFrameSet.cpp:96-118` `CGLTexImageIOSurface2D(cgl,
  GL_TEXTURE_RECTANGLE, GL_RGBA8, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0)` + an FBO on the rect texture on
  the app's 4.1 core context; :62-70 blit canvas -> rect FBO + `glFenceSync` + glFlush (a different context reads);
  :74-84 releaseGL drops fences ("a sync object dies with its context"); `OutputPresenter.cpp:33-72` binds per generation
  (the reader-side rule). `CMakeLists.txt:591-596` links IOSurface to the app on Apple whether or not Syphon is built.
  `tests/CMakeLists.txt:2246-2290` test_surface_pool (no GL) and test_shared_frame_gl (private CGL contexts, SKIPs loudly
  without a pixel format) -- the GL-in-ctest precedent this plan's `test_video_player_gl` copies.
Probe rig
- `.harmony/probe-video.sh` launches PRODUCTION mode (7070); `probe-video.py:562-631` steady_scene / steady (w1: 5 fresh
  loads, sequential `trigger_clip` + `wait_active` per layer -- the trigger sequence that decides the bunch pattern, diag
  step 2), :279-322 load / trig / wait_active; `probe-video.json` fpsMin 110, fps4kMin 80 (W1), (a2) INFO (W1b),
  uploads5s [540, 660], lateMaxSteady4k 12. `/api/trigger_column {"column": N}` (`ApiServer.cpp:169`, handler parses
  "column") -- the diag's `w1col` scene: ONE trigger_column = pattern 4 on 291/291 windows.
- Test mode (`src/Main.cpp:14-26 --test-mode`) starts the TestServer on 8080 (`MainComponent.cpp:1863`) AND the ApiServer
  on 7070 (:1895, unconditional): a test-mode launch serves both. `TestServer` holds `Renderer& renderer_` (TestServer.h:154).
- Diag reference numbers (agg/main-reference.txt, MAIN app adf9b8a, w1col, 5 launches x 3 loads): fps median 115.8, min
  110.5; per-load median `peak_callback_ms` 6.30-6.47 (15/15); an earlier 15-load set 112.7 [107.1, 115.1]; uploads 600-610
  per 5 s, late 0 -- and the counterfactuals (agg/q1-table: cap 2 119.8 [116.9, 120.2], render p90 3.48 ms, 604 uploads,
  none skipped; cap 1 119.9 but 586 uploads = ~3 % skipped; QOS=4 119.0 [111.1, 120.1], lost ticks 2.1 -> 1.1/s;
  agg/q2, q2b: 4 x 4K base 89.75 / 90.55, IOSurface + blit 104.95 / 106.85 (per-upload GL CPU 0.96 -> 0.08-0.10 ms), no
  upload 111.05 / 116.0, cap 1 94.85; BGRA / PBO / QoS 0; client storage 28.0).
- CLAUDE.md is 24,980 B of 25,000 (VERIFIED `wc -c`); `docs/claude/pitfalls.md` ends at 57 plus an asyncload "NN." entry
  (:125) -- this lane's entry is "NN (vupload)"; Harmony numbers both at merge (next free = 58).

## 2. RULINGS (every design fork; the strongest counterargument to each is in section 8)

- R-1 Budget unit = COUNT of uploads per render frame, not bytes. A byte budget shared with the images (`uploadBudget_`,
  8 MiB, "first always") would admit ~1 x 1080p or 0 x 4K after any image upload, and a 16-image load (pumpImages runs
  first at the frame top, :311-313) would starve every video for 16+ frames -> skipped content frames. The images'
  budget is MIRRORED (same reset point, same "the frame's allowance" idea, same Pitfall 53 hold semantics), never shared.
- R-2 Adaptive cap = max(kFloor 2, ceil(mean requests per frame over the last 16 frames) + 1). 4 x 30 fps at 120 Hz:
  mean 1.0 -> cap 2 (= the measured cap-2 arm: 119.8 fps, 604 uploads, none skipped). 8 players: mean 2 -> cap 3.
- R-3 Bounded deferral instead of an explicit rotor. A player asks again every frame while its new frame is the newest at
  or before its clock; it is force-admitted once deferred for maxDefer = clamp(floor(K) - 1, 0, 2) frames, K = frameDur /
  renderDt (per player, per frame). Its next content frame becomes pickable only K frames after this one did, so a frame
  admitted within K-1 frames is never skipped by the budget; a player with K <= 1 (120 fps content) is never deferred.
  "No layer starves" is exactly this bound, and it is what the ctest asserts for any N and K. Because a player that
  uploaded does not ask again for K frames, the deferred set of a bunch naturally rotates through the bunch in visit
  order (frame t: A B; t+1: C D; ...); an explicit rotor would add state to buy nothing measurable.
- R-4 A deferred player HOLDS: `uploadToTexture` returns `texture_` with `*pending = false`, does not pick (no slot
  changes state), counts `uploadsDeferred` + `holdFrames`, never `lateFrames` (it does not go through `judge`). The
  hold is <= 2 render frames (16.7 ms) at 30 fps, always shorter than the 1.5-frame Late threshold.
- R-5 Exempt from the budget: `!shown_.everShown` (first frames: C1 / C3 untouched) and `!textureCreated_` (the first
  upload after a GL release: the picture must come back on the first frame).
- R-6 Admission happens BEFORE the pick (`Ring::peek`, const, no CAS): a deferral leaves the ring exactly as it was.
  Stale / far-ahead frees wait for the next pick (<= 2 frames); no un-pick, no partial pick.
- R-7 The shown frame's slot is the reader's until a newer frame is shown (`VideoRing::Retire::held`): the writer has 2
  slots of look-ahead instead of 3 (needs <= 1 at 30 fps on 120 Hz; the catch-up is decode-bound, not slot-bound);
  `pol.reseekBehindSec = (kSlots + 1) fd` stays a valid upper bound. kSlots stays 3 (RSS unchanged); if w2 (e) / w3 (c)
  regress, kSlots = 4 is the one-constant fallback (+33 MB per 4K player).
- R-8 IOSurface pixel format = 'BGRA' with GL_BGRA / GL_UNSIGNED_INT_8_8_8_8_REV (the prototype AND SharedFrameSet);
  sws converts every source format to AV_PIX_FMT_BGRA on the surface path. The blit is a GPU copy between two GL_RGBA8
  images: the effect chain reads the same RGBA values, alpha included (straight alpha, as today). Every format the app
  plays (yuv420p, yuv420p10le, yuv422p10le, yuva444p10le, rgb0 / rgba HAP, pal8 ...) takes the surface path; the malloc
  path is kept ONLY for non-Apple builds and for a player whose IOSurfaceCreate fails (per player, decided in open()).
- R-9 The blit source is re-bound before each blit (`glBindTexture(GL_TEXTURE_RECTANGLE, rect); glBindTexture(.., 0)`) --
  the reader-side rule of CGLIOSurface.h the codebase already follows (OutputPresenter.h:28). INFERRED necessary for a
  CPU writer in the same context; costs nothing; the per-pixel gate (w10) is the judge.
- R-10 Fence protocol: each blit gets `glFenceSync`; `uploadToTexture` polls every fence with `glClientWaitSync(f, 0, 0)`
  first thing; a signaled, non-held slot -> `ring_.release`; the held slot's fence is deleted when signaled but the slot
  stays Reading. A context loss drops the fences (they die with the context) and releases every fenced non-held slot;
  the held slot survives (it is the hold source). `texture_` needs no fence: the same context's command stream orders
  frame t's draws before frame t+1's blit into it (exactly today's glTexSubImage2D hazard, none).
- R-11 GL objects per player: 3 rect textures + 3 read FBOs + 1 dst FBO + 1 texture + <= 3 syncs, created lazily on the
  GL thread, deleted in releaseGL (Pitfall 54's bound). No GL call outside `uploadToTexture` / `releaseGL` / `trimIfIdle`.
- R-12 GOP cache: OWN LANE. It changes decodeLoop's seek / catch-up policy and adds a second memory budget (a 1080p GOP of
  250 frames is 2 GB of RGBA; it needs an eviction design like SeqVram's) with a rate gate unrelated to this lane's; it
  does not touch the upload path. File "lane gopcache (R-12): bounded reverse-decode window or GOP cache for reverse /
  ping-pong on long-GOP files; gate = reverse-play fps row; fence VideoPlayer::decodeLoop / seekToTimestamp only".
- R-13 QoS is set on the render thread in `newOpenGLContextCreated` (runs on that thread; re-runs per context so a
  recreated JUCE thread gets it again). Decode threads stay `normal` (the QOS=1 arm gained 0 at 4K).
- R-14 Idle trim is two-owner by construction: the GL thread (reader) drops Ready slots and sets a request; the decode
  thread (writer) purges only Free slots and un-purges a slot after it acquires it. Nothing else may touch purgeable
  state. Idle = no draw for kTrimIdleMs = 1000 ms (the thread parks at 250; a crossfade's 1-3-frame outgoing skip and a
  fenced frame's 1-2-frame hold are never idle). The held slot is never purged (33 MB residual per idle 4K player).
- R-15 Counters: `video_*` fields go in BOTH state blocks (the video lane's precedent; probe-video runs in production on
  7070). TEST-ONLY things -- `gl_context_gen`, `gl_thread_qos`, `phys_footprint_mb`, `POST /api/debug/gl_context_cycle`
  -- live in TestServer only (8080); their rows run a test-mode launch (which also serves 7070).
- R-16 Bars: w1c's from the diag (RED on main by 2.3 ms / 3-6 fps); the 4K bar ONLY from the builder's interleaved A/B
  (rule in 4.7); every existing threshold untouched (w2 (a2) stays INFO); nothing re-thresholded, ever.

## 3. TRADEOFFS CONSIDERED (rejected)

- Share the images' byte budget (Pitfall 53 "share"): rejected (R-1) -- video would starve behind image loads.
- Offset the clock phases at trigger (diag option 6): mixed frame rates drift back into coincidence; shifts cue timing.
- Decoder stagger (diag arm): +1.3 fps, does not remove the bunched frame.
- Sample the rectangle texture directly (no blit): CGLError 10008 on GL_TEXTURE_2D; sampler2DRect needs a compositor
  shader variant (Pitfall 39: `layer_transform` is one shared program); at most the B share (6-9 fps, by difference).
- PBO / BGRA client upload / client storage: measured 0 / 0 / -62 fps.
- VideoToolbox: 0 fps at the measured load; medium-high risk (HAP / 4444 / 10-bit stay software; NV12 shader; seek
  semantics). Filed, not built.
- A 4th ring slot instead of the held-slot rule: +33 MB per 4K player for the same hold; kept as the fallback (R-7).
- Deferring by un-picking (Reading -> Ready): a new reader transition and a partially applied pick; peek is simpler (R-6).
- Trim by the reader alone (purge from the GL thread): races the writer's Free -> Writing CAS (a purge landing after the
  writer's un-purge = a garbage frame); ownership split instead (R-14).
- `kIOSurfacePurgeableVolatile` (reclaim only under pressure): not measurable on a 32 GiB machine; Empty discards now.

## 4. DECISION / SPEC

### 4.1 `src/media/VideoUploadBudget.h` (NEW, pure: no JUCE, no GL, no FFmpeg)
```cpp
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
// s-rta-0929 vupload P1: the per-render-frame VIDEO upload budget (count-based; mirrors ImageTexCache::UploadBudget's
// frame-top reset, never shared with it: R-1). Owned by the Renderer, reset at the frame top, consulted by every drawn
// player's uploadToTexture BEFORE its pick. Pure: tests/test_video_upload_budget.cpp.
namespace VideoUpload
{
constexpr int kFloor = 2;            // no demand history yet: 4 x 30 fps on 120 Hz spreads 2 + 2 (the measured cap-2 arm)
constexpr int kHistory = 16;         // >= 4 content-frame periods at 30 fps / 120 Hz
constexpr int kMaxDeferFrames = 2;   // a ready frame is held at most this many render frames (16.7 ms at 120 Hz)

// Render frames a ready frame may wait without the NEXT content frame becoming pickable first: floor(K) - 1, K = render
// frames per content frame. K <= 1 (content at or above the render rate): never deferred.
inline int maxDefer(double frameDur, double renderDt)
{
    if (frameDur <= 0.0 || renderDt <= 0.0) return 0;
    const int k = static_cast<int>(std::floor(frameDur / renderDt + 1e-9));
    return std::clamp(k - 1, 0, kMaxDeferFrames);
}

struct Budget
{
    int cap = kFloor, used = 0, requests = 0;
    std::array<int, kHistory> hist{};
    int h = 0;
    // Frame top: yesterday's demand into the history; cap = max(kFloor, ceil(mean demand) + 1).
    void beginFrame()
    {
        hist[static_cast<size_t>(h)] = requests;
        h = (h + 1) % kHistory;
        int sum = 0;
        for (int x : hist) sum += x;
        cap = std::max(kFloor, static_cast<int>(std::ceil(static_cast<double>(sum) / kHistory - 1e-9)) + 1);
        used = 0;
        requests = 0;
    }
    // One drawn player with a NEW frame ready. True = upload now. False = hold this frame (ask again next frame).
    bool admit(int deferredFrames, int maxDeferFrames)
    {
        ++requests;
        if (used < cap || deferredFrames >= maxDeferFrames) { ++used; return true; }
        return false;
    }
};
} // namespace VideoUpload
```

### 4.2 `src/media/VideoRing.h` additions (pure; the 15 existing cases untouched)
- `Ring::peek(clock, gen, tolSec) const -> Pick{slot, pts, seq}`: the slot pick() WOULD choose (same rule: newest Ready
  of this gen with pts <= clock + tol; ties by seq), with NO state change. Ctest: peek == the following pick on the
  case-1 ring, on a ring holding stale-gen frames, and when nothing qualifies.
- `Ring::dropReady() -> int`: every Ready slot -> Free (reader-owned; R-14). Ctest: Reading / Writing slots untouched.
- `template <int N> struct Retire` (the reader's ownership beyond the pick; the GL calls are the caller's):
```cpp
template <int N> struct Retire
{
    int held = -1;                 // the slot of the frame on screen: stays Reading until a newer frame is shown (P4a)
    std::array<bool, N> fenced{};  // a GPU copy from this slot was issued and is not yet known complete (P3)
    // A new frame is shown from `slot` (its copy issued now; fenced iff hasFence). Returns the previously held slot to
    // release NOW (-1 = none, or it is still fenced: release it when its fence signals).
    int shown(int slot, bool hasFence);
    // The caller saw slot's fence signal. True = release the slot now (it is not the held one).
    bool signaled(int slot);
    // The context died: every fence with it. Fills `out` with the slots to release now (fenced, not held); the held
    // slot stays (the hold source of the next context). Returns the count.
    int contextLost(std::array<int, N>& out);
};
```
  Ctests (test_video_ring, new cases): (a) shown(0,false) -> -1, held 0; shown(1,false) -> 0, held 1. (b) shown(0,true);
  shown(1,true) -> -1; signaled(0) -> true; signaled(1) -> false (held); shown(2,true) then signaled(1) -> true.
  (c) contextLost with held 1 and 0 fenced -> {0}, held stays 1, no fence left. (d) signaled on an unfenced slot -> false.
  (e) the held slot is never in contextLost's output. Teeth (the lane's named pattern): "release the held slot on
  signaled" -> (b) fails; "contextLost clears held" -> (c) fails.

### 4.3 `src/media/VideoStats.h`
Add `std::atomic<int64_t> uploadsDeferred{0}, holdNoTexture{0}, slotsPurged{0};` and `std::atomic<int> uploadCap{0};`.
`/api/state` (both servers, inside the existing video block, same order): `video_uploads_deferred`,
`video_hold_no_texture` (a drawn, previously shown player returned texture 0 -- the FX-only witness; must stay 0),
`video_slots_purged`, `video_upload_cap` (INFO).

### 4.4 `src/media/VideoPlayer.{h,cpp}` (the only media class touched)
Header additions (private): `bool useSurfaces_ = false; int deferredFrames_ = 0; bool trimmed_ = false;
std::atomic<bool> trimRequested_{false}; std::array<bool, kSlots> purged_{}; VideoRing::Retire<kSlots> retire_;`
and under `#if JUCE_MAC`: `std::array<void*, kSlots> surf_{}; std::array<GLuint, kSlots> rectTex_{}, readFbo_{};
std::array<GLsync, kSlots> fence_{}; GLuint dstFbo_ = 0;`. Signature: `GLuint uploadToTexture(bool* pending,
VideoUpload::Budget* budget = nullptr, double renderDt = 0.0);` (null budget = no deferral: ctests, tools). New:
`void trimIfIdle(int64_t nowMs)` (GL thread, frame top), `static int64_t nowMs()` made public (the Renderer's scan).
Private helpers: `bool createSurfaces()`, `void uploadSlot(int slot)` (blit or client upload; creates `texture_` + dst
FBO lazily), `void pollFences()`, `void purgeFreeSlots()` (decode thread), `void unpurge(int slot)` (decode thread).

open() (:171-195): decide the slots BEFORE the sws context. `#if JUCE_MAC`: `createSurfaces()` = 3 x IOSurfaceCreate
(width, height, BytesPerElement 4, 'BGRA' -- copy SurfacePool.cpp:33-50's CFDictionary code; no .mm); on success
`useSurfaces_ = true`, `rowBytes_ = IOSurfaceGetBytesPerRow(surf_[0])` (assert all three equal, else release them and
fall back), `slotBytes_[i] = IOSurfaceGetBaseAddress(surf_[i])` (stable for the surface's life); sws dst =
`useSurfaces_ ? AV_PIX_FMT_BGRA : AV_PIX_FMT_RGBA`. Failure (or non-Apple): today's malloc path, `rowBytes_ = width_ * 4`,
one log line. Odd widths: bytesPerRow is padded (63 px -> 256 B): the negative-stride conversion already uses
`rowBytes_`; the client-upload fallback of a surface player MUST set `glPixelStorei(GL_UNPACK_ROW_LENGTH, rowBytes_ / 4)`
and reset it to 0 (today's code assumes tight rows).
convertInto(slot): `if (useSurfaces_) IOSurfaceLock(surf, 0, nullptr);` sws (negative stride, unchanged);
`IOSurfaceUnlock`. Runs on the message / pool thread for frame 0 and on the decode thread after start() (as today).
onDecoded ring-full loop (:634-648): after `acquireWrite` succeeds: `unpurge(s)` (no-op unless purged_[s]); then
convertInto. decodeLoop top (:572, BEFORE the idle check): `if (trimRequested_.exchange(false)) purgeFreeSlots();`
(the parked thread is notified by trimIfIdle; it purges and parks again).
uploadToTexture (replaces :397-483; the W3 / judge tail stays):
```
*pending = false; if (!open_) return texture_; firstDrawMs_ stamp (W3);
pollFences();  trimmed_ = false;                                     // P3 releases; P4b: drawn again
const auto pk = ring_.peek(currentTime_, gen, 0.5 * frameDur_);
if (pk.slot >= 0 && shown_.needsUpload(pk.seq) && budget && shown_.everShown && textureCreated_
    && !budget->admit(deferredFrames_, VideoUpload::maxDefer(frameDur_, renderDt)))
{   ++deferredFrames_; stats: ++uploadsDeferred, ++holdFrames; return texture_; }   // R-4: a hold, never pending
deferredFrames_ = 0;
const auto p = ring_.pick(...);  stats framesSkipped as today;
if (p.slot >= 0)
{   if (shown_.needsUpload(p.seq)) { uploadSlot(p.slot); shown_.onUpload(p.seq); stats uploads / peakUploadMs; }
    lastShownPts_ = p.pts;
    const int prev = retire_.shown(p.slot, useSurfaces_ && fence_[p.slot] != nullptr);
    if (prev >= 0) { ring_.release(prev); releasedThisFrame_ = true; }
    return texture_;
}
if (!textureCreated_ && retire_.held >= 0)                            // P4a: the picture is back on the FIRST frame
{   uploadSlot(retire_.held); return texture_; }
W3 + judge as today; on Held / Late with texture_ == 0: ++stats_->holdNoTexture (the witness; must never fire now).
```
uploadSlot(slot): surface path = lazily `glGenTextures` rect + `CGLTexImageIOSurface2D(GL_TEXTURE_RECTANGLE, GL_RGBA8,
w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surf, 0)` + read FBO (log once + fall back to the BGRA client upload of the
surface bytes under a read-only IOSurfaceLock if it errors), lazily `texture_` (GL_RGBA8, w x h, data nullptr, the same
LINEAR / CLAMP params) + dst FBO; save GL_READ/DRAW_FRAMEBUFFER_BINDING; re-bind the rect texture (R-9); bind read =
readFbo_[slot], draw = dstFbo_; `glBlitFramebuffer(0,0,w,h, 0,0,w,h, GL_COLOR_BUFFER_BIT, GL_NEAREST)`; restore the
bindings; `fence_[slot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0)`; no glFlush (same context reads). Orientation:
the slot is bottom-up, the rect texture's row 0 is the slot's first row = the image's bottom row, the 1:1 blit keeps it:
identical to today's glTexSubImage2D. Malloc path = today's :419-437 code moved here.
pollFences(): per slot with a fence: `glClientWaitSync(f, 0, 0)` -> ALREADY_SIGNALED / CONDITION_SATISFIED ->
`glDeleteSync`, null; `if (retire_.signaled(i)) { ring_.release(i); releasedThisFrame_ = true; }`.
releaseGL() (:485-500): delete rect textures, read FBOs, dst FBO, texture_ (as today); `fence_ = {}` (dropped, never
glDeleteSync'd: SharedFrameSet.cpp:80 rule); `retire_.contextLost(out)` -> `ring_.release` each; keep `retire_.held`;
`textureCreated_ = false; shown_.onReleaseGL();` V1's paused-clip bump unchanged. Runs on the message thread inside JUCE's
pause() (section 1): no concurrent uploadToTexture, the decode thread only ever touches Free / Writing slots.
trimIfIdle(nowMs) (GL thread, Renderer::scanVideoIdle): `if (trimmed_ || !shown_.everShown || nowMs - lastDrawMs_ <=
kTrimIdleMs) return; pollFences(); ring_.dropReady(); trimmed_ = true; trimRequested_.store(true); thread_.notify();`
purgeFreeSlots() (decode thread): for each slot whose header state is Free (writer-owned: only the writer can leave
Free) and !purged_: Apple `IOSurfaceSetPurgeable(surf, kIOSurfacePurgeableEmpty, nullptr)`; portable `std::free` +
null; `purged_ = true; ++stats_->slotsPurged`. unpurge(s) (decode thread, after acquireWrite): Apple
`IOSurfaceSetPurgeable(surf, kIOSurfacePurgeableNonVolatile, nullptr)` (the base address is unchanged: INFERRED --
verified by u4b (d) and test_video_player_gl case 5); portable `std::malloc` (page-lazy) + pointer store BEFORE publish
(release). The held slot and Ready slots are never purged: the GL thread never reads a purged pointer.
~VideoPlayer(): after `thread_.stopThread`: releaseGL as today, then `CFRelease` each surface (Apple) / free (portable).
Constants (named levers, VideoPlayer.h): `kTrimIdleMs = 1000`.

### 4.5 `src/render/Renderer.{h,cpp}`
- Renderer.h: `#include "media/VideoUploadBudget.h"`; `VideoUpload::Budget videoUploadBudget_;` beside `uploadBudget_`
  (:304); `void scanVideoIdle();` beside `scanSequenceVram` (:311); `std::atomic<int> glThreadQos_{-1};
  std::atomic<uint64_t> glContextGen_{0};` + getters (TestServer).
- Renderer.cpp:311 after `uploadBudget_.reset()`: `videoUploadBudget_.beginFrame(); videoStats_.uploadCap.store(
  videoUploadBudget_.cap)`. :323 after `scanSequenceVram()`: `scanVideoIdle()` = `{ const int64_t now =
  VideoPlayer::nowMs(); std::lock_guard<std::mutex> lock(videoPlayerMutex_); for (auto& [id, p] : videoPlayers_)
  p->trimIfIdle(now); }` (one more O(#players) critical section on the lookup mutex, like scanSequenceVram on
  imageSeqMutex_; w7 (a) `msg_video_lock_wait_max_ms <= 2` gates it).
- :1823 `player->uploadToTexture(&videoPending, &videoUploadBudget_, static_cast<double>(dt))` (dt = realDt).
- :227 newOpenGLContextCreated, first lines: `#if JUCE_MAC pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
  glThreadQos_.store(static_cast<int>(qos_class_self())); #endif glContextGen_.fetch_add(1);` (includes `<pthread.h>`,
  `<sys/qos.h>` under JUCE_MAC). Re-runs per context creation on purpose (JUCE may recreate its thread).
- openGLContextClosing: unchanged (the players' releaseGL loop does the P3/P4a work).

### 4.6 `src/test/TestServer.{h,cpp}` (TEST-ONLY) and `src/api/ApiServer.cpp`
- Both video blocks: the four new `video_*` fields (4.3).
- TestServer state only: `gl_context_gen` (renderer getter), `gl_thread_qos` (-1 until the first context; 33 =
  QOS_CLASS_USER_INTERACTIVE), `phys_footprint_mb` (`proc_pid_rusage(getpid(), RUSAGE_INFO_V4, ..)` ->
  `ri_phys_footprint / 2^20`; Apple only, else absent).
- TestServer route `POST /api/debug/gl_context_cycle` (registered beside `/api/output_probe`, :244): 409 when
  `renderer_.getContext().getTargetComponent() == nullptr`; else `juce::MessageManager::callAsync([this] { auto& r =
  renderer_; if (auto* c = r.getContext().getTargetComponent()) { r.detach(); r.attachTo(*c); } })` and answers
  `{ok, gen_before}` at once (JUCE asserts the message thread in `remove`; `Renderer::attachTo` re-applies the context
  options, Renderer.cpp:48-54). The probe waits for `gl_context_gen` to advance. Never opens a window; never in production.
  Docs: testing-eyes.md (section 10).

### 4.7 Probe rows
probe-video.py / .json (production, 7070; every existing row and threshold untouched):
- NEW `w1c_column_trigger_1080x4` (after w1): 3 fresh loads per launch (new clip ids), each: load 4 x a1080 on the 1080p
  canvas, ONE `POST /api/trigger_column {"column": 0}`, wait_active x4 (<= 10 s), 2 s, a 5 s window polled every 50 ms
  (Poller(FPS_POLL)), s1. Per load print: median fps, median / p90 of `peak_callback_ms` polls, uploads, late, deferred,
  skipped, cap, %cpu, load avg. PASS (a) in EVERY load the median of the per-poll `peak_callback_ms` <= w1cPeakMedianMaxMs
  4.0; (b) the median over the loads of each load's median fps >= w1cFpsMin 118.5 (run the .sh twice = 6 loads for the
  merge verdict); (c) per load `video_uploads` delta >= w1cUploadsMin 595 AND `video_late_frames` delta == 0 (the
  anti-naive-cap guard: cap 1 reads 586). INFO: `video_uploads_deferred`, `video_frames_skipped`, `video_upload_cap`.
- NEW `w2c_steady_4kx4_blit` (after w2): 4 x a4k on the 4K canvas, 2 loads (no stills), the w2 window. PASS (a) each
  load's median fps >= fps4kBlitMin (json; null = INFO until 4.9's evidence sets it); (b) uploads in uploads5s;
  (c) late <= lateMaxSteady4k; (d) gl_video_decode_calls 0. Print `peak_video_upload_ms` (per-upload GL CPU: 0.9 -> 0.1).
- NEW `w10_pixel_identity` (last): per fixture in {a1080_g250.mp4 (yuv420p), pr1080.mov (yuv422p10le), pr4444a_1080.mov
  (prores_ks profile 4, yuva444p10le, NEW), hapa_1080.mov (hap, -format hap_alpha, rgba, NEW), x265_10_1080.mp4 (libx265
  yuv420p10le, NEW, SKIP with an INFO line when the encoder is absent)}: 1080p canvas; layer 130 (lower) = warm.png
  triggered; layer 131 (upper, Transparent -- confirm the enum in src/model/Layer.h) = the video clip with "speed": 0.0
  (frame 0 held) -> trig, `videos_pending` 0, cap `<f>_f0`; reload with inPoint 0.5 -> trig + prime retrigger (seek to
  frame 150), settle_late, cap `<f>_f150` (code band == 150 exactly: speed 0). With `VIDEO_REF_DIR` set: PASS max
  |diff| over all channels <= 1 (numpy int16, K2 = 1/255) against the reference PNG of the same name; unset: write them
  and print INFO. The alpha fixtures use `format=rgba,geq=a='255*X/W'` after the band filter so the composite over warm
  shows the ramp (asserted: the f0 capture's left / right means differ by > 20). Self-check printed every run: a1080 f0
  vs f150 max diff >> 1 (the comparator can RED).
probe-vupload.{sh,py,json} (NEW; a clone of probe-video.sh that launches `--args --test-mode`, health on 8080, rows on
7070 + 8080; same refuse / lock / quiet / foreign-traffic / quit / outwins rig; fixtures via $VIDEO_FIXTURES):
- `u2_gl_thread_qos`: 8080 state `gl_thread_qos` == 33. (RED on main: absent.)
- `u4a_context_cycle`: 1080p canvas, 4 x a1080, trigger_column, 2 s, s0; 3 x [`POST /api/debug/gl_context_cycle`; poll
  `gl_context_gen` +1 (<= 5 s: the new context recompiles every shader)]; 1 s; s1; settle_late; cap. PASS (a)
  gl_context_gen delta 3; (b) `video_hold_no_texture` delta 0; (c) `video_pending_frames` delta 0 and `videos_pending`
  0 at s1; (d) the top layer's code within its bracket. RED on the commit-1 app: (b) >= 4 per cycle (every player 1-4).
- `u4b_idle_ring_trim`: 4K canvas, deck 0 = 4 x a4k, deck 1 = warm.png; trigger_column; 3 s; s0 (`phys_footprint_mb`,
  `ps -o rss= -p <pid>` via `ps -eo pid=,ucomm=`, `video_slots_purged`); switch_deck 1; 2.5 s; s1; switch_deck 0;
  settle_late; cap; s2. PASS (a) `video_slots_purged` delta == 8; (b) footprint OR rss dropped by >= trimDropMinMb 150
  (expected 4 x 2 x 33.2 = 265; both printed); (c) after the return `video_pending_frames` +0, `video_hold_no_texture`
  +0 and the code within the bracket (the un-purged slots carry a correct picture); (d) `video_slots_purged` does not
  grow while the deck is on screen (5 s). RED on the commit-1 app: (a) absent / 0, (b) < 30 MB (ASSUMED; print).
- `probe-vupload-ab.sh A_APP B_APP ROUNDS` (NEW runner; the interleaving rule): for r in 1..ROUNDS: for app in A, B:
  `acquire_quiet_lock` (wf/lock.sh copied to the scratchpad, SPL fixed), `VIDEO_APP=$app probe-video.sh <out>
  w2c_steady_4kx4_blit`, release; collects every "median fps" into `ab-w2c.tsv` (round, arm, launch median, loads).
  With `--identity REF`: A writes w10 references, B compares. ROUNDS >= 5 (>= 5 runs per arm), never sequential batches.
- Existing rows: the full probe-video.sh twice on the final app, plus probe-image-load / probe-crossfade / probe-deck-clock
  / probe-seq-vram / probe-media-open / probe-idle-paint (P2: message-thread CPU) / probe-outputs (SharedFrameSet shares
  the IOSurface + rect-texture code path) once each: GREEN, thresholds untouched.

### 4.8 Ctests (tests/CMakeLists.txt; `apply_sanitizers` on each; TSan build dir `-DADNA_SANITIZE=thread`)
- `test_video_upload_budget` (NEW, pure): (1) 4 requesters every 4th frame from cold: per-frame uploads <= 2, no
  requester deferred > 1 frame, cap settles at 2, total uploads = total requests; (2) 8 requesters, K = 4: cap reaches
  3 within kHistory frames, no deferral > 2, none skipped (every request admitted within maxDefer); (3) K = 2 (60 fps
  content): maxDefer 1; (4) K <= 1: maxDefer 0, admit always; (5) a burst of 12 on one frame at cap 2: admitted within
  maxDefer + 1 frames; (6) fuzz: random N <= 16, K in {2, 4, 5}, 2000 frames: no requester deferred > maxDefer
  consecutive frames, and `used <= cap + (requesters at the bound)` every frame; (7) `maxDefer` under dt jitter (a 25 ms
  frame) shrinks, never grows past 2. Teeth: drop the force-admit -> (2)(6) fail; cap floor 1 -> (1) fails.
- `test_video_ring` (existing 15 + peek / dropReady / Retire cases of 4.2; the two-thread stress case re-run under TSan).
- `test_video_player_gl` (NEW, Apple only, the test_shared_frame_gl shape: one private CGL 4.1 core context, no window;
  SKIPs loudly without a pixel format; links VideoPlayer.cpp + FFmpeg + juce_core + juce_opengl + IOSurface + OpenGL +
  CoreFoundation; `juce::gl::loadFunctions()` after making current; each case its own player; NO decode thread (start()
  is never called: deterministic)): (1) open(64x64 h264) -> uploadToTexture(&p) returns a texture, p false, the held slot
  is Reading and freeCount() == 2; glGetTexImage equals a test-side sws RGBA conversion of frame 0 (the identity of the
  surface path at K2 <= 1/255) -- also for `video_rawrgba_63x37.mov` (NEW fixture: rawvideo rgba, 3 frames, an odd size
  with a padded bytesPerRow) and `video_hapa_64x64.mov` (NEW: HAP Alpha; alpha bytes compared too); (2) releaseGL() ->
  uploadToTexture returns a NON-ZERO texture on the first call with p false and the same pixels -- RED on main's
  VideoPlayer (returns 0: build the test once against a copy of main's VideoPlayer.{h,cpp} by -I shadowing, record the
  FAILED line, restore the sha); (3) after (1), `glFinish()`; the next uploadToTexture polls the fence: the held slot
  stays Reading, no slot released (only one frame shown) -- then publish a second frame by calling the private
  conversion through a test hook? NO private hooks: instead open a second player on the 3-frame raw fixture and drive
  `advanceFrame` + start()? -- rejected (timing). Case (3) is therefore: two uploads from the SAME held slot after two
  releaseGL()s: the fence of the first blit signals after glFinish and pollFences deletes it without releasing the
  held slot (freeCount stays 2) -- the "never release the held slot" half of the protocol on real GL; the writer-side
  half is the pure Retire test; (4) a budget with cap 0 and maxDefer 0 still admits the first frame (exempt) and the
  post-release re-upload (exempt); (5) trimIfIdle at nowMs + 2000 then purgeFreeSlots() called directly on the test
  thread (no decode thread: the test IS the writer) -> `slotsPurged == 2`, the held slot untouched; unpurge(1) then
  IOSurfaceLock + write a known pattern + Unlock + publish + uploadToTexture -> the pattern reads back (the binding
  survives Empty -> NonVolatile: R-14's INFERRED claim, VERIFIED here or the plan's fallback re-binds).
- `test_video_player_open`: add `"-framework IOSurface" "-framework OpenGL"` on Apple (VideoPlayer.cpp now calls both);
  its two cases unchanged.
- TSan: `cmake -S . -B build-tsan -DADNA_SANITIZE=thread -DCMAKE_BUILD_TYPE=Debug` (cmake/Sanitizers.cmake), build only
  test_video_ring test_video_upload_budget test_video_player_open, run 3 x; the GL test is run under TSan and REPORTED
  (driver frames may report; not a gate).

### 4.9 The 4K bar (fps4kBlitMin) -- derived, not typed
On the commit-5 app vs the pre-lane app (a copy of main's build made before commit 1; sha recorded), `probe-vupload-ab.sh
A B 5` under quiet locks. Let mA = the A launch medians (5), mB = the B launch medians (5). Set fps4kBlitMin =
floor(min(mB)) - 3 only if that is >= max(mA) + 8; else leave null (INFO) and report -- never a knife-edge, never a bar
from numbers taken while other lanes ran. Prediction (diag q2 / q2b, INFERRED for this build): mB 104-107, mA 89-91 ->
fps4kBlitMin ~ 100-101, RED on main by >= 9.

## 5. GATES -- RED on current main predicted with the number

| gate | main (pre-lane) | after this lane | evidence |
|---|---|---|---|
| w1c (a) median per-poll peak_callback_ms <= 4.0 every load | 6.30-6.47 ms, 15/15 loads (RED) | ~3.5 (P1, cap 2) / ~2.1 (P1+P3) | agg/main-reference main2_col; q1b_cap2 render p90 3.48; q2 IOSurface per-upload 0.08-0.10 |
| w1c (b) median fps over 6 loads >= 118.5 | 112.7 / 115.8 (RED) | 119.8-119.9 (min window 116.9) | q1b_cap2 |
| w1c (c) uploads >= 595, late 0 | PASS (604-610, 0) | 604, 0 | the anti-cap-1 guard (cap 1: 586) |
| w2c (a) >= fps4kBlitMin (~100) | 89.3-91 (RED) | 105-107 | q2 / q2b iosurf; 4.9 |
| w10 identity max diff <= 1 (5 formats x 2 frames) | reference | 0 predicted (INFERRED; the audit's MUST closed) | -- |
| u2 gl_thread_qos == 33 | absent (RED) | 33 | q1b_qos4 "GL UI" |
| u4a video_hold_no_texture delta 0 over 3 cycles | commit-1 app: >= 12 (4 players x >= 1 frame x 3) (RED) | 0 | section 1 P4a bug |
| u4b slots_purged == 8, drop >= 150 MB, return correct | commit-1 app: 0 / flat (RED) | 8 / ~265 MB | R-14 |
| test_video_player_gl (2) non-zero after releaseGL | main's VideoPlayer: 0 (RED, FAILED line recorded) | pass | 4.8 |
| test_video_ring Retire / peek, test_video_upload_budget | do not compile on main (RED) | pass | 4.2 / 4.8 |
| existing probe-video x2, image-load, crossfade, deck-clock, seq-vram, media-open, idle-paint, outputs | GREEN | GREEN, thresholds untouched | R-16 |
| ctest full | 920 / 920 | 920 + new / all | -- |

## 6. MUST NOT CHANGE (and how each is kept)
- Pitfall 56's contract: never 0 / no media for a shown player (now stronger: the held slot), PENDING only for never-shown,
  FAILED (W3) unchanged, the render thread never waits (fences polled with timeout 0; `peek` / `pick` never block), no
  FFmpeg on the GL thread. Kept by construction; w3 / w3b / w4 / w4b / w6 / w6b / w9 re-run.
- C1 (`neverShown()`) and C3 (`notePendingImage`): a deferred or fenced hold is not pending (R-4); render_frame captures
  a hold as before (settle_late).
- The effect chain and every shader: `texture_` stays a GL_TEXTURE_2D RGBA8 sampler; the rectangle texture is only a
  blit source (never bound as a sampler outside VideoPlayer).
- The decode policy (decide / shouldPublish / NONREF / idle park / reseekBehindSec), `advanceTransport`, seek requests,
  the Loop-wrap generation bump, frame 0 in open(), the thumbnail, `installVideoPlayer` (asyncload's seam),
  `videoPlayerMutex_` = lookup only (+ the frame-top scan, gated by w7 (a) <= 2 ms), decode threads at `normal`.
- The images' `UploadBudget` and `pumpImages`; ImageSequence / SeqVram; SharedFrameSet / the outputs; the Output windows.
- Non-Apple builds: the malloc + glTexSubImage2D path compiles and behaves as today (P1 / P4a / P4b apply to it too).
- Every existing probe threshold (w1 (a) 110 in 4 of 5 stays INFO-class as ruled; w2 (a) 80, (a2) INFO).
- Rig: no Output window, no synthetic input, `open -g` only, one app under the lock, counters via /api/state, no
  debugger / sampler (`ps` and `proc_pid_rusage` only), >= 5 runs per arm, quiet numbers only.

## 7. FENCES (this lane vs the lanes in flight: asyncload, g4cpu)
Edits: `src/media/VideoPlayer.{h,cpp}`, `src/media/VideoRing.h`, `src/media/VideoStats.h`, `src/media/VideoUploadBudget.h`
(NEW), `src/render/Renderer.{h,cpp}` (frame top, syncMedia :1823, newOpenGLContextCreated, scanVideoIdle; nothing else),
`src/api/ApiServer.cpp` (video block only), `src/test/TestServer.{h,cpp}` (video block, 3 fields, 1 route),
`tests/CMakeLists.txt` (2 new targets + frameworks on test_video_player_open), `tests/test_video_ring.cpp`,
`tests/test_video_upload_budget.cpp` (NEW), `tests/test_video_player_gl.cpp` (NEW), `tests/fixtures/video_rawrgba_63x37.mov`
+ `video_hapa_64x64.mov` (NEW, < 100 KB each), `.harmony/probe-video.{sh,py,json}`, `.harmony/probe-vupload.{sh,py,json}` +
`probe-vupload-ab.sh` (NEW), `docs/claude/{rendering,pitfalls,testing-eyes}.md`, `CLAUDE.md`.
Never: `ImageSequence.*`, `SeqVram.h`, `CompositorEngine.*`, `MainComponent.cpp`, `UndoService.*`, `CompositionLoad.h`,
`MediaOpener.*` (asyncload), any `src/ui/*` (g4cpu), `EmbeddedShaders.h`. asyncload's R16 fence excludes VideoPlayer.* /
VideoRing.h / Renderer.* (plan-asyncload.md:276) -- no source overlap; the SHARED files are the state blocks
(ApiServer.cpp / TestServer.cpp), `tests/CMakeLists.txt`'s tail, pitfalls.md (two "NN" entries), CLAUDE.md, rendering.md:
whichever merges second rebases with the video-rebase rule (both blocks verbatim, never a hand-merged interleave;
Harmony numbers the pitfalls). asyncload's pool-thread `open()` runs IOSurfaceCreate + IOSurfaceLock + sws on a pool
thread: no GL, no Renderer state -- compatible.

## 8. RISKS (strongest counterargument first; what to verify)
1. "P3 makes P1 moot (the blit costs 0.1 ms) -- drop P1." The GPU side of 4 bunched 4K blits still lands on one frame
   (the B share, 6-9 fps, was measured by difference; its split between the bunch and the copy is unknown), the malloc
   fallback and non-Apple builds keep the CPU cost, and 8-16 players exceed any single-frame slack. P1 is pure, 60 lines,
   ctested, and measured (+2.0 / +3.4 median, min 107.7 -> 116.9). Verify: w1c on commit 2 alone (P1) reads GREEN.
2. Per-pixel identity at 1/255 for 10-bit sources (pr1080, x265 10-bit): sws's yuv -> BGRA vs -> RGBA use the same
   coefficients and dither, so 0 is expected, but this is INFERRED. If w10 reads 2-3/255 on a 10-bit source only: do NOT
   loosen; try an 'RGBA' IOSurface (kCVPixelFormatType_32RGBA) bound with GL_RGBA / GL_UNSIGNED_INT_8_8_8_8_REV so sws
   keeps its RGBA path (INFERRED to bind; the prototype never tried it), else report for a ruling.
3. The held slot leaves the writer 2 slots: could raise `video_late_frames` at 4K (w2 (e) <= 12) or the catch-up holds
   (w3 (c) 60, w3b (c) 200). Steady state needs <= 1 frame of look-ahead at 30 fps / 120 Hz and the catch-up is
   decode-bound; INFERRED no change. Verify on commit 4 (before P3): w2, w3, w3b, w4 unchanged; fallback kSlots = 4.
4. `IOSurfaceSetPurgeable(Empty)` on a surface bound to a rect texture, then NonVolatile + a CPU write: INFERRED the
   binding survives (the surface object is unchanged). Verified by test_video_player_gl (5) and u4b (c); fallback: the
   writer sets a per-slot `rebind_` flag and uploadSlot re-runs CGLTexImageIOSurface2D for that slot.
5. Purged IOSurface pages may not leave `ps rss` (shared / wired accounting). `phys_footprint` is the primary witness;
   both printed; if neither moves by >= 150 MB with slots_purged == 8, report (do not lower the bar) -- the RSS goal
   (R-14) would then need a different mechanism (surface release + re-create), a ruling.
6. w1c (a)'s 4.0 ms bar has ~0.5 ms margin on P1 alone (cap 2 = 3.48 p90 instrumented); with P3 the margin is ~1.9 ms.
   The merge verdict is taken on the final app; commit 2's own run is evidence, not the bar. If P1-alone reads > 4.0 on
   a quiet run, report the number (no re-threshold).
7. QoS USER_INTERACTIVE on the render thread could contend with the message thread (both UI): the render thread spends
   its time blocked in flushBuffer (0.24 ms CPU per 6 ms wait); the QOS=4 arm showed no message-thread symptom (not
   measured there) -- probe-idle-paint's message-thread CPU rows gate it. The thread is shared with the Output windows'
   contexts: they benefit; nothing else runs on it.
8. `peek` and `pick` diverging (a different frame admitted than uploaded): the ctest asserts equality on every ring
   shape the existing cases build; both use one selection rule (factor it into a private `select()` if the builder
   prefers -- the 15 existing cases must stay green unchanged).
9. The context-cycle endpoint: `detach()` on the message thread waits for the render thread's current frame (<= 1 frame)
   and the new context recompiles 135+ programs (~100s of ms): the probe waits on `gl_context_gen`. It exists only in
   the TestServer; it never opens a window; a real context loss (preview hide / minimise / Output window layout) runs the
   same JUCE path (pause -> openGLContextClosing -> initialiseOnThread -> newOpenGLContextCreated).
10. The 4K A/B on a shared rig: 5 x 2 windows per arm, interleaved, quiet -- the diag's two sets agreed within 1.6 fps
    per arm; the "bar >= main + 8" rule refuses a knife-edge. If the rig is never quiet, the row stays INFO.
11. Odd sizes: bytesPerRow padding is handled by `rowBytes_`; the client-upload fallback needs GL_UNPACK_ROW_LENGTH
    (specified). Verified by the 63x37 fixture in test_video_player_gl (1).
12. The asyncload merge: the shared blocks conflict textually (not semantically); resolution rule in section 7.

## 9. COMMIT SEQUENCE (each builds, passes ctest, keeps every existing probe GREEN; each reverts alone)
1. `test(s-rta-0929 vupload): u0 -- counters, TEST-ONLY context cycle, rows w1c / w2c / w10 / u2 / u4a / u4b (RED harness)`
   -- VideoStats + both state blocks (4.3), TestServer fields + route (4.6), Renderer getters (`glContextGen_`,
   `glThreadQos_` = -1 for now), probe rows + json keys (w1cPeakMedianMaxMs 4.0, w1cFpsMin 118.5, w1cUploadsMin 595,
   fps4kBlitMin null, trimDropMinMb 150) + new fixtures + the A/B runner. Records RED on this commit's app: w1c (a)(b),
   u4a (b), u4b (a)(b); w2c prints INFO. No behaviour change.
2. `feat(s-rta-0929 vupload): u1 -- per-frame video upload budget (P1)` -- VideoUploadBudget.h, Ring::peek, the
   deferral in uploadToTexture, the Renderer wiring, test_video_upload_budget + the peek cases. Gate: w1c GREEN (a)(b)(c),
   w1 / w5 / w6 / w6b / w8 unchanged, ctest.
3. `perf(s-rta-0929 vupload): u2 -- GL render thread at QoS USER_INTERACTIVE (P2)` -- newOpenGLContextCreated. Gate: u2
   GREEN, w1c still GREEN, probe-idle-paint GREEN (message-thread rows), probe-outputs GREEN.
4. `feat(s-rta-0929 vupload): u3 -- the shown slot stays the reader's; context loss re-uploads it (P4a, both paths)` --
   VideoRing::Retire + dropReady, retire_ in VideoPlayer (malloc path: prev released at the next pick), releaseGL keeps
   the held slot, `holdNoTexture`, test_video_ring Retire cases, test_video_player_gl (1: malloc variant by forcing
   `useSurfaces_ = false` -- a constructor-less static lever `VideoPlayer::kForceClientUpload` for tests? NO: the
   surfaces do not exist until commit 5, so (1)(2) run on the malloc path here and again on the surface path in 5)
   + (2) with the RED recorded against main's VideoPlayer, u4a GREEN, w2 (e) / w3 / w3b / w4 unchanged (risk 3).
5. `perf(s-rta-0929 vupload): u4 -- IOSurface ring slots + one blit per new frame, fenced (P3)` -- createSurfaces, BGRA
   sws, uploadSlot blit, pollFences, releaseGL fences, the test frameworks, test_video_player_gl (1)(3)(4) on the surface
   path, TSan. Gates: w10 identity vs the PRE-LANE app (A = the saved main build), the interleaved A/B -> fps4kBlitMin
   (4.9) -> w2c GREEN, w5 / w8 (ProRes) / w6 / w9 (HAP) / w3b GREEN, full probe-video x2, probe-outputs.
6. `feat(s-rta-0929 vupload): u5 -- idle players purge their free ring slots (R-14, P4b)` -- trimIfIdle / scanVideoIdle,
   purgeFreeSlots / unpurge, `slotsPurged`, test_video_player_gl (5), u4b GREEN, w4 / w4b / w7 (a) unchanged.
7. `docs(s-rta-0929 vupload): rendering.md, pitfall NN (vupload), CLAUDE.md index (paid), testing-eyes.md, probe docstrings`
   (section 10), APP-INVENTORY counts (ctest targets +2).
Each commit: `cmake --build build-lane`, `ctest --test-dir build-lane -j1`, the rows named above under the lock; the
per-commit app copies kept in the scratchpad (`apps/c1..c7`) for the RED / GREEN table in the lane report.

## 10. DOCS (text; "NN" until Harmony assigns; CLAUDE.md paid by the exact trims below, final `wc -c` <= 25,000)

docs/claude/rendering.md -- append to the "Video playback" paragraph (:75), after "Live: `.harmony/probe-video.sh`.":
"**Video uploads (s-rta-0929 vupload)**: uploads are budgeted per render frame (`src/media/VideoUploadBudget.h`,
count-based, demand-adaptive: cap = max(2, ceil(mean requests per frame over 16 frames) + 1); a drawn player with a new
frame asks BEFORE its pick (`VideoRing::peek`) and, over the cap, HOLDS its shown frame for at most min(2, K - 1) render
frames (K = render frames per content frame) -- never pending, never late, never a skipped content frame; never-shown
players and the first upload after a GL release are exempt; both chains of a crossfade count) -- four 30 fps clips whose
clocks start on one render frame (a column trigger) used to land all four 1.4 ms uploads on one frame and lose 2-8
display-link ticks a second (104-117 fps; diag-vfps). On macOS the ring slots are IOSurfaces (BGRA; `sws_scale` writes
the locked surface bottom-up as before), each bound once to a GL_TEXTURE_RECTANGLE read FBO; a new frame is ONE
`glBlitFramebuffer` into the clip's GL_RGBA8 `texture_` (the effect chain sees the same sampler2D; 0.9 -> 0.1 ms of GL
CPU per 4K frame, 4 x 4K 90 -> 105 fps), fenced; a slot returns to the decode thread only when its fence has signaled
(`glClientWaitSync` timeout 0, polled at the next draw), the frame ON SCREEN keeps its slot until a newer one is shown
(`VideoRing::Retire`; the writer runs 2 frames ahead), and after a GL context loss the first draw re-uploads that held
slot -- a playing clip never returns 0 (it used to run FX-only for 1-4 frames; `video_hold_no_texture` must stay 0).
Fences die with the context (`releaseGL` drops them and releases the fenced slots; the held slot survives). The
non-Apple build and a player whose IOSurface allocation fails keep the malloc + `glTexSubImage2D` path. A player not
drawn for 1 s drops its Ready slots and its parked decode thread purges the Free ones (`IOSurfaceSetPurgeable`;
`video_slots_purged`); the writer un-purges a slot when it next takes it; the held slot is never purged. The GL render
thread (JUCE's `std::thread`, QoS DEFAULT) is raised to USER_INTERACTIVE in `newOpenGLContextCreated` (wake-up p99
0.036 -> 0.018 ms, lost ticks halved; it does not keep a light GL thread off the E-cores). `/api/state`:
`video_uploads_deferred`, `video_hold_no_texture`, `video_slots_purged`, `video_upload_cap`; test mode: `gl_context_gen`,
`gl_thread_qos`, `phys_footprint_mb`, `POST /api/debug/gl_context_cycle`. Levers: `VideoUpload::kFloor` (2),
`kMaxDeferFrames` (2), `kHistory` (16), `VideoPlayer::kTrimIdleMs` (1000). Guards: `tests/test_video_upload_budget.cpp`,
`tests/test_video_ring.cpp` (peek, Retire), `tests/test_video_player_gl.cpp` (a private CGL context: identity of the
surface path at 1/255 incl. an odd size and HAP Alpha, the post-release re-upload, the purge round trip); live:
`.harmony/probe-video.sh` rows w1c / w2c / w10, `.harmony/probe-vupload.sh` (u2 / u4a / u4b), the A/B runner
`.harmony/probe-vupload-ab.sh`. VideoToolbox is filed (software 14.2 vs VT 0.87 ms CPU per 4K frame; 0 fps at this
load; HAP / ProRes 4444 / 10-bit stay software; NV12 needs a YUV shader); the GOP cache for reverse / ping-pong is its
own lane (R-12)."

docs/claude/pitfalls.md -- new entry after :125 (the asyncload "NN."); marker "NN (vupload)":
"NN. **A video frame's ring slot is the reader's while it is on screen, its upload is budgeted, and an IOSurface slot
returns only after its blit fence -- never release the shown slot at the upload, never upload past the frame's budget,
never read a slot the GPU may still be copying**: `uploadToTexture` used to release a slot right after
`glTexSubImage2D`, so after a GL context loss a playing clip had no picture to hold and returned texture 0 with
`pending = false` for 1-4 frames -- the FX-only trap of Pitfall 53 on a video (`video_hold_no_texture`); and four
bunched 1.4 ms uploads on one render frame lost display-link ticks (JUCE's `pendingRender` is a flag, not a counter:
a frame that spans two vblanks drops one). Rules: (1) every new frame asks `VideoUpload::Budget::admit` BEFORE its
pick (`VideoRing::peek`); refused = HOLD the shown frame (`*pending` false, no slot changes state), for at most
`maxDefer` render frames -- the bound, not a rotor, is the no-starvation guarantee; never-shown and post-release uploads
are exempt; (2) the shown slot stays Reading until a newer frame is shown (`VideoRing::Retire::held`) and is the
picture re-uploaded on the first draw of a new context; (3) a slot copied by the GPU (IOSurface blit) is released only
when its `glFenceSync` signals (`glClientWaitSync` timeout 0 at the next draw) or the context dies (fences die with
it; the held slot survives); (4) the GL_TEXTURE_RECTANGLE bound to an IOSurface is a blit SOURCE only -- an IOSurface
cannot bind as GL_TEXTURE_2D on this Mac (CGLError 10008) and the effect chain samples `texture_`; (5) purge only Free
slots and only from the writer (the decode thread), un-purge before writing; the reader drops Ready slots and asks.
Guards: `tests/test_video_upload_budget.cpp`, `tests/test_video_ring.cpp`, `tests/test_video_player_gl.cpp`; live:
`.harmony/probe-video.sh` w1c / w2c / w10, `.harmony/probe-vupload.sh` u2 / u4a / u4b."

CLAUDE.md -- index line (after 57): "NN. Video uploads are budgeted, fenced IOSurface blits; the shown slot stays the
reader's -- before touching `uploadToTexture`, `releaseGL` or a ring release." (~150 B). Paid by (exact, in order):
rule 5 (:97) NOTE -> "**NOTE**: the shipped effects are inline strings in `src/render/EmbeddedShaders.h` (the `shaders/`
dir went in Wave 0; hot-reload is inert -- `docs/claude/effects.md`)." (-82 B); index 47 (:226) tail "-- before writing
or debugging a test that loads a source through 8080." -> "-- before a test loads a source through 8080." (-31 B);
index 54 (:233) tail "or a media class's per-frame GL objects." -> "or per-frame GL objects." (-17 B); index 28 (:207)
"-- before relying on Eyes captures to verify clip/layer/global or frame-history effects." -> "-- before verifying
clip/layer/global or frame-history effects from Eyes captures." (-12 B). Budget: 20 spare + 142 = 162 >= the new line;
the builder's final check is `wc -c CLAUDE.md` <= 25000 and prints the number in the report.

docs/claude/testing-eyes.md -- after the output_probe line (:13): "**GL context cycle, test mode only (s-rta-0929
vupload)**: `POST /api/debug/gl_context_cycle` detaches and re-attaches the preview panel's GL context on the message
thread (the same JUCE path as a preview hide / app minimise / Output layout change: `openGLContextClosing` then
`newOpenGLContextCreated`; every shader recompiles) -- poll `gl_context_gen` (8080 `/api/state`) until it advances; no
window is opened. Used by `.harmony/probe-vupload.sh` u4a." And in the render_frame row (:68) after "a video HOLD (...)
is not pending: `render_frame` may capture it": ", as is a frame held by the per-frame video upload budget (<= 2 render
frames)".

probe docstrings: probe-video.py (rows w1c / w2c / w10 in the docstring's row list and order), probe-vupload.py (u2 /
u4a / u4b, the test-mode launch), probe-video.json `_` notes for each new key. .harmony/APP-INVENTORY.md: ctest targets
+2 (Harmony runs the counts).

## 11. OPEN QUESTIONS FOR HARMONY (rulings that change the build)
1. R-15: the four `video_*` counters in BOTH state blocks (the video lane's precedent, needed by production-mode rows)
   vs the rig note "TEST_SERVER-only REST endpoints for counters" -- adopt R-15, or move w1c / w2c / w10 to test mode?
2. R-3: bounded deferral in visit order instead of the requested "rotating priority" -- the guarantee is the same
   (no player waits > min(2, K-1) frames); accept, or require an explicit rotor (adds a per-frame start offset)?
3. R-7: kSlots stays 3 with the held slot (writer look-ahead 2) -- accept, or make it 4 (+33 MB per 4K player)?
4. w1c (a) = 4.0 ms (the diag's number; ~0.5 ms margin on P1 alone, ~1.9 with P3): keep, or 4.5 with the same RED?
5. R-12: GOP cache = its own lane (ruled here); confirm and file the lane text of R-12.
6. R-14: kTrimIdleMs 1000 and the held slot exempt from the purge (33 MB residual per idle 4K player) -- accept?
7. Risk 2: if 10-bit identity reads 2-3/255, the 'RGBA' IOSurface attempt is authorised, and failing that, a ruling.

## 12. COMPACT
- Cause (VERIFIED, diag): bunched uploads (4 x 1.4 ms on one frame) + JUCE's flag-paced display link drop ticks; 4K
  uploads cost 0.9 ms GL CPU each; the shown slot is released at the upload (P4a bug); the GL thread is QoS DEFAULT.
- Design: P1 pure count budget (cap = max(2, ceil(mean)+1), bounded deferral min(2, K-1), peek-before-pick, hold never
  pending); P2 QoS UI in newOpenGLContextCreated; P3 BGRA IOSurface slots + rect FBO + one blit into the existing RGBA8
  texture_, fenced, slots released on signal, 3+3+1 GL objects per player, malloc path kept for non-Apple / failure;
  P4a held shown slot (VideoRing::Retire) + first-draw re-upload after a context loss; P4b reader drops Ready, writer
  purges Free (IOSurfaceSetPurgeable Empty), un-purge on acquire, held never purged; P4c GOP cache = own lane; VT out.
- Gates: w1c (main 6.30-6.47 ms / 112.7-115.8 fps -> <= 4.0 / >= 118.5, uploads >= 595, late 0); w2c bar from a 5 x 2
  interleaved A/B (predicted ~100; main 89-91); w10 per-pixel <= 1/255 on 5 formats x 2 frames vs the pre-lane app;
  u2 QoS 33; u4a hold_no_texture 0 (commit-1 app >= 12); u4b purged 8, footprint -265 MB; ctests: budget (pure), ring
  peek / Retire (pure), player GL (private CGL: identity incl. 63x37 + HAP Alpha, post-release re-upload RED on main,
  purge round trip); TSan on the pure + open tests; every existing row untouched.
- 7 commits, each revertible; docs text in section 10; CLAUDE.md paid by four exact trims (-142 B).
- Fence: VideoPlayer.* / VideoRing.h / VideoStats.h / VideoUploadBudget.h / Renderer.* (3 sites) / the two state blocks /
  a TestServer route / tests / probes / docs; nothing asyncload or g4cpu edits.

STATUS: DONE

## HARMONY ADOPTION (s-rta-0929, 14:28) — OVERRIDES THE BODY WHERE THEY DIFFER
Plan authored by Fable (wf_885858f4-911). Attacked blind by attack-vupload-gl.md, -vj.md, -gates.md. The body is ADOPTED
except where a ruling below overrides it. Branch from main 3e15613 (asyncload merged: MainComponent / ApiServer / state
blocks moved; Pitfall 58 taken — your pitfall is "NN").
- VU1 (gl MUST 1) On un-purge, re-run CGLTexImageIOSurface2D for that slot's rect texture as the DEFAULT path (not a
  fallback); ctest purge -> un-purge -> write -> blit -> pixels == the new frame.
- VU2 (gl MUST 2) pollFences handles GL_WAIT_FAILED like SharedFrameSet.cpp:59 (the slot is released and a counter
  video_fence_failed ticks; never stuck Reading); ctest with an injected failed-fence result.
- VU3 (gl MUST 3 + SHOULD 4 + 5) releaseGL zeroes every handle (rectTex_ / readFbo_ / dstFbo_ / fence_) exactly as
  SharedFrameSet::releaseGL :79-86; ctest releaseGL -> next upload re-creates and draws the right pixels. Each per-slot read
  FBO is checked with glCheckFramebufferStatus once at creation; incomplete -> that player uses the malloc path (counted
  video_surface_fallbacks). Correct R-9's text (the blit source is the bound READ FBO, not a texture unit).
- VU4 (gl NIT + vj SHOULD 6) w10 gains a malloc-path arm (a TEST-ONLY env var forces the fallback) diffed against the
  pre-lane app — the moved fallback is proven too.
- VU5 (vj MUST 1 + SHOULD 4) video_hold_no_texture delta == 0 and video_late_frames delta 0 are asserted in EVERY NEW live
  row (w1c, w2c, w10, u-rows), plus a NEW row u6_crossfade_video: two video clips crossfading on one layer (Dissolve) under
  a column trigger of 4 layers -> hold_no_texture 0, both chains' late 0, and a decoded mid-fade frame whose band is a
  blend (not one side). Existing rows untouched.
- VU6 (vj MUST 2, PARTIAL) The TEST-ONLY detach()/attach cycle IS JUCE's context-loss path (openGLContextClosing ->
  newOpenGLContextCreated) and is the gate; a GPU-driven loss cannot be produced on this rig (no Output window, no
  synthetic input) — labelled INFERRED in the report and listed for Boris (minimise / restore the main window, or unplug
  the projector, while a video plays: no flash of effects-only frames).
- VU7 (vj MUST 3 + Q3) NEW row u7_reverse_pingpong: reverse and ping-pong on a 1080p short-GOP and a long-GOP (-g 250) file,
  INTERLEAVED arms vs the pre-lane app (>= 5 launches per arm): uploads/s of the clip and late frames must be no worse
  than main (bar derived from main's arm: >= 0.9 x main's median uploads/s). kSlots stays 3; if u7 or w1c / w2c late
  frames regress, move to kSlots 4 (+33 MB per 4K player), re-run, report both.
- VU8 (vj SHOULD 5) A player's first frame after a seek / retrigger is EXEMPT from the budget (like never-shown): the
  retrigger-to-visible latency does not change. ctest.
- VU9 (gates MUST 1) State the real bound: the ctest fuzz runs N = 4, 8, 16 synchronized players and asserts per-frame
  uploads <= the stated bound (derive it: cap + requesters at the defer bound, or tighter) and no player deferred more
  than maxDefer; the plan's "never skipped / no starvation" language is qualified with that bound. NEW live INFO row
  w1d_column_trigger_1080x8 (8 players): fps, max uploads per frame, late 0 printed.
- VU10 (gates MUST 2) A TSan two-thread ctest races publish() between peek() and pick(); request / used / deferred
  accounting stays consistent when pick returns a frame peek did not see.
- VU11 (gates MUST 3) QoS is applied in EVERY context's newOpenGLContextCreated (the Renderer's AND OutputWindow's
  presenter) — cheap and idempotent; u2 re-checks gl_thread_qos AFTER the u4a context cycle. The "preview detached with
  an Output window open" state is not testable (no gate opens an Output window) — noted.
- VU12 (Q1) video_* counters live in BOTH state blocks (the video lane's precedent; production-mode rows read them); new
  ENDPOINTS (gl_context_cycle, the fallback env hook) are TEST_SERVER-only. (Q2) bounded deferral in visit order ADOPTED
  iff the VU9 fuzz proves no starvation at N = 4 / 8 / 16. (Q4) w1c (a) bar 4.0 ms on the FINAL app (P1-only intermediate
  = INFO). (Q5) GOP cache = its own lane, CONFIRMED (filed). (Q6) R-14 idle 1000 ms, held slot never purged: ADOPTED.
  (Q7) 10-bit identity > 1/255 -> try the 'RGBA' IOSurface; still > 1/255 -> STOP that format on the malloc path and
  report (never a loosened bar).
- VU13 Existing probes re-run on the final app, never re-thresholded: probe-video (all rows; w6b (a) is a known
  pre-existing load-sensitive flake: main fails it 4/5 — report its 5-run distribution on both apps), probe-crossfade,
  probe-media-open, probe-async-load, probe-seq-vram, probe-image-load, probe-capture, probe-idle-paint; Tier-1 5 files;
  ctest serial; TSan build of the pure + ring tests. Another build lane (g4cpu fix round) shares the live lock.

## HARMONY ADOPTION ADDENDUM 2 (s-rta-0929, 19:24) — rulings on the lane's concerns (lane HEAD 3a9e166)
- VU14 (kWriterLookAhead, 90b2ca9) ADOPTED as an evidenced deviation from section 6: the must-not-change INTENT was "reverse /
  ping-pong behave as on main"; the held slot shrank the writer's look-ahead, so the reseek distance must follow the
  look-ahead to keep main's geometry. u7 (10 vs 10 interleaved, 4/4 PASS) is the proof. Section 6's line now reads
  "reseek geometry measured from the writer's look-ahead (kWriterLookAhead = kSlots - 1)".
- VU15 (w7 (b) heavier tail — DIAGNOSE BEFORE MERGE, pre-registered) Arms, INTERLEAVED round-robin launch by launch, quiet,
  10 launches each: MAIN (the pre-lane app copy), FINAL (3a9e166), NOQOS (FINAL with 11f200d reverted, a scratch build —
  never committed unless the rule below says so). w7 only (same fixture / script as the lane's runs). Metric per arm:
  pooled count of trigger round trips > 20 ms and the per-launch max. Decision (no round trip to Harmony):
  (a) FINAL - NOQOS >= 4 AND NOQOS <= MAIN + 2 -> QoS is the cause: revert 11f200d on the lane (a revert commit), u2 row
      becomes INFO "QoS not applied (VU15)", docs updated (P2 text: tried, reverted, why), re-run w1c 3 loads x 2
      launches on the reverted app (bars unchanged) + w7 x 5.
  (b) |FINAL - NOQOS| < 4 AND FINAL >= MAIN + 4 -> the tail comes from P1 / P3 / P4: keep P2; attribute it with ONE
      TEMPORARY instrumentation arm (per-RTT: was the message thread busy, and in what; was the GL thread mid-upload / mid-
      blit / inside an IOSurfaceLock the message thread waits on) and STOP for a Harmony ruling with the table.
  (c) FINAL <= MAIN + 3 -> not reproduced at 10 launches: record the numbers, file it, merge proceeds.
  Report every launch's values.
- VU16 (gates SHOULD) u7 asserts hold_no_texture delta 0 (VU5), not only prints it; re-run u7 once to show the line.
- VU17 (gates NIT) a wrapper (probe-video-w10-all.sh or a flag) runs w10 on the three paths (blit / client / malloc) in one
  invocation against the same reference.
