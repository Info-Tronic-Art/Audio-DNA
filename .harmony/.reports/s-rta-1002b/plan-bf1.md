# PLAN lane bf1 (s-rta-1002b) -- Record to Clip: the output or the selected layer, bar line to bar line, into the top layer's next empty cell

Architect (Fable), 2026-10-02. Base: main 5e47d17; every file cited is unchanged in meaning at fa9604d (lane hyg changed
comments / logging only -- `git diff --stat 5e47d17 fa9604d -- src`). Evidence rig (scratch, rebuildable, no app launched):
$S = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf1
-- bench.sh (encoders), dec.sh / dec2.sh (decoders) and their output files.

PREMISE UPDATE (read first): the dispatch says "a deck change mid-recording (bf9: decks stop)". That premise is SUPERSEDED by
Boris's 14:44 clarification, BORIS_DECISIONS.md:350-356 "Decks are boxes of clips; the layers are what plays": the layers are
ONE shared playing stack, a deck switch changes only which clips the grid shows, nothing plays invisibly (re-planned as lane
bf9b; plan-bf9b.md was a skeleton when this was written). This plan is written for the bf9b model and stays correct on the
pre-bf9b model (section 3, F9).

## (1) GOAL

One press of REC (above the preview, or a key / MIDI pad / REST) writes the whole output -- or the selected layer -- into a
new video clip that starts and ends exactly on bar lines and lands, ready to trigger, in the first empty cell of the top
layer of the deck on screen, without changing anything that plays.

Boris (verbatim, backlog BF1): "Sampling to other layers/ clip. Think how to do this, like set a clip to record output of the
composition and they be able to use it as soon as it is done. This would be good to have it automatically start and end on
bars or beats, quantized". His answer (binding-decisions.md:574-577): "whole ouput of select layer, stop start and snap to
bar, next empty cell on top layer no need to select it, video files yes".

Why a record toggle is not a playback stop (BORIS_DECISIONS.md:358 "No stop model ... there is no stop buttons anywhere"):
REC never stops, pauses, rewinds, clears or replaces anything that plays -- no clip, layer, deck or routine changes state and
the outputs are untouched. It only decides which stretch of the picture is written into a new clip, the way Snapshot
(an existing camera-shutter action, Binding.h:43) captures one frame. Its second press ENDS THE TAKE ("Ends at bar"), it is
never labelled Stop, and the show keeps playing through it. The ruling is about playback controls; this is a capture control.

## (2) ESTABLISHED FACTS

The existing recorder (reuse candidate)
- E1 VERIFIED `VideoRecorder::submitFrame` does a synchronous `glReadPixels` of the bound READ framebuffer into a CPU triple
  buffer on the GL thread (VideoRecorder.cpp:102-140, the read at :130; first-use `resize` allocations at :123-124); the
  encoder thread stamps `pts_++` per encoded frame (:385) at time base 1/fps (:241); libx264 ultrafast / zerolatency / CRF,
  no keyint set = x264's long default GOP (:255-263); `stopRecording` joins the encoder thread and flushes on the CALLER
  thread (:66-94).
- E2 VERIFIED the menu / binding recorder runs at fps 30, canvas-sized, into ~/Documents/Audio-DNA/Recordings
  (MainComponent.cpp:7099-7117, :7838-7858).
- E3 VERIFIED the Renderer calls `submitFrame` once per render frame, normal path only (Renderer.cpp:1033-1039); the fenced
  hold (:415-429), nothing-to-render (:518-526) and no-content (:836-844) exits return without it.
- E4 VERIFIED the GL loop repaints continuously (Renderer.cpp:58) and runs at ~120 fps on this Mac (rendering.md:77:
  "105 -> 120 fps", "0 with the blit at 120 fps").
- E5 INFERRED from E1-E4 (FOUND, not fixed, outside BF1): a menu recording stamps ~120 render frames per second at 1/30 s --
  a ~4x slow-motion file; frames from the three early exits are missing (pauses vanish); a canvas resize mid-recording
  hands sws a smaller capture than the context it was built for (VideoRecorder.cpp:118-127 vs :318-320). Cheapest check: Output
  > Start Recording for 10 s, `ffprobe` duration (expect ~40 s if E5 holds).

Frame, beat and bar on the GL thread
- E6 VERIFIED one snapshot per frame: `frameSnap_ = featureBus_.read()` before every exit (Renderer.cpp:401-405); bar-snapped
  pending triggers fire on the GL thread inside `Autopilot::processFrame` (Renderer.cpp:546-561) on the `totalBeatCount` delta
  (Autopilot.cpp:65, own `OnsetPulse` baseline) with Bar = `beatInBar == 0` (Layer.h:459-461); the frame composites AFTER, so
  the bar's first frame already shows the triggered clip. `OnsetPulse::consume` = delta since my last look (OnsetPulse.h:21-28).
- E7 VERIFIED quantize honesty: with no tracker lock a quantized trigger fires at once (MainComponent.cpp:27-34); routines use
  `trackerState == STATE_LOCKED && bpm > 0` (MainComponent.cpp:4089-4091).
- E8 VERIFIED the frame time: `now` (HiRes seconds) at the top of renderOpenGL (Renderer.cpp:360), before every exit.

Layer pictures
- E9 VERIFIED a layer's picture is saved AFTER its stages -- clip transform + clip opacity, clip effects, clip transition,
  feedback, layer effects, layer transform (CompositorEngine.cpp:980-1026) -- and BEFORE keying, blend and layer opacity
  (:1142-1191); `saveLayerOutput` records owner {deckId, frameSerial_} (:236-262, owner at :241); a pending image re-touches the
  owner with the held picture (:1119-1126); outputs are canvas-sized and recreated after a resize (:130-137); keyed by layer
  id (CompositorEngine.h:555-559). FX-only / Mask / ThreeD layers never save one (:1194-1245).
- E10 VERIFIED compositing is bottom to top, index 0 at the bottom (CompositorEngine.cpp:1069); the grid shows the highest
  index at the top (DeckView.cpp:138-144) -> "top layer" = `layers.back()`.

Cells, drops, players
- E11 VERIFIED an empty cell is an empty optional (Deck.h:148-162) or a blank Clip, which ClipCell paints as empty
  (ClipCell.cpp:18-97). `Deck::setClip` grows only that layer's vector (Deck.h:137-146); `addColumn` grows all (:90-95).
- E12 VERIFIED the single-file drop opens the video ON the message thread and installs it before the fence
  (MainComponent.cpp:5238-5285 -> Renderer::openVideoForClip / installVideoPlayer, Renderer.cpp:1503-1540); `commitDrop` runs
  inside `withDeckDetached` (MainComponent.cpp:5287-5320); a multi-video drop grows columns for EVERY layer inside the fence
  and pushes SetColumnCountCmd + SetClipCmds as one undo entry (:809-860). Clip ids: `s_nextClipId++` (:17, :5254).
