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

---

## Fix round (s-rta-0927, lane routine-display-fix) — 2026-09-27 11:20-12:20

STATUS: DONE_WITH_CONCERNS

RESULT: Both MUSTs are fixed. So are the four cheap SHOULDs: the Delete row is red, a pressed-again pad shows a
restart mark, and the band x and the layer X now have tooltips. The other five SHOULDs are left as they are. Each
is a design decision or needs Boris's Tier-4 pass; the reasons are below. The round is six commits on
`lane/routine-display-0927`, on top of f15f318, plus this report commit. Full serial ctest: 694/694. The live
probe is RED 11/4 on the pre-fix-round lane build and GREEN 15/0 on the final build. The hook build was 23/0. The
existing probes are unchanged and green: routines 98/0, deck-tabs 6/0, manual-bpm 22/0. Not merged, not pushed.

FACTS (disk-cited):
- Commits (`git -C .claude/worktrees/rta0927-w3 log --oneline f15f318..HEAD`):
  - 7656d83 F1 engine: a pending routine re-syncs its settings.
  - b28e554 F2 routine cue colour.
  - 038242e F3 red Delete row.
  - a434846 F4 restart-pending cue.
  - 6606dc7 F5 tooltips.
  - 504de9c F6 probe + shots + notebook.
  - This report commit.
- ctest (`ctest --test-dir build-lane`, serial, on the final hook-free build): `100% tests passed, 0 tests failed
  out of 694`, and `ctest -N` gives `Total Tests: 694`. The count moved 690 → 694 as each new test case was added.
- Per-commit REDs. Each was recorded before its fix; the full text is in the commit messages:
  - F1: `test_routine_engine D6` failed with `assertions: 39 | 23 passed | 16 failed`. Examples:
    - `CHECK( rig.slot(0).startsOn == "4bar" )`: startsOn stayed "bar".
    - `CHECK( rig.fd.count(Ev::Touch, op1) == 0 ) with expansion: 1 == 0`: Jump was chosen, but the restore still eased.
    - `CHECK( rig.fd.firedRestores() == 0 ) with expansion: 1 == 0`: Start from now was chosen, but the restore still ran.
  - F2, running REDs. These tests decode by hue and compile against the old source:
    - `test_param_control_routine_cue`: `CHECK( cuePixels(lane, digits) >= 10 ) with expansion: 0 >= 10` and
      `CHECK( cyanPixels(lane, whole) == 0 ) with expansion: 303 == 0`. Result: `12 | 6 passed | 6 failed`.
    - `test_layer_strip_follows_model`: the band name gave `cue 0 (without 0), accent cyan 41`. Result:
      `33 | 29 passed | 4 failed`.
  - F3:
    - A running RED in `test_lookandfeel_square`: `red text pixels: coloured 0, coloured+highlighted 0, plain 0`
      (`12 | 10 passed | 2 failed`).
    - A compile RED: `no member named 'destructive'`.
  - F4:
    - A running RED in D5: `"" == "bar"` (`12 | 11 passed | 1 failed`).
    - A compile RED: `no member named 'restartPending' in 'RoutineDeckView::Pad'`.
  - F5:
    - A running RED from a scratch copy of the case. It was swapped in and then restored byte-identical (sha256
      3ce559ed…8063). Output: `layer X tooltip: ''`, failing the clip, routine and undone checks.
    - A compile RED: `no member named 'tooltipAt'`.
