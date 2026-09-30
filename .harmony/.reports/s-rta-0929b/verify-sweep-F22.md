# Verify F22 (sweep s-rta-0929b) - CompositorEngine.cpp:992 (crossfadeStart_ observe) vs Layer.h:250 (<- MainComponent.cpp:4820 handleColumnTrigger)
VERDICT: REAL_BUG (true C++ data race, UB by the letter; same family as F5/F9/F10/F12/F13) - severity LOW - owner: app. Not a tool artefact.

## Report read (VERIFIED, runs/tsan-3/tsan.49175 lines 2146-2270)
- Write 4 B, main thread: Layer::triggerClip Layer.h:250 (= inlined triggerClipImmediate; previousClipColumn/activeClipColumn/crossfadeProgress, Layer.h:276-278)
  <- MainComponent::handleColumnTrigger MainComponent.cpp:4820 (deck->triggerColumn) <- ApiServer::handleTriggerColumn via MessageManager::callAsync.
- Prior read 4 B, T47 "OpenGL Renderer" (mutexes M0 JUCE render mutex + M1 CGL ctx only): CompositorEngine::renderLayerStages :991-992 (crossfadeStart_[clipKey].observe(previousClipColumn, activeClipColumn, crossfadeProgress))
  <- compositeDeck :1135 <- Renderer::renderOpenGL :728 <- juce_OpenGLContext.cpp:414. Message thread holds no mutex. Heap block 3200 B = live vector<Layer> storage (Deck::fromVar Deck.h:187); no free in either stack.
- Same as F12 (identical stacks; F12 = handleClipTrigger 4614 route, F22 = column route 4820). The sweep's "as F12 via trigger_column" is correct.

## Refutation attempts (all failed)
1. Atomic/lock? Layer.h:181-183: plain int/int/float. No lock in triggerClip/triggerColumn. VERIFIED.
2. Hidden JUCE lock / happens-before? Report mutex sets (render M0+M1 only) agree; callAsync orders API thread -> message thread, never message -> GL. INFERRED (as F9 paper).
3. Deliberately tolerated? CrossfadeHistory.h:23-30 says Layer fields are "written on the message thread and read on the GL thread without atomics" and that every write ordering is handled. A design comment, not synchronisation: it argues the logic tolerates a torn triple, not that C++ allows it. VERIFIED (read).
4. Stats counter / third-party? No: live control state, app code. VERIFIED.

## What it can do live
- Crash / heap corruption: none. observe() (CrossfadeHistory.h:47-57) is pure compares on its arguments, no indexing. VERIFIED (read). (A Clip lifetime race on the `clip` const& is a different finding class, not shown here.)
- Torn value: none (aligned 4 B, single-copy atomic on arm64). INFERRED.
- Real effect: one frame sees an inconsistent (prev, active, progress) triple: (a) spurious or missed handOverClipHistory (:993) -> one frame of temporal/feedback history on the wrong chain key (outgoing chain blank/stale for a frame); self-heals since observe() re-latches. (b) write/write conflict with render-thread advanceCrossfade (LayerClock.h:24-26) can lose the trigger's crossfadeProgress=0 -> fade starts mid-way / hard cut. INFERRED.
- Likelihood: the 3 stores (ns; up to ~us across layers in a column trigger) must land in the ~ns read window of a 16 ms frame: ~1e-4..1e-3 per layer per trigger, and only matters for clips with temporal/feedback FX. Fixture hammers triggers, hence 1/6 runs. INFERRED estimate.
- Live-show impact: at worst a one-frame visual glitch on a column fire; no crash.

## Smallest fix
- Same as F9/F12 (one change closes all): message thread enqueues the trigger to a GL-thread mailbox drained at frame start (autopilot already triggers from the GL thread, MainComponent.cpp:4585 comment). Interim: relaxed std::atomic wrappers on the 3 fields (silences UB/TSan, not multi-field consistency). Sacred Rule 2 is the rule bent. Not worth more than LOW.
