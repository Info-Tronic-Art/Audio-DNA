# Verify F2 (TSan race Renderer.cpp:593 activeClipColumn vs Layer.h:250 triggerClipImmediate)
STATUS: DONE   VERDICT: REAL_BUG (MEDIUM; benign-looking, never crashes in practice, can glitch a visual)
Raw: scratchpad/sweep/runs/tsan-2/tsan.45047 lines 47-168 (also tsan-3.49175, tsan-5.50099 carry the same pair).

## Refutation attempts (all failed)
- Lock hidden from tool? VERIFIED no. Reader holds M0 (JUCE render-thread mutex, juce_OpenGLContext.cpp:795) + M1 (CGL ctx);
  the writer (message thread, callAsync body) lists NO mutex. The mutexes are not held by the writer, so no common lock.
- Happens-before via callAsync? VERIFIED no: callAsync orders the HTTP thread -> message thread only; nothing orders
  message thread -> GL thread T47. Renderer reads deck through FencedPtrSlot (Renderer.cpp:387) but that fences
  structural edits (withDeckDetached), and handleColumnTrigger (MainComponent.cpp:4781-4820) never enters it.
- Atomic the tool models? VERIFIED no: Layer.h:181-184 activeClipColumn/previousClipColumn/crossfadeProgress are plain int/float.
- Stats counter / 3rd-party? VERIFIED no: app model state, app code both sides. Not a TSan artefact (real interleaving, 3/6 runs).
- Location is 6400 B vector<Layer> (Deck::fromVar Deck.h:187): trigger never resizes/frees clips or layers
  (Layer.h:255-288 only stores ints/floats/playhead/playing) -> no use-after-free from THIS pair. VERIFIED by reading triggerClipImmediate.

## What it can do live
1. Tearing: none. Aligned 4-byte int/float on arm64 (INFERRED from ISA; TSan class is "formal UB", not tearing).
2. Inconsistent tuple for one frame (VERIFIED by reading): trigger writes previous/active/crossfadeProgress as 3 separate stores
   (Layer.h:276-278); render reads them at Renderer.cpp:593, CompositorEngine.cpp:991-992,1072-1075 without a snapshot.
   -> one frame of wrong opacity / wrong clip flash / history handover keyed on a half-written tuple (crossfadeStart_.observe).
3. Lost update (VERIFIED by reading, the worst case): render thread also WRITES the same fields, LayerClock.h:17-27
   (crossfadeProgress += step; previousClipColumn = -1 at end). If a fade finishes on the GL thread while the operator
   fires the next column, the GL store of previousClipColumn=-1 / progress=1.0 can land after the trigger's stores ->
   the new clip HARD-CUTS instead of crossfading (no outgoing clip). Window ~ns..us, needs fade-end in that exact frame:
   likelihood per trigger low (est. well under 1%, ASSUMED, not measured); consequence = one missed fade, self-heals next trigger.
4. Crash: theoretically possible only if the compiler reloads activeClipColumn inside getActiveClip (Layer.h:195-198): bounds
   check sees >=0, index reload sees -1 (clearActiveClip, Layer.h:338 on an empty-cell trigger) -> clips[size_t(-1)].
   INFERRED very unlikely (single load after CSE; clang TBAA; TSan run did not crash) but it is UB, not excluded.
5. Sibling races in the same family (F3-F6, F9-F13, F15-F16, F19-F22: playing / playheadPosition) share the cause: a retrigger's
   playheadPosition=inPoint (Layer.h:268/282) can be overwritten by the render write-back at Renderer.cpp:1823 -> a retrigger that
   does not restart (INFERRED, not asserted by F2 itself).

## Likelihood: race is hit on EVERY live column/clip trigger (keyboard/MIDI/OSC/REST all go handleColumnTrigger, message thread);
   visible effect rare. Sweep b/c ran 120 fps with a trigger scenario; no crash/visible glitch reported.

## Smallest fix
(a) Minimal, silences UB + OOB: give the 4 runtime fields (activeClipColumn, previousClipColumn, pendingTriggerColumn,
    crossfadeProgress) a copyable relaxed-atomic wrapper (Layer is moved into vector<Layer>, so std::atomic alone won't compile);
    load activeClipColumn ONCE into a local in getActiveClip. Does NOT fix the lost-update fade-skip (item 3).
(b) Proper: message thread posts {deck,layer,column,forcedSnap} into an SPSC/atomic mailbox drained at the top of
    renderOpenGL (single-writer model, same idea as pendingTriggerColumn/processPendingTrigger already run by the render side).
    Cost: undo capture (MainComponent.cpp:4800-4840 before/after snapshot) needs the result synchronously -> bigger change.
    Do NOT use withDeckDetached per trigger (holds 1-2 frames, black/held picture on every trigger).
Recommend (a) now, (b) with the Family B batch (F2-F6,F9-F13,F15-F16,F19-F22 are ONE bug).
