# PLAN lane tsan-r5 (s-rta-1002b) -- R5 config scalars + R7 unfenced httplib reader, race-free by design, TSan-gated

Architect (Fable), 2026-10-02, main HEAD 5e47d17 (code = 5b507b1). INTERIM DRAFT -- sections (3)-(9) being written.

## (1) GOAL
Make every model field another thread reads either a `Relaxed<T>` (config scalars, R5) or structure that only the
message thread (under the GL fence) mutates, and make every httplib route read / write the live model only on the
message thread (R7) -- zero behaviour change, no render-path mutex, no render wait -- gated RED/GREEN by ThreadSanitizer
(unit [tsan] targets + app scenarios e / f / g).

## (2) ESTABLISHED FACTS (interim notes; being rewritten)
- Threads that touch the LIVE model: message thread (owner), GL render thread, httplib workers (ApiServer 7070,
  TestServer 8080). VERIFIED no live-model access on: OSC receiver (MessageLoopCallback, OscHandler.h:29-30), MIDI
  input (callAsync, MidiHandler.cpp:70-100), Link (atomics, LinkSync.h:14-15), MediaOpen pool (unpublished players,
  MediaOpener.cpp:52), presence pool (path strings only, MediaPresence.h job), analysis thread (no model include).
- Unfenced structural writes FOUND: kCompCollectMedia relink `clip.mediaFile = dest` (MainComponent.cpp:6542),
  kCompRelocateFiles `clip.mediaFile = candidate` (:6596, :6606) vs GL getKeyTexture path copy
  (CompositorEngine.cpp:274); TestServer add/remove_global_effect run on the GL thread (TestServer.cpp:1385-1389,
  :1438-1446) vs ConnectionEngine::tick iterating globalEffects on the message thread (ConnectionEngine.cpp:337).
- /api/composition iterates the live model on the httplib thread (ApiServer.cpp:385-491); /api/state reads
  decks.size() (:1553) and the legacy EffectChain fields (:1528-1550); TestServer twins + composition_params /
  global_effects / set_composition_params / set_clip_opacity / set_global_effect_bypass read or write on httplib.
- House patterns available: LoadTicket bounded wait released by ApiServer::stop() (LoadTicket.h, ApiServer.cpp:109-119);
  handleDebugUiText Box + WaitableEvent 2 s (ApiServer.cpp:2013-2042).

STATUS: IN PROGRESS
