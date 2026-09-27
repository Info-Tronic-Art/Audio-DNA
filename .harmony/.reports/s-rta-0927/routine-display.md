# routine-display (lane R, s-rta-0927) — BUILDER REPORT

STATUS: DONE_WITH_CONCERNS

RESULT: The adopted plan's slice A is built on `lane/routine-display-0927`: seven plan commits (C1-C7) plus this
report commit, all on top of main 4b0c39a. The deck has a ROUTINES row of eight pads above the column numbers.
A routine's name is banded on every layer it plays, with an x that removes it. The layer X also clears routines.
The V/S faders now follow the model, and the V fill (and, from C7, the inspector digits with "ROUTINE") turns cyan
under a routine's hand. The pad menu has Loop/Once, Restore first/Start from now, Start: Ease/Jump, Quantize,
Rename..., Remove from layers, and Delete routine behind a confirm. The Record tab's pad row is gone.
Full serial ctest: 689/689. The live gate is RED 3/9 on the pre-change app and GREEN 12/0 on the hook-free lane
build. The existing probes are unchanged and green: routines 98/0, deck-tabs 6/0, manual-bpm 22/0. Not merged,
not pushed.

FACTS (disk-cited):
- Commits (`git -C .claude/worktrees/rta0927-w3 log --oneline main..HEAD`): ee887e2 C1 engine · 192d486 C2 view
  model · fcbf57d C3 strip · 1d7972a C4 pads · 4f6c914 C5 record tab · c9a5881 C6 probe+docs+shots · 5eb3735 C7 knob
  cue · (+ this report/notebook commit). Base = main 4b0c39a.
- ctest (final tree, `ctest --test-dir build-lane`, serial): `100% tests passed, 0 tests failed out of 689`;
  `ctest -N` → `Total Tests: 689`.
