# Verify F5 (sweep s-rta-0929b) - Clip::playing race, Renderer.cpp:1787 vs TriggerCommands.h:68
VERDICT: REAL_BUG (true C++ data race, UB by the letter) - severity LOW - owner: app. Not a tool artefact.

## Report read (VERIFIED, runs/tsan-2/tsan.45047 lines 427-555; in the sweep scratchpad, not under .harmony/)
- Read 1 byte, T47 (JUCE "OpenGL Renderer") in Renderer::syncMedia Renderer.cpp:1787 <- CompositorEngine.cpp:1103 <- Renderer::renderOpenGL 728.
- Prior write 1 byte, message thread: TriggerClipCmd::execute TriggerCommands.h:68 (inlined apply -> `clip->playing = *playing`, :112)
  <- UndoManager::perform <- MainComponent::pushCommands <- handleColumnTrigger MainComponent.cpp:4863 <- callAsync from ApiServer::handleTriggerColumn.
- Heap block 6816 B = vector<optional<Clip>> storage from Layer::fromVar Layer.cpp:283 (a loaded composition). The block is live, not freed.
- Mutexes on T47: only M0/M1 (JUCE render mutex + CGL context lock). The message thread holds none of them. VERIFIED.

## Refutation attempts (all failed)
1. Field is not atomic: `mutable bool playing = false;` Clip.h:228. VERIFIED. No lock around it in Layer.h:286 / TriggerCommands.h:112 / Renderer.cpp:1787/1826.
2. JUCE MessageManager::Lock as hidden sync: renderFrame takes the mm lock only `if (context.renderComponents && isUpdating)` (juce_OpenGLContext.cpp:349-389).
   Renderer.cpp:57 sets setComponentPaintingEnabled(false), and renderOpenGL runs at :414 outside any mm lock. VERIFIED. The TSan mutex set agrees.
3. Fence (withDeckDetached / fenced, Renderer.cpp:390-428) covers load/drop model mutation, NOT the ordinary column-trigger path. VERIFIED (a trigger does not enter the fence).
4. Happens-before via callAsync: the post orders the API thread -> message thread only; nothing orders message thread -> GL thread. INFERRED from the stacks.
5. Benign stats counter / third-party lib: no, this is a live control field in app code. VERIFIED.
6. Redundant write: handleColumnTrigger already ran deck->triggerColumn (MainComponent.cpp:4816) which sets playing (Layer.h:286); the command's execute re-applies the same value (:112).
   So F5's own write is value-idempotent, but the real trigger write (Layer.h:286, same class as F13) is unsynchronised too.

## What it can do live
- Torn value: none (1 byte, aligned; arm64 byte access is single-copy atomic). VERIFIED by type, INFERRED by hardware.
- Crash / heap corruption: none. No pointer, no resize, no free in either stack. INFERRED.
- Real effect = LOST UPDATE. Render syncMedia reads clip->playing (:1787), then after advanceFrame writes it back `clip->playing = player->isPlaying()` (Renderer.cpp:1826).
  If a trigger flips it between those two points, the write-back can overwrite the trigger: an auto-played video clip stays paused (frozen frame) until re-triggered, or a pause is undone.
  Window = one frame, widest when a decode runs in between (ms). Needs a trigger landing in that window; it showed only as 2/6 TSan runs under the load-many-videos + trigger fixture (b).
  Likelihood per trigger: low (about window/16.7 ms, single digits %) and visible only on video clips; images and procedural sources do not use this write-back. INFERRED.
- Compiler-optimisation risk (hoisting the load): none, the read is once per call and is not in a loop. INFERRED.

## Smallest fix
- Make the flag `mutable RelaxedBool playing` (a tiny struct wrapping std::atomic<bool>, relaxed load/store, copy ctor/assign because Clip is copied, Clip.h:321 / optional<Clip>).
  Removes the UB and the TSan noise for F5/F13 and the Layer.h writer. About 15 lines. It does NOT fix the lost update.
- Optional, to fix the lost update: the render write-back only when the player changed it, `if (p != clip->playing) clip->playing = p;` after re-reading. Still a small window.
  The full fix is the message thread posting the play request to a GL-thread mailbox, which is not worth it for LOW.
- Honour Sacred Rule 2 (UI->hot path via std::atomic): this field is a documented UI->render channel that skipped it.
