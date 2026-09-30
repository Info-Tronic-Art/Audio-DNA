# Verify sweep F15 - TSan race Renderer.cpp:593 (R, T47) vs TriggerCommands.h:68 (W, main) via handleClipTrigger:4744
VERDICT: BENIGN-in-practice formal data race (C++ UB, no atomics). Severity LOW. Owner: app. Same class/verdict as F3 (verify-sweep-F3.md).
## Raw report (VERIFIED, tsan-6/tsan.50561 lines 171-294; tsan-3/tsan.49175:761,1262 same frames)
- R: 4B at Renderer.cpp:593 (`layer->activeClipColumn < 0`, playlist-beat loop) by T47 holding only JUCE render mutex M0 + CGL ctx mutex M1.
- W: 8B by main thread TriggerClipCmd::execute (TriggerCommands.h:68 -> apply -> applyLayerRuntime, DeckCommands.h:210-216)
  < UndoManager::perform:15 < pushCommands < handleClipTrigger:4744 < REST /api/trigger_clip callAsync lambda (tsan-3: MainComponent.cpp:5113 frame).
- Location: 3200-B heap block = std::vector<Layer> storage (Deck::fromVar Deck.h:187, loadComposition:3300); live, no free/realloc in stack.
- Only difference vs F3: single-cell trigger (not a column composite) and a 2-layer block instead of 4.
## Refutation attempts (all failed)
- Hidden lock: NO. Writer's stack shows no mutex; M0/M1 are render-side only (VERIFIED, report header).
- Message-post happens-before: NO. Render thread never waits on the message thread for this field (VERIFIED: fields plain int/float, Layer.h ~180-184).
- JUCE-internal / tool artefact: NO. Both frames are app code; the fields are non-atomic. The race is real by the model (VERIFIED).
- Design intent: TriggerCommands.h:34-36 "NO GL FENCE ... status-quo field-level race" (VERIFIED) - accepted, but at odds with CLAUDE.md rule 2.
## Why it is nearly harmless
- Mutate-then-push (TriggerCommands.h:20-26; MainComponent.cpp:4731-4747, VERIFIED): the live trigger already ran and rtAfter was captured from
  the live Layer; execute() re-stores identical values (push only when something changed). Same-value rewrite -> no visible change (INFERRED).
- Reader tests only `< 0` then getActiveClip() bounds-checks against clips.size() (Layer.h:193-201, VERIFIED) -> no OOB. No vector resize/free involved.
- 8B write = two adjacent aligned 4B stores merged; arm64 aligned stores are single-copy atomic; 4B read cannot tear (INFERRED, not disassembled).
## Live-show impact
- Crash: none from this pair. Torn value: no. Wrong visual: none normally.
- Residual (INFERRED, not reproduced): execute() rewrites pendingTriggerColumn/crossfadeProgress/previousClipColumn from a snapshot taken
  microseconds earlier; if the GL thread advances them in that window (beat-snap consume, autopilot triggerClip) the stale snapshot wins:
  one clip re-fire or crossfade reset. Microsecond window per human trigger; likelihood negligible; effect = one-clip glitch.
- Fires in TSan whenever a REST/UI trigger overlaps a frame (2/6 reflects the scenario mix only).
## Smallest fix (proposal, not applied)
- (a) TriggerClipCmd: `bool first_=true;` execute(){ if(first_){first_=false;return;} apply(after_,...);} ~4 lines; state already live.
  NOT checked: that UndoManager redo calls execute() again and that perform() is the only first-call path; mergeWith interplay untested.
- (b) Class-wide (F2,F3,F20-F22...): make Layer runtime fields relaxed std::atomic; larger, touches copies/serialisation.
## Labels
VERIFIED: stacks, source lines, no lock, mutate-then-push, plain fields, bounds check. INFERRED: no tearing, negligible stale window. ASSUMED: none.
