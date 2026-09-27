VERDICT: REVISE - the plan fixes the counting but never interrogates whether the resulting catch-up
behavior is musically correct; it ships a mechanism that, for any real stall, fires a burst of discrete
routine/clip events with zero spacing between them, and it already found a worse form of this (loop
re-fire) and explicitly deferred it rather than fixing it.

1. (MUST) Catch-up is "fire everything due, all at once, no spacing" - never examined as a musical
   choice. Player::advanceTo (src/recording/Player.cpp:126-137) fires every discrete event with
   at <= pos in a single while loop with no time gap between sink.fire() calls. Before this plan,
   a stall could lose at most ~1 beat of position (plan's own analysis, section 1a: "every whole beat
   inside a gap is lost"), which bounded how far behind pos could ever legitimately be. After the fix,
   pos is exact for a stall of ANY length (plan lines 113-116, table row "message thread stalls N
   beats" at line 125: "next tick adds exactly N; Player::advanceTo fires everything due... later
   events back on the grid"). A performer who tabs away, hits a modal dialog, or gets a 3-second OS
   hiccup mid-routine will, on the next tick, have every clip trigger/preamble/point that fell inside
   that window fire in the same ~16 ms message-thread tick - audibly a stack of simultaneous launches,
   not the performance that was programmed. The plan calls this "unavoidable" (line 125) and never asks
   whether holding position (drop the missed triggers, resume live on the current beat) would serve the
   performer better than replaying a compressed burst of everything missed. For a routine meant to land
   events "on the one," firing four bars of triggers in one frame is not "on the one" for any of them.

2. (MUST) The plan's own risk #3 (lines 381-385) is exactly this failure at loop scale, and it is
   deferred, not fixed. RoutineEngine.cpp:576-579 folds one loop cycle per tick; for a stall longer
   than a routine's length, "a looping routine would spin one catch-up cycle per tick (preamble
   re-fired each)." The plan's mitigation is "realistic stalls (0.05-1s) do not reach it for routines
   >= 2 beats" - but Class 2 (the audio-clock pause, section 2d) is explicitly of UNKNOWN duration
   (line 68: "the run's app-err.log was not kept... device rate is unknown too"), and a message-thread
   stall (Class 1, the thing this plan is actually fixing) has no duration bound at all - a debugger
   pause, a blocking file dialog, or the OS swapping the app out can run many seconds. The plan finds
   this bug, states its own fix would surface it more (because today's lossy clock hides it), and ships
   anyway with "follow-up (not here)." That is choosing to convert a silent loss into an audible
   multi-preamble stutter and calling it done.

3. (SHOULD) The only RED test for "several events due at once" tests exactly one event, not several.
   tests/test_routine_engine.cpp case at plan line 345: "the point inside it fires once" - one point,
   one gap. No case in section 5 constructs a gap spanning 2+ discrete points in the same program to
   show what actually happens (bunched, correct-but-simultaneous fire) versus what a reviewer would
   want to see verified before shipping "fires everything due" as the answer.

4. (SHOULD, weaker than I first read it) Section 4 groups "TopBar" with Autopilot/Renderer as an
   untouched lossy wrap-reader (lines 304-306, citing MainComponent.cpp:4134) - but the actual TopBar
   readout (src/ui/TopBar.cpp:320-321, displaySnap_.barCount = snap.barCount) is a direct per-frame
   copy of the tracker's instantaneous barCount, not an accumulating wrap-detector; it has no
   stall-loss bug of the Class-1 kind at all, and today already visibly disagrees with the (currently
   lossy) routine clock during a stall since it never lost anything. MainComponent.cpp:4134 (verified
   read) is setClipFitMode/handleClipTrigger territory, not a beat-phase wrap-reader - the real
   untouched wrap-readers I verified are MainComponent.cpp:3877 (slideshow) and :4038
   (beatSyncRandomize), neither named in the plan's "must not change" list. The substantive point
   (Autopilot/Renderer/slideshow/beatSyncRandomize keep the old lossy behavior) still stands and is
   under-counted; the specific TopBar citation is wrong.

5. (Answering the dispatch directly) Resync/Tap/BPM/Link handling itself is sound and does not
   double-count: BPMTracker.cpp's new realignPhaseToZero() (plan 3.2, mirrored by
   RecorderClock.cpp:57-75 today) decides complete-vs-restart once, at the writer, per hop - verified
   consistent with today's reader-side 0.5 rule at RecorderClock.cpp:57,62. A running routine "never
   jumps back" (table row, line 128) is correctly derived from raw < lastRaw_ absorption
   (RecorderClock.cpp design in 3.4). I found no drift or double-count in the counting arithmetic
   itself - my objection is entirely about what the corrected count is then used to DO (fire a burst).

Strongest counterargument to my own position: skipping missed events instead of firing them changes
take/take-recording semantics the plan explicitly protects (section 4: take format, anchor kinds,
steady-state values unchanged) and would require its own design (which events are droppable, which
aren't, does a loop-restore still restore) - arguably a bigger, riskier change than what's proposed, and
"fire everything due" is at least deterministic and already how Player::advanceTo behaves for smaller
gaps today (a seek does the same thing, just usually over less musical distance). I hold my position
because the dispatch's question is specifically whether catch-up is right at all, and the plan answers
that question by omission - it fixes the counting with real rigor and then asserts the consequence is
"unavoidable" without stating the alternative was considered and rejected, which is the one place in an
otherwise disciplined plan where the musical judgment call is skipped rather than argued.
