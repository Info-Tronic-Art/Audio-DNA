# plan-seqvram -- bound ImageSequence's texture memory (s-rta-0928b START HERE item 2)

Architect plan (Fable). Base: main @ b9c9ab2 (last code commit a3691e1; every file:line below is at that tree, read
2026-09-28). Bins: VERIFIED (read at the cited line, or measured in a cited report) / INFERRED (derived from verified
facts) / ASSUMED (not checked -- the builder checks it and records the result). An opus builder executes this in a
worktree; Harmony adopts or overrides after two blind attackers.

QUESTION: ImageSequence keeps every frame's GL texture for its lifetime (300 frames at 1080p = 2.32 GiB of VRAM,
INFERRED arithmetic from VERIFIED code). Design a bounded per-sequence texture window with eviction that keeps
playback smooth, split so the policy is a ctest and the GL wiring is proven live, RED-first on memory counters.

APPROACH (stated first):
- ONE pure header `src/media/SeqVram.h` (no GL, no JUCE; `tests/test_seq_vram.cpp`): (a) `Window` -- frames ordered by
  TRAJECTORY DISTANCE (how many playback steps until the frame is shown again, following the sequence's own mode:
  Loop wraps, PingPong bounces, OneShot orders like Loop because a retrigger restarts at the in-point, reverse flips
  it): the look-ahead requests the nearest frames, eviction drops the farthest, and the frame on screen (`lastShown`)
  and the current frame are never evicted; (b) `Slots` -- a per-sequence table of equal-size GL textures recycled with
  `glTexSubImage2D` (a frame of another size re-specifies the slot with `glTexImage2D`; never glGen/glDelete per
  frame); (c) `allowance()` -- how many bytes one sequence may hold this frame.
- The budget: `kBudgetBytes` = 1 GiB for ALL image sequences together (ASSUMED, a named constant), SEPARATE from the
  stills' `kImagePrefetchBudgetBytes` (ruling R-5 below). A sequence that fits whole keeps every frame (today's
  behaviour: zero re-decode after its first pass -- the common short loop). A sequence that does not fit plays through
  a window as large as the free budget allows (Belady-optimal for a cyclic pass: the far frames stay resident, one
  slot cycles through the rest) and never below a floor of `kMinWindowFrames` = 8 frames (current + shown + 4 ahead +
  2 arriving). Eviction runs BEFORE an upload, so a sequence never exceeds its allowance.
- Renderer, once per frame at the frame top (like `pumpImages`, B2): sums every sequence's resident bytes under the
  existing `imageSeqMutex_`; when the total is over the budget it trims sequences NOT drawn in the previous frame
  (inactive decks, inactive columns) to {current, lastShown}, least-recently drawn first. It hands each drawn
  sequence a `Grant` {allowance bytes, frame serial, stats}. No new mutex; no CompositorEngine change.
- Pitfall 53 semantics are untouched: a frame not resident shows the last frame shown; `*pending` only when nothing
  was ever shown; the capture gate, C1 pause and `firstFramePending` read the same `textures_[]` they read today.
- Counters `seq_*` in `/api/state` (7070 + 8080) come FIRST (commit 1, no behaviour change) so the memory rows are
  RED BY VALUE on an instrumented main (300 textures / 2373 MiB), not only by absence; then the policy + tests; then
  the wiring; live probe `.harmony/probe-seq-vram.{sh,py,json}` modelled on probe-image-load.

## 1. TODAY'S CODE, RE-DERIVED (file:line at b9c9ab2)

F1 (VERIFIED) Frame index per mode -- `ImageSequence::advanceFrame` (`src/media/ImageSequence.cpp:125-194`):
   - time-based: `currentTime_ += dt * speed * direction` (:147); `direction` = +1, flipped by PingPong's
     `pingPongForward_` (:142-143) and by `reverse` (:144-145);
   - Loop: `fmod` at both ends (:155, :172); PingPong: reflect + flip (:157-159, :175-176); OneShot: clamp to the end
     (or 0 in reverse) and `playing_ = false` (:161-163, :178-180);
   - frame = `int(currentTime_ * fps)` clamped to [0, n-1] (:186-189); playhead published (:192-193).
   - There is no random or scrub mode on a sequence. Seeks: `seekTo` (:115-123) from the cuepoint jump
     (`src/MainComponent.cpp:1500-1504`), beat snap on trigger (:4298-4302) and the same-column retrigger
     (:4324-4328, seeks to `inPoint`; the retrigger branch `Layer.h:263-273` starts NO crossfade). `seekTo` is called
     on the MESSAGE thread (all three callers) and writes `currentTime_` / `currentFrameIndex_` that the GL thread
     reads -- a pre-existing unsynchronised write (FOUND, not this lane's; see section 7 R6).
   - Transport sync from the clip each frame: `Renderer::syncMedia` (`src/render/Renderer.cpp:1644-1728`): speed with
     master speed (:1658-1659), reverse except PingPong (:1660-1661), playing (:1662-1665), fps (:1666), loop mode
     (:1667-1672), BPM sync sets fps = frames / (beatDivision * 60 / bpm) (:1674-1691), in/out points (:1700-1712:
     OneShot stops, else `seekTo(inPoint)`).
   - `decode == false` (a deck that is not on screen, `DeckClock::tick`, `Renderer.cpp:713` ->
     `tickMediaClock` -> `syncMedia(..., false)`) advances the clock and returns 0 BEFORE `getCurrentTexture`
     (:1717-1718): rule 15 (CLAUDE.md), no texture work for an off-screen deck. `DeckClock.h:33-39` also ticks the
     OUTGOING clip of a fade while off screen; on screen `applyTransition` fetches it (`CompositorEngine.cpp:1675`).
   Each clip's sequence is advanced exactly once per frame: compositeDeck (:1090), Mask (:1198), persistent layers of
   OTHER decks (:1357; `DeckClock.h:27-32` skips persistent media), the outgoing clip (:1594 via :1675). A layer's
   two live chains during a fade are two Clips with two ids (`Layer.h:276-277`: previous != active) -> two
   `ImageSequence` instances (keyed by clip id, `Renderer.h:593`), each with its own textures, even for the same files.
F2 (VERIFIED) Who creates / deletes textures, on which thread:
   - created in `ImageSequence::uploadFrame` (`ImageSequence.cpp:239-253`: `glGenTextures` + `glTexImage2D` + 4
     params, one NEW texture per frame index, `textures_[idx] = tex`), called from `getCurrentTexture` (:292), which
     runs on the GL thread under `imageSeqMutex_` (`Renderer.cpp:1646`, :1720);
   - a frame's texture is kept as long as the object lives: NOTHING deletes it per frame (:297 returns it when set;
     :276 drops any second result for a resident index);
   - deleted only by `releaseGL` (:331-349): at context loss (`Renderer::openGLContextClosing`, `Renderer.cpp:1127-1132`)
     and when a retired sequence is drained (`drainRetiredMedia`, :1528; retired by `closeMediaForClip` on the message
     thread, :1497-1505; drained every frame at :311 and at context close :1145). `close()` touches no GL (:95-106).
   - Every open is a NEW object (`openImageSequenceForClip`, `Renderer.cpp:1455-1472`: `make_unique`, the old one
     retired), so `open()` runs once per object.
   Memory (INFERRED arithmetic): 1920x1080 RGBA8 = 8,294,400 B = 7.91 MiB; 3840x2160 = 33,177,600 B = 31.64 MiB. A
   300-frame 1080p sequence = 2,488,320,000 B = 2373.05 MiB = 2.32 GiB after ONE full pass (every frame uploaded
   once); a second pass adds nothing (:276). 40 frames at 4K = 1265.6 MiB.
F3 (VERIFIED) Decode path (s-rta-0928 R1.4, `21091d9`): `requestFrame` (:204-213) issues one `ImageDecode::Decoder`
   job per frame (tag = openGen << 32 | idx, weak mailbox `box_`), at most `kMaxOutstanding = 4` in flight
   (`ImageSequence.h:118-119`); `requestAhead` (:215-236) requests the next `kLookAhead = 3` frames in the play
   direction (Loop wraps, PingPong follows `pingPongForward_`, OneShot stops at the end, :226-233). `getCurrentTexture`
   (:255-312): drains the mailbox (try_lock, :266), keeps results in `ready_` until the frame's `UploadBudget` grants
   them (:287-291; the first upload of a frame is always granted, more while <= 8 MiB, `ImageTexCache.h:259-271`),
   uploads (:292), returns the current frame (:297-302) or the LAST SHOWN frame (:305-306), returns 0 "no media" only
   for a failed frame when nothing was ever shown (:307-308), else `*pending` (:309-311). `firstFramePending` (:316-329)
   is the same rule, read-only (C1 for sequences). The Renderer counts a pending sequence for the capture gate
   (`Renderer.cpp:1721-1726` -> `compositor_.notePendingImage`; gate at :2333).
   CPU-side decoded memory: `Result.rgba` (w*h*4) per outstanding job, in the mailbox and in `ready_`; freed on
   upload (:293) or when a result is dropped (:278, :284). Bound per sequence: `kMaxOutstanding` x frame bytes
   (INFERRED: 32 MiB at 1080p, 127 MiB at 4K) plus, in the pool, 3 running decodes x (juce::Image ARGB + rgba).
