# plan-asyncload -- asynchronous composition / deck loads (s-rta-0929 START HERE 1; plan-mediaopen commit 7 re-derived)

Lane name: **asyncload**. Probe rows `a1..`; commit messages `(s-rta-0929 asyncload)`; pitfall text "NN" (Harmony assigns 58).
Authored by Fable (architect) against main HEAD **adf9b8a** in /Users/boriskarpman/projects/RealTimeAudio. Every file:line
below was read at that commit; a claim not re-derived from source is marked INFERRED or ASSUMED. Read with:
plan-mediaopen.md 4.4 + HARMONY ADOPTION P1 (the deferred design and this lane's acceptance), attack-mediaopen-threads.md,
attack-mediaopen-vj.md (every MUST lives here), plan-video.md section 5 + adoptions V1-V7 / W1-W5, video.md, mediaopen.md,
seqvram (Pitfall 54), BORIS_DECISIONS.md "Playback Behaviour", pitfalls 35 / 36 / 53-57, CLAUDE.md rule 15.

QUESTION: how does a composition load, a deck append (Load Deck) or a deck duplicate open its video players OFF the message
thread so the UI stays live and the output keeps playing the OLD composition until the new one is ready, then cuts with no
black and no FX-only frame -- meeting acceptance A1-A8 -- without changing what any existing REST / OSC / probe caller sees?

APPROACH (stated first): the three load paths STAGE a private model (as today, steps 1-3b unchanged), seed presence, then
hand one job per present video clip to a 2-thread low-priority `MediaOpener` pool that runs `VideoPlayer::open` on a NEW,
unpublished player; each landing (message thread) writes dims / alpha / thumbnail into the STAGED clip and installs the
player under its (re-minted, collision-free) id through the video lane's seam `Renderer::installVideoPlayer` (the thread
parks at once: nothing draws it); the LAST landing runs the completion -- staged sequences opened (no I/O), then exactly
today's swap / `InsertDeckCmd` / label code. One staged load at a time (newest wins); a cancel retires every id the batch
adopted and restores the label; `POST /api/load_composition` answers only after the staged swap completes (a waitable
`LoadTicket`), so every existing single-connection caller sees today's behaviour; a live trigger during the window acts on
the OLD composition (the show on screen), never on the staged one. Image-only / sequence-only loads swap on the same
message as today (an empty batch completes synchronously). The audio witness is the OS's own overload count
(`AudioIODevice::getXRunCount`) plus two compile-time-gated relaxed counters in the callback.

## 1. TODAY'S CODE, RE-DERIVED (file:line at adf9b8a; VERIFIED unless marked)

**The three load paths (message thread, synchronous)**
- `MainComponent::loadComposition` (`src/MainComponent.cpp:3071-3137`): 1 STAGE `incoming.loadFromFile` (:3077-3086;
  `Composition::loadFromFile` = `JSON::parse(file.loadFileAsString())` + `fromVar`, `src/model/Composition.h:675-681`),
  2 VALIDATE (:3091-3099), 3 RE-MINT `compload::remintClipIds(incoming, s_nextClipId)` (:3105; the mint is file-static
  `s_nextClipId = 1000`, :17, monotonic, shared with drops :4964 / :1151 / :1280), 3b RECONCILE (:3110), 4 OPEN
  `openMediaForDeck(deck)` per deck (:3119-3120), 5 NAME (:3126), 6 SWAP `swapCompositionModel([&]{ composition_ =
  std::move(incoming); })` (:3131), 7 LABEL "Loaded: <base name>" + browser refresh (:3134-3136).
