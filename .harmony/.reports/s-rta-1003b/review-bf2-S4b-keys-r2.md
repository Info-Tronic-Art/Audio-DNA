# Reviewer Verdict - bf2 S4b keys, round 2
STATUS: DONE
VERDICT: PASS_WITH_NITS (APPROVE) - no MUST
REVIEWED: lane/bf2-keys head 9eab9bdbb7156e9c1a54bb6b5ef62ab3c56847dd (base 68abc16; fix round d4653bd..9eab9bd), read through git objects only;
no build, no test, no probe run. Labels: VERIFIED = read / grepped by me at the head; INFERRED = not executed here.
FILES: .harmony/probe-sync-selftest.py, .harmony/.reports/s-rta-1003b/bf2-s4b-selftest-mutants.py, bf2-s4b-unit-mutants.py, bf2-s4b.md
(the fix round touched ONLY these 4; `git diff --stat d4653bd..9eab9bd -- src tests docs CLAUDE.md .harmony/APP-INVENTORY.md` is empty, VERIFIED);
whole lens re-read at the head: Binding.h, BindingManager.{h,cpp}, MidiLearnOverlay.{h,cpp}, BindingOverlay.cpp, ApiServer.{h,cpp} (S4b hunks),
MainComponent.cpp (S4b hunk + SyncNudge case), probe-sync.{py,sh} rows R13/R14, tests/test_binding_sync_nudge.cpp, docs, APP-INVENTORY.

## Round-1 MUSTs
Round 1 (review-bf2-S4b-keys-r1.md and the gates r1) had NO MUST. The fix round answers its SHOULD/NIT items:
- keys S2 (G1-RED outside the repo): FIXED as found. bf2-s4b-unit-mutants.py is in the repo beside the report; the 5 edit texts
  (M1-M5) match src at the head exactly once each (VERIFIED: Binding.h:119-124 bindingIsLive body, BindingManager.cpp:12-27 learnMidiCC
  order, the syncNudgeDeltaMs `value > 0.0f` line, the key-down `actionCallback_(b, 1.0f)` site); revert in `finally`, sha re-check,
  restore rebuild must compile >=1 object and run green, MUTANT-marker sweep; exit 1 on any NOT RED. M3-M5 now named (S3 closed).
- gates SHOULD-1 (no offline case for R13/R14): FIXED as found. 15 cases at probe-sync-selftest.py (head) drive the probe's REAL row_r13 /
  row_r14 (+ real set_sync / get_sync / wait_settled) with only http() and the clock replaced; both restored in `finally`.
  42 `case(` lines at the head = 27 + 15 (VERIFIED by grep at d4653bd and head). RED: the probe at 68abc16 has no row_r13 (AttributeError,
  RED by absence) plus 8 mutated copies K1-K8 (bf2-s4b-selftest-mutants.py: copies in a temp dir, committed probe sha re-checked; each K's
  old-text must occur once; each must DIFF the named case).
- keys S1 = gates SHOULD-2 (G7 ML-1 bar false on 15 pre-existing 32-px buttons), keys S5 (em-dash mojibake), keys S6: NOT fixed, and rightly:
  a pre-registered gate row / a visible text / a NIT. Recorded as stop items for Harmony in the report (STOP ITEMS 1-4). Not a builder's to move.

## Hand-verified self-test counts (I recomputed the model by hand, not by run)
R13 flip (1 pass / 8 fail), nocall (3/6), release (3/6), late (3/6), noclamp (7/2), 404 (0/9); R14 dead (4/7), binds (5/10), strips (5/3),
leaves (5/5), nolast (6/2), title (7/1), 404 (0/8): every stated (passes, fails) and R13P/R14P value matches my trace of the model
against the probe's row code (VERIFIED by reading; the builder's run lines "0 case(s) differ" are INFERRED). Mutant K1 really removes only
the sleep (the next line still reads the dial), K4 removes the control arm, K6/K7/K8 bite the cases named: each named case's counts change.

## The seven asks at the head
(1) NO screen can make a dead Sync binding. Creation/load paths, git grep addBinding|learnMidiCC|fromVar|loadFromFile|ccMode= at the head
    (VERIFIED): BindingOverlay.cpp:200 (inputType Keyboard -> live); MidiLearnOverlay.cpp:293 (MidiNote -> live); MidiLearnOverlay.cpp:332
    -> BindingManager::learnMidiCC (BindingManager.cpp:12-27) -> bindingIsLive (Binding.h:119-124) BEFORE any removal or add; the only
    other entry is BindingManager::fromVar via loadFromFile (MainComponent.cpp:7389, Import Bindings), left as today by HD5: a hand-made
    Absolute-CC Sync entry loads, does nothing, saves back (processMidiCC:186-196 arm reads the same predicate). No code writes
    `ccMode =` or `action =` on a stored binding except fromVar (BindingManager.cpp:301-303); the ApiServer.cpp:2927 `binding.action =` builds a
    Binding VALUE for the test route and never calls addBinding. The overlay always builds ccMode Absolute, so every learned CC on a Sync
    target is refused. Residual (RULED HD5): the file entry.
