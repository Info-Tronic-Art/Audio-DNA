# Verify F19 (sweep s-rta-0929b) - Layer active-column race, CompositorEngine.cpp:1072 vs Layer.h:250 (<- MainComponent.cpp:4820)
VERDICT: REAL_BUG (true C++ data race, UB by the letter) - severity LOW - owner: app. Not a tool artefact. Same family as F9 (the sweep's "as F9" is right), different entry path.

## Report read (VERIFIED, runs/tsan-3/tsan.49175 lines 1768-1800; the report at :252 is the sibling)
- Write 4 B, main thread: Layer::triggerClip Layer.h:250 (= inlined triggerClipImmediate call site; writes previousClipColumn/activeClipColumn/crossfadeProgress, Layer.h:275-277)
  <- MainComponent::handleColumnTrigger MainComponent.cpp:4820 (`deck->triggerColumn(column, forcedSnap)`, Deck.h:111-118 loops layer.triggerClip)
  <- lambda $_90 <- ApiServer::handleTriggerColumn via MessageManager::callAsync (juce_MessageManager.cpp:212).
- Prior read 4 B, T47 "OpenGL Renderer", mutexes M0 (JUCE render mutex) + M1 (CGL ctx) only: CompositorEngine::compositeDeck :1072 (`layer.getActiveClip()`, Layer.h:194-198) <- Renderer.cpp:728 <- juce_OpenGLContext.cpp:414.
- Heap block 3200 B = live vector<Layer> storage (Deck::fromVar Deck.h:187); no free in either stack. The message thread holds no mutex.

## Refutation attempts (all failed)
1. Atomic/lock: activeClipColumn is a plain int (Layer.h:181); triggerClip/getActiveClip take no lock. VERIFIED.
2. Hidden JUCE lock: renderOpenGL runs with only M0/M1 (report's own mutex sets); the message thread holds neither. VERIFIED (report).
3. Fence (withDeckDetached): handleColumnTrigger 4779-4830 read, no fence call. VERIFIED for that range.
4. callAsync orders API -> message thread only, not message -> GL thread. INFERRED (JUCE semantics).
5. Stats counter / third-party: no; live control state in app code. VERIFIED.
6. Write/write too: render thread writes crossfadeProgress/previousClipColumn in advanceCrossfade (render/LayerClock.h:24-26, called CompositorEngine.cpp:1075) while the trigger writes the same fields (Layer.h:276-277, 337-339). VERIFIED.

## Live-show effect
- Torn value: none (aligned 4 B, single-copy atomic on arm64). INFERRED.
- Wrong visual: one frame of an odd (previous, active, progress) triple; self-heals next frame. INFERRED.
- Lost update: render's read-modify-write `crossfadeProgress = min(p+step,1)` can overwrite the trigger's `crossfadeProgress = 0` -> that crossfade is skipped (hard cut), previousClipColumn = -1. Window ~tens of ns -> practically never per trigger. INFERRED estimate.
- Crash: correction to the F9 paper's "only ever a validated column". An EMPTY-cell trigger reaches clearActiveClip (Layer.h:224-230, 330-339), which sets activeClipColumn = -1. getActiveClip (Layer.h:194-198) reads the plain int for the bounds test, has_value() and value(). If the compiler does not fuse those loads, a trigger landing between them turns `clips[size_t(-1)]` into an out-of-bounds read of the optional (garbage Clip*). The optimizer probably fuses them (TSan shows ONE read at :1072). INFERRED, very low likelihood; never observed in 6 TSan + 31 ASan launches (ASan: zero reports).
- Overall: nothing visible in a normal show; a trigger-hammering fixture (column trigger via API) exposes it 1 in 6 runs.

## Smallest fix
- Minimal: in getActiveClip (and its twin Layer.h:203-208) copy activeClipColumn to a local once (`const int c = activeClipColumn;`) - 2 lines, removes the OOB possibility even without atomics.
- Also make active/previousClipColumn/crossfadeProgress relaxed std::atomic wrappers (copy-constructible struct, Layer sits in vector<Layer>) - silences TSan, ~20 lines; no multi-field consistency.
- Proper: message thread posts the trigger into a GL-thread mailbox drained at frame start (single writer), per Sacred Rule 2.