- `appendDeckFromFile` (:3214-3290): parse + shape check (:3221-3234), validate (:3238-3246), re-mint (:3253), reconcile
  (:3256), `openMediaForDeck(incoming)` (:3259), name + `sourceFile` (:3265-3266), ONE `InsertDeckCmd` via `pushCommands`
  (:3277-3281; `DeckCommands.h:811-850`: first execute appends through `Composition::appendDeck` with "media ALREADY open
  (the caller)" :848, makes it active :850; redo reconnects through `mediaHook_` :835-840), `rebuildGrid` (:3282),
  `refreshPreviewFromActiveClip` (:3283-3284), LABEL "Loaded deck: <name>" (:3287-3289).
- `duplicateDeck` (:3435-3457): `compload::duplicateDeck` value copy, "<name> copy", every clip re-minted, queued triggers
  dropped (`src/core/CompositionLoad.h:154-167`; Pitfall 36), `copyName` (:3445), `openMediaForDeck(copy)` (:3446),
  `InsertDeckCmd "Duplicate Deck"` (:3448-3452), rebuild / refresh (:3453-3455), LABEL "Duplicated deck: <copyName>" (:3456).
  NOT reachable over REST / OSC (tab menu only, :1447).
- `openMediaForDeck` (:3013-3048): `presence::seed(deck)` (:3017; `src/core/MediaPresence.h:80-86`, one stat per Image /
  Video clip), then per playable clip: Video -> skip if `mediaMissing` (:3027), `renderer.openVideoForClip(clip.id, file)`
  (:3028) then `hasAlpha / clipWidth / clipHeight / thumbnail = getThumbnail(90, 72)` from the player (:3030-3036);
  ImageSequence -> `openImageSequenceForClip(id, files, fps)` (:3044; no I/O since mediaopen, `ImageSequence.h:29-31`).
- `swapCompositionModel` (:2934-2973): `routineEngine_.stopAll()` (:2938), `before = playableClipIds` (:2942), the fence
  `undoService_.withDeckDetached(mutation)` (:2951; `src/core/UndoService.cpp:55-100`: `detachActiveDeckFenced` ->
  blocking `executeOnGLThread` drain -> mutation -> `setActiveDeck(restored)`), retire by set difference
  `closeMediaForClip` (:2958-2961), `postImageSet(imagePaths)` (:2964), `undoManager_.clear()` (:2970),
  `refreshUiAfterModelSwap` (:2972; :2870-2932: null inspectors, `rebuildGrid`, `setActiveColumn(-1)`, inspector
  refresh, `refreshPreviewFromActiveClip`). Also called by New Composition (:6171, `initDefault`).
- `~MainComponent` (:2357-2371): `apiServer_->stop()` FIRST (:2370-2371), routines / recorder / testServer, then
  `renderer.detach()` (:2397) -- the shutdown order the comment block (:2359-2369) makes load-bearing.
- Lazy opens elsewhere (unchanged by this lane): a trigger of a video clip with no player opens it synchronously
  (:4445-4450); `makeClipMediaHook` reconnect (:4734-4760, `needsVideoReopen`: `src/core/MediaReconnect.h:12-17`);
  `prepareFileDrop` opens the dropped video before the fence (:4948-4995, :4981).
- `activeClipColumn` is NOT serialized: `src/model/Layer.h:181` default -1; no hit in `Layer.cpp` / `Composition.h` /
  `Deck.h` (grep, VERIFIED by absence) -- a loaded composition or deck file has NO active clip until triggered
  (`:2926-2931` comment says the same). A DUPLICATE copies the live deck's `activeClipColumn` by value -> its clips ARE
  drawn the frame after the insert.

**The video lane's seam (VideoPlayer untouched by this lane)**
- `VideoPlayer::open(file)` (`src/media/VideoPlayer.h:38-40`, `.cpp:51-230`): "Call from message thread, once per player
  ... Starts NO thread: the whole synchronous prepare touches only this object (the part-M seam)" -- the body touches
  only members: `avformat_open_input` (:61), `avformat_find_stream_info` (:67), decoder + `thread_count = 2` (:108,
  `avcodec_open2` :110), `AV_PIX_FMT_NONE` guard (:163-169), sws (:173), 3 page-lazy slot mallocs (:186-195), the FIRST
  FRAME decoded into slot 0 with pts clamped <= 0 + the <= 90x72 thumbnail (:204-216, `makeThumbnail` :517-543), then
  `open_ = playing_ = true` (:218-219) and one `std::cerr` line (:221-227). No GL, no thread, no Renderer state: it may
  run on a pool thread for an UNPUBLISHED player; publication (the `callAsync` post / receive) is the happens-before for
  every non-atomic member (`sourceFile_`, `width_`, `thumbnail_`, ...) the message thread reads afterwards.
- `Renderer::installVideoPlayer(uint32_t clipId, std::unique_ptr<VideoPlayer>)` (`src/render/Renderer.h:228`,
  `.cpp:1482-1510`), message thread: `closeMediaForClip(clipId)` (retire whatever holds the id), insert under
  `videoPlayerMutex_` (:1502-1505, `players` count), `raw->start()` (:1508) -- the decode thread. `openVideoForClip`
  (:1473-1480) = make + `setStats(&videoStats_)` + open + install. `getVideoStats()` is public (`Renderer.h:254`).
  `setStats` is a plain pointer store "Before start()" (`VideoPlayer.h:120-121`); `open()` never reads `stats_`
  (the uses are :236-239 start, :410-479 upload, :557-670 decode thread).
- A started but never-drawn player PARKS at once: `decodeLoop` (:562-615) checks `VideoRing::idleStep(nowMs,
  lastDrawMs_, pol)` first (:575); `lastDrawMs_{0}` (`VideoPlayer.h:181`) and `idleStep = nowMs - lastDrawMs > idleMs`
  (`src/media/VideoRing.h:191-194`) -> `Park` on the first iteration (`video_threads_awake` stays 0); the first draw's
  `advanceFrame` posts and notifies. Frame 0 sits in slot 0 from `open()`, so the first `uploadToTexture` shows it --
  never pending (Pitfall 56).
- Destroying a never-installed player on the message thread makes no GL call: `releaseGL` deletes only `texture_ != 0`
  (:485-491); `close()` frees the FFmpeg contexts itself for a never-started player (:245-255).
- `closeMediaForClip` (:1531-1569) retires video / sequence by id into the GL-drained lists; `drainRetiredMedia`
  (:1571-1634) destroys a player only once its thread exited (:1616-1629).

**REST / OSC callers of a load (A8 enumeration)**
- `POST /api/load_composition` -> `ApiServer::handleLoadComposition` (`src/api/ApiServer.cpp:238`, body :1010-1067): file
  exists (:1035-1040), a throwaway parse + validate ON THE HTTP THREAD (:1042-1056), then `callAsync([this, f]{
  onLoadComposition(f); })` (:1058-1064) and `jsonOk()` AT ONCE (:1066) -- FIRE-AND-FORGET. Today's "load then act"
  callers work only because the load runs synchronously inside its message and the next request's `callAsync` queues
  behind it (message-queue FIFO). `onLoadComposition` = `std::function<void(juce::File)>` (`ApiServer.h:67-70`), wired
  at `MainComponent.cpp:1909`. TestServer (8080) has NO load route (grep); OSC has none (`src/osc/OscHandler.cpp`: trigger
  / switch_deck / routine only); MIDI none; the Eyes harness (`tests/visual/vj_controller.py:88, :173`) uses load_image /
  load_source only. Every load caller is sequential on ONE connection and acts only after the response:
  `.harmony/probe-{canvas:113, capture:69, crossfade:102, deck-clock:111 (+1 s sleep), effects-parity:126, fitmode:126,
  outputs:162, media-open:202, image-load:211, idle-paint:317, video:286, render-state:157, seq-vram:181}.py` (timeouts
  10-30 s) and `.harmony/probe-{deck-tabs:127, deck-path:71, lane3:146/263/277, onset-render:114, resync:88,
  mastersignal:366/456, routine-display:172, routines:701, step3:246}.sh` (`curl --max-time 6-10`). The only probes with
  threads (`image-load:322`, `seq-vram:370`, `video:89`, `media-open:325`) poll `/api/state` from the second thread; none
  triggers from it. `ApiServer::stop()` (:103-113) = `server_.stop()` + join: httplib joins its workers, so a handler
  blocked in a wait would block the quit (see 5.5). `MessageManager::callAsync` returns bool
  (`build/_deps/juce-src/modules/juce_events/messages/juce_MessageManager.h:109`) and refuses posts once quit began
  (the :528-552 comment block).
- httplib worker pool: `CPPHTTPLIB_THREAD_POOL_COUNT` (`build/_deps/httplib-src/httplib.h:183-191`; hardware
  concurrency or 8) -- a waiting load handler leaves >= 7 workers for `/api/state` polls.

**Witness plumbing that exists**
- TEST-ONLY message-thread heartbeat: `src/api/MessageHeartbeat.h` (whole), `POST /api/debug/heartbeat`
  (`ApiServer.cpp:293`), `/api/state peak_message_stall_ms` reset on read (:1403-1408). Drop endpoint shape to copy:
  `handleDebugDropFiles` (:1722-1750; `onDebugDropFiles` `ApiServer.h:170-173`; TEST-ONLY decls :230-238).
- `/api/state` (both servers): `fence_hold_frames` / `fence_black_frames` (`ApiServer.cpp:1398-1399`,
  `TestServer.cpp:675-676`), `media` provider (:1401-1402 / :677-678; `MainComponent::mediaStateVar` :5032-5039, wired
  :1873 / :2136), `video_players`, `video_uploads`, `video_pending_frames`, `videos_pending`, `video_late_frames`,
  `seq_open`, `peak_callback_ms` (the GL callback -- NOT the audio callback) (field list from the `setProperty` scan).
- Audio: the RT callback is `CombinedCallback::audioDeviceIOCallbackWithContext` (`src/audio/CombinedCallback.h:41-140`)
  -> `AudioCallback` (`src/audio/AudioCallback.cpp:8-45`, `ringBuffer_.push` :44, its return value ignored) -> `AudioTap`.
  JUCE's CoreAudio device counts `kAudioDeviceProcessorOverload` into `getXRunCount()`
  (`build/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp:1157-1162, :1274, :1702-1704`);
  `Time::getHighResolutionTicks()` = `mach_absolute_time()` (`juce_SystemStats_mac.mm:351`, a commpage read, no
  syscall). The device manager is `AudioEngine::deviceManager_` (`src/audio/AudioEngine.h:63`, `getDeviceManager()`
  :26; the callback added at `AudioEngine.cpp:21`); `MainComponent::timerCallback` runs at 30 Hz on the message thread
  (`MainComponent.cpp:3825-3829`, `outputs_.pollDisplays()` is its first line).
- Thread priorities on macOS (`build/_deps/juce-src/modules/juce_core/native/juce_Threads_mac.mm:51-58`): `low` ->
  QOS_CLASS_UTILITY, `background` -> BACKGROUND, `normal` -> DEFAULT. Decode threads run `normal`
  (`VideoPlayer.h:207`); the existing pools: `ClipThumbnails` 2 x low (`src/ui/ClipThumbnails.h:107-108`),
  `MediaPresenceSweeper` 1 x low (`MediaPresence.h:155-156`), `ImageDecode` N x low (`src/render/ImageDecode.h:123-124`).
  Pool pattern to copy (WeakReference landings, `masterReference.clear(); pool_.removeAllJobs(true, 5000)` in the
  destructor, jobs capture no `this`): `ClipThumbnails.h:32-36, :57-66`.
- `juce::ThreadPool::removeAllJobs(interruptRunning, timeOutMs)` (`juce_ThreadPool.cpp:279-`): removes queued jobs, waits
  for active ones up to the timeout -- INFERRED that `timeOutMs = 0` returns without waiting (the visible head collects
  `jobsToWaitFor`; the builder verifies the tail before relying on it, 5.3).
- The compositor's pending rule (`src/render/CompositorEngine.cpp:1106-1124`): pending -> hold `heldLayerOutput(layer,
  deck)`; nothing to hold -> the layer draws nothing (`image_skip_frames`), never FX-only; texture 0 and NOT pending ->
  FX-only (:1120-1124). `postImageSet` / `ImageTexCache::applyImageSet` (`ImageTexCache.h:150-172`): entries not in the
  set are dropped, the rest queued for prefetch one at a time.
- `MainComponent.h` member order (destruction reverse): `audioEngine_` :270, `analysisThread_` :275, `fileLabel_` :292,
  `previewPanel_` :300 (the Renderer), `composition_` :379, `undoService_` :381, `deckView_` :400, `presence_` :404,
  `mediaStateVar` :405, `testMode_` :598; decls `loadComposition` :139, `openMediaForDeck` :155, `appendDeckFromFile`
  :156, `duplicateDeck` :167, `debugDropFiles` :484. `CMakeLists.txt:93-97` lists sources explicitly (core .cpp at
  :171-173). Test target shapes: pure (`tests/CMakeLists.txt:2747-2750`), model (:2704-2720), VideoPlayer + FFmpeg in a
  forked child (:2757-2770, `tests/test_video_player_open.cpp:1-40`; control fixture `tests/fixtures/video_h264_64x64.mp4`,
  RED fixture `video_h264_cut_header.mp4`).
- Not present in src (grep): any autosave. The recorder (`recorderHost_`) is not touched by `swapCompositionModel`.

## 2. WHAT THE 1.1 s IS (attribution) and the FIRST commit's measurement gate

Attribution from source + the lane measurements (VERIFIED chain, INFERRED split): `loadComposition` on the message thread
= parse (1-3 ms for a 16-clip file, INFERRED) + validate / re-mint / reconcile (us) + 16 x `VideoPlayer::open` + 16 x
`installVideoPlayer` (a thread start each, ~0.1 ms) + the fence drain (<= one GL frame, 8-12 ms with 4 x 4K playing:
video.md callback p90 3.8-6.5, max 16.7) + `rebuildGrid` / inspector refresh / browser refresh. diag-media S2 measured
99-103 ms per 4K `open()` INCLUDING the old 31.9 ms full-frame thumbnail; the video lane's thumbnail is 0.85-1.38 ms
(video.md "Thumbnail at open"), so a 4K open is now ~68 ms: 16 x 68 = 1.09 s = the HANDOFF's "~1.1 s". The non-open
part of a load is bounded by mediaopen row m4 (a two-sequence load, no video opens): 17.9-20.4 ms message-thread peak.
So >= 95 % of the 1.1 s is the 16 opens (avformat probe + `avcodec_open2` with 2 frame threads + the first 4K decode +
one 4K `sws_scale` + the thumbnail); parse, presence stats, sequence opens, thumbnails and the grid rebuild are <= 25 ms
together. What moves: ONLY the opens. What stays on the message thread: parse, validate, re-mint, reconcile, presence
seed, 16 landings (a walk of the staged decks + 3 field writes + install each), the fence, the swap, the grid.

**Commit 1 confirms this before anything moves** (TEST-ONLY, no behaviour change): `LoadTiming` scopes
(`std::chrono::steady_clock`) in `loadComposition` / `appendDeckFromFile` / `duplicateDeck` / `openMediaForDeck` record
the LAST load's `parse_ms`, `prep_ms` (validate + remint + reconcile + seed), `opens_ms`, `open_count`, `open_max_ms`,
`seq_ms`, `swap_ms` (the `swapCompositionModel` / `pushCommands` call), `ui_ms` (rebuild + refresh + label), `total_ms`,
published as `/api/state load.timing {...}` (a mutex-guarded struct written on the message thread, copied by the HTTP
thread). Gate: probe row a2 on the commit-1 app prints them; PROCEED only if `opens_ms / total_ms >= 0.8` (predicted
0.95-0.98, opens_ms 1000-1200, open_max_ms 55-90). If it is NOT the opens, the lane STOPS at commit 1 and reports the
split (the plan's remaining commits assume the opens; a parse- or grid-dominated load needs a different design --
say so, do not improvise).

## 3. RULINGS (every fork the task names; the source that decides each)

- **R1 REST semantics (A8).** `POST /api/load_composition` keeps its request / response SHAPE and answers `{"ok":true}`
  only AFTER the staged swap completed (or at once for an empty batch, as today) -- a waitable `LoadTicket` the handler
  blocks on (bounded, 60 s). Decided by `ApiServer.cpp:1058-1066` (fire-and-forget) + the FIFO dependence of every
  caller in section 1: with the wait, a caller's next request is posted after the cut exactly as it is today (its
  `callAsync` used to queue behind the synchronous load). Failure paths (`jsonFail` :1028-1056) unchanged. NEW outcomes
  no existing caller can see: `{"ok":false,"reason":"superseded by a newer load"}` (a newer load / New Composition /
  an explicit cancel while it was staged), `{"ok":false,"reason":"cancelled"}` (quit), `{"ok":false,"reason":"timed
  out"}` (60 s). No OSC / TestServer / MIDI change (they have no load). TEST-ONLY `POST /api/debug/load_deck` and
  `/api/debug/duplicate_deck` are fire-and-forget (rows poll).
- **R2 A trigger during the window (A4).** Any trigger / switch / edit that reaches the message thread while a load is
  staged -- key, MIDI, OSC, a REST request from ANOTHER connection -- acts on the LIVE composition (the one on screen)
  and is NOT replayed onto the staged one. Decided by BORIS_DECISIONS "this is a playing app" (:342) + "keep playing"
  (:354-355): the show the performer sees must answer; and by section 1's caller survey (no automation triggers from a
  second connection before its load answered; a single connection cannot, R1). Documented as the contract; row a4 proves
  it both ways.
