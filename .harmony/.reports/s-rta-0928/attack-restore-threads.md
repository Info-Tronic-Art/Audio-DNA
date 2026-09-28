# Attack: plan-restore.md (threading + routine-semantics skeptic) -- s-rta-0928

VERDICT: SOUND_WITH_FIXES

## What checks out (verified against main@6db8d67)
- No new data race. RoutineEngine::tick is message-thread gated
  (ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD(), src/recording/RoutineEngine.cpp:514), and Player::advanceTo
  (src/recording/Player.cpp:126-181) is only ever called from there or from startNow, itself message-thread-only.
  ClipThumbnails's pool jobs (plan-restore.md:470-479) touch only a value-copied juce::File; cache_/inFlight_/
  failed_ are touched only inside get()/landed(), both jassert'd or scheduled onto the message thread via
  callAsync. No shared mutable state crosses the pool/message boundary unguarded. This is the load-bearing
  threading question and the plan clears it.
- F5's mechanism is accurate: touch at Player.cpp:157, "release with no set" close branch at :162-170, exactly as
  cited (plan said :157/:162-170).
- F6's ownership check is accurate: SlotSink::set/release key off laneOwner_ (RoutineEngine.cpp:125-139),
  confirming a later routine's/human's grip is respected by the new end-value write too.
- F1 (decode-on-every-refresh) verified live: ClipCell::updateThumbnail (ClipCell.cpp:393-412) and
  LayerStrip::updateThumbnail (LayerStrip.cpp:938-987) both do a synchronous ImageFileFormat::loadFrom on the
  calling thread today, matching the diag and the plan's F1/F2.

## MUST/SHOULD findings

1. SHOULD -- The cause-2 fix is broader than "removes the stall bug"; it changes normal, non-stalled restore math.
   G5/J2 (tests/test_routine_engine.cpp:1127-1142, :1519-1525) prove it: a gesture recorded to end exactly ON a loop
   boundary already produces an ev0 sequence without any stall involved, and the plan's fix changes the Ease glide's
   *from* value at that boundary from the last-sampled tick (0.886, an artifact of tick granularity) to the recorded
   end (0.9). That is a real change to "what a routine restores" for a common recording pattern (a knob held through
   the loop point), not an edge case reachable only via a message-thread stall. The plan discloses and pins this
   (RULINGS Cause 2 / MUST NOT CHANGE, plan-restore.md:160, :785-789) -- not hidden -- but its own framing
   ("It is one line", plan-restore.md:38) understates the blast radius: every routine whose Ease-loop lane ends a
   gesture on the boundary changes its glide-from value on every normal cycle, forever, not just during stalls.
   FIX: reframe the summary line to say what F7/RULINGS already say -- this also changes normal glide-restore math,
   not only the stalled case -- so whoever accepts the plan is accepting that trade knowingly. BORIS_DECISIONS.md:328
   only rules on the glide's target/"recorded starting look", not its from-value at a mid-loop gesture end.

2. SHOULD -- New telemetry edge case at every gesture close, untested. SlotSink::set increments "yielded"
   whenever laneOwner_[key] differs from slot (RoutineEngine.cpp:125-131), and the code comment there says "once
   per gesture: the Player never calls set() again for a displaced gesture" -- true today, false after this fix,
   because the fix adds a second sink.set() call (at close) for gestures that were NOT displaced at touch time. If
   a second routine takes the same knob between a gesture's last in-gesture tick and its close tick, "yielded" now
   increments an extra time it never did before. The plan names this in R-3 (plan-restore.md:851-855) but ships no
   dedicated test for it -- C1-C5/E1 all use a single-lane FakeSink with no ownership handoff mid-gesture. Given D9
   ("the later routine takes it over ... yes", BORIS_DECISIONS.md:329) is a Boris-ruled, user-visible behavior,
   recommend one stacking test: routine A holds a gesture ending at beat T; routine B takes the same key just before
   T; assert the end-value set is refused, no release, and yielded/refusedByHand land where the existing stacking
   tests (plan cites :604-668, G6, row 11) expect them to.

3. NIT -- A few wiring citations have drifted 1-3 lines from HEAD: the setClip call in rebuildGrid is now at
   DeckView.cpp:186, not the plan's ":184/:185"; ClipCell::updateThumbnail begins at ClipCell.cpp:393, not ":394".
   Non-fatal (Core Rule 1 has the builder re-read before writing regardless), but since the dispatch explicitly asks
   for file:line verification, note it: a builder trusting the line numbers over the re-read could misplace the
   setThumbnails(...) calls relative to setLayer/setClip by a line or two -- harmless here since both are
   single-statement insertions immediately before a named call, but the pattern is worth a mid-plan re-grep gate.

4. SHOULD -- Diagnosis attribution meets the bar the temperament asks for (numbers, five or more runs): card
   fixture n equals 10 per event, big4k/many n equals 5 (restore-diag.md section 3), and the stall reproduction is
   5 of 5 versus 0 of 5 control (section 5). This is a real attribution, not a guess-fix. The one place the plan's
   own confidence is weaker: T1/T2 "expected under 1 ms" for the fixed state is INFERRED from a no-image
   counterfactual (restore-diag.md:68-70), never measured with the actual ClipThumbnails store in place (nothing
   was built). That's disclosed as INFERRED and gated behind Step 7's live run, which is correct process --
   flagging only because "expected under 1ms" reads confident in the Targets table (plan-restore.md:196-201)
   despite being unmeasured until the builder runs Step 7.

## Self-counterargument
The strongest case against finding 1 is that the plan already rules on it explicitly, tests it exactly (G5/J2), and
names it in "MUST NOT CHANGE" with the honest delta (0.9 vs 0.886) -- a diligent reviewer reading the whole document
would not be surprised. I hold the finding anyway because the summary line the architect wrote ("Cause 2 ... It is
one line") is what a time-pressured chair or builder skims first, and it does not by itself convey that a normal
(non-stalled) loop-return glide changes for a common recording shape -- the reader has to reach RULINGS/MUST-NOT-
CHANGE to learn that. A one-line summary that undersells a real, permanent behavior change on the hot path a Boris
ruling governs (routine feel) is exactly the kind of thing a routine-semantics skeptic should surface even when the
detail is present elsewhere in the same document.
