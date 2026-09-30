# Verify sweep F6 -- Clip::playheadPosition write/write race (render vs clip trigger)
STATUS: DONE
VERDICT: REAL race, BENIGN in effect (LOW). Not a tool artefact, not JUCE.

## Evidence
- VERIFIED raw tsan-2/tsan.45047 lines 558-683 (also tsan-5): Write 8B by T47 (mutexes M0,M1 = JUCE renderAll callbackMutex/listMutex + CGL ctx lock)
  Renderer::syncMedia Renderer.cpp:1823 <- CompositorEngine.cpp:1103 <- Renderer.cpp:728; previous write 8B by main thread
  Layer::triggerClip Layer.h (triggerClipImmediate inlined; plain writes at :268/:282) <- MainComponent.cpp:4820 (handleColumnTrigger) <- ApiServer callAsync
  (/api/trigger_column). Location = heap block 6816B = std::vector<optional<Clip>> storage (Layer.cpp:283), long-lived, no free in the stack.
- VERIFIED field: Clip.h:229 `mutable double playheadPosition`, plain (non-atomic). Render writes it every frame from the player's atomic (VideoPlayer.h:92) at
  Renderer.cpp:1823 (video) / :1910 (sequence) and on the out-point wrap :1839/:1924; the message thread writes inPoint at Layer.h:268/:282, MainComponent.cpp:4639.

## Refutation attempts (all failed)
1. JUCE lock / message-manager lock: VERIFIED no. Renderer.cpp:57 setComponentPaintingEnabled(false) => juce_OpenGLContext.cpp:367 (`renderComponents && isUpdating`)
   never takes the MessageManager lock, and renderOpenGL() (:414) runs outside it anyway. TSan's mutex set (M0,M1 only) matches. UndoService.cpp:89 says the same.
2. Happens-before via callAsync: VERIFIED no. callAsync orders posting thread -> message thread only; the GL thread is not in that chain.
3. Atomic TSan models: VERIFIED no. Both writes are plain. (LayerStrip.cpp:750 reads via atomic_ref, but its own comment :749 says the writers are still plain;
   mixed plain/atomic access is still a race, which is why family C also reports.)
4. Third-party benign pattern: no, both frames are app code.
5. Stats counter: no, it is model state -- but it is self-healing (see below).

## Live-show effect
- Crash / heap corruption: none. VERIFIED no free/resize in the stack; an aligned 8B double store on arm64 is single-copy atomic in practice (INFERRED from the ISA,
  not a C++ guarantee). (Vector reallocation vs render is the separate, already-fenced UAF class: DeckCommands.h:27 / UndoService withDeckDetached.)
- Torn value: practically none (INFERRED). Formally UB; a compiler could re-load the field at Renderer.cpp:1829 (`clip->playheadPosition >= outPoint`) and see the
  trigger's inPoint instead of the player position -> at worst one skipped/extra out-point wrap check for one frame (INFERRED).
- Lost update: REAL but already known. If render's write lands after the trigger's, the inPoint reset is overwritten with the player's position (MainComponent.cpp:4659
  comment; the retrigger path seeks the player itself to compensate). Playhead readout / ClipPositionSignal (ClipPositionSignal.h:35) is stale <= 1 frame (16.7 ms);
  next frame re-syncs from the player. Nothing visible to an audience.
- Likelihood: needs a trigger within the same ~ns window as the render write; seen 2/6 launches only because scenario b hammers triggers. Human show: rare, harmless. LOW.

## Smallest fix (not applied; read-only)
Route every access through `std::atomic_ref<double>(x).store(v, relaxed)` / `.load(relaxed)`: Renderer.cpp:1823,1839,1910,1924 (read once into a local for the :1829/:1914
compare), Layer.h:268,282, MainComponent.cpp:4639. LayerStrip.cpp:750 already reads via atomic_ref. This clears F6 and family C (F7,F11,F14,F17,F28,F29) together.
Better long-term: a trigger posts "seek to inPoint" to the renderer so the player is the single writer -- larger, not needed for a show.