- **R3 Pool size and priority.** `juce::ThreadPool` of **2** threads at **`Priority::low`** (QOS_CLASS_UTILITY), named
  "MediaOpen". Arithmetic on the M1 Pro (8P + 2E): a show at 4 x 4K keeps ~3 P cores in decode threads (video.md %cpu
  288-314), the GL thread ~1, analysis / message / WindowServer ~1 -> 3-4 P cores + 2 E cores free. One open is ~1-2
  cores busy for ~68 ms (the decoder's 2 frame threads, `VideoPlayer.cpp:108`). Two low-priority openers use the free
  cores and yield to every `normal` decode thread of the OLD composition (`VideoPlayer.h:207`) -- the visible failure
  (a hold on the projector while a load runs) is the one to avoid; a later cut is the cheaper failure because the UI
  is live and the show plays. Predicted 16 x 4K wall: 16 x 68 / 2 = 0.55 s on P cores, ~1.1 s if UTILITY lands both on
  E cores (INFERRED: Apple schedules UTILITY efficiency-biased; BACKGROUND is E-only) -- bar `swapMaxS 2.5`. The audio
  RT callback cannot be preempted by QoS threads (A3 is therefore a GUARD, R9). 4 threads would add 4 x 3 runnable
  threads against an upload-bound show (w2: 4 x 4K is already at 84-90 fps) for a 0.3 s gain nobody sees -- rejected.
  Commit 1 records per-open wall times (`open_max_ms`) so Harmony can re-rule with data.
- **R4 One staged load at a time; newest wins.** A load / append / duplicate begun while one is staged CANCELS it
  (adopted ids retired through `closeMediaForClip`, its ticket Superseded, its label restored, queued pool jobs dropped,
  in-flight opens land Stale and are destroyed unpublished). Decided by plan-mediaopen 4.4 (adopted) and the label rule
  (A2): a queue of staged loads would need per-batch labels and ordering rules for nothing Boris asked for. Corner: two
  Duplicate clicks within the window yield ONE copy (today: two) -- open question Q3.
- **R5 Staged ids vs live ids (Pitfall 36).** Staged clips are re-minted from `s_nextClipId` BEFORE any job is issued
  (steps 3 / `compload::duplicateDeck`), the mint is monotonic and never reused, drops mint from the same counter on the
  same thread -> a staged id can never equal a live id or a drop's id. Adopted staged players sit in `videoPlayers_`
  beside the live ones; the GL thread reaches a player only by a LIVE deck's clip id (`syncMedia` :1747-1758), so a
  staged player is invisible until the cut. `makeClipMediaDisposeHook`'s liveness scan (:4783-4791) walks the LIVE model
  only; a staged id is never in it and never closed by it.
- **R6 A staged player's first frame is ready at the cut (Pitfall 56).** `open()` decoded frame 0 into slot 0 with pts
  <= 0 (:204-216); the installed thread parked before decoding anything else (`idleStep` with `lastDrawMs_ = 0`); the
  first draw after the cut picks slot 0 -> `Shown::New`, never pending, never a hold. A first frame that never decodes
  behaves as today (pending -> FAILED after 2 s, W3). Sequences: opened at COMPLETION (no I/O) and their frame 0 decodes
  on first draw as today (pending -> hold / draw nothing, never FX-only: `CompositorEngine.cpp:1106-1118`); pre-warming
  a sequence's frame 0 needs GL-thread machinery inside seqvram-owned code -- OUT OF SCOPE, filed (Q4).
- **R7 What the user may do during the window** (each on the LIVE model; the staged model is private):
  | action | rule |
  |---|---|
  | edit the live composition (drop, param, trigger, effects, opacity) | allowed; a Composition load replaces it at the cut (as today's confirm says: "Everything playing now will be replaced"); Append / Duplicate keep it |
  | Save / Save As | saves the LIVE composition (the staged one is not the composition yet); after the cut, the new one |
  | deck switch | on the live composition; the cut applies the file's `activeDeckIndex` (Composition) or makes the inserted deck active (Append / Duplicate) -- as today |
  | undo / redo | on the live history; a Composition cut clears it (`undoManager_.clear()` :2970, as today); Append / Duplicate push their `InsertDeckCmd` at completion; a RemoveDeck undone meanwhile does not disturb an append (it appends at the end) |
  | another load / append / duplicate / New Composition | cancels the staged one (R4) |
  | a drop onto a cell | as today (`prepareFileDrop` opens synchronously -- unchanged, out of scope) |
  | a routine firing | on the live model; a Composition cut runs `routineEngine_.stopAll()` (:2938, as today); Append / Duplicate never stop routines |
  | the take recorder / Audio Store | untouched by any load path today (`recorderHost_` is not referenced by :2934-2973) -- unchanged |
  | settings (`AppSettings`: outputs, milkDropPresetDir) / autosave | no interaction; no autosave exists (grep) |
  | quit | `~MainComponent` cancels the staged load FIRST (adopted players -> retire list, drained by `detach()`'s context-closing drain), then stops the servers (their waiters released by `ApiServer::stop()` itself, 5.5); `~MediaOpener` waits <= one in-flight open (<= ~100 ms at 4K) |
- **R8 Failure mid-batch.** A missing file: skipped at staging exactly as :3027 (`mediaMissing` seeded; no job; the clip is
  "no media" until presence sees it back). An undecodable / truncated file: `open()` returns false on the pool thread
  (the pix_fmt guard :163-169 or any earlier failure) -> the landing carries a null player -> the staged clip keeps its
  file defaults (no dims, no thumbnail) exactly as today's :3028 false branch; `opens_failed` +1; the batch continues;
  the swap runs when the last landing (failed or not) is in. A file that opens but never decodes: as today (W3).
  Nothing in a batch can abort the load; a parse / validate failure stays synchronous and never stages.
- **R9 The audio witness (A3).** Three witnesses, none touching Sacred Rule 1: (1) PRIMARY `audio_xruns` =
  `AudioIODevice::getXRunCount()` -- the OS's own verdict that the callback overran its deadline, counted by CoreAudio's
  `kAudioDeviceProcessorOverload` listener at zero callback cost, sampled on the message thread's 30 Hz timer into an
  atomic; (2) `audio_callback_gap_max_ms` -- in the callback, `#if AUDIODNA_TEST_SERVER` only: one
  `Time::getHighResolutionTicks()` (mach_absolute_time, no syscall) and a relaxed max store of the inter-arrival gap,
  reset on read -- shows the MARGIN a coarse overload count hides; (3) `analysis_ring_overruns` -- `RingBuffer::push`
  returned fewer than asked (one compare + a relaxed add on the overrun path only): the consumer (analysis thread) was
  starved. Why not a callback-duration timer: it would measure our own memcpy, which a load cannot change; why not
  AudioTap's gap markers: they exist only while a take is armed. No allocation, lock or syscall in any of the three.
  RED honesty: the audio callback runs at RT priority and the load never ran on it, so no build can RED by value; A3 is a
  GUARD (adoption P4 logic) whose RED is the fields' absence on main plus a data-only teeth arm (8 x `highest` pool) in
  the report.
- **R10 The heartbeat bar (A6).** RED today: `peak_message_stall_ms` 1000-1200 on a 16 x 4K load, 380-450 on 16 x 1080p
  (16 x 60-70 / 16 x 25 ms; HANDOFF "~1.1 s"). GREEN bar 50 ms (the mediaopen probe's `stallMaxMs`): m4's whole-load
  cost with no opens 17.9-20.4 ms + one GL-frame drain with 4 x 4K playing <= 12 ms + 16 landings x ~0.2 ms (a walk of
  <= 64 cells, 3 writes, an install with a thread start) = ~36 ms predicted; 50 is 1.4x. Not a re-threshold of anything.
- **R11 The cut (A7).** (i) `fence_black_frames` +0 across the cut, `fence_hold_frames` delta <= 3 (the swap's fence
  holds the canvas: Pitfall 55); (ii) the OLD composition keeps animating: `video_uploads` of its playing 1080p clip
  keeps rising during the window and its `playheadPosition` advances; (iii) no pending / FX-only at the cut:
  `video_pending_frames` +0 and `videos_pending` 0 across the Duplicate cut (the copy's active 4K clip draws frame 0 at
  once) and across the first trigger after a Composition cut; (iv) decoded pixels: the capture after the Duplicate cut /
  the first trigger equals the video's reference (dbox <= 3). A bare Composition / Append cut shows the clear colour by
  DESIGN (no active clip until triggered, section 1) -- not a black-frame defect; the rows measure what the design
  draws.
- **R12 Sequences (A1).** A staged batch opens NO sequence until its completion (no I/O, `ImageSequence::open` is O(files)
  string copies), so a cancelled batch never leaves a sequence in `imageSequences_`; the bookkeeping is nevertheless
  generic -- `StagedLoad::adopted` records EVERY media id handed to the renderer (video at landing, sequence at
  completion-before-swap) and `cancelStagedOpen` retires all of them -- so a future "warm sequences early" cannot leak
  (the threads MUST, kept by construction and by ctest).
- **R13 Labels (A2, A5).** `loadingText = "Loading <name>..."` where name = file base name (Composition, DeckAppend) or
  `copy.name` = "<deck> copy" (DeckDuplicate -- the threads NIT). Set at staging when the batch is non-empty; the
  completion writes today's final text; a cancel restores the text the label had BEFORE staging IFF it still reads this
  batch's `loadingText` (a trigger's label write meanwhile is kept: `refreshPreviewFromActiveClip` writes the label at
  :4695-4714). Pure functions, ctested.
- **R14 Image prefetch.** NOT posted at staging: `applyImageSet` drops entries not in the set (`ImageTexCache.h:155-165`),
  so a staged-only set would black the live show, and a union buys nothing -- a Composition / Append cut has no active
  image (section 1), a Duplicate's images are the same paths already resident. `postImageSet` stays where it is (:2964).
- **R15 Thread safety of the witnesses.** `MediaOpener` mirrors its ledger into relaxed atomics (`pending_, batches_,
  stale_, failed_`) for `/api/state` (HTTP thread); `LoadTiming` is copied under a mutex; the audio counters are relaxed
  atomics; `audio_xruns_` is an atomic written on the message thread.
- **R16 Fence for this lane.** `src/media/VideoPlayer.*`, `ImageSequence.*`, `SeqVram.h`, `VideoRing.h`, `Renderer.*`
  (nothing needed: the seam and `getVideoStats()` exist), `UndoService.*`, `FencedPtrSlot.h`, `CompositionLoad.h`,
  `DeckCommands.h`: NOT edited. Edited: `MainComponent.h/.cpp`, `ApiServer.h/.cpp`, `TestServer.h/.cpp` (state field),
  `CombinedCallback.h`, `AudioCallback.h/.cpp`, `AudioEngine.h`, NEW `src/core/LoadTicket.h`, `src/core/StagedLoad.h`,
  `src/core/MediaOpener.h/.cpp`, `src/core/LoadTiming.h`, tests, probe, docs, `CMakeLists.txt` (+1 .cpp).

## 4. TRADEOFFS CONSIDERED (rejected, with why)
- Keep `/api/load_composition` fire-and-forget + a `load_id` the caller polls: every one of the 23 callers in section 1
  would have to change (A8 forbids) -- rejected. A hybrid "wait unless `?async=1`" is free to add later; not now.
- Queue every message-thread command that arrives during the window and replay it after the cut: makes a MIDI pad go
  dead for ~1 s and contradicts "keep playing" (R2) -- rejected.
- Open on the message thread but interleaved with the message loop (one open per timer tick): the UI stays live but each
  tick still freezes 68 ms at 4K (above the 50 ms bar) and the cut takes the same 1.1 s -- rejected.
- One pool thread: 16 x 68 ms x ~2 (E-core) = 2.2 s worst case against a 2.5 s bar -- too close; 4+ threads: R3.
- Adopt players WITHOUT starting their thread and start all at the cut: needs a second Renderer entry point beside
  `installVideoPlayer` for no gain (an installed thread parks at once, R6) -- rejected.
- Open sequences at staging and track them (plan-mediaopen 4.4 + the threads MUST): opening at completion is leak-proof
  by construction and costs microseconds (R12) -- adopted instead, with the generic retire list kept.
- Prefetch the staged composition's images during the window (union set): R14.
- Move the JSON parse to the pool: 1-3 ms measured class (m4), and re-minting must happen on the message thread anyway
  -- rejected; commit 1's `parse_ms` confirms.

## 5. DECISION / SPEC (exact changes; line numbers at adf9b8a)

### 5.0 Builder step 0 (rig)
Worktree lane `asyncload` from main adf9b8a (df -h first; 8 GB/lane). Copy `.harmony/.reports/s-rta-0928b/wf/lock.sh`
to the scratchpad, fix SPL, `LANE=asyncload`; `acquire_quiet_lock` before every live batch; `open -g` only; production
mode 7070 on the TEST_SERVER build (`-DAUDIODNA_BUILD_TEST_SERVER=ON`, as the mediaopen probe); never an Output window;
no synthetic input; count `UserNotificationCenter` windows == 0 after every batch (a7 opens a truncated file: `open()`
fails cleanly since fix round 2, but check); >= 5 runs per arm for any flake verdict; perf bars only from quiet numbers.
Keep app copies per commit (`apps/c1 .. final`) for RED arms. Verify at step 0 (record in the report): (a)
`juce::ThreadPool::removeAllJobs(false, 0)` returns without waiting for active jobs (`juce_ThreadPool.cpp:279-` tail);
(b) `MessageManager::callAsync` returns false after quit began (`:109` + the post refusal) -- both INFERRED here.

### 5.1 `src/core/LoadTicket.h` (NEW, juce_core only; `tests/test_load_ticket.cpp`)
```cpp
// s-rta-0929 asyncload: the waitable outcome of ONE composition / deck load. The REST handler waits on it (any thread);
// the message thread finishes it once (Done at the staged swap, Failed on a synchronous refusal, Superseded when a newer
// load / New Composition / an explicit cancel retires the staged batch); ApiServer::stop() finishes every outstanding
// ticket Cancelled before joining its workers. The first finish wins; a wait that times out reports Pending.
struct LoadTicket {
    enum class Outcome : int { Pending = 0, Done, Failed, Superseded, Cancelled };
    bool finish(Outcome o);            // true when this call set it (compare_exchange from Pending), then signals
    Outcome wait(int timeoutMs);       // WaitableEvent::wait(timeoutMs); returns outcome() (Pending on timeout)
    Outcome outcome() const { return static_cast<Outcome>(outcome_.load(std::memory_order_acquire)); }
private:
    std::atomic<int> outcome_{ 0 };
    juce::WaitableEvent done_;
};
```
### 5.2 `src/core/StagedLoad.h` (NEW, pure: `<cstdint> <string> <vector> <algorithm>`; `tests/test_staged_load.cpp`)
```cpp
namespace stagedload {
enum class Kind : uint8_t { Composition, DeckAppend, DeckDuplicate };
// The ONE live asynchronous open batch: begin() cancels the live batch and mints a generation; land(gen) is Apply for
// the live generation (++done) and Stale otherwise; complete(gen) = live and done >= total; total 0 = complete at once.
class Ledger { public:
    uint64_t begin(int total); enum class Landing { Apply, Stale }; Landing land(uint64_t gen);
    bool complete(uint64_t gen) const; void cancel(); int pending() const; uint64_t liveGen() const; uint64_t batches() const;
private: uint64_t gen_ = 0; int total_ = 0, done_ = 0; bool live_ = false; };
// Labels (A2 / A5): the text while a batch is staged, the final text, and what a cancel restores.
std::string loadingLabel(Kind k, const std::string& name);             // "Loading <name>..." for every Kind
std::string doneLabel(Kind k, const std::string& name);                // "Loaded: " / "Loaded deck: " / "Duplicated deck: "
std::string labelAfterCancel(const std::string& current, const std::string& loadingText, const std::string& before);
                                                                        // current == loadingText ? before : current
// Every media id a batch handed to the renderer (video at landing, sequence at completion); a cancel retires them all.
struct Adopted { std::vector<uint32_t> ids; void add(uint32_t id); std::vector<uint32_t> takeAll(); };
}
```
Name for DeckDuplicate = the COPY's `Deck::name` ("<name> copy"); for Composition / DeckAppend = `file.getFileNameWithoutExtension()`.

### 5.3 `src/core/MediaOpener.h/.cpp` (NEW; JUCE + VideoPlayer; the app only; `CMakeLists.txt:171-173` block + 1 line)
```cpp
// s-rta-0929 asyncload: opens VideoPlayers OFF the message thread. One job per clip on a 2-thread LOW-priority pool
// (R3): the job makes a NEW player (setStats(stats), then VideoPlayer::open -- no GL, no thread, no Renderer state on an
// unpublished object) and posts it to the message thread, where landed() applies or drops it by generation (StagedLoad
// Ledger) and calls onLanded (Apply) / onDone (the last Apply of the live batch). An EMPTY batch runs onDone inside
// begin(). Jobs capture a WeakReference + the job data only; a landing after the owner is gone destroys the player on
// the message thread (unpublished: no GL object, no thread). cancel() never waits: it drops the QUEUED jobs
// (removeAllJobs(false, 0)) and lets an in-flight open land Stale. The destructor waits <= 5 s for an in-flight open.
class MediaOpener {
public:
    struct Job { uint32_t clipId = 0; juce::File file; };
    using Landed = std::function<void(uint32_t clipId, std::unique_ptr<VideoPlayer>)>;   // null = open() failed
    using Done = std::function<void()>;
    explicit MediaOpener(VideoStats* stats);   ~MediaOpener();   // masterReference.clear(); pool_.removeAllJobs(true, 5000)
    void begin(std::vector<Job> jobs, Landed onLanded, Done onDone);   // message thread; cancel() first
    void cancel();                                                     // message thread; non-blocking (5.0 (a))
    int pending() const; uint64_t batches() const; uint64_t stale() const; uint64_t failed() const;   // atomics, any thread
    static constexpr int kThreads = 2; static constexpr juce::Thread::Priority kPriority = juce::Thread::Priority::low;
private:
    void landed(uint64_t gen, uint32_t clipId, std::unique_ptr<VideoPlayer> player);
    stagedload::Ledger ledger_; Landed onLanded_; Done onDone_; VideoStats* stats_;
    std::atomic<int> pending_{0}; std::atomic<uint64_t> batches_{0}, stale_{0}, failed_{0};
    juce::ThreadPool pool_{ juce::ThreadPoolOptions{}.withNumberOfThreads(kThreads).withThreadName("MediaOpen")
                                                     .withDesiredThreadPriority(kPriority) };
    JUCE_DECLARE_WEAK_REFERENCEABLE(MediaOpener)
};
```
`begin`: `cancel(); gen = ledger_.begin(n); ++batches_; pending_ = n; store callbacks; if (n == 0) { auto d = std::move(onDone_);
onDone_ = nullptr; ledger_.cancel(); if (d) d(); return; }`. Per job: `pool_.addJob([self = WeakReference(this), gen, job,
stats = stats_] { auto p = std::make_unique<VideoPlayer>(); p->setStats(stats); if (!p->open(job.file)) p.reset(); auto box =
std::make_shared<Box>(gen, job.clipId, std::move(p)); juce::MessageManager::callAsync([self, box] { if (auto* o = self.get())
o->landed(box->gen, box->clipId, std::move(box->player)); }); })` (a `std::function` must be copyable: the shared box).
`landed`: `if (ledger_.land(gen) == Stale) { ++stale_; return; }` (the player dies here); `--pending_; if (!player) ++failed_;
onLanded_(clipId, std::move(player)); if (ledger_.complete(gen)) { auto d = std::move(onDone_); onDone_ = nullptr;
ledger_.cancel(); if (d) d(); }` -- moved out FIRST and nothing touched after `d()` returns (the completion may `begin()` the
next batch or `cancel()` re-entrantly: `swapCompositionModel` cancels at its top). `cancel`: `ledger_.cancel(); pending_ = 0;
pool_.removeAllJobs(false, 0); onLanded_ = onDone_ = nullptr;`. Test injection as `ClipThumbnails::setBackendsForTests`:
`setPosterForTests(std::function<void(std::function<void()>)>)` so a ctest drains landings by hand (no message loop).

### 5.4 `MainComponent.h/.cpp` -- the staged flow
`MainComponent.h`: `#include "core/LoadTicket.h"`, `"core/StagedLoad.h"`, `"core/MediaOpener.h"`, `"core/LoadTiming.h"` beside
:42-49. :139 -> `void loadComposition(const juce::File& file, std::shared_ptr<LoadTicket> ticket = nullptr);`. :155
`openMediaForDeck` -> removed; in its place:
```cpp
    // s-rta-0929 asyncload (Pitfall NN): a composition / deck load is STAGED -- its video players open on MediaOpener's
    // pool, each landing writes the STAGED clip (never a live Clip) and installs the player under its re-minted id; the
    // last landing runs the completion (staged sequences opened, then today's swap / InsertDeckCmd / label). One staged
    // load at a time: a newer one, swapCompositionModel (any caller), an explicit cancel or destruction retires it.
    struct StagedLoad {
        stagedload::Kind kind; Composition comp; Deck deck; juce::File file; std::string name;
        juce::String loadingText, labelBefore; stagedload::Adopted adopted; std::shared_ptr<LoadTicket> ticket;
    };
    void beginStagedOpen(std::unique_ptr<StagedLoad> staged);            // seeds presence, queues the videos (or completes)
    void onStagedLanded(uint32_t clipId, std::unique_ptr<VideoPlayer> player);
    void finishStagedLoad();
    void cancelStagedOpen(LoadTicket::Outcome why);                      // retire adopted ids, restore the label, finish the ticket
    Clip* findStagedClip(uint32_t clipId);                               // a walk of staged_->comp.decks or staged_->deck
    juce::var loadWitnessVar() const;                                    // /api/state "load" (any thread: atomics / a mutex copy)
```
Members after :405 (`mediaStateVar`): `std::unique_ptr<StagedLoad> staged_; MediaOpener mediaOpener_{
&previewPanel_.getRenderer().getVideoStats() }; LoadTiming loadTiming_; std::atomic<int> audioXruns_{ -1 };` -- declared after
`previewPanel_` (:300, the Renderer outlives them) and after `composition_` / `deckView_` / `presence_`.

`MainComponent.cpp`:
- `loadComposition(file, ticket)` (:3071-3137): steps 1-3b unchanged; on the two early returns (:3085, :3098) add
  `if (ticket) ticket->finish(LoadTicket::Outcome::Failed);`. Replace :3112-3136 with: `auto s = std::make_unique<StagedLoad>();
  s->kind = Composition; s->comp = std::move(incoming); s->file = file; s->name = base name; s->comp.name = s->name (step 5,
  unchanged text); s->ticket = std::move(ticket); beginStagedOpen(std::move(s));`.
- `appendDeckFromFile` (:3214-3290): steps 1-3b unchanged; :3259 removed; step 5 (:3265-3266) stays on `incoming`; :3268-3289
  -> `StagedLoad{ DeckAppend, deck = std::move(incoming), file, name }` + `beginStagedOpen`. The completion is :3277-3289 verbatim
  (`InsertDeckCmd` from `staged->deck`, rebuild, refresh, label = `doneLabel`, browser refresh).
- `duplicateDeck` (:3435-3457): :3446 removed; `StagedLoad{ DeckDuplicate, deck = std::move(copy), name = copyName }`; the
  completion is :3448-3456 verbatim.
- `beginStagedOpen(staged)`: `cancelStagedOpen(Superseded); staged_ = std::move(staged);` then for each staged deck
  (`comp.decks` or `deck`): `presence::seed(deck)` (today's :3017); for each playable Video clip with `!mediaMissing` push
  `Job{ clip.id, clip.mediaFile }` (a missing one is skipped as :3027). Sequences: nothing here (R12). If `!jobs.empty()`:
  `staged_->labelBefore = fileLabel_.getText(); staged_->loadingText = loadingLabel(kind, name); fileLabel_.setText(loadingText)`.
  `mediaOpener_.begin(std::move(jobs), [this](uint32_t id, std::unique_ptr<VideoPlayer> p) { onStagedLanded(id, std::move(p)); },
  [this] { finishStagedLoad(); })` -- `this` captures are safe: `mediaOpener_` is a member destroyed before `this` and every
  landing goes through its WeakReference.
- `onStagedLanded(id, player)`: `Clip* clip = findStagedClip(id); if (!clip) { return; }` (the player dies unpublished --
  cannot happen while staged_ is live, defensive); `if (!player) return;` (R8: the clip keeps its defaults, as :3028 false);
  `clip->hasAlpha = player->hasAlpha(); clipWidth / clipHeight; clip->thumbnail = player->getThumbnail(90, 72)` (:3032-3035
  verbatim); `previewPanel_.getRenderer().installVideoPlayer(id, std::move(player)); staged_->adopted.add(id);`.
- `finishStagedLoad()`: `auto s = std::move(staged_); if (!s) return;` then open the staged SEQUENCES under their ids
  (`openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps)` for every playable ImageSequence clip with
  non-empty files -- today's :3041-3044; `s->adopted.add(id)` each) BEFORE the swap (the GL thread finds them by id at the
  first frame after the cut); then by Kind: Composition -> `swapCompositionModel([&] { composition_ = std::move(s->comp); });
  fileLabel_.setText(doneLabel); browser refresh` (:3131-3136 verbatim); DeckAppend / DeckDuplicate -> the verbatim
  completions above. Finally `if (s->ticket) s->ticket->finish(Done)`. `swapCompositionModel`'s own `cancelStagedOpen` finds
  `staged_` null (moved out) -> no-op.
- `cancelStagedOpen(why)`: `mediaOpener_.cancel(); if (!staged_) return; auto s = std::move(staged_); for (id :
  s->adopted.takeAll()) renderer.closeMediaForClip(id); if (s->loadingText.isNotEmpty()) fileLabel_.setText(
  labelAfterCancel(fileLabel_.getText(), s->loadingText, s->labelBefore)); if (s->ticket) s->ticket->finish(why);`.
- `swapCompositionModel` (:2934): first statement `cancelStagedOpen(LoadTicket::Outcome::Superseded);` (before `stopAll`).
- `~MainComponent` (:2357): first statement `cancelStagedOpen(LoadTicket::Outcome::Cancelled);` -- BEFORE `apiServer_->stop()`
  (:2370): the adopted players go to the retire list that `renderer.detach()` (:2397) drains context-closing; the opener's
  destructor (member order) later waits <= one in-flight open.
- :1909 -> `apiServer_->onLoadComposition = [this](juce::File f, std::shared_ptr<LoadTicket> t) { loadComposition(f, std::move(t)); };`.
  New TEST-ONLY wiring beside :2138-2140: `onDebugLoadDeck = [this](juce::File f) { appendDeckFromFile(f); }`,
  `onDebugDuplicateDeck = [this](int i) { duplicateDeck(i); }`, `onDebugCancelLoad = [this] {
  cancelStagedOpen(LoadTicket::Outcome::Superseded); }`, `onDebugUiText = [this] { return fileLabel_.getText(); }` (runs on
  the message thread; the server marshals). Providers before `start()` at :1873 / :2136: `setLoadWitnessProvider([this] {
  return loadWitnessVar(); })` on both servers.
- `timerCallback` (:3825-3829): after `pollDisplays()`: `if (auto* d = audioEngine_.getDeviceManager().getCurrentAudioDevice())
  audioXruns_.store(d->getXRunCount(), std::memory_order_relaxed);`.
- `loadWitnessVar()`: `{ "opens_pending", "open_batches", "opens_stale", "opens_failed" }` from `mediaOpener_`; `"staged": 0/1`;
  `"timing": loadTiming_.var()`; `#if AUDIODNA_TEST_SERVER`: `"audio_xruns": audioXruns_`, `"audio_callbacks"`,
  `"audio_callback_gap_max_ms"` (reset on read), `"audio_callback_period_ms"` (period samples / `sourceSampleRateCell()` x 1000),
  `"analysis_ring_overruns"` from `audioEngine_` (5.7).
- `handleClipTrigger`'s lazy open (:4445-4450), `makeClipMediaHook` (:4734-4760), `prepareFileDrop` (:4948-4995): UNCHANGED.

### 5.5 `ApiServer.h/.cpp` -- the wait, the registry, the TEST-ONLY routes, the state object
- `ApiServer.h:67-70`: `std::function<void(juce::File, std::shared_ptr<LoadTicket>)> onLoadComposition;` (comment: answers
  after the staged swap; R1). New: `void setLoadWitnessProvider(std::function<juce::var()>)` beside :168; TEST-ONLY
  callbacks beside :170-173: `onDebugLoadDeck(juce::File)`, `onDebugDuplicateDeck(int)`, `onDebugCancelLoad()`,
  `std::function<juce::String()> onDebugUiText`; handler decls beside :230-238; members: `std::mutex ticketsMutex_;
  std::vector<std::weak_ptr<LoadTicket>> tickets_;` `static constexpr int kLoadWaitMs = 60000;` `std::function<juce::var()>
  loadWitnessProvider_;`.
- `handleLoadComposition` (:1058-1066) becomes: `auto ticket = std::make_shared<LoadTicket>(); { lock; prune expired;
  tickets_.push_back(ticket); } const bool posted = juce::MessageManager::callAsync([this, f, ticket] { onLoadComposition(f,
  ticket); }); if (!posted) { res = jsonFail("cancelled"); return; } switch (ticket->wait(kLoadWaitMs)) { case Done:
  jsonOk(); case Failed: jsonFail("could not load"); case Superseded: jsonFail("superseded by a newer load"); case Cancelled:
  jsonFail("cancelled"); case Pending: jsonFail("timed out"); }`. The pre-validate (:1042-1056) stays (it still answers the
  two refusals without touching the message thread).
- `stop()` (:103-113): FIRST `{ lock; for (auto& w : tickets_) if (auto t = w.lock()) t->finish(Cancelled); tickets_.clear(); }`
  then `server_.stop()` + join -- no worker can be blocked when httplib joins. (The message thread does not pump during
  `~MainComponent`, so a ticket posted after quit began would never finish: `callAsync`'s false return answers it at once,
  and this release covers a post that slipped in before.)
- TEST-ONLY routes (inside :286-294): `GET /api/debug/ui_text` -> `{ "ok": true, "file_label": "<text>" }` via
  `callAsync` + a `juce::WaitableEvent` (2 s; on timeout `{"ok":false,"reason":"message thread did not answer within 2 s"}`)
  -- also a responsiveness witness; `POST /api/debug/load_deck {"path"}` (absolute, exists) -> `callAsync(onDebugLoadDeck)`,
  `jsonOk()` at once; `POST /api/debug/duplicate_deck {"deck": i}` -> `callAsync(onDebugDuplicateDeck)`; `POST
  /api/debug/cancel_load` -> `callAsync(onDebugCancelLoad)`. All shaped as `handleDebugDropFiles` (:1722-1750: 400 on a
  malformed body, 503 unwired).
- `/api/state` (:1401-1402 and `TestServer.cpp:677-678`): after `media`, `if (loadWitnessProvider_) obj->setProperty("load",
  loadWitnessProvider_());` -- same field on both servers (`TestServer.h:75/:174` shape).

### 5.6 `src/core/LoadTiming.h` (NEW, header-only, juce_core; commit 1)
`struct LoadTiming { struct Scope {...}; void begin(); void note(const char* field, double ms); void end(); juce::var var() const;
private: mutable std::mutex m_; struct Last { double parse_ms, prep_ms, opens_ms, open_max_ms, seq_ms, swap_ms, ui_ms, total_ms;
int open_count; } last_, cur_; }` -- written on the message thread by RAII scopes placed in the three load paths (commit 1 on
the OLD code: around :3077-3086 parse, :3091-3110 prep, :3119-3120 opens (per-open max inside `openMediaForDeck` :3028),
:3131 swap, :3134-3136 ui; the same for :3221-3289 and :3444-3456), read by `loadWitnessVar()`. After commit 4 the scopes stay:
`opens_ms` then measures the landings' message-thread cost (the witness that they are cheap).

### 5.7 The audio witness (`src/audio/CombinedCallback.h`, `AudioCallback.h/.cpp`, `AudioEngine.h`; commit 1)
- `CombinedCallback.h`, inside `#if AUDIODNA_TEST_SERVER`: members `int64_t lastTicks_ = 0;` (callback-local),
  `std::atomic<int64_t> gapMaxTicks_{0}; std::atomic<uint64_t> callbacks_{0}; std::atomic<int> periodSamples_{0};`. At the
  top of `audioDeviceIOCallbackWithContext` (:46, before :51): `const int64_t now = juce::Time::getHighResolutionTicks(); if
  (lastTicks_ != 0) { const int64_t gap = now - lastTicks_; if (gap > gapMaxTicks_.load(relaxed)) gapMaxTicks_.store(gap,
  relaxed); } lastTicks_ = now; callbacks_.fetch_add(1, relaxed); periodSamples_.store(numSamples, relaxed);`.
  `audioDeviceAboutToStart` (:142): `lastTicks_ = 0`. Accessors: `takeGapMaxMs()` (exchange 0, ticks -> ms via
  `getHighResolutionTicksPerSecond`), `callbacks()`, `periodSamples()`. Cost: one commpage clock read + 3 relaxed atomics
  (~30 ns of a 100 us budget); no allocation / lock / syscall / exception. Outside the TEST_SERVER build: no code.
- `AudioCallback.cpp:44`: `if (ringBuffer_.push(dest, n) < n) overruns_.fetch_add(1, relaxed);` (`std::atomic<uint64_t>
  overruns_` in `AudioCallback.h`, same `#if`); accessor `ringOverruns()`.
- `AudioEngine.h`: `#if AUDIODNA_TEST_SERVER` pass-throughs: `takeAudioGapMaxMs()`, `audioCallbacks()`,
  `audioPeriodSamples()`, `ringOverruns()`.
- `tests/test_audio_tap_sync.cpp` (links `CombinedCallback.h` without the define): unchanged, still compiles.

### 5.8 ctests (RED first; registered at the EOF of `tests/CMakeLists.txt` in the named shapes)
1. `tests/test_load_ticket.cpp` [asyncload][ticket] (pure shape :2747-2750 + juce_core): (a) finish(Done) from a
   `std::thread` -> wait(2000) == Done; (b) a second finish(Failed) returns false, outcome stays Done; (c) wait(10) on an
   unfinished ticket == Pending. RED: by absence (no header).
2. `tests/test_staged_load.cpp` [asyncload][ledger][label][adopted] (pure): Ledger (a) begin(3), land x3 -> complete only at
   the third, pending 3 -> 0; (b) begin(2) then begin(1): a gen-1 landing is Stale and does not count; gen 2 completes after
   one; (c) begin(0) complete at once; (d) cancel -> Stale, pending 0, liveGen 0; (e) batches() counts. Labels (f)
   `loadingLabel(DeckDuplicate, "A copy") == "Loading A copy..."` and `doneLabel` per Kind matches today's three texts
   (:3134, :3287, :3456); (g) `labelAfterCancel("Loading X...", "Loading X...", "Loaded: W") == "Loaded: W"` and
   `labelAfterCancel("clip.mp4", "Loading X...", "Loaded: W") == "clip.mp4"`. Adopted (h) add(video id), add(sequence id)
   -> takeAll() returns both, then empty. RED: by absence. Teeth: `land` ignores gen -> (b) FAILs; `labelAfterCancel`
   returns `before` unconditionally -> (g) second case FAILs; `takeAll` returns only the first id -> (h) FAILs.
3. `tests/test_media_opener.cpp` [asyncload][opener] (the `test_video_player_open` shape :2757-2770: VideoPlayer.cpp + FFmpeg +
   juce_events, no GL, the manual poster injected): (a) begin(2 jobs on `tests/fixtures/video_h264_64x64.mp4`), drain the
   poster -> 2 landings with non-null 64x64 players, onDone once, pending 2 -> 0, batches 1; (b) one job on a non-existent
   path -> a null landing, failed() 1, onDone once; (c) begin(2), cancel() before draining -> both landings Stale (stale()
   2), onDone never, pending 0; (d) begin(0) -> onDone inside begin. RED: by absence. Teeth: `landed` skips the Stale check
   -> (c) FAILs.
4. `tests/test_hot_thread_io_lint.cpp`: unchanged (no new `loadFrom` / `existsAsFile` in the linted files; `presence::seed`
   already lives in MainComponent).
Expected ctest count: 920 + 3 targets' cases (Harmony records the number at merge).

### 5.9 Live probe: `.harmony/probe-async-load.sh` + `.py` + `.json` (NEW; commit 1)
`.sh` = `probe-media-open.sh` verbatim with `ASYNCLOAD_APP / _PY / _ENV`, out dir `asyncload.XXXXXX`, and after the .py: the
quit-mid-load check (row `end_quit_mid_load` leaves a load in flight; the existing graceful quit must succeed: "app terminated"
within 30 s, and no `~/Library/Logs/DiagnosticReports/Audio-DNA-*.ips` newer than the run's start, and 0
`UserNotificationCenter` windows). `.py` copies probe-media-open.py's helpers (:89 la, :151 make_fixtures, :170-200 clip /
layer / load builders, :223 comp_state, :246 wait_until, :261 wait_active, :266 cap, :293 dbox, :305 wait_no_compiler, :325
Poller, :360 fence_deltas, :368 stall_max) and adds `load_async(tag, ...)` (posts on a `threading.Thread`, records t_post /
t_answer / the JSON), `ui_text()`, `state_load()`. Fixtures: `v4k.mp4`, `v1080.mp4` (the media-open recipes, `-g 250`), 32
APFS clones each (`cp -c`), `ref4k.png` / `ref1080.png`, `base.png` / `top.png` (flat 256x144, distinct), `seqA/`, `seqB/`
(300 frames as m4), `cut_header.mp4` = a copy of `tests/fixtures/video_h264_cut_header.mp4`, `deck16.json` (a `Deck::toVar`
shape: "name", "id", "numColumns", "layers", 16 x v4k cells). Constants (`.json`, each with its `_` note): `stallMaxMs 50`
(R10), `swapMaxS 2.5` (R3), `cancelAnswerMaxS 0.5`, `holdMaxPerCut 3`, `boxTol 3.0`, `lumaMin 20`, `heartbeatPeriodMs 4`,
`audioGapFactor 2.5` (gap max <= factor x `audio_callback_period_ms`; commit 1 records 5 idle readings and the builder RAISES
the factor only with the arithmetic in the report, never silently), `uploadsMinPerS 10`, `pollMs 15`, `triggerDelayS 0.1`,
`cancelDelayS 0.15`, `w16 16`, `w32 32`, canvas 1920x1080, layer ids 81-89.

| row | steps | PASS | RED on main / on the commit-1 app (predicted) |
|---|---|---|---|
| a1_witness_fields | heartbeat on; `state.load` has opens_pending / open_batches / opens_stale / opens_failed / staged / timing{...} / audio_xruns / audio_callbacks / audio_callback_gap_max_ms / audio_callback_period_ms / analysis_ring_overruns; `GET /api/debug/ui_text` answers `file_label`. | all present; audio_xruns >= 0 (the device supports it); period_ms printed. | main: absent / 404 -> FAIL. commit-1 app: PASS (the fields land in c1). |
| a2_load_16x4k | load W (layer 0 col 0 = v1080, triggered, playing); 1 s; quiet; s0 (peaks reset); Poller 15 ms (peak_message_stall_ms, load.audio_*, video_uploads, video_late_frames, `/api/composition` layer 0 clip playhead every 4th poll); `load_async(V16)` (16 x v4k cells, 1 deck, 1 layer, 16 columns); wait for the answer (<= swapMaxS + 1); s1; `/api/composition` at once. | (a) max stall <= 50; (b) the answer is ok:true within swapMaxS AND `/api/composition` read right after it names V16 with 16 clips (the answer came after the cut); (c) fence_black +0, fence_hold delta <= holdMaxPerCut; (d) OLD comp animated: video_uploads delta over the window >= uploadsMinPerS x window_s and the old clip's playhead advanced (window = t_answer - t_post, printed); (e) every V16 clip: thumbnailW > 0, clipWidth 3840 (landings wrote the staged clips); (f) trig(0,0), wait_active, cap -> dbox(cap, ref4k) <= 3 and video_pending_frames +0 across the trigger; (g) audio: audio_xruns delta 0, analysis_ring_overruns delta 0, gap max <= audioGapFactor x period; (h) the old clip's video_late_frames delta <= 2 (the pool did not starve its decode thread); (i) `load.timing` printed (c1: opens_ms / total_ms >= 0.8 is the section-2 gate). | main: fields absent. c1: (a) 1000-1200 (the load runs inside the message; the peak is read after), (b) the answer is immediate and `/api/composition` still names W -> FAIL, (d)(e)(f)(h) PASS (guards: the GL thread renders through a frozen message thread), (g) PASS (guard, R9). |
| a2b_load_16x1080 | as a2 with v1080 x 16. | as a2 ((e) 1920). | c1: (a) 380-450, (b) FAIL. |
| a3_cancel_by_newer_load | load W2 (image comp, 2 columns: base / top; label ends "Loaded: W2"); s0; `load_async(V32)`; at +cancelDelayS `load_async(W3)` (image comp); wait both. | (a) V32's answer is ok:false "superseded by a newer load" within cancelAnswerMaxS of W3's post; (b) W3's answer ok:true within swapMaxS and `/api/composition` names W3; (c) `ui_text` == "Loaded: W3" (never stuck on "Loading V32..."); (d) open_batches +2, opens_stale >= 0 printed; (e) after 1 s settle: video_players == 0 (the cancelled batch's adopted players retired; W3 has none); (f) max stall <= 50 (the cancel never waits on an open). | main / c1: (a) FAIL (both loads answer ok:true at once; sync), (d) FAIL (absent / no batches). |
| a3b_explicit_cancel | load W2; s0; `load_async(V32)`; at +cancelDelayS `POST /api/debug/cancel_load`; wait. | (a) the answer is ok:false "superseded..." within cancelAnswerMaxS; (b) `/api/composition` still names W2 (nothing swapped); (c) `ui_text` == "Loaded: W2" (restored); (d) after settle video_players == 0; (e) fence_black +0. Teeth T5 (restore removed) -> (c) FAIL "Loading V32...". | main: 404 on cancel_load -> FAIL. c1: the load has finished before the cancel arrives -> (a) FAIL (ok:true), (b) FAIL (V32). |
| a3c_cancel_with_sequences | as a3b with V32S = V32 + 2 sequence cells (seqA, seqB at fps 30). | a3b's (a)-(e) and (f) seq_open unchanged after settle (A1: no sequence adopted by a cancelled batch). | as a3b. |
| a4_load_then_trigger | load W2 (layer 0: col 0 base triggered, col 1 top); 1 s; s0; `load_async(V32)`; at +triggerDelayS `POST /api/trigger_clip {0,1}` from the main thread, then `/api/composition` at once; wait for V32's answer; `/api/composition`. | (a) the trigger answered BEFORE the load answered (t_trig_answer < t_load_answer); (b) the composition read right after the trigger names W2 with layer 0 activeClipColumn 1 (the OLD show answered); (c) after the load: name V32 and EVERY layer activeClipColumn -1 (the trigger did not leak into the new composition); (d) fence_black +0; (e) cap right after (c) + trig(0,0) shows v4k (the new show works). | main / c1: (a) FAIL (the load answered at once), (b) FAIL (the message thread is frozen: the trigger has not run; or names V32), (c) FAIL by value (the queued trigger lands on V32: layer 0 activeClipColumn 1). |
| a5_duplicate_deck | load D (deck "A": layer 0 col 0 v4k triggered, cols 1-3 v4k); trig(0,0); 1 s; quiet; s0; `POST /api/debug/duplicate_deck {"deck":0}`; at +0.05 s `ui_text`; poll `/api/composition` until numDecks == 2 (<= swapMaxS); s1; cap. | (a) the +0.05 s ui_text == "Loading A copy..." and the final == "Duplicated deck: A copy" (A5); (b) activeDeck == 1 (the copy is active, as today); (c) video_pending_frames +0 and videos_pending 0 across the window (frame 0 ready: R6) and fence_black +0, hold <= holdMaxPerCut; (d) dbox(cap, ref4k) <= 3 (the copy's active clip draws at once; A7); (e) max stall <= 50; (f) video_players == 8 (4 + 4, the copy's own players under new ids: Pitfall 36). | main: 404. c1: (a) FAIL (the message thread is frozen at +0.05 s: ui_text answers after the duplicate with the final text), (e) 250-300. |
| a6_append_deck | load W2; s0; `POST /api/debug/load_deck {"path": deck16.json}`; poll numDecks == 2 (<= swapMaxS); s1. | (a) max stall <= 50; (b) numDecks 2 within swapMaxS, activeDeck 1, `ui_text` == "Loaded deck: deck16"; (c) 16 clips of deck 1 have clipWidth 3840; (d) fence_black +0; (e) audio as a2 (g). | main: 404. c1: (a) 1000-1200. |
| a7_failure_mid_batch | V16F = V16 with cell 7 = cut_header.mp4 and cell 9's file deleted before the load; `load_async`; wait. | (a) answer ok:true within swapMaxS; (b) 16 clips: cell 7 clipWidth == the file default (not 3840) and thumbnailW 0, cell 9 mediaMissing true, the other 14 clipWidth 3840; (c) opens_failed +1; (d) trig(0,0) -> cap == v4k; (e) no crash dialog. | main / c1: (c) absent -> FAIL; (b) PASS (guard: same outcome synchronously). |
| end_quit_mid_load | the LAST row: `load_async(V32)` and return after 0.1 s (the .sh quits the app while it is staged). | .sh: "app terminated" within 30 s; no new .ips; 0 UserNotificationCenter windows. | main / c1: PASS (guard: a synchronous load just delays the quit). |
| end_hold_witness | after every row. | fence_hold_frames over the run >= 1. | c1: PASS (mediaopen's hold is in main). |

Order: a1, a2, a2b, a3, a3b, a3c, a4, a5, a6, a7, end_hold_witness, end_quit_mid_load. GREEN = every row PASS on 2 consecutive
runs of the final app on a quiet machine; RED lines on main and on the commit-1 app recorded verbatim; teeth T1-T5 lines with
the restore sha. A flake verdict needs >= 5 runs per arm.

Teeth (record the FAIL line, restore, `sleep 1; touch`): T1 `Ledger::land` ignores gen -> ctest 2(b) + a3 (a); T2
`Adopted::takeAll` drops sequence ids -> ctest 2(h); T3 the REST handler answers before the wait (skip `ticket->wait`) -> a2
(b), a4 (a)(c) FAIL live; T4 (DATA, not a gate) a temporary env-switched build with an 8-thread `highest` pool -> a2 5x: report
audio_xruns / gap max / the old clip's late frames beside the shipped pool's 5x (restore by blob hash, `strings | grep -c
AUDIODNA_ASYNCLOAD_` = 0); T5 `labelAfterCancel` returns `current` unconditionally -> a3b (c) FAIL.

### 5.10 What "done" looks like
- ctest serial 100 % with the three new targets; teeth lines recorded.
- probe-async-load GREEN twice; RED runs on main and on the c1 app recorded (a2 (a) ~1000-1200 / (b), a2b (a) ~400, a3 (a)(d),
  a3b (a)(b), a4 (a)(b)(c), a5 (a)(e), a6 (a), a7 (c)); the section-2 attribution numbers from c1 in the report.
- Every existing probe GREEN with NO threshold touched: probe-media-open 38/0, image-load 37/0, crossfade 35/0, seq-vram 66/0,
  deck-clock 10/0, render-state 35/0, canvas 15/0, capture 9/0, fitmode 10/0, effects-parity 46/0, outputs 17/0, idle-paint,
  deck-tabs, deck-path, lane3, routines, routine-display, step3, resync, mastersignal, onset-render (the battery
  `.harmony/.reports/s-rta-0928b/wf/final.sh`), probe-video at its documented state (w1 (a) / w2 (a2) are open items, not
  this lane's); Tier-1 unchanged.
- Report tables: per-row numbers (stall max, swap wall, cancel answer time, hold / black deltas, audio fields, late frames,
  timing split), the T4 data table, quit time with a load in flight.

## 6. MUST NOT CHANGE (and how each is kept)
- `VideoPlayer.*`, `ImageSequence.*`, `SeqVram.h`, `VideoRing.h`: not edited (R16); the contract used is quoted in section 1.
- `Renderer::installVideoPlayer` / `openVideoForClip` / `closeMediaForClip` / `drainRetiredMedia` / the fence: not edited.
- `/api/load_composition` request shape, success body `{"ok":true}`, the two `jsonFail` refusals, the "answer after the model
  changed" ordering every caller relies on (R1): kept by the ticket; a2 (b) / a4 (a) prove the order.
- Image-only / sequence-only / source-only compositions and deck files swap on the same message as today (an empty batch
  completes inside `begin()`): probe-image-load / seq-vram / crossfade / deck-clock / render-state timings unchanged.
- Undo: a Composition cut clears history; Append / Duplicate are one `InsertDeckCmd` each (pushed at completion); the
  commands' bodies untouched.
- Routines: stopped at a Composition cut only (:2938); Append / Duplicate never stop them.
- Presence semantics (P5), thumbnails (`ClipThumbnails`), Pitfall 55's fence hold, Pitfall 56's pick / hold / FAILED rules.
- The audio callback in a non-TEST_SERVER build: byte-identical logic (the counters compile out).
- Drops (`prepareFileDrop`), the trigger's lazy open, the reconnect hooks: synchronous as today (out of scope, named).
- Every existing probe's thresholds and the mediaopen probe's rows.

## 7. RISKS (strongest counterargument first)
1. "Blocking the REST handler is a regression: a caller that spams loads or a stuck message thread ties up httplib workers and
   the `/api/state` polls behind them." -- At most ONE waiter is ever blocked: a newer load releases the older at once
   (Superseded), the pool has >= 8 workers, the wait is bounded (60 s), and `stop()` releases every ticket before joining.
   a3 proves two overlapping loads both answer within 2.5 s while the Poller keeps reading state. A stuck message thread
   already breaks every marshalled endpoint today.
2. "A trigger during the window on the OLD composition surprises a two-connection automation." -- None exists (section 1);
   the single-connection pattern is identical to today (R1); a4 documents and proves the defined outcome. If Harmony
   prefers "replay onto the staged model", it is a different product rule (Q2), not a bug.
3. "Low-priority openers on E cores make the cut land LATER than today's 1.1 s." -- Possible up to ~1.1-1.2 s (R3); the
   UI is live and the show plays meanwhile, the bar is 2.5 s, and commit 1 / a2 print `open_max_ms` and the window so
   Harmony can move to `normal` with data (one constant).
4. "The completion message (16 landings + the swap + `rebuildGrid` with 16 4K thumbnails) exceeds 50 ms." -- Predicted ~36
   ms (R10); if measured over 50 on a quiet machine that is a real finding (the grid), reported, never re-thresholded.
5. "The 16 staged players (each with a parked decode thread + 2 codec threads and 3 page-lazy 4K slots) exist during the
   window beside the old composition's." -- Exactly today's footprint (today opens all 16 before the swap too); frame 0
   touches one slot each (33 MB x 16 = 0.5 GB, as today). The pool adds 2 threads.
6. "`removeAllJobs(false, 0)` might still wait for active jobs (a cancel stalls the message thread by one open)." -- Verified
   at step 0 before commit 3 (5.0 (a)); a3 (f) gates it (stall <= 50 during a cancel with an open in flight). Fallback if
   JUCE waits: cancel() only bumps the ledger (queued jobs then open uselessly and land Stale -- ~0.5 s of wasted pool time,
   no stall) -- say which in the report.
7. "A stale landing destroys a 4K player on the message thread (3 x 33 MB `free`, `avcodec_free_context` joining 2 codec
   threads)." -- < 1 ms per stale landing (munmap + a join of idle threads); a3 (f) covers it.
8. "A3 cannot RED by value." -- Stated (R9): a guard with absence-RED and the T4 data arm; the OS overload count is the
   right witness precisely because nothing this lane does can be seen by a callback-duration timer.
9. "Two Duplicate clicks in one window produce one copy" (R4, Q3) -- documented; a queue is a follow-up if Boris wants it.
10. "`presence::seed` still stats 16-32 files on the message thread at staging." -- ~20 us each on APFS (mediaopen measured
    the whole seed inside the 18-20 ms m4 load); commit 1's `prep_ms` shows it; moving it into the jobs is a one-line
    follow-up if it ever matters.
11. Verify before building (INFERRED items): 5.0 (a)(b); `std::function` copyability of the landing lambda (the shared box);
    that `getXRunCount()` reports >= 0 on the rig's default device (a1 prints it; -1 = unsupported -> the row reports it and
    the gap counter carries A3).

## 8. COMMIT SEQUENCE (each builds, passes ctest, keeps every existing probe GREEN; each reverts alone)
1. `perf(s-rta-0929 asyncload): TEST-ONLY load timing scopes, audio-callback witnesses, /api/state load{}, debug ui_text /
   load_deck / duplicate_deck / cancel_load, probe-async-load (RED harness)` -- 5.6, 5.7, the state object, the TEST-ONLY
   routes (cancel_load wired to a no-op until commit 4: answers ok, does nothing -- documented in the row's RED), 5.9. No
   behaviour change. GREEN: ctest unchanged; a1 PASS; the section-2 attribution gate PASSES (else STOP and report).
2. `feat(s-rta-0929 asyncload): LoadTicket + StagedLoad (ledger, labels, adopted ids) + ctests` -- 5.1, 5.2, tests 1-2
   (RED by absence first). No behaviour change.
3. `feat(s-rta-0929 asyncload): MediaOpener (2 x low pool, WeakReference landings, non-blocking cancel) + ctest` -- 5.3,
   test 3, CMakeLists (+1 .cpp). No caller yet.
4. `feat(s-rta-0929 asyncload): staged composition / deck loads; REST load answers after the swap; cancel retires adopted
   ids and restores the label` -- 5.4, 5.5 (the wait, the registry, `stop()` release, the callbacks). GREEN: a2-a7 + ends;
   the battery. Teeth T1-T5 recorded.
5. `docs(s-rta-0929 asyncload): rendering.md, integration.md, pitfall NN, CLAUDE.md index (paid), testing-eyes (no change)`
   -- section 9.
6. `docs(s-rta-0929 asyncload): report .harmony/.reports/s-rta-0929/asyncload.md` (the house report shape; notebook lines;
   Boris checks; the T4 table; the attribution numbers).

## 9. DOCS (text; pitfall as "NN"; Harmony edits HANDOFF / APP-INVENTORY, not the lane)
- `docs/claude/rendering.md:73`: replace the sentence "A composition / deck load still opens its videos on the message thread
  before the swap (the asynchronous load is a follow-up lane)." with: "A composition / deck load is STAGED (s-rta-0929
  asyncload, Pitfall NN): its video players open on `MediaOpener`'s 2-thread low-priority pool (`VideoPlayer::open` on an
  unpublished player), each landing writes the STAGED clip and installs the player through `Renderer::installVideoPlayer`
  (its thread parks until the first draw), and the last landing runs today's swap / `InsertDeckCmd` / label; the old
  composition keeps playing until the cut."
- New paragraph after :75: "**Asynchronous loads (s-rta-0929 asyncload, Pitfall NN)**: `loadComposition`, `appendDeckFromFile`
  and `duplicateDeck` stage a private model (parse, validate, re-mint, reconcile, presence seed on the message thread: ~20
  ms), issue one `MediaOpener` job per present video clip and return; landings (message thread) write dims / alpha /
  thumbnail into the staged clip only and install the player under its re-minted id; sequences open (no I/O) at completion,
  just before the swap. One staged load at a time: a newer load, `swapCompositionModel` (New Composition), `POST
  /api/debug/cancel_load` or destruction cancels it -- every adopted id is retired through `closeMediaForClip`, queued
  opens are dropped, in-flight opens land Stale and die unpublished, the file label is restored unless something else wrote
  it, and the load's `LoadTicket` finishes Superseded / Cancelled. A trigger, switch or edit that arrives while a load is
  staged acts on the LIVE composition (the show on screen), never on the staged one. `POST /api/load_composition` answers
  only after the staged swap (`{"ok":true}` as before; `{"ok":false,"reason":"superseded by a newer load" | "cancelled" |
  "timed out"}` are the only new answers), so 'load then act' on one connection behaves as it always did; a 16 x 4K load
  keeps the message thread under 50 ms (was ~1.1 s) and cuts within ~0.6-1.2 s. `/api/state load {opens_pending,
  open_batches, opens_stale, opens_failed, staged, timing{parse_ms, prep_ms, opens_ms, open_count, open_max_ms, seq_ms,
  swap_ms, ui_ms, total_ms}}` (both servers); TEST-ONLY: `load.audio_xruns` (CoreAudio's processor-overload count),
  `audio_callbacks`, `audio_callback_gap_max_ms` (reset on read), `audio_callback_period_ms`, `analysis_ring_overruns`;
  `GET /api/debug/ui_text`, `POST /api/debug/load_deck {path}`, `/api/debug/duplicate_deck {deck}`, `/api/debug/cancel_load`.
  Guards: `tests/test_load_ticket.cpp`, `test_staged_load.cpp`, `test_media_opener.cpp`; live `.harmony/probe-async-load.sh`."
- `docs/claude/integration.md:9`: after "/api/load_composition" add "(answers after the staged swap completes -- s-rta-0929
  asyncload; see rendering.md 'Asynchronous loads')".
- `docs/claude/pitfalls.md` (append): "NN. **A load is staged, never opened on the message thread -- and a message that
  arrives while it is staged belongs to the LIVE composition**: every video `open()` of a composition / deck load runs on
  `MediaOpener`'s pool against an unpublished player (`VideoPlayer::open` touches only its object); a landing writes the
  STAGED clip only (a live `Clip` write would race the GL thread: juce::Image ref counts) and installs the player under its
  re-minted id (Pitfall 36: never a live id), where its thread parks until the first draw; the completion opens the staged
  sequences (no I/O) BEFORE the swap (a sequence found after the cut would render FX-only); one staged load at a time -- a
  cancel retires EVERY adopted id (video and sequence), restores the label if it still shows this load's, finishes the ticket
  Superseded; `POST /api/load_composition` blocks its HTTP worker on the ticket (bounded) and `ApiServer::stop()` releases
  every ticket before httplib joins (a blocked worker would hang the quit); a REST / OSC / MIDI / key command during the
  window acts on the live composition and is NOT replayed. Never open media inside a load's message, never write a landed
  value into a live clip, never answer a load before its swap, never wait for the message thread from `ApiServer::stop()`.
  Guards: `tests/test_staged_load.cpp`, `test_media_opener.cpp`, `test_load_ticket.cpp`; live `.harmony/probe-async-load.sh`
  (a2 stall / order, a3 / a3b cancel, a4 trigger-in-window, a5 duplicate, end_quit_mid_load)."
- `CLAUDE.md` index line (~130 B): "NN. A load is staged off the message thread; a command during the window acts on the live
  composition -- before touching loadComposition / appendDeckFromFile / duplicateDeck / the load REST handler." PAID by
  trimming the "Routine pads and bands" UI-pattern paragraph (1,172 B) to: "**Routine pads and bands**: a pad's press is
  always Fire; there is no stop control -- a routine leaves by its band's x, the layer X, its own end, the pad menu's
  'Remove from layers' or Stop (routines only); 'Delete routine' (warning red, behind a confirm) is the only path that
  erases one. Pads, bands, strip faders and bound controls are model-driven from `RoutineEngine::Status` every 30 Hz tick
  (never panel memory; Pitfall 41); the routine cue `AudioDNALookAndFeel::kRoutineCue` (chartreuse) is reserved -- never
  the accent cyan; full text `docs/claude/recording.md` 'Surfaces'." (~640 B; saves ~530 B; the moved sentences go verbatim
  into recording.md "Surfaces"). Final `wc -c CLAUDE.md` <= 25,000 (Harmony confirms the trim at merge).
- `docs/claude/testing-eyes.md`: no change (8080 has no load route).

## 10. COMPACT
QUESTION: async composition / deck loads that keep the UI live and the old show playing, cut clean, and change nothing for
existing callers (A1-A8). APPROACH: stage the model as today, open videos on a 2 x low `MediaOpener` pool against
unpublished players, land into the STAGED clips + `Renderer::installVideoPlayer` (parked thread, frame 0 ready), complete on
the last landing with today's swap / InsertDeckCmd / label; sequences open at completion; newest load wins; cancel retires
every adopted id, restores the label, finishes a waitable ticket; REST load answers after the swap; a command during the
window acts on the live show. ATTRIBUTION: >= 95 % of the 1.1 s is 16 x `VideoPlayer::open` (~68 ms each at 4K); commit 1
measures it before anything moves (gate: opens >= 80 %). BARS: stall <= 50 ms (RED 1000-1200 / 380-450), cut <= 2.5 s (16 x
68 / 2, x2 E-core), audio xruns / overruns +0 and gap <= 2.5 x period (GUARD), late frames <= 2, black +0, hold <= 3. ROWS a1-a7
+ two ends; teeth T1-T5. NOT EDITED: VideoPlayer / ImageSequence / Renderer / the fence / any threshold. OPEN FOR HARMONY:
Q1 pool `low` vs `normal` (data in c1 / T4); Q2 trigger-in-window = live show (R2) -- confirm; Q3 newest-wins for
Duplicate / Append (one copy per window); Q4 sequence frame-0 pre-warm filed (needs seqvram GL machinery); Q5 the CLAUDE.md
payment text (the routine paragraph trim) -- confirm or name another 130 B.

REPORT_FILE: .harmony/.reports/s-rta-0929/plan-asyncload.md
STATUS: DONE

## HARMONY ADOPTION (s-rta-0929, 09:15) — OVERRIDES THE BODY WHERE THEY DIFFER
Plan authored by Fable (wf_d57eaf2e-b47). Attacked blind by attack-asyncload-threads.md, -vj.md, -gates.md. The body is
ADOPTED except where a ruling below overrides it.
- AL1 (threads MUST 1, ADOPT as a correction) R3's arithmetic is wrong: `VideoPlayer.cpp:108` gives each open 2 FFmpeg frame
  threads, so 2 pool threads = up to ~6 runnable threads during a batch. VideoPlayer stays fenced (no thread_count change).
  The verdict is the OLD show's rows, not the arithmetic: run a2 5x on the final app; if a2 (h) (old clip late frames) or
  the hold delta fails in >= 1 of 5, re-run a2 5x with a 1-thread pool and report both tables; Harmony picks at the gate.
  Pool starts at 2 threads, Priority::low (Q1: data decides, T4 is the data arm). Correct R3's text in the report.
- AL2 (threads MUST 2, ADOPT) Never force-kill a thread inside FFmpeg. `~MediaOpener`: if `removeAllJobs(true, 5000)` times
  out (a job still inside open()), INTENTIONALLY LEAK the pool (hold it by unique_ptr and release() it; log one line) —
  never let ThreadPool's destructor stopThread/kill a thread blocked in FFmpeg (a crash dialog on Boris's screen). The
  job holds only its own unpublished player + a WeakReference; state that it never touches stats_ / the renderer (a
  comment at the lambda). NEW row end_quit_hung_open (before end_quit_mid_load; it ends the run): a video cell whose file is
  a FIFO (`mkfifo`, no writer: open(2) blocks forever) + 1 real v4k cell; load_async; after 0.5 s the .sh quits the app.
  PASS: "app terminated" within 30 s, no new .ips, 0 UserNotificationCenter windows; the .sh deletes the FIFO after. RED
  (teeth T6): the pool's destructor path restored (no leak) -> record what happens (a crash / a hang past 30 s) or, if it
  still passes, say so honestly (then it is a GUARD). AVIOInterruptCB cancellation of a hung open = filed (VideoPlayer is
  fenced), not this lane. While a FIFO open hangs, the rest of the app must keep working: a3-style second load of an image
  comp after it answers ok (the hung job occupies one pool thread; the batch it belongs to is Superseded).
