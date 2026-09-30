# Verify F16 - TSan race CompositorEngine.cpp:401 (incomingImagePending) vs TriggerCommands.h:68 (TriggerClipCmd::execute)
VERDICT: REAL_BUG (genuine data race, technically UB), severity LOW for a live show. Same race as F4, different call path. Not a tool artefact.

## Report read (tsan.49175 and tsan.50561, identical in shape) - VERIFIED
- Read 4B by T47 (JUCE "OpenGL Renderer", holds M0 = RenderThread mutex, M1 = CGL ctx mutex): incomingImagePending
  CompositorEngine.cpp:401 <- compositeDeck :1074 <- Renderer.cpp:728.
- Write 4B by main thread: TriggerClipCmd::execute TriggerCommands.h:68 (inlines apply -> applyLayerRuntime, core/DeckCommands.h:212-216)
  <- UndoManager::perform UndoManager.cpp:15 <- MainComponent::pushCommands (:5103-5121) <- handleClipTrigger :4744
  <- ApiServer::handleTriggerClip callAsync. (F4 = the same write inside a CompositeCommand, the column-trigger path.)
- Block: 3200 B = std::vector<Layer> storage of one Deck (Layer ~400 B; offset 0x630 -> layer 3), from Deck::fromVar Deck.h:187. No free/realloc in either stack.

## Refutation attempts (all failed)
- Common lock: M0/M1 held by the render thread only; the message-thread writer holds none. VERIFIED (report "mutexes:" line).
- Happens-before via message post: none. The render thread is free-running; callAsync only orders the writer after the HTTP thread. VERIFIED (stacks).
- Atomics: Layer::previousClipColumn is plain int, crossfadeProgress plain float (model/Layer.h:182-183). VERIFIED. core/UndoManager.cpp:3-5 states
  "the GL render thread reads the model lock-free" as a design assumption, not a synchronization.
- Third-party / stats counter: no; app code both sides, and the fields drive rendering. VERIFIED.
- Field: :401 reads crossfadeProgress and previousClipColumn; the writer stores both (DeckCommands.h:213-214). VERIFIED.

## Mechanism and impact (INFERRED from source; not reproduced)
- The render thread also WRITES both fields (LayerClock.h:24-26 advanceCrossfade: progress += step; previousClipColumn = -1 at fade end; called
  CompositorEngine.cpp:1073-1074). Two writers: a trigger landing inside that read-modify-write can lose an update: (a) fade starts partway or
  snaps (1-frame pop), (b) fade-end -1 clobbers the trigger's previousClipColumn -> hard cut instead of a fade, (c) one frame sees progress and
  previous from different triggers. Guards (CompositorEngine.cpp:401, 1646-1647) return the new clip texture: no black frame, no stall.
- Crash: none expected. Aligned 4-byte int/float do not tear on arm64; previousClipColumn is -1 or a valid column and getClipAt(null) is handled
  (:1650-1653). No vector resize in these stacks; a composition swap overlapping a render read is NOT shown by F16. INFERRED.
- Likelihood: needs a trigger inside a ns-us window of a 16.7 ms frame while a fade runs; likeliest when mashing triggers on one layer mid-fade.
  Hit 2/6 only because the sweep fired triggers at REST rate. Live show: rare one-frame glitch or a cut instead of a fade.
- Also reachable from MIDI/keyboard/OSC triggers and Undo/Redo (same apply()). INFERRED (all route through pushCommands).

## Smallest fix
Make Layer::previousClipColumn and crossfadeProgress std::atomic<> (relaxed, with a copy shim since Layer lives in std::vector) and clear
previousClipColumn at fade end via compare_exchange only while progress is still >= 1. Better, one fix for all of family B: the trigger posts one
atomic pending-trigger word that the render thread consumes at frame start, so the render thread is the single writer of Layer runtime state.
Not show-blocking; schedule with family B. Dedupe F16 into F4 (same fields, same writer body).
