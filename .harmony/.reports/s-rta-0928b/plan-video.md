# plan-video -- video decoding off the GL render thread (s-rta-0928b START HERE item 3, part V)

Architect plan (Fable). Base: main @ 5b78d43 -- every file:line below was read at that HEAD in
/Users/boriskarpman/projects/RealTimeAudio. Bins: VERIFIED (read at the line, measured in diag-media, or run in a
scratch rig today) / INFERRED (derived) / ASSUMED (not checked -- the builder checks it). Inputs read in full:
diag-media.md (+ agg-v / pool-v / seekstats / instr_vp.py), plan-renderleft.md + its HARMONY ADOPTION, plan-seqvram.md
+ its HARMONY ADOPTION, Pitfalls 35 / 52 / 53, rendering.md, performance-controls.md (rule 15, DeckClock),
BORIS_DECISIONS.md "Playback Behaviour", the source named below.

QUESTION: take video decode + colour conversion + flip + the redundant re-upload off the GL render thread (S1a), end
the mid-GOP re-seek loop that freezes the whole output for ~3.4 s (S1b), stop holding `videoPlayerMutex_` across the
decode (X1), and the three "also" items (getThumbnail race, the per-frame tempRow allocation, the beat-snap seek) --
deciding between the full off-thread design, the cheap steps b1 / b2, or both in sequence; keeping Pitfall 53 / 35,
rule 15 and Boris's playback rulings; with RED-first gates an opus builder can run under the rig rules.

