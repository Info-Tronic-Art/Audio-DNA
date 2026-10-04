# Reviewer Verdict - bf2 S4b, lens gates, round 1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) -- 0 MUST, 2 SHOULD, 3 NIT
PINNED: worktree .claude/worktrees/bf2keys, lane/bf2-keys, base 68abc16, head d4653bd. Read through git objects only.
FILES: .harmony/probe-sync.py, .harmony/probe-sync.sh, src/api/ApiServer.{h,cpp}, src/MainComponent.cpp (2352-2390),
 src/ui/MidiLearnOverlay.{h,cpp}, src/binding/{Binding.h,BindingManager.{h,cpp}}, tests/test_binding_sync_nudge.cpp,
 docs/claude/{performance-controls,testing-eyes}.md, .harmony/APP-INVENTORY.md, .harmony/.reports/s-rta-1003b/bf2-s4b.md

## Answer
R13 and R14 are built exactly as ruling-bf2-stops section 5 pre-registers them and can fail on every case the lens names.
No MUST. The live runs (GREEN on the lane app, RED on build-mut-r13) are the builder's report only; I did not run any app
(INFERRED from the report); I did run the probe's own row functions offline against stub http simulators (VERIFIED, below).

## Row R13 (probe-sync.py:1245-1300 at d4653bd)
- Steps, order, inputs, values, wanted targetMs and "still" flags = the ruling's table 1-9 exactly (R13_STEPS 1246-1256). VERIFIED by reading.
- PASS / FAIL strings verbatim: "R13 the handler case moves the dial: %d of %d steps"; "R13 step %d: targetMs %s, want %d". VERIFIED.
- Start: set_sync(0) + settled; end: set_sync(0) + settled (1291-1292), main() sets it again before R6 (probe-sync.py:1423). Dial restored.
- Offline repro (my rig in $TMPDIR, stub http with a model dial, the REAL row_r13): correct handler -> PASS 9 of 9;
  sign flipped -> "FAIL R13 step 1: targetMs -1, want 1" (+7 more); no call at all -> "FAIL R13 step 1: targetMs 0, want 1";
  a release that nudges -> "FAIL R13 step 2: targetMs 2, want 1". VERIFIED (logic; the app side is the builder's live RED).
- A non-200 from the route is a FAIL naming "not a TEST-SERVER build" (not a silent skip). VERIFIED.
- Steps 3, 7 (and 2 on a vacuous handler) can pass vacuously on a dead handler, but step 1 fails first; the ruling accepts this shape.

## Row R14 (probe-sync.py:1303-1368)
- 8 states 0,a..g: POST bodies, wanted fields, strings ("Master Signal", "Sync +1 ms", "Send a MIDI note or CC...",
  "Send a MIDI note...", "CC 21 val=100 ch=1", "CC 21 val=101 ch=1", "CC 22 val=5 ch=1", "Note 36") = the ruling's table. VERIFIED.
- Control arm b: d and e print "not evaluated" and the row FAILS when b is bad or its route answered non-200 (1337-1349).
  Offline repro (stub overlay simulator, REAL row_r14): injection dead (CC never reaches the overlay) -> b FAILs 4 fields,
  d / e "not evaluated", row FAIL, no PASS line; learn that binds the CC on Sync -> d / e / f FAIL; title unchanged on a Sync
  target -> "FAIL R14 state c: title "Send a MIDI note or CC...", want "Send a MIDI note..."". Correct model -> "8 of 8 states". VERIFIED.
- The route orders correctly: midi -> overlay callAsync from the HTTP thread, the GET is a later callAsync on the same
  FIFO (ApiServer.cpp handleDebugMidiLearnState); open / close / select block until done (2 s). VERIFIED by reading.
- "title" and the painted top line both come from titleText() (MidiLearnOverlay.cpp:87, 118, 153); "binding" and the
  painted tag both from bindingTag() (:98, :122, :195). The dump cannot drift from paint(). VERIFIED.

## Hygiene
- probe-sync.sh diff = comments + the two default row lists only; teardown is the existing quit_ours (own pid only, a
  foreign Audio-DNA named and left); the Output-window count and the settings.json sha256 (R6) checks are unchanged. VERIFIED.
- Bindings are never auto-persisted: the only bindingManager_.saveToFile / loadFromFile calls are the Export / Import menu
  items (MainComponent.cpp:7374, 7389), so R14's in-memory CC 21 / Note 36 bindings cannot reach settings.json. VERIFIED by grep.
  The report's sha256 before == after (af6fc014...) is the builder's live line (INFERRED).
- No MUTANT marker in src/tests at the head (git grep: none); .gitignore already carries /build-mut-*/ and .venv/ (not changed
  by this diff); no .venv link in the tree. Report: post-revert git diff --quiet rc=0. VERIFIED (tree), INFERRED (the rc).
- Routes: 78 registrations / 30 /api/debug (git grep -c) = report = APP-INVENTORY = testing-eyes. VERIFIED. All new
  declarations / registrations / handlers / wiring sit inside #if AUDIODNA_TEST_SERVER (read; no production build made).
- Pins: ctest -N 1335 = APP-INVENTORY.md:31 = 1330 + T1, T2, T3, T5, T6 (named in report item 1 and in the inventory line);
  the five cases are in tests/test_binding_sync_nudge.cpp:221-330 and drive the real BindingManager / syncNudgeDeltaMs. VERIFIED by reading; the count INFERRED.
- Ruling conformance: A10 (one predicate bindingIsLive; learnMidiCC refuses before removing; addBinding / fromVar untouched,
  HD5), A11 docs (held key repeats; Relative entry 64-offset only; Absolute entry ignored; nothing says "the encoder case
  works"), A12 titleText, A13 T6, A14 route + R13 (ccMode dropped), A15 route pair + seams (selectAt, stateForTests, TEST-SERVER
  only), A16 pins. All present. VERIFIED.
- No on-screen text announcing an event or failure added. The only visible change is the waiting title on a Sync target,
  a prompt, admitted by A12 / HD4. VERIFIED.

## Findings
SHOULD-1 No offline SELFTEST case covers R13 / R14 (probe-sync-selftest.py not in the diff). The ruling does not require it
 (section 5 SELFTEST line lists only D0 / R7r cases) and the live REDs cover the sign flip and the refused-learn mutant, but
 the control-arm-skip branch (probe-sync.py:1337-1341), the title check (state c) and the R13 "still" steps are exercised by
 no recorded run (the live mutant RED had b passing). I exercised them with stub simulators (above) and they behave. A
 permanent selftest case would keep them honest when D0 / R7r edit the same file. Fix: add stub-http cases (my rig:
 $TMPDIR/s4b/rig.py, rig13.py) to probe-sync-selftest.py in a later probe stage.
SHOULD-2 Builder's F1 stands and blocks S5b, not this stage: G7's ML-1 bar "every label_w <= w - 4" is false on 15
 pre-existing 32-px layer buttons (report F1; INFERRED, from the builder's dump). Harmony must re-state the bar before S5b; a builder may not adjust it.
NIT-1 T3, T5, T6 are GREEN on the ruling's stubs (they pin existing behaviour; A13 says "No code"); the builder disclosed it and
 added scratch mutants M3-M5 that make them RED. Accepted; "RED before" holds for T1, T2 and two S4 cases only.
NIT-2 The overlay's MIDI entry is called on the HTTP thread through midiLearnOverlay_ (MainComponent.cpp:2379-2386). Safe today
 (the member is set once at construction and the call only posts); under a shutdown race it is a stale pointer. Test-only; no action.
NIT-3 R13 / R14 have no subset knob, so the "subset is labelled" rule does not apply; a row list such as R0,R13,R14,R6 is the
 ruling's own gate command. Nothing to do.

## Not run (by instruction)
No build, no ctest, no app, no probe against an app. The only code I executed is the probe's row functions against stub http in $TMPDIR (no network, no app).
