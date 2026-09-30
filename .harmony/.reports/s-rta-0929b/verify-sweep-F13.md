# Verify F13 (sweep s-rta-0929b) - Clip::playing race, Renderer.cpp:1787 vs TriggerCommands.h:68 (handleClipTrigger path)
VERDICT: REAL_BUG (true C++ data race, UB by the letter), severity LOW, owner app. Same defect as F5 (same read/write lines; only the message-thread caller differs). Not a tool artefact.

## Report read (VERIFIED: scratchpad sweep/runs/tsan-3/tsan.49175 lines 995-1123; tsan-6/tsan.50561 lines 427-552, 1187-1312)
- Read 1 B on T47 (OpenGL Renderer): Renderer::syncMedia Renderer.cpp:1787 (`if (clip->playing && ...)`) <- CompositorEngine.cpp:1103 <- Renderer.cpp:728. Mutexes held: only M0/M1 (JUCE render mutex, CGL lock).
- Prior write 1 B on main thread: TriggerClipCmd::apply `clip->playing = *playing` (TriggerCommands.h:68 in the sweep's build, inlined via execute) <- UndoManager::perform (core/UndoManager.cpp:15 `cmd->execute()`) <- pushCommands <- handleClipTrigger MainComponent.cpp:4744 <- callAsync from ApiServer::handleTriggerClip.
- Heap block 6816 B = vector<optional<Clip>> storage from Layer::fromVar Layer.cpp:283 (composition load). Live block, no free in either stack.

## Refutation attempts (all failed)
1. Field is `mutable bool playing = false;` Clip.h:228, not atomic; no lock in TriggerCommands.h:68, Layer.h:286 or Renderer.cpp:1787/1826. VERIFIED.
2. JUCE message-manager lock as hidden sync: not held by the render thread on this path (renderOpenGL runs at juce_OpenGLContext.cpp:414 outside it; the TSan mutex set shows only M0/M1). VERIFIED by the report; the F5 verifier read the same JUCE lines.
3. callAsync happens-before orders API thread -> message thread only, nothing orders message thread -> GL thread. INFERRED from stacks.
4. Not a stats counter, not third-party: a live control field, app code. VERIFIED.
5. Write is redundant in value: handleClipTrigger already ran Layer::triggerClipImmediate (Layer.h:286, `clip->playing = true`, same unsynchronised class), took playAfter from the model (MainComponent.cpp:4619), then pushCommands -> UndoManager::perform re-executes the command (UndoManager.cpp:15) and rewrites playing = playAfter. So F13's own write is a value-idempotent second write of the same flag, UNLESS the render thread changed playing in between (OneShot end / write-back at Renderer.cpp:1826), in which case execute() re-asserts the stale value. VERIFIED by reading; the interleaving is INFERRED.

## What it can do live
- Torn value: none (aligned 1-byte bool; arm64 byte access is single-copy atomic). VERIFIED by type, INFERRED by hardware.
- Crash / heap corruption: none. No pointer, resize or free involved. INFERRED.
- Real effect: LOST UPDATE on video clips. syncMedia reads clip->playing (:1787), decodes, then writes back `clip->playing = player->isPlaying()` (:1826). A trigger/undo landing between read and write-back can be overwritten: a freshly auto-played video stays paused (frozen frame) until re-triggered, or a pause is undone. Compiler hoisting: none (single read per call, no loop). INFERRED.
- Likelihood: a trigger must land in a ms-scale window (widest when a decode runs), so low single-digit % per video-clip trigger; only 2/6 TSan runs under the load-many-videos + trigger fixture. Images/procedural sources do not use the write-back. Visible failure = one video layer not starting; recoverable by re-trigger. INFERRED.

## Smallest fix
- Make `playing` a relaxed atomic wrapper (`mutable RelaxedBool`, std::atomic<bool> with copy ctor/assign since Clip is copied, ~15 lines, Clip.h:228). Removes the UB and TSan reports F5/F13 and the Layer.h:286 writer. Does not fix the lost update.
- Cheap mitigation of the lost update: at :1826 write back only on change (`const bool p = player->isPlaying(); if (p != clip->playing) clip->playing = p;`). Full fix (GL-thread mailbox for play requests) not worth it at LOW.
- The double-apply (mutation in handleClipTrigger, then execute() re-applies) is by design for the redo path, not a separate bug.
