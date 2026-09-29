# plan-mediaopen -- media opens, drops and file stats on the message thread; the fence's black frames (s-rta-0928b START HERE item 3, part M)

Architect plan (Fable). Base: main @ 5b78d43 (every `file:line` below is at that tree, read 2026-09-28). The lane BRANCHES
AFTER `lane/seqvram` (d2b4411) merges: where seqvram moved a line this plan gives BOTH numbers ("main :N / seqvram :M").
Bins: VERIFIED (read at the cited line, or measured in diag-media.md with >= 5 launches per arm) / INFERRED (derived from
verified facts) / ASSUMED (not checked -- the builder checks it and records the result). An opus builder executes this in
a worktree; Harmony adopts or overrides after two blind attackers. Inputs read in full: diag-media.md (+ agg-iq / pool-iq /
instr_mc.py / instr_seq.py), plan-seqvram.md + its HARMONY ADOPTION, plan-renderleft.md HARMONY ADOPTION + FILED,
Pitfalls 35 / 51 / 52 / 53, rendering.md, performance-controls.md, BORIS_DECISIONS "Playback Behaviour", and the source.

QUESTION: (1) every `withDeckDetached` fence shows >= 1 deck-less BLACK frame on every output (1 per load, 2 per image
drop, 5-14 per video drop); (2) a video drop opens the file inside the fence; (3) a composition / deck load opens every
video + makes its thumbnail on the message thread (16 x 4K = 1.58 s UI freeze); (4) a sequence open stats 300 files and
decodes frame 0 twice; (5) a stat per image layer per GL frame and per media-cell paint; (6) `Clip::fromVar` reads
"speed" unguarded (a file without it loads frozen). Decide and specify each, with RED-first gates.

APPROACH (stated first):
- FENCE: a frame that finds the renderer FENCED and deck-less HOLDS the canvas exactly as the previous frame left it
  (no clear, no composite, no capture answered): the outputs keep the last composited picture. Two counters
  (`fence_hold_frames`, `fence_black_frames`) make the defect RED by value on a counters-only commit (13-14 black on a
  4K video drop) and the fix GREEN (0).
- DROPS: every drop handler PREPARES outside the fence (mint the id, open the media under it, thumbnail, dims) and
  COMMITS inside it (`before` snapshot + `deck->setClip`). A fence holds for 1-2 frames whatever was dropped.