F4 (VERIFIED) Context loss: `releaseGL` (:331-349) deletes and clears `textures_`, `requested_`, `ready_`,
   `outstanding_`, `lastShown_`; `ensureFrameState` (:196-202) re-sizes on the next call (D8 of renderleft.md).
F5 (VERIFIED) Precedents for texture reuse in this codebase: `TextureManager::uploadPixels`
   (`src/render/TextureManager.cpp:40-47`: `glTexSubImage2D` when the size matches, else a new texture) and
   `VideoPlayer::uploadToTexture` (`src/media/VideoPlayer.cpp:346-365`: one texture, `glTexSubImage2D` every frame).
   No code changes pixel-store state (plan-renderleft F7, unchanged: `grep -rn GL_UNPACK src` = none), so a
   `glTexSubImage2D` of the same bytes gives the same texels as `glTexImage2D`.
F6 (VERIFIED) The stills' cache is separate: `ImageTexCache::Cache` in CompositorEngine (`CompositorEngine.h:254`),
   prefetch capped by `kImagePrefetchBudgetBytes = 1 GiB` (:270), demand never refused, released by composition
   membership (`applyImageSet`, `ImageTexCache.h:152-174`), reported as `image_textures` / `image_texture_mb`.
   Sequences are NOT in it and never prefetched (renderleft.md NUANCE).
F7 (VERIFIED) `/api/state` today (`src/api/ApiServer.cpp:1297-1328`, `src/test/TestServer.cpp:619-649`): fps,
   frame_time_ms, temporal_buffers, frame_rings, frame_ring_cells, peak_frame_time_ms, peak_callback_ms,
   image_hold_frames, image_skip_frames, images_pending, image_textures, image_texture_mb, peak_image_upload_ms,
   image_pump_frames, gpu_time_ms, peak_gpu_time_ms, master_level, onset_pulse_frames, outputs{...}. NO field names a
   sequence (no texture count, no bytes, no late/shown frame counters). `/api/composition` (`ApiServer.cpp:382-401`)
   reports per layer `activeClipColumn`, `previousClipColumn`, `crossfadeProgress` and per clip `playing`,
   `playheadPosition` (plan4 T7). `/api/switch_deck` takes `{"deck": i}` (:698). `/api/set_clip_param` writes
   `fitMode` only (:620): transport fields come from the composition file (`Clip::fromVar`, `src/model/Clip.cpp:213-216`
   reads `transportMode`, `loopMode`, `speed`, `reverse` UNCONDITIONALLY -- a clip JSON without `"speed"` gets speed 0.0;
   INFERRED consequence: probe-image-load's `seq_clip()` (`.harmony/probe-image-load.py:189-191`) never animates; i6
   still passes because it accepts any of the 12 frames. Every sequence clip in the new probe carries
   `"speed": 1.0, "transportMode": 0, "loopMode": m, "reverse": false`).
