# S166-L1 — ONE evaluator for the signal registry, on ONE thread.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s166/builder-L1-single-evaluator.md

## YOU OWN THE BUILD SLOT
You are the ONLY builder running in this repo right now. `build/` is yours exclusively.
Two read-only reviewers are reading source; they never build. Do not start any other
agent. If that changes I will tell you.

## THE LANE IN ONE LINE
`SignalRegistry::evaluateAll()` is currently called from BOTH the GL thread
(`Renderer.cpp`, around line 231 — RE-GREP BY ANCHOR TEXT, line numbers drift) AND the
message thread (`SignalBar.cpp`, around line 117) on the SAME registry. Two writers, no
ownership, non-deterministic which evaluation wins. Make the message thread the single
owner and delete the other call sites.

## WHY THE MESSAGE THREAD OWNS IT — the design's reasoning, so you can defend deviations
From this session's architecture pass
(`/Users/boriskarpman/Harmony_Main/memory/.reports/s166/arch-universal-connection.md` §4.2 —
read it):
- The preview GL context DETACHES when `previewPanel_` is hidden or zero-sized; this repo
  already moved the v1 mapping tick OFF the GL thread for exactly that reason, so parameters
  keep moving while the preview is hidden. Evaluating on GL re-introduces that bug.
- There are TWO GL contexts (`OutputRenderer`, the preview) and neither is guaranteed alive.
- The registry's `Signal` objects are MUTATED by the UI on the message thread
  (`SignalInspector`) while `getValue` reads them. `OscillatorSignal`'s shape/beatDuration and
  `EnvelopeSignal`'s points are plain fields, and `setPoints` reallocates a vector the GL
  thread iterates. **So GL-thread evaluation is a live data race TODAY.** Confinement removes
  it. This is the real prize of the lane, not tidiness.
- The consumers are on the message thread anyway.

## WHAT TO DO — and the ORDER IS LOAD-BEARING
1. In `MainComponent`'s `tickFeaturePipeline` (the 120 Hz message-thread timer), call
   `signalRegistry_.evaluateAll(snap)` AFTER the snapshot is read and **BEFORE**
   `globalMacroBank_.updateValues(...)` — macros must see this tick's signal values. There is
   an existing comment in that function stating the same ordering rule for macros; find it and
   obey it.
2. ONLY THEN delete the GL-thread call in `Renderer.cpp` and the call in `SignalBar.cpp`.
   **Reversed, this is UNSAFE** — removing the existing evaluations before the new one exists
   leaves the registry unevaluated and every meter and signal-driven parameter frozen.
3. `SignalBar` keeps its own repaint timer and reads `getCachedValue` only. Verify it still
   updates.
4. Add `jassert(juce::MessageManager::existsAndIsCurrentThread())` (match the precedent already
   used in `MappingEngine.cpp` — find it, copy its exact form) at the top of `evaluateAll`.
5. Update the stale comment on `Signal.h` line ~8 that says signals are "evaluated each frame on
   the render thread". It will be false after this lane.
6. **Separately and conditionally:** the design also says to delete the dead
   `routingEngine_.processFrame` call in `Renderer.cpp`. Before you touch it, grep `tests/` AND
   `tests/visual/` for `add_route` (and a second pattern of a different shape — `addRoute`,
   `route` in an e2e URL). If ANY test depends on routes being processed, LEAVE IT and say so;
   it moves to a later lane. If nothing depends on it, delete the call only — do NOT delete the
   RoutingEngine class, its members, or its endpoints. That is a later lane's job and deleting
   it here would put you outside your fence.

## WHAT DONE MEANS
1. `evaluateAll` is called exactly ONCE per tick, from the message thread, and from nowhere else.
   Prove "nowhere else" with TWO greps of DIFFERENT SHAPE.
2. Signal meters still move — including with the preview hidden, which is the case the old GL
   call could not serve.
3. No new allocation inside the tick.
4. A unit test that proves the cached value changes ONLY after an explicit tick (i.e. that
   nothing else is secretly evaluating).

## FENCE — you may edit ONLY these
- `src/MainComponent.cpp` (and `.h` if the tick needs it)
- `src/render/Renderer.cpp`
- `src/ui/SignalBar.cpp`
- the `SignalRegistry` / `Signal` headers+sources for the jassert and the comment
- a test file under `tests/` and its `tests/CMakeLists.txt` entry
Anything else: STOP and report. Do NOT delete the RoutingEngine class or any endpoint.

## BUILD AND TEST RULES — this repo's documented false-green traps
- **Capture the build exit code and READ it before quoting any test number.**
- ctest baseline is **232/232**, produced by me on a clean serialized build at HEAD `2e1afa3`
  with no concurrent builders. Re-run it; never inherit it. Build dir `build`, Release.
- Prove your new test load-bearing: neutralize the fix, rebuild, confirm it FAILS, restore
  byte-identically (verify with md5), confirm it PASSES. Report both numbers.
- Do NOT launch the app. I run the behavioral gate — and for this lane the gate is specifically
  "signal meters still move with the preview hidden", which I will exercise myself.

## REPORTING
STATUS (DONE / DONE_WITH_CONCERNS / BLOCKED), what you changed, the routingEngine grep result
and your decision, build exit code, before/after ctest, load-bearing proof, PACKET QUALITY
(what did this packet get wrong?), and anything outside the fence you noticed.