- E13 VERIFIED `MediaOpener::begin` cancels the live batch first (MediaOpener.h:42-43): it cannot carry one landing while a
  composition load is staged. Pitfall 56: `open()` on the message thread or MediaOpener's pool, on an UNPUBLISHED player.
- E14 VERIFIED a staged load is `staged_` (MainComponent.h:452); a command during the window acts on the live model and is not
  replayed (Pitfall 58); `swapCompositionModel` bumps `modelEpoch_` (MainComponent.cpp:2988-2997).
- E15 VERIFIED decode uses FFmpeg `thread_count` 2 (VideoPlayer.cpp:129); the thumbnail is made inside `open()` and read by
  `getThumbnail` (VideoPlayer.cpp:709-734); intra-only files keep NO GOP cache -- one seek + one decode per reverse frame
  (rendering.md:79); an index of keyframes only counts as intra-only (plan-mkvidx C1, VideoPlayer.cpp:294-295); VideoToolbox
  decode is filed, not built (rendering.md:77).
- E16 VERIFIED (plan-mkvidx X5, by experiment) at HEAD a frame seek truncates, so intra-only files reverse slightly late
  ("HAP or ProRes at FFmpeg's 1/15360: late 16, shown 262" of 270 over 9 s); plan-mkvidx item 3 fixes it (late 0).

IOSurface frame sharing (reuse candidate)
- E17 VERIFIED `output::SurfacePool` (no GL; BGRA IOSurfaces; `kSlots` 4; `retainSurface(gen, slot)` from any thread) and
  `SharedFrameSet::publish` (CGLTexImageIOSurface2D rect textures as FBO attachments, one blit, glFenceSync + glFlush,
  timeout-0 polls, contexts can die and rebind) -- SharedFrameSet.h:63-148, SharedFrameSet.cpp:16-124. Tested offscreen with
  private CGL contexts (tests/CMakeLists.txt:2322-2360 test_surface_pool, test_shared_frame_gl).

Controls, persistence, UI
- E18 VERIFIED `Binding::Action` is append-only, TriggerRoutine last (Binding.h:24-48); the loader casts the stored int
  (BindingManager.cpp:273); the append-test pattern (tests/test_routine_engine.cpp:757-777); learnable global targets
  (MainComponent.cpp:7443-7459); dispatch (:7537; TriggerRoutine's press / Momentary release at :7743-7750).
- E19 VERIFIED REST pattern: a hook + `callAsync` (ApiServer.cpp:2234-2258); a synchronous status read from a mutex-guarded
  copy (:2357-2369, ApiServer.h:161-166); routine routes registered at :333-338.
- E20 VERIFIED `Composition::name` / `filePath` (Composition.h:30-31); media paths are absolute (Clip.cpp:32, :175).
- E21 VERIFIED Clip "BPM Sync" for video sets speed = videoBeats / beatDivision whatever the tempo (Renderer.cpp:1814-1824):
  no tempo-locked video transport exists (FOUND; affects loops after a tempo change, not this lane).
- E22 VERIFIED the preview panel's tab bar is 26 px with two 80 px buttons on the left (PreviewPanel.h:66-71,
  PreviewPanel.cpp:40-52) -- free space to the right. The routine view precedent: a pure derive pushed every 30 Hz tick
  (MainComponent.cpp timerCallback, `deckView_->setRoutineView(deriveRoutineDeckView(...))`).
- E23 VERIFIED teardown: HTTP servers stop, then the GL detaches, then `videoRecorder_.stopRecording()` (MainComponent.cpp:
  ~2392-2442); `previewPanel_` is declared at MainComponent.h:338, `videoRecorder_` at :667.
- E24 VERIFIED `tests/test_log_line_lint.cpp` pins the worker-thread files that must log with `logLine` (no std::cerr).

