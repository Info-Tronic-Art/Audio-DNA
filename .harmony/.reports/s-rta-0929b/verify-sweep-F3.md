# Verify sweep F3 - TSan race Renderer.cpp:593 (R, T47) vs TriggerCommands.h:68 (W, main)
VERDICT: BENIGN-in-practice formal data race (UB by the C++ model), same class as F2. Severity LOW. Owner: app.

## Raw report (VERIFIED, tsan-2/tsan.45047 ~lines 172-235; same stack in tsan-5/tsan.50099:248,359)
- Read 4B at Renderer.cpp:593 by T47 (holds only JUCE render mutex M0 + CGL ctx mutex M1); prior write 8B by main thread
  TriggerClipCmd::execute (TriggerCommands.h:68, inlined applyLayerRuntime) < CompositeCommand.h:35 < UndoManager.cpp:15
  < MainComponent::pushCommands:5120 < handleColumnTrigger:4863 < ApiServer callAsync lambda.
- Location: 6400-B heap block = std::vector<Layer> storage from Deck::fromVar (Deck.h:187); long-lived, no free/realloc in the stack.
## Refutation attempts
- Hidden lock? NO. Writer stack holds no mutex; reader's M0/M1 are render-side only. No common lock. (VERIFIED, report header)
- Message-post happens-before? NO. Render thread never waits on the message thread here. (INFERRED from stack)
- Tool artefact? NO. Layer.h:181-184 are plain int/float, not atomic (VERIFIED).
- Design intent: TriggerCommands.h:34-36 says "NO GL FENCE ... status-quo field-level race, exactly like today's direct writes":
  a known accepted choice (VERIFIED), at odds with the letter of CLAUDE.md sacred rule 2.
## Why it is nearly harmless
- Mutate-then-push (TriggerCommands.h:20-26; MainComponent.cpp:4831-4863, VERIFIED): the live trigger already ran and `after` was
  captured from the live Layer, so execute() re-stores identical values. Same-value write: no visible change (INFERRED).
- The 8-B store = compiler merging two adjacent aligned ints; aligned stores are single-copy atomic on arm64; reader loads 4B: no
  tearing (INFERRED, codegen not disassembled).
- Reader Renderer.cpp:593 only tests activeClipColumn < 0; getActiveClip() bounds-checks vs clips.size() (Layer.h:193-201, VERIFIED).
## Live-show impact
- Crash: none from this pair (no vector resize/free involved). Torn value: no. Wrong visual: none normally.
- Residual (INFERRED, not reproduced): execute() also rewrites pendingTriggerColumn/crossfadeProgress/previousClipColumn from a
  snapshot taken microseconds earlier. If the render thread changes them inside that window (beat-snap consumption of a queued
  trigger; autopilot Layer::triggerClip on the GL thread) the stale snapshot wins: a clip could re-fire (restart at in-point) on the
  next beat or a crossfade reset. Window = microseconds per Human trigger; negligible probability; effect = one-clip glitch.
- TSan pair fires whenever a trigger overlaps a frame (2/6 only reflects the scenario mix).
## Smallest fix (proposal, not applied)
- (a) In TriggerClipCmd add `bool firstPerform_=true;` and make execute(){ if(firstPerform_){firstPerform_=false;return;} apply(after_,...);}
  (state is already live; redo after undo still works). Removes F3 and the stale-snapshot window, ~4 lines. NOT checked: that
  UndoManager redo calls execute() again and that perform() is the only first-call path.
- (b) Class-wide (F2, F20-F22, ...): make Layer runtime fields std::atomic (relaxed). Larger; touches copies/serialisation.
## Labels
VERIFIED: stacks, source lines, no lock, mutate-then-push, plain fields. INFERRED: no tearing, negligible stale window. ASSUMED: none.
