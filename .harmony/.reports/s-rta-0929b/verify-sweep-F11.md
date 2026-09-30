# Verify F11 (sweep s-rta-0929b) - Clip::playheadPosition, Renderer.cpp:1823 write vs LayerStrip.cpp:763 atomic_ref read (deck-refresh stack)
VERDICT: REAL_BUG by the letter (C++ data race), severity LOW, owner app. Same defect as F7 (and F14/F17/F28/F29); F11 differs only in the reader's call stack. Live-show effect: nothing visible.

## Report read (VERIFIED: runs/tsan-3/tsan.49175, race at lines ~1495-1560; the 3 stacks around 629/1528 share the DeckView.cpp:276 frame)
- Write 8 B, T47 "OpenGL Renderer", mutexes M0 (juce_OpenGLContext.cpp:795 render mutex) + M1 (CGL ctx lock): Renderer::syncMedia Renderer.cpp:1823 <- CompositorEngine.cpp:1103 <- Renderer.cpp:728.
- Previous "atomic read" 8 B, message thread: LayerStrip::updateTransportView LayerStrip.cpp:763 <- LayerStrip::refresh :737 <- DeckView::refresh DeckView.cpp:276 <- DeckView::setActiveColumn :323 <- MainComponent::handleColumnTrigger MainComponent.cpp:4871 <- callAsync from ApiServer::handleTriggerColumn (the sweep's /api/trigger_column scenario).
- Block: 6816 B vector<optional<Clip>> built by Layer::fromVar Layer.cpp:283 from a live loaded composition (VERIFIED, stack in report). The clip is live, not freed.
- Source (VERIFIED): Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` plain store. LayerStrip.cpp:750 `std::atomic_ref<double>(clip->playheadPosition).load(relaxed)`; comment :747-749 says GL-side writers still write plainly (known open item).

## Refutation attempts (all failed)
1. Hidden lock: reader holds no mutex (report lists mutexes only on the write side; the read is a message-thread timer/callAsync path, no MessageManager::Lock is taken by the renderer for it). VERIFIED (report), JUCE lock semantics INFERRED.
2. Happens-before via callAsync: the message posted from ApiServer thread orders httplib->message thread only; nothing orders GL thread -> message thread. VERIFIED (stacks).
3. Atomic modelled by tool: atomic_ref load vs a NON-atomic store on the same address is still a race; TSan reports "Write" vs "atomic read". The plain writer is the defect. VERIFIED.
4. Third-party / stats counter / tool artefact: no. App-owned live playhead field, mutable double in Clip.h (F7 verify: Clip.h:229). VERIFIED by F7 read; not re-read here.
=> Survives.

## Live-show impact
- Crash: none. INFERRED (live block, scalar double; no pointer or lifetime involved).
- Torn value: essentially none. INFERRED: aligned 8-byte double load/store is single-copy atomic on arm64/x86-64; compiler emits one ldr/str.
- Wrong visual: at most a one-frame-stale playhead cursor in the layer-strip transport bar after a column trigger; the trigger path (handleColumnTrigger -> setActiveColumn) refreshes once, the 30 Hz timer corrects it next tick. INFERRED.
- Adjacent (not this race): a message-thread scrub/reset write to playheadPosition can be overwritten by the next render write (lost update; F7 verify). Not torn, not a crash.
- Likelihood: race is real but only fires while a clip plays and the UI reads (continuous); harmless in practice. Hit 1/6 only because it needs a column trigger with a playing clip.

## Smallest fix
Same as F7: Clip::setPlayhead/getPlayhead via std::atomic_ref<double> relaxed store/load, route the GL writers (Renderer.cpp:1823/1839/1910/1924), Layer.h triggerClipImmediate writes and other message-thread writes/reads through it; static_assert(std::atomic_ref<double>::is_always_lock_free). Closing F7 closes F11; defer past the show.