Codec measurements (VERIFIED by experiment, $S/bench.sh + dec.sh + dec2.sh; FFmpeg 8.0 CLI, 1080p60, 4 s of noisy synthetic
content = worst-case entropy; CPU = user time minus the generator's baseline):
| candidate | encode CPU / frame | file rate | decode + BGRA CPU / frame (2 threads) |
|---|---|---|---|
| ProRes 422 Proxy, VideoToolbox (BGRA in) | ~0.6 ms | 72 Mbit/s | 10.8 ms (decode alone 7.0) |
| ProRes 422 LT, VideoToolbox | ~0.6 ms | 201 Mbit/s | 18.0 ms (decode alone 14.0) |
| ProRes 422, VideoToolbox | ~0.7 ms | 309 Mbit/s | 24.8 ms |
| H.264 all-intra, x264 ultrafast + fastdecode, CRF 18 | ~9.8 ms incl. BGRA->yuv420p | 51 Mbit/s | 3.9 ms (decode alone 3.3) |
| H.264 GOP 30, x264 ultrafast (today's recorder) | ~9.1 ms | 21 Mbit/s | 2.4 ms |
| H.264 all-intra, VideoToolbox (-g 1, nv12) | ~13 ms (conversion), 100 fps ceiling | 80 Mbit/s (set) | -- |
The x264 all-intra file: 240/240 packets key-flagged, Constrained Baseline, no B-frames, colour untagged, MP4 time base 1/15360.

## (3) DESIGN FORKS

F1 How the picture leaves the GPU. CHOICE: an IOSurface tap -- one flipped blit of the source into a free IOSurface slot +
a fence per captured frame on the GL thread; the encoder thread reads the surface after the fence (reusing output::SurfacePool,
E17). Runner-ups: (a) VideoRecorder's synchronous glReadPixels (E1) -- loses: it waits for the GPU to finish the frame and
copies 8 MB (1080p) on the GL thread for every captured frame, in a 8.3 ms frame budget at 120 Hz; (b) PBO readback -- loses:
still a mapped copy owned by the GL thread and unknown behaviour under Apple's GL-on-Metal, while E17 is already proven here.
Cost: macOS only -- like outputs and Syphon (integration.md:21); other builds compile it as unavailable (button hidden).

F2 The frame clock. CHOICE: wall-clock constant frame rate. Each render frame gets tick = floor((t - t0) x fps / 1000 + 0.5);
the FIRST render frame of each new tick is captured; a tick with no captured frame (a fenced frame, a hitch, no free slot) is
written as a repeat of the previous frame; ticks before the first capture repeat the first. Rounding (not flooring) keeps a
render rate equal to the take rate (a 60 Hz projector) from skip / repeat pairs under +-half-frame jitter. Runner-up:
timestamps with gaps (VFR) -- loses: this app indexes video by average frame duration (rendering.md:79, "VFR is one frame per
average-duration index"), so gaps become holes in reverse; a constant rate file is simplest for every reader. Today's
"one encoded frame per render frame" (E1/E5) is exactly the bug this avoids.

F3 Who decides the bar. CHOICE: the GL thread, per frame, from THE frame's snapshot (E6): bar edge = own `OnsetPulse` delta
of `totalBeatCount` > 0 and `beatInBar == 0` -- the autopilot's rule for bar-snapped triggers, in the recorder's own baseline
(Pitfall 38 spirit). The take starts ON the edge frame (included) and ends ON the next chosen edge frame (excluded): exactly N
bars, and a clip triggered with bar snap in the same bar is in the take's first frame. Not locked (E7's routines rule) =
start / end on the next frame. Runner-up: the message thread's 120 Hz tick (RoutineEngine style) -- loses: 0-8 ms plus
message-thread jitter after the GL thread has already fired that bar's triggers, so the first frame can miss them.

F4 Codec / GOP / rate. CHOICE: H.264 all-intra (libx264, keyint 1, preset ultrafast, tune fastdecode + zerolatency, CRF 18,
yuv420p, threads 4) in .mp4, 60 fps (30 fps when the canvas is larger than 2560 x 1440), canvas size rounded down to even.
Every frame is a keyframe, so this app plays it forward, reverse and ping-pong on its intra path (no GOP cache, one seek + one
decode per frame, E15) and the clip bar scrubs instantly. Runner-up: ProRes 422 Proxy / LT through VideoToolbox -- encode
nearly free (0.6 ms CPU) and the VJ-standard intra format, but it LOSES because a recording is made once and then played many
times, several at once, both directions: ProRes software decode costs 2.8-4.6x the CPU per frame of the H.264 file in this
app (10.8-18.0 vs 3.9 ms measured) and VideoToolbox decode is not built (E15). Pre-registered fallback: if G4's CPU-driven bars
fail at 1080p, switch the take to ProRes 422 Proxy via VideoToolbox and re-run G4 + G5. Rejected: long-GOP H.264 (GOP cache
memory, slow scrubs), VideoToolbox H.264 all-intra (CPU conversion, 100 fps ceiling), HAP (software-decoded here, huge),
MJPEG (no gain over x264 intra). Rate: 120 doubles encode, decode and size for no visible gain on 60 Hz projectors; 30 is
visibly steppy for audio-reactive motion.

F5 What "the selected layer" records. CHOICE: the layer's saved picture (E9) -- its clip, clip effects, transition, feedback,
layer effects and transform, BEFORE its keying, blend and opacity, so the take plays back on any layer looking like that layer
did, mixed by the new layer's own fader. A frame where the layer has no picture of its own (empty, hidden, its clip FX-only)
records opaque black. FX-only / Mask / ThreeD layers are refused at the press (no picture of their own). Runner-ups: after keying
+ opacity (bakes the mix fader in; needs a new pass); the accumulator after the layer (that is "the output minus the layers
above", not the layer). Transparency is not kept (yuv420p) -- Q4.

F6 Which cell, and when it is chosen. CHOICE: the deck ON SCREEN WHEN THE TAKE LANDS, its top layer, the first empty cell from
the left; a full row grows one column (all layers, as a multi-video drop does, E12). The cell is shown live during the take
(a red mark in the grid, recomputed every 30 Hz tick), never reserved. Runner-up: reserve the cell at the press -- loses: the
reserved cell can be filled, moved or have its deck switched during the take and then needs conflict rules anyway, while the
live mark already shows where it will go. "First from the left" vs "after the last clip" is Boris's (Q1).

F7 How the file becomes a playable clip. CHOICE: the recorder's worker thread, which just wrote the file, opens a VideoPlayer
on it as an UNPUBLISHED player (setStats, open, getThumbnail) and posts it to the message thread, which installs it
(`installVideoPlayer`) and writes the cell inside one `withDeckDetached` fence with one undo entry -- the multi-video drop's
shape (E12) without FFmpeg on the message thread. A landing that arrives while a composition load is staged waits (re-tried on
the 30 Hz tick) until the load lands or is cancelled, then lands on the deck on screen (Pitfall 58: never write into the model
about to be swapped out). Runner-ups: `prepareFileDrop` on the message thread (10-30 ms of FFmpeg on the message thread
mid-show, INFERRED); MediaOpener (its begin cancels a staged load's batch, E13). Pitfall 56's text gains this third opener.

F8 Files. CHOICE: saved composition -> "<composition folder>/<composition file name> Recorded Clips/"; unsaved ->
"~/Documents/Audio-DNA/Recorded Clips/". Written as a hidden ".rec-<uuid>.mp4", renamed when finished to
"<Source> <N> bars <HHMMSS>.mp4" (a take that did not end on a bar: "<Source> <S.s> s <HHMMSS>.mp4"), Source = "Output" or the
layer's name, `getNonexistentSibling` on collision; clip name = the file name. Files stay put if an unsaved composition is
saved later (absolute paths, E20); undo removes the clip, never the file. Nothing is deleted automatically except an
unfinished take's own temp file (cancel, failure, nothing captured).

F9 Changes during a take. Under bf9b (decks are boxes) a deck switch changes nothing that plays: an Output take records on;
a Layer take records on (the layer is in the shared stack). Generic rule that is right on both models: a Layer take ENDS
(cut, kept, landed) on the first frame its layer is no longer in the stack being rendered -- removed, or (pre-bf9b only) its
deck left the screen. A composition load: an Output take records on (it is the output); a Layer take ends as above; the
landing follows F7. GL context loss (preview hidden, SignalBar expanded -- SharedFrameSet.h:7-10): the take ends at its last
captured frame (cut, kept, landed). Canvas size change: the take keeps its size (the blit scales). Quit: the take is finished
into a valid file, not landed. The cap: 64 bars, or 300 s without a lock (Q3).

F10 Where REC lives. CHOICE: the right end of the preview panel's tab bar (E22): [Output | Layer] switch + REC button, next to
the picture it records; no contention with TopBar (bf2's sync dial and bf7's bar readouts are likely there). Runner-up: the
TopBar ("REC" appears in the old Top Chrome mockup, BORIS_DECISIONS.md:161) -- loses on TopBar crowding and merge contention;
the critic panel and Boris's artifact page can still move it.

F11 Press semantics (one funnel for button, key, MIDI and REST "press"):
| phase | press | Momentary release / REST stop | REST cancel |
|---|---|---|---|
| Idle | arm: Waiting (locked) or start next frame (no lock) | -- | -- |
| Waiting | cancel (nothing written, temp removed) | cancel | cancel |
| Recording | Ending: ends on the next bar line (no lock: next frame) | Ending | -- (a control never discards a take) |
| Ending | ignored | -- | -- |
| Saving | arm the next take; it starts on a bar line once the last file is done | -- | -- |
REST "start" = press from Idle / Saving only. Runner-up: a waiting second press ignored (the routine pads' rule) -- loses: an
accidental arm could not be taken back before it starts.

F12 New subsystem vs extending VideoRecorder (CLAUDE.md rule 13). CHOICE: new ClipRecorder / ClipEncoder / RecordTap /
TakeClock; VideoRecorder untouched. Its synchronous readback (F1) and per-render-frame stamping (E5) are what BF1 must not
inherit, and changing it is outside the request. Existing systems ARE extended: SurfacePool, OnsetPulse, installVideoPlayer,
commitDrop + SetColumnCountCmd + SetClipCmd, the binding / REST funnels, the routine view pattern.

Thread model (Sacred rule 2 asks for explicit discussion of any new mutex): GL thread -> worker = an SPSC slot queue + atomics;
the GL side only notifies a condition variable (VideoRecorder's precedent, Pitfall 25) and never locks; the worker's wait
mutex and the status-copy mutex (RoutineEngine::status precedent) are taken only by the worker, the message thread and httplib
-- never by the GL, audio or analysis thread. Message -> GL = one release-stored request word (seq + command) after relaxed
writes of its parameters; GL -> everyone = relaxed atomics. Harmony: these two off-hot-path mutexes are the explicit ask.

## (4) ITEMS + BUILD STAGES

I1 TakeClock -- the pure timing core. NEW src/recording/TakeClock.h (header-only; no JUCE / GL / FFmpeg).
- `take::Gate::onFrame(FrameIn) -> FrameOut`. FrameIn {tMs, totalBeatCount, beatInBar, locked, captureAllowed (false on a
  fenced frame), sourceGone, workerFree, cmd}. FrameOut {started, ended, reason (Bar / Unlocked / Cap / SourceGone /
  ContextLost / Quit), captureTick (-1 = none), totalTicks}. Phases Idle / Waiting / Recording / Ending; F2 ticks, F3 edges
  (own OnsetPulse), F11 commands, the cap. A start fires on an edge only when `workerFree` (the previous take is finished);
  a fenced frame counts edges but captures nothing and never ends a take for SourceGone.
- `take::tickOf`, `take::totalTicks`, `take::fill(prevTick, tick, first)` (the ticks a captured frame covers).
- RED-first: tests/test_take_clock.cpp (NEW), written first against a stub Gate whose onFrame returns {} (compiles, FAILS):
  TC1 a press mid-bar starts exactly on the next bar-edge frame, tick 0 captured on it (`REQUIRE(out.started)` fails on the
  stub); TC2 no lock -> starts on the next frame; TC3 60 fps from 120 Hz +-1 ms (every tick once, no repeat), from 60 Hz
  +-3 ms (no skip / repeat pair), from 50 Hz (10 repeats/s), a 100 ms hitch (6 repeats); TC4 a stop mid-bar 2 ends on the
  bar-3 edge frame, that frame not captured, totalTicks 240 +- 1 at 120 BPM; TC5 the F11 table, incl. Saving + press starting
  only on an edge with workerFree; TC6 fenced frames: edges counted, no capture, repeats; TC7 sourceGone ends a Layer take
  (never on a fenced frame); TC8 the cap at the 64th edge, 300 s unlocked; TC9 lock lost while Waiting / Ending acts at once;
  TC10 a start on a fenced frame -> earlier ticks repeat the first capture.
- GREEN: TC1-TC10 pass (ASan / UBSan build clean). Risk: R3.

I2 ClipEncoder -- the H.264 all-intra MP4 writer. NEW src/recording/ClipEncoder.{h,cpp} (FFmpeg only).
- `open(tempFile, w, h, fps, err)`, `add(bgraTopDown, strideBytes, tick)` (sws BGRA -> yuv420p with sws defaults -- the same
  defaults VideoPlayer decodes with, so colours round-trip; repeats re-send the last converted, ref-counted frame with the next
  pts), `finish(totalTicks, finalFile) -> {ok, frames, durationMs, err}` (fill, flush, trailer, close, rename), `abort()`
  (close + remove temp). F4 settings. Worker-thread logging via logLine; add the .cpp to test_log_line_lint's list (E24).
- RED-first: tests/test_clip_encoder.cpp (NEW; links FFmpeg like test_video_player_open) against a stub that writes nothing:
  CE1 ticks {0,1,2,5,6} + finish(9) -> the file decodes to 9 frames, all key-flagged, h264, has_b_frames 0, 60/1, duration
  150 ms +- 1, frames 3-4 equal frame 2 and 7-8 equal frame 6 (+-4/255) (`REQUIRE(frames == 9)` fails on the stub); CE2 top
  half red / bottom blue -> decoded row 0 red, last row blue (+-6/255); CE3 first capture at tick 2 -> frames 0-1 equal it; CE4
  finish with no frames -> false, no file left; CE5 an unwritable folder -> open false with an error, nothing left; CE6 rename
  to the final name, a collision gets a sibling name.
- GREEN: CE1-CE6. Risks: R5, R8.

I3 RecordTap -- GL to IOSurface. NEW src/render/RecordTap.{h,cpp} (macOS; elsewhere `available() == false`, every call a
no-op); reuses output::SurfacePool unchanged.
- GL thread: `ensure(takeW, takeH)` (pool generation + rect textures + FBOs on this context, SharedFrameSet's binding recipe);
  `capture(srcFBO, srcW, srcH, tick)` (a Free slot -> blit flipped to top-down rows (dst y0 = takeH, y1 = 0), GL_NEAREST at
  equal size else GL_LINEAR, scissor off around it (Pitfall 60 (4): a blit obeys the scissor), glFenceSync + glFlush, leaves
  GL_FRAMEBUFFER = canvas FBO as SharedFrameSet does; srcFBO 0 = clear opaque black; no Free slot -> false, `droppedNoSlot`);
  `poll()` (fences in capture order, timeout 0: signalled -> Queued + SPSC push {gen, slot, tick} + notify; GL_WAIT_FAILED ->
  Free, `fenceFailed`); `releaseGL()` (delete names, DROP fences without glDeleteSync, Fenced slots Free and counted).
  Allocation-free per frame; zero-heap `logLinef` only.
- Worker: `pop(Item&)`, `retain(Item)` (`retainSurface`), `release(Item)`; slot states Free / Fenced / Queued as atomics.
- RED-first: tests/test_record_tap_gl.cpp (NEW; private CGL context, the test_shared_frame_gl recipe; SKIPs loudly without a
  pixel format) against a stub whose capture returns false: RT1 a top-red / bottom-blue FBO lands as BGRA with row 0 red
  (read on a second thread under IOSurfaceLock read-only) (`REQUIRE(tap.capture(...))` fails on the stub); RT2 four held slots
  -> the fifth capture is refused and counted, held bytes unchanged; RT3 80 x 60 source scaled into 64 x 48; RT4 releaseGL with
  a fence pending -> nothing in flight, counted; RT5 srcFBO 0 -> opaque black. INFO print: capture + poll CPU per frame.
- GREEN: RT1-RT5. Risks: R2, R10.

I4 ClipRecorder -- the owner. NEW src/recording/ClipRecorder.{h,cpp}.
- Message thread: `press / start / stop / cancel (Source, optional LayerKey, Context{folder, layerName})`, `shutdown()`
  (finish a live take into a valid file, no landing, join); `status()` (any thread). `onLanded` delivered on the message thread
  through a poster (default callAsync; tests inline). LayerKey = {deckId, layerId} today; bf9b may reduce it to the shared
  layer's id -- the key is opaque to everything but `Renderer::tapClipRecorder` (I5).
- GL thread: `onFrame(FrameIn, Sources{canvas fbo/w/h, layer fbo/w/h or 0, layerPresent})` runs I1, then I3 (`ensure` when a
  take arms -- never on the start frame unless unlocked -- capture, poll); `onGLContextClosing()` (end: ContextLost, releaseGL);
  `onGLContextCreated()` (beat baseline reset).
- Worker (one persistent thread, QOS_CLASS_UTILITY): on the GL's "armed, size W x H" opens folder + temp file + encoder (an
  open failure ends the arm with an error; nothing left); consumes items (IOSurfaceLock read-only -> ClipEncoder::add ->
  unlock -> release); at the end waits for in-flight slots (<= 1 s, then drops them, counted), finishes, renames, then opens
  a VideoPlayer on the final file as an UNPUBLISHED player (setStats, open, getThumbnail(90, 72)) and posts Landed {file,
  player, thumbnail, w, h, frames, bars, wholeBars, reason, source name}. `workerFree` true again only after that.
- Status: phase (idle / waiting / recording / ending / saving), source, layer {deck, layer, name}, locked, bars, beatInBar,
  fps, w, h, ticks, droppedNoSlot, repeats, startRepeats, fenceFailed, encoderOpenMs, lastTake {ok, file, frames, durationMs,
  bars, cut reason, landed {deck, layer, column} | null, error}, takes.
- A FrameSink seam (I3 in the app; a CPU sink of BGRA buffers in tests) so the real gate + encoder + finalize + landing
  hand-off run headless.
- RED-first: tests/test_clip_recorder.cpp (NEW) against the stub: CR1 5 s of a 120 BPM clock at 120 Hz, press at 0.3 s, stop
  at 3.1 s -> one Landed, ok, frames 240 +- 1, bars 2, a player opened at the take size, thumbnail valid, final name
  "Output 2 bars <HHMMSS>.mp4" (`REQUIRE(landed)` fails on the stub); CR2 cancel while Waiting -> no file, no landing; CR3
  shutdown mid-take -> valid file, no landing; CR4 [tsan] GL-driver thread + press / stop / status thread + worker in a TSan
  build -> no report (ctest label tsan; run by .harmony/probe-tsan-unit.sh).
- GREEN: CR1-CR3 under ASan, CR4 under TSan (abort_on_error=0). Risks: R6, R12.

I5 Renderer + Compositor hooks.
- src/render/Renderer.h: `setClipRecorder(ClipRecorder*)` beside `setVideoRecorder` (:203); member beside `videoRecorder_`
  (:647); private `tapClipRecorder(const FeatureSnapshot&, double frameMs, const Deck*, bool canvasValid)`.
- src/render/Renderer.cpp `renderOpenGL`: ONE call at each of the four exits -- the fenced hold (:415-429, canvasValid
  false: edges only), nothing-to-render (:518-526), no-content (:836-844) and right after the old recorder's submit block
  (:1033-1039); frame time = the existing `now` (:360) in ms. `tapClipRecorder` builds FrameIn from the frame's snapshot
  (E6/E7 lock rule), the canvas (canvasFBO_ / canvasW_ / canvasH_), and for a Layer take the compositor's fresh picture plus
  "is the layer in the stack rendered this frame" (`deck->layers` ids; under bf9b the shared stack). `newOpenGLContextCreated`
  (:233) -> onGLContextCreated; `openGLContextClosing` (:1165) -> onGLContextClosing.
- src/render/CompositorEngine.h: one const accessor beside `getLayerOutputTexture` (:569-584): `freshLayerOutput(LayerKey)`
  = {fbo, w, h} iff that layer's picture was saved or held THIS frame (owner == {deck, frameSerial_}), else {0, 0, 0}.
- No behaviour change while no take is armed: one OnsetPulse consume + one branch per frame.
- RED: Renderer.cpp is in no ctest (Pitfall 52) -> probe rows rc1-rc3, rc9-rc12, rc14 (404 on main). GREEN: those rows.

I6 Landing into the top layer.
- NEW src/recording/RecordTarget.h (pure): `topLayerIndex(deck)`, `firstEmptyColumn(layer, numColumns)` (E11's empty;
  numColumns = grow), `layerRecordable(type)` (Opaque / Transparent), `takeFolder(composition)`, `takeName(...)`,
  `makeClip(Landed, id)` (Video, playing true as a dropped video :5267, Loop, beat snap as dropped clips (Off; the global
  Quantize governs -- Q5), videoBeats = beatDivision = bars x 4 for whole-bar takes (metadata), w / h / thumbnail).
- src/MainComponent.{h,cpp}: member `ClipRecorder clipRecorder_` declared after `previewPanel_` (MainComponent.h:338);
  `setClipRecorder` beside :531; `landRecordedTake(Landed)` modeled on onMultiVideoDropped (:809-860): `staged_` set ->
  `pendingLanding_`, re-tried from timerCallback; else active deck, top layer, first empty column; mint `s_nextClipId++`;
  `installVideoPlayer(id, player)`; one `withDeckDetached` around the column growth (all layers, as :829-836) + `commitDrop`;
  one undo entry "Record '<name>'" = SetColumnCountCmd (if grown) + makeSetClipCmd (as :845-852); `rebuildGrid()`; status
  `landed`. Teardown: `clipRecorder_.shutdown()` right after the GL detach, beside `videoRecorder_.stopRecording()` (:2442).
- RED-first: tests/test_record_target.cpp (NEW, pure) against stubs: RG1 top = last index; nullopt and a blank Clip are empty, a
  Source / FX-only clip is not (`CHECK(firstEmptyColumn(...) == 2)` fails on the stub's -1); RG2 a full row -> numColumns;
  RG3 recordable types; RG4 names (whole bars / cut seconds / layer name); RG5 folders (saved / unsaved). Probe rc4, rc7.
- GREEN: RG1-RG5, rc4, rc7. Risks: R7, R14.

I7 Controls: the funnel, the binding action, REST.
- src/MainComponent.cpp `recordClipPress(...)`: the ONE funnel -- source from `recordSource_` (the preview switch; Output
  default), layer from `deckView_->getSelectedLayerIndex()` (DeckView.h:113) or REST "layer"; a Layer source with no layer
  selected or a non-recordable type is refused (status error, nothing armed).
- src/binding/Binding.h: append `RecordClip` after TriggerRoutine (:46) -- APPEND ONLY. handleBindingAction (:7537) new case
  after :7743-7750: press -> funnel; Momentary release -> stop. buildBindableTargets (:7443-7459): "Record Clip" in the global
  row. BindingOverlay.cpp (:268-292) and MidiLearnOverlay.cpp (:315-339): one label case each.
- src/api/ApiServer.{h,cpp}: `POST /api/record_clip` {"action": "press" | "start" | "stop" | "cancel", "source": "output" |
  "layer", "layer": N (0-based on the deck on screen; default = the selected layer)} -> callAsync(onRecordClip) (E19);
  `GET /api/record_clip/status` -> onRecordClipStatus() (the mutex-guarded copy). Registered after :338. Compatible with
  tsan-r5's R7 by construction (no live-model read on httplib).
- RED-first: tests/test_record_clip_binding.cpp (NEW): RB1 `RecordClip == TriggerRoutine + 1` and a BindingManager save /
  load round trip of a Momentary RecordClip binding -- fails to compile on main (no enumerator). Probe rows 404 on main.
- GREEN: RB1; probe rows. Key / MIDI live paths are not driven (no synthetic input): they share the funnel REST "press" uses.

I8 On screen: REC above the preview + the landing mark in the grid.
- NEW src/ui/RecordClipView.h (pure): `deriveRecordClipView(status, uiSource, selectedLayerName, recordable)` -> {text, style,
  enabled, tooltip, paintKey}; `deriveRecordMark(status, deck on screen)` -> {layer, column, mark}.
- NEW src/ui/RecordClipControl.{h,cpp}: [Output | Layer] two-way switch + REC button, right end of the preview tab bar;
  text-only states (Pitfall 6): Idle "REC"; Waiting "Starts at bar" (red outline); Recording "REC 3.2" (bars.beats since the
  start, solid red); Ending "Ends at bar" (solid red); Saving "Saving" (grey); refused = disabled + tooltip "Select a layer
  (click its name) to record it". Fixed width (no jumping). Repaint only when paintKey changes (Pitfall 57). Tooltip: "Record
  the picture into a new clip -- starts and ends on the next bar line; it lands in the top layer's first empty cell".
- src/ui/PreviewPanel.{h,cpp}: member + `resized()` (:40-52) right side of the 26 px bar.
- src/ui/ClipCell.{h,cpp}: `setRecordMark(None | Armed | Live)` + an overlay after the thumbnail branches (Armed = dashed
  red outline + "REC" in 9 px; Live = solid; Saving = dashed grey "SAVING"). src/ui/DeckView.{h,cpp}: `setRecordMark(layer,
  column, mark)` -> the cell, change-only. MainComponent::timerCallback (:4113+, beside setRoutineView): push both views every
  30 Hz tick and re-try a pending landing.
- Colour: red only for recording states; default token `kMeterRed` (LookAndFeel.h:31) -- no new palette entry unless the
  critic panel asks (Rejected list: multiple accents, glyphs, rounded corners).
- RED-first: tests/test_record_clip_view.cpp (NEW, pure) against stubs: RV1 phase -> text / style (unlocked never shows
  "Starts at bar"); RV2 refused Layer state; RV3 paintKey changes at most once per beat while recording; RV4 the mark = (top
  layer, first empty column) of the deck on screen in Waiting / Recording / Ending / Saving, none when Idle.
- GREEN: RV1-RV4 + G6 (visual work gate).

I9 Probes. NEW .harmony/probe-record-clip.sh + .py (rows in (5) G3), .harmony/probe-record-clip-ab.py (G4, the
probe-vupload-ab.py shape), scene u13 in probe-vupload.py (a landed take in reverse and ping-pong, built as a composition
file like its other scenes). HTTP clients send Connection: close; one app at a time via the lock helper (wf/lock.sh).

BUILD STAGES (one builder context each):
- S1 core, headless: I1, I2, I4 (CPU sink), RecordTarget.h of I6; tests test_take_clock, test_clip_encoder,
  test_clip_recorder, test_record_target. New files only + CMakeLists.txt source list + tests/CMakeLists.txt +
  test_log_line_lint's list. Ships alone: yes (inert). Shares NO source file with any lane -> can be built now, in parallel.
- S2 GL + wiring: I3, I5, I6 landing, I7, I9; tests test_record_tap_gl, test_record_clip_binding. Depends on S1; merges after
  bf9b (Renderer / CompositorEngine / MainComponent) and at bf1's slot of the pre-registered order. Ships alone: yes
  (REST / key / MIDI, no on-screen button) -- Harmony may hold it for S3.
- S3 UI: I8 + test_record_clip_view + G6 + the docs pass. Depends on S2; after lane ui (ClipCell).

FILES SHARED WITH OTHER LANES (function-level, for merge sequencing):
| file | bf1 touches | other lanes there |
|---|---|---|
| src/render/Renderer.{h,cpp} | setClipRecorder + member; one tapClipRecorder call at each of the 4 renderOpenGL exits; newOpenGLContextCreated / openGLContextClosing one line each | bf9b (inactive-deck loop :739-790, deck transition :464-500 / :910-965), tsan-r5 (config-scalar reads) |
| src/render/CompositorEngine.h | one const accessor beside getLayerOutputTexture | bf9b (persistent removal, layer model) |
| src/MainComponent.{h,cpp} | members; ctor wiring beside :531 + ApiServer hooks; ~MainComponent beside :2442; timerCallback beside setRoutineView; buildBindableTargets global row; handleBindingAction new case; NEW landRecordedTake / recordClipPress | bf9b (deck switch), ui (deck rename, Show in Finder), bf2 (venue / sync wiring), bf45, bf6 |
| src/api/ApiServer.{h,cpp} | 2 routes after :338, 2 hooks beside onRoutine* | tsan-r5 (R7 marshalling), bf2 / ui if they add routes |
| src/binding/Binding.h | append RecordClip | any lane appending an action (order = merge order) |
| src/ui/BindingOverlay.cpp, MidiLearnOverlay.cpp | one label case | any lane adding an action |
| src/ui/ClipCell.{h,cpp} | setRecordMark + a paint overlay | ui (BF3 tooltip + right-click menu), bf9b (cell states?) |
| src/ui/DeckView.{h,cpp} | setRecordMark forwarding | bf9b, ui (BF8 tab rename) |
| src/ui/PreviewPanel.{h,cpp} | member + resized() | none known |
| CMakeLists.txt, tests/CMakeLists.txt, tests/test_log_line_lint.cpp | new sources / targets / list entries | everyone (mechanical) |
Cross-lane notes: bf2 -- REC reads exactly what Autopilot::processFrame reads (frameSnap_), so any shift bf2 applies to that
snapshot moves take bars and bar-snapped triggers together; a shift applied only on the message thread moves neither.
bf7 -- "REC 3.2" uses bar.beat; reuse bf7's formatter if merged first. mkvidx item 3 -- improves G5's reverse row only.

## (5) GATES (Harmony runs these after each merge; bars pre-registered)

G1 Unit: full ctest, serial: all pass; count = base + the new cases (S1 +~30, S2 +~7, S3 +~4).
G2 TSan: .harmony/probe-tsan-unit.sh incl. test_clip_recorder CR4 [tsan] -> 0 reports (abort_on_error=0).
G3 Live rows, .harmony/probe-record-clip.sh (open -g ... --args --test-mode; beat clock injected through /api/inject_features
at 120 BPM unless a row says otherwise; never an Output window; ps check first):
- rc1 a press mid-bar starts on the next bar edge: status startBeat == the first injected downbeat after the press; started
  within 1 bar + 1 frame of the press.
- rc1b a press 0-20 ms before an injected downbeat starts on that downbeat; startRepeats <= 2 (INFO on the process's first take).
- rc2 a stop mid-bar ends on the next downbeat: 2 bars -> 240 +- 1 frames; ffprobe: h264, every packet key, 60/1, size even.
- rc3 alignment (THE teeth for F3): composition (quantizeMode NextDownbeat, layer transition = cut): a blue still playing in
  L1 C1, a red still in L1 C2 (both resident: 2 s settle after load); start REC and trigger L1 C2 in the same bar -> decoded
  frame 0 of the take is red (R >= 200, G, B <= 40); blue in 0 of the first 3 frames.
- rc4 landing: /api/composition shows a Video clip at (deck on screen, top layer, first empty column) whose mediaFile ==
  status.lastTake.file within 1.5 s of the stop edge; message-thread heartbeat max gap during landing <= 50 ms.
- rc5 playable at once and true to the picture: a 1-bar take of a still -> trigger it -> render_frame (complete) vs a
  render_frame of the original still: mean abs diff <= 4/255.
- rc6 real-time speed: after triggering, the clip playhead advances 1.0 s of content per 1.0 s wall +- 3 %.
- rc7 a full top row -> the take lands in a new last column (numColumns + 1, all layers grown).
- rc8 cancel while waiting -> phase idle, no landing, no file and no ".rec-*" left in the folder.
- rc9 Layer source: L1 green source, L3 (top) red still; REC with "layer": 0 -> frame 0 green (G >= 200), not red.
- rc10 a deck switch mid-take (REST switch_deck): Output take records on and lands on the deck now on screen; under bf9b a
  Layer take also records on; pre-bf9b it ends cut "source_gone" and lands. (Row expectation follows the merged model.)
- rc11 GL context cycle mid-take (POST /api/debug/gl_context_cycle) -> ends cut "context_lost", ffprobe-valid file, lands,
  app alive.
- rc12 canvas change mid-take (set_composition_params 1280x720 -> 1920x1080) -> the file keeps its first size; frames ==
  round(wall duration x 60) +- 1.
- rc13 quit mid-take (the async-load probe's end_quit method) -> the take is a valid file or absent, never a corrupt one;
  no crash report.
- rc14 no lock (trackerState 0) -> the take starts within 2 frames of the press.
- rc15 INFO: Output > Start Recording still records, independently (E5 is not this lane's).
G4 Perf, interleaved A/B, .harmony/probe-record-clip-ab.py (normal launch, manual BPM 120, 1080p canvas; fixture composition =
2 x 1080p30 H.264 videos looping + 1 procedural source + 2 effects; >= 5 launches per arm, interleaved; a 20 s window; arm A =
no take, arm B = an Output take spanning the window; an A/A pair FIRST to measure each metric's drift D -- any bar whose teeth
are <= D is INFO). Pass on medians: fps B >= A - 3; peak frame ms B <= A + 2.0; mean frame ms B <= A + 0.5; video_late_frames
B - A <= 5 per 20 s; heartbeat max gap B <= A + 10 ms; take integrity droppedNoSlot == 0, repeats (minus startRepeats) <= 1 %
of ticks; process CPU B - A INFO (expected ~60 % of a core). 4K canvas row (30 fps take): same metrics, INFO. Decision rule:
a failing CPU-driven bar (fps / peak / late) -> builder switches the take to ProRes 422 Proxy via VideoToolbox (F4 fallback),
re-runs G4 + G5, and the result is recorded as the codec decision.
G5 Playback of a take, probe-vupload.py scene u13 (interleaved runner, >= 5 per arm): forward uploads/s >= 58; reverse and
ping-pong >= 55 with late <= 10 per 5 s WITH mkvidx item 3 merged (without it: INFO, expected per E16); video_reverse_nonmonotonic
== 0; CPU per playing take INFO.
G6 VISUAL WORK GATE (S3, before Boris sees anything): REST drives each state on a still composition; captures by Quartz
window id of the main window only (never full screen, never an Output window): REC idle (Output), idle (Layer, none
selected -> disabled), Starts at bar, REC 2.3, Ends at bar, Saving, the grid mark Armed / Live / Saving, the landed cell.
Decoded captures go to a critic panel: visual-design, UX, graphic-design and logic critics, plus a dedicated interaction-logic
critic for the press table (F11), the marks and the Layer refusal. Fixes until clean; then an artifact page for Boris
(captures + one paragraph per state + the 5 questions).
G7 Regression: probe-video.sh, probe-vupload.sh, probe-media-open.sh (fence_black_frames 0), probe-async-load.sh (a4:
a landing during a staged load waits), probe-outputs.sh (SurfacePool shared), probe-capture.sh -- unchanged results.

## (6) DOCS

- docs/claude/recording.md: NEW section "### Record to Clip (BF1, s-rta-1002b)" -- sources (F5), the bar rule (F3), the
  frame clock (F2), codec / rate (F4), files (F8), landing (F6 / F7), changes during a take (F9), press table (F11), status
  fields, REST / binding, the thread model, guards. Trigger-table row: append "or Record to Clip".
- docs/claude/integration.md: REST list +2 (41 -> 43 routes); the Video Recording paragraph says the menu recorder and Record
  to Clip are separate and records E5 as a known issue.
- docs/claude/pitfalls.md: NEW "Pitfall NN" (Harmony assigns; next free 64): "A recorded frame is picked by the wall-clock
  tick, never one per render frame; a take's bars are the frame's own snapshot -- the autopilot's totalBeatCount delta with
  beatInBar 0, in the recorder's own baseline; the stop frame is not in the take; the GL side of the tap never locks or
  waits (Pitfall 25)". AMEND Pitfall 56 (`open()` also on ClipRecorder's finalize thread, an unpublished player).
- CLAUDE.md (24,002 bytes; net change <= 0, measured with wc -c before / after): add ", record-to-clip on the bar" to Key
  capabilities (~27 B), the Pitfall NN index line (~130 B), "or Record to Clip" to the recording.md trigger row (~18 B); pay by
  moving the 18-category source breakdown "(3D 24, Geometric 11, ... Routing 1)" (~200 B) to APP-INVENTORY section 2,
  replaced by "(per-category counts: APP-INVENTORY section 2)" -- Harmony may pick another payment.
- .harmony/APP-INVENTORY.md: PreviewPanel row (REC + Output / Layer switch), ClipCell row (record mark), counts (REST 41 -> 43,
  binding actions 21 -> 22, tests), section 2 receives the source breakdown.
- BORIS_DECISIONS.md (Harmony writes; proposed text): "Record to Clip (2026-10-02, verbatim): 'whole ouput of select layer,
  stop start and snap to bar, next empty cell on top layer no need to select it, video files yes' -> REC (above the preview;
  key / MIDI / REST) records the whole output (default) or the selected layer; the take starts and ends on the next bar line;
  it lands, ready to trigger, in the first empty cell of the top layer of the deck on screen (a new column when the row is
  full); H.264 video files beside the saved composition (Documents/Audio-DNA/Recorded Clips when unsaved). REC never stops
  playback -- the no-stop model is unchanged."

## (7) RISKS (each with the cheapest test that could refute the plan)

- R1 x264 encoding during a take costs the show frames. Measured ~9.8 ms CPU per 1080p frame incl. conversion (~0.6 core at
  60 fps), 4x at 4K (INFERRED; hence 30 fps above 1440p). Test: G4. Fallback pre-registered (F4).
- R2 The tap's blit + fence stalls the GL thread. INFERRED <= 0.2 ms GPU, ~0 CPU per capture. Test: RT INFO print, then G4
  peak frame.
- R3 The take starts one frame early / late against bar-snapped triggers (wrong snapshot, wrong exit). Test: rc3 (blocking).
- R4 Reverse lateness on intra-only files at HEAD (E16). Test: G5 with / without mkvidx item 3.
- R5 Colour or range shift (BGRA -> yuv420p -> BGRA). Test: CE2, rc5.
- R6 A VideoPlayer opened on a new thread kind. Test: CR4 under TSan; the unpublished-player rule is MediaOpener's (E13).
- R7 The landing hitches the message thread (fence + grid rebuild). Test: rc4 heartbeat gap.
- R8 Disk full / unwritable folder mid-take. Test: CE5; the take ends with an error, the temp is removed, nothing lands.
- R9 Canvas resize mid-take. Test: rc12.
- R10 GL context loss mid-take (preview hidden). Test: rc11.
- R11 An immediate (unlocked) start before the encoder is open overruns 4 slots. INFERRED x264 opens in < 30 ms. Test: rc1b /
  rc14 startRepeats; fix if needed = open the encoder at app idle (pre-registered: only if rc1b fails on the second take).
- R12 bf9b changes the layer identity or the compositor's per-layer owner. Test: rc9 + rc10 on the merged model; the key is
  confined to tapClipRecorder + freshLayerOutput (I4 / I5).
- R13 Two "record" ideas confuse (the Record tab's event takes vs REC). Test: G6's UX critic; tooltips name what each writes.
- R14 The top layer is FX-only / Mask: a take still lands there (Boris's rule) and acts as that layer's content when fired.
  Test: none needed to build; visible in the grid mark; listed for Boris (8).
- R15 Loops drift: a take is N bars +-1 frame at the recording tempo and video "BPM Sync" does not follow tempo (E21). Test:
  rc2 length; drift after a tempo change is a follow-up, not this lane.
- R16 Strongest counterargument to the codec (F4): ProRes via VideoToolbox costs ~nothing to record and is the format a VJ
  expects. Why it loses: playback dominates (2.8-4.6x the decode CPU here, E15 / measurements) and stays decided by G4's
  fallback rule if recording CPU proves the bigger problem.
- R17 Strongest counterargument to the tap (F1): macOS-only. Loses: outputs and Syphon are macOS-only already; a stalling
  cross-platform readback would cost every frame of a take on the only machine Boris plays.
- R18 Sacred rule 2: two new mutexes (worker wait, status copy), both off the GL / audio / analysis threads. Needs Harmony's
  explicit OK (section 3, thread model). Test: CR4 TSan + a grep that RecordTap.cpp contains no lock.

## (8) WHAT ONLY BORIS CAN CHECK (feel / taste on the live rig)

- Where REC sits and whether "Starts at bar / REC 3.2 / Ends at bar / Saving" read at a glance mid-show.
- "On the bar" to his ear at a venue (the app's downbeat follows the analysis; bf2's sync dial may matter).
- Whether a 4-bar take loops smoothly and in time (+-1 frame per loop; no tempo follow).
- Picture quality of a take against the live output on his projector (fine fractal lines, gradients, dark scenes).
- The show stays smooth while recording on his heaviest set.
- That landing in the top row suits his layer layout (if his top layer is an effect or mask layer, takes land there too).
- A key or pad for REC (Shortcuts > Edit Keyboard / MIDI: "Record Clip"); none is bound by default.

## (9) QUESTIONS for Boris (plain words; each has a default so the build never waits)

- Q1 When a recording lands in the top row, should it go in the first empty spot from the left, or always after the last
  clip in that row? DEFAULT: first empty spot from the left.
- Q2 When a recording is done, should it just appear in its spot ready for you to play, or start playing by itself right
  away? DEFAULT: appear, ready to play.
- Q3 If you forget to end a recording, when should it end by itself? DEFAULT: after 64 bars (about 2 minutes at 128 BPM).
- Q4 If you record one layer that has see-through parts (like a logo), should the recording stay see-through? DEFAULT: no --
  see-through parts become black (smaller files, lighter playback).
- Q5 When you play a recording later, should it wait for the next bar so it lines up the way it was recorded, or start the
  moment you press it like your other clips? DEFAULT: like your other clips (your Quantize setting decides).

FOUND, not fixed (for the loose-ends ledger): E5 (the menu recorder's speed, gaps and resize read), E21 (video BPM Sync ignores
the tempo).

STATUS: COMPLETE -- plan-bf1 ready for Harmony (S1 buildable now, no shared source files; S2 / S3 at bf1's slot after bf9b).

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 16:48:04) — overrides the ruling, which overrides the plan body
1. ADOPTED: ruling-bf1.md IN FULL (17 amendments; gates G1-G7). S1 buildable now; S2 only after bf9b merges (AM9 stop
   rule); S3 after S2 + lane ui.
2. DECISION D1 (Sacred Rule 2): the two off-hot-path mutexes M1 (worker condition-variable mutex: worker + message thread)
   and M2 (status copy: worker + message tick, read by httplib) are APPROVED by Harmony — neither is reachable from the
   GL, audio or analysis thread; lint RL1 bans locks / waits / allocation on the GL tap path. D2-D4 as the ruling says.
3. BORIS QUESTIONS — defaults until he answers: Q1 first empty spot from the left; Q2 appears ready (no auto-play); Q3
   auto-end after 64 bars; Q4 see-through parts become black; Q5 plays like other clips (Quantize decides); Q6 lands in
   the deck shown when REC was pressed.
4. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not start is his (s-rta-1002b incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses, stop the batch and release. Visual work: Harmony's critic panel on decoded captures before Boris sees it. MERGE by Harmony; rebase onto whatever main is at launch.
