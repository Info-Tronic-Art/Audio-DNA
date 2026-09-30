# Verify F21 - TSan race CompositorEngine.cpp:991 (Layer crossfade fields) vs Layer.h:250 (triggerClipImmediate via handleColumnTrigger)
VERDICT: REAL_BUG (genuine unsynchronised data race, formally UB), severity LOW for a live show. Same family as F4/F16 (family B). Not a tool artefact.

## Report read - VERIFIED (runs/tsan-3/tsan.49175 l.2020-2100)
- Write 4B, main thread: Layer::triggerClip -> inlined triggerClipImmediate (Layer.h:250 is the call line; body Layer.h:260-278) <-
  MainComponent::handleColumnTrigger MainComponent.cpp:4820 (deck->triggerColumn) <- callAsync from ApiServer::handleTriggerColumn.
- Prev read 4B, T47 "OpenGL Renderer" (mutexes M0 RenderThread, M1 CGL ctx): renderLayerStages CompositorEngine.cpp:991-992
  crossfadeStart_[..].observe(layer.previousClipColumn, layer.activeClipColumn, layer.crossfadeProgress) <- compositeDeck :1135 <- Renderer.cpp:728.
- Block 3200 B = std::vector<Layer> storage (8 x ~400 B; hit offset 3180 = layer 7), allocated Deck::fromVar Deck.h:187. No free/realloc in either stack.
- Exact field is one of the three at :991-992 (all 4B); the inlined write covers all three (Layer.h:276-278). Which one: INFERRED, not pinned by offsetof.

## Refutation attempts (all failed)
- Common lock: M0/M1 held by render thread only; writer holds none. VERIFIED (report "mutexes:" line).
- Happens-before via message post: none; render thread is free-running, callAsync only orders HTTP thread -> message thread. VERIFIED.
- Atomics: activeClipColumn/previousClipColumn plain int, crossfadeProgress plain float (model/Layer.h:181-183). VERIFIED.
- Deck fence: the column-trigger path (MainComponent.cpp:4781-4820) is not inside undoService_.withDeckDetached (used at :818, 906, ... for
  structural edits only). VERIFIED (read the trigger path; grep of fence users).
- Stats counter / third-party: no; app code both sides, fields drive rendering. VERIFIED.
- Acknowledged design: render/CrossfadeHistory.h:24-31 says Layer fields are "read on the GL thread without atomics" and the detector is written
  to tolerate the intermediate write orderings. That is a known tolerance, not synchronisation. VERIFIED.

## Live-show impact (INFERRED from source; not reproduced)
- Crash: none expected. Aligned 4B int/float do not tear on arm64; no resize/free in the stack; a bad activeClipColumn is range-checked in
  getActiveClip (Layer.h:195-198). Composition-swap overlap is NOT shown by F21.
- Visual: render can see a mixed (prev, active, progress) triple (e.g. new active with old progress) for one frame -> one-frame wrong blend, or
  observe() misfires/misses handOverClipHistory once (a one-frame temporal-effect history glitch on that layer). Render also WRITES progress/
  previous (LayerClock.h:24-26): a trigger landing inside that read-modify-write can be lost (hard cut instead of fade). Self-corrects next frame or next trigger.
- Likelihood: needs a trigger inside a ~us window of a 16.7 ms frame; low. Hit 1/6 only because the sweep fired triggers at REST-call rate.
  Reachable also by MIDI/keyboard/OSC column triggers (same handleColumnTrigger). ASSUMED for MIDI/OSC (not read).

## Smallest fix
Trigger posts one std::atomic<uint64_t> pending-trigger word (column + serial) per Layer; render thread consumes it at frame start (before
advanceCrossfade, CompositorEngine.cpp:1073-1075), so the render thread is the sole writer of Layer runtime state. Cheaper stopgap: make
previousClipColumn/activeClipColumn/crossfadeProgress relaxed std::atomic (needs a copy shim: Layer lives in std::vector) - fixes UB, not the mixed-triple frame.
Not show-blocking. Dedupe F21 with F4/F16 (same writer body, family B).
