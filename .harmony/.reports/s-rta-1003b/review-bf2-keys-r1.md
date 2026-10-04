# Reviewer Verdict — bf2 keys & MIDI (S4) round 1
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES (FAIL: 1 MUST)
PINNED: lane/bf2 head db950abdbe4bd95b1d5faa07c1c3898d4913479d, base 101dcab (3 first-parent commits: e15d1d0 code+test, d9e34b4 docs, db950ab report)
FILES: src/binding/Binding.h, src/binding/BindingManager.cpp, src/MainComponent.cpp, src/ui/BindingOverlay.{h,cpp}, src/ui/MidiLearnOverlay.cpp, tests/test_binding_sync_nudge.cpp, tests/CMakeLists.txt, docs/claude/{performance-controls,analysis,architecture}.md, .harmony/APP-INVENTORY.md

## MUST
1. [VERIFIED] A MIDI knob / encoder learned onto "Sync -1 ms" / "Sync +1 ms" does nothing, and no UI can make a Relative CC binding.
   `git grep ccMode` at the head: only Binding.h:69 (default Absolute), BindingManager.cpp:173/177/239/285. MidiLearnOverlay.cpp
   CC-capture block (:262-:277) builds `Binding b` and never sets ccMode, so every learned CC is Absolute. BindingManager.cpp:173-174
   then returns false for Absolute + SyncNudge. Result: the MIDI-CC half of F10(a) ("endless-encoder CCs nudge 1 ms per tick") works
   only from a hand-edited bindings file ("ccMode": 1); through the overlay Boris gets a bound-looking control that is dead,
   with no message (correct under his no-text rule, so the fault is silent). The lens line "a nudge from a MIDI CC (relative mode
   included) reaches the dial" holds at the engine and in the unit test, not for a user. The builder disclosed this (lane report
   STOP ITEM 1) and did not improvise; it still ships dead and Harmony has not ruled it.
   FIX (for Harmony to rule, a builder to do): when MidiLearnOverlay learns a CC for Action::SyncNudge set `b.ccMode = Relative`.
   The builder's objection (a fader would then nudge by value-64) costs nothing real: an Absolute CC on this action is ignored
   anyway, so Relative is the only mode in which a learned CC does anything. Add a test of the learn path (or move the choice
   into a pure helper the test calls), and update performance-controls.md. Alternative: rule (b) "keys and notes only" and delete
   the CC claims from the doc and the two overlay targets' CC reach.

## SHOULD
2. [VERIFIED] docs/claude/performance-controls.md:41 says a Relative CC "moves it 1 ms per tick" and never says no UI creates a
   Relative binding (see MUST 1). Same overclaim in APP-INVENTORY.md "22 actions" line. Fix with the ruling on 1.
3. [VERIFIED] test_binding_sync_nudge.cpp:166-170 re-implements the handler's case body in a lambda; the real
   MainComponent.cpp:7939-7944 case is not driven. A mutant there (wrong sign, missing nudge call, `syncOffset_` not called)
   passes all 6 cases. Mitigated: the rule `syncNudgeDeltaMs` is the shared pure function and the body is 2 lines; MainComponent
   is not unit-constructible. Accept, or add a source-lint case that the handler's SyncNudge case contains `syncOffset_->nudge(`.
   INFERRED for the mutant claim (not run: read-only review).

## NIT
4. [VERIFIED] Binding.h:~108-110 `ticks * step` and `-binding.targetSyncStepMs`: a hand-edited "targetSyncStepMs" of INT_MIN or
   a huge value is signed-overflow UB; fromVar (BindingManager.cpp:296-297) does not clamp, "0" makes the key dead. The dial itself
   clamps (SyncVenues::nudge -> clampMs, SyncVenues.cpp setMs), so the only exposure is the UB on a corrupt file. Clamp the step to
   +-500 in fromVar if wanted. Builder already noted it as not ruled.
5. [INFERRED] handleBindingAction dereferences `syncOffset_` (MainComponent.cpp:7944) unguarded; it is created at :1831, after the
   binding callback (:1812) and midiHandler_->start (:~1824). Safe because MIDI is marshalled with callAsync onto the message
   thread and the constructor runs there; REST/OSC callbacks (:2322, :2442) deref the same way. No change needed.

