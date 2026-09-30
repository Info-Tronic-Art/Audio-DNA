# Verify F27 - TSan race composition_.activeDeckIndex (MainComponent.cpp:5492 write) vs Renderer.cpp:471 (renderOpenGL read)
VERDICT: REAL_BUG (formal data race / UB on a plain int), severity LOW for a live show. Not a tool artefact. Owner: app.

## Report read - VERIFIED (runs/tsan-6/tsan.50561 l.682-795)
- Write 4B, main thread: handleDeckSwitch MainComponent.cpp:5492 <- REST onSwitchDeck lambda (MainComponent.cpp:1905) <- callAsync (juce_MessageManager.cpp:212).
- Prev read 4B, T47 "OpenGL Renderer" holding only M0 (RenderThread) + M1 (CGL ctx): Renderer.cpp:471 `const int currentDeckIdx = composition_->activeDeckIndex;`.
- Block 96064 B = MainComponent (Main.cpp:48); composition_ is a by-value member. Field is `int activeDeckIndex = 0` (model/Composition.h:35), plain, not atomic. VERIFIED.

## Refutation attempts (all failed to make the pair synchronised)
- Common lock: M0/M1 are render-thread-only; the message-thread writer holds none. VERIFIED (report "mutexes:" line).
- Message-post happens-before: none; the GL thread is free-running, callAsync orders HTTP -> message thread only. VERIFIED.
- Atomic the tool models: the ONLY atomic nearby is activeDeck_ (FencedPtrSlot.h:36-45, release store / acquire view()).
  Renderer.cpp:467-470 comment relies on it ("read after the acquire-load of activeDeck_"). Writer order is idx store (:5492) THEN setActiveDeck
  = release, so a frame that sees the NEW deck ptr is ordered and sees the new idx. A frame that sees the OLD ptr has no ordering vs the
  idx store -> exactly the reported race. The comment's guarantee is one-directional. VERIFIED (read both sides).
- Benign stats counter / third party: no; app code both sides, and the value gates a render decision. VERIFIED.
- Tool artefact: no; aligned 4 B, real concurrent access. Hit 1/6, timing-rare (deck switch inside the same ~16 ms frame). VERIFIED (sweep.md count).
- Other writers of the same field exist (Composition.h:177,221,439; DeckCommands.h:719 remove/undo deck, inside the deck fence) - same class. VERIFIED (grep).

## Live-show impact
- Torn value: no (aligned 4-byte int on arm64). Crash: no - the value is only compared to prevActiveDeckIndex_ and stored (Renderer.cpp:471-493), not used as an index there. VERIFIED.
- Worst realistic outcome: renderer sees new idx with the old deck ptr for ONE frame: cross-deck transition starts 1 frame (16.7 ms) before the deck
  actually swaps; the blit copies the canvas = old deck's last frame, so the picture is identical. On a fenced RemoveDeck the transition may be spuriously
  started or cut. Practical result: nothing visible. INFERRED (from reading :471-493; not reproduced).
- Formal UB: a compiler could in theory cache/reorder the plain read; today it is one read per frame, so low. ASSUMED.

## Smallest fix (not applied)
- Renderer.cpp:471: derive the index from the pointer it already acquire-loaded: idx = deckView.ptr - composition_->decks.data() (as at :544-546), keep
  prevActiveDeckIndex_ when deckView.ptr is null (fenced). No new atomics, no type change. INFERRED (decks.data() is read outside a lock, but only under
  the same deck fence that already guards :544).
- Alternative: std::atomic_ref<int> relaxed at all 5 access sites (needs C++20 atomic_ref in this Xcode libc++: ASSUMED).
- Or a tolerance comment + TSan suppression, as done for the Layer crossfade fields (CrossfadeHistory.h:24-31; see F21 paper).