(2) VERIFIED (MidiLearnOverlay.cpp:317-336): a refused CC sets only lastMidiMessage_ (the standing "Last:" echo that every message updates,
    pre-existing), then `learnMidiCC(b)` false -> waitingForMidi_ / selectedTargetIndex_ untouched, nothing removed (guard is the first
    statement of learnMidiCC), no refusal text; titleText() on a Sync target is "Send a MIDI note..." (a state, HD4).
(3) VERIFIED: handlers ApiServer.cpp inside the `#if AUDIODNA_TEST_SERVER` block (2872-3129), 3 registrations inside 368-378, ApiServer.h
    members + declarations under #if, MainComponent wiring inside the existing test-server block, MidiLearnOverlay.h/.cpp stateForTests
    under #if. binding_action -> the real MainComponent::handleBindingAction; midi -> the overlay's own handleIncomingMidiMessage (the
    real learn path incl. learnMidiCC); select -> selectAt (= mouseDown's body); open/close -> enterMidiLearnMode. Not restatements.
    Pins VERIFIED by grep: registrations 75 -> 78, /api/debug/ 27 -> 30 at 68abc16 vs head.
(4) VERIFIED: route -> callAsync (message thread) -> handleBindingAction MainComponent.cpp:7975-7981 -> syncNudgeDeltaMs ->
    syncOffset_->nudge (SyncOffsetController.cpp:100) -> SyncVenues::nudge -> clampMs. R13 steps 8/9 drive both ends.
(5) T1,T2 RED on the stubs (raw lines in the report), mutants M1 (4 RED), M2 (1 RED), M3-M5 each RED, restore green: lines are in the
    report and match the script's own printing; run results INFERRED (not re-run). 11 TEST_CASEs at the head (grep) = 6 + 5; 1335 = 1330 + 5
    INFERRED (ctest not run here). T3/T5/T6 are green on the stubs (honest in the report; M3-M5 now name their bite).
(6) VERIFIED: docs claims match code (learn stays waiting, "Last:" line updates, other CC binding left alone, title text,
    64-offset coding only, Absolute file entry inert; "Shortcuts menu" -> Import Bindings at MainComponent.cpp:7389 region); APP-INVENTORY 1335
    and 78/30/48 add up. "A held key repeats at the system's rate" is the OS's behaviour, INFERRED, and ruled (A13).
(7) VERIFIED: the only visible change at the head from S4b is the top line on a Sync target (+ paint() refactored to bindingTag/labelFont,
    output-identical). No new event/failure text. Targeting modes, addBinding, fromVar untouched (diff). No MUTANT in src/tests at the
    head (git grep), no build dir / .venv in the tree (ls-tree count 0); the 4 files added/changed by the round carry no instrumentation.
    probe-sync.sh teardown is still `quit_ours` (.harmony/probe-sync.sh:55,125); the S4b diff to it is rows/usage text only. The two new
    scripts launch and quit no app.

## Findings
N1 NIT   The selftest's models are the builder's own (acknowledged as LIMIT in the report): they pin the ROW's reading, not the app. The
         live R13/R14 rows (Harmony's gate) carry the app claim. No action.
N2 NIT   bf2-s4b-unit-mutants.py edits src/binding in the worktree in place and reverts in `finally`; a SIGKILL / power loss mid-run leaves
         a MUTANT in src (the sha / marker sweep runs only on a normal exit). Mitigation exists (report says never run beside an app build);
         worth one sentence in the hand-over to Harmony: after an aborted run, `git diff --quiet -- src tests` first.
N3 NIT   R14 has no `finally` (state g `close` skipped if an earlier state raises); session-local, probe quits its own app (carried S6).
N4 SHOULD (Harmony, not the builder) G7's ML-1 bar "label_w <= w - 4" is false on 15 pre-existing 32-px L1-L3 buttons: re-state before
         S5b (carried S1; stop item 1 in the report).
N5 NIT   The standing title literal holds a UTF-8 em dash in a narrow string (pre-existing, moved verbatim into titleText()): mojibake in
         the dump, on screen INFERRED (carried S5; stop item 3).
N6 NIT   probe-sync.py conflicts on the merge into lane/bf2 (report stop item 4, keep both blocks; selftest auto-merges). Harmony's merge.

SLIM: nothing excess in the round (15 cases + 8 copies trace to the gates finding; the script to keys S2). bindingIsLive-false arm in
processMidiCC stays JUSTIFIED_KEEP (hand-made file entries reach it). No new EXCESS_*.
Confidence: VERIFIED for code/doc/test reading and the hand-traced self-test counts; INFERRED for every compile / ctest / live-row result.