- THUMBNAIL: `VideoPlayer::open` makes the 90x72 thumbnail itself (sws_scale straight to the thumbnail size, from the
  first frame it already decodes) BEFORE the player is published; `getThumbnail` returns that image (no per-pixel loop
  over the full frame -- 31 % of a load -- and no read of `frameBuffer_` under the GL thread's writes: FNF 5 closed).
- LOADS: open-THEN-swap, asynchronously. `loadComposition` / `appendDeckFromFile` / `duplicateDeck` stage the model as
  today, hand every video open to a 2-thread `MediaOpener`, and run their swap / command push when the LAST player has
  landed. Thumbnails and dims are written into the STAGED clips only (the MainComponent.cpp:3008-3014 constraint holds
  by construction: nothing live is written). The audience sees exactly today's picture (the old composition until the
  new one's players are ready, then the cut -- now without the black frame); the UI stays live; the file label says
  "Loading <name>...". An EMPTY batch (images / sequences / sources only) completes synchronously inside the call, so
  every existing probe's timing is unchanged. The ready player is handed over through `Renderer::adoptVideoPlayer` --
  the seam part V's decode thread plugs into.
- SEQUENCES (after seqvram): `ImageSequence::open` does NO file I/O -- no frame-0 decode (`frameBytesHint_` = 0 until the
  first upload, seqvram H5), no stats (a missing frame decodes as Failed and the previous frame repeats, renderleft
  R-7); the sequence thumbnail comes from `ClipThumbnails` keyed by its first file (Pitfall 51's store, off-thread);
  `seekTo` becomes an atomic request consumed at the top of `advanceFrame` on the GL thread (VideoPlayer's shape).
- PRESENCE: `Clip::mediaMissing` (runtime bool) is set by a 1 Hz off-thread sweep (`MediaPresence`) and seeded at load /
  drop; the compositor's 6 sites and `ClipCell::paint` read it -- zero stats on the GL thread and per paint. A file
  deleted mid-show is treated as missing within <= 1 s (was: next frame); a textual lint ctest keeps the hot paths free
  of `existsAsFile` / `ImageFileFormat::loadFrom`.
- SPEED: fix -- a missing "speed" loads the struct default 1.0 (Clip.h:121), like every other guarded field.
- GATES: counters + a TEST-ONLY heartbeat (`peak_message_stall_ms`) + a TEST-ONLY drop endpoint FIRST (commit 1, no
  behaviour change) so the rows are RED BY VALUE; 6 ctest files (2 lints, 2 pure state machines, seek/open semantics,
  speed); live probe `.harmony/probe-media-open.{sh,py,json}` modelled on probe-image-load, 10 rows with predicted RED.

## 1. TODAY'S CODE, RE-DERIVED (file:line at 5b78d43)

F1 (VERIFIED) The fence. `UndoService::withDeckDetached` (`src/core/UndoService.cpp:34-97`): `saved = getActiveDeck()`
   (:65), `setActiveDeck(nullptr)` (:66), RAII `restoreDeck` re-resolves `composition->getActiveDeck()` on scope exit
   (:70-83), `executeOnGLThread([]{}, true)` drains one in-flight frame (:89), then `mutation()` (:91-92). The GL thread
   is NOT blocked during the mutation: it renders frames with `activeDeck_ == nullptr`. `Renderer::renderOpenGL`
   (`src/render/Renderer.cpp:283`) loads the deck at :357-358 and, with no source and no deck, runs `resolveLegacy()`
   (:451-452) and the "nothing to render" path (:455-462: `publishToOutputs` + `processPendingCapture` + `presentCanvas`
   of the canvas CLEARED to black at :445-449) -- or shows the legacy single image if one is resident (:451, :455).
   Measured (diag S2): 1 black frame per load (5/5), 0-2 per image drop, 5-6 per 1080p video drop, 13-14 per 4K video
   drop (the open runs inside the fence: 106.6 ms). `setActiveDeck(nullptr)` has ONE writer: this fence (grep of
   src/: every other call passes `composition_.getActiveDeck()`, MainComponent.cpp:652, :4715, :5087, :5133, :7150,
   :7189; none in src/api, src/test, src/ui).
F2 (VERIFIED) What the canvas holds at the top of a frame: the previous frame's final picture. `juce::OpenGLHelpers::clear`
   at :343 clears the WINDOW framebuffer only; the canvas (`canvasFBO_`) is cleared at :445-449, after `ensureCanvasFBO`
   (:439). The deck-transition block (:411-437) relies on exactly this ("only here does the canvas still hold the
   previous frame", :405-410). `presentCanvas` (:1014-1043) draws `canvasTex_` letterboxed; `publishToOutputs`
   (:1046-1052) copies the canvas into the shared frames when an output is live; the recorder (:964-968), Syphon (:973-974)
   and `processPendingCapture` (:977) run only on the full path -- the two early returns (:455-462, :764-770) call
   publish + capture + present only.
F3 (VERIFIED) Drops. `applyFileDrop` (`src/MainComponent.cpp:4847-4898`): mints `clip.id = s_nextClipId++` (:4870),
   for a video `renderer.openVideoForClip` + `getVideoPlayer` -> `hasAlpha / clipWidth / clipHeight / thumbnail =
   getThumbnail(90, 72)` (:4882-4892), then `deck->setClip` (:4895). `handleFileDrop` (:4900-4919) runs it INSIDE
   `withDeckDetached` (:4910). `applyMultiFileDrop` (:4921-4969): sort + name, `ImageFileFormat::loadFrom(files[0])`
   for the thumbnail (:4957-4959), `openImageSequenceForClip` (:4964), `setClip` (:4966); fenced by
   `handleMultiFileDrop` (:5026) and, for 2 images, the spread loop (:4991-5003). The multi-video (:797-841) and mixed
   (:851-925) handlers grow columns and call `applyFileDrop` / `applyMultiFileDrop` inside ONE fence each (:809, :871).
   `ClipCell::filesDropped` (`src/ui/ClipCell.cpp:275-347`) splits images / videos by extension, sorts the videos,
   and reaches these through DeckView's callbacks (`src/ui/DeckView.cpp:200-207`, `MainComponent.cpp:789-796`).
F4 (VERIFIED) Loads. `loadComposition` (:2969-3037): STAGE -> VALIDATE -> RE-MINT -> RECONCILE -> `openMediaForDeck`
   per staged deck (:3017-3018) -> NAME -> `swapCompositionModel([this,&incoming]{ composition_ = std::move(incoming); })`
   (:3029) -> LABEL. Step 4's comment (:3008-3014) states the constraint: thumbnails / dims must be written into
   `incoming` BEFORE the swap (a post-swap write into a live Clip races the GL thread). `openMediaForDeck` (:2913-2946):
   per playable clip -- Video: `existsAsFile` (:2924), `openVideoForClip` + `getThumbnail(90, 72)` (:2925-2933);
   ImageSequence: `openImageSequenceForClip` + `ImageFileFormat::loadFrom(sequenceFiles[0])` for the thumbnail
   (:2939-2942). `appendDeckFromFile` (:3112-3190) and `duplicateDeck` (:3333-3355) share it and push an `InsertDeckCmd`
   (:3176-3178, :3347-3349); `swapCompositionModel` (:2834-2874): `routineEngine_.stopAll()`, ids before, fence, ids
   after -> `closeMediaForClip` per retired id, `postImageSet`, `undoManager_.clear()`, `refreshUiAfterModelSwap`.
   REST `/api/load_composition` (`src/api/ApiServer.cpp` handleLoadComposition) validates a throwaway copy, posts
   `onLoadComposition(f)` through `callAsync`, and answers ok BEFORE the swap (already asynchronous to its caller).
   Measured (diag S2): 16 x 1080p 423 ms, 16 x 4K 1583 ms; NO_THUMB 293 / 1108 (the thumbnail is 31 %).
F5 (VERIFIED) `VideoPlayer::open` (`src/media/VideoPlayer.cpp:32-193`): under `ffmpegMutex_`, FFmpeg open / stream info /
   codec (thread_count 2) / sws to RGBA / `frameBuffer_`, then `decodeNextFrame(); convertFrameToRGBA(); frameReady_ =
   true;` (:178-180), `open_ = true` (:182). `getThumbnail` (:380-402): a per-pixel `setPixelColour` loop over the WHOLE
   `frameBuffer_` then `rescaled`; measured 7.9 ms (1080p) / 31.9 ms (4K); it reads `frameBuffer_` that
   `convertFrameToRGBA` writes on the GL thread once the player is published (FNF 5). `~VideoPlayer` = `close()` +
   `releaseGL()` (:26-30); `releaseGL` deletes only when `texture_ != 0` (:370-378) -- a never-uploaded player holds no
   GL object. `Renderer::openVideoForClip` (`Renderer.cpp:1432-1453`): make + `open` + `closeMediaForClip(id)` (retire
   the old holder) + insert under `videoPlayerMutex_`. `getVideoPlayer` (:1535-1540) locks the same mutex, which
   `syncMedia` (:1561-1651 video branch) holds across the decode (FNF 2, part V).
F6 (VERIFIED) Sequences. `ImageSequence::open` (`src/media/ImageSequence.cpp:17-81`, same lines at seqvram): a stat per
   file (`if (f.existsAsFile()) files_.push_back(f)`, :32), a frame-0 decode for `width_/height_` (:57-67; seqvram
   :55-67 also seeds `frameBytesHint_` there), the "Opened" log with dims (:76-78). `getWidth/getHeight` have no caller
   outside the class (grep: only `player->getWidth()` at MainComponent.cpp:4888/:6555). `getThumbnail` (:351-367;
   seqvram :464) has NO caller. `seekTo` (:115-123) writes `currentTime_ / currentFrameIndex_ / playheadPosition_` from
   the message thread (callers MainComponent.cpp:1503, :4301, :4327) while the GL thread reads / writes them in
   `advanceFrame` (:125-194) and `getCurrentTexture`; the GL thread's own wrap seek is `Renderer.cpp:1709` (video's
   :1637 is already a deferred request: `VideoPlayer::seekTo` :212-217 -> consumed at :224-235 / :246-254).
   seqvram (d2b4411) changed `getCurrentTexture` / slots / grant and KEPT :32 and :55-67; its H5 makes
   `frameBytesHint_ == 0` mean "unknown -> the floor" until the first upload, so the frame-0 decode is dead weight
   there (plan-seqvram section 5, first bullet). Measured (diag S3): two 300-frame loads 128 ms (frame-0 decode 55.7 +
   second decode 40.8 for the PNG clip; 300 stats 1.5); a 300-file drop 27.1 ms inside the fence.
F7 (VERIFIED) Stats on hot paths. GL thread: `CompositorEngine.cpp:1076, :1186, :1234, :1238, :1342, :1583`
   (`clip->mediaFile.existsAsFile()` guarding `getKeyTexture` / `clipHasContent` / `getClipTexture`); measured 4.2 us
   median, 1.0-1.55 ms outliers while videos decode, 0.023 ms per frame at 4 layers. Message thread: `ClipCell.cpp:206`
   (the "!" indicator) at 30 Hz x visible media cells = 700 stats/s (4.3 ms/s). Every other `existsAsFile` in src/render
   is a load-time path (PngWrite.h, ShaderManager.cpp, LUTLoader.cpp). `ImageTexCache::Cache` already knows Resident /
   Pending / Failed per image path (`ImageTexCache.h:34, :50-70`): a missing image decodes as Failed -> tex 0 -> the
   FX-only branch (`CompositorEngine.cpp:1108-1116`), the same branch today's `!existsAsFile()` reaches.
F8 (VERIFIED) Thumbnails in the grid. `ClipCell::updateThumbnail` (`ClipCell.cpp:395-420`) never decodes: `Clip::thumbnail`
   when valid (video / sequence), else for an IMAGE `thumbs_->get(mediaFile)` (`ClipThumbnails`, `src/ui/ClipThumbnails.h`,
   path + mtime, a 2-thread low-priority pool, `onLanded -> DeckView::refresh()` `DeckView.cpp:5`). `LayerStrip::updateThumbnail`
   (`LayerStrip.cpp:939-975`) the same rule (:949-950). A sequence has no path there (type != Image), so its picture
   exists only if `Clip::thumbnail` was filled by the message-thread decode of F4/F3.
F9 (VERIFIED) `Clip::fromVar` (`src/model/Clip.cpp:213-216`) reads `transportMode / loopMode / speed / reverse`
   unconditionally: a JSON without "speed" -> `var()` -> 0.0 (default in `Clip.h:121` is 1.0f). `clipWidth`, `fitMode`,
   `inPoint` etc. are `hasProperty`-guarded (:239-256). `toVar` writes "speed" (:67). The backcompat test
   (`tests/test_composition.cpp:516-550`) asserts struct defaults for missing keys -- not speed. BORIS_DECISIONS
   "Playback Behaviour" (:320-357) has no ruling on speed; probe-deck-clock.json `_doc` and probe-image-load.py:189
   work around the defect by always writing 1.0. seqvram H16 filed it to the ledger for THIS lane.
F10 (VERIFIED) REST shapes to mirror: TEST-ONLY build path on the production port = `#if AUDIODNA_TEST_SERVER` in
   `ApiServer.cpp:283-288` (`/api/debug/stall_message_thread` -> `handleDebugStallMessageThread` :1601-1616, declared
   under the same `#if` at `ApiServer.h:209-210`); a fire-and-forget handler = `callAsync` + `jsonOk()`
   (handleTriggerClip); MainComponent wires callbacks at :1888-2117 and `apiServer_->start()` at :2118; a state
   provider hook = `setOutputsStateProvider` (`ApiServer.cpp:1343-1344`, `TestServer.h:73`, `TestServer.cpp:667-668`).
   `/api/state` blocks: `ApiServer.cpp:1297-1330` (`master_level` :1330), `TestServer.cpp:619-651` (:651).
   `/api/composition` per clip: `ApiServer.cpp:392-405` (`sourceType` :404). ctests link neither MainComponent.cpp,
   ApiServer.cpp, UndoService.cpp nor VideoPlayer.cpp (grep tests/CMakeLists.txt): those paths are proven LIVE.
F11 (VERIFIED) Test scaffolding: a source lint ctest = `test_shader_param_lint` (`tests/CMakeLists.txt:2456-2460`,
   `AUDIODNA_SRC_DIR`); a pure test block = `test_frame_ring` (:2327-2331); `test_composition` links Clip.cpp /
   Layer.cpp / ConnSerialization.cpp / UndoManager.cpp (:359-388); `test_deck_thumbnails` links DeckView / ClipCell /
   LayerStrip headless with a counting decoder (:2549-2561; cases B0-B2); many targets link `juce::juce_opengl`.
   Rig: `.harmony/.reports/s-rta-0928/gate-scripts/lock.sh` (acquire_lock / wait_quiet / quit_app / outwins /
   start_app), probe scaffolding `.harmony/probe-image-load.sh` (71 lines; `IMGLOAD_APP` / `_PY` / `_ENV`), its .py
   helpers (load / trig / state / comp_state / wait_active / cap / img / dbox / Poller / wait_no_compiler / la),
   `ffmpeg` 8.0 at /opt/homebrew/bin (probe-deck-clock.py:291-299 already builds fixtures with it). CLAUDE.md is
   24,482 B of 25,000 (seqvram adds one ~150 B index line at merge); pitfalls.md ends at 53 (:115-117; seqvram adds 54).
F12 (VERIFIED) Undo commands: `SetClipCmd::apply` (`src/core/ClipCommands.h:70-125`) runs `cell = *state` then
   `mediaHook_(*state)` INSIDE the fence (`runFenced`); `makeClipMediaHook` (`MainComponent.cpp:4632-4658`) reopens a
   video only when missing or a different file (`needsVideoReopen`, `src/core/MediaReconnect.h`), so a player opened
   BEFORE the command's execute is found and kept. `makeClipMediaDisposeHook` (:4660-4689) closes by id after a
   liveness scan. `Clip` copies travel by value (`snapshotCell`, `CellEdit`, `MainComponent.h:191-196`).
F13 (VERIFIED) `DeckClock::tick` / persistent layers / autopilot for other decks run only under `if (deckActive)`
   (`Renderer.cpp:666-716`): a deck-less frame already skips them (rule 15's clocks pause for the fence's 1-2 frames
   today too; `realDt` is measured from `lastFrameTimestampMs_` :640-643 and clamped to 0.25 s, so the first frame
   after a hold advances the clocks by the held time).
F14 (VERIFIED) `VideoPlayer::open` has no thread assumption beyond "one thread at a time on this object" (`ffmpegMutex_`,
   FFmpeg contexts private to the player; `sourceFile_` written at :34 and read by `getFile()` on the message thread
   after publication). Today it runs concurrently with the GL thread's decodes of OTHER players (message thread vs GL).

## 2. RULINGS (every design fork)

R-1 The fence HOLDS the canvas; it does not keep the old deck drawable, and it does not need a second model.
   Rejected: (a) rendering the old deck during the mutation is the very UAF the fence exists for (the mutation
   reallocates `layer.clips` / `decks`); (b) a double-buffered model or a deferred-mutation queue is a rewrite of every
   command. The canvas already IS "the previous frame's FBO" (F2): holding it costs one `if` and changes no pixel of any
   composited frame. A hold does not answer a `render_frame` (the canvas is not this frame's picture; the fence lasts
   1-2 frames, the 5 s timeout is the backstop) and does not feed the recorder / Syphon (the two existing early returns
   do not either, F2) -- a snapshot / recording sees the frame before and the frame after, never a black one.
R-2 The fence is an EXPLICIT flag (`Renderer::setFenced`), not "deck null while a composition exists". A future writer of
   `setActiveDeck(nullptr)` would otherwise freeze the output forever; with the flag it shows today's black frame and the
   counter names it. The flag is set BEFORE the deck is nulled and cleared AFTER it is restored; the GL thread reads the
   flag before the deck and re-reads it once when the deck is null (section 4.1 spells out the happens-before).
R-3 Drops open BEFORE the fence, synchronously on the message thread (prepare / commit split). Rejected: an
   asynchronous drop (the cell would fill ~100 ms after the gesture; the undo entry would be pushed by a completion;
   a Clear in between would race it). A drop's UI hold stays 18-70 ms per video (open + a ~1-3 ms thumbnail) -- the
   same class as today minus the thumbnail -- and the output holds for 1-2 frames instead of 5-14 black ones. The
   `MediaOpener` can take multi-video drops later (named follow-up), not here.
R-4 The video thumbnail is made INSIDE `open()`, before publication, and cached. This is the cheap step (sws straight to
   the thumbnail size; INFERRED 0.3-1.5 ms against 7.9 / 31.9 ms) AND the fix for FNF 5 (nothing reads `frameBuffer_`
   after publication). Rejected: a thumbnail decoded by `ClipThumbnails` from the video file (a second FFmpeg open per
   cell, and ClipThumbnails is an image store); a thumbnail from the GL texture (GL-thread work for a UI picture).
R-5 Loads are asynchronous, open-THEN-swap (A), not swap-then-open (B). (B) would show a fresh deck whose video clips
   have nothing to hold (a fresh deck has no previous picture: Pitfall 53's hold draws nothing) -- black cells for
   18-100 ms per clip -- strictly worse than today for the audience. (A) reproduces today's picture exactly (old
   composition -> cut) and removes the freeze. Boris feel: unchanged look, live UI, "Loading <name>..." in the file label.
   An empty batch completes synchronously (no message-loop hop), so image / sequence / source compositions -- every
   existing probe -- swap on the same call as today. The cheap step alone (R-4) leaves 16 x 4K at ~1.1 s of freeze
   (1583 - 475 of thumbnails, F4): not enough, hence (A).
R-6 The staged model is written by completions, never the live one: `StagedLoad` owns `incoming` until the swap; a
   landing writes `hasAlpha / clipWidth / clipHeight / thumbnail` into the STAGED clip found by id and adopts the player
   under that (re-minted, not yet live) id. No side table, no fence for the writes -- the MainComponent.cpp:3008-3014
   constraint is met by construction. Cancellation: a new staged begin, ANY `swapCompositionModel` (New, REST load) or
   destruction cancels the pending batch: its adopted players are closed through `closeMediaForClip` (the retire list),
   late-arriving players are destroyed on the message thread (no GL texture, F5).
R-7 `Renderer::adoptVideoPlayer(id, unique_ptr<VideoPlayer>)` is the publication seam; `openVideoForClip` becomes
   make + open + adopt. Part V hands a ready player (with its decode thread) through the same call (section 5).
R-8 Sequences: no I/O in `open()`. A missing file stays in `files_` as a frame that decodes Failed (the previous frame
   repeats, renderleft R-7 / seqvram) instead of silently shortening the sequence (today :32 drops it: the timing of
   every later frame shifts). A sequence of all-missing files opens with N failed frames: `getCurrentTexture` returns 0
   with `*pending == false` (a failed current frame and nothing shown, seqvram step 6) = "no media", exactly what a
   failed `open()` gives today (no sequence -> `syncMedia` 0). `width_/height_` go (no reader); `getThumbnail` goes
   (no caller); the "Opened" log loses its dims.
R-9 `ImageSequence::seekTo` = request + consume, VideoPlayer's shape (`seekRequested_` / `seekTarget_`), consumed at the
   top of `advanceFrame` BEFORE the `playing_` check (a cue jump on a paused sequence must move the frame; the clock-only
   off-screen path calls `advanceFrame` too, so an off-screen seek lands as well). Parity note: the GL thread's own wrap
   `seq->seekTo(inPoint)` (`Renderer.cpp:1709`) now lands one frame later -- exactly video's behaviour at :1637 today.
   `clip->playheadPosition = inPoint` (:1710) stays immediate. seqvram H4 asked for exactly this here.
R-10 Presence = a per-clip runtime flag written by a 1 Hz off-thread sweep, semantics preserved (a missing file ->
   `clipHasContent` false / FX-only, as the stat gives today) at <= 1 s latency. Rejected: dropping the stats and
   trusting `ImageTexCache` ("resident implies exists") -- it changes semantics (a deleted file's resident texture would
   keep showing until the next swap) and gives `ClipCell::paint` nothing to read; it also makes a deck whose only clip
   is a missing image count as content (no legacy fallback). The flag is a plain bool in `Clip` (the house class of
   message-thread-written fields the GL thread reads: `clip->playing`, `layer.visible`); copies carry it; not serialized.
   Seeded synchronously on the STAGED decks at load and on a prepared drop, so the first frames behave as today.
R-11 The heartbeat is TEST-ONLY and opt-in (`POST /api/debug/heartbeat {"on":true}`), period 4 ms (ASSUMED), because a
   500 Hz `callAsync` in every TEST_SERVER launch would sit under every other probe's timing rows. Its peak
   (`peak_message_stall_ms`, reset on read) is read while the message thread is frozen (the HTTP thread reads an
   atomic), so a 1.6 s freeze is recorded when the ping finally runs -- the poller keeps the max.
R-12 Lints, not dead counters, for "no decode / no stat on a hot path": a permanent counter at a site this plan deletes
   would have no incrementer. `test_hot_thread_io_lint` FAILs on main by hit count (loadFrom: MainComponent.cpp x2 +
   ImageSequence.cpp x2; existsAsFile: CompositorEngine.cpp x6 + ImageSequence.cpp x1 + ClipCell.cpp x1) and is GREEN
   after; the live rows prove the effect (stall, fence counters, pixels).

## 3. TRADEOFFS CONSIDERED (rejected, with why)
- Blocking the GL thread for the mutation (a mutex around the model): breaks Sacred Rule 4 (the render thread never
  waits) and would stall the outputs for the mutation's duration -- the hold gives a still frame instead of a stall.
- Making `/api/load_composition` wait for the swap: today it answers before the swap (F4); probes already poll. Adding a
  `media.opens_pending` state field and a `thumbnailW` per clip gives them a witness without changing the contract.
- One shared `ImageDecode::Decoder` for video opens: 3 low-priority threads busy with image decodes would delay a
  user-initiated load by seconds under prefetch; a 2-thread normal-priority `MediaOpener` (ASSUMED priority; Q4).
- `MediaPresence` stats on the message thread at 1 Hz (24 stats/s): still I/O on the UI thread, and a network volume
  would stall it; the sweep runs on its own low-priority thread and posts results.
- A `frames_without_deck` counter derived from `composition_->decks` on the GL thread: reading the decks vector during
  the mutation is the race the fence prevents; the flag + two counters need no model read.

## 4. DECISION / SPEC

### 4.0 Builder step 0 (rig)
- `df -h /System/Volumes/Data` first (8 GB/lane + 20 GB; max 3 build lanes); worktree branched from main AFTER
  `lane/seqvram` is merged (`git log --oneline -1 -- src/media/SeqVram.h` must show the seqvram commit); `LANE=mediaopen
  . <gate-scripts>/lock.sh`; `open -g` only; quit via `quit_app`; NEVER the Output window; no lldb / dtrace / sample; no
  synthetic input; HTTP `Connection: close`; >= 5 runs per arm for any flake verdict; TEST_SERVER-only endpoints for
  every new endpoint. Build Release with `-DAUDIODNA_BUILD_TEST_SERVER=ON`; keep a signed copy of the app after every
  commit (`MAIN` = the untouched post-seqvram main build for RED runs). Record before any change: ctest count (846 +
  seqvram's), probe-image-load 37/0, probe-crossfade 35/0, probe-seq-vram GREEN, probe-deck-clock GREEN.
- Every mutation used as teeth: record the FAIL line, restore, `sleep 1; touch` the file before rebuilding (notebook).

### 4.1 The fence holds the canvas (`src/render/Renderer.h/.cpp`, `src/core/UndoService.cpp`)
`Renderer.h`:
- after :111 (`getActiveDeck`): 
  ```cpp
  // s-rta-0928b mediaopen: UndoService::withDeckDetached marks its fence BEFORE it nulls the active deck and unmarks it
  // AFTER it restores the deck. A frame that finds the renderer fenced and deck-less HOLDS the canvas as the previous
  // frame left it (no clear, no composite, no capture): the outputs never show the deck-less black frame.
  void setFenced(bool fenced) { fenced_.store(fenced, std::memory_order_release); }
  ```
- after :391 (`takePeakCallbackMs`): `int64_t getFenceHoldFrames() const; int64_t getFenceBlackFrames() const;` (relaxed loads).
- after :517 (`activeDeck_`): `std::atomic<bool> fenced_{ false }; std::atomic<int64_t> fenceHoldFrames_{ 0 }, fenceBlackFrames_{ 0 };`
  with the comment: hold = a fenced deck-less frame that re-presented the canvas; black = one that could not (no canvas
  yet) and fell to the "nothing to render" path -- the pre-fix behaviour, kept countable.
`Renderer.cpp`:
- :357-358 become:
  ```cpp
  // s-rta-0928b mediaopen: the fence flag is read BEFORE the deck. withDeckDetached stores the flag, then the null deck
  // (both release): a null deck seen with the flag unseen means the fence began between the two loads -- the acquire that
  // saw the null synchronises with its store, which is sequenced after the flag store, so a second load sees the flag.
  // The fence clears the flag only AFTER it restores the deck, so "flag seen, deck restored" is an ordinary frame.
  bool fenced = fenced_.load(std::memory_order_acquire);
  Deck* deck = activeDeck_.load(std::memory_order_acquire);
  if (deck == nullptr && !fenced)
      fenced = fenced_.load(std::memory_order_acquire);
  const bool deckActive = (deck != nullptr);
  ```
- after :371 (`const FeatureSnapshot& snap = frameSnap_;` -- the bus read stays before the return, as the comment at
  :361-370 requires), before the canvas block comment at :373:
  ```cpp
  // s-rta-0928b mediaopen: inside a withDeckDetached fence the model is being mutated -- hold the canvas exactly as the
  // previous frame left it (F2: nothing has touched canvasFBO_ yet this frame), re-present it, re-publish it to the
  // outputs. No capture is answered (a held frame is not this frame's picture; the fence lasts 1-2 frames), no recorder
  // / Syphon frame (as the two existing early returns), no deck-transition detection (the first unfenced frame detects
  // it with the held picture as the outgoing one), no canvas-size debounce step. It used to fall to the "nothing to
  // render" path and show one black frame per fence on every output (5-14 on a video drop, diag-media S2).
  if (fenced && !deckActive)
  {
      if (canvasTex_ != 0 && canvasW_ > 0 && canvasH_ > 0)
      {
          fenceHoldFrames_.fetch_add(1, std::memory_order_relaxed);
          GLint heldFBO = 0;
          glGetIntegerv(GL_FRAMEBUFFER_BINDING, &heldFBO);
          auto* heldComponent = glContext_.getTargetComponent();
          const float heldScale = static_cast<float>(glContext_.getRenderingScale());
          const int heldW = heldComponent != nullptr ? static_cast<int>(heldComponent->getWidth() * heldScale) : 1;
          const int heldH = heldComponent != nullptr ? static_cast<int>(heldComponent->getHeight() * heldScale) : 1;
          publishToOutputs(canvasW_, canvasH_);
          presentCanvas(static_cast<GLuint>(heldFBO), RenderGeometry::fitCanvas(canvasW_, canvasH_, heldW, heldH));
          return;
      }
      fenceBlackFrames_.fetch_add(1, std::memory_order_relaxed);   // nothing to hold: today's black frame, counted
  }
  ```
  (`publishToOutputs` asserts the current context and copies `canvasFBO_` -- :1046-1052; `presentCanvas` binds
  `windowFBO` and draws `canvasTex_` -- :1014-1043; both are already used on the early returns.) Commit 1 ships the
  counters WITHOUT the hold (the `if` body is only the black counter) so the RED numbers are real: the counter then
  counts every fenced deck-less frame.
`UndoService.cpp` :65-66 become:
  ```cpp
  Deck* saved = renderer_->getActiveDeck();
  // s-rta-0928b mediaopen: mark the fence BEFORE the deck is nulled; the flag guard is declared BEFORE restoreDeck so its
  // destructor runs AFTER the deck is restored (reverse construction order): the GL thread never sees "no deck, no flag".
  renderer_->setFenced(true);
  struct FenceFlagGuard { Renderer* renderer; ~FenceFlagGuard() { renderer->setFenced(false); } } fenceFlag{ renderer_ };
  renderer_->setActiveDeck(nullptr);
  ```
  (`UndoService.h:80-89` comment: add one sentence naming the flag.) No other change to the fence: the drain (:89) and
  the mutation (:91-92) stay.
Invariants: the hold path touches no `LayerStateKey` history (Pitfall 35: nothing composites), no capture state
(Pitfall 52: `processPendingCapture` is not called; `lockedSize_` / `captureFlight_` untouched), no deck id (36). The
canvas-size debounce (`candW_/stableW_`, :392-398) skips held frames: a load that changes the canvas size applies it two
frames after the fence ends instead of one (INFERRED, harmless).

### 4.2 Drops: prepare outside the fence, commit inside (`src/MainComponent.h/.cpp`, `src/ui/ClipCell.h/.cpp`)
`MainComponent.h` :467-480: replace `applyFileDrop` / `applyMultiFileDrop` with
```cpp
// s-rta-0928b mediaopen: a drop is PREPARED outside the GL fence -- the id minted, the media opened under it, dims / alpha /
// thumbnail read (a video's open + thumbnail used to run INSIDE the fence: 5-14 black output frames per video drop) -- and
// COMMITTED inside it (the before-snapshot + deck->setClip, the reallocation the fence exists for). prepare* refuses exactly
// as applyFileDrop did (no active deck / content-locked cell -> nullopt, nothing opened). Nothing runs between a prepare and
// its commit (same synchronous handler), so the refusal cannot go stale.
struct PreparedDrop { int layerIndex = 0; int column = 0; Clip clip; };
std::optional<PreparedDrop> prepareFileDrop(int layerIndex, int column, const juce::File& file);
std::optional<PreparedDrop> prepareMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files);
std::optional<CellEdit> commitDrop(const PreparedDrop& prepared);   // INSIDE withDeckDetached only
```
`MainComponent.cpp`:
- :4847-4898 `applyFileDrop` -> `prepareFileDrop`: the body up to and including the video block (:4855-4893) minus the
  `setClip` (:4895); the thumbnail line reads `player->getThumbnail(90, 72)` unchanged (now the cached one, 4.3); add
  `clip.mediaMissing = false;` (a Finder drop exists -- a seed, 4.6). Returns `PreparedDrop{ layerIndex, column, clip }`.
- new `commitDrop`: `auto* deck = composition_.getActiveDeck(); if (!deck) return std::nullopt;` `before =
  snapshotCell(deck->getLayer(p.layerIndex), p.column)` (:4864 today), `deck->setClip(p.layerIndex, p.column, p.clip)`,
  return `CellEdit{ p.layerIndex, p.column, before, std::optional<Clip>(p.clip) }`.
- :4921-4969 `applyMultiFileDrop` -> `prepareMultiFileDrop`: refusal (:4927-4933), the Clip (:4938-4956), NO
  `ImageFileFormat::loadFrom` (:4957-4959 deleted: the grid pulls the sequence thumbnail from ClipThumbnails, 4.5),
  `renderer.openImageSequenceForClip` (:4964, no I/O after 4.5), return the PreparedDrop.
- Callers, each "prepare all, then ONE fence that grows columns and commits":
  - `handleFileDrop` (:4900-4919): `auto p = prepareFileDrop(...); if (!p) return; std::optional<CellEdit> edit;
    undoService_.withDeckDetached([&] { edit = commitDrop(*p); });` then the push + rebuild as today.
  - `onMultiVideoDropped` (:797-841): `std::vector<PreparedDrop> prepared; for i: if (auto p = prepareFileDrop(layerIdx,
    col + i, files[i])) prepared.push_back(std::move(*p));` then the fence: the growth loop (:815-820) + `for p:
    commitDrop(p) -> edits`. Column growth stays inside the fence (it resizes every layer's clips vector).
  - `onMixedFilesDropped` (:851-925): the same shape -- images (one prepareFileDrop / two / prepareMultiFileDrop, per
    :880-898) and videos (:901-903) prepared first; growth + commits fenced (:871-904).
  - `handleMultiFileDrop` (:4971-5037): the 2-image spread prepares both then fences growth + commits; the N-image
    path prepares one then fences one commit (:5026).
- `ClipCell.h/.cpp`: factor the extension split of `filesDropped` (:275-347) into a pure static
  `struct DropRoute { std::vector<juce::File> images, videos; }; static DropRoute classifyDrop(const juce::StringArray&)`
  (the two extension lists + the natural sort of the videos, :281-304) used by `filesDropped` AND by the TEST-ONLY drop
  endpoint (4.8) so the endpoint reaches the same handlers a Finder drop reaches through the same split.
Replace Content (:6493-6590) already opens OUTSIDE its fence (:6550-6558, fence :6575): unchanged apart from the cheaper
`getThumbnail`. A prepared drop whose commit never runs (a deck vanishing inside the same handler: impossible today) would
leak one player until quit -- stated, accepted.

### 4.3 `VideoPlayer::open` makes the thumbnail (`src/media/VideoPlayer.h/.cpp` -- these lines ONLY, part V owns the rest)
`VideoPlayer.h`: after :96 (`getThumbnail`) rewrite its comment: "The first-frame thumbnail, made inside open() BEFORE the
player is published (sws_scale straight to a 90x72-fit size from the frame open() decodes; no read of frameBuffer_ after
publication -- s-rta-0928b, FNF 5). Any thread after open() returned. maxWidth/maxHeight other than 90x72: a rescale of
the cached image." Members after :129 (`frameReady_`): `juce::Image thumbnail_;` and `static constexpr int kThumbW = 90,
kThumbH = 72;`; private `void makeThumbnail();`.
`VideoPlayer.cpp`:
- :180 (`frameReady_ = true;` in open) -> add `makeThumbnail();` right after it (still under `ffmpegMutex_`, before
  `open_ = true` :182).
- new `makeThumbnail()`: `thumbnail_ = juce::Image();` if `decodedFrame_ == nullptr || decodedFrame_->data[0] == nullptr
  || width_ <= 0 || height_ <= 0` return (a failed first decode: invalid, as today's empty-buffer return). Fit:
  `scale = std::min(kThumbW / (float) width_, kThumbH / (float) height_)`, `tw = max(1, int(width_ * scale))`, `th` likewise
  (today's math :393-397). `SwsContext* s = sws_getContext(width_, height_, codecCtx_->pix_fmt, tw, th, AV_PIX_FMT_RGBA,
  SWS_AREA, nullptr, nullptr, nullptr)`; a `std::vector<uint8_t> rgba(tw * th * 4)`; `uint8_t* dst[4] = { rgba.data(),
  nullptr, nullptr, nullptr }; int stride[4] = { tw * 4, 0, 0, 0 }; sws_scale(s, decodedFrame_->data, decodedFrame_->linesize,
  0, height_, dst, stride); sws_freeContext(s);` then `juce::Image img(ARGB, tw, th, false)` filled through
  `BitmapData(writeOnly)` with `setPixelColour(x, y, Colour(r, g, b, a))` over the tw x th thumbnail (<= 6480 pixels;
  same premultiply semantics as today's loop). `thumbnail_ = img`.
- :380-402 `getThumbnail`: `if (!thumbnail_.isValid()) return {}; if (maxWidth == kThumbW && maxHeight == kThumbH)
  return thumbnail_;` else `thumbnail_.rescaled(fit)` (the fit math on the cached size).
- `close()` (:195-218): `thumbnail_ = juce::Image();` with the other resets.
Cost (INFERRED from sws downscale throughput; the builder records `[VideoPlayer] Opened` timings with a scope): 0.3-1.5 ms.
The picture differs from today's (area filter on the native frame vs JUCE low-quality rescale of a full ARGB copy) --
a UI thumbnail, not an output pixel; Boris check listed in section 9.

### 4.4 Asynchronous loads: `MediaOpenBatch.h` (pure) + `MediaOpener` + `Renderer::adoptVideoPlayer` + the staged flow
`src/core/MediaOpenBatch.h` (NEW, pure: `<cstdint>` only; `tests/test_media_open_batch.cpp`):
```cpp
namespace mediaopen {
// The bookkeeping of the ONE live asynchronous media-open batch (s-rta-0928b mediaopen): a load / deck append / duplicate
// stages its model, issues one job per video clip and swaps when the LAST result has landed. A newer batch cancels the
// older one: its later landings are Stale. total 0 = complete at once (the caller runs its completion synchronously).
class Ledger {
public:
    uint64_t begin(int total);                   // cancels the live batch; returns the new gen (>= 1)
    enum class Landing { Apply, Stale };
    Landing land(uint64_t gen);                  // Apply: ++done for the live batch
    bool complete(uint64_t gen) const;           // gen is live and done >= total
    void cancel();                               // no live batch: landings are Stale, pending() == 0
    int pending() const;                         // total - done of the live batch, else 0
    uint64_t liveGen() const;                    // 0 = none
    uint64_t batches() const;                    // begins so far
private:
    uint64_t gen_ = 0; int total_ = 0, done_ = 0; bool live_ = false;
};
}
```
`src/core/MediaOpener.h/.cpp` (NEW; JUCE + VideoPlayer; the app only):
```cpp
// Opens VideoPlayers OFF the message thread (ClipThumbnails' pool + callAsync pattern): one job per clip; a job opens a
// NEW, unpublished player (VideoPlayer::open has no thread assumption on an unpublished object; the first frame and the
// thumbnail are made inside it) and posts it to the message thread, where onLanded writes the STAGED clip and adopts the
// player under its id; onDone runs after the last landing of the live batch. An EMPTY batch runs onDone synchronously
// inside begin(). Jobs capture no `this` (WeakReference): a landing after the owner is gone destroys the player there --
// a never-published player holds no GL object (VideoPlayer::releaseGL deletes only texture_ != 0).
class MediaOpener {
public:
    struct Job { uint32_t clipId = 0; juce::File file; };
    using Landed = std::function<void(uint32_t clipId, std::unique_ptr<VideoPlayer> player)>;   // null = open failed
    using Done = std::function<void()>;
    MediaOpener(); ~MediaOpener();               // masterReference.clear(); pool_.removeAllJobs(true, 5000) (a 4K open <= ~100 ms)
    void begin(std::vector<Job> jobs, Landed onLanded, Done onDone);
    void cancel();
    int pending() const; uint64_t batches() const;
private:
    void landed(uint64_t gen, uint32_t clipId, std::unique_ptr<VideoPlayer>&& player);
    mediaopen::Ledger ledger_; Landed onLanded_; Done onDone_;
    juce::ThreadPool pool_{ juce::ThreadPoolOptions{}.withNumberOfThreads(2).withThreadName("MediaOpen")
                                                     .withDesiredThreadPriority(juce::Thread::Priority::normal) };   // ASSUMED (Q4)
    JUCE_DECLARE_WEAK_REFERENCEABLE(MediaOpener)
};
```
`begin`: `cancel()` first; `gen = ledger_.begin(jobs.size())`; store the callbacks; if `jobs.empty()` -> `auto done =
std::move(onDone_); onDone_ = nullptr; if (done) done(); return;`. Per job: `pool_.addJob([self = WeakReference(this), gen,
job] { auto p = std::make_unique<VideoPlayer>(); if (!p->open(job.file)) p.reset(); auto box = std::make_shared<Landing>(gen,
job.clipId, std::move(p)); juce::MessageManager::callAsync([self, box] { if (auto* o = self.get()) o->landed(box->gen,
box->clipId, std::move(box->player)); }); })`. `landed`: Stale -> return (the unique_ptr dies here: message thread, no GL);
Apply -> `onLanded_(clipId, std::move(player))`; `if (ledger_.complete(gen)) { auto done = std::move(onDone_); onDone_ =
nullptr; ledger_.cancel(); if (done) done(); }` (moved out first: a completion may `begin()` the next batch).
`Renderer.h/.cpp`:
- after `openVideoForClip` (Renderer.h :215):
  ```cpp
  // s-rta-0928b mediaopen: publish a player that open()ed on another thread (MediaOpener) under a clip id -- the seam part V's
  // decode thread hands ready players through. Message thread. Retires any media live under the id first (the GL-drained
  // retire list of closeMediaForClip), then inserts; the GL thread may find it from the next frame on.
  void adoptVideoPlayer(uint32_t clipId, std::unique_ptr<VideoPlayer> player);
  ```
- `Renderer.cpp:1432-1453` `openVideoForClip` = `auto player = std::make_unique<VideoPlayer>(); if (!player->open(videoFile))
  return false; adoptVideoPlayer(clipId, std::move(player)); return true;`; `adoptVideoPlayer` = the retire + insert body
  (:1445-1452 verbatim: `closeMediaForClip(clipId)`, lock, `videoPlayers_[clipId] = std::move(player)`).
`MainComponent.h/.cpp` -- the staged flow:
- members after `undoService_` (`MainComponent.h:378`; declared AFTER `previewPanel_` :297, so destroyed BEFORE the renderer):
  ```cpp
  // s-rta-0928b mediaopen: a composition / deck load is staged, its video players opened off the message thread, and the
  // swap (or the InsertDeckCmd) runs when the last one has landed. Landings write the STAGED clips only (never a live Clip:
  // loadComposition step 4's constraint). One staged load at a time: a newer one, any swapCompositionModel or destruction
  // cancels it (adopted players closed through the retire list; late players destroyed here, unpublished: no GL object).
  struct StagedLoad { enum class Kind { Composition, DeckAppend, DeckDuplicate } kind; Composition comp; Deck deck;
                      juce::File file; std::vector<uint32_t> adopted; };
  std::unique_ptr<StagedLoad> staged_;
  MediaOpener mediaOpener_;
  void beginStagedOpen(std::unique_ptr<StagedLoad> staged);    // seeds presence, opens sequences, queues the videos
  void cancelStagedOpen();
  void finishStagedLoad();                                     // the completion: NAME / SWAP or InsertDeckCmd / LABEL
  ```
- `loadComposition` (:2969-3037): steps 1-3b unchanged; replace :3017-3037 with `auto s = std::make_unique<StagedLoad>();
  s->kind = Kind::Composition; s->comp = std::move(incoming); s->file = file; beginStagedOpen(std::move(s));`. The old
  steps 5-7 (:3020-3036) move verbatim into `finishStagedLoad`'s Composition branch (`incoming` -> `staged_->comp`).
- `appendDeckFromFile` (:3112-3190): steps 1-3b unchanged; :3157 (`openMediaForDeck(incoming)`) and the steps after
  (:3159-3189) -> `StagedLoad{ Kind::DeckAppend, deck = std::move(incoming), file }` + `beginStagedOpen`; the
  completion does NAME (:3163-3164), the `InsertDeckCmd` push (:3175-3181), `rebuildGrid`, `refreshPreviewFromActiveClip`,
  LABEL (:3186-3189).
- `duplicateDeck` (:3333-3355): the copy (:3341) then `StagedLoad{ Kind::DeckDuplicate, deck = std::move(copy) }`; the
  completion does :3345-3354 (`InsertDeckCmd "Duplicate Deck"`, rebuild, refresh, label).
- `beginStagedOpen`: `cancelStagedOpen(); staged_ = std::move(staged);` walk the staged decks (`comp.decks` or `deck`):
  Image -> `clip.mediaMissing = !clip.mediaFile.existsAsFile()` (the seed, 4.6); Video -> `clip.mediaMissing = !exists;
  if (exists) jobs.push_back({ clip.id, clip.mediaFile })` (a missing video is skipped exactly as :2924 skips it);
  ImageSequence -> `renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps)` synchronously (no
  I/O, 4.5). If `!jobs.empty()` -> `fileLabel_.setText("Loading " + name + "...", dontSendNotification)`.
  `mediaOpener_.begin(std::move(jobs), [this](uint32_t id, std::unique_ptr<VideoPlayer> p) { onStagedLanded(id, std::move(p)); },
  [this] { finishStagedLoad(); })` -- lambdas capture `this`: `mediaOpener_` is a member destroyed before `this` and its
  landings check the WeakReference, so no callback outlives MainComponent.
- `onStagedLanded(id, player)`: find the staged clip by id (a walk of the staged decks); `if (!player) return;` (as today:
  a failed open leaves the clip without dims); `clip.hasAlpha = player->hasAlpha(); clipWidth/clipHeight; clip.thumbnail =
  player->getThumbnail(90, 72);` then `renderer.adoptVideoPlayer(id, std::move(player)); staged_->adopted.push_back(id);`.
- `finishStagedLoad`: `auto s = std::move(staged_);` then the Kind branch (above); `swapCompositionModel`'s own cancel is
  a no-op then (staged_ is already null).
- `cancelStagedOpen`: `mediaOpener_.cancel(); if (staged_) { for (id : staged_->adopted) renderer.closeMediaForClip(id);
  staged_.reset(); }`. Called at the top of `swapCompositionModel` (:2834, before `stopAll`) and in the destructor before
  the servers stop (the first lines of `~MainComponent`, :2280-2290 region).
- `openMediaForDeck` (:2913-2946) is deleted (its three callers are the staged flow now).
- REST witness: `apiServer_->setMediaStateProvider([this] { return mediaStateVar(); })` and the same on `testServer_`
  before their `start()` (:2118, :1866): `mediaStateVar()` = `{ "opens_pending": mediaOpener_.pending(), "open_batches":
  batches(), "presence_sweeps": presence_.sweeps(), "presence_changed": presence_.changes() }` (4.6 / 4.8).
What Boris sees: the old composition stays on screen and keeps playing; the UI stays live and the file label reads
"Loading <name>..." for ~0.3-1.2 s (16 x 4K, ASSUMED with 2 threads: 16 x ~68 ms / 2 + landings); then the cut -- the
same moment today's frozen UI came back -- with no black frame. Image-only compositions swap on the same call as today.

### 4.5 Sequences after seqvram (`src/media/ImageSequence.h/.cpp`, `src/ui/ClipCell.cpp`, `src/ui/LayerStrip.cpp`)
`ImageSequence.cpp` (line numbers: main / seqvram where they differ):
- :32 `if (f.existsAsFile()) files_.push_back(f);` -> `files_.push_back(f);` (comment: a missing file is a frame that decodes
  Failed -- the previous frame repeats, renderleft R-7 -- never a stat on the message thread; it used to be dropped from
  the list, shifting every later frame).
- :57-67 / seqvram :55-67 (the frame-0 decode, `width_/height_`, and seqvram's `frameBytesHint_` seed) DELETED:
  `frameBytesHint_` stays 0 until the first upload = seqvram H5's "unknown -> kMinWindowFrames" (VERIFIED at seqvram's
  `getCurrentTexture` step 1: `allowanceFrames(grant.allowanceBytes, frameBytesHint_)`, `allowanceFrames(_, 0) ==
  kMinWindowFrames`). :76-78 log: drop the dims. `close()` :103-104: drop the `width_/height_` resets.
- `getThumbnail` (:351-367 / seqvram :464-480): DELETED (no caller).
- seek: `seekTo` (:115-123) becomes `seekTarget_.store(clamp(pos)); seekRequested_.store(true, release);`; at the top of
  `advanceFrame` (:125-131), after the `!open_ || files_.empty()` return (:127-128) and BEFORE the `playing_` return (:130-131):
  ```cpp
  // s-rta-0928b mediaopen: a seek is a REQUEST (message-thread callers: cue jump, beat snap, retrigger) consumed here on the GL
  // thread -- VideoPlayer's shape (seekRequested_ / seekTarget_). Before the playing check: a cue jump on a paused sequence
  // moves the frame. The off-screen clock path calls advanceFrame too, so an off-screen seek lands as well.
  if (seekRequested_.exchange(false, std::memory_order_acq_rel))
  {
      const double p = seekTarget_.load(std::memory_order_relaxed);
      currentTime_ = p * getDuration();
      currentFrameIndex_ = std::clamp(static_cast<int>(p * static_cast<double>(files_.size() - 1)), 0, static_cast<int>(files_.size()) - 1);
      playheadPosition_.store(p, std::memory_order_relaxed);
  }
  ```
  (today's :117-122, moved.) `open()` and `close()` reset `seekRequested_ = false`.
`ImageSequence.h`: delete `openDirectory`? NO (unused but harmless; out of scope). Delete `getWidth/getHeight` (:42-43),
`width_/height_` (:111-112), `getThumbnail` (:97); add `std::atomic<bool> seekRequested_{ false }; std::atomic<double>
seekTarget_{ 0.0 };` beside the transport atomics (:108); update the class comment (:24-26: "Call from message thread" ->
"any thread; does no file I/O").
`ClipCell.cpp:403-404`: the thumbnail source path = `type == Image ? mediaFile : type == ImageSequence && !sequenceFiles.empty()
? sequenceFiles[0] : File()` when `!cached`; `thumbs_->get(thatFile)` at :419. `LayerStrip.cpp:950-951` the same (the
`thumbs_->get` at :958). Pitfall 51's store now serves a sequence's first frame (path + mtime; `ClipThumbnails` unchanged).
`MainComponent.cpp:2940-2942` / `:4957-4959` (the message-thread frame-0 decodes) are gone with 4.2 / 4.4.
`Renderer.cpp:1455-1472` `openImageSequenceForClip`: unchanged (it now costs a sort + vector copy).

### 4.6 Presence (`src/core/MediaPresence.h` NEW, `src/model/Clip.h`, `CompositorEngine.cpp`, `ClipCell.cpp`, `MainComponent`)
`Clip.h` after :232: `bool mediaMissing = false;   // s-rta-0928b mediaopen: MediaPresence found mediaFile absent (Image /
Video; a 1 Hz off-thread sweep, seeded at load / drop). Read by the compositor and ClipCell::paint instead of a stat.
Runtime, not serialized.` `replaceContent` after :286: `mediaMissing = newContent.mediaMissing;` `clear()` after :339
(`speed = 1.0f;` region -- with the media resets :327-333): `mediaMissing = false;`.
`src/core/MediaPresence.h` (header-only; juce_core + juce_events; the pure functions tested in `tests/test_media_presence.cpp`
against a bare Composition like test_composition):
```cpp
namespace presence {
struct Seen { std::string path; bool exists; };
inline std::vector<std::string> mediaPaths(const Composition& c);     // Image / Video clips' non-empty mediaFile paths, deduplicated
inline int apply(Composition& c, const std::vector<Seen>& seen);      // clip.mediaMissing = !exists for matching paths; returns flags changed
inline void seed(Deck& d);                                            // clip.mediaMissing = !existsAsFile() (message thread, a STAGED deck)
}
// A 1 Hz juce::Timer (message thread) that snapshots mediaPaths(comp), stats them on ONE low-priority pool thread and applies
// the answers on the message thread (WeakReference completion); one sweep in flight at a time. onChanged fires when a flag
// flipped (DeckView::refresh repaints the "!" indicators). stats() / sweeps() / changes() for /api/state "media".
class MediaPresenceSweeper : private juce::Timer { public: void start(Composition&, int intervalMs = 1000); std::function<void()> onChanged; ... };
```
`MainComponent`: member `MediaPresenceSweeper presence_;` (after `mediaOpener_`); in the constructor after the DeckView
wiring (:660 region): `presence_.onChanged = [this] { if (deckView_) deckView_->refresh(); }; presence_.start(composition_);`.
`beginStagedOpen` seeds the staged decks (4.4); `prepareFileDrop` seeds the dropped clip (4.2).
`CompositorEngine.cpp`: a file-local `static bool mediaPresent(const Clip& c) { return c.mediaFile != juce::File() &&
!c.mediaMissing; }` (parity: `existsAsFile()` of an empty File is false); :1076, :1186, :1342, :1583 -> `clip->mediaType ==
Image && mediaPresent(*clip)`; :1234 / :1238 (`clipHasContent`) -> `mediaPresent(clip)`. No other change.
`ClipCell.cpp:206`: `clip_->mediaFile != juce::File() && clip_->mediaMissing`.
Semantics (documented, section 9): a file deleted mid-show becomes "missing" within <= 1 s + one message-loop hop (was:
the next frame); until then its resident texture / open player keeps showing (macOS keeps an open file readable). A file
that comes back is "present" within <= 1 s (a resident image shows again at once: `ImageTexCache` kept it).

### 4.7 `Clip::fromVar` speed (`src/model/Clip.cpp:215`, `tests/test_composition.cpp`)
:215 -> `if (obj->hasProperty("speed")) speed = static_cast<float>(static_cast<double>(obj->getProperty("speed")));`
(comment: an old / hand-written file without the key plays at the struct default 1.0, not frozen at 0 -- s-rta-0928b;
an explicit 0.0 is kept). `test_composition.cpp` SECTION "Clip old-format" (:521-548): add `REQUIRE_THAT(clip.speed,
WithinAbs(1.0f, 0.001f));` -- RED on main (reads 0.0). New SECTION "Clip explicit speed": `"speed": 0.0` -> 0.0;
`"speed": 2.0` -> 2.0. Ruling basis: F9 (the struct default is the file's meaning for every other guarded key; no Boris
ruling names speed). Probe fixtures keep writing 1.0 (harmless); Harmony amends the notebook line.

### 4.8 REST / TEST-ONLY additions (`src/api/ApiServer.h/.cpp`, `src/api/MessageHeartbeat.h` NEW, `src/test/TestServer.h/.cpp`)
- `/api/state` (both servers, after `master_level`: `ApiServer.cpp:1330`, `TestServer.cpp:651`):
  `fence_hold_frames`, `fence_black_frames` (int64, cumulative) from the Renderer; `media` = the provider's object
  (`opens_pending`, `open_batches`, `presence_sweeps`, `presence_changed`); ApiServer only, `#if AUDIODNA_TEST_SERVER`:
  `message_heartbeat_on` (bool) and `peak_message_stall_ms` (double, reset on read; 0 when off).
- `/api/composition` per clip (after `sourceType`, `ApiServer.cpp:404`): `mediaMissing` (bool), `clipWidth`, `clipHeight`
  (int), `thumbnailW`, `thumbnailH` (int, 0 when invalid) -- the witnesses of a landed asynchronous open and of presence.
- TEST-ONLY (ApiServer, inside the `#if AUDIODNA_TEST_SERVER` block :283-288, declared beside :209-210):
  - `POST /api/debug/heartbeat {"on": bool, "period_ms": 1..50 (default 4)}` -> `MessageHeartbeat` (a `juce::Thread`
    posting `callAsync([peak = peak_, t0 = now] { record(now - t0) })` every period; `peak_` a
    `std::shared_ptr<std::atomic<float>>` so a ping that runs after `stop()` writes a dead cell; `takePeakMs()` exchange 0).
  - `POST /api/debug/drop_files {"layer": L, "column": C, "files": ["/abs/path", ...]}` -> `callAsync` ->
    `onDebugDropFiles(L, C, files)`; MainComponent wires it to `debugDropFiles`: `auto r = ClipCell::classifyDrop(files)`
    then EXACTLY `ClipCell::filesDropped`'s dispatch (:309-347) onto `deckView_->onMixedFilesDropped / onFileDropped /
    onMultiVideoDropped / onMultiFileDropped` (the handlers a Finder drop reaches through DeckView.cpp:200-207), then
    `deckView_->clearSelection()` as :791 / :795 do.
  Both answer `jsonOk()` at once (the stall endpoint's shape); absent from a build without AUDIODNA_BUILD_TEST_SERVER.

### 4.9 ctests (RED first: write each case, watch it fail against main / the counters-only build, then implement)
Registered at the EOF of `tests/CMakeLists.txt` (pure block shape :2327-2331; lint shape :2456-2460; `test_composition`
block :359-388 for its new cases):
1. `tests/test_media_open_batch.cpp` [mediaopen][batch] (pure, RED by absence): (a) begin(3): land x3 -> complete only at
   the third, pending 3 -> 0; (b) begin(2) then begin(1): a landing for gen 1 is Stale and does not count; gen 2 completes
   after one landing; (c) begin(0) -> complete(gen) true at once, pending 0; (d) cancel -> landings Stale, pending 0,
   liveGen 0; (e) batches() counts begins.
2. `tests/test_media_presence.cpp` [mediaopen][presence] (links Clip.cpp / Layer.cpp like test_composition): (a) mediaPaths
   lists Image + Video paths once each, skips sequences / sources / empty files; (b) apply flips exactly the matching
   clips, returns the count of CHANGES (a second identical apply returns 0); (c) `Clip::clear()` resets the flag;
   `replaceContent` carries the new content's flag; (d) seed on a Deck whose file does not exist sets it, an existing
   temp file clears it.
3. `tests/test_image_sequence_open.cpp` [mediaopen][imageseq] (links `${SRC_DIR}/media/ImageSequence.cpp` + juce_opengl +
   juce_graphics + juce_events; no GL call is reached): (a) [open] three NON-EXISTENT .png paths -> open() true,
   getFrameCount() 3 (RED on main: false / dropped), and no file is created or read (a `std::filesystem::last_write_time`-free
   check: the paths still do not exist); (b) [open] a 12-path list with path 5 missing keeps 12 frames (RED on main: 11);
   (c) [seek] after open(3 paths), `seekTo(0.5)`: getPlayheadPosition() is still 0.0 before advanceFrame (RED on main:
   0.5 at once), and 0.5 after `advanceFrame(0.0)`; (d) [seek] `setPlaying(false); seekTo(1.0); advanceFrame(0.0)` ->
   playhead 1.0 (a paused sequence takes the seek); (e) [seek] two seeks before one advance: the LAST target lands.
4. `tests/test_hot_thread_io_lint.cpp` [mediaopen][lint] (AUDIODNA_SRC_DIR): (a) `ImageFileFormat::loadFrom` occurs ONLY
   in `render/ImageDecode.h`, `ui/ClipThumbnails.h`, `ui/FilesBrowser.cpp` (RED on main: MainComponent.cpp:2940, :4957,
   ImageSequence.cpp:57, :356 -> 4 hits named in the failure message); (b) `existsAsFile` does not occur in
   `render/CompositorEngine.cpp`, `media/ImageSequence.cpp`, `ui/ClipCell.cpp` (RED on main: 6 + 1 + 1). Comment
   the DEBT: textual, like test_shader_param_lint.
5. `tests/test_composition.cpp`: the speed cases of 4.7 (RED on main by value).
6. `tests/test_deck_thumbnails.cpp`: B3 "a SEQUENCE cell's thumbnail comes from ClipThumbnails (its first file), decoded
   once off the calling thread; a cell whose Clip::thumbnail is valid re-derives nothing" -- an ImageSequence clip of 3
   PNGs in the grid, the counting decoder sees ONE decode of file 0 and `updateThumbnail` never decodes on the calling
   thread (RED on main: the cell derives nothing -- `thumbnail_` stays invalid -- so an `isValid()` expectation after the
   drained poster FAILs).
Teeth (record FAIL line, restore, `sleep 1; touch`): drop the `playing_`-before-consume order -> 3(d) FAILs; re-add the
stat at ImageSequence.cpp:32 -> 3(a)(b) + 4(b) FAIL; make `Ledger::land` ignore gen -> 1(b) FAILs.

### 4.10 Live probe: `.harmony/probe-media-open.sh` + `.py` + `.json`
`.sh` = probe-image-load.sh verbatim with `MEDIAOPEN_APP` / `MEDIAOPEN_PY` / `MEDIAOPEN_ENV`, out dir `mediaopen.XXXXXX`,
fixtures written BEFORE launch, production mode (7070; the TEST-ONLY endpoints exist in the TEST_SERVER build without
`--test-mode`, F10), the foreign-traffic check, `rm -rf "$OUT/media"` after the quit. `.py` copies probe-image-load.py's
helpers (load / trig / state / comp_state / wait_active / cap / img / dbox / Poller / wait_no_compiler / la). Fixtures
(`$OUT/media`): `v4k.mp4` and `v1080.mp4` = `ffmpeg -f lavfi -i color=c=0x3060c0:s=WxH:r=30:d=3 -g 250 -c:v libx264
-pix_fmt yuv420p -preset ultrafast` (flat colour: the yuv420p round trip is exact within +-2), 16 APFS clones each
(`cp -c v4k.mp4 cell_NN.mp4`), reference frames `ffmpeg -i v.mp4 -frames:v 1 ref.png`; `img4k.png` (noisy 4K, probe-image-load's
`noisy`); `warm.png` / `base.png` / `top.png` flat 256x144 (distinct colours); `seqA/`, `seqB/`: 300 frames each, frame 0 a
NOISY 1080p PNG (a slow decode, ~45 ms), frames 1-299 flat (fast to write); `seq12/`: 12 frames 640x360, frame k's colour
(20k, 0, 255-20k) -> `code(cap)` = the nearest of the 12; `present.png` = top.png's twin for m7. Clip JSON as
probe-image-load (Video: `mediaType 2, mediaFile, speed 1.0, transportMode 0, loopMode 0`; sequence: `mediaType 5,
sequenceFiles, sequenceFps, speed 1.0`; m9's clip OMITS speed on purpose). Constants (`.json`): `stallMaxMs 50`,
`dropStall4kMaxMs 150`, `dropStall1080MaxMs 60`, `holdMaxPerDrop 3`, `swapMaxS 3.0`, `presenceMaxS 1.5`, `boxTol 3.0`,
`heartbeatPeriodMs 4`, `seqFrames 300`, layer ids 91-99. Every row prints the load average; perf rows wait for no
clang / clang++ first.

| row | steps | PASS | RED on main / on the counters-only app (commit 1) |
|---|---|---|---|
| m1_heartbeat | `POST /api/debug/heartbeat {"on":true,"period_ms":4}`; 0.5 s; s. | ok; `message_heartbeat_on` true; `peak_message_stall_ms` present. | main: 404 -> FAIL. |
| m2_load_16x4k | load E (1 deck, 1 layer, 0 clips); 1 s; quiet; s0 (resets the peak); t0; load V16 (16 x 4K cells, none triggered); Poller every 15 ms: max `peak_message_stall_ms`, and stop 0.5 s after `/api/composition` shows 16 clips (<= swapMaxS); s1. | (a) max stall <= 50; (b) swap within swapMaxS (prints the wall time); (c) `fence_black_frames` delta 0; (d) every clip: `thumbnailW > 0`, `clipWidth 3840`; (e) trig(0,0), wait_active, cap -> dbox(cap, ref4k) <= 3 (an adopted player renders). | main: fields absent. commit 1: (a) 1577-1617 (the load runs synchronously; the peak is read after), (c) 1. |
| m2b_load_16x1080 | as m2 with 1080p. | as m2 ((d) 1920). | commit 1: (a) 419-433. |
| m3_drop_4k_video | load W (layer 0 col 0 = warm.png, layer 1 empty, 1 column); trig(0,0); 1 s; quiet; s0; `drop_files {layer 1, column 0, [v4k.mp4]}`; poll `/api/composition` until layer 1 col 0 exists (<= 5 s); 0.3 s; s1. | (a) `fence_black_frames` delta 0; (b) `fence_hold_frames` delta <= 3 (the fence holds only setClip); (c) max stall <= 150 (the open still runs on the message thread; reported); (d) trig(1,0), wait_active, cap -> dbox(cap, ref4k) <= 3. | main: endpoint absent -> FAIL. commit 1: (a) 13-14. Teeth T2 (open moved back inside the fence): (b) 13-14 hold frames -> FAIL. |
| m3b_drop_1080_video | as m3 with v1080. | (a) 0; (b) <= 3; (c) <= 60. | commit 1: (a) 5-6. |
| m3c_drop_4k_image | as m3 with img4k.png. | (a) 0; (b) <= 3; cap = the image. | commit 1: (a) 0-2 (a guard: not reliably RED). |
| m3d_drop_seq300 | as m3 with the 300 files of seqA. | (a) 0; (b) <= 3; (c) <= 50; trig, 1 s, cap not black (mean luma > 20). | commit 1: (a) 4-5, (c) 31-43 (marginal). |
| m4_load_two_seq | load E; s0; load S2 (seqA + seqB at fps 30, not triggered); Poller until both clips appear; s1. | (a) max stall <= 50; (b) `fence_black_frames` 0; (c) trig both; 1 s; cap: mean luma > 20. | commit 1: (a) 124-135 measured with noisy 300-frame sets; with this fixture (noisy frame 0 x 2 decodes each) predicted 170-220. |
| m5_missing_frame_repeats | seq12 with frame 05's file DELETED before the load; fps 1, Loop; trig; at t = 5.4 s cap. | code(cap) == 4 (the failed frame repeats the previous one); no pending (`images_pending` 0). | main: code 6 (the missing file is dropped from the list; index 5 is file 6) -> FAIL by value. |
| m6_retrigger_seek (guard) | seq12 fps 1, `inPoint 0.5`; trig; 1.2 s; trig the same cell (retrigger -> seekTo(inPoint)); 0.25 s; comp + cap. | playheadPosition in [0.5, 0.8); code in {6, 7}. | PASS on main too (the request lands within one frame). |
| m7_presence | load P (layer 0 = base.png triggered, layer 1 = present.png triggered); 1 s; capA (dbox vs top ref <= 3); delete present.png; poll `/api/composition` clip `mediaMissing` every 50 ms (<= 2.5 s); capB; rewrite present.png; poll until false; capC. | mediaMissing true within presenceMaxS (prints t); dbox(capB, base ref) <= 3 (the missing layer is skipped: today's semantics); false again within presenceMaxS; dbox(capC, capA) <= 3 (the resident texture shows again). | main: field absent -> FAIL. |
| m8_speed_default | load a comp whose v1080 clip JSON has NO "speed"; trig; 1.0 s; comp. | playheadPosition >= 0.15 (a 3 s clip after 1 s ~ 0.33). | main: 0.0 -> FAIL. |
| end_hold_witness | after every row: s. | `fence_hold_frames` total over the run >= 1 (the hold path ran: >= 6 fences in the run; a load shows 1 fenced frame 5/5 in diag). | commit 1: 0 -> FAIL. Teeth T1 (the hold branch disabled): m2 (c) / m3 (a) FAIL by value. |

Order: m1, m2, m2b, m3, m3b, m3c, m3d, m4, m5, m6, m7, m8, end. GREEN = every row PASS on 2 consecutive runs of the final
app on a quiet machine; RED lines on MAIN and on the commit-1 app recorded verbatim; teeth T1 / T2 + the ctest teeth
recorded with the restore sha. A flake verdict needs >= 5 runs per arm.

### 4.11 What "done" looks like
- ctest: the post-seqvram count + the new cases green, serial (`ctest -j1`); teeth lines recorded.
- probe-media-open GREEN twice; RED runs recorded (main: absent fields / endpoint; commit 1: m2 1577-1617, m3 13-14,
  m3b 5-6, m3d 4-5, m4 by value, m5 code 6, m8 0.0, end 0).
- probe-image-load 37/0, probe-crossfade 35/0, probe-seq-vram GREEN, probe-deck-clock GREEN, probe-render-state / canvas /
  capture at their recorded counts (loads of image / sequence compositions swap synchronously: unchanged timing).
- Report tables: per-row numbers (stall max, swap wall time, hold / black deltas, presence latency, codes).

## 5. THE SEAM WITH PART V (video decode off the GL thread -- authored in parallel)
- THIS lane's `VideoPlayer` edits are confined to `open()` :178-182 (+ `makeThumbnail`), `getThumbnail` :380-402,
  `close()`'s reset and the two members. Part V owns `advanceFrame / advanceClock / decodeFrameAtTime / decodeNextFrame /
  convertFrameToRGBA / uploadToTexture`, `syncMedia`'s video branch (`Renderer.cpp:1574-1651`) and the
  `videoPlayerMutex_` hold (FNF 2). Neither lane touches the other's lines.
- Publication seam: `Renderer::adoptVideoPlayer(id, std::unique_ptr<VideoPlayer>)` (4.4). Contract part V's player must
  keep: (1) `open()` is callable on ANY thread on an unpublished object (the MediaOpener's pool) -- a decode thread may
  be started inside `open()` or lazily by the first `advanceFrame`; (2) `close()` joins that thread from any thread;
  (3) destroying a never-published player touches no GL (releaseGL only for `texture_ != 0`); (4) the first frame and the
  thumbnail exist when `open()` returns (the staged clip needs `hasAlpha / getWidth / getHeight / getThumbnail` on landing).
- Part V "keeps open() on the message thread" for its synchronous callers (drops, trigger fallback :4348, undo hook
  :4647, Replace Content :6550): unchanged by this lane. When part V later wants the async path for those, it calls
  `mediaOpener_.begin` with a Landed that adopts and refreshes the cell.

## 6. MUST NOT CHANGE (and how each is kept)
- Composited pixels: the hold never composites and never alters a frame that IS composited (byte identity: abpix-style
  A/B of a `render_frame` on a static composition, main vs lane, array_equal). Thumbnails are UI images, not output.
- Pitfall 35 / 36 / 52 / 53: no history key, no deck-id path, no capture-lock / override / flight change (the hold does
  not call `processPendingCapture`), no `getKeyTexture` / pending semantics change (the presence flag replaces the stat
  guard in front of it; the Failed path is untouched).
- Transport math: `advanceFrame`'s body after the consume, `advanceTransport`, `syncMedia`'s sync block, DeckClock /
  LayerClock, rule 15 -- untouched; the only timing change is the sequence's GL-side wrap seek landing one frame later
  (video parity, R-9); probe-deck-clock's `d_imageseq_keeps_time` is the witness.
- Undo: the same commands (`SetClipCmd`, `SetColumnCountCmd`, `InsertDeckCmd`) with the same before / after snapshots;
  the prepare / commit split changes only WHEN the open runs. `test_undo_commands` unchanged.
- The retire lifecycle (`closeMediaForClip` / `drainRetiredMedia` / `openGLContextClosing`), `videoPlayers_` keying by
  clip id, `openVideoForClip`'s contract (no-op on failure) -- kept (adopt is its second half).
- seqvram's window policy, grant and counters -- untouched; only `open()`'s dead frame-0 block and the stat go.
- `/api/state` and `/api/composition`: fields added, none renamed; `/api/load_composition`'s contract (ok = accepted;
  the swap follows on the message thread) unchanged. The 7070 route count is unchanged (TEST-ONLY paths are not counted rows).
- Fence of files: `src/MainComponent.h/.cpp`, `src/core/UndoService.h/.cpp`, NEW `src/core/MediaOpenBatch.h`,
  `src/core/MediaOpener.h/.cpp`, `src/core/MediaPresence.h`, `src/render/Renderer.h/.cpp` (:110-111 setter, :357-372 hold,
  :391 getters, :517 members, :1432-1453 adopt), `src/render/CompositorEngine.cpp` (6 sites + the helper), `src/ui/ClipCell.h/.cpp`
  (:206, :403-419, classifyDrop), `src/ui/LayerStrip.cpp` (:950-958), `src/media/ImageSequence.h/.cpp` (open / seek / members),
  `src/media/VideoPlayer.h/.cpp` (open thumbnail / getThumbnail / close reset ONLY), `src/model/Clip.h/.cpp` (:215 guard;
  mediaMissing), `src/api/ApiServer.h/.cpp` + NEW `src/api/MessageHeartbeat.h`, `src/test/TestServer.h/.cpp`, tests (6
  files) + `tests/CMakeLists.txt` EOF, `.harmony/probe-media-open.*`, docs. Nothing in `src/render/ImageDecode.h`,
  `ImageTexCache.h`, `SeqVram.h`, `src/recording`, `src/output`, probe-image-load / probe-seq-vram / probe-deck-clock.

## 7. RISKS (strongest counterargument first)
R1 "The asynchronous load is a state machine (staging, generations, cancel, adoption, a pool) for a UI freeze that happens
   once per 20 minutes and never touches the output." It loses because: the cheap step alone leaves 1.1 s at 16 x 4K
   (F4), a UI freeze mid-show is the last message-thread hold of this class (renderleft / restore removed the others), the
   machine is ~120 lines whose bookkeeping is a pure ctest, an empty batch is synchronous so nothing changes for every
   existing composition without videos, and the audience sees today's picture exactly. Lever left for Harmony: the commit
   order puts it LAST (commit 7); commits 1-6 stand alone and can ship without it (Q1).
R2 The hold's two-load ordering. Covered formally in 4.1 (flag before deck; re-read on a null deck; clear after restore).
   Residual: a frame that reads the flag true and the deck restored renders normally -- correct. The RAII guards keep the
   flag paired with the deck on exceptions. Witness: `fence_black_frames` 0 across the run and the teeth build.
R3 A held frame is a still frame: for the 1-2 frames of a setClip fence it is invisible; for a 300-file drop's sort or a
   composition swap it is <= 2 frames (the I/O left the fence). The recorder / Syphon get no frame for those frames, as
   they get none on today's early returns (F2) -- a recording duplicates or skips one frame instead of recording black.
R4 `MediaPresence` changes a semantics detail: a deleted file keeps showing for <= 1 s (was: next frame). A live show
   prefers a picture over an instant FX-only flash; documented; Q2 asks Boris whether he would rather keep the picture
   until the next load (the rejected alternative in R-10 is one line to switch).
R5 `VideoPlayer::open` on a pool thread runs FFmpeg concurrently with the GL thread's decodes: independent contexts
   (F14), today's message-thread open already does; `avcodec_find_decoder` / `sws_getContext` are thread-safe. A 2-thread
   pool + thread_count 2 per codec = up to 8 busy threads on a 10-core M1 Pro during a load (ASSUMED fine; m2 records
   `peak_callback_ms` during the load -- if the render callback suffers, drop the pool to 1 thread / low priority, Q4).
R6 The sequence missing-frame semantics change (R-8): a sequence whose file 5 is gone now HOLDS frame 4 for that slot
   instead of shortening the loop. m5 pins it; a Boris line in section 9.
R7 The seek request lands one frame late for the sequence's own out-point wrap (R-9): 8 ms at 120 Hz, video parity.
   If a probe row (probe-seq-vram H6 in/out cases) reads the frame at the wrap boundary exactly, it may need a one-frame
   tolerance -- run probe-seq-vram after commit 5 and record.
R8 `ClipThumbnails` keyed by `sequenceFiles[0]` + mtime: a sequence whose first file is replaced in place gets a new
   thumbnail at the next refresh (a stat per get on a miss; a hit is a map lookup, F8 / Pitfall 51's rules).
R9 A REST load_composition immediately followed by a trigger of a VIDEO cell (no probe does this today: probe-deck-clock
   sleeps 1 s) would trigger before the swap; the composition's clip count / `media.opens_pending` are the witnesses a
   probe should wait on. Documented in the rendering.md paragraph.
R10 Renderer.cpp / MainComponent.cpp / UndoService.cpp are in no ctest (F10): the hold, the drop split, the staged flow and
   the endpoints are proven live only (m2-m8 + teeth T1 / T2); the pure parts (Ledger, presence, lints, seek/open) are ctests.
R11 CLAUDE.md: 24,482 + seqvram's ~150 B leaves ~370 B; the index line below is ~160 B. If Harmony's final text does not
   fit, shorten line 230 (Pitfall 53's index line, 194 B) by 30 B as plan-seqvram section 9 already proposed.
R12 Perf-ish rows (stall maxima) flake under load: quiet rule, load average printed, >= 5 runs per arm; the diag-idle
   lane's unattributed 16-28 ms idle stall (diag-media "Rig and baseline") is UNDER the 50 ms bar by design.

## 8. COMMIT SEQUENCE (each builds, passes ctest, keeps every existing probe GREEN; each reverts alone)
1. `perf(mediaopen): counters -- fence_hold_frames / fence_black_frames; TEST-ONLY message heartbeat (peak_message_stall_ms)
   and /api/debug/drop_files; /api/composition clip mediaMissing / clipWidth / clipHeight / thumbnailW / thumbnailH;
   probe-media-open RED harness` -- no behaviour change (the hold `if` counts only; `mediaMissing` exists but is never
   set). RED runs recorded: main (absent) and this app (m2 1577-1617 / 1 black; m3 13-14; m3b 5-6; m3d 4-5; m4 by value;
   end 0).
2. `fix(model): Clip::fromVar defaults a missing "speed" to 1.0 (+ test_composition cases)` -- m8 GREEN.
3. `fix(render): a fenced frame holds the canvas -- withDeckDetached never shows a deck-less black frame` -- 4.1. GREEN:
   m2 (c), m3-m3d (a), end; teeth T1 recorded.
4. `perf(drop): prepare a drop outside the fence, commit setClip inside; VideoPlayer::open makes its 90x72 thumbnail (sws),
   getThumbnail returns it (FNF 5)` -- 4.2 + 4.3 + `ClipCell::classifyDrop`. GREEN: m3 / m3b (b) <= 3; teeth T2 recorded.
5. `perf(seq): ImageSequence::open does no file I/O (no frame-0 decode, no stats: a missing frame Failed = the previous
   frame repeats); sequence thumbnails from ClipThumbnails; seekTo is a GL-thread request; test_image_sequence_open;
   test_deck_thumbnails B3` -- 4.5. GREEN: m4, m5, m6; probe-seq-vram re-run recorded (R7).
6. `perf(presence): MediaPresence -- Clip::mediaMissing from a 1 Hz off-thread sweep, seeded at load / drop; no stat in the
   compositor or ClipCell::paint; test_media_presence; test_hot_thread_io_lint` -- 4.6 + 4.9(4). GREEN: m7, lint.
7. `perf(load): asynchronous media opens -- MediaOpenBatch (pure) + MediaOpener; loadComposition / appendDeckFromFile /
   duplicateDeck swap when the staged players have landed; Renderer::adoptVideoPlayer (the part-V seam); media state
   in /api/state` -- 4.4 + 4.9(1). GREEN: m2 / m2b (a)(b)(d)(e).
8. `docs(mediaopen): rendering.md paragraph; pitfall NN; Pitfall 51 sentence; CLAUDE.md index` (section 9).
9. The report commit (`.harmony/.reports/s-rta-0928b/mediaopen.md`, `git add -f`).
Trailer on every commit: the session's attribution line.

## 9. DOCS (text; pitfall as "NN" until Harmony assigns the number; Harmony edits HANDOFF / APP-INVENTORY, not the lane)
- `docs/claude/rendering.md`, a new paragraph after :71:
  "**Media opens and the fence (s-rta-0928b mediaopen)**: a frame inside `UndoService::withDeckDetached` (the renderer
  FENCED and deck-less: `Renderer::setFenced` is set before the deck is nulled and cleared after it is restored) HOLDS the
  canvas as the previous frame left it -- no clear, no composite, no capture answered, no recorder / Syphon frame; the
  outputs re-present the last composited picture. It used to show one black frame per fence on every output (5-14 on a
  video drop). Drops PREPARE outside the fence (id, open, dims, thumbnail; `MainComponent::prepareFileDrop /
  prepareMultiFileDrop`) and COMMIT inside it (`commitDrop`: before-snapshot + `setClip`). A composition / deck load is
  STAGED (`StagedLoad`), its video players open on `MediaOpener` (2 threads, off the message thread; `VideoPlayer::open`
  makes the 90x72 thumbnail itself, sws-scaled, before the player is published) and the swap / `InsertDeckCmd` runs when
  the last player has landed (`Renderer::adoptVideoPlayer`); landings write the STAGED clips only, never a live Clip. The
  old composition stays on screen until then (the file label reads "Loading <name>..."); an EMPTY batch (no video clips)
  swaps synchronously inside the call. A newer load, any composition swap or quit cancels a pending one. REST:
  `/api/load_composition` answers before the swap (as before); wait on `/api/composition`'s clips or `/api/state.media.opens_pending`.
  `ImageSequence::open` does no file I/O: no frame-0 decode (the window starts at its floor until the first upload, SeqVram
  H5), no stats -- a missing frame decodes Failed and the previous frame repeats (the file used to be dropped, shifting
  every later frame); a sequence's grid thumbnail comes from `ClipThumbnails` keyed by its FIRST file (Pitfall 51);
  `ImageSequence::seekTo` is a request consumed at the top of `advanceFrame` on the GL thread (VideoPlayer's shape; the
  out-point wrap lands one frame later, as video's does). Presence: `Clip::mediaMissing` (runtime) is set by
  `MediaPresence`'s 1 Hz off-thread sweep and seeded at load / drop; the compositor (`clipHasContent`, the clip-texture
  branches) and `ClipCell::paint`'s "!" read it -- no `existsAsFile` on the GL thread or per paint (`test_hot_thread_io_lint`).
  A file deleted mid-show is treated as missing within <= 1 s (its resident picture / open player shows until then); a file
  that comes back is present within <= 1 s. `/api/state`: `fence_hold_frames`, `fence_black_frames`, `media {opens_pending,
  open_batches, presence_sweeps, presence_changed}`; TEST-ONLY (AUDIODNA_BUILD_TEST_SERVER, production port):
  `POST /api/debug/heartbeat {on, period_ms}` -> `peak_message_stall_ms` (reset on read), `POST /api/debug/drop_files
  {layer, column, files[]}` = a Finder drop's handlers. `/api/composition` clips: `mediaMissing`, `clipWidth`, `clipHeight`,
  `thumbnailW`, `thumbnailH`. `Clip::fromVar`: a missing "speed" loads 1.0. Guards: `tests/test_media_open_batch.cpp`,
  `test_media_presence.cpp`, `test_image_sequence_open.cpp`, `test_hot_thread_io_lint.cpp`; live `.harmony/probe-media-open.sh`."
- `docs/claude/pitfalls.md` after the last entry (seqvram's 54):
  "NN. **A `withDeckDetached` fence is not a picture -- a fenced, deck-less frame HOLDS the canvas; media opens run outside
  the fence, never inside it**: `UndoService::withDeckDetached` nulls the renderer's active deck, drains one GL frame and
  runs the mutation while the GL thread keeps rendering; every deck-less frame fell to the "nothing to render" path and
  showed BLACK on every output (measured s-rta-0928b diag-media: 1 frame per composition load, 2 per image drop, 5-14 per
  video drop -- `applyFileDrop` opened the video and made its thumbnail INSIDE the fence, 106.6 ms at 4K). Rules: (1) the
  renderer reads `fenced_` before the deck (re-read once on a null deck) and re-presents `canvasFBO_` untouched -- no clear,
  no composite, no capture, no recorder frame -- counted in `fence_hold_frames`; a fenced frame that could not hold is
  `fence_black_frames` (must read 0); (2) a fence wraps ONLY the model write (a `setClip`, a column growth, the swap):
  open the media, read dims / alpha, make the thumbnail BEFORE it (`prepare* / commitDrop`; the staged load lands its
  players before its swap); (3) never write a LIVE Clip's thumbnail / dims from a completion -- write the staged clip
  (loadComposition step 4). Guards: `.harmony/probe-media-open.sh` rows m2-m3d + `end_hold_witness`; the lint
  `tests/test_hot_thread_io_lint.cpp` keeps `ImageFileFormat::loadFrom` / `existsAsFile` off the compositor, the
  sequence and the cell paint."
- `docs/claude/pitfalls.md` Pitfall 51 (:113): after "a video / sequence's is its `Clip::thumbnail`" -> "a video's is its
  `Clip::thumbnail`, made inside `VideoPlayer::open` before the player is published (s-rta-0928b); a SEQUENCE's comes from
  `ClipThumbnails` keyed by its first file".
- `CLAUDE.md` after seqvram's index line (~160 B; `wc -c` <= 25,000): "NN. A fence holds the canvas (never a black frame);
  media opens run before the fence, and a completion writes staged clips only -- before touching withDeckDetached, a drop
  handler or loadComposition." Fallback if the cap is hit: R11.
- Boris checks (for Harmony's page): (1) dropping a video or loading a show no longer flashes the projector black -- the
  picture freezes for a frame or two instead; (2) loading a show with many videos no longer freezes the app: the old show
  keeps running, the bottom label says "Loading <name>...", then the new one cuts in (about as long as the old freeze);
  (3) video cell thumbnails are made a different way (slightly different look at 90 x 50); (4) a file deleted while the show
  runs disappears from the output within a second (was: at once) -- say if he would rather keep the picture; (5) an image
  sequence with a missing frame now holds the previous frame for that slot instead of skipping it.

## 10. COMPACT
- Today: the fence nulls the deck and the GL thread renders black until the mutation ends (`UndoService.cpp:66,89`;
  `Renderer.cpp:455-462`); a video drop opens inside the fence (`MainComponent.cpp:4882-4895` under :4910); a load opens
  every video + thumbnail on the message thread pre-swap (:2913-2946, :3017); `ImageSequence::open` stats 300 files and
  decodes frame 0 (:32, :57-67) and the grid decodes frame 0 again (:2940, :4957); 6 GL-thread stats
  (`CompositorEngine.cpp:1076..1583`) + a stat per cell paint (`ClipCell.cpp:206`); `Clip.cpp:215` reads speed unguarded.
- Design: explicit fence flag + canvas HOLD (no clear / composite / capture; counters hold / black); drops prepare
  outside / commit inside; `VideoPlayer::open` makes the thumbnail (sws, cached; FNF 5); loads open-then-swap on a
  2-thread `MediaOpener` with a pure `Ledger` (staged clips written, players adopted via `Renderer::adoptVideoPlayer`;
  empty batch = synchronous); sequences: no I/O in open (Failed frames repeat), thumbnails via ClipThumbnails, seekTo a
  GL-thread request; presence: `Clip::mediaMissing` from a 1 Hz off-thread sweep (<= 1 s latency, seeded at load / drop);
  speed defaults to 1.0.
- Gates: 6 ctest files (lint x2 RED by hit count; seek / open RED by value; speed RED by value; Ledger / presence pure);
  probe-media-open 13 rows -- RED on main by absence and RED BY VALUE on the counters-only commit (m2 1577-1617 ms + 1
  black; m3 13-14 black; m3b 5-6; m3d 4-5; m4 124-220; m5 code 6; m8 0.0; end 0); teeth T1 (no hold) / T2 (open inside
  the fence) FAIL the fence rows.
- Seam with part V: `Renderer::adoptVideoPlayer`; this lane touches VideoPlayer only in `open()` / `getThumbnail` / `close()`.
- Open questions for Harmony: Q1 ship commit 7 (async loads) now or after part V (commits 1-6 stand alone); Q2 presence
  semantics -- FX-only within 1 s (chosen, today's) vs keep the picture until the next load (Boris feel); Q3 heartbeat
  opt-in at 4 ms (chosen) vs always-on in TEST_SERVER builds; Q4 MediaOpener 2 threads at normal priority (ASSUMED;
  m2's `peak_callback_ms` decides).

REPORT_FILE: .harmony/.reports/s-rta-0928b/plan-mediaopen.md
STATUS: FINAL

## HARMONY ADOPTION (s-rta-0928b, 20:59) — OVERRIDES THE BODY WHERE THEY DIFFER
Plan authored by Fable (wf_e133b94a-580). Attacked by attack-mediaopen-threads.md and attack-mediaopen-vj.md. Rulings:
- P1 (Q1, SCOPE) This lane ships commits 1-6 + 8-9 (counters/heartbeat/drop endpoint, speed, the fence HOLD, drops prepared
  outside the fence, sequences with no I/O in open(), presence + lint, docs, report). Commit 7 (asynchronous loads:
  MediaOpenBatch / MediaOpener / staged swap / adoptVideoPlayer) is DEFERRED to a follow-up lane after the video lane merges:
  it depends on the video lane's VideoPlayer contract (open() on any thread, the install seam), and every attacker MUST
  lives in it. Filed acceptance for that lane (from the attacks): cancelStagedOpen retires staged SEQUENCE ids too
  (threads MUST); the file label is restored on cancel + a cancel row (vj MUST 2); an audio-callback witness during a
  16 x 4K load (vj MUST 1: the GL peak_callback_ms is not the audio callback); a load-then-immediately-trigger row with a
  defined outcome (vj MUST 3); DeckDuplicate's label from the deck name (threads NIT). Rows m2 / m2b (the async-load rows)
  move with it; m2's (c) part — the load's fence frame holds, no black — stays here if it can be asserted without commit 7.
- P2 (CONFLICT with the video lane, OVERRIDE 4.3) Do NOT edit src/media/VideoPlayer.*. The video lane (plan-video R-16)
  makes the 90 x 72 thumbnail inside open() and removes the frameBuffer_ race (FNF 5). Your drop prepare calls whatever
  getThumbnail returns. If the video lane has merged when you branch/rebase, you get O(1) thumbnails for free; if not, the
  drop's UI hold keeps today's thumbnail cost (the OUTPUT still holds — the point of this lane). Say which in the report.
- P3 (threads SHOULD, ADOPT) The fence state is read ONCE per frame from a single atomic that carries both the deck and the
  fenced state (e.g. a sentinel "fenced" pointer value, or a packed 64-bit word) — no two-load TOCTOU on the set or the
  clear edge. If the protocol is a pure header, give it a two-thread stress ctest (and a TSan run if the build supports it).
- P4 (vj SHOULD m3c, ADOPT) A row whose RED is not reliable is a GUARD, not RED evidence: run m3c 5 times per arm on the
  counters-only app and report the distribution; count it as RED only if it fails >= 4 of 5.
- P5 (Q2, ADOPT the plan) Presence keeps today's semantics (a missing file -> FX-only / no media) at <= 1 s latency.
- P6 (Q3, ADOPT the plan) The heartbeat is TEST-ONLY, opt-in, 4 ms; build it as a reusable header (src/api/MessageHeartbeat.h) —
  the idle-stall fix lane will reuse it for its gate.
- P7 Branch from main AFTER lane/seqvram merges (Harmony launches you then); confirm SeqVram.h is present. If the video lane
  merges while you work, rebase onto main before your final gates and keep both lanes' docs / CMake / state-field blocks.
  Pitfall text "NN"; CLAUDE.md <= 25,000 B (pay for your index line — seqvram and video each added one).
