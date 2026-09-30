# Verify F9 (sweep s-rta-0929b) - Layer active-column race, CompositorEngine.cpp:1072 vs Layer.h:250 (<- MainComponent.cpp:4614)
VERDICT: REAL_BUG (true C++ data race, UB by the letter; same family as F5/F13) - severity LOW - owner: app. Not a tool artefact.

## Report read (VERIFIED, runs/tsan-3/tsan.49175 lines 237-359)
- Write 4 B, main thread: Layer::triggerClip Layer.h:250 (= inlined triggerClipImmediate: previousClipColumn / activeClipColumn / crossfadeProgress, Layer.h:276-278)
  <- MainComponent::handleClipTrigger MainComponent.cpp:4614 (layer->triggerClip) <- ApiServer::handleTriggerClip via callAsync.
- Prior read 4 B, T47 "OpenGL Renderer" (mutexes M0 JUCE render mutex + M1 CGL ctx only): CompositorEngine::compositeDeck :1072 (`layer.getActiveClip()` reads activeClipColumn, Layer.h:195-198)
  <- Renderer::renderOpenGL :728 <- juce_OpenGLContext.cpp:414. Message thread holds no mutex. Heap block 3200 B = live vector<Layer> storage (Deck::fromVar Deck.h:187); no free in either stack.

## Refutation attempts (all failed)
1. Atomic / lock? `int activeClipColumn` Layer.h:181, plain int; no lock in triggerClip or getActiveClip. VERIFIED.
2. JUCE MessageManager::Lock as hidden sync? renderOpenGL runs at juce_OpenGLContext.cpp:414 outside any mm lock (only taken when component painting is on; Renderer disables it - see F5 paper). Mutex sets in the report agree. VERIFIED.
3. Fence (withDeckDetached) covers load/drop, not a column trigger. INFERRED from F5 paper + no fence call on the 4614 path (handleClipTrigger 4585-4620 read; none).
4. callAsync happens-before orders API thread -> message thread only, not message -> GL. INFERRED.
5. Benign stats counter / third-party lib: no, live control state in app code. VERIFIED.
6. Not a one-writer field: the render thread ALSO writes crossfadeProgress and previousClipColumn (LayerClock::advanceCrossfade, render/LayerClock.h:24-26, called at CompositorEngine.cpp:1075) -> write/write conflict with Layer.h:277-278. VERIFIED.

## What it can do live
- Crash / heap corruption: none. activeClipColumn is only ever set to a validated column (Layer.h:255 guard) and getActiveClip re-checks bounds + has_value (Layer.h:195-198); the clips vector is not resized by a trigger. INFERRED that a re-load of activeClipColumn between check and index cannot go OOB for that reason.
  Not covered here: a Clip deleted/replaced while the render holds `const Clip*` (different finding class).
- Torn value: none (aligned 4 B, single-copy atomic on arm64). INFERRED.
- Real effect = one-frame inconsistency + a lost update:
  (a) render reads activeClipColumn at :1072, Renderer.cpp:593, and :991-992 in the same frame; a trigger between them mixes old clip with new column/progress -> CrossfadeStart::observe (:991) sees an odd (prev, active, progress) triple; worst = one frame of wrong picture or one skipped/duplicate history hand-over. Self-heals next frame.
  (b) lost update: render's read-modify-write `crossfadeProgress = min(p+step,1)` (LayerClock.h:24) can overwrite the trigger's `crossfadeProgress = 0` (Layer.h:278) -> the new crossfade starts mid-way or is skipped (a hard cut) and previousClipColumn is set -1 (:26). Needs a trigger to land in the ~tens-of-ns window inside advanceCrossfade, i.e. ~1e-5 per trigger per layer. INFERRED estimate.
- Likelihood: (a) visible as a 1-frame flicker on a few % of triggers at most; (b) practically never. Live-show impact: nothing to a cosmetic 1-frame glitch. It showed once in 6 TSan runs only because the fixture hammers triggers.

## Smallest fix
- Make the three fields (activeClipColumn, previousClipColumn, crossfadeProgress) relaxed-atomic wrappers (copyable struct around std::atomic; Layer is copied into vector<Layer>). ~20 lines, removes UB/TSan noise, NOT the multi-field consistency.
- Correct fix: the message thread posts the trigger to a GL-thread mailbox drained at frame start (autopilot already triggers from the GL thread, MainComponent.cpp:4585 comment), so all writers are one thread. Sacred Rule 2: this UI->render channel skipped std::atomic. Not worth more than LOW.
