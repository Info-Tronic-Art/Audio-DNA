# Verify F4 - TSan race CompositorEngine.cpp:401 incomingImagePending vs TriggerCommands.h:68
VERDICT: REAL_BUG (true data race, technically UB), severity LOW for a live show. Not a tool artefact.

## Refutation attempts (all failed)
- Mutexes in the report (M0 = JUCE RenderThread mutex, M1 = CGL context mutex) are held by the RENDER thread only;
  the message-thread write holds none. VERIFIED (tsan.45047 ~l.299, 331-345). No common lock.
- Message-post happens-before: NONE. Writer = message thread (callAsync -> handleColumnTrigger -> pushCommands ->
  UndoManager::perform -> TriggerClipCmd::apply); the render thread is a free-running GL thread that never receives
  the write via a post. VERIFIED (stack; MainComponent.cpp:5103-5121; core/TriggerCommands.h:64-72).
- Atomics TSan models: fields are plain `int previousClipColumn` / `float crossfadeProgress` (model/Layer.h:182-183). VERIFIED.
  "Render reads the model lock-free" is a stated design assumption (core/UndoManager.cpp:3-5), not a synchronization.
  The deck fence (FencedPtrSlot/withDeckDetached, Renderer.h:112-118) is not used for triggers. VERIFIED.
- Third-party / benign stats counter: no, app code on both sides (CompositorEngine.cpp:401,1074; Layer.h:276-278). VERIFIED.
- No free/realloc in either stack (block = std::vector<Layer> storage from Deck::fromVar).

## Mechanism (INFERRED from reading; not reproduced)
crossfadeProgress has TWO writers: message thread (=0, Layer.h:278) and render thread (LayerClock.h:24 progress+step;
:26 previousClipColumn=-1 at fade end). The render RMW can interleave with the trigger's stores:
 a) lost update: render stores old+step over the trigger's 0 -> new fade starts partway or snaps (one-frame pop).
 b) render's fade-end previousClipColumn=-1 overwrites the trigger's previousClipColumn=X -> fade skipped (hard cut);
    progress then stays 0 with previous<0 until the next trigger. Guards (CompositorEngine.cpp:401, 1646-1647) return the
    new clip texture directly: new clip is shown, no black, no stall. Guards VERIFIED; outcome INFERRED.
 c) reader sees progress/previous from different triggers: one frame of odd mix or cut.

## Live-show impact
- Crash: none expected. Naturally aligned int/float words on arm64 do not tear; previousClipColumn is -1 or a valid
  column, getClipAt null-path returns newClipTex (CompositorEngine.cpp:1650-1653); no vector resize in these stacks.
  INFERRED. A composition-swap overlapping a render read is NOT shown by F4.
- Visual: at most a one-frame crossfade glitch, or a cut instead of a fade, only when a trigger lands within the
  ns-us window of the render thread's step/fade-end in a 16.7 ms frame. Very unlikely per trigger; likeliest when
  mashing triggers on one layer mid-fade. Hit 2/6 because the sweep fired triggers at API rate. (Likelihood INFERRED.)
- Compiler-UB caveat: plain reads could in theory be hoisted; they are per-frame loads in a big function, so unlikely.

## Smallest fix
Make previousClipColumn and crossfadeProgress std::atomic (relaxed) with a copy-ctor/assign shim (Layer lives in a
vector), and clear previousClipColumn at fade end via compare_exchange only if progress is still >= 1. Better, one fix
for the whole family B (F2, F3, F5, F6 share the writers): the trigger stores one atomic pending-trigger word that the
render thread consumes at frame start, so the render thread is the single writer of Layer runtime state.
Not show-blocking; schedule with family B.