- AL3 (gates MUST 1, ADOPT) ctest 3(c) is non-deterministic (`removeAllJobs` deletes QUEUED jobs outright, verified at
  juce_ThreadPool.cpp:290-309). Make both cases deterministic: (c1) an injected start-latch so BOTH jobs are running
  before cancel() -> stale()==2; (c2) a hook that holds the workers so both jobs are still QUEUED at cancel() -> a new
  `dropped()` counter == 2 and no landing ever arrives. The live rows print opens_stale + opens_dropped.
- AL4 (gates MUST 2, ADOPT) VideoPlayer.h:38-40's contract comment is amended TEXT-ONLY (allowed exception to R16: comment
  lines only, no code): "Call once per player, before start(): from the message thread, or from MediaOpener's pool thread
  on an unpublished player (nothing else may hold it)". Pitfall 56 text gets the same clause (docs).
- AL5 (vj MUST 1, ADOPT) While a load is staged, the file label keeps "Loading <name>..." — refreshPreviewFromActiveClip (and
  any other label writer) writes the would-be text into the staged load's restore slot instead of the label. Completion
  writes today's final text; cancel restores the LATEST would-be text (so a trigger during the window shows its text after
  a cancel). R13 amended accordingly; ctest on the pure function; row a4 adds (f): ui_text right after the trigger still
  reads "Loading V32..." and, in a3b-style cancel after a trigger, the trigger's text.