F8 (VERIFIED) Message-thread decodes around a sequence (the NEXT item's territory, not designed here):
   `ImageSequence::open` decodes frame 0 for `width_/height_` (`ImageSequence.cpp:57-67`) -- `getWidth/getHeight` have
   NO caller outside the class (grep `->getWidth()` / `->getHeight()` on sequences: none; only the log line :76-78
   reads them); `openMediaForDeck` decodes the first frame for the thumbnail (`MainComponent.cpp:2940-2942`); the
   sequence drop does too (:4957-4959); `open` also stats every file (:32).
F9 (VERIFIED) Measured costs to size the window (renderleft.md FACTS, R1.0 table, load 5.8-6.0): decode 1920x1080
   noisy PNG 28.0-28.3 ms, 3840x2160 110-114 ms; convert (old loop) 5.5 / 23 ms -- the R1.1 row loop is faster
   (INFERRED); upload 1.9-2.0 / 1.6-3.4 ms. The decoder is 3 low-priority threads (`ImageDecode.h:120-126`).
   INFERRED throughput: ~100 frames/s at 1080p, ~25/s at 4K, aggregate, idle machine. A 1080p 30 fps sequence needs
   ~1 thread; a 4K 30 fps sequence cannot be sustained by the decoder (today's FIRST pass has the same limit).
F10 (VERIFIED) Rig / repo facts: CLAUDE.md is 24,482 B of 25,000; its pitfall index ends at 53 (:230); pitfalls.md ends
   at 53 (`docs/claude/pitfalls.md:115`); the image-loading paragraph is `docs/claude/rendering.md:71`; tests are
   appended at the EOF of `tests/CMakeLists.txt` (2650 lines; pure-test block shape at :2593-2601); the lock helper is
   `.harmony/.reports/s-rta-0928/gate-scripts/lock.sh`; probe scaffolding = `.harmony/probe-image-load.sh` (71 lines).
   ctest 846 / 90 targets at close (APP-INVENTORY.md:31).

## 2. RULINGS (every design fork)

R-1 Retain whole when it fits, window when it does not -- NOT "always window". A short loop (the VJ's normal asset:
   3-30 frames, default 2.5 fps `Clip.h:31`) keeps today's zero-decode steady state; only sequences beyond the free
   budget re-decode per pass. Rejected: always-window (regresses 4 layers of 1080p 30 fps loops from 0 to ~3.6
   thread-seconds/s of decode -- late frames, INFERRED from F9); a per-sequence cap without a global one (20 cells of
   30-frame 1080p loops = 4.7 GiB, the same failure spread over cells).
R-2 The window's size = the free budget, floor `kMinWindowFrames` = 8. With trajectory-distance eviction a cyclic pass
   over N frames with C slots keeps ~C-2 frames resident every lap (Belady: the evicted frame is always the one just
   passed) -- so a lone 300-frame 1080p loop holds 129 frames (1 GiB) and re-decodes ~171 per lap instead of 300.
   Rejected: a small fixed window (8 frames) -- same memory bound only when several sequences play, 100 % re-decode
   otherwise, no benefit.
R-3 Eviction order = trajectory distance (the frame shown farthest in the future goes first), never `cur`, never
   `lastShown`. Direction-aware by construction: PingPong near a bounce keeps the frames just behind (they are next);
   reverse flips the walk. A reverse toggle mid-play costs a few late frames (hold) -- accepted.
R-4 Seeks / jumps: read `cur` once per call; a jump outside the window requests `cur` (demand) + look-ahead and shows
   `lastShown` (Pitfall 53's hold, never 0); frames now far from `cur` are evicted as new ones arrive; a decoded
   result for a frame that is now outside the window is DROPPED, not uploaded (`wanted()`), saving the upload budget.
R-5 TWO budgets, not one. Stills: membership retain (a VJ's next trigger must be instant; plan-renderleft rejected LRU),
   prefetch-stop soft cap, demand never refused. Sequences: trajectory eviction, hard allowance with a floor. One
   shared pool would either pin every sequence at its floor while a big still set sits at the cap (constant
   re-decode), or let sequences evict stills a VJ is about to fire. Both are named constants (ASSUMED 1 GiB each); both
   are reported (`image_texture_mb` + `seq_texture_mb`); the machine's memory is an open question for Harmony.
R-6 Texture reuse: a per-sequence slot table; a frame is uploaded into a free slot of the same size with
   `glTexSubImage2D` (F5 precedent, per frame in VideoPlayer), a free slot of another size is re-specified with
   `glTexImage2D` (parameters persist on the texture object), a new slot is created only while slots < allowance.
   Evicting frees the slot (no GL call); `shrink` deletes free slots beyond a smaller allowance; `releaseGL` deletes
   all. VRAM held = allocated slots, which is what `seq_texture_mb` reports.
R-7 Idle sequences (not drawn in the previous frame -- an inactive deck, an inactive column, a retired-but-undrained
   one is already gone) are trimmed to {cur, lastShown} ONLY under budget pressure, least-recently drawn first; under
   the budget they stay warm (a deck switched back shows at once). A trimmed or moved-on sequence shows `lastShown`
   for one decode latency on return (~30 ms 1080p / ~120 ms 4K, F9) -- a Boris feel item (section 9).
R-8 Look-ahead `kLookAhead` 3 -> 4 (covers 30 fps at 1080p and 10 fps at 4K with one decode of latency, F9:
   rate x T_decode + 1 = 1.9 / 2.3), `kMaxOutstanding` stays 4. The look-ahead is counted in TRAJECTORY frames
   (the next 4 frames, whether resident or not), so a failed frame does not extend it (R-7 of renderleft: the last
   frame repeats over a failed one). OneShot requests never cross the end (today's rule, :230-231); its eviction ORDER
   wraps (a retrigger restarts at the in-point, so frame 0 is "next" after n-1).
R-9 Counters first. Commit 1 adds only counters + the probe, so the memory rows are RED by value on that build
   (the R1.0 pattern), and RED by absence on main.
R-10 Not in scope, named as the seam (section 5): the message-thread decodes of F8, the `seekTo` cross-thread write
   (F1), video (F16 of renderleft).

## 3. TRADEOFFS CONSIDERED (rejected)
- Sharing frames between two instances of the same files (a path-keyed sequence cache): two cells with the same folder
  are rare; the cache would need cross-object lifetime rules; each instance's window already bounds it.
- Adaptive look-ahead from a running decode-time estimate: two knobs to tune with no measured need (F9 shows 4 frames
  cover every sustainable rate); the late-frame counter tells when it is wrong.
- Pre-warming the incoming deck's sequences at a deck switch: ~30-120 ms of `lastShown` on return is the same class of
  hold Boris already sees for a cold still; add only if he asks (section 9).
- A PBO / off-thread upload: not provable on this driver without a debugger (plan-renderleft tradeoffs), and the upload
  is 2-3 ms and already budgeted per frame.
- LRU by last-shown time: for a cyclic pass LRU is the worst policy (every frame misses); trajectory distance is the
  optimal one and costs one O(n) walk per drawn sequence per frame (n = 300 -> negligible).

## 4. DECISION / SPEC

### 4.0 Builder step 0 (rig)
- Worktree lane (`df -h /System/Volumes/Data` first; 8 GB/lane rule), `LANE=seqvram . <gate-scripts>/lock.sh`,
  `open -g` only, quit via `quit_app`, never the Output window, no lldb/dtrace/sample, no synthetic input, HTTP
  `Connection: close`, >= 5 runs per arm for any flake verdict. Build Release with `AUDIODNA_BUILD_TEST_SERVER=ON`; keep
  a signed copy of the app after every commit (renderperf METHOD; `MAIN` = the untouched main build for RED runs).
- Record on main's app before any change: ctest count (846), probe-image-load 37/0 and probe-crossfade 35/0.

### 4.1 `src/media/SeqVram.h` (NEW, header-only, pure: `<algorithm> <atomic> <cstdint> <vector>` only)
```cpp
namespace SeqVram {
constexpr size_t kBudgetBytes = size_t{1} << 30;   // ASSUMED: all image sequences together; stills have their own
constexpr int kLookAhead = 4, kMaxOutstanding = 4, kMinWindowFrames = 8;   // cur + shown + 4 ahead + 2 arriving
struct Stats {  // relaxed atomics, written on the GL thread, read by /api/state
    std::atomic<int64_t> framesShown{0}, lateFrames{0}, pendingFrames{0}, uploads{0}, slotReuses{0},
                         evictions{0}, staleDrops{0}, residentBytes{0};
    std::atomic<int> openCount{0}, residentSlots{0}, overBudget{0};
};
struct Grant { size_t allowanceBytes = 0; uint64_t frameSerial = 0; Stats* stats = nullptr; };
inline size_t allowance(size_t othersBytes, size_t minBytes)   // this sequence may hold: the free budget, never < floor
{ return std::max(minBytes, kBudgetBytes > othersBytes ? kBudgetBytes - othersBytes : size_t{0}); }
inline int allowanceFrames(size_t allowanceBytes, size_t frameBytes)
{ return frameBytes == 0 ? kMinWindowFrames : std::max<int>(kMinWindowFrames, static_cast<int>(allowanceBytes / frameBytes)); }

enum class Mode : uint8_t { Loop, PingPong, OneShot };
struct Transport { int n = 0; int cur = 0; bool forward = true; Mode mode = Mode::Loop; };
// dist[j] = playback steps from cur until frame j is shown (0 at cur), by ONE walk of the trajectory (<= 2n steps):
// Loop and OneShot wrap (a OneShot retrigger restarts, so 0 follows n-1), PingPong reflects at both ends (n-1 -> n-2,
// 0 -> 1) flipping the direction. Steps count time, so a PingPong revisit of cur counts (n=6, cur=1, backward:
// dist[0]=1, dist[2]=3).
inline void distances(const Transport& t, std::vector<int>& dist);
// Requests: cur first if not resident/requested/failed (demand), then the first kLookAhead trajectory frames after cur
// that are none of those -- for OneShot only frames reachable without crossing the end; at most freeOutstanding.
// Evictions: while resident - evicted + incoming > allowanceFrames, the resident frame with the largest dist that is
// neither cur nor lastShown; stop when none is evictable (the floor).
struct Plan { std::vector<int> request, evict; };
inline Plan plan(const Transport& t, const std::vector<int>& dist, const std::vector<uint8_t>& resident,
                 const std::vector<uint8_t>& requested, const std::vector<uint8_t>& failed, int lastShown,
                 int allowanceFrames, int incoming, int freeOutstanding);
// A decoded result is still wanted iff its frame is cur, lastShown, or inside the window (dist < allowanceFrames).
inline bool wanted(const std::vector<int>& dist, int j, int cur, int lastShown, int allowanceFrames);

class Slots {   // GL-thread owned; the caller does the GL calls the returned Act names
public:
    enum class Act : uint8_t { Reuse, Respecify, Create, Full };   // SubImage / TexImage on the slot / Gen+TexImage / evict first
    struct Acquire { int slot = -1; Act act = Act::Full; };
    Acquire acquire(int frame, int w, int h, int cap);   // prefers a free slot of the same size, then any free slot,
                                                         // then Create while size() < cap; marks the slot occupied
    void bind(int slot, uint32_t tex);                   // after Create: the new texture name
    void release(int frame);                             // the slot becomes free (VRAM kept for reuse)
    std::vector<uint32_t> shrink(int cap);               // textures of FREE slots beyond cap (to delete); occupied stay
    std::vector<uint32_t> releaseAll();                  // every texture (context loss / retire)
    int slotOf(int frame) const; uint32_t texOf(int slot) const; int frameOf(int slot) const;
    size_t allocatedBytes() const;   // every allocated slot, free or not = the VRAM held
    int size() const; int occupied() const;
private:
    struct Slot { uint32_t tex = 0; int w = 0, h = 0, frame = -1; };
    std::vector<Slot> slots_;
};
}
```

### 4.2 `src/media/ImageSequence.h/.cpp` -- the window + slots, same semantics for callers
`ImageSequence.h`:
- :5-6 add `#include "media/SeqVram.h"`.
- :69-74 signature: `GLuint getCurrentTexture(ImageDecode::Decoder&, ImageTexCache::UploadBudget&, const SeqVram::Grant&, bool* pending);`
  comment: frames live in a bounded window of recycled slots (SeqVram.h); `*pending` and the late-frame repeat as before.
- after :79 add (GL thread): `size_t residentBytes() const;` (= `slots_.allocatedBytes()`), `int residentSlots() const;`,
  `uint64_t lastDrawnSerial() const;`, `size_t minWindowBytes() const;` (= kMinWindowFrames x `frameBytesHint()`),
  `void trimToMinimum();` (evict all but cur / lastShown, then `shrink(2)` + glDeleteTextures).
- :95-97 keep `textures_` (frame -> slot texture or 0: `firstFramePending` :316-329 and the return path read it);
  DELETE `textureWidths_` / `textureHeights_` (written :250-251, read nowhere); add `SeqVram::Slots slots_;`,
  `std::vector<int> dist_;` (scratch), `size_t frameBytesHint_ = 0;` (the first decoded result's w*h*4; before that
  `width_*height_*4` from open(), else 1080p), `uint64_t lastDrawnSerial_ = 0;`, `int lastReturned_ = -1;`.
- :118-119 replace `kLookAhead` / `kMaxOutstanding` with `SeqVram::kLookAhead` / `kMaxOutstanding`.
- :127-130 helpers: keep `ensureFrameState`, `requestFrame`; DELETE `requestAhead`; `uploadFrame(const Result&, int idx)`
  becomes "acquire slot + SubImage/TexImage"; add `void evictFrame(int idx)` (`slots_.release`, `textures_[idx] = 0`,
  `++stats->evictions`), `SeqVram::Transport transport() const`.
`ImageSequence.cpp`:
- :45-54 `open`: `textures_.assign(n, 0)` (was resize; slots are GL objects and are never touched here);
  `frameBytesHint_` from :59-62's dims when valid.
- :196-202 `ensureFrameState`: `textures_` only (+ requested_/failed_ as today).
- :204-213 `requestFrame`: unchanged (bounded by `SeqVram::kMaxOutstanding`).
- :215-236 `requestAhead`: deleted -- `SeqVram::plan` decides.
- :238-253 `uploadFrame`: `auto a = slots_.acquire(idx, r.w, r.h, cap)`; Full -> the caller evicted first (never
  reached: plan guarantees room; jassert); Create -> the old GL calls verbatim (:242-248) + `slots_.bind`; Respecify ->
  bind + `glTexImage2D(... r.w, r.h ...)`; Reuse -> bind + `glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, r.w, r.h, GL_RGBA,
  GL_UNSIGNED_BYTE, r.rgba.data())` (`++slotReuses`); then `textures_[idx] = slots_.texOf(a.slot)`; `++uploads`;
  `frameBytesHint_ = r.w * r.h * 4` on the first upload.
- :255-312 `getCurrentTexture(decoder, budget, grant, pending)` -- new body, in this order:
  1. `*pending = false`; open/empty checks; `ensureFrameState`; `lastDrawnSerial_ = grant.frameSerial`;
     `const int cur = clamp(currentFrameIndex_)` read ONCE; `t = transport()`; `SeqVram::distances(t, dist_)`;
     `cap = SeqVram::allowanceFrames(grant.allowanceBytes, frameBytesHint())`.
  2. Drain `box_` into `ready_` (as :265-271). For each ready result: wrong gen / out of range / already resident ->
     erase (as :276-280); Failed -> `failed_[idx] = 1`, erase (as :281-286); `!wanted(dist_, idx, cur, lastShown_, cap)`
     -> erase, `++staleDrops`; else count it as `incoming`.
  3. `p = SeqVram::plan(t, dist_, residentFlags, requested_, failed_, lastShown_, cap, incoming, kMaxOutstanding - outstanding_)`;
     `for j in p.evict: evictFrame(j)`; if `slots_.size() > cap`: `for tex in slots_.shrink(cap): glDeleteTextures`.
  4. Upload the wanted results within `budget.take(bytes)` (as :287-293); a result that misses the budget stays in
     `ready_` (its slot is not taken until upload; the `incoming` count reserved room for it this frame -- if it is
     still unwanted next frame, step 2 drops it).
  5. `for j in p.request: requestFrame(decoder, j)`.
  6. Return: `textures_[cur] != 0` -> `lastShown_ = cur`, return it; else if `lastShown_` resident -> `++lateFrames`,
     return it; else failed -> 0 (:307-308); else `*pending = true`, `++pendingFrames`, return 0. Whenever the returned
     frame index != `lastReturned_` -> `++framesShown`, `lastReturned_ = idx`.
- :316-329 `firstFramePending`: unchanged (it reads `textures_[]`, `lastShown_`, `failed_`).
- :331-349 `releaseGL`: `for tex in slots_.releaseAll(): glDeleteTextures`; `textures_.clear()` (as today) and the rest
  verbatim; `lastReturned_ = -1`.
- new `trimToMinimum()`: for every resident j != cur, != lastShown_: `evictFrame(j)`; then delete `slots_.shrink(2)`.
Invariants the ctest + probe guard: `lastShown_` and `cur` are never evicted; `slots_.size() <= cap` after step 3;
`textures_[j] != 0` iff `slots_.slotOf(j) >= 0`.

### 4.3 `src/render/Renderer.h/.cpp` -- the grant, the frame-top scan, the trim
`Renderer.h`:
- after :286: `SeqVram::Stats seqStats_; uint64_t seqFrameSerial_ = 0; size_t seqResidentTotal_ = 0;
  void scanSequenceVram();   // frame top, GL thread: totals + idle trim under pressure (SeqVram.h)` with the
  Sacred-Rule-2 note: no new mutex -- one more O(#sequences) critical section on `imageSeqMutex_`, which `syncMedia`
  already takes per drawn clip (:1646).
- public getter `const SeqVram::Stats& getSeqStats() const`.
- :617-627 comment: `syncMedia` hands the sequence a `SeqVram::Grant`.
`Renderer.cpp`:
- after :311 (`drainRetiredMedia();`): `scanSequenceVram();` -- every frame, before the early returns at :451/:455 (B2).
- new `scanSequenceVram()` next to `drainRetiredMedia` (:1509-1533): `++seqFrameSerial_`; lock `imageSeqMutex_`; sum
  `residentBytes()` / `residentSlots()`; collect idle = `lastDrawnSerial() + 1 < seqFrameSerial_`; if total >
  `SeqVram::kBudgetBytes`: sort idle by `lastDrawnSerial` ascending, `trimToMinimum()` each until total <= budget;
  `seqResidentTotal_ = total`; publish `seqStats_.residentBytes / residentSlots / openCount / overBudget`.
- :1717-1727 (the decode branch): before `getCurrentTexture`:
  ```cpp
  const size_t mine = seq->residentBytes();
  const SeqVram::Grant grant{ SeqVram::allowance(seqResidentTotal_ - std::min(seqResidentTotal_, mine), seq->minWindowBytes()),
                              seqFrameSerial_, &seqStats_ };
  const GLuint tex = seq->getCurrentTexture(imageDecoder_, uploadBudget_, grant, &seqPending);
  ```
  (two sequences drawn in one frame can overshoot the budget by at most that frame's uploads -- <= the 8 MiB budget
  + one first upload; the next frame's allowances correct it: the allowance is the free budget as of the frame top.)
- `openGLContextClosing` (:1127-1132) and `drainRetiredMedia` (:1528): unchanged (`releaseGL` deletes the slots).
`src/api/ApiServer.cpp` after :1324 and `src/test/TestServer.cpp` after :645 (additive, both servers):
```cpp
// s-rta-0928b seqvram: image sequences play through a bounded window of recycled textures (SeqVram.h).
{
    const auto& s = renderer_.getSeqStats();
    obj->setProperty("seq_open", s.openCount.load(std::memory_order_relaxed));
    obj->setProperty("seq_textures", s.residentSlots.load(std::memory_order_relaxed));                 // allocated slots
    obj->setProperty("seq_texture_mb", s.residentBytes.load(std::memory_order_relaxed) / (1024.0 * 1024.0));
    obj->setProperty("seq_over_budget", s.overBudget.load(std::memory_order_relaxed));
    obj->setProperty("seq_frames_shown", (juce::int64) s.framesShown.load(std::memory_order_relaxed));  // frame-index changes presented
    obj->setProperty("seq_late_frames", (juce::int64) s.lateFrames.load(std::memory_order_relaxed));    // current frame not resident: the shown one repeated
    obj->setProperty("seq_pending_frames", (juce::int64) s.pendingFrames.load(std::memory_order_relaxed)); // nothing to show yet
    obj->setProperty("seq_uploads", (juce::int64) s.uploads.load(std::memory_order_relaxed));
    obj->setProperty("seq_slot_reuses", (juce::int64) s.slotReuses.load(std::memory_order_relaxed));    // glTexSubImage2D into a recycled slot
    obj->setProperty("seq_evictions", (juce::int64) s.evictions.load(std::memory_order_relaxed));
    obj->setProperty("seq_stale_drops", (juce::int64) s.staleDrops.load(std::memory_order_relaxed));
}
```
Commit 1 ships these fields with `Stats` filled from the OLD code paths (uploads / framesShown / lateFrames /
pendingFrames in `getCurrentTexture`; residentBytes = Σ over `textures_ != 0` of w*h*4 from `textureWidths_/Heights_`,
counted in the frame-top scan without any trim) so the RED numbers are real.

### 4.4 ctests (RED first: write each case, watch it fail against the missing header / a stub, then implement)
NEW `tests/test_seq_vram.cpp`, registered at the EOF of `tests/CMakeLists.txt` like `test_frame_ring` (:2324-2331:
`Catch2::Catch2WithMain`, `apply_sanitizers`, `catch_discover_tests`), tag `[seq_vram][s-rta-0928b]`:
 (1) distances Loop: n=6 cur=4 forward -> {2,3,4,5,0,1}; reverse cur=1 -> dist[0]=1, dist[5]=2, dist[4]=3.
 (2) distances PingPong: n=6 cur=4 forward -> dist[5]=1, dist[3]=2, dist[2]=3, dist[0]=5; cur=1 backward ->
     dist[0]=1, dist[2]=3 (the revisit of cur counts a step); n=1 -> {0}.
 (3) OneShot: distances wrap like Loop (n=6 cur=4: dist[0]=2), but plan() requests only {5} beyond cur (never 0,1,2);
     reverse at cur=1 requests {0} only.
 (4) plan requests, Loop n=10 cur=8 lookahead 4, resident {8}: {9,0,1,2}; requested {9} -> {0,1,2}; freeOutstanding 2
     -> {0,1}; failed {0} -> {1,2} (a failed frame does not extend the look-ahead); cur not resident -> cur first.
 (5) eviction order, Loop n=300 cur=150 lastShown=149 resident 140..153 allowance 10 incoming 1 -> evict
     {148,147,146,145,144} exactly, never 150 / 149; resident - 5 + 1 == 10.
 (6) the floor: allowance 1, resident {cur, lastShown, x} -> evict {x} only (cur / lastShown stay even over the cap).
 (7) PingPong near a bounce keeps the frames behind: n=40 cur=38 forward lastShown=37 resident 30..39 allowance 8
     incoming 0 -> evict {30, 31}; 37 never evicted.
 (8) allowance(): others 0 -> 1 GiB; others 1 GiB - 1 -> the floor; others > budget -> the floor.
     allowanceFrames(1 GiB, 8,294,400) == 129; (1 GiB, 33,177,600) == 32; frameBytes 0 -> kMinWindowFrames.
 (9) wanted(): a result with dist >= allowance is stale unless it is cur or lastShown.
(10) Slots: cap 2 -> Create slot 0, bind 7; Create slot 1, bind 9; third acquire -> Full; release(frame 0) -> acquire
     same size -> Reuse slot 0 (tex 7); release(frame 1) -> acquire 1920x1080 in a 256x256 slot -> Respecify;
     shrink(1) returns the FREE slot's texture only (occupied stays); releaseAll returns every texture;
     allocatedBytes == Σ w*h*4 over allocated slots (free ones included).
(11) invariants by simulation (a small model of getCurrentTexture over 2,000 steps, Loop / PingPong / reverse, random
     seeks every 97 steps, allowance 8 and 129): resident never > allowance; cur / lastShown never evicted; after a seek
     the request list starts with cur.
Teeth, run once on mutated copies (record the FAIL line, restore, `sleep 1; touch` before rebuilding -- notebook):
 drop the lastShown protection -> (5)(6)(7)(11) FAIL; no wrap in Loop distances -> (1)(4) FAIL; PingPong without the
 bounce -> (2)(7) FAIL; Reuse for a mismatched size -> (10) FAIL; OneShot requesting past the end -> (3) FAIL.
Byte identity: abpix-style S4 (renderleft BYTE-IDENTITY): a 3-frame sequence at `sequenceFps` 0.1 -- `render_frame`
arrays array_equal main vs lane (same `Result.rgba`, F5). Existing gates unchanged: probe-image-load 37/0 (its i6 and
i2ms / i3s rows drive sequences), probe-crossfade 35/0, ctest 846 + the new cases.

### 4.5 Live probe: `.harmony/probe-seq-vram.sh` + `.py` + `.json`
`.sh` = probe-image-load.sh's scaffolding verbatim (F10) with `SEQVRAM_APP` / `SEQVRAM_PY` / `SEQVRAM_ENV`, out dir
`seqvram.XXXXXX`, fixtures written BEFORE launch, production mode, foreign-traffic check, `rm -rf "$OUT/media"` after
the quit. `.py` reuses probe-image-load.py's helpers (load / trig / state / cap / dbox / Poller / wait_no_compiler /
wait_active; copy them -- the two probes stay independent files). Rows print load avg; every capture is decoded with
PIL.
Fixtures (`$OUT/media`, numpy seed 928, PIL compress_level 1): frame k of a set = R = 255x/W, G = 255y/H, B = (37k) % 256
(smooth: ~0.3-0.5 MB per 1080p frame, decode ~15-30 ms INFERRED from F9) plus a 64-px CODE BAND at the bottom: 10
cells across, cell c white iff bit c of k (1024 codes). `code(cap)` = threshold the mean of each cell's inner 50 %.
Sets: `L300` = 300 x 1920x1080 (~120 MB, ~30-40 s to write, ASSUMED -- printed), `Q40` = 40 x 3840x2160, plus
`warm.png` (256x144 flat). Every sequence clip JSON: `mediaType 5, sequenceFiles [...], sequenceFps f, speed 1.0,
transportMode 0, loopMode m, reverse false` (F7). Layers: `transitionSpeed 0` (cuts). Constants in `.json`:
`budgetMB 1024`, `slackMB 8`, `frameMB1080 7.91`, `frameMB4k 31.64`, `minWindowFrames 8`, `shownTol 3`,
`lateMaxAfterJump 12`, `boxTol 3.0`, `peakMaxMs 16.7`, layer ids 81-87.
Expected frame index for a capture = `int(playheadPosition * n)` from `/api/composition` read right after the capture
(F1: frame = int(t * fps), playhead = t / dur, n = dur * fps); accept |code - expected| <= 1 (mod n).

| row | steps | PASS | RED on main / on the commit-1 app |
|---|---|---|---|
| v1_memory_1080 | canvas 1920x1080; layer 81 col 0 = L300 at fps 30, Loop. load; trig; wait 2 laps (2 x 10 s) + 1 s; s1; capA (+ playhead). | (a) `seq_textures` <= budgetMB/frameMB1080 + 1 = 130; (b) `seq_texture_mb` <= 1024 + 8; (c) `seq_over_budget` == 0; (d) code(capA) == expected +- 1 and dbox(capA, fixture[code]) <= 3.0 (pixels decoded, the slot holds the frame it claims). Prints textures / MB / uploads / evictions / reuses. | main: fields absent -> FAIL. commit 1: `seq_textures` 300, `seq_texture_mb` 2373.0 (2 laps) -> (a)(b) FAIL by value. |
| v2_smooth_loop_1080 | L300 at fps 10, Loop; trig; 1 s; quiet wait; s0; poll `/api/state` 8 s (Poller adds `peak_frame_time_ms`); s1; then caps at +0, +1, +2 s each with a playhead read. | (a) `seq_late_frames` delta == 0; (b) `seq_pending_frames` delta == 0; (c) `seq_frames_shown` delta in [80 - 3, 80 + 3]; (d) max `peak_frame_time_ms` <= 16.7 (no upload stall); (e) the 3 codes strictly increase mod 300 and each == expected +- 1 with dbox <= 3.0. | main: absent. commit 1: PASS (everything resident after lap 1 -- the guard). Teeth T1 (kLookAhead 0, kMaxOutstanding 1): late ~160-240 (INFERRED: 2-3 render frames per 30 ms decode at 60 Hz) -> (a) FAIL. |
| v3_smooth_pingpong_4k | canvas 3840x2160; Q40 at fps 10, PingPong; trig; 1 s; quiet; s0; 12 s poll; s1; 2 caps. | (a) late delta == 0; (b) pending delta == 0; (c) shown delta in [120 - 3 - 3, 120 + 3] (3 bounces, one repeated index each); (d) `seq_texture_mb` <= 1024 + 8 and `seq_textures` <= 33; (e) codes == expected +- 1, dbox <= 3.0. | main: absent. commit 1: (d) FAIL by value (40 x 31.64 = 1265.6 MiB). Teeth T2 (kMinWindowFrames 3 + kBudgetBytes 64 MiB): allowance 3 frames at 4K -> late > 0 -> (a) FAIL (a window too small to hide the decode). |
| v4_jump_outside_window | L300 at fps 30, Loop; trig; 5.5 s (cur ~ 150-165, far from frame 0); s0; trig the SAME column (a retrigger: `Layer.h:263-273` + `MainComponent.cpp:4324-4328` -> `seekTo(inPoint)` = frame 0, outside the window); poll 0.5 s; s1; cap at once (+ playhead). | (a) pending delta == 0 (never black / nothing); (b) 1 <= late delta <= 12 (a hold of the shown frame for the decode, not a freeze); (c) code(cap) == expected +- 1 (or +- 2: 30 fps), dbox <= 3.0 -- a decoded frame, not a stale slot. | main: absent. commit 1: (b) FAIL (late 0: frame 0 is resident on main; the row documents the new hold -- Harmony may downgrade (b) to "<= 12" only). |
| v6_budget_share | 2 layers: 82 = L300[0:120] (120 frames, 949 MiB, fits whole) at fps 30; 83 = L300[120:300] (180 frames) at fps 10. trig 82 only; 5 s (> 1 lap of 4 s); s1; 4 s (a second lap); s2; trig 83; 8 s; s3 (+ Poller late over the 8 s). | (a) `seq_uploads`(s2) - (s1) == 0 (a sequence that fits re-decodes nothing on its second lap -- today's behaviour kept) and `seq_textures`(s2) == 120; (b) `seq_texture_mb`(s3) <= 1024 + 8 (the second sequence gets the free 75 MiB = 9 frames, near the floor); (c) late delta over the 8 s == 0 (both smooth at the floor); (d) `seq_over_budget`(s3) == 0. | main: absent. commit 1: (a) PASS, (b) FAIL by value (120 + 80 frames after 8 s at 10 fps = 1582 MiB). |
| v7_deck_keeps_time | deck 0 layer 84 = L300 at fps 10; deck 1 layer 85 = warm.png. trig (0, 84, 0); 2 s; read playhead p0 + s0; `switch_deck 1`; 3.0 s; `switch_deck 0`; read p1 at once; 1 s; s1; cap (+ playhead). | (a) (p1 - p0) x 30 s in [2.6, 3.4] (rule 15: the clock ran off screen); (b) pending delta == 0 since s0; (c) late delta <= 12 (one decode of hold on return, R-7); (d) code == expected +- 1, dbox <= 3.0. | main: absent (a guard; (a) holds on main by T4). |

Order: v1, v2, v4, v6 (1080p rows) then v3 (4K canvas) then v7. GREEN = every row PASS twice on the final app at
load avg printed; RED lines on MAIN and on the commit-1 app recorded verbatim in the report; teeth T1 / T2 recorded
with the restore sha. A flake verdict needs >= 5 runs per arm (rig rule).

### 4.6 What "done" looks like
- ctest: 846 + 11 new cases green, serial (`ctest -j1`); teeth lines recorded.
- probe-seq-vram: 6 rows GREEN twice on the final app; RED lines on main (absent) and on the commit-1 app (v1 300 /
  2373.0, v3 1265.6, v6 1582) recorded; T1 / T2 FAIL lines recorded, restore sha equal.
- probe-image-load 37/0, probe-crossfade 35/0, probe-render-state / canvas at their recorded counts (the sequence rows
  they contain still pass); abpix S4 array_equal.
- The report's tables: per-row numbers (textures / MB / uploads / evictions / reuses / late / shown / peak frame).

## 5. THE SEAM FOR THE NEXT ITEM (message-thread decodes -- named, not designed)
- `ImageSequence::open` frame-0 decode (`ImageSequence.cpp:57-67`) exists only to fill `width_/height_`, which no caller
  reads (F8). This design does NOT depend on them: `frameBytesHint_` takes the first decoded result's dims and falls
  back to 1080p until then. The next lane can delete :57-67 (and the members) with nothing else to change; the
  sequence's `[ImageSequence] Opened` log line loses its dims.
- First-frame thumbnails (`MainComponent.cpp:2940-2942`, :4957-4959; `ImageSequence::getThumbnail` :351-367) -- the
  seam is `ClipThumbnails` (Pitfall 51: the pool + `callAsync` store), keyed by `sequenceFiles[0]`; nothing here touches
  thumbnails (the restore lane's fence, renderleft C6).
- `open` stats every file on the message thread (:32) -- a 300-file burst; the same lane's call.
- `seekTo` from the message thread (F1) -- the lane that moves decodes off the message thread should turn it into a
  request consumed on the GL thread (an atomic pending seek), like `Renderer::loadImage` became O(1).

## 6. MUST NOT CHANGE (and how each is kept)
- Transport math and playhead: `advanceFrame` (:125-194), `seekTo` (:115-123), `syncMedia`'s sync block
  (`Renderer.cpp:1653-1712`), `DeckClock` / `LayerClock` -- untouched; rule 15 untouched (the off-screen path returns
  before `getCurrentTexture`, :1717-1718). v7 (a) is the witness.
- Pitfall 53 / C1 / C3: `*pending` only when nothing was ever shown; `firstFramePending` (:316-329) reads the same
  `textures_[]`; the capture gate (`Renderer.cpp:2333`) and `notePendingImage` (:1721-1726) untouched. `lastShown_` is
  never evicted, so "late frame repeats" always has a texture after the first show. i2ms / i3s stay GREEN.
- Pixels: the same `Result.rgba` (StraightRGBA, `ImageDecode.h`) uploaded with the same format/type and unchanged
  pixel-store state (F5) -> byte-identical texels; abpix S4 array_equal.
- `CompositorEngine.h/.cpp`, `ImageDecode.h`, `ImageTexCache.h` (the stills' cache and its budget), `UploadBudget`
  semantics (sequences still `take()` from the frame's budget), `PixelConvert.h`: untouched.
- No new FBO (Pitfall 3), no history-key change (35), no capture-path change (52), no new mutex (Sacred Rule 2: one
  more O(#sequences) critical section per frame on the existing `imageSeqMutex_`, GL work inside it only in the
  pressure trim -- precedent: `getCurrentTexture` already uploads under it, :1646-1720).
- `/api/state`: fields added, none renamed; `/api/composition` untouched.
- The retire / drain lifecycle (`closeMediaForClip`, `drainRetiredMedia`, `openGLContextClosing`): untouched; `releaseGL`
  still deletes everything on the GL thread.
- Fence: this lane edits `src/media/ImageSequence.h/.cpp`, NEW `src/media/SeqVram.h`, `src/render/Renderer.h/.cpp`
  (:286 members, :311 call, :1717-1727 grant, one new function), `src/api/ApiServer.cpp` + `src/test/TestServer.cpp`
  (the state block), `tests/test_seq_vram.cpp` + `tests/CMakeLists.txt` (EOF append), `.harmony/probe-seq-vram.*`, docs.
  Nothing in `src/ui`, `src/render/CompositorEngine.*`, `src/model`.

## 7. RISKS (strongest counterargument first)
R1 "A bounded window re-decodes long sequences every lap: a 300-frame loop that played flawlessly after lap 1 now costs
   ~17 decodes/s forever (171 of 300 frames per 10 s lap at 30 fps) and can stutter under CPU load, because the decoder
   threads are low priority." It loses: today that loop holds 2.32 GiB of VRAM -- two of them exhaust an 8-16 GB
   unified-memory Mac (INFERRED; the machine's memory is ASSUMED), a hard, unbounded failure against a soft one; the
   window is the largest the budget allows (Belady: 43 % of the frames still hit), sequences that fit keep today's
   zero-decode steady state (v6 (a)), and `seq_late_frames` reports the stutter when it happens. Lever left: the
   named constants (`kBudgetBytes`, the thread count) -- Harmony / Boris decide.
R2 A deck switched back, or a jump outside the window, shows the last frame for one decode (~30 ms at 1080p, ~120 ms
   at 4K) where today it was instant after lap 1 (F2). A feel item for Boris (section 9); pre-warm is the fallback.
R3 `glTexSubImage2D` into a slot the GPU read last frame may stall the driver (implicit sync). Precedent: VideoPlayer
   does it every frame (F5). v2 (d) / v3 (d) (`peak_frame_time_ms` <= 16.7 during steady playback) catch a stall; if
   RED, use two spare slots (kMinWindowFrames covers +2) and never reuse the slot released THIS frame -- a one-line
   rule in `Slots::acquire` (prefer the least-recently released free slot).
R4 A slot-mapping bug shows the wrong frame with the right index. The code band vs `playheadPosition` check (v2 (e),
   v4 (c), v7 (d)) and ctest (10)/(11) are the witnesses.
R5 Two budgets plus floors can exceed 2 GiB of image VRAM with many 4K sequences (each floor = 253 MiB). Reported
   (`seq_over_budget`); the constants are ASSUMED; open question Q1.
R6 The pre-existing message-thread `seekTo` write (F1) can change `cur` between `advanceFrame` and `getCurrentTexture`;
   the policy reads `cur` once per call and tolerates any value (worst case one extra late frame). Not fixed here.
R7 Fixtures: 300 + 40 PNGs (~200 MB, ~1 min to write) and ~3 min of rows under the live lock; rows are selectable by
   name (the `.sh`'s second argument) for re-runs.
R8 Renderer.cpp is in no ctest (renderleft F8): the grant / scan / trim are proven live only (v6, v7) plus the teeth
   builds. The policy itself is pure and fully tested.
R9 Perf-ish rows (late frames) flake under load: quiet rule (no clang running), load avg printed, >= 5 runs per arm.
R10 CLAUDE.md is 24,482 of 25,000 B: one index line (~150 B) fits; if Harmony's final line does not, shorten index
   line 53 (194 B) as in section 9.
R11 `openGen_` / `outstanding_` bookkeeping is unchanged, but the new stale-drop path erases results earlier than
   today: `outstanding_` is decremented at arrival (:269), before any drop -- keep that order.

## 8. COMMIT SEQUENCE (each builds, passes ctest and keeps every existing probe GREEN; each reverts alone)
1. `perf(seq): sequence VRAM counters seq_* in /api/state (7070 + 8080); probe-seq-vram RED harness` -- `SeqVram::Stats`
   + `Grant` (header, no policy yet), counters in the OLD `getCurrentTexture` and a frame-top scan that only sums;
   the 3 probe files. RED runs recorded: main (absent) and this app (v1 300 / 2373.0; v3 1265.6; v6 1582).
2. `feat(seq): SeqVram -- trajectory-distance window policy + recycled slot table (pure) + tests` -- `SeqVram.h`
   complete, `tests/test_seq_vram.cpp` (11 cases), CMake EOF block, teeth recorded. No app behaviour change.
3. `perf(seq): image sequences play through a bounded window of recycled textures; Renderer grants allowances from
   kBudgetBytes and trims idle sequences under pressure` -- ImageSequence + Renderer wiring (4.2, 4.3). GREEN: the 6
   rows twice; teeth T1 / T2 builds (mutated constants), FAIL lines + restore sha; abpix S4; image-load 37/0,
   crossfade 35/0; serial ctest.
4. `docs(seq): rendering.md sequence window; pitfall NN; CLAUDE.md index` (section 9).
5. The report commit (`.harmony/.reports/s-rta-0928b/seqvram.md`, `git add -f`).
Trailer on every commit: the session's attribution line.

## 9. DOCS (text; pitfall as "NN" until Harmony assigns the number)
- `docs/claude/rendering.md:71`: replace the sentence "**Image sequences** decode their frames on the same decoder with
  a look-ahead of 3 frames ... (it used to index an emptied texture vector)." with:
  "**Image sequences (s-rta-0928b seqvram)** decode their frames on the same decoder, 4 frames ahead along the
  sequence's own trajectory (`SeqVram::distances`: Loop wraps, PingPong bounces, reverse flips; <= 4 outstanding), and
  play through a BOUNDED window of recycled GL textures (`SeqVram::Slots`: a frame is uploaded into a free slot with
  `glTexSubImage2D`, never glGen/glDelete per frame). All sequences together hold at most `SeqVram::kBudgetBytes`
  (1 GiB, ASSUMED) -- separate from the stills' prefetch budget: a sequence that fits keeps every frame (no re-decode
  after its first pass); one that does not gets the free budget as its window (never below 8 frames), evicting the
  frame shown farthest in the future -- never the current frame, never the one on screen. Every frame the Renderer
  sums the sequences' resident bytes at the frame top (`scanSequenceVram`) and, only over the budget, trims sequences
  not drawn last frame (inactive decks / columns) to their current + shown frames, least-recently drawn first. A frame
  not resident (a seek outside the window, a deck switched back) shows the last frame shown for one decode; `*pending`
  only when nothing was ever shown (the `render_frame` gate, C1 / C3 unchanged); a frame that fails to decode is never
  retried. `/api/state`: `seq_open`, `seq_textures` (allocated slots), `seq_texture_mb`, `seq_over_budget`,
  `seq_frames_shown`, `seq_late_frames`, `seq_pending_frames`, `seq_uploads`, `seq_slot_reuses`, `seq_evictions`,
  `seq_stale_drops`. `ImageSequence::releaseGL` deletes every slot (context loss / retire). Guards:
  `tests/test_seq_vram.cpp`; live `.harmony/probe-seq-vram.sh`."
- `docs/claude/pitfalls.md` after :115:
  "NN. **An image sequence never owns more VRAM than its window -- frames live in a recycled slot ring evicted by
  trajectory distance, and the current frame and the frame on screen are never evicted**: `ImageSequence` used to
  `glGenTextures` + `glTexImage2D` one texture per frame index and keep it for the object's life (a 300-frame 1080p
  sequence = 2373 MiB after one pass; 40 frames at 4K = 1266 MiB). Textures now come from `SeqVram::Slots` (reuse with
  `glTexSubImage2D`), sized by `SeqVram::allowance` (the free share of `kBudgetBytes`, floor 8 frames) and ordered by
  `SeqVram::distances` (steps until a frame is shown again: Loop / OneShot wrap, PingPong bounces, reverse flips) --
  never LRU (for a cyclic pass LRU misses every frame; farthest-next-use keeps ~allowance-2 frames hitting each lap).
  Rules: evict BEFORE uploading; a decoded result outside the window is dropped, not uploaded; `lastShown_` / `cur`
  are protected (a late frame always has a texture -- Pitfall 53's hold, `firstFramePending` unchanged); only the
  Renderer's frame-top scan trims sequences that were not drawn, and only over the budget. Before adding a per-frame GL
  object to a media class, give it a slot table and a budget. Guards: `tests/test_seq_vram.cpp` (order, floor,
  protection, slots); live: `.harmony/probe-seq-vram.sh` (v1 / v3 / v6 memory, v2 / v3 smoothness, v4 / v7 hold)."
- `CLAUDE.md` after :230 (~150 B; `wc -c` must stay <= 25,000):
  "NN. A sequence's textures are a bounded recycled window (SeqVram; the shown frame is never evicted) -- before
  touching ImageSequence textures or a media class's per-frame GL objects."
  If the cap is hit: shorten line 230 to "53. Images decode off the GL thread: pending is never 0 / no media;
  render_frame waits for a complete frame -- before touching getKeyTexture or a clip-texture branch." (-30 B).
- `.harmony/HANDOFF.md` START HERE item 2: closed by Harmony with the numbers (shared file; the lane does not edit it).
  APP-INVENTORY counts at close (Harmony).
- Boris checks (for Harmony's page): (1) a long image sequence (more than ~130 frames at 1080p / ~30 at 4K) now
  re-loads its frames as it plays instead of keeping them all -- on an idle machine the playback is unchanged; under
  heavy CPU load a frame may repeat (`seq_late_frames`). (2) Coming back to a deck whose sequence moved on, or jumping
  in a long sequence, shows the last frame for a blink (~1/30 s at 1080p, ~1/8 s at 4K) before the new one; short
  loops are unaffected. (3) Total sequence memory is capped at 1 GiB (plus 1 GiB for photos); say if his machine wants
  a different split.

## 10. COMPACT
- Today: one GL texture per sequence frame, kept for the object's life (`ImageSequence.cpp:239-253`, deleted only in
  `releaseGL` :331-349); 300 x 1080p = 2373 MiB after one pass. Frame index is time-based per mode (:125-194); seeks
  from 3 message-thread callers; one advance per clip per frame; off-screen decks tick without decoding (:1717-1718).
- Design: pure `SeqVram.h` (trajectory-distance window: request nearest, evict farthest, never cur / lastShown; slot
  table recycled via glTexSubImage2D; `allowance` = free share of a 1 GiB sequence budget, floor 8 frames; fits-whole
  sequences keep every frame) + Renderer frame-top scan (sums, trims idle sequences under pressure, grants).
- Two budgets (stills' prefetch cap stays); the sum is visible in /api/state.
- Gates: 11 pure ctest cases with teeth; probe-seq-vram 6 rows -- RED on main by absence, RED by VALUE on the
  counters-only commit (v1 300 textures / 2373.0 MiB; v3 1265.6; v6 1582), GREEN after; smoothness rows (late == 0,
  frames shown == expected +- 3, code band == playhead frame, pixels decoded) with teeth T1 (no look-ahead) / T2 (a
  3-frame window) that FAIL them.
- Seam left for the message-thread lane: `open`'s frame-0 decode is dead once `frameBytesHint_` exists; thumbnails go to
  ClipThumbnails; `seekTo` becomes a GL-thread request.
- Open questions for Harmony: Q1 the machine's memory (the two 1 GiB constants are ASSUMED); Q2 v4 (b)'s lower bound
  (documenting the new hold) -- keep or drop; Q3 pre-warm on deck switch (R2) -- only if Boris dislikes the blink.

REPORT_FILE: .harmony/.reports/s-rta-0928b/plan-seqvram.md
STATUS: FINAL

## HARMONY ADOPTION (s-rta-0928b, 17:50) — OVERRIDES THE BODY WHERE THEY DIFFER
Plan authored by Fable (tier: fable). Attacked by two blind seats: attack-seqvram-gl.md, attack-seqvram-vj.md. Rulings:

- H1 (GL MUST-1, ADOPT) `Slots::acquire` returning `Full` has a DEFINED Release behaviour — never an assert-only path:
  the result is NOT uploaded this frame (it stays in `ready_`; step 2 re-judges it next frame), `++stats->uploadDeferred`
  (new counter `seq_upload_deferred` in /api/state), and every `Slots` accessor (`bind`, `texOf`, `frameOf`, `release`)
  is a no-op / returns 0 for an out-of-range slot. New ctest: starve the allowance mid-frame (cap 2, both slots
  occupied by cur + lastShown, one incoming result) -> acquire returns Full, no state changes, no out-of-range access
  (run the pure test under the existing sanitizer wiring, `apply_sanitizers`).
- H2 (GL MUST-2, ADOPT) `Slots::releaseAll()` returns every texture AND empties the table (`slots_.clear()`). New ctest:
  after `releaseAll()` -> `size() == 0`, `occupied() == 0`, `allocatedBytes() == 0`, and the next `acquire()` returns
  `Create` (never Reuse / Respecify).
- H3 (GL SHOULD-1, ADOPT) The Renderer keeps a RUNNING total within the frame: after each drawn sequence's
  `getCurrentTexture`, `seqResidentTotal_ += residentBytes(after) - residentBytes(before)`; the next sequence's grant is
  computed from the running total (not the frame-top snapshot). The frame-top scan still re-sums from scratch.
- H4 (GL SHOULD-2, NOTE ONLY) State in code comment + report: eviction protects `lastShown_` (GL-thread-only state)
  independently of `cur`, so a stale `currentFrameIndex_` read (pre-existing message-thread `seekTo` write) can mis-order
  one request but can never evict the frame on screen. Converting `seekTo` into a GL-thread request belongs to the
  media-decode lane (section 5 seam), not here.
- H5 (GL NIT, ADOPT) Before the sequence's frame size is known (no decoded result yet AND `open()` gave no valid dims),
  `allowanceFrames` = `kMinWindowFrames` (never a 1080p guess): a 4K sequence cannot over-create slots at startup, and
  the seam that later deletes `open()`'s frame-0 decode stays safe.
- H6 (HARMONY FINDING — the seats missed it) CLIP IN/OUT POINTS. `Renderer::syncMedia` applies in/out points
  (plan F1 :1700-1712: OneShot stops, else `seekTo(inPoint)`), but `SeqVram::Transport` has no in/out range, so near the
  out-point the look-ahead would request frames past it (never shown) and the distances would order the loop body
  wrongly. The builder re-derives how in/out are expressed and applied to sequences; then `Transport` carries
  `[inFrame, outFrame]`: Loop wraps out -> in, PingPong reflects at in and out, OneShot stops at out; frames outside
  the range get the largest distance (evicted first, never requested). New ctest cases (n = 300, in = 100, out = 199:
  cur 197 forward -> requests {198, 199, 100, 101}; nothing outside [100, 199] is requested; with allowance 8 an
  outside-range resident frame is evicted before any in-range one). If in/out do not apply to sequences, record the
  evidence and skip H6.
- H7 (Q2 + a Harmony-found ERROR in row v4, OVERRIDE) Row v4's premise is wrong: trajectory-distance (Belady) eviction
  KEEPS the frames right after the wrap (frames 0..~112 stay resident at cur ~150-165 in lap 1 at a 129-frame window;
  the evicted ones are the most recently passed, cur-2 downward). A retrigger to the in-point is therefore a HIT by
  design, and v4 (b) "late >= 1" would FAIL on the correct build. Replace v4 with `v4_retrigger_hit`: L300 at fps 30,
  Loop; trig; wait until `seq_evictions` > 0 and `seq_textures` >= 125 (window full, lap 1); s0; retrigger the same
  column (seekTo(inPoint) = frame 0); poll 0.5 s; s1; cap. PASS: (a) pending delta == 0; (b) late delta == 0 (the
  in-point frames were kept — the VJ value of Belady); (c) code(cap) == expected +- 2, dbox <= 3.0; (d) `seq_textures`
  <= 130 at s1. RED: main absent; commit-1 app (d) by value. The "miss -> hold the shown frame" witness moves to v7:
  add (c') late delta >= 1 on the return (deterministic: frames the clock ran past while off screen were never decoded).
- H8 (VJ MUST-2 + GL MUST-3, ADOPT) New row `v8_crossfade_two_long`: one layer, column 0 = L300 at fps 30 and column 1 =
  a second 300-frame 1080p set (different code offset), layer `transitionSpeed` set for a real ~2 s dissolve (read how
  transition time is expressed). Trig col 0; wait until its window is full (as H7); s0; trig col 1 (the dissolve: two
  live chains, Pitfall 35); Poller over the fade + 2 s; s1; caps at mid-fade and after. PASS: (a) pending delta <= 6 and none after column 1's
  first shown frame (only the incoming chain's first decode: Pitfall 53's "nothing shown yet", which a sequence does
  not pause for -- renderleft D5); (b) max `peak_frame_time_ms` over the fade <= 16.7; (c) `seq_texture_mb` <= 1024 + 16 at s1 (after the outgoing chain
  retires); (d) mid-fade capture not black (mean luma > 20) and the after-fade capture's code == expected +- 2 of
  column 1; (e) late delta over the fade reported, asserted <= 12 (one decode of hold on the incoming chain's first
  frames is expected; a sustained late run is not). RED: main absent; commit-1 app (c) by value.
- H9 (VJ MUST-1, ADOPT) New row `v9_three_decoding`: three layers, each a 150-frame 1080p set (distinct codes) at fps 15,
  Loop, triggered together; 25 s (2.5 laps; each sequence's window ~43 frames at a 1 GiB budget, so each re-decodes
  ~11 frames/s: aggregate ~33 decodes/s against the shared 3-thread pool and the per-frame UploadBudget). PASS on a
  quiet machine (no compiler, load printed): (a) late delta over the last 20 s == 0; (b) pending delta over the same 20 s == 0;
  (c) `seq_texture_mb` <= 1024 + 16; (d) codes of each layer == expected +- 2 in one capture. RED: main absent;
  commit-1 app (c) by value (450 frames = 3559 MiB). If (a) fails on a quiet machine in >= 3 of 5 runs, STOP and report
  to Harmony with the late distribution (a design finding, never a re-threshold).
- H10 (VJ MUST-3, ADOPT — ruling on the budget semantics) FLOORS WIN: a drawn sequence never drops below
  `kMinWindowFrames`; when the drawn sequences' floors alone exceed `kBudgetBytes`, the total may exceed it and
  `seq_over_budget` = 1 says so. Invariant (doc + ctest on `allowance`): settled total <= max(kBudgetBytes, sum of drawn
  floors) + one frame's uploads. New row `v10_floors_win`: canvas 4K; five layers each a 20-frame 4K set (subsets of Q40)
  at fps 3, triggered together; 15 s; s1. PASS: (a) `seq_open` == 5, `seq_over_budget` == 1; (b) `seq_texture_mb` <=
  5 x 8 x 31.64 + 32 (= 1298); (c) no pending on any layer after 10 s. Late reported, not asserted (4K decode-bound).
  RED: main absent; commit-1 app (b) by value (100 frames = 3164 MiB).
- H11 (VJ SHOULD-5 + NIT-6, ADOPT as one row) New row `v7b_trim_spike`: deck 0 = three long 1080p sequences drawn until
  the budget is full; deck 1 = two long 1080p sequences; `switch_deck 1`; Poller over 5 s. PASS: (a) max
  `peak_frame_time_ms` over the switch window <= 16.7 (the frame-top trim of deck 0's idle sequences is not a spike);
  (b) `seq_texture_mb` <= 1024 + 16 after 5 s; (c) deck 1's sequences show (codes == expected +- 2).
- H12 (VJ SHOULD-4, RULING: ctest + document, no live row) Reverse toggling and BPM-synced fps change no code this plan
  touches (fps / reverse math is on the must-not-change list); the policy's reverse handling is covered by ctest
  (1)(2)(11). The decoder pool is the throughput ceiling today as well (a 4K sequence above ~10 fps cannot be sustained
  on its first pass on main either); `seq_late_frames` reports it. Document that ceiling in rendering.md.
- H13 (Q1, RULED) Boris's machine: Apple M1 Pro, 32 GiB unified memory (`sysctl hw.memsize`, VERIFIED 17:47). Keep
  `kBudgetBytes` = 1 GiB for sequences + the stills' 1 GiB (named constants). Boris check: say if a longer sequence
  should keep more frames.
- H14 (Q3, RULED) No pre-warm on deck switch; a Boris feel item only.
- H15 Row list and order: v1, v2, v4 (H7), v6, v8, v9, v7, v7b (1080p) then v3, v10 (4K). GREEN = every row PASS on
  2 consecutive runs of the final app on a quiet machine; any flake verdict >= 5 runs per arm. Keep the plan's teeth
  T1 / T2 and add T3: `releaseAll` without `slots_.clear()` -> the H2 ctest FAILs (record, restore, `sleep 1; touch`).
- H16 Filed, NOT this lane (Harmony ledger): `Clip::fromVar` reads `speed` unconditionally (a hand-written clip JSON
  without "speed" plays at 0) and probe-image-load's `seq_clip()` therefore never animates. Do not edit
  probe-image-load in this lane.
- H17 Pitfall text stays "NN" (Harmony assigns 54 at merge). The lane does not touch HANDOFF.md or APP-INVENTORY.md.
  Reviewers check `git diff main..lane -- docs CLAUDE.md` removes nothing it did not mean to.

## HARMONY ADOPTION ADDENDUM — fix round (s-rta-0928b, 20:47) — rulings on the lane's R-A / R-B / R-C and the r1 reviews
Evidence: lane report seqvram.md (d2b4411), review-seqvram-gl-r1.md (FAIL: 1 MUST), review-seqvram-gates-r1.md (FAIL: 2 MUST,
1 SHOULD). Accepted as re-derived from code: D1 (PingPong distances), D2 (Transport models syncMedia's real in/out behaviour —
out-point only; Loop and PingPong jump to in), D4 (wanted() keeps a result when it fits), D6, D8.
- F1 (R-A, ADOPT the idle-age rule) A sequence is IDLE only after it has not been drawn for >= kIdleFrames = 60 consecutive
  frame serials (~0.5 s at 120 Hz). A Pitfall 53 pending hold (CompositorEngine.cpp ~1093, which skips the outgoing chain for
  1-3 frames at a crossfade start) therefore never makes the outgoing chain idle. CompositorEngine stays untouched. ctest: a
  pure helper (e.g. SeqVram::isIdle(lastDrawn, serial)) with the boundary cases 59 / 60.
- F2 (R-B, ADOPT a per-frame GL delete budget) kMaxDeletesPerFrame = 8 glDeleteTextures per render frame, SHARED by the
  frame-top trim, every per-sequence shrink, AND the retire drain (drainRetiredMedia -> a retired sequence releases at most the
  remaining budget per frame and stays on the retired list until empty). Context loss (openGLContextClosing) still releases
  everything at once. Slots evicted beyond the budget stay allocated as FREE slots (counted in seq_texture_mb, reusable) and are
  deleted on later frames while over the budget. Report the per-frame delete count (new counter seq_deletes, cumulative).
- F3 (R-C, ADOPT: drawn sequences outrank idle ones) The grant for a drawn sequence counts idle sequences' bytes above their
  2-frame minimum as RECLAIMABLE: allowance = max(floor, kBudgetBytes - (drawn others' bytes) - (idle sequences' minimum
  bytes)). As the drawn sequence grows the total exceeds the budget and the frame-top scan trims idle sequences (least
  recently drawn first) under F2's delete budget. Under the budget with no drawn sequence wanting memory, idle sequences stay
  warm (R-7 unchanged). H10 floors-win unchanged.
- F4 (GL SHOULD / D7, ADOPT) Check slot feasibility (acquire) BEFORE budget.take(), so a Full never spends the frame's upload
  budget. The probe prints seq_upload_deferred in every row.
- F5 (gates SHOULD-1, ADOPT) v8_crossfade_two_long gains (b') max peak_callback_ms over the fade <= 16.7 (the frame-top scan
  runs outside the frame timer). PASS required on 5 of 5 consecutive runs on the fixed app; RED arm = the lane head d2b4411
  app, 5 runs, recorded verbatim (the lane measured 3/7 at 22-24 ms).
- F6 (R-B gate, ADOPT) Promote the scratch row v7c_one_per_deck into the probe: one 300-frame 1080p sequence per deck, switch
  when deck 0's window is full; PASS: max peak_callback_ms over the 2 s after the switch <= 16.7 on 5 of 5 runs, and
  seq_texture_mb <= 1024 + 16 after 5 s. RED arm = d2b4411 app, 5 runs (the lane measured 5/5 at ~22 ms).
- F7 (R-C gate, ADOPT) v8 gains (f): within 3 s after the fade completes the incoming sequence holds >= 64 textures (read a
  per-layer count if one exists; else seq_textures minus the outgoing's minimum) and late == 0 over the next 2 s; RED arm =
  d2b4411 app (the lane measured the incoming held at its 8-frame floor).
- F8 Retire drain: add one report-only row v11_retire (after v1's state, load an empty composition; print max peak_callback_ms
  and seq_deletes over 2 s). Not asserted this round (the composition swap has its own fence costs; the media-open lane owns
  those).
- F9 Every other row stays as adopted; GREEN = the full probe PASS on 2 consecutive runs + F5/F6 5 of 5; ctest serial; the
  existing probes image-load 37/0, crossfade 35/0 re-run once on the final app. Docs: the rendering.md sentence and pitfall NN
  text gain the idle-age rule, the delete budget and "drawn outranks idle" in <= 3 lines; CLAUDE.md untouched beyond the NN line.
