# Reviewer Verdict - bf2 S4b keys, round 1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) - no MUST
REVIEWED: lane/bf2-keys d4653bd (base 68abc16), read through git objects only; no build, no test, no probe run.
FILES: src/binding/Binding.h, BindingManager.{h,cpp}, src/ui/MidiLearnOverlay.{h,cpp}, src/MainComponent.cpp,
 src/api/ApiServer.{h,cpp}, tests/test_binding_sync_nudge.cpp, .harmony/probe-sync.{py,sh}, APP-INVENTORY.md,
 docs/claude/performance-controls.md, docs/claude/testing-eyes.md, the S4b report.

## Answers to the seven asks
(1) NO screen can make a dead Sync binding. Creation / load paths at d4653bd (VERIFIED by git grep addBinding / fromVar /
    loadFromFile / setBinding*): BindingOverlay.cpp:200 (inputType Keyboard only -> always live); MidiLearnOverlay.cpp:293
    (MidiNote -> live); MidiLearnOverlay.cpp:332 -> BindingManager::learnMidiCC (BindingManager.cpp:12-27, bindingIsLive
    first, before any removal); BindingManager::fromVar via MainComponent.cpp:7389 Import Bindings (file: HD5, loads inert,
    Relative entries are live). ApiServer/OSC/MidiHandler never add a binding. The overlay always builds ccMode Absolute,
    so every learned CC on a Sync target is refused. Residual (RULED, HD5, INFERRED visible): a hand-made file's Absolute-CC
    Sync entry loads, paints a "CC n" tag on the Sync target (MidiLearnOverlay.cpp bindingTag) and does nothing.
(2) VERIFIED (MidiLearnOverlay.cpp:317-336): refused CC sets lastMidiMessage_ (the standing "Last:" readout), builds b,
    learnMidiCC false -> waitingForMidi_ / selectedTargetIndex_ untouched; nothing removed (T2 + mutant M2). The only
    on-screen change is the standing "Last: CC .." echo; no refusal text. titleText() on a Sync target = "Send a MIDI
    note..." (a state, HD4).
(3) VERIFIED: all 3 registrations inside the existing #if AUDIODNA_TEST_SERVER (ApiServer.cpp:368-378 block, handlers
    inside 2872-3129); MainComponent wiring inside the existing test-server block; ApiServer.h and MidiLearnOverlay.h
    declarations under #if; the flag is set only by AUDIODNA_BUILD_TEST_SERVER (CMakeLists.txt:490). binding_action ->
    real MainComponent::handleBindingAction; midi -> overlay's own handleIncomingMidiMessage; select -> selectAt (= mouseDown's
    body); open/close -> enterMidiLearnMode (only when state differs). Not re-statements.
    "Production build compiles none of it" is READ, not built (builder F5). The 400 answers were never run (builder says so).
(4) VERIFIED: route -> callAsync -> handleBindingAction (MainComponent.cpp:7975-7981) -> syncNudgeDeltaMs ->
    SyncOffsetController::nudge -> apply -> SyncVenues::nudge -> clampMs; real MIDI also reaches it on the message thread
    (MidiHandler.cpp callAsync). R13 steps 8/9 drive the clamp both ends.
(5) T1,T2 RED on the ruling's stubs, raw lines match scratchpad/bf2-S4b/unit-red-stub.run.log (4 of 11 failed, :226, :253,
    :257 + the two S4 cases) and mutants.log (M1 4 RED, M2 1 RED, RESTORE green, 0 MUTANT markers). Cases drive real
    BindingManager. 11 cases at head = 6 + 5 (VERIFIED by grep). 1335 = 1330 + 5 is INFERRED (ctest not run here).
(6) VERIFIED: pins - route registrations 75 -> 78, /api/debug 27 -> 30 (git show counts at both commits); doc claims
    (T2 "left alone", T5 "any velocity", T6 "each key-down one step", paint's "Last:" line) each match code. No stale
    "learned onto Sync does nothing" text left in live files (git grep).
(7) VERIFIED: paint() refactor (bindingTag, labelFont) is output-identical; only visible change is the Sync-target top
    line. No new event/failure text. Targeting modes, addBinding, fromVar untouched (diff). No MUTANT in src/tests at head;
    no build dir / .venv in the tree (ls-tree). probe-sync.sh quit logic untouched (quit_ours).

## Findings
S1 SHOULD  The ruling's G7 ML-1 bar "every label_w <= w - 4" FAILS on today's tree for 15 pre-existing targets (builder F1;
   VERIFIED by me on out-green/probe-sync.json: 80 targets, 15 bad, e.g. L3 Bypass 45 vs 32). Not an S4b defect; a
   defect in the pre-registered gate row. Harmony must re-rule the bar before S5b (a builder may not adjust it).
S2 SHOULD  G1-RED depends on SP/mutants.py in a session scratchpad (outside the repo); the report states M1/M2 only in
   words. Put the two edits as text in the hand-over or copy the script beside the report so the RED is re-runnable.
S3 NIT     T3, T5, T6 pass on the stubs (RED-first is true for T1, T2 only). Honest in the report, A13 says "no code" for
   T5/T6, and M3-M5 show their bite - but M3-M5 are not in G1-RED, so a future change to them is unguarded by a named RED.
S4 NIT     R14's control-arm clause only changes the message (the row already fails when b fails: passed < 8). Harmless;
   no selftest case covers r14/r13 pure helpers (live REDs only, as ruled).
S5 NIT     F2 (builder): the standing title literal holds a UTF-8 em dash in a narrow string -> mojibake in the new dump and
   probably on screen. Pre-existing text moved into titleText(); fix is a one-liner but out of S4b scope.
S6 NIT     R14 leaves the overlay open and bindings in memory if a state before g throws/aborts the row; session-local only.

## Ruling conformance (A10-A16, section 5 R13/R14): all items present (predicate, learnMidiCC, shared guard, overlay
block, titleText, selectAt, stateForTests, 3 routes, 9-step R13, 8-state R14 with control arm, mutant RED lines, docs, pins).
SLIM: no excess found; bindingIsLive-false arm in processMidiCC is JUSTIFIED_KEEP (HD5 file entries reach it).
Confidence: VERIFIED for code reading and raw-log match; INFERRED for compile/ctest/live results.