- AL6 (vj MUST 2 + SHOULD 3, ADOPT) Keep `~MainComponent`'s documented order: the HTTP servers stop FIRST (ApiServer::stop()
  finishes every ticket Cancelled before httplib joins — 5.5), THEN cancelStagedOpen. State in a comment that the server's
  sweep wins at quit and cancelStagedOpen's finish is then a no-op (compare-exchange).
- AL7 (vj SHOULD 4 + Q3, OVERRIDE R4) The END STATE of any sequence of requests equals today's (where a frozen UI
  processed them in order): a Composition load supersedes everything staged or queued before it (identical end state —
  the last composition wins — minus the intermediate flash); an Append / Duplicate that arrives while any load is staged
  is QUEUED (FIFO, bounded 8; the 9th is refused with a label note) and PREPARED when dequeued against the then-live
  composition (a Duplicate names its source deck by id; a source deck that no longer exists at dequeue -> skipped with a
  label note, ctested). Two Duplicate clicks within a window = two copies, in order. New row a5b_duplicate_twice (two
  duplicate_deck posts 0.05 s apart -> numDecks 3, copies named "A copy" and "A copy 2" or whatever today's naming gives —
  derive it from source, do not invent) with RED on main by the 404 and on c1 by value if any; ctests for the queue.
