# Verify F10 (sweep s-rta-0929b) - Layer crossfade state race, LayerClock.h:24 (via CompositorEngine.cpp:1075) vs Layer.h:250
STATUS: DONE
VERDICT: REAL_BUG (true C++ data race, UB by the letter), severity LOW, owner: app. Not a tool artefact. NOT identical to F9 -- see below.

## Report read (VERIFIED, runs/tsan-3/tsan.49175 lines 362-486; scratchpad sweep/runs, not under .harmony/)
- Write 4 B, main thread: Layer::triggerClip Layer.h:250 (inlined triggerClipImmediate) <- MainComponent::handleClipTrigger :4614 <- callAsync of ApiServer::handleTriggerClip.
- Prior read 4 B, T47 (JUCE OpenGL Renderer), mutexes M0 (JUCE render mutex) + M1 (CGL ctx lock): CompositorEngine::compositeDeck :1075 <- Renderer.cpp:728 <- juce_OpenGLContext.cpp:414.
- Block 3200 B = vector<Layer> storage (Deck::fromVar Deck.h:187, loadFromFile), live, not freed. Offset 0xC70=3184 -> 2nd Layer (sizeof 1600) at +1584, the tail runtime-state fields (INFERRED: crossfadeProgress/previousClipColumn).
- Line :1075 is `advanceCrossfade(layer, dt)` -> LayerClock::advanceCrossfade (render/LayerClock.h:17-28). VERIFIED. It READS crossfadeProgress/previousClipColumn and WRITES both back.

## Refutation attempts (all failed)
1. Atomic/lock? crossfadeProgress/previousClipColumn/activeClipColumn are plain int/float (Layer.h:181-183). No lock in triggerClipImmediate (:255-285) or LayerClock.h. VERIFIED.
2. JUCE MessageManager::Lock? Render callback runs at juce_OpenGLContext.cpp:414 outside it (only taken when component painting is on; Renderer.cpp:57 disables it, per F5 paper). M0/M1 are render-side locks; message thread holds neither. VERIFIED (report + F5 paper).
3. Fence (withDeckDetached, Renderer.cpp:385-428) covers load/drop mutation, not a column/clip trigger; MainComponent.cpp:4600-4614 has no fence. VERIFIED.
4. callAsync happens-before orders API thread -> message thread only, not message -> GL thread. INFERRED from the stacks.
5. Third-party / stats counter / benign? No: live control state in app code. VERIFIED.

## Why not simply "same as F9"
F9 = read of activeClipColumn (:1072, getActiveClip): stale value for a frame. F10 = the render side is a READ-MODIFY-WRITE
(LayerClock.h:24 `crossfadeProgress = min(progress+step,1)`, :26 `previousClipColumn = -1`), so it can CLOBBER the message thread's trigger writes (lost update), not just read stale. Same root cause, slightly stronger effect.

## Live-show effect
- Crash / heap corruption: none. Scalars in a live block; no pointer, resize, free. getClipAt (:1647) bound-checks the index. INFERRED.
- Torn value: none (aligned 4 B, arm64 single-copy atomic). INFERRED.
- Lost update, two shapes (INFERRED from LayerClock.h + Layer.h:276-278; not reproduced):
  a) render loads p, trigger sets prev=X/active=Y/progress=0, render stores p+step (>=1): the new fade is skipped = hard cut instead of dissolve.
  b) render stores progress=1, trigger lands, render then stores previousClipColumn=-1: state prev=-1, progress=0, active=Y. advanceCrossfade is gated on prev>=0 so progress stays 0, but every consumer (:401, :1646, DeckClock.h:37) also gates on prev<0, so the incoming clip simply shows: again a hard cut. Self-heals on the next trigger.
- Wrong visual = one missed crossfade on one layer, only if a trigger lands in a few-ns window of the LAST frame of a running fade. Likelihood per trigger ~1e-6 or lower (ASSUMED order of magnitude); 1/6 TSan runs is detection of the ordering under a trigger-storm fixture, not a frequency. No audio effect, no stuck picture.

## Smallest fix
- ~20 lines, Sacred Rule 2: make the three fields relaxed std::atomic (copyable wrapper, like the Clip::playing fix proposed in F5). Removes UB/TSan noise for F9+F10, NOT the lost update.
- To fix the lost update: message thread posts the trigger to a render-thread mailbox (one atomic per layer, applied at the top of the render loop before advanceCrossfade) or a seqlock around {prev,active,progress}. Not worth it for LOW; batch the atomic wrapper with F5/F9/F13.