APPROACH (stated first):
- BOTH, in sequence, with b2 dropped as a standalone step. Commit 1 = counters + the probe (RED by value on the old
  code, the seqvram "counters first" pattern). Commit 2 = b1 (upload a video frame only when a new one was decoded:
  6 lines, measured -4.2 ms median at 4 x 1080p, 93 -> 104 fps). Commit 3 = the decode thread: one serial decode
  context per player (`VideoPlayer` owns a `juce::Thread` at normal priority; FFmpeg's `thread_count` stays 2), a
  3-slot pre-allocated RGBA ring written bottom-up by `sws_scale` with a NEGATIVE destination stride (VERIFIED today
  on this machine's FFmpeg 8.0: byte-equal to the flipped positive-stride output -- the 0.39 / 1.73 ms flip and the
  tempRow allocation disappear), tagged {pts, generation, seq}. The GL thread keeps the transport clock
  (`advanceTransport` unchanged), posts the wanted time, picks the newest ring frame with pts <= clock (+ half a
  frame, today's `:431` rule) of the current generation, uploads ONLY when it changed, and never waits (atomic slot
  states, CAS; no lock, no FFmpeg call). Seeks stay requests (`seekTo` unchanged for every caller): the GL thread
  applies them to its clock and bumps the generation; the thread re-seeks to the keyframe and catches up, dropping
  frames WITHOUT converting them; meanwhile the layer holds the last shown frame (never 0 / no media). b2 (a bounded
  catch-up inside `decodeFrameAtTime`) is superseded by that thread policy and is written once, not twice; it is the
  named fallback if commit 3 cannot land.
- Why not b1 + b2 alone (the strongest counterargument): the diag's v2 arm (upload-only-new) measured 4 x 4K at 46
  fps -- decode + sws + flip (8.3 ms per new 4K frame per player) stay on the GL thread and its `receive_frame`
  blocks when FFmpeg's workers fall behind (p90 20.6 ms); a time-bounded b2 turns a 150-frame 4K catch-up into a
  ~2.7 s hold at 4 ms per frame; and X1 (the mutex across the decode: 25-120 ms message-thread waits) stays. Only
  the thread fixes m1 at 4K x 4; v1 (no decode, 4 uploads still per frame) gave 95.9 fps -- with one upload per
  render frame on average the callback is ~1.4 ms (INFERRED), so >= 110 fps is the bar.
- Pitfall 53 for video: shown-before = HOLD the texture (counted `video_late_frames` when the clock has passed the
  next content frame); never-shown = PENDING (C1 crossfade pause via a generalised media-pending provider; C3 gate
  via the same `notePendingImage` counter). `open()` still decodes frame 0 (message thread, unchanged cost) into ring
  slot 0 and builds the 90x72 thumbnail there, so a fresh trigger without a seek is never pending and the
  `getThumbnail` race is gone (nothing reads a frame buffer any more).
- Rule 15: an off-screen deck's player advances its clock only (`advanceClock`, unchanged); its thread idles after
  250 ms without a draw request; on return the thread re-seeks and catches up while the layer holds -- bounded by
  one GOP of decode: ~0.3 s at 1080p / ~1.3 s at 4K for 150 frames past a keyframe (INFERRED from the diag's loop
  rate 2.2 / 8.8 ms per frame), against today's 3.4 s freeze of the WHOLE output at 14 / 3.8 fps.
- X1: `videoPlayerMutex_` guards the map LOOKUP only (O(1)); `close()` signals the thread and returns at once; the
  retire list destroys a player only after its thread has exited (deferred destroy on the GL thread, no join on any
  hot thread); the thread frees the FFmpeg contexts itself on exit.
- Beat-snap semantics unchanged (no Boris ruling on them; `seekTo(beatPhase)` stays). Note (INFERRED, F13): the
  trigger is QUEUED to the beat, so the seek lands near phase 0 -- rarely mid-GOP; the retrigger row covers the
  mechanism.
- VideoToolbox: out (cross-platform app). The ring is the seam a hardware path would feed later (section 5).

## 1. TODAY'S CODE, RE-DERIVED (file:line at 5b78d43)

F1 (VERIFIED) `VideoPlayer` (`src/media/VideoPlayer.h:29-175`): one CPU `frameBuffer_` (:130-133), one GL texture
   (:136-137), transport atomics (:140-150), `ffmpegMutex_` for open/close only (:173-174). Threading comment :22-28
   ("advanceFrame()/uploadToTexture() called from GL thread each frame"). `open()` (`VideoPlayer.cpp:32-193`): FFmpeg
   contexts, `thread_count = 2` (:103), sws to RGBA (:157-159), the frame buffer (:167-173), decodes and converts the
   FIRST frame on the message thread (:175-180), `playing_ = true` (:183). `close()` (:195-216) frees everything at
   once. Every open is a NEW object (`Renderer::openVideoForClip`, `Renderer.cpp:1434`), so re-open never happens.
F2 (VERIFIED) The GL path: `advanceFrame` (:225-253) applies a seek request (:231-242: `seekToTimestamp` + ONE
   `decodeNextFrame`), runs the transport (:244), then `decodeFrameAtTime(currentTime_)` (:248) + `convertFrameToRGBA`
   (:250). `advanceClock` (:255-272, plan4 T3) = the seek to the clock only + `advanceTransport`. `advanceTransport`
   (:274-336): speed / reverse / loop / ping-pong / one-shot; the Loop wrap calls `seekToTimestamp` (:301, :319) -- a
   DEMUXER seek on the GL thread; playhead store (:333-334).
F3 (VERIFIED) `decodeFrameAtTime` (:417-470): "close enough" = |framePts - t| < 0.5 / fps (:427-433); re-seek when
   diff < -0.1 s or > 2.0 s (:436-445); then decode up to 30 frames until pts >= t - 0.5 / fps (:448-467); "used up
   attempts, return what we have" (:469). With x264's default GOP 250 a target > ~3 s past a keyframe never reaches
   the target in 30 decodes, and the NEXT frame re-seeks again: the S1b loop (diag: 47-48 frames of 67 ms at 1080p,
   14 of 265 ms at 4K, the picture stuck on keyframe + 29 for 3.3-3.5 s; a GOP-30 control never loops).
   `decodeNextFrame` (:494-529): `av_read_frame` -> `avcodec_send_packet` -> `avcodec_receive_frame`; EOF returns
   false WITHOUT draining the decoder (frame-threading holds the last 1-2 frames back). `seekToTimestamp` (:472-492):
   `av_seek_frame(... AVSEEK_FLAG_BACKWARD)` + `avcodec_flush_buffers`.
F4 (VERIFIED) `convertFrameToRGBA` (:531-556): `sws_scale` into the buffer top-down (:537-540), then a row-swap flip
   with a `std::vector tempRow` allocated per decoded frame (:544-555) -- the heap allocation on the render thread.
   Measured (diag pool-v): sws 0.49 / 1.91 / 4.04 ms, flip 0.39 / 1.73 / 0.37 ms (1080p H.264 / 4K / ProRes).
F5 (VERIFIED) `uploadToTexture` (:338-368) returns `texture_` when `!frameReady_` (:340-341) and otherwise uploads
   EVERY call (`glTexSubImage2D`, :361-364); `frameReady_` is set in `open` (:180) and `advanceFrame` (:239, :251) and
   never cleared -> a 30 fps clip is uploaded 120x per second (diag: 603 uploads / 151 decodes; 1080p upload 1.2-1.36
   ms, 4K 0.86 ms). `releaseGL` (:370-378) deletes the texture.
F6 (VERIFIED) `getThumbnail` (:380-413) reads `frameBuffer_` per pixel on the MESSAGE thread (:389-401) while the GL
   thread may be writing it (F4) -- the race the diag INFERRED (found_not_fixed 5). All three callers pass (90, 72):
   `MainComponent.cpp:2932` (openMediaForDeck), `:4890` (drop), `:6557` (Replace Content). Cost 7.9 ms (1080p) /
   31.9 ms (4K) per clip (diag S2).
F7 (VERIFIED) `Renderer::syncMedia` video branch (`src/render/Renderer.cpp:1575-1643`): `videoPlayerMutex_` is taken
   at :1577 and held to the return at :1642 -- across the transport sync (:1584-1616), `advanceFrame` / `advanceClock`
   (:1618-1621), the playhead / playing write-back (:1622-1625), the in/out enforcement (:1627-1640) and
   `uploadToTexture` (:1642). The message thread waits behind it in `getVideoPlayer` (:1535-1540),
   `getVideoPlayerFile` (:1542-1547), `closeMediaForClip` (:1486) and `openVideoForClip`'s insert (:1450): 25-120 ms
   during the S1b loop, 85 ms fence waits at a swap with 4 x 4K (diag X1 / S2). `getVideoFrameTexture` (:1556-1559)
   is the compositor's `videoFrameFn_` (:263-265). `tickMediaClock` (`Renderer.h:627`) = `syncMedia(clip, dt, false)`.
F8 (VERIFIED) Lifecycle: `openVideoForClip` (:1432-1453) opens on the message thread, retires the old id through
   `closeMediaForClip` (:1448), inserts under the mutex (:1450-1451). `closeMediaForClip` (:1474-1507) calls
   `close()` under the mutex (:1490) and moves the player to `retiredVideoPlayers_` (:1491-1493). `drainRetiredMedia`
   (:1509-1533, GL thread, every frame at :311 and at context close :1145) swaps the lists out under
   `retiredMediaMutex_`, `releaseGL()`s and destroys them. `openGLContextClosing` releases every live player's texture
   under the mutex (:1120-1125). The GL-THREAD DESTROY GUARD comment: `Renderer.h:595-615`. `videoPlayers_` /
   `videoPlayerMutex_`: `Renderer.h:588-589`.
F9 (VERIFIED) The compositor calls `videoFrameFn_(clip, dt, &pending)` at `CompositorEngine.cpp:1090` (Opaque /
   Transparent), `:1198` (Mask), `:1357` (persistent layers), `:1594` (`getClipTexture`, the OUTGOING clip of a fade);
   `pending` -> hold `heldLayerOutput` / nothing (:1094-1107, Pitfall 53), the Mask's last image mask (:1204-1214).
   `VideoFrameFn` comment says pending is "never set for video" (`CompositorEngine.h:113-116`). C1: `advanceCrossfade`
   runs only if `!incomingImagePending(layer, clip)` (:1060-1062; body :383-395) -- it returns false for Video (:385-
   386) and asks `sequencePendingFn_` for sequences (:393-394; provider `Renderer.cpp:267-271` ->
   `ImageSequence::firstFramePending`). C3: `notePendingImage` / `framePendingImages` (`CompositorEngine.h:87-88`), the
   render_frame gate at `Renderer.cpp:2333`. The sequence branch already sets `*pending` + `notePendingImage`
   (:1719-1726) -- the shape the video branch copies.
F10 (VERIFIED) Rule 15: `DeckClock::tick` (`src/render/DeckClock.h:16-41`) ticks the active AND the outgoing clip of
   every off-screen deck through `tickMediaClock` (`Renderer.cpp:713`); persistent layers are drawn (decode = true) by
   `compositePersistentLayers` (:698). Boris: "keep playing" (BORIS_DECISIONS.md:354-355); B2 wording "with no decode
   and no upload ... the next on-screen frame seeks and decodes, bounded per call" (performance-controls.md:49).
F11 (VERIFIED) Seek callers, all message thread, all through `seekTo(normalized)`: the cuepoint jump
   (`MainComponent.cpp:1491-1505`), beat snap on trigger (:4283-4304: `seekTo(snap.beatPhase)`), the same-column
   retrigger (:4305-4329: `seekTo(clip->inPoint)`; `Layer::triggerClipImmediate`'s retrigger branch `Layer.h:264-274`
   resets the model playhead and starts NO crossfade), and the GL thread itself at the out-point (`Renderer.cpp:1637`).
   A first trigger opens a missing player on the message thread (:4347-4348).
F12 (VERIFIED) `Clip::fromVar` reads `speed` unconditionally (`src/model/Clip.cpp:215`): a clip JSON without it plays
   at 0 (FILED by seqvram H16; not this lane). `MediaType` Video = 2 (`Clip.h:22`); `playing` / `playheadPosition` are
   mutable, written by the GL thread (`Clip.h:228-229`).
F13 (VERIFIED path, INFERRED consequence) `Layer::triggerClip` (`Layer.h:222-250`) QUEUES a trigger whose clip has
   `beatSnapMode != Off || beatSnap` (:238-247); it fires at the next beat crossing, and only then does
   `handleClipTrigger` seek to `snap.beatPhase` (:4288-4296). So the beat-snap seek target is phase ~0.00-0.07 (the
   30 Hz tick latency): 0-0.7 s into a 10 s clip = <= 21 frames past keyframe 0 -- it does NOT normally enter S1b.
   The semantics stay (no ruling); the mechanism (any mid-GOP seek) is what this plan fixes.
F14 (VERIFIED, measured today in a scratch rig) `sws_scale` honours a negative destination stride: an 8x6 yuv420p
   frame converted with `dst[0] = buf + (H-1) * W*4, dstStride[0] = -W*4` is byte-equal to the vertically flipped
   positive-stride output (FFmpeg 8.0, libavcodec 62, `/opt/homebrew/opt/ffmpeg`, VERIFIED by header + run). No
   ctest links FFmpeg (grep tests/CMakeLists.txt: none), so the builder keeps this as a step-0 rig check
   (`$TMPDIR`, `cc ... -I/opt/homebrew/opt/ffmpeg/include -L.../lib -lswscale -lavutil`; `pkg-config` is not on PATH).
F15 (VERIFIED, measured today) An ffmpeg `geq` code band with the frame number `N` -- bottom rows, 10 cells, cell c
   white (235) iff bit c of N -- survives libx264 (crf 20, yuv420p, `-g 250 -sc_threshold 0`) and decodes exactly
   (frames 0 / 5 / 37 / 59 -> codes 0 / 5 / 37 / 59) by thresholding each cell's inner 50 % at 128. `-g 250` on a
   2 s clip gives one keyframe at 0.000 (a 10 s clip: 0 and 8.333 s, INFERRED -- the probe asserts it with ffprobe,
   as diag fixtures.sh does).
F16 (VERIFIED) Precedents: `VideoRecorder` hands GL frames to its encoder thread through atomic indices + a condition
   variable, no mutex on the GL hot path (`src/output/VideoRecorder.h:82-101`, Pitfall 25); `ImageDecode::Decoder` (3
   low-priority pool threads, weak_ptr mailbox, try_lock drain: `src/render/ImageDecode.h`); `ImageSequence`'s
   `lastShown_` hold + `firstFramePending` (`src/media/ImageSequence.h:66-79`). `juce::Thread` API (JUCE 8,
   `build/_deps/juce-src/modules/juce_core/threads/juce_Thread.h:65-482`): `Priority {highest 2, high, normal, low,
   background -2}`, `startThread(Priority)`, `signalThreadShouldExit`, `threadShouldExit`, `stopThread(ms)`,
   `wait(ms)`, `notify()`. macOS QoS mapping (`juce_Threads_mac.mm:51-58`): highest -> USER_INTERACTIVE, high ->
   USER_INITIATED, normal -> DEFAULT, low -> UTILITY, background -> BACKGROUND.
F17 (VERIFIED) `/api/state` today: `TestServer.cpp:619-672` (`onset_pulse_frames` at :653, then the `outputs` block);
   `ApiServer.cpp:1297-1340` (`master_level` at :1330, then the `outputs` block at :1331). `/api/composition` reports
   per clip `playheadPosition` / `playing` (seqvram F7). 7070 endpoints incl. `trigger_clip`, `switch_deck`,
   `load_composition`, `render_frame`, `snapshot` (`ApiServer.cpp:157-264`). Probe scaffolding:
   `.harmony/probe-image-load.sh` (71 lines, production mode) + `.py` helpers (`load / trig / state / comp_state /
   wait_active / cap / dbox / Poller / wait_no_compiler`, :210-330); `.harmony/probe-deck-clock.py` already generates
   video fixtures with ffmpeg (`ramp_video`, :291-298) and refuses without ffmpeg on PATH (its .sh:15).
   `probe-deck-clock` rows that touch video: `d_video_keeps_time` (320x240 GOP 30, away 4 s, capture 0.3 s after the
   return; the catch-up is <= 29 tiny frames -> done within 0.3 s, INFERRED) and `d_return_hitch` (1080p GOP 60, away
   5 s, asserts `peak_frame_time_ms` across the return <= 50 ms "REPORT -- target <= 16": this plan makes it ~normal).
F18 (VERIFIED) Machine: Apple M1 Pro, 10 cores = 8 performance + 2 efficiency (`sysctl hw.perflevel0/1.physicalcpu`),
   32 GiB. Measured costs to size the design (diag pool-v, 5 launches): 1080p H.264 decode call 2.13 / p90 2.34 /
   max 7.5 ms (of which receive 0.045 -- the decode runs on FFmpeg's 2 workers), 4K 4.62 / 4.89 / 28.2, ProRes
   5.66 / 5.92 / 15.3; loop-bound decode throughput 2.2 ms per 1080p frame, 8.8 ms per 4K frame (seekstats:
   67 ms / 30, 265 ms / 30); 4 x 4K collapse: receive blocks (decoding-call p90 20.6, up to 4 decodes per call), fps
   36.0, callback p90 81.7; v1 (no decode, uploads kept) 4.43 ms / 95.9 fps; v2 (upload only new) 1080p x 4 2.66 ms /
   104.1 fps, 4K x 4 46.0 fps.
F19 (VERIFIED) CLAUDE.md is 24,482 B of 25,000 (518 B left); the pitfall index ends at 53 (:230); pitfalls.md's list
   format is "NN. **Title**: text" (its 53 ends the file); tests are appended at the EOF of `tests/CMakeLists.txt`
   (2650 lines; pure-test block shape `:2593-2601`; `apply_sanitizers` from `cmake/Sanitizers.cmake:22`, sanitizers
   off by default, `-DADNA_SANITIZE=thread` in a separate build dir). Renderer.cpp / VideoPlayer.cpp are in no ctest.
F20 (VERIFIED) The seqvram lane (in flight) edits `Renderer.cpp:311` (+1 call after `drainRetiredMedia();`), adds a
   function next to `drainRetiredMedia` (:1509-1533), edits the sequence branch `:1717-1727`, `Renderer.h` after
   :286 (+ a getter), `ApiServer.cpp` after :1324, `TestServer.cpp` after :645, `tests/CMakeLists.txt` EOF,
   rendering.md after :71, pitfalls "NN", the CLAUDE.md index (plan-seqvram 4.3 + H15-H17). Section 7 fences this.

## 2. RULINGS (every design fork)

R-1 Both, in sequence; b2 superseded. Commit 1 counters + probe (RED by value), commit 2 b1 (its own gate: uploads
   ~600 per 5 s at 4 x 1080p instead of ~2400; fps 93 -> ~104), commit 3 the thread (everything else). b2 is not
   written as a step: its content (catch up across calls, bounded per call) is exactly the thread's policy, written
   once (section 4.4); it is the fallback only if commit 3 cannot land in the lane (6 lines named in section 8, R-B).
R-2 One decode thread PER PLAYER (`juce::Thread`, `Priority::normal` = QOS_CLASS_DEFAULT, F16), not a pool with serial
   queues: an FFmpeg context is single-threaded by contract, the per-player thread IS the serial queue, idle threads
   cost nothing (they sleep on `wait`; only DRAWN players decode), and there is no cross-player scheduling to get
   wrong. Thread count: 1 + FFmpeg's 2 workers per open player (the 2 exist today): a 16-cell deck = 48 threads, at
   most (drawn players) active. `thread_count` stays 2 (unchanged behaviour; 4 x 4K steady = 4 x 30 x 8.8 ms = 1.06
   core-s/s of decode wall time, <= ~2.1 core-s/s CPU with 2 workers, + sws 0.23 + GL uploads 0.10 -- <= ~2.5 of 8
   performance cores, INFERRED). `high` and `thread_count 4` are named levers (R-13), not defaults.
R-3 The ring: 3 slots of w*h*4 bytes, allocated in `open()` (message thread; `mmap`-lazy, so RSS grows only when a
   slot is first written: slot 0 at open = today's frame buffer, slots 1-2 at the first draw), freed in the
   destructor (GL thread, deferred -- never while the GL thread may hold a slot). Slot header: `atomic<uint8_t> state
   {Free, Writing, Ready, Reading}`, `double pts`, `uint32_t gen`, `uint64_t seq`. Writer: CAS Free -> Writing, fill
   (sws, negative stride), set pts/gen/seq, `store(Ready, release)`. Reader (GL): scan Ready slots (`load(acquire)`);
   the newest with `gen == current && pts <= clock + 0.5 / fps` -> CAS Ready -> Reading, upload, `store(Free)`; every
   other Ready slot with a stale gen, or the same gen and pts <= the chosen one, -> CAS Ready -> Free (dropped /
   skipped); Ready slots with pts > clock + tol stay (the future). Classic triple buffering (F16 precedent): the
   writer is at most 2 content frames ahead (67 ms at 30 fps), the reader never waits, no slot ever has two owners.
   Memory: 25 MB per 1080p player, 100 MB per 4K player once drawn (16 x 4K all drawn = 1.6 GB vs 530 MB today) --
   R-14 names the trim lever.
R-4 The GL thread owns the clock. `advanceFrame(dt)` (decode path) = apply a seek request (clock jump + `gen++`),
   `advanceTransport` unchanged (the Loop wrap's `seekToTimestamp` becomes `gen++`: a discontinuity, no FFmpeg call on
   the GL thread), publish `wantTime = currentTime_`, `wantGen`, stamp `lastDrawMs`, `notify()` the thread when a slot
   was released this frame / the gen changed / the previous draw was > 100 ms ago (wake from idle). `advanceClock(dt)`
   (rule 15) = the same without the want / stamp / notify (a seek still bumps gen: the ring's frames are stale). The
   transport math (`:274-336`) is byte-identical except those two `seekToTimestamp` calls.
R-5 The thread's policy = today's rules, moved off the GL thread and made non-blocking for the GL thread:
   - read {gen, want} each iteration; `gen` changed -> `av_seek_frame(BACKWARD, want)` + `avcodec_flush_buffers`,
     catch-up mode; else `want < newestPts - 0.1 s` or `want > newestPts + 2.0 s` (today's `:441`) -> the same seek
     (reverse play and far-ahead jumps; reverse of a long-GOP file is O(GOP) per frame today too -- R-12);
   - decode one frame; `pts < want - 1.5 / fps` -> DROP (no sws, no slot; `video_frames_dropped`); else take a Free
     slot (wait <= 20 ms per try until the reader frees one), sws into it, publish; `newestPts = pts`;
   - EOF -> drain the decoder (`send_packet(nullptr)`, receive until EOF -- the last 1-2 frames now show, a documented
     change), then wait: the Loop wrap's gen bump re-seeks; OneShot's clock stops at `duration_`;
   - no draw request for 250 ms -> idle (`wait(-1)` until a `notify`): rule 15's "no decode, no upload" off screen,
     kept verbatim (B2); on return the first draw wakes it and the gap triggers the seek + catch-up above;
   - exit requested -> free the FFmpeg contexts (the thread owns them), `threadDone_ = true`, return.
   Catch-up accelerator `AVDISCARD_NONREF` (skip non-reference frames while > 10 frames from the target; safe by
   definition -- a non-ref frame is never referenced): the policy header carries the predicate, DISABLED by default;
   the builder enables it only if row w3b's 4K hold exceeds its bar (a measured trigger, section 4.6).
R-6 Pitfall 53 for video: `uploadToTexture(bool* pending)`: a picked frame -> upload iff `seq != lastUploadedSeq`
   (b1's rule, structural), `lastShownPts`; no pick and a frame was shown before -> return the texture (HOLD; count
   `video_late_frames` when `playing && |clock - lastShownPts| > 1.5 / fps`: steady 30 fps on 120 Hz shows each frame
   3 extra times legitimately and is NOT late); no pick and NOTHING shown yet -> return 0 with `*pending = true`
   (`video_pending_frames`, `videos_pending`), the compositor holds the layer's last picture / nothing (F9) and
   `notePendingImage` feeds the render_frame gate (C3). C1: `incomingImagePending` includes Video through a
   generalised `mediaPendingFn_` (Renderer: video -> `player->neverShown()`, sequence -> `firstFramePending()`).
   Because `open()` decodes frame 0 into slot 0 (gen 0), a fresh trigger without a seek is never pending; pending
   happens only for a broken first frame (today: a black upload) or a seek before the first draw (beat snap on a
   first trigger: the hold is the catch-up, the picture nothing / the layer's previous picture -- the renderleft
   feel-call, C5). REJECTED: showing frame 0 as a stand-in during a first-trigger seek (a wrong time position
   flashing before the jump -- renderleft R1-a rejected wrong-picture stand-ins).
R-7 Rule 15 on return = a HOLD of the last shown frame for one seek + catch-up (R-5), never nothing; the GOP-250
   worst case (249 frames) is 0.55 s at 1080p / 2.2 s at 4K (INFERRED from F18's loop rate; NONREF and thread_count
   are the levers). REJECTED: decoding off screen to keep the ring warm (B2 says no decode; 16 off-screen 4K players
   would cost ~4 core-s/s); a periodic off-screen re-position (still decoding).
R-8 Seeks stay requests through the existing `seekTo` (message thread and the GL out-point) -- no API change for
   `MainComponent.cpp` (F11); the GL thread applies them (clock + gen), the thread serves them. A seek's landing frame
   is the first decoded frame with pts >= target - 0.5 / fps (today's `:460` rule).
R-9 Close / retire without a join on a hot thread: `close()` = `open_ = false`, `signalThreadShouldExit()`, `notify()`,
   return (a never-started thread: free the contexts inline, as today). `drainRetiredMedia` calls `releaseGL()` on
   every retired player each frame (the texture goes now, GL thread) but DESTROYS only those with `threadDone()`; the
   rest stay in `retiredVideoPlayers_` for the next frame. Bounded: a thread is inside at most one `av_read_frame` +
   one decode (<= 9 ms 4K, <= 15 ms ProRes max measured) when signalled. `~VideoPlayer` (`stopThread(3000)`) is
   reached with a finished thread except at shutdown (`~Renderer`, message thread: <= one decode per player; the
   quit-time check is a report row, B4 pattern). A context close (`openGLContextClosing`) drains as today; a player
   whose thread is still exiting keeps its object (texture already 0) and is destroyed by a later drain -- its
   `releaseGL` is then a no-op, so no cross-context delete (the Renderer.h guard holds).
R-10 `videoPlayerMutex_` guards the map lookup only (an O(1) find, `Renderer.cpp:1577-1582` becomes a scoped block);
   the raw pointer is safe for the rest of the frame because a player is destroyed only by `drainRetiredMedia` on
   this thread (R-9). Message-thread waits (F7) fall to the lookup (~1 µs; `msg_video_lock_wait_max_ms` proves it).
   `ffmpegMutex_` is deleted: the thread owns the contexts after `start()`, `open()` runs before it, `close()` never
   touches them.
R-11 Video uploads stay OUTSIDE `ImageTexCache::UploadBudget` (`ImageTexCache.h:257-271`): a video frame is
   time-critical (deferring it = a late frame), bounded by construction (<= 1 `glTexSubImage2D` per drawn player per
   frame into an existing texture: 0.86 ms at 4K, 1.3 at 1080p), and the budget's pump runs at the frame top before
   any video draw, so images are never starved by video. `peak_video_upload_ms` reports it.
R-12 Reverse / ping-pong on a long-GOP file: correct frames at the rate the seek + catch-up allows (holds between),
   instead of today's re-seek + 30 decodes per render frame ON the GL thread (which also shows the wrong frame,
   keyframe + 29, when > 30 frames past). Documented ceiling (rendering.md); a GOP cache for reverse is FILED.
R-13 Levers, in order, each behind a named constant, each with its trigger: `kSkipNonRefInCatchUp` (off; on iff w3b
   hold > bar); `kDecodeThreadPriority` (normal; high iff w2 shows late frames on a quiet machine); FFmpeg
   `thread_count` (2; 4 iff the 1080p hold in w3 > 0.5 s after NONREF). No lever changes the protocol.
R-14 Memory lever (FILED, not built): trim an idle player's slots 1-2 after 30 s off screen. Named because 16 x 4K all
   drawn = 1.6 GB RSS on a 32 GiB machine (R-3 arithmetic, INFERRED).
R-15 Counters first (seqvram R-9): commit 1 instruments the OLD code so every row is RED by value on that build and by
   absence on main; commit 3 replaces the instrumentation points, the fields stay. Fields go in BOTH servers'
   `/api/state` (the image_* / seq_* precedent, additive) -- no new endpoint anywhere (the rig's TEST_SERVER-only rule
   is about endpoints; the probe runs in production mode on 7070 like probe-image-load).
R-16 Thumbnail at `open()`: a second small sws context (`SWS_AREA`, w x h -> fit 90x72) converts the first decoded
   frame straight to a <= 90x72 RGBA buffer, copied into a `juce::Image` (<= 6480 `setPixelColour` calls, ~0.1 ms);
   `getThumbnail(maxW, maxH)` returns it (rescaled only if a caller ever asks another size). Race gone (F6); the
   4K thumbnail's 31.9 ms becomes ~2 ms (INFERRED) -- part M's S2 (b) is pre-empted here because `frameBuffer_` no
   longer exists; section 5 fences it.

## 3. TRADEOFFS CONSIDERED (rejected)
- b1 + b2 only: fails m1 at 4K x 4 (46 fps measured, F18) and keeps X1; the hold with a 4 ms-per-call bound is longer
  than the thread's (R-1).
- A shared pool with per-player serial queues: adds scheduling and fairness logic for no gain -- the FFmpeg context
  is per player anyway (R-2).
- Ring depth 2: the writer blocks whenever the reader has not consumed yet; depth 3 is the VideoRecorder precedent
  and bounds look-ahead to 2 frames. Depth 4+: 33 MB more per 4K player for no visible gain.
- Keeping the shown slot in CPU memory (re-upload after context loss without a decode): costs a fourth slot or halves
  the look-ahead; context loss is rare and the layer holds its last picture for the 1-2 frames until the next decode
  lands (renderleft R1-f accepted the same).
- Flipping on the decode thread (row swap) instead of the negative stride: 0.39 / 1.73 ms of pure copy per frame; the
  stride is free and VERIFIED (F14). Kept as the one-line fallback if a codec path ever rejects it.
- A PBO / shared-context upload: renderleft rejected both (unprovable on the macOS driver without a debugger).
- Video uploads inside the UploadBudget: a deferred video frame is a late frame (R-11).
- Showing frame 0 while a first-trigger seek catches up: a wrong-time picture (R-6).
- Off-screen decoding to make a deck return instant: against B2, ~4 core-s/s for a 4K deck (R-7).
- A detached thread that outlives the player: UAF class; deferred destroy is simpler and provable (R-9).
- Joining in `close()` (message thread): <= one decode per drawn player, so a swap with 4 x 4K playing could hold the
  UI ~36 ms; the dispatch asks for joins off the message thread (R-9).
- VideoToolbox / hardware decode: macOS-only; frames land as NV12 / IOSurface and would need a second colour path;
  the ring is where they would be published later (section 5).
- `best_effort_timestamp` instead of `pts`: a behaviour change for streams with odd timestamps; kept as today
  (`decodedFrame_->pts`), named as a lever if a fixture ever shows AV_NOPTS_VALUE frames.

## 4. DECISION / SPEC

### 4.0 Builder step 0 (rig)
- Worktree lane on main @ 5b78d43 (or later: rebase, section 7). Rig rules (.harmony/HANDOFF.md:36-49): `df -h` first,
  the live lock (`LANE=<name> . .harmony/.reports/s-rta-0928/gate-scripts/lock.sh`), `open -g` only, osascript quit,
  >= 45 s between lock holds, no lldb / dtrace / Instruments / sample, no synthetic input, NEVER the Output window,
  `Connection: close`, one live app at a time, >= 5 runs per arm for any flake verdict, never `cd`.
- Build Release (`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`, test server ON as the repo builds it); keep a signed
  copy of the app after EVERY commit's build (main's, commit 1's, 2's, 3's).
- Record on main's app before any change: ctest count (846 at close of s-rta-0928b's start; more if seqvram merged),
  probe-image-load 37/0, probe-crossfade 35/0, probe-render-state / canvas / fitmode / effects-parity / outputs at
  their recorded counts, probe-deck-clock (its `d_video_keeps_time`, `d_return_hitch` REPORT line and
  `d_imageseq_keeps_time` rows -- they are this plan's regression witnesses for rule 15).
- Step-0 rig checks (scratch, `$TMPDIR`): (i) the negative-stride flip program (F14: compile against
  `/opt/homebrew/opt/ffmpeg`; expect "NEGATIVE DST STRIDE FLIPS"); (ii) the `geq` band fixture (F15: a 2 s 320x180
  clip, extract frames 0 / 5 / 37 / 59, decode the codes). Both are also what the probe's fixture step re-asserts.
- Perf rows run only when no clang runs (`wait_no_compiler`) and print `os.getloadavg()`.

### 4.1 Commit 1 -- counters on the OLD code + the probe (RED harness; no behaviour change)
1. NEW `src/media/VideoStats.h` (pure: `<atomic> <cstdint>`; no JUCE, no FFmpeg):
   ```cpp
   // s-rta-0928b video: counters for /api/state (relaxed atomics; "take*" = reset on read). Owned by Renderer, shared by
   // every VideoPlayer through a pointer (nullptr = no stats, as in ctests).
   struct VideoStats {
       std::atomic<int>      players{0}, threadsRunning{0}, pendingNow{0};
       std::atomic<int64_t>  glDecodeCalls{0}, uploads{0}, framesDecoded{0}, framesDropped{0}, framesSkipped{0},
                             seeks{0}, lateFrames{0}, pendingFrames{0}, holdFrames{0};
       std::atomic<int>      glMaxDecodesPerCall{0};          // reset on read
       std::atomic<float>    peakUploadMs{0.0f}, msgLockWaitMaxMs{0.0f};   // reset on read
       int   takeGlMaxDecodesPerCall() { return glMaxDecodesPerCall.exchange(0, std::memory_order_relaxed); }
       float takePeakUploadMs()        { return peakUploadMs.exchange(0.0f, std::memory_order_relaxed); }
       float takeMsgLockWaitMaxMs()    { return msgLockWaitMaxMs.exchange(0.0f, std::memory_order_relaxed); }
       void  noteMax(std::atomic<float>& a, float v) { float c = a.load(std::memory_order_relaxed);
                                                       while (v > c && !a.compare_exchange_weak(c, v)) {} }
       void  noteMax(std::atomic<int>& a, int v)     { int c = a.load(std::memory_order_relaxed);
                                                       while (v > c && !a.compare_exchange_weak(c, v)) {} }
   };
   ```
2. `VideoPlayer.h`: `VideoStats* stats_ = nullptr; void setStats(VideoStats* s) { stats_ = s; }` (after :137).
   Instrumentation of the OLD code, each point replaced in commit 3 (all `if (stats_)`):
   - `decodeNextFrame` (:494): `++glDecodeCalls` at entry (it IS the GL thread today), `++framesDecoded` on success;
   - `decodeFrameAtTime` (:448-469): count the loop's decodes, `noteMax(glMaxDecodesPerCall, count)` at every return
     path; `++lateFrames` at the "used up attempts" return (:469) -- the S1b witness;
   - `seekToTimestamp` (:480): `++seeks`;
   - `uploadToTexture` (:346-365): `++uploads` around either GL upload, `noteMax(peakUploadMs, ms)`.
3. `Renderer.h`: `VideoStats videoStats_;` next to `videoPlayers_` (after :589) and `const VideoStats& getVideoStats()
   const`. `Renderer.cpp`: `player->setStats(&videoStats_)` in `openVideoForClip` before the insert (:1450);
   `players` = the map size after insert / erase (:1451, :1493); a `steady_clock` timer around the lock acquisition in
   `getVideoPlayer` (:1537), `getVideoPlayerFile` (:1544) and `closeMediaForClip` (:1486) -> `noteMax(msgLockWaitMaxMs)`
   (these three are the message-thread sites the diag timed).
4. `/api/state`, additive, BOTH servers -- `TestServer.cpp` after :653 (`onset_pulse_frames`), `ApiServer.cpp` after
   :1330 (`master_level`) -- i.e. NOT adjacent to the seqvram lane's insertion points (:645 / :1324):
   ```cpp
   // s-rta-0928b video: video decodes off the GL thread (VideoPlayer decode thread + VideoRing; probe-video). Same
   // fields as <the other server>. gl_video_decode_calls counts avcodec calls made ON the render thread (0 by
   // construction from commit 3); *_max_* / peak_* reset on read.
   {
       auto& v = renderer_.getVideoStats();
       obj->setProperty("video_players", v.players.load());
       obj->setProperty("video_threads", v.threadsRunning.load());
       obj->setProperty("gl_video_decode_calls", (juce::int64) v.glDecodeCalls.load());
       obj->setProperty("gl_video_max_decodes_per_call", const_cast<VideoStats&>(v).takeGlMaxDecodesPerCall());
       obj->setProperty("video_uploads", (juce::int64) v.uploads.load());
       obj->setProperty("video_frames_decoded", (juce::int64) v.framesDecoded.load());
       obj->setProperty("video_frames_dropped", (juce::int64) v.framesDropped.load());   // catch-up: decoded, not converted
       obj->setProperty("video_frames_skipped", (juce::int64) v.framesSkipped.load());   // GL: an older ready frame passed over
       obj->setProperty("video_seeks", (juce::int64) v.seeks.load());
       obj->setProperty("video_hold_frames", (juce::int64) v.holdFrames.load());         // drawn frames that re-showed the last frame
       obj->setProperty("video_late_frames", (juce::int64) v.lateFrames.load());         // ... while the clock had passed the next frame
       obj->setProperty("video_pending_frames", (juce::int64) v.pendingFrames.load());   // nothing shown yet
       obj->setProperty("videos_pending", v.pendingNow.load());                          // this frame
       obj->setProperty("peak_video_upload_ms", (double) const_cast<VideoStats&>(v).takePeakUploadMs());
       obj->setProperty("msg_video_lock_wait_max_ms", (double) const_cast<VideoStats&>(v).takeMsgLockWaitMaxMs());
   }
   ```
   (Give `getVideoStats()` a non-const overload instead of the casts if the builder prefers; the fields are the
   contract.) `videos_pending` is reset at the frame top: `videoStats_.pendingNow.store(0)` right after
   `uploadBudget_.reset();` (`Renderer.cpp:304`; the seqvram lane's frame-top line goes after :311 -- 7 lines apart).
5. The probe files (section 4.6) are committed here with their thresholds; RED runs recorded verbatim on (a) main's
   app (fields absent -> every row FAILs by absence; fps / callback / trigger-RTT lines FAIL by value), (b) the
   commit-1 app (RED by VALUE: the table in 4.6).
   Gate: build; ctest count unchanged; probe-image-load / crossfade / deck-clock GREEN at their counts (fields only).

### 4.2 Commit 2 -- b1: upload only a newly decoded frame (behaviour: fewer uploads, same pictures)
`VideoPlayer.h`: `bool newFrame_ = false;` next to `frameReady_` (:133). `convertFrameToRGBA` (:531) sets
`newFrame_ = true` after the flip; `open()`'s first frame too (:179). `uploadToTexture` (:338-368): after the
`frameReady_` check, `if (textureCreated_ && !newFrame_) return texture_;` then `newFrame_ = false` before the upload.
This is the diag's v2 counterfactual made permanent (`instr_vp.py:49-52`). Gate: w1 (d) GREEN (uploads ~600 per 5 s),
w1 (a) still RED (~104 fps), w2 still RED (~46 fps), w6 (b) GREEN; pictures byte-identical (4.7 BYTE-IDENTITY).

### 4.3 Commit 3a -- `src/media/VideoRing.h` (NEW, header-only, pure: `<atomic> <cstdint> <cmath> <array>`) + ctests
```cpp
// s-rta-0928b video: the lock-free frame ring between a VideoPlayer's decode thread (one writer) and the GL thread (one
// reader), and the pure catch-up policy. No FFmpeg, no GL, no JUCE: tests/test_video_ring.cpp drives it headless.
namespace VideoRing {
enum class SlotState : uint8_t { Free, Writing, Ready, Reading };
struct SlotHeader { std::atomic<uint8_t> state{ (uint8_t) SlotState::Free }; double pts = -1.0; uint32_t gen = 0;
                    uint64_t seq = 0; };
template <int N = 3> class Ring {                       // slot BYTES live in VideoPlayer (allocated at open); this is the protocol
public:
    int  acquireWrite();                                // writer: CAS Free -> Writing on the first Free slot; -1 = none
    void publish(int slot, double pts, uint32_t gen, uint64_t seq);   // writer: fields, then state.store(Ready, release)
    void abandon(int slot);                             // writer: Writing -> Free (a dropped frame after a late decision)
    struct Pick { int slot = -1; double pts = -1.0; uint64_t seq = 0; int skipped = 0; int staleDropped = 0; int readyAhead = 0; };
    Pick pick(double clock, uint32_t gen, double tolSec);   // reader: newest Ready with gen == gen && pts <= clock + tol ->
                                                            // Ready -> Reading; stale-gen Ready -> Free; same-gen Ready with
                                                            // pts <= chosen.pts -> Free (skipped); Ready ahead of the clock stay
    void release(int slot);                             // reader: Reading -> Free (no-op unless Reading -- H1 rule: defined, no assert-only path)
    int  readyCount() const; int freeCount() const;
    const SlotHeader& header(int i) const;
};
enum class Step : uint8_t { Decode, Reseek };
struct Policy { double reseekBehindSec = 0.1; double reseekAheadSec = 2.0;     // VideoPlayer.cpp:441 today
                double dropBehindFrames = 1.5; int fullDecodeFrames = 10; bool skipNonRefInCatchUp = false; };
inline Step decide(double want, double newestPts, bool haveNewest, const Policy& p)
{ return (!haveNewest || (want >= newestPts - p.reseekBehindSec && want <= newestPts + p.reseekAheadSec)) ? Step::Decode : Step::Reseek; }
inline bool shouldPublish(double framePts, double want, double frameDur, const Policy& p)
{ return framePts >= want - p.dropBehindFrames * frameDur; }
inline bool useSkipNonRef(double framePts, double want, double frameDur, const Policy& p)
{ return p.skipNonRefInCatchUp && (want - framePts) > p.fullDecodeFrames * frameDur; }
enum class Shown : uint8_t { New, Held, Late, Pending };
inline Shown judge(bool picked, bool shownBefore, bool playing, double clock, double lastShownPts, double frameDur)
{ if (picked) return Shown::New; if (!shownBefore) return Shown::Pending;
  return (playing && std::fabs(clock - lastShownPts) > 1.5 * frameDur) ? Shown::Late : Shown::Held; }
}
```
Memory model: `publish` writes pts/gen/seq then `state.store(Ready, memory_order_release)`; `pick` reads
`state.load(acquire)` before the fields and CASes with `acq_rel`; the slot BYTES follow the same pair (written before
publish, read after the CAS). `acquireWrite` CASes `Free -> Writing` with `acq_rel` so the writer never touches a slot
the reader still uploads from.
NEW `tests/test_video_ring.cpp` (Catch2 only, registered at the EOF of `tests/CMakeLists.txt` like `test_frame_ring`:
`Catch2::Catch2WithMain`, `apply_sanitizers`, `catch_discover_tests`; tag `[video_ring][s-rta-0928b]`). Cases:
 (1) publish pts 0.000 / 0.033 / 0.066 gen 1; pick(0.040, 1, 0.0167) -> the 0.033 slot (Reading), the 0.000 slot Free
     with skipped == 1, the 0.066 slot still Ready (readyAhead == 1).
 (2) stale gen: three Ready gen 1; pick(any, 2) -> slot -1, staleDropped == 3, all Free.
 (3) nothing at or before the clock: pick -> -1, nothing freed, readyAhead == 3.
 (4) acquireWrite with no Free slot -> -1; after release(picked) -> a slot.
 (5) a Reading slot is never returned by acquireWrite and is untouched by a second pick.
 (6) tolerance: pts == clock + tol - 1e-9 is picked; pts == clock + tol + 1e-9 is not (mirrors `:431`).
 (7) release on a Free / Ready / Writing slot is a no-op (defined behaviour); abandon on a Writing slot -> Free.
 (8) decide(): behind by 0.11 -> Reseek; ahead by 2.01 -> Reseek; within -> Decode; haveNewest false -> Decode.
 (9) shouldPublish: pts < want - 1.5 fd -> false, else true; useSkipNonRef: false while disabled; enabled -> true iff
     want - pts > 10 fd.
 (10) judge(): picked -> New; not picked, never shown -> Pending; shown, playing, |clock - last| > 1.5 fd -> Late;
      shown, paused or within -> Held.
 (11) stress (`std::thread` writer + reader, 200,000 frames, pts = k / 30, gen bumped every 997 frames by the
      reader-side "seek" through an atomic the writer reads): the writer stamps each slot's 64-byte fake payload with
      seq; the reader verifies payload == header.seq after every pick (a torn slot fails), asserts picked pts is
      non-decreasing within a gen, an atomic owner tag per slot is never held by both sides, and every slot ends
      Free. Run once more in a separate `-DADNA_SANITIZE=thread` build dir (record the run; the default build keeps
      sanitizers off).
 Teeth, one mutated copy each (record the FAIL line, restore, `sleep 1; touch`): pick accepting a stale gen -> (2)
 and (11) FAIL; pick not freeing older same-gen frames -> (1) FAIL; acquireWrite returning a Reading slot -> (5) and
 (11) FAIL; the tolerance sign flipped -> (6) FAIL; decide's thresholds swapped -> (8) FAIL; judge's Late test using
 0.5 fd -> (10) FAIL.

### 4.4 Commit 3b -- `VideoPlayer` on the ring + its decode thread (`src/media/VideoPlayer.h/.cpp`)
Public API (message thread unless noted; every existing caller compiles unchanged except the two named):
```cpp
bool  open(const juce::File&);             // contexts, sws, the ring's 3 slots, frame 0 -> slot 0 (gen 0, seq 1), the thumbnail. NO thread.
void  start();                             // Renderer::installVideoPlayer: starts the decode thread (juce::Thread, Priority::normal)
void  close();                             // signals the thread + notify, open_ = false, returns at once; never-started -> frees inline
bool  threadDone() const;                  // no decode thread runs (never started, or exited) -- drainRetiredMedia's destroy gate
void  setStats(VideoStats*);
// transport: setSpeed / getSpeed / setReverse / getReverse / setLoopMode / getLoopMode / setPlaying / isPlaying / seekTo /
// getPlayheadPosition / getWidth / getHeight / getDuration / getFrameRate / getTotalFrames / isOpen / hasAlpha / getFile: UNCHANGED
void  advanceFrame(double dt);             // GL: seek -> clock + gen++; advanceTransport; want = clock; draw stamp; notify (R-4)
void  advanceClock(double dt);             // GL: seek -> clock + gen++; advanceTransport; no want, no notify (rule 15)
GLuint uploadToTexture(bool* pending);     // GL: pick + upload-if-new; hold; pending = never shown. (was uploadToTexture())
bool  neverShown() const;                  // GL thread: no frame uploaded yet (open && texture_ == 0) -- the C1 provider reads it
void  releaseGL();                         // GL: deletes the texture; lastUploadedSeq_ = 0 (the next pick re-uploads)
juce::Image getThumbnail(int maxW, int maxH);   // message thread: the image made in open() (rescaled only if the size differs)
```
Private state: `std::array<uint8_t*, 3> slotBytes_` + `VideoRing::Ring<3> ring_`; `std::atomic<double> wantTime_`;
`std::atomic<uint32_t> gen_`; `std::atomic<int64_t> lastDrawMs_`; `std::atomic<bool> exitRequested_` (juce's
`threadShouldExit` covers it), `threadDone_{true}`; GL-only: `uint64_t lastUploadedSeq_ = 0; double lastShownPts_ =
-1; bool releasedThisFrame_`; thread-only: the FFmpeg contexts, `swsCtx_`, `double newestPts_`, `uint32_t myGen_`,
`uint64_t seq_`, `bool catchUp_`; `juce::Image thumbnail_` (message thread); `struct DecodeThread : juce::Thread`
member with a back-pointer (`run()` -> `owner.decodeLoop()`). DELETED: `frameBuffer_`, `frameBufferWidth_/Height_`,
`frameReady_`, `newFrame_`, `rgbaFrame_`, `ffmpegMutex_`, `decodeFrameAtTime`, `convertFrameToRGBA`.
`open()` (message thread): as today up to the sws context (:157-165); then `rowBytes_ = width_ * 4`, three slots of
`width_ * height_ * 4` bytes (`std::malloc`; page-lazy), the first frame: `decodeNextFrame()` -> `convertInto(slot 0)`
-> `ring_.publish(0, pts0, 0, ++seq_)` -> the thumbnail from `decodedFrame_` (R-16); `newestPts_ = pts0`;
`currentTime_ = 0`, `playing_ = true`, `open_ = true` (as today). A failed first decode publishes nothing (the player
is pending until the thread lands a frame; today it uploaded zeros).
`convertInto(slot)`: `uint8_t* dst[4] = { slotBytes_[slot] + (height_ - 1) * rowBytes_, nullptr, nullptr, nullptr };
int dstStride[4] = { -rowBytes_, 0, 0, 0 }; sws_scale(swsCtx_, decodedFrame_->data, decodedFrame_->linesize, 0,
height_, dst, dstStride);` -- bottom-up in one pass (F14). No allocation.
`decodeLoop()` (the thread; R-5 verbatim, in code order):
```cpp
VideoRing::Policy pol; pol.skipNonRefInCatchUp = kSkipNonRefInCatchUp;   // constexpr false (R-13 lever)
while (!thread_.threadShouldExit()) {
    if (nowMs() - lastDrawMs_.load() > kIdleMs /*250*/) { thread_.wait(-1); continue; }      // rule 15: off screen = idle
    const uint32_t g = gen_.load(std::memory_order_acquire); const double want = wantTime_.load();
    if (g != myGen_ || VideoRing::decide(want, newestPts_, haveNewest_, pol) == VideoRing::Step::Reseek)
    { myGen_ = g; seekToTimestamp(want); haveNewest_ = false; catchUp_ = true; if (stats_) ++stats_->seeks; }
    codecCtx_->skip_frame = (haveNewest_ && VideoRing::useSkipNonRef(newestPts_, want, frameDur_, pol)) ? AVDISCARD_NONREF : AVDISCARD_DEFAULT;
    if (!decodeNextFrame()) { if (!drained_) { drainDecoder(); continue; } thread_.wait(20); continue; }   // EOF: the wrap's gen bump re-seeks
    const double pts = decodedFrame_->pts >= 0 ? decodedFrame_->pts * timeBase_ : want;   // no pts -> take it (today's :463-466)
    if (stats_) ++stats_->framesDecoded;
    if (!VideoRing::shouldPublish(pts, wantTime_.load(), frameDur_, pol)) { if (stats_) ++stats_->framesDropped; continue; }  // no sws
    int s; while ((s = ring_.acquireWrite()) < 0 && !thread_.threadShouldExit()) thread_.wait(20);   // ring full: the reader frees + notifies
    if (s < 0) break;
    convertInto(s); ring_.publish(s, pts, g, ++seq_); newestPts_ = pts; haveNewest_ = true; catchUp_ = false;
}
freeFfmpeg();                       // the thread owns the contexts: nobody else touches them after start()
threadDone_.store(true, std::memory_order_release); if (stats_) --stats_->threadsRunning;
```
(`wantTime_` is re-read for the drop decision so a moving clock during a long catch-up is chased; `frameDur_ = 1 /
frameRate_`; `drainDecoder()` = `avcodec_send_packet(codecCtx_, nullptr)` then receive until `AVERROR_EOF`, publishing
through the same drop / acquire / publish path, then `drained_ = true`; `seekToTimestamp` resets `drained_`.)
`advanceFrame(dt)`: `if (seekRequested_) { clock = target * duration_; playhead store; gen_.fetch_add(1, release);
seekRequested_ = false; }` (the request's decode at :237-238 is gone); `advanceTransport(dt)` with the two
`seekToTimestamp` calls (:301, :319) replaced by `gen_.fetch_add(1, release)`; `wantTime_.store(currentTime_)`; `const
auto now = nowMs(); const bool wake = releasedThisFrame_ || genChanged || now - lastDrawMs_ > 100; lastDrawMs_ = now; if
(wake) thread_.notify(); releasedThisFrame_ = false;`. `advanceClock(dt)`: the same minus want / stamp / notify.
`uploadToTexture(bool* pending)`: `if (pending) *pending = false; if (!open_) return texture_;` (hold after close, as
today's :340-341); `auto p = ring_.pick(currentTime_, gen_.load(acquire), 0.5 * frameDur_)`; stats `framesSkipped +=
p.skipped`; if `p.slot >= 0`: `if (p.seq != lastUploadedSeq_) { upload slotBytes_[p.slot] (glTexImage2D once /
glTexSubImage2D, today's :346-365 verbatim); ++uploads; peakUploadMs; lastUploadedSeq_ = p.seq; }`, `lastShownPts_ =
p.pts; ring_.release(p.slot); releasedThisFrame_ = true;` else `switch (judge(false, texture_ != 0, playing_,
currentTime_, lastShownPts_, frameDur_))`: `Held -> ++holdFrames`, `Late -> ++holdFrames, ++lateFrames`, `Pending ->
++pendingFrames, ++pendingNow, *pending = true`. Return `texture_` (0 when never uploaded). `neverShown() = open_ &&
texture_ == 0`.
`close()`: `open_ = false; if (thread_.isThreadRunning()) { thread_.signalThreadShouldExit(); thread_.notify(); } else
freeFfmpeg();` (a never-started player still owns its contexts and frees them inline, as today; a finished thread has
already freed them -- `freeFfmpeg` is idempotent). `~VideoPlayer`:
`thread_.stopThread(3000)` (finished already except at shutdown), `freeFfmpeg()` if still owned, free the slots,
`releaseGL()` (no-op after a drain). `start()`: `threadDone_ = false; ++threadsRunning; thread_.startThread(
kDecodeThreadPriority)`; on failure `threadDone_ = true` and a `[VideoPlayer]` log line (the player then shows frame 0
only).
Sacred Rule 2 discussion (explicit): the ring is a new lock-free channel (atomics + CAS, one writer / one reader); the
only lock the GL thread touches is `juce::WaitableEvent`'s internal mutex inside `Thread::notify()` -- uncontended,
O(1), held for nanoseconds, the same class renderleft R-5 accepted for `ThreadPool::addJob`; `notify()` is called at
most once per drawn player per frame (R-4). No mutex is held across a decode, an upload or an I/O anywhere.

### 4.5 Commit 3b -- `Renderer` + `CompositorEngine` wiring (exact lines at 5b78d43)
`Renderer.h`:
- after :213 (`openVideoForClip`): `bool installVideoPlayer(uint32_t clipId, std::unique_ptr<VideoPlayer> player);   //
  s-rta-0928b: retire the old id, insert, start the decode thread. The seam for an asynchronous open (part M): open()
  on a pool thread, then this on the message thread.`
- after :589: `VideoStats videoStats_;` (commit 1) -- nothing else near the seqvram lane's :286.
- :617-627 comments: `syncMedia`'s "decode = true is the on-screen path (decode + upload...)" -> "(a frame request to
  the player's decode thread + the upload of the newest ring frame <= its clock; the GL thread never decodes)";
  `pending` sentence gains "or a video that has never shown a frame".
`Renderer.cpp`:
- `openVideoForClip` (:1432-1453) becomes `auto player = std::make_unique<VideoPlayer>(); player->setStats(
  &videoStats_); if (!player->open(videoFile)) return false; return installVideoPlayer(clipId, std::move(player));`
  and `installVideoPlayer` = today's :1448-1452 (`closeMediaForClip(clipId)`, lock, insert, `players` count) +
  `player->start()` AFTER the insert (the GL thread may already draw it -- a draw before `start()` shows frame 0).
- `syncMedia` video branch :1577-1582 -> the scoped lookup (R-10); :1584-1616 unchanged; :1618-1621 unchanged calls
  (`advanceFrame` / `advanceClock` -- their meaning changed inside VideoPlayer); :1622-1640 unchanged; :1642 ->
  ```cpp
  if (!decode) return 0;
  bool videoPending = false;
  const GLuint tex = player->uploadToTexture(&videoPending);   // picks the newest ring frame <= the clock; uploads only a new one; never waits
  if (videoPending) { compositor_.notePendingImage(); if (pending != nullptr) *pending = true; }   // C3: the gate's counter (as the sequence branch :1721-1726)
  return tex;
  ```
- `drainRetiredMedia` (:1509-1533), the video half: after swapping the lists out, `for (auto& p : videoToRetire)
  p->releaseGL();` then partition: `threadDone()` -> destroyed by scope exit; not done -> pushed back into
  `retiredVideoPlayers_` under `retiredMediaMutex_` (next frame re-checks). The `if (both empty) return;` early-out
  stays. Comment: "a player is destroyed only once its decode thread has exited (R-9); its texture is released at
  once". The seqvram lane ADDS a function after :1533 -- this edit stays inside :1518-1527 (a clean 3-way merge).
- `closeMediaForClip` (:1486-1494): unchanged text (`close()` now returns at once; the comment at :1476-1484 gains one
  sentence: "close() only signals the decode thread; the FFmpeg contexts are freed by the thread itself").
- `openGLContextClosing` (:1120-1125, :1145): unchanged.
- the C1 provider (:267-271): `setMediaPendingProvider([this](const Clip* clip) -> bool { if (clip->mediaType ==
  Clip::MediaType::Video) { std::lock_guard<std::mutex> lock(videoPlayerMutex_); auto it = videoPlayers_.find(clip->id);
  return it != videoPlayers_.end() && it->second->neverShown(); } /* sequence: as today */ })`. GL thread; the lock is
  the O(1) lookup; `neverShown` is GL-thread state.
`CompositorEngine.h:122-123`: `SequencePendingFn` -> `MediaPendingFn`, `setSequencePendingProvider` ->
`setMediaPendingProvider` (one caller: `Renderer.cpp:267`); :113-116 comment: "pending (R1.4 / s-rta-0928b video): an
image sequence with nothing to show yet, or a video that has never shown a frame". `CompositorEngine.cpp:383-395`
(`incomingImagePending`): the type test admits `Video` (:385-386); `:393-394` -> `if (mediaType == ImageSequence ||
mediaType == Video) return mediaPendingFn_ && mediaPendingFn_(clip);`. No other compositor change: the hold at
:1094-1107 / :1204-1214 already handles `pending` from `videoFrameFn_`.
`MainComponent.cpp`: NO edit (F6 / F11: `getThumbnail(90, 72)`, `seekTo`, `getVideoPlayer`, `openVideoForClip` keep
their signatures and semantics).

### 4.6 Live probe: `.harmony/probe-video.sh` + `.py` + `.json` (committed in commit 1)
`.sh` = `probe-image-load.sh`'s scaffolding verbatim (F17) with `VIDEO_APP` / `VIDEO_PY` / `VIDEO_ENV`, out dir
`video.XXXXXX`, production mode (7070), the foreign-traffic check, `rm -rf "$OUT/media"` after the quit; ADDITIONALLY
REFUSES when `ffmpeg` / `ffprobe` are not on PATH (probe-deck-clock.sh:15 precedent). Fixtures are encoded BEFORE the
launch (`--make-fixtures`; the 4K encode is the long pole: `-preset veryfast`, ~40-90 s, printed). `.py` copies
probe-image-load.py's helpers (`load / trig / state / comp_state / wait_active / cap / dbox / Poller /
wait_no_compiler`; the two probes stay independent files) and adds: `switch(deck)` (`/api/switch_deck`, the deck-clock
probe's), `code(cap, bandRows)` (threshold each of 10 cells' inner 50 % at 128 -> the frame number; F15),
`playhead(li)` (`/api/composition` -> the layer's active clip `playheadPosition`, read RIGHT AFTER the capture),
`expected(p) = int(p * 300)`, `settle_late()` (poll `video_late_frames` until its delta over 200 ms is 0, <= 4 s: the
catch-up ended -- a capture during a hold is a legitimately held frame, so correctness rows capture only settled),
`fixture_frame(name, k)` (extract frame k of a fixture on demand: `ffmpeg -i f -vf select='eq(n\,k)' -frames:v 1`).
Every video clip JSON: `{"mediaType": 2, "mediaFile": p, "speed": 1.0, "transportMode": 0, "loopMode": 0, "reverse":
false, "inPoint": ip, "outPoint": 1.0, "effects": []}` (F12: `speed` is mandatory). Layers `transitionSpeed 0` unless
a row says otherwise; the canvas = the clip's size (Stretch -> 1:1, so the band sits in the capture's bottom rows).
Fixtures (`$OUT/media`, ffmpeg `-threads 2`, nice'd; 10 s, 30 fps, 300 frames; the band = the bottom `bandRows` rows
(64 at 1080p, 128 at 4K), 10 cells, cell c = 235 iff bit c of the code, chroma 128; F15's `geq` expression):
- `a1080_g250.mp4`: testsrc2 1920x1080, code N, `libx264 -preset veryfast -crf 20 -g 250 -keyint_min 250
  -sc_threshold 0 -pix_fmt yuv420p` -- keyframes 0.000 and 8.333 s (the .py asserts the ffprobe list; a mismatch
  FAILs the fixture step);
- `b1080_g250.mp4`: as A with code N + 512 (bit 9 = "B");
- `g30_1080.mp4`: as A with `-g 30 -keyint_min 30` (the control);
- `a4k_g250.mp4`: 3840x2160, code N, GOP 250;
- `pr1080.mov`: `-c:v prores_ks -profile:v 2 -pix_fmt yuv422p10le`, code N (intra-only: every frame a keyframe);
- `warm.png`: 256x144 flat (deck 1 for the return row).
`.json`: `fpsMin 110`, `p90CallbackMaxMs 8.3`, `peakMaxMs 16.7`, `uploads5s [540, 660]`, `lateMaxRetrigger1080 60`,
`lateMaxRetrigger4k 200`, `lateMaxReturn 60`, `codeTol 2`, `boxTol 3.0`, `lockWaitMaxMs 2.0`, `triggerRttMaxMs 30`,
`bandRows {1080: 64, 4k: 128}`, `keyframesG250 [0.0, 8.333]`, layer ids 91-106, the fixture recipes (arg arrays with
`@OUT@`). Perf rows wait for no compiler and print the load average and the app's `%cpu` (`ps -o %cpu`).

| row | steps | PASS | RED predicted: main / commit-1 app / commit-2 app / final |
|---|---|---|---|
| w5_correctness_gop30 (guard) | canvas 1080p; layer 99 col 0 = g30. load; trig; 2 s; caps at +0 / +1 / +2 s, playhead read right after each. | each `code == expected +- 2`; `dbox(cap, fixture_frame(code)) <= 3.0` (a decoded picture, not a stale slot); `video_late_frames` delta over the 2 s == 0. | main: codes PASS (the synchronous path is frame-accurate), late FAILs by absence / commit-1: PASS / final: PASS -- a guard that frame accuracy survives the ring. |
| w1_steady_1080x4 | canvas 1080p; layers 91-94 col 0 = a1080 (4 players, one file). load; trig x4; 2 s; quiet; s0; Poller 5 s; s1. | (a) median fps over the polls >= 110; (b) p90 of `peak_callback_ms` polls <= 8.3; (c) `gl_video_decode_calls` delta == 0; (d) `video_uploads` delta in [540, 660] (4 x 30 x 5); (e) `video_late_frames` delta == 0. Prints load, %cpu, decode calls, uploads. | main: (a) 93.2, (b) 9.13, (c)(d)(e) absent / commit-1: (a)(b) same, (c) ~600, (d) ~2400 / commit-2: (a) ~104, (c) ~600, (d) GREEN ~600 / final GREEN (INFERRED: v1 arm 117.2 fps with per-frame uploads kept). |
| w2_steady_4kx4 (m1) | canvas 4K; layers 91-94 = a4k. As w1. | (a) fps >= 110; (b) p90 <= 8.3; (c) decode calls delta 0; (d) uploads in [540, 660]; (e) late delta <= 12. If (e) fails on a quiet machine in >= 3 of 5 runs: STOP, report the late distribution (a design finding, never a re-threshold; H9 pattern). | main: (a) 36.0, (b) 81.7 / commit-1: same + (c) ~600, (d) ~2400 / commit-2: (a) ~46 / final GREEN (INFERRED: v1 95.9 fps with FOUR uploads per frame; now ~1). |
| w3_retrigger_midgop_1080 (m2) | canvas 1080p; layer 95 col 0 = a1080, `inPoint 0.5` (5.0 s = frame 150, 150 past keyframe 0). trig; 3 s; quiet; capPre + playhead (code ~240); s0; RETRIGGER the same column (seekTo(inPoint), F11); Poller 3.0 s; s1; settle_late; cap + playhead. | (a) every `peak_callback_ms` poll <= 16.7; (b) `gl_video_max_decodes_per_call` <= 2; (c) `video_late_frames` delta <= 60 (prints the hold in ms = late x the median frame interval); (d) `code(cap) == expected +- 2` and 150 <= code < 300 (resumed from the in-point, not from 0); (e) capPre code in [225, 255]. | main: (a) 47-48 polls of 67-69 ms, (b)(c) absent / commit-1: (a) same, (b) 30, (c) 47-48 (passes (c): the RED is (a)(b)) / commit-2: as commit-1 / final GREEN (INFERRED hold 150 x 1.5-2.2 ms = 0.23-0.33 s = 27-40 frames at 120 Hz). |
| w3b_retrigger_midgop_4k | as w3 at 4K (layer 96, a4k); plus capHold at +0.3 s after the retrigger (inside the hold). | (a)(b)(d)(e) as w3; (c) late delta <= 200 (if > 200: set `kSkipNonRefInCatchUp = true` (R-13), rebuild, re-run, report both numbers); (f) NEVER BLACK, NEVER WRONG: capHold mean luma > 20 and its code is capPre's +- 3 (the held frame) or >= 148 (landed) -- never 29 (keyframe + 29, today's frozen picture), never 0-2 (a frame-0 stand-in). | main: (a) 14 polls of 264-269 ms, (f) code 29 (VERIFIED by the diag: "the picture stays on keyframe + 29") / commit-1 & 2: same / final GREEN (INFERRED hold 150 x 8.8 ms = 1.3 s = ~160 frames). |
| w4_deck_return_1080 (rule 15) | deck 0 layer 97 = a1080 (inPoint 0); deck 1 layer 98 = warm.png. load; trig (0, 97, 0); 2 s; p0 + s0; `switch_deck 1`; 3.5 s (the clock runs to ~5.5 s = frame 165); `switch_deck 0`; Poller 1.5 s; s1; settle_late; cap + p1. Then re-run `probe-deck-clock` and record its `d_return_hitch` REPORT line. | (a) every poll in the return window <= 16.7; (b) `video_pending_frames` delta == 0 (shown before: never nothing); (c) 1 <= late delta <= 60 (deterministic: the frames the clock ran past off screen were never decoded -> a bounded hold on return); (d) `(p1 - p0) x 10 >= 5.0` s (the clock ran off screen and on) and `code(cap) == expected +- 2`. | main: (a) 22 polls of 67-69 ms, (b)(c) absent, (d) PASS / commit-1: (a) same, (c) 22 (passes) / final GREEN; deck-clock `d_return_hitch`: main ~50 ms-class, final <= 16 (its own "target"). |
| w6_crossfade_two_players (Pitfall 35) | canvas 1080p; layer 100 `transitionSpeed 2.0`; col 0 = a1080, col 1 = b1080. trig 0; 2 s; quiet; s0; trig 1; Poller 2.5 s; capMid at +1.0 s; s1 at +2.5 s; settle_late; capAfter + playhead. | (a) max `peak_callback_ms` over the fade <= 16.7; (b) `video_uploads` delta s0 -> s1 in [130, 180] (2 players x 30 x 2 s + B alone 0.5 s + A's frames around the start ~ 150); (c) capMid mean luma > 20 and dbox to A's frame > 3 and to B's frame > 3 (a blend: both chains live); (d) capAfter code has bit 9 set (B) and `(code & 511) == expected +- 2`; (e) `video_pending_frames` delta == 0 (B's frame 0 came from open). | main: (b) absent / commit-1: (b) ~600 (every render frame re-uploads) / commit-2: (b) GREEN / final GREEN. |
| w7_message_thread_no_wait (X1) | canvas 4K; layers 101-104 = a4k `inPoint 0.5`; layer 105 cols 0-1 = warm.png. load; trig 101-104; 2 s; s0; retrigger 101 (the mid-GOP catch-up starts); 5 x trig(105, alternating cols) at 100 ms spacing, each round trip timed; s1. | (a) `msg_video_lock_wait_max_ms` <= 2.0; (b) every trigger round trip <= 30 ms. | main: (a) absent, (b) 68-120+ ms (the diag's 4K lockwaits) / commit-1: (a) 68-120 / final GREEN (INFERRED: a map lookup). |
| w8_prores_steady | canvas 1080p; layer 106 = pr1080. trig; 2 s; quiet; Poller 5 s; settle; cap + playhead. | fps >= 110; decode calls delta 0; late delta 0; `code == expected +- 2`. | main: by absence (fps 118.8 passes) / commit-1: decode calls ~150 / final GREEN. |

Order: w5, w1, w3, w4, w6, w8 (1080p canvas) then w2, w3b, w7 (4K). GREEN = every row PASS on 2 consecutive runs of
the final app on a quiet machine (load printed); RED lines on main, the commit-1 and the commit-2 apps recorded
verbatim in the report; any flake verdict >= 5 runs per arm. The beat-snap path has no row (F13: its seek lands near
phase 0; w3 exercises the mechanism it shares).

### 4.7 What "done" looks like
- ctest: main's count + 11 `[video_ring]` cases green (`ctest -j1`), the TSan run of case (11) recorded, teeth lines
  recorded with the restore sha.
- probe-video: 9 rows GREEN twice on the final app; RED lines recorded on main (absence + fps / polls / RTT), on the
  commit-1 app (decode calls ~600 / 5 s, max decodes per call 30, uploads ~2400 / 5 s, lock wait 68-120 ms, capHold
  code 29) and on the commit-2 app (uploads GREEN, fps 104 / 46 still RED).
- Regressions at their recorded counts: probe-image-load 37/0, probe-crossfade 35/0, probe-render-state, canvas,
  fitmode, effects-parity, outputs, and probe-deck-clock GREEN with its `d_return_hitch` REPORT line <= 16 ms.
- BYTE-IDENTITY (abpix-style S-vid scene, test mode so 7070 + 8080 both run): a1080 in a clip with `"speed": 0.0`
  (the F12 gotcha put to work: frame 0 held) -> `render_frame` arrays `array_equal` main vs lane (the decode + convert
  + upload path produces the same bytes: F14); the S1-S4 image scenes unchanged.
- Quit time (B4 pattern): osascript quit -> process gone, main vs lane, with 4 x 4K playing: the lane adds <= 0.5 s
  (the destructor joins <= one decode per player).
- The report's tables: per-row numbers; the hold table (1080p / 4K retrigger, 1080p return) in frames and ms, with
  and without NONREF if it was enabled; %cpu at 4 x 4K; thumbnail cost per clip (open's `[VideoPlayer] Opened` line
  gains `thumb=X ms`, message thread).

## 5. THE SEAM FOR PART M (asynchronous opens) and the hardware path
- `VideoPlayer::open()` is the whole synchronous prepare -- FFmpeg contexts, sws, the ring's slots, frame 0 into slot
  0, the thumbnail -- and touches ONLY the object it owns: no GL, no Renderer state, no thread. It may therefore run on
  a pool thread as-is. `Renderer::installVideoPlayer(clipId, std::unique_ptr<VideoPlayer>)` (message thread) is the
  hand-over: retire the old id, insert, `start()` the decode thread. Part M's asynchronous open = `open()` on its pool
  + `callAsync -> installVideoPlayer` keyed by (clip id, composition generation) -- its concern, not this lane's.
- Part M must NOT edit `src/media/VideoPlayer.*` (this lane rewrites them) and needs no change here: `getThumbnail`
  is already O(1) after `open()` (R-16 makes its S2 (b) moot: the 4K thumbnail's 31.9 ms is gone), `clipWidth /
  clipHeight / hasAlpha` read as today. If part M keeps `openVideoForClip` as its message-thread fallback, it stays.
- VideoToolbox (not planned; macOS only): the FFmpeg `videotoolbox` hwaccel would run inside the same decode loop;
  a hardware frame is transferred (`av_hwframe_transfer_data`, NV12) and converted into the same slot, so the ring,
  the pick and the upload are untouched. A zero-copy IOSurface -> texture path would bypass the ring and needs a
  per-platform texture path -- a separate plan if ever.

## 6. MUST NOT CHANGE (and how each is kept)
- Transport math: `advanceTransport` (:274-336) byte-identical except `seekToTimestamp` at :301 / :319 -> a
  generation bump; speed / reverse / loop / ping-pong / one-shot / playhead store unchanged. `probe-deck-clock`'s
  `d_video_keeps_time` (INFERRED GREEN: 320x240 GOP 30, the return catch-up is <= 29 tiny frames within its 0.3 s
  settle) is the witness; w4 (d) too.
- `seekTo` (any thread, normalized) and every `MainComponent.cpp` caller (F11): unchanged. The seek's landing rule
  (first frame with pts >= target - 0.5 / fps) and the "close enough" rule (0.5 / fps): unchanged (Policy + tol).
- `syncMedia`'s transport sync (:1584-1616), write-back (:1622-1625) and in/out enforcement (:1627-1640): unchanged
  lines; the out-point `seekTo` still works (a request).
- Rule 15 / DeckClock (:713, DeckClock.h): unchanged; `advanceClock` still ticks off-screen clocks with no decode
  and no upload (B2 verbatim).
- Pitfall 35: two players per fading layer, each with its own thread and ring, keyed by clip id as today
  (`videoPlayers_` unchanged); the compositor's chain keys untouched. w6 is the witness.
- Pitfall 53: the compositor's hold code (:1094-1107, :1204-1214) untouched; video now feeds `pending` through the
  same flag; the render_frame gate (:2333) unchanged and fed through `notePendingImage`.
- Pitfall 52 / captures: `captureFrame` untouched; a video HOLD is not pending (a capture may legitimately show a
  held frame during a catch-up -- testing-eyes.md gains the sentence; probe rows capture after `settle_late`).
- Pitfall 37 / 39 / 3 / 13 / 14 / 20 / 25: no canvas, fit, FBO, EffectChain, ring-buffer or VideoRecorder edit.
- The GL-THREAD DESTROY GUARD (`Renderer.h:595-615`): kept and extended -- a player is still destroyed only in
  `drainRetiredMedia` on the GL thread, now only after its thread has exited; textures are still released there or
  in `openGLContextClosing`.
- `ImageTexCache::UploadBudget` and every image path: untouched (R-11).
- The output tap / IOSurface path, Syphon, the recorder: untouched.
- 7070 / 8080 shapes: `/api/state` fields ADDED (both servers), nothing else; no new endpoint.
- Thumbnail size and callers (90 x 72): unchanged; its pixels come from an sws downscale instead of `Image::rescaled`
  (a documented, invisible-in-tests change).
- No allocation on the GL thread per frame (the slots are allocated in `open()`); no FFmpeg call on the GL thread
  (`gl_video_decode_calls` is the structural witness); no lock held across any decode / upload / I/O.

## 7. FENCES (this lane vs the lanes in flight)
This lane's files: `src/media/VideoPlayer.h/.cpp`, NEW `src/media/VideoStats.h`, NEW `src/media/VideoRing.h`,
`src/render/Renderer.h` (after :213, after :589, comments :617-627), `src/render/Renderer.cpp` (:263-271 provider,
:304 one line, :1432-1453, :1486 / :1537 / :1544 timers, :1518-1527 inside `drainRetiredMedia`, :1577-1582,
:1642), `src/render/CompositorEngine.h:113-123`, `src/render/CompositorEngine.cpp:383-395`, `src/api/ApiServer.cpp`
after :1330, `src/test/TestServer.cpp` after :653, `tests/CMakeLists.txt` EOF, NEW `tests/test_video_ring.cpp`, NEW
`.harmony/probe-video.{sh,py,json}`, docs (section 10).
- seqvram lane (plan-seqvram 4.3 + adoption): `Renderer.cpp:311` (+1 line; ours is at :304 -- 7 lines apart, the
  hunks do not overlap), a NEW function after :1533 (ours edits inside :1518-1527: a clean 3-way merge; if the lane
  inserted INSIDE the function, take both), the sequence branch :1717-1727 (ours ends at :1642), `Renderer.h` after
  :286 (ours after :213 / :589), `ApiServer.cpp` after :1324 / `TestServer.cpp` after :645 (ours after :1330 / :653 --
  separated by the gpu / master / onset lines: adjacent-hunk conflicts at worst, resolve by keeping both blocks in
  either order), `tests/CMakeLists.txt` EOF (both append: keep both blocks, notebook.md:1987-1989),
  `docs/claude/rendering.md` after :71 (both add a paragraph: order image, sequences (seqvram), video), pitfalls "NN"
  (Harmony assigns), the CLAUDE.md index line (both add one; the byte pay in section 10 covers ours). Never touch
  `src/media/ImageSequence.*` or `SeqVram.h`.
- part M lane (media opens, drops, fence black frames, stats; planned in parallel): owns `MainComponent.cpp`'s open
  paths (:2913-2946, :4645-4647, :4882-4891, :6550-6558), `UndoService`, the drop hook. This lane edits NONE of them
  and owns `VideoPlayer.*` (section 5). Shared line: none. Shared field names: none (part M's `peak_message_stall_ms`
  / `frames_without_deck` vs ours `video_*`).
- restore / thumbnails lane (renderleft C6): `ClipCell / LayerStrip / DeckView` -- untouched here.
Rebase before each commit; reviewers check `git diff main..lane -- docs CLAUDE.md` removes nothing it did not mean to
(renderleft lost pitfall lines once; HANDOFF.md:44-46).

## 8. RISKS (strongest counterargument first)
R-A "A per-player thread + a lock-free ring is a subsystem where 30 lines (b1 + b2) would do." It loses on the
   measured facts: v2 (b1) leaves 4 x 4K at 46 fps because decode + sws + flip stay on the GL thread and
   `receive_frame` blocks behind FFmpeg's workers (F18); a time-bounded b2 stretches a 4K mid-GOP hold to ~2.7 s at
   4 ms per frame while still costing those 4 ms; X1 stays. The ring protocol is ~150 lines and is proven headless
   (11 cases, a TSan stress run); the thread loop is ~60 lines of today's policy moved. b1 still ships first, alone.
R-B If commit 3 cannot land (a driver or codec surprise the rows expose), the fallback is b2 in `decodeFrameAtTime`
   (6 lines): remember `pendingTarget_`, do not re-seek while `decodedPts < target` and `decodedPts >= lastKeyframe`,
   bound the loop by a 4 ms `steady_clock` budget instead of 30 frames, keep decoding forward on later calls. It
   bounds S1b (a held picture that catches up over frames) but leaves S1a's 4K collapse and X1 -- say so in the report.
R-C New visible state: a HOLD of the last frame after a mid-GOP retrigger / cue / deck return -- ~0.3 s at 1080p,
   ~1.3 s at 4K with x264's default GOP (INFERRED from F18's loop rate), and a first-trigger beat-snap of a long-GOP
   clip shows nothing / the layer's previous picture for that time. Today the WHOLE output runs at 14 / 3.8 fps for
   3.4 s with the video frozen on a wrong frame. Levers R-13. Boris feel item (C5 pattern): "after a retrigger to a
   mid-clip in-point a long-GOP video holds its last frame for up to ~1 s at 4K instead of freezing the show".
R-D Memory: 3 slots per player = 25 MB (1080p) / 100 MB (4K) once drawn; 16 x 4K all drawn = 1.6 GB RSS vs 530 MB
   today (INFERRED arithmetic; 32 GiB machine). R-14 names the trim.
R-E Lock-free protocol bugs (a torn slot, a double owner) cannot be seen live -- Renderer.cpp / VideoPlayer.cpp are in
   no ctest. Mitigation: the protocol is a pure header driven by 11 cases incl. a two-thread stress run under TSan;
   the live rows check pixels against the clock (w5 / w3 (d) / w4 (d): a torn or stale slot would show a wrong
   code); the "never 29" check (w3b f) catches a stand-in.
R-F `glTexSubImage2D` from client memory copies before it returns (the GL spec without a bound PBO; VERIFIED for the
   spec, INFERRED for the macOS driver) -- the slot is released right after the call. A driver that deferred the read
   would show tearing in w5's dbox / code checks.
R-G A blocking `av_read_frame` on a slow or network volume stalls a decode thread, not the GL thread (a hold); a
   close then leaves the object in the retire list until the read returns (bounded by the OS). Today the same read
   stalls the whole output.
R-H Thread priority `normal` under a CPU storm (a compile) delays decodes -> late frames (holds), never a GL stall.
   `high` (USER_INITIATED) is the lever; the rows print the load and refuse to judge under a compiler.
R-I Variable-frame-rate files: `frameDur_` comes from `avg_frame_rate` (as today's tolerances); a VFR file may show a
   frame one early/late -- unchanged class of behaviour.
R-J Behaviour changes, intended, each documented: the last 1-2 frames of a clip now show (EOF drain); a failed first
   frame is pending (was a black upload); the thumbnail is an sws downscale; reverse / ping-pong on long-GOP files
   shows correct frames at a low rate instead of the wrong frame at full rate (R-12); the Loop wrap has one render
   frame of hold (the re-seek + first decode happen off-thread; today it was in-frame) -- imperceptible at 120 Hz,
   INFERRED; w5 / w1 (e) late == 0 over windows that include wraps would catch a longer one (a1080 wraps every 10 s;
   the 5 s windows may or may not include one -- the report says which).
R-K CLAUDE.md bytes: 518 B free; the index line (~175 B) is paid by trimming rule 5's NOTE (~170 B) so seqvram's line
   fits too; `wc -c CLAUDE.md` <= 25,000 is part of the docs step.
R-L Merge with seqvram / part M: section 7. The one real adjacency is `tests/CMakeLists.txt` EOF and the docs
   paragraphs -- append-only, keep both.
R-M Quit: `~Renderer` joins <= 16 threads x <= one decode; B4-style measurement in 4.7. If a thread is stuck in a
   network read, quit waits for it (as today's message-thread close would).

## 9. COMMIT SEQUENCE (each builds, passes ctest and keeps every existing probe GREEN; each reverts alone)
1. `perf(video): counters for the GL-thread video path (decode calls, max decodes per call, uploads, late frames,
   message-thread lock wait) + probe-video (RED harness)` -- 4.1 + 4.6.
2. `perf(video): upload a video frame only when a new one was decoded (b1)` -- 4.2.
3. `feat(media): VideoRing.h -- the lock-free frame ring and the catch-up policy, headless ctests` -- 4.3.
4. `perf(video): decode off the GL thread -- one decode thread per player feeding a 3-slot RGBA ring (sws with a
   negative stride); the GL thread keeps the clock, picks the newest frame <= it and never waits; seeks are requests;
   close never blocks; the thumbnail is made at open` -- 4.4 + 4.5.
5. `docs(video): rendering.md video paragraph, pitfall NN, CLAUDE.md index (paid), performance-controls B2,
   architecture tree, testing-eyes, effects.md` -- section 10 (in the commit that makes each true is also fine).
6. The report commit (`git add -f` the lane's reports; HANDOFF.md / APP-INVENTORY.md untouched -- Harmony's).
Trailer: the session's attribution line.

## 10. DOCS (text; pitfall as "NN" until Harmony assigns the number)
`docs/claude/rendering.md`, new paragraph after the image-loading paragraph (:71; after the seqvram paragraph if it
merged first):
> **Video playback (s-rta-0928b video)**: no video frame is decoded, converted or flipped on the GL thread.
> `VideoPlayer` runs ONE decode thread per player (`juce::Thread`, normal priority; FFmpeg's own `thread_count` 2
> stays) that feeds a 3-slot RGBA ring (`src/media/VideoRing.h`: lock-free, one writer / one reader; each slot is
> written bottom-up in one pass by `sws_scale` with a negative destination stride, so there is no flip and no
> allocation per frame). The GL thread keeps the transport clock (`advanceTransport`, unchanged), posts the wanted
> time, and in `uploadToTexture` picks the newest ring frame with pts <= clock + half a frame of the current request
> generation, uploading it only when it changed (`glTexSubImage2D`, outside the image `UploadBudget`: bounded by one
> upload per drawn player per frame). A seek (`seekTo` from any thread, the out-point, a cue, a retrigger) and a Loop
> wrap bump the generation; the thread re-seeks to the keyframe and catches up, dropping frames without converting
> them (`video_frames_dropped`) until it reaches the clock; a far jump or reverse play (> 0.1 s behind / > 2 s ahead
> of the newest decoded frame) re-seeks the same way -- reverse or ping-pong on a long-GOP file therefore shows
> correct frames at the rate one seek + catch-up per frame allows (a GOP cache is filed). No frame at or before the
> clock = HOLD the last shown frame (`video_hold_frames`; `video_late_frames` when the clock has passed the next
> content frame), never 0 / no media; a player that has never shown a frame is PENDING (Pitfall NN / 53: the layer
> holds its last picture or draws nothing, a crossfade onto it waits (C1, `incomingImagePending` through the
> media-pending provider), `render_frame` waits (C3)) -- `open()` still decodes frame 0 on the message thread into
> slot 0 and builds the thumbnail there, so pending happens only for a broken first frame or a seek before the first
> draw. Rule 15: an off-screen deck's player advances its clock only (`advanceClock`); its thread idles after 250 ms
> without a draw request and, on return, re-seeks and catches up while the layer holds -- bounded by one GOP of
> decode (~0.3 s at 1080p, ~1.3 s at 4K for 150 frames past a keyframe with x264's default GOP 250; the whole output
> used to run at 14 / 3.8 fps for ~3.4 s with the picture frozen: diag-media S1b). `videoPlayerMutex_` guards the
> `videoPlayers_` lookup only; `close()` signals the thread and returns; the thread frees the FFmpeg contexts itself;
> `drainRetiredMedia` releases a retired player's texture at once and destroys the object only once its thread has
> exited. The last 1-2 frames of a clip now show (the decoder is drained at EOF). `/api/state` (both servers):
> `video_players`, `video_threads`, `gl_video_decode_calls` (0 by construction), `gl_video_max_decodes_per_call`,
> `video_uploads`, `video_frames_decoded` / `_dropped` / `_skipped`, `video_seeks`, `video_hold_frames`,
> `video_late_frames`, `video_pending_frames`, `videos_pending`, `peak_video_upload_ms`, `msg_video_lock_wait_max_ms`.
> Levers (named constants, off by default): `kSkipNonRefInCatchUp` (AVDISCARD_NONREF while > 10 frames from the
> target), the decode thread priority, FFmpeg `thread_count`. Live: `.harmony/probe-video.sh`.
`docs/claude/pitfalls.md`, appended after 53:
> NN. **Video decodes OFF the GL thread -- the render thread owns the clock, picks and uploads; it never calls FFmpeg
> and never waits**: `VideoPlayer` runs one decode thread per player feeding a 3-slot RGBA ring (`src/media/VideoRing.h`,
> lock-free, written bottom-up by `sws_scale` with a negative stride). `advanceFrame` advances the transport clock and
> posts the wanted time (a seek or Loop wrap bumps the request generation; `seekTo` stays a request from any thread);
> `uploadToTexture(&pending)` picks the newest ring frame with pts <= clock (+ half a frame) of the current generation
> and uploads only when it changed. No frame ready = HOLD the last shown frame (`video_late_frames` once the clock has
> passed the next one) -- never 0 / no media (Pitfall 53); a player that has never shown a frame is PENDING (the C1
> pause, the C3 gate), which after `open()`'s frame-0 decode happens only for a broken first frame or a seek before
> the first draw. Off screen the thread idles (rule 15: clock only); a mid-GOP retrigger, cue or deck return re-seeks
> to the keyframe and catches up (dropping without converting) while the layer holds ~0.3 s at 1080p / ~1.3 s at 4K
> for 150 frames past a keyframe (x264 GOP 250) -- it used to freeze the WHOLE output at 14 / 3.8 fps for 3.4 s
> (diag-media S1b). `videoPlayerMutex_` guards the map lookup only; `close()` signals and returns; `drainRetiredMedia`
> destroys a player only once its thread has exited. Never call FFmpeg or touch a ring slot from the GL thread; never
> hold a lock across a decode. Guards: `tests/test_video_ring.cpp`; live: `.harmony/probe-video.sh`.
`CLAUDE.md`: index line after 53 (~175 B):
> NN. Video decodes off the GL thread: the render thread picks the newest ring frame <= its clock and never waits; a
> hold is not pending -- before touching VideoPlayer or syncMedia's video branch.
Paid by rule 5 (:97) becoming: `5. **Every new effect is a GLSL file in `/shaders`**: Never hardcode effect logic in
C++. One `.frag` file per effect. **NOTE (actual shipped model)**: the 135 shipped effects are inline strings in
`src/render/EmbeddedShaders.h`, compiled at startup (the legacy `shaders/` dir was removed Wave 0; hot-reload is
inert for them -- `docs/claude/effects.md`).` and `docs/claude/effects.md:9` gaining: "Shaders compiled from files are
hot-reloadable from disk (`ShaderManager`); the embedded set is not." `wc -c CLAUDE.md` <= 25,000 after both lanes.
`docs/claude/performance-controls.md:49`, the B2 parenthesis "(`Renderer::tickMediaClock` -> `VideoPlayer::advanceClock`;
the next on-screen frame seeks and decodes, bounded per call)" -> "(`Renderer::tickMediaClock` ->
`VideoPlayer::advanceClock`; the player's decode thread idles off screen and, on return, re-seeks and catches up while
the layer holds its last frame -- rendering.md "Video playback")".
`docs/claude/architecture.md:210`: "VideoPlayer.h/cpp ... FFmpeg video decode ... -> GL texture" -> "... FFmpeg video
decode on a per-player decode thread -> VideoRing (3 RGBA slots) -> GL texture picked by the render thread".
`docs/claude/testing-eyes.md`: one sentence next to the completeness gate: "A video HOLD (the last shown frame while
its decode thread catches up after a seek) is not pending: `render_frame` may capture it; poll `video_late_frames`
until it stops moving before a frame-accuracy check (probe-video `settle_late`)."
`src/media/VideoPlayer.h:22-28` threading comment rewritten to the model above (message thread: open / start / close /
getThumbnail / transport setters; GL thread: advanceFrame / advanceClock / uploadToTexture / releaseGL / neverShown;
decode thread: everything FFmpeg after start()).
BORIS_DECISIONS.md: nothing (no ruling changed). HANDOFF.md / APP-INVENTORY.md: Harmony's.

## 11. COMPACT
- DECISION: both, in sequence -- commit 1 counters + probe (RED by value), commit 2 b1 (upload only new), commits 3-4
  the per-player decode thread + 3-slot lock-free RGBA ring (sws negative stride, VERIFIED) with the GL thread keeping
  the clock and picking newest-<=-clock without waiting; b2 superseded (fallback named); VideoToolbox out.
- SEMANTICS: shown-before = hold (late counted), never-shown = pending (C1 / C3 via the existing counters); open()
  still decodes frame 0 + the thumbnail (race gone); rule 15 = idle thread off screen, seek + catch-up on return; seeks
  stay requests; close signals, the thread frees FFmpeg, destroy deferred to the GL drain; mutex = lookup only.
- GATES: ctest +11 (ring protocol, policy, judge, TSan stress; teeth listed); probe-video 9 rows (m1 = w2, m2 = w3 /
  w3b + the never-black capture; rule 15 = w4 + probe-deck-clock; Pitfall 35 = w6; X1 = w7; guards w5 / w8), RED
  predicted per build (main 36 fps / 81.7 p90 / 67-269 ms polls / code 29 / absent fields; commit-1 ~600 decode
  calls per 5 s, 30 per call, ~2400 uploads, 68-120 ms lock waits).
- FENCES: seqvram (Renderer.cpp :311 / after :1533 / :1717-1727, Renderer.h after :286, state after :1324 / :645,
  CMake EOF, rendering.md after :71); part M (MainComponent open paths; must not edit VideoPlayer.*; seam =
  `installVideoPlayer`).
- RISKS: hold up to ~1.3 s at 4K mid-GOP (levers NONREF / priority / thread_count; Boris feel item); RSS up to 1.6 GB
  for 16 drawn 4K players (trim filed); protocol proven headless only; driver upload-copy assumption; CLAUDE.md bytes
  paid.
- OPEN FOR HARMONY: (1) the 4K hold bar (200 frames) and whether to enable NONREF by default; (2) fields in both
  servers vs TEST_SERVER-only (precedent says both); (3) whether part M may ALSO run `open()` off-thread before this
  lane merges (the seam is designed for it, the files are fenced); (4) the sequence-lane merge order (docs paragraphs).

REPORT_FILE: .harmony/.reports/s-rta-0928b/plan-video.md
STATUS: FINAL

## HARMONY ADOPTION (s-rta-0928b, 20:58) — OVERRIDES THE BODY WHERE THEY DIFFER
Plan authored by Fable (wf_199c463b-aaa). Attacked by attack-video-gl.md and attack-video-vj.md. Adopted: R-1..R-16, the
commit sequence, the 9 rows. Rulings:
- V1 (GL MUST-1, ADOPT) "Shown before" is an `everShown_` flag set on the first upload and NEVER cleared by `releaseGL()`
  (context loss) — only a new open() resets it; `judge()` reads it, never `texture_ != 0`. A context loss mid-hold stays a
  HOLD (the layer shows its previous picture until the next upload), never Pending. Gate: a pure ctest in test_video_ring
  (the judge inputs after a simulated release: Held/Late, not Pending). No live row: the only context-loss triggers open or
  resize the Output window, which no gate may do (screen-safety law) — say so in the report.
- V2 (GL SHOULD-2 + VJ MUST-2, ADOPT) The idle rule is checked INSIDE the ring-full wait too: no draw request for 250 ms ->
  `wait(-1)` (park) even mid-catch-up. Put the predicate in the pure policy header with a ctest (ring full, last draw 300 ms
  ago -> park). Live witness: a new TEST-visible `/api/state` field `video_threads_awake` (decode threads not parked) and a
  row w4b: canvas 4K, a4k inPoint 0.5, trig, 2 s, retrigger (catch-up starts), within 100 ms `switch_deck 1`, wait 1.5 s ->
  PASS `video_threads_awake` == 0. RED arm: a teeth build without the inner check (record the FAIL line, restore sha).
- V3 (GL NIT, ADOPT) Every ring CAS states its memory order explicitly (acq_rel on success, acquire on failure).
- V4 (VJ MUST-1, RULED: do NOT pause the fade; ADD a witness row) A retrigger of the incoming clip during a live crossfade
  holds that clip's last SHOWN frame (a real frame of the same clip, already on screen) while it catches up; the fade keeps
  running — Boris's "keep playing" (BORIS_DECISIONS.md:354-355) and strictly better than today (whole output at 14 / 3.8 fps
  on keyframe + 29). C1's pause stays reserved for "nothing shown yet". New row w6b: layer transitionSpeed 2.0, col 0 = a1080
  inPoint 0.5 playing 3 s, trig col 1 = b1080 (fade starts), at +0.5 s retrigger col 1 to a mid-GOP in-point (b1080 inPoint
  0.5 — choose the ordering the code supports: retrigger = same column as the ACTIVE clip); PASS: every peak_callback_ms poll
  <= 16.7; captures during the hold are never black (mean luma > 20) and never carry code 29 / 29+512 (keyframe + 29); after
  settle the code == expected +- 2 with bit 9 set; the fade completes (crossfadeProgress reaches 1 within 2.5 s + the hold).
  Boris feel item: "retriggering a long-GOP video mid-fade holds its last frame for up to ~0.3 s (1080p) / ~1.3 s (4K) while the
  fade continues".
- V5 (VJ SHOULD, RULED) Hold length is a Boris feel item, not a merge blocker — today's behaviour is a 3.4 s whole-output freeze
  on a wrong frame. To shorten the hold now: `kSkipNonRefInCatchUp` defaults ON (skipping non-reference frames that the
  catch-up drops anyway is invisible by construction; the landing frame is always decoded because the predicate applies only
  while > 10 frames from the target). Measure w3 and w3b with it ON and OFF (5 runs each, both tables in the report); if ON
  ever fails a correctness row (w5 / w3 (d) / w3b (f) codes or dbox), turn it OFF and report why.
- V6 Fences as section 7. This lane starts now from main 5b78d43 while seqvram is still in its fix round on Renderer.* and the
  probe docs: rebase onto main after seqvram merges (Harmony tells you in the next round, or the reviewer checks) — never edit
  ImageSequence.* / SeqVram.h. Pitfall text "NN". CLAUDE.md: pay for the index line by trimming rule 5's NOTE (R-K); final
  `wc -c CLAUDE.md` <= 25,000 including seqvram's line.
- V7 GREEN definition as 4.7 + w4b + w6b; quiet-before-lock (the lock helper's acquire_quiet_lock).