- Live gate `.harmony/probe-routine-display.sh`. Scratch logs are fixlive-b1..b3.log. Summary lines verbatim:
  - Pre-fix-round lane app (f15f318's `build-lane` binary, copied aside before any fix-round build): `11 PASS / 4 FAIL`.
    - `FAIL  d13 quantize set to bar while waiting: startsOn still '4bar' (state pending)`
    - `FAIL  d14 restart pending witness: expected (True, 'bar', 'running'), got (True, '', 'running')`
    - `FAIL  d9 03-playing routine-cue bands / V fill (cue 0 -> 0, need > +300)`
    - `FAIL  d12 03-playing added accent cyan (cyan 146 -> 3798, allowed +100): the routine cue is the app's accent cyan`
  - Lane build, hook-free, 11:56: `15 PASS / 0 FAIL`.
  - Hook build (Phase 1 + 2), 12:02-12:03: `23 PASS / 0 FAIL`.
  - Final hook-free build, 12:10: `15 PASS / 0 FAIL`. Values: `03-playing: ... cyan=146 cue=3828`;
    `d12 ... (cyan 146 -> 146)`.
- Existing probes on the final build, re-run with no re-thresholding:
  - `probe-routines.sh` (ROUTINES_RECORD_PAUSE=1.8): `98 PASS / 0 FAIL`.
  - `probe-deck-tabs.sh`: `6 PASS / 0 FAIL`.
  - `probe-manual-bpm.sh`: `22 PASS / 0 FAIL`.
- Hook hygiene:
  - The same hook patch was used (sha256 4b55ed59…4f54). It was applied at 11:57 and reverted with `git apply -R`
    at 12:04.
  - After the rebuild, `strings build-lane/.../MacOS/Audio-DNA | grep -c AUDIODNA_DEBUG_` returns `0`. The hook
    build read 5.
  - `git diff f15f318..HEAD -- src/MainComponent.cpp` is empty: this round made no MainComponent change.
- Lock: taken 11:55:31 (after an 860 s wait) and released 11:56:53. Taken 12:02:09 and released 12:03:55. Taken
  12:10:09 and released 12:13:36. Owner `routine-display-fix`.
- `.venv` was symlinked only inside each batch and was gone before every commit. The Audio-DNA left running after
  the last batch is pid 77956, owned by the `routines-timing` lane, which holds the lock. It is not this lane's app.

### Findings → disposition
| # | Sev | Finding | Disposition |
|---|-----|---------|-------------|
| 1 | MUST | The routine cue cyan is identical to the app-wide "mapped knob" cyan | **Fixed (F2).** `AudioDNALookAndFeel::kRoutineCue` #b4ff2e is chartreuse, about 82 degrees. A hue survey of every UI colour literal found 60-120 degrees empty. It is applied to the band name, the V fill, and the inspector digits, ROUTINE hint and slider fill/thumb. No accent cyan is left on a routine-held row (d12, pixel test). The band hairline stays teal, the pad's colour. |
| 2 | MUST | Settings edits on a Waiting pad never reached the pending start | **Fixed (F1).** `RoutineEngine::resyncPending` runs every tick while the routine waits. It re-reads quantize, loop, restore and style, releases or schedules the glides, and keeps `startsOn` (the tooltip) in step with the menu tick (D6, d13). |
| 3 | SHOULD | `restartPending` was plumbed through but never shown | **Fixed (F4).** The pad draws a "back to the start" mark left of "5/8" and the tooltip reads "Restarting from the top on the next bar." `startsOn` is now published for a pending restart too (D5, d14, after-13 shot). |
| 4 | SHOULD | The "Delete routine" row was not styled as dangerous | **Fixed (F3).** The row is drawn in kMeterRed via `addColouredItem`. The app LookAndFeel now honours an item's own colour (crop-after-10-pad-menu). |
| 5 | SHOULD | Band x: no tooltip, and it stops the routine on every layer | **Tooltip added (F5).** It reads "Stop this routine on every layer it plays on. A routine stop cannot be undone." The behaviour itself is unchanged: it follows design-final 2.8 and still **needs Boris's Tier-4 sign-off**. |
| 6 | SHOULD | Layer X silently stops routines too | **Tooltip added (F5)** on the X, which had none: "Clear this layer's clip. Also stops every routine playing on this layer (the clip comes back with Undo; a routine stop cannot be undone)." The behaviour is unchanged (design-final 2.8, question 2) and still **needs Tier-4 sign-off**. |
| 7 | SHOULD | "x" is drawn instead of "×" | **Left.** This is the deliberate Pitfall 6 workaround; the finding itself says to revisit it only with a reliable 9-10 pt × render. |
| 8 | SHOULD | Idle and Waiting pads look nearly the same | **Left.** This is the exact design-final spec; the finding asks for a Tier-4 comparison before any change. |
| 9 | SHOULD | Only LOOP gets a badge | **Left.** This is deliberate design scope; the finding makes it conditional on Boris's live use. |
| 10 | SHOULD | Pad numerals echo the column numerals | **Left.** The pad number is the fire identity (keys, MIDI, OSC and REST use pad 1-8, `RoutineDeckView::Pad::number`). Changing it to a letter or dropping it is a design change, not a cheap fix. Flag it for Tier-4 (it was already in RISKS). |

### Files changed (f15f318..HEAD)
- `src/recording/RoutineEngine.h/.cpp`:
  - `resyncPending` (F1);
  - `startsOn` is also published for a pending restart (F4).
- `src/ui/LookAndFeel.h`: `kRoutineCue` (F2).
- `src/ui/LookAndFeel.cpp`: `drawPopupMenuItem` honours `textColour` (F3).
- `src/ui/LayerStrip.h/.cpp`:
  - the cue colour on the band name and V fill (F2);
  - `juce::TooltipClient` + `tooltipAt`, and the `clearBtn_` tooltip (F5).
- `src/ui/UniversalParamControl.h/.cpp`: `routineHandHolds()`; the digits, hint and thumb in the cue colour (F2).
- `src/ui/RoutineDeckView.h`:
  - `MenuItem::destructive` (F3);
  - `Pad::restartPending` and `restartText` (F4).
- `src/ui/DeckView.cpp`: `addColouredItem` for the destructive row (F3).
- `src/ui/RoutinePad.h/.cpp`: the restart mark (F4).
- Tests:
  - `test_routine_engine`: D6, and D5 extended.
  - `test_param_control_routine_cue`: rewritten to hue-decode.
  - `test_layer_strip_follows_model`: hue, band colour and tooltip cases.
  - `test_lookandfeel_square`: an item-colour case.
  - `test_routine_deck_view`: destructive and restart cases.
  - `test_routine_pad_press`: a restart-mark pixel test.
  - `tool_routine_deck_snapshot`: a t8 render.
- `.harmony/probe-routine-display.sh`: d12, d13 and d14 added; the d9 oracle now decodes by hue.
- Docs: `CLAUDE.md` (UI Patterns "Routine pads and bands"), `docs/claude/pitfalls.md` 40,
  `docs/claude/recording.md` Surfaces, `.harmony/APP-INVENTORY.md` (LayerStrip row), `.harmony/notebook.md`.

### Shots (all current) — `.harmony/.reports/s-rta-0927/routine-display-shots/`
- `after-*` are this round's captures:
  - Phase 1 comes from the final hook-free build: 01-idle, 02-waiting, 03-playing, 04-two-on-one, 05-offdeck,
    06-removed, 08-empty, 09-warning, 12-fader-follow, and the new 13-restart-pending.
  - Phase 2 comes from the hook build: 07-layer-x-before, 07-layer-x, 10-pad-menu-snapshot, 11-record-tab,
    03-playing-inspector.
- `fixround-before-*` show 03-playing, 03-playing-inspector, 10-pad-menu-snapshot and 13-restart-pending on the
  pre-fix-round build.
- `before-*` (main, pre-lane) are unchanged.
- `crop-after-*` and `crop-fixround-before-*` are cut at the original rectangles. The rows, deck and strip-band
  crops are joined by `crop-after-10-pad-menu` and `crop-after-03-playing-inspector-rows`.
- `tool-t1..t8-*` are headless renders.
- Every changed PNG was opened and looked at:
  - the chartreuse band names and L1 V fill;
  - the inspector's Master/Opacity rows, where the digits, ROUTINE and the slider are chartreuse and no cyan remains;
  - the red "Delete routine";
  - "|◀ 5/8" on pad 1.
  The Mod1/Mod2 macros and the Link rings stay the ordinary cyan and read clearly apart from the cue.

METHOD: One commit per finding group. For each: write the tests first, record a RED against f15f318 (a running RED
wherever the test can compile against the old source, a compile RED otherwise), implement, run the targeted tests,
run the full build plus serial ctest, then commit. For the live runs, the pre-fix-round app was copied aside before
the first fix-round build to serve as the RED baseline. Batches went through `fixlive.sh`, which handles the lock
poll, the `.venv` link and the owner-only release, with `AUDIODNA_LOCK_OWNER=routine-display-fix` and `open -g`
only. Captures are window-only, by Quartz window id. No Output window was opened: the Output combo stays "Off" in
every shot. There was no synthetic input.

CONFIDENCE + VERIFY:
- HIGH for F1-F4: unit-pinned and live-witnessed over REST and pixels.
- MEDIUM for F5: the tooltip text is unit-pinned, but the hover display needs a real mouse (Tier-4).
- VERIFY (Harmony gate):
  - `ctest --test-dir build-lane` = 694/694;
  - `probe-routine-display.sh` = 15/0 on build-lane, and RED on f15f318's build;
  - Tier-4: in a Waiting pad's menu, pick Start: Jump or Quantize 4 Bar and check that the start follows; press a
    playing pad again and look for the mark; hover the band x and the layer X; compare Idle and Waiting pads side
    by side; sign off on the band-x and layer-X routine stops.

UNKNOWNS / NOT DONE:
- SHOULDs 7-10 are left, with the reasons in the table above.
- A pending **restart** (re-fired while running) still takes its Quantize from the moment of the re-fire. The
  Loop/Restore/Style settings are re-read at the cycle end, as before. A Quantize edit made during that short wait
  applies from the next start. This is disclosed, not fixed: the MUST named the Waiting path only.
- tests/visual Tier-1 was not run: no shader, source or effect changed.
- No perf numbers were taken: the change is message-thread UI plus one settings copy per waiting routine per tick.

NUANCE:
- The d9 row of this lane's own probe changed its oracle, from cyan to the routine-cue hue, because the colour
  changed by design. The +300 threshold is unchanged. d12 is a new row that pins the old colour's absence. No
  other probe was touched.
- `fixround-before-10-pad-menu-snapshot` and `fixround-before-03-playing-inspector` are the previous round's
  committed after-shots, copied before they were overwritten.
- The hue survey covered `0xffRRGGBB` literals in `src/ui` + `MainComponent.cpp` only. Shader output is not UI
  chrome. Colour-bar test cards (yellow/green) fall outside 68-100 degrees, and 01-idle measured `cue=0` live.

HANDOFF-NEEDS: Harmony's behavioral gate, an independent Reviewer and a critic re-check, then Tier-4 by Boris for
SHOULDs 5, 6, 8 and 10. The merge-order note with lane O still holds. This round added no MainComponent,
CMakeLists.txt or tests/CMakeLists.txt changes.

INBOX-RECHECK: none

### PACKET QUALITY (fix round)
- Clarity: CLEAR. Each finding carried a fix direction.
- Missing context: the app LookAndFeel ignored popup item colours, so the "red Delete" SHOULD needed a
  LookAndFeel change. That change is app-wide but inert: there are no other `addColouredItem` callers.
- Unused context: none.
- Self-brief files: this report, design-final.md (colour spec), the prior hook patch and scratch logs — all useful.