- AL8 (threads SHOULDs, ADOPT) (a) a ctest compares a thumbnail made on a pool thread with one made on the message thread
  (same player file, identical pixels); (b) static_assert(std::atomic<T>::is_always_lock_free) for every counter the audio
  callback touches; (c) removeAllJobs(false, 0) semantics are now VERIFIED by the gates seat (queued deleted, active keep
  running) — cite it in the code comment. NIT: `/api/state` gains `staged_players` (adopted, not yet live) so
  video_players is read correctly during a window.
- AL9 (gates SHOULDs, ADOPT) (a) the callback counter line in AudioCallback.cpp sits inside `#if AUDIODNA_TEST_SERVER`; the
  lane builds the AudioDNA target ONCE with -DAUDIODNA_BUILD_TEST_SERVER=OFF in a scratch build dir (compiles; `strings`
  count of the counter names == 0) and deletes that dir; (b) correct R9's "a duration timer measures only our memcpy"
  text (memory-bandwidth contention can move it; the OS overload count is chosen because it is the OS's own verdict and
  costs nothing); (c) a4 asserts its precondition t_load_answer - t_post > triggerDelayS before (a)-(c) (else the row
  FAILs "window too short to test"); NIT: one doc line that open() now logs to stderr from pool threads.
- AL10 (Q2, CONFIRM R2) A trigger during the window acts on the live show and is not replayed — "keep playing" (Boris).
  (Q4) sequence frame-0 pre-warm: filed. (Q5) CLAUDE.md: the payment MOVES the routine-pads text VERBATIM into
  docs/claude/recording.md "Surfaces" (no deletion; the report shows the no-loss check: every moved sentence present in
  its new home) — never shorten a Boris ruling's wording.
- AL11 (vj NIT 5, ADOPT as a comment) Member order (mediaOpener_ declared after previewPanel_ / renderer users so it is
  destroyed first) is load-bearing: say so in a comment at the declaration.
- AL12 Row set = the body's + a5b + end_quit_hung_open + a4 (f); GREEN = every row PASS on 2 consecutive runs on a quiet
  machine; a2 x5 for AL1. Existing probes to re-run on the final app (never re-threshold): probe-media-open, probe-video,
  probe-seq-vram, probe-image-load, probe-crossfade, probe-deck-tabs, probe-routines (ROUTINES_BUILD_DIR, pause 1.8),
  probe-idle-paint, probe-capture; Tier-1 test_sources.py test_effects.py test_audio_reactivity.py test_time_sweep.py
  test_performance.py. Another lane (diag-vfps, diagnosis only) and later build lanes share the live lock.
