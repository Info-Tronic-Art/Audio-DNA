# Reviewer Verdict -- bf2 keys & MIDI (S4) round 2
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES (FAIL: 1 MUST, the r1 MUST still open)
PINNED: lane/bf2 head 68abc16d71f3825cb31fbbb0a8a00939a4ee63e6; fix round db950ab..68abc16 (7 first-parent commits)
FILES: docs/claude/performance-controls.md, .harmony/APP-INVENTORY.md (keys-relevant); src/ui/MidiLearnOverlay.cpp, src/binding/* (re-grepped, unchanged)

## MUST
1. [VERIFIED] r1 MUST 1 is NOT fixed; the round only documented it. `git diff db950ab..68abc16 -- src tests` touches only
   src/analysis/BeatLead.{cpp,h} (comments). `git grep -n ccMode 68abc16 -- src` still finds only Binding.h:69 (default
   Absolute) and BindingManager.cpp:173/177/239/285; no overlay sets Relative. So a knob or encoder MIDI-learned onto
   "Sync -1 ms" / "Sync +1 ms" is bound and does nothing (BindingManager.cpp:173 drops Absolute + SyncNudge). The plan chose
   "(a) CHOSEN -- keys / notes nudge; endless-encoder CCs (relative mode) nudge 1 ms per tick" (plan-bf2.md:207, :378-:385), so the
   MIDI-CC half of the ruled item is dead for a user. The lane report FIX-R1 row 1 admits "NOT FIXED ... STOP ITEM 1";
   rulings-bf2.md (39 lines, H-1..H-8) holds no ruling on it (grep for relative / ccMode / S4 / I9 empty), so nobody has
   decided and the defect ships. The brief says a MUST must be fixed as found, not documented away.
   FIX (Harmony to rule, builder to do): candidate (a) -- MidiLearnOverlay's CC capture block sets `b.ccMode = Relative`
   when `target.action == SyncNudge`, plus a test of the learn path (or a pure helper the test calls). A fader learned there
   would then nudge by value-64, but an Absolute CC on this action is ignored anyway, so Relative is the only mode in which a
   learned CC does anything. Or rule (b): keys and notes only, delete the CC claims from performance-controls.md:41 and
   APP-INVENTORY, and make the engine's CC arm and the overlay's CC reach match that. Either way this is Harmony's call first.

## SHOULD
2. [VERIFIED] r1 SHOULD 3 (the handler case MainComponent.cpp:7939-7944 is re-implemented in the test's lambda,
   test_binding_sync_nudge.cpp:166-170, never driven) is declined with reasons (lane report row 9). Reasons hold: a string-presence
   lint passes on a wrong-sign mutant and no unit target builds MainComponent. Accepted as a residual risk; the mutant claim
   is INFERRED (nothing was run).

## NIT
3. [VERIFIED] r1 NIT 4 (targetSyncStepMs unclamped in fromVar, BindingManager.cpp:296-297; INT_MIN * ticks is signed overflow on a
   corrupt file) and NIT 5 are untouched; unchanged from r1, no action required.

## FIX CHECK
- r1 SHOULD 2 (docs overclaim): FIXED. performance-controls.md:41 now reads "in the ENGINE ONLY today: no screen makes a Relative
  binding (MIDI learn creates every CC binding Absolute ... a knob or encoder LEARNED onto a Sync target does nothing ...)";
  APP-INVENTORY.md:303 says the same. Both match the code at the head (grep above). VERIFIED. The honest docs make the MUST
  visible but do not remove it: they re-create no advisory trap, they document a dead control.
- New defects from the fix round in my lens: none. The round's src change is 2 files of comments (BeatLead), outside S4. The
  docs edit adds no on-screen text. CLAUDE.md untouched (VERIFIED, not in the diff stat). Tests: the round adds none for S4, so
  there is no new test to check for RED; test_binding_sync_nudge.cpp is unchanged since r1.
- Other r1 conformance items (targets house way, message-thread single writer, clamp, persistence, no event text) are in files the
  round did not touch (diff stat above), so r1's VERIFIED lines stand.

SLIM: nothing added in my lens this round.
SUMMARY: 1 blocking (r1 MUST open, needs Harmony's ruling then a builder), 1 suggestion, 1 nit. Confidence: VERIFIED by git objects at the pinned head; nothing built or run (not permitted).
METADATA: reviewer=reviewer-keys-r2, date=2026-10-03
