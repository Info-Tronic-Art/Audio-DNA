# Verify F12 - Layer.h:250 write vs CompositorEngine.cpp:992 read (tsan-3, tsan.49175 report at line 870-993)
VERDICT: REAL_BUG (formal C++ data race), severity LOW, owner app. Design-known unsynchronised class; live-show effect at worst a one-frame visual glitch.

## Report (VERIFIED, raw file read in full for this stack)
- W: main thread, Layer::triggerClip -> Layer.h:250 (inlined triggerClipImmediate) <- MainComponent.cpp:4614 (handleClipTrigger) <- ApiServer::handleTriggerClip callAsync (JUCE message delivery).
- R: T47 GL render thread, renderLayerStages CompositorEngine.cpp:992 <- compositeDeck :1135 <- Renderer.cpp:728. Line 992 = `crossfadeStart_[clipKey].observe(layer.previousClipColumn, layer.activeClipColumn, layer.crossfadeProgress)` (VERIFIED src). Which of the 3 fields raced is not resolvable from the stack (offset 0xc70 in a 3200-byte vector<Layer> block; INFERRED it is one of the three).
- Render thread held only M0 (JUCE RenderThread mutex) and M1 (CGL context mutex); the message thread takes neither in this path (VERIFIED report + Layer.h:250-285 + MainComponent.cpp:4600-4614). No happens-before exists.

## Refutation attempts
- JUCE lock / message-post HB: no. The GL thread does not take the MessageManager lock around renderOpenGL; trigger runs on the message thread with no render fence. Not refuted.
- Atomics: fields are plain int/float (Layer.h:181-183, VERIFIED). Not refuted.
- Stats counter / third-party: no; app model fields. Not refuted.
- Tool artefact: no. Real concurrent access. (On arm64 an aligned 4-byte load/store is not torn in hardware, INFERRED; the compiler-level UB remains.)
- Design intent (VERIFIED): CrossfadeHistory.h:22-30 states "Layer fields are written on the message thread and read on the GL thread without atomics", and the detector explicitly tolerates every intermediate write order of triggerClipImmediate. Pitfall 38 (docs/claude/pitfalls.md) calls this "the same unsynchronised class". Known, accepted pattern, not a new defect.
- Memory safety: block allocated at load (Deck::fromVar), no free/resize stack reported; scalars only, observe() takes ints/float by value (CrossfadeHistory.h:43). No crash path from this access (VERIFIED for :992).

## Live-show impact
- Worst case (INFERRED from CrossfadeHistory.h + Layer.h:276-278): render frame sees a mix of old/new previousClipColumn/activeClipColumn/crossfadeProgress. The crossfade-start detector fires one frame late/early, so handOverClipHistory (temporal-effect history copy) is skipped or fires spuriously = one frame of wrong temporal-effect history on the triggered layer. Also a lost update: render's advanceCrossfade progress += dt (LayerClock) can overwrite the trigger's crossfadeProgress = 0, turning one fade into a cut (INFERRED). No crash, no persistent corruption.
- Likelihood: render read must land in the ns window of a trigger; seen 1/6 runs, only under TSan with heavy scripted trigger load. Realistically rare and invisible at 60 fps.

## Smallest fix (if ever wanted)
- Wrap previousClipColumn / activeClipColumn / crossfadeProgress in a copyable relaxed-atomic wrapper (Layer lives in std::vector<Layer>, needs a custom copy ctor): silences the report, still not a consistent triple. Consistent fix: handleClipTrigger enqueues the trigger to the render thread (command queue) - larger, not justified for a one-frame glitch. Recommended: leave as is; add a TSan suppression for Layer runtime fields (F2, F9, F10, F12 share the class).
- NOT shown: whether a composition swap can free vector<Layer> under a render read (no free stack here) - Pitfall 55/58 fence, separate question.