- Per-commit REDs (each recorded before the fix; full text in the commit messages):
  - C1 (compiled against `git archive main src`): 15 × `no member named 'deck' in 'RoutineEngine::Status::Slot'`,
    21 × 'layers', 15 × 'fireSeq', 12 × 'startsOn', 12 × 'restartPending', 3 × 'touchesComp', and
    4 × `no member named 'stopOnLayer' in 'RoutineEngine'`.
  - C2: `fatal error: 'ui/RoutineDeckView.h' file not found`.
  - C3, a running RED on main's LayerStrip: `REQUIRE( strip.findChildWithID("layerOpacity") != nullptr ) with
    expansion: nullptr != nullptr`. Plus a compile RED: 8 × `no member named 'syncFromModel'`.
  - C4: `fatal error: 'ui/RoutinePad.h' file not found`.
  - C5, a running RED: `CHECK( p.findChildWithID("routinePad" + juce::String(i)) == nullptr )` failed 8 times —
    `assertions: 12 | 4 passed | 8 failed`.
  - C7, a running RED: `CHECK( cyanPixels(lane, digits) >= 10 ) with expansion: 0 >= 10` —
    `assertions: 8 | 5 passed | 3 failed`.
- Live gate `.harmony/probe-routine-display.sh` (scratch logs live-b1..b5.log), each summary line verbatim:
  - pre-change `build/` app: `3 PASS / 9 FAIL`. It fails d1 d2 d3(layers) d4 d5 d6 d8 d9 d9, e.g.
    `FAIL  d8 pad 3 warning persists: running 1, after the end ('idle', 0)` and
    `FAIL  d9 03-playing cyan bands (cyan 585 -> 585, need > +300)`.
  - lane build, hook-free: `12 PASS / 0 FAIL`.
  - hook build (Phase 1 + 2): `20 PASS / 0 FAIL`, including `PASS  d7 layer X on L1: Drop and Build (both touch L1)
    idle ('idle', 'idle'), L1 clip cleared (-1)`.
- Existing probes on the final lane build, re-run with no re-thresholding:
  - `probe-routines.sh` (ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8): `98 PASS / 0 FAIL`.
  - `probe-deck-tabs.sh`: `6 PASS / 0 FAIL`.
  - `probe-manual-bpm.sh`: `22 PASS / 0 FAIL`.
- Hook hygiene: the hook patch (sha256 4b55ed59…4f54) was applied for batch b2 only, then removed with
  `git apply -R`. After the rebuild (binary mtime 10:56, after the revert):
  - `strings build-lane/.../MacOS/Audio-DNA | grep -c AUDIODNA_DEBUG_` → `0`. The hook build read 5.
  - `git diff --stat -- src/MainComponent.cpp` → empty.
  - `git diff main..HEAD -- src/MainComponent.cpp | grep -c AUDIODNA_DEBUG` → 0.
- Fence check: `git diff --stat main..HEAD` has no `src/output/*`, `OutputWindow.*`, `Renderer.*`, `src/api/*` or
  `.harmony/probe-routines.sh` changes. `git diff main..HEAD -- src/MainComponent.cpp | grep -c outputWindow_` → 0.
  MainComponent hunks are exactly: the layer-X line, the wiring block after `dispatch.notify`, the three deleted
  Record-panel lines, the `timerCallback` tail, the `routineStatusVar` keys, and `renameRoutine`/`deleteRoutine`
  after `perfRoutineRemove`.

METHOD: Read the plan in full, plus design-final (+ADDENDUM), the mockup PNGs and every cited source region.
Per commit: write the tests first, record the RED (a compile RED against `git archive main src`, or a running RED
from a stub built before the source change), implement, build, run the full serial ctest, then commit with the RED
text. Shared `tests/CMakeLists.txt` blocks were staged per commit through `git hash-object` + `update-index`.
Live runs went through one wrapper: `/tmp/audiodna-live.lock` taken with a 20-s poll, `.venv` symlinked only for
the run, `AUDIODNA_LOCK_OWNER=routine-display`, `open -g` only, and the lock released each batch. The lock was held
at 10:49:58-10:51:25, 10:53:13-10:55:04, 10:57:39-10:59:03, 11:01:39-11:03:21 and ~11:05-11:06:25; the first wait
was 2560 s behind the outputs-c1 and routines-timing lanes. Captures are window-only, by Quartz window id. Every
kept PNG was opened and compared with the mockup.

CONFIDENCE + VERIFY: HIGH for the engine/status layer and the view model (unit-pinned and live-witnessed over
REST), and for what each shot shows (opened). MEDIUM for real-hand interaction, which no probe drives (no synthetic
input): a real left-press/right-click on a pad, clicking a menu item, the band x, the Rename and Delete dialogs.
Only the component-level synthesised mouseDown (ctest) and the hook's direct calls cover them.
VERIFY (Harmony gate):
- rebuild, then `ctest --test-dir build-lane` = 689/689;
- `.harmony/probe-routine-display.sh <out>` (ROUTINE_DISPLAY_BUILD_DIR=build-lane) = 12/0, and the same probe on
  the pre-change app = RED;
- a Tier-4 human pass: press pad 1, right-click it and flip Start: Jump, press a band x, press the layer X, check
  the Rename and Delete dialogs.

UNKNOWNS / NOT DONE:
- Slice B-D items (key/MIDI tag on pads, "What it moves", the band right-click menu, the steady named knob cue,
  band fade-out) are not built, per the plan.
- tests/visual Tier-1 was not run. No shader, source or effect changed.
- No perf numbers taken: the change is message-thread UI and one status copy per 30 Hz tick.
- The "skipped while dragging" guard in `syncFromModel` has no test (a real mouse source is needed). It is covered
  by review only.

NUANCE:
- Deviations from the plan's letter:
  - (1) The fixture's Drop carries one recorded L1-opacity move over beats 8..28, where the plan had `lanes: []`.
    Without it no lane-rank grip exists at bar 5, so the cyan V/inspector cue could never be shot.
  - (2) Row d2 asserts `startsOn == "4bar"`, not "bar": the waiting shot's 4-bar quantize trick makes 4bar the
    effective grid.
  - (3) C5 kept the Save Routine notice. `applyRoutines` became `applyRoutineNotice` with only the notice expiry.
    Deleting its whole body, as the plan said, would have dropped the notice the plan says to keep.
  - (4) The lane report is committed after C7, in its own commit, not inside C6.
  - (5) Per-commit ctest ran on the working tree at that point. For C1-C4 it excluded the pre-registered RED
    targets of later commits (C3 stub, C5), noted in each commit message.
- Evidence provenance:
  - The C6 shots came from builds that already held C7's UniversalParamControl change (the C7 source was in the
    tree). Only the Layer-tab shot shows it.
  - The "before" Record tab is a headless render from main's `build/tests/tool_routine_strip_snapshot`. The
    pre-change app has no tab hook.
  - The 10-pad-menu PNG is an in-app `createComponentSnapshot`. A PopupMenu dismisses within ~50 ms in a
    background app.
- Rank-based cue (disclosed by the design): it also lights for a take replay and blinks between a routine's
  gestures.
- The pixel oracle decodes the window's deck region with colour-managed values (notebook entry).
- Undo of a layer X restores the clip only. Routine run state is not undoable, like Stop.
- Stale comments still name the retired `tool_routine_strip_snapshot`: `tests/test_topbar_link_toggle.cpp:63` and
  a `tests/CMakeLists.txt` comment near `tool_uitoggle_snapshot`. Left alone (outside the fence).

HANDOFF-NEEDS: Harmony's behavioral gate plus an independent Reviewer. Merge order with lane O: both touch
`MainComponent.h/.cpp`, `CMakeLists.txt` and `tests/CMakeLists.txt`, in disjoint hunks (new test targets sit after
`test_routine_bank_model`, never at EOF). Whoever merges second rebases and re-runs ctest.

INBOX-RECHECK: none

---

## SUMMARY
Slice A of the routine display, built exactly on the adopted plan's fence and commit order. The engine gained
additive status fields and `stopOnLayer`. A pure juce_core view model drives the pads, bands, corner note and menu.
The Record tab lost its stop-in-disguise pad row. The fader-follow bug is fixed.

## FILES CHANGED
- `src/recording/RoutineEngine.h/.cpp`:
  - new Slot fields deck / layers / touchesComp / restartPending / fireSeq / startsOn, computed once at fire from
    resolved targets;
  - `stopOnLayer`;
  - `lastForcedSnap_`;
  - a LastRun warning memory, cleared by stopAll.
- `src/ui/RoutineDeckView.h` (new): `deriveRoutineDeckView`, `bandsToDraw`, `padMenu`, `settingsChangeFor`.
- `src/ui/RoutinePad.h/.cpp` (new): custom-painted pad. Press = fire, right-click = menu, no stop callback.
- `src/ui/DeckView.h/.cpp`: the ROUTINES row, `setRoutineView`, band fan-out, `showRoutinePadMenu`, callbacks,
  `getNaturalHeight` +22.
- `src/ui/LayerStrip.h/.cpp`: `syncFromModel` (V/S follow, cyan cue), slider ids, trackColour fill, band
  paint/hit test, `setRoutineBands`, `onRoutineRemove`.
- `src/ui/RecordPanel.h/.cpp`, `src/ui/RoutineBankModel.h`: pad row removed; Save Routine row and notice kept.
- `src/ui/UniversalParamControl.cpp` (C7): cyan digits + ROUTINE hint under a lane-rank grip.
- `src/MainComponent.h/.cpp`: the fenced hunks listed in FACTS.
- `CMakeLists.txt`: `src/ui/RoutinePad.cpp` added after `DeckView.cpp`.
- Tests:
  - new: test_routine_deck_view, test_layer_strip_follows_model, test_routine_pad_press,
    test_record_panel_pads_removed, test_param_control_routine_cue, tool_routine_deck_snapshot;
  - edited: test_routine_engine (D1-D5), test_routine_bank_model (5 clamp cases kept);
  - retired: tool_routine_strip_snapshot.
- `.harmony/probe-routine-display.sh/.json` (new).
- Docs: CLAUDE.md, docs/claude/{pitfalls,recording,performance-controls}.md, .harmony/APP-INVENTORY.md,
  .harmony/notebook.md.

## TESTS
- Full serial ctest: 689/689.
- New cases:
  - RoutineEngine display D1-D5;
  - RoutineDeckView 10 cases;
  - LayerStrip follows-model 6 cases (29 assertions);
  - RoutinePad press 5 cases (+ HasStopCallback static_assert — teeth checked on a scratch struct with onStop);
  - RecordPanel pads removed (12 assertions);
  - param-control routine cue (8 assertions).
- Live: see FACTS.

## UI EVIDENCE (critic panel) — `.harmony/.reports/s-rta-0927/routine-display-shots/`
All paths are absolute under `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w3/.harmony/.reports/s-rta-0927/routine-display-shots/`.
- before-/after- window captures, one per state: 08-empty, 01-idle, 02-waiting, 03-playing, 04-two-on-one,
  05-offdeck, 06-removed, 09-warning, 12-fader-follow.
- after-only (hook build): after-07-layer-x-before, after-07-layer-x, after-10-pad-menu-snapshot,
  after-11-record-tab, after-03-playing-inspector.
- before-11-record-tab-padrow-headless (main's old tool).
- crop-{before,after}-*-routines-row / -deck (from the 2x captures).
- crop-after-04-two-on-one-L1-strip-bands, crop-after-03-playing-L3-strip-band.
- tool-t1..t7 headless row/strip renders.
- State 12 (TopBar) was already shipped; not in this lane.

## ISSUES
- `build-lane/` is untracked (kept by instruction); `git status` is otherwise clean.
- A probe from another lane, `.harmony/probe-manual-bpm.sh`, is mode 644. Run it with `bash`.

## SKILL_PROPOSALS
none

## RISKS
- +22 px of deck height costs the bottom panels 22 px at 1280x800. Checked visually in the shots: the preview and
  inspector are still readable.
- Pad N sits over column N — a Tier-4 confusion check (design risk).
- A band covers the top 16/32 px of the thumbnail.

## METRICS
- 8 commits.
- Live batches: 5; lock held ~9 minutes in total.

## KNOWLEDGE CONTEXT
- Tools used: grep. Impact authority: grep (not authoritative) — conservative posture: nothing was deleted except
  the plan-named Record pad row, RoutineBankView and the retired tool. Every user was grep-verified first.
- Risk level: NORMAL.

## PACKET QUALITY
- Clarity: CLEAR.
- Missing context:
  - window captures are colour-managed (found live);
  - `applyRoutines` also owns the Save Routine notice.
- Unused context: none.
- Self-brief files: plan, design-final, mockup shots, notebook (hook entries) — all useful.

## STATUS
DONE_WITH_CONCERNS. The concerns are the disclosed deviations (1)-(5) and the MEDIUM coverage of real-hand
interaction.

## NEXT ACTION
Harmony gate + Reviewer. Tier-4 by Boris.