## CONFORMANCE (item by item, lens keys)
- Targets added the house way: SyncNudge appended after TriggerRoutine (Binding.h:46-48), asserted == TriggerRoutine+1
  (test_binding_sync_nudge.cpp:71; also test_routine_engine.cpp:761 still pins TriggerRoutine == MasterSignal+1). Capture + find-
  existing cases beside TriggerRoutine in BindingOverlay.cpp:198/296 and MidiLearnOverlay.cpp:234/282/344. [VERIFIED]
  BindingTarget.h (bf9b's TriggerClip resolver) is untouched and irrelevant; the 3 targeting modes are untouched: no hunk in
  resolveBindingTarget or its call at MainComponent.cpp:7749. [VERIFIED]
- Key / note path: processKeyDown (MainComponent.cpp:4150, message thread) -> action value 1.0 -> step; release/note-off value 0 -> 0.
  Note-on velocity > 0 -> step. [VERIFIED]
- MIDI path: MidiHandler.cpp:80-99 marshal note/CC with callAsync before bindingManager_.process*; so the action runs on the
  message thread. Nothing is written from the MIDI or render thread. [VERIFIED]
- Single writer + clamp: handleBindingAction calls `syncOffset_->nudge(deltaMs)` (MainComponent.cpp:7944), the same op as REST
  (:2323) and OSC (:2442); apply() (SyncOffsetController.cpp:70-92) -> SyncVenues::nudge -> setMs -> SyncOffset::clampMs. [VERIFIED]
- Persisted where ruled: via the controller's 500 ms-debounced settings.json "sync" key; nothing new written; binding saved
  with targetSyncStepMs in toVar (BindingManager.cpp:250), fromVar absent -> +1 (:296-297). Not a ControlPath, never in a take. [VERIFIED]
- Relative CC: raw `value - 64`, accumulator skipped (BindingManager.cpp:168-177). Absolute ignored (:173-174). [VERIFIED]
- Test drives real code and can fail: real BindingManager entry points, real SyncOffsetController over a temp settings.json
  (:59-66, :161). Lane-report RED lines map exactly onto this file's lines (:84-:89, :92, :106-:109, :123, :139-:152, :192-:213);
  I did not run it (read-only). The 55 vs 54 assertion count is explained (a REQUIRE inside the action callback runs once per fired
  action; the stub fired one extra for the Absolute CC). Each key assertion has a mutant it catches: accumulator left on (:92),
  Absolute not ignored (:106-:109), fromVar guard removed (:132, absent loads 0 not 1), sign/step (:87-:89,:139-:153). [VERIFIED reading; RED/GREEN itself INFERRED from the report]
- No on-screen event text: the diff adds only the two target labels "Sync -1 ms" / "Sync +1 ms" (MainComponent.cpp:7646), which
  name bindable controls inside the bind / MIDI-learn overlays; `nudge` returns a Result that the handler discards; no label,
  log line or repaint text announces a nudge. grep of src/ui and MainComponent.cpp for new announcing strings: none. [VERIFIED]
  The overlay row is 8 targets wide (868 px), same width as the 8-pad routine row; the report's STOP ITEM 2 (nobody has looked
  at the two targets in a capture) stands for Harmony's visual gate.
- Docs: performance-controls.md:41 (SyncNudge), analysis.md:95/:169 (pointer + guard list), architecture.md:263, APP-INVENTORY 22
  actions / 1330 tests. The heading "Binding System Extensions" exists (performance-controls.md:32). CLAUDE.md is not in the diff. [VERIFIED]
  Claims in the SyncNudge bullet match the code except the reachability gap (SHOULD 2).
- Nothing stray: 13 files, all in S4's scope; no build dirs / scratch in the diff.

SLIM: no EXCESS_DEAD / VESTIGIAL / DUP / SPEC in the diff. The end-to-end test case (:156-216) is the builder's own SLIM note:
JUSTIFIED_KEEP reason="only case that runs key, note, encoder and absolute fader through a real controller". Pre-existing smell the builder
saw (three capture blocks copy the same target fields; find-existing switch duplicated): DEBT_FILED by the lane report, not this diff's.

SUMMARY: 13 files, 5 issues (1 blocking, 2 suggestions, 2 nits). Confidence: VERIFIED by reading git objects at the pinned head; no build, test or probe run (not permitted).
METADATA: reviewer=reviewer-keys-r1, date=2026-10-03
