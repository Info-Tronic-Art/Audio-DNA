# Verify F14 (sweep s-rta-0929b) - Clip::playheadPosition, Renderer.cpp:1823 write vs LayerStrip.cpp:763 atomic_ref read (resized path)
STATUS: DONE
VERDICT: REAL_BUG by the letter (C++ data race, UB), severity LOW, owner app. Same defect as F7; not a tool artefact. Live-show effect: nothing visible.

## Correction to the sweep's first look
- The sweep says "Component paint". VERIFIED wrong: the stack is LayerStrip::resized() LayerStrip.cpp:694 <- juce Component::setBounds
  <- DeckView::layoutGrid DeckView.cpp:373 <- DeckView::resized :106 <- rebuildGrid :259 <- MainComponent::handleDeckSwitch :5508
  <- ApiServer::handleSwitchDeck callAsync (tsan-3/tsan.49175:1129-1145; tsan-6/tsan.50561:802 and :1703, 2 reports). No paint frame.
  So this is the deck-switch layout path, not the 30 Hz timer path of F7 (F7 = timerTick :783). Same read function, different caller.

## Report read (VERIFIED)
- Read: "Atomic read of size 8", main thread, LayerStrip.cpp:763 (transportViewOf's std::atomic_ref<double>(clip->playheadPosition).load(relaxed)).
- Write: 8 B, T47 OpenGL Renderer, holding only M0 (JUCE render mutex) + M1 (CGL lock): Renderer::syncMedia Renderer.cpp:1823
  `clip->playheadPosition = player->getPlayheadPosition();` <- CompositorEngine.cpp:1103 <- Renderer.cpp:728. Plain store.
- Field: Clip.h:229 `mutable double playheadPosition` (plain double). Heap block = vector<optional<Clip>> from Layer::fromVar (the live loaded composition).

## Refutation attempts (all failed)
1. Hidden lock / happens-before: message thread holds neither M0 nor M1 (TSan mutex sets, VERIFIED); a switch_deck REST callAsync gives no ordering
   with the GL thread (INFERRED from stacks). Not JUCE-internal: both frames app code.
2. Tool models atomic_ref: it does, and reports "Atomic read" vs plain "Write"; atomic vs non-atomic on one address is still a race. VERIFIED.
3. Benign stats counter? No: a live control value that drives the drawn playhead x. VERIFIED (LayerStrip.cpp:763-770).
4. Lifetime: the block is live, not freed (no "freed by" section); no heap-use-after-free class here. VERIFIED (report has only alloc stack).
=> Survives. Half-fix already in tree: reader is atomic_ref (comment LayerStrip.cpp:747-749 admits GL writers are still plain), writers are not.

## Live-show impact
- Crash: none from this race (INFERRED; scalar double, no pointer/heap effect).
- Torn value: essentially none on arm64 (INFERRED: aligned 8-byte str/ldr is single-copy atomic; compiler emits one access for a scalar).
- Wrong visual: at most a playhead x one frame stale right after a deck switch, corrected by the next timer tick (30 Hz, timerTick updateTransportView). ASSUMED negligible.
- Likelihood: 2 of 6 TSan runs only because scenario b/c issued switch_deck; occurs on every deck switch with a playing clip. Harmless in practice.

## Smallest fix (same as F7; one fix closes F6/F7/F11/F14/F17/F28/F29)
Add Clip::setPlayhead(double)/getPlayhead() using std::atomic_ref<double>(playheadPosition).store/load(relaxed) (+ static_assert is_always_lock_free),
route the GL writers (Renderer.cpp:1823/1839/1910/1924, Layer.h:268/282) and the unsynchronised message-thread readers/writers through it.
Do not make the field std::atomic<double>: Clip is copied/moved. Defer past the show.
