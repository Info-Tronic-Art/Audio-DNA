# Reviewer Verdict — routine-strip r1
STATUS: DONE
VERDICT: APPROVE

## Scope
Pinned worktree `.claude/worktrees/wf_b3f13eab-924-2` @ 1832e5e8, branch
worktree-wf_b3f13eab-924-2 (commit "feat(s-rta-0926 routine-strip): optional
Record-panel Routines bank strip"). Diff vs main: 10 files, 536 insertions
(3 PNG fixtures + src/MainComponent.cpp, src/ui/RecordPanel.{h,cpp},
src/ui/RoutineBankModel.h (new), tests/CMakeLists.txt,
tests/test_routine_bank_model.cpp (new), tests/tool_routine_strip_snapshot.cpp (new)).
Reviewed against plan-routines-s1-final.md §5.4 and §7 LANE 3 (same file, present
in the pinned worktree).

## Checklist findings (VERIFIED — read on disk, no build)

1. **RoutineBankModel.h pure + juce_core-only** — VERIFIED. `src/ui/RoutineBankModel.h`
   includes only `recording/RoutineEngine.h`, `<juce_core/juce_core.h>`, `<cmath>`,
   `<string>`. Walked the transitive include chain
   (RoutineEngine.h → Program.h/Player.h/RecorderClock.h → Take.h/Lane.h/PerfState.h/
   TempoMap.h/ControlPath.h/AutomationCurve.h) — every one of those headers is
   juce_core-or-std only, no juce_gui_basics/juce_graphics. `tests/CMakeLists.txt`'s
   new `test_routine_bank_model` target links `Catch2::Catch2WithMain` + `juce::juce_core`
   only — same posture as `test_record_panel_model` (cited by name in both the
   header's own comment and the CMake comment).

2. **View never holds engine pointers (hooks only)** — VERIFIED. `RecordPanel.h`
   adds exactly 4 `std::function` hooks (`onFireRoutine`, `onStopRoutine`,
   `onSaveRoutine`, `onRoutineStatus`) and no `RoutineEngine*`/`Composition*`
   member. `MainComponent.cpp`'s wiring block sets `onRoutineStatus = [this] {
   return routineEngine_.status(); }` — the engine pointer stays in MainComponent;
   the panel only ever sees a `RoutineEngine::Status` value.

3. **Refresh cadence** — VERIFIED. `RecordPanel::refresh()` (called from
   `MainComponent::timerCallback()`'s existing ~4 Hz block, `MainComponent.cpp:3522-3525`,
   which already drove the take-recorder half of the panel) now also calls the new
   `applyRoutines(nowSeconds)`, which calls `onRoutineStatus()` →
   `routineEngine_.status()` (a mutex-guarded copy, per `RoutineEngine.h:105`).
   `timerCallback()` is a `juce::Timer` callback, i.e. message thread — confirmed
   no new thread/lock introduced. (The task text names this "routineStatusVar";
   there is no such variable — the hook calls `status()` directly each tick, which
   is the correct/lighter mechanism and still message-thread-only.)

4. **Every state rendered** — VERIFIED. `RoutineEngine::Status::Slot::state` has
   four values (`"empty"|"idle"|"pending"|"running"`, default `"empty"` per
   `RoutineEngine.h:59`); `deriveRoutineBankView` branches on all four (the fourth
   via an `else` catch-all labelled "empty" in a comment). `test_routine_bank_model.cpp`
   has one `TEST_CASE` per state (empty/idle/pending/running) plus a bar-1 edge
   case and a pad-independence case — 6 cases total, all asserting concrete text/
   tone/enabled/firing values, not tautologies.

5. **Save refusal text whole words** — VERIFIED by reading every string literal
   returned from `MainComponent::perfRoutineSave`'s `refuse(...)` lambda (pre-existing
   function this lane reuses unmodified: "Could not read the take at …", "No take
   is loaded. Use Load Take... first.", "There is no routine pad N; the pads are
   1 to 8.", "The routine bank is full. Remove a routine or choose a pad to
   replace.", "Could not put the routine on pad N."). All whole-word, no
   abbreviations. `RecordPanel::runRoutineAction` surfaces this string verbatim
   in `routineNoticeLabel_` for `kNoticeSeconds`; on empty-string (success) it
   clears the name editor instead — correctly distinguishes refusal from success.
   All new panel-authored strings (pad text, tooltips, "Save Routine", "From bar"/
   "To bar", "Name (blank = Routine N)") are also whole-word.

6. **MainComponent diff confined to the RecordPanel wiring block** — VERIFIED.
   `git diff main...worktree-wf_b3f13eab-924-2 -- src/MainComponent.cpp` shows a
   single hunk, entirely inside the pre-existing `{ auto& rp = browserPanel_->
   getRecordPanel(); ... }` scope (the same block the plan cites,
   `MainComponent.cpp:1587-1604` pre-diff / `:1614-1629` post-diff) — 4 new lambda
   assignments, nothing outside that block touched.

7. **Message-thread only** — VERIFIED. All new call sites (pad `onClick`,
   `saveRoutineBtn_.onClick`, `timerCallback`'s `refresh()` call) are JUCE UI
   callbacks / `juce::Timer::timerCallback`, all message thread. `RoutineEngine::status()`
   is documented mutex-guarded (`RoutineEngine.h:105`, "HTTP thread reads it") so
   the message-thread read here is uncontended with that existing contract; no new
   mutex, no audio/analysis-thread touch.

8. **Whole-word UI text** — VERIFIED, scanned every `setText`/`setButtonText`/
   `setTooltip`/`setTextToShowWhenEmpty` call added in this diff; none abbreviate.

9. **Tests honest** — VERIFIED. `test_routine_bank_model.cpp` cases assert
   concrete derived strings/enums (not smoke-only). `tool_routine_strip_snapshot.cpp`
   is correctly excluded from `catch_discover_tests` (a manual PNG-producing tool,
   matches the plan's SHOTS requirement in §7 LANE 3) and does not silently
   masquerade as a ctest target.

## Cross-checks against the plan (VERIFIED)
- §5.4 hook names (`onFireRoutine(int)`, `onStopRoutine(int)`, `onSaveRoutine(name,
  fromBar, toBar)`, `onRoutineStatus()`) match verbatim.
- §7 LANE 3 file list (`RoutineBankModel.h` new, `RecordPanel.{h,cpp}`,
  `test_routine_bank_model.cpp` + CMake block, "ONLY the RecordPanel wiring block
  of MainComponent.cpp") matches the actual diff exactly — no scope creep.
- `RoutineBankModel.h`'s `kRoutineBeatsPerBar = 4.0` claim ("RoutineSlice.cpp's own
  local kBeatsPerBar") — VERIFIED against `src/recording/RoutineSlice.cpp:11`
  (`constexpr double kBeatsPerBar = 4.0;`).
- `Status::Slot::position` semantics ("routine beats since this cycle's start")
  used for the bar-number computation — VERIFIED against `RoutineEngine.cpp:435`
  (`sl.position = r.pending ? 0.0 : r.position;`) and the tick logic setting
  `r.position` from a bar-relative accumulator.
- `AudioDNALookAndFeel::drawButtonText`'s no-shrink/clip behavior, cited in a
  `resized()` comment to justify "2 pads per row not 4" — VERIFIED
  (`LookAndFeel.cpp:78-85`: fixed 14pt font, `g.drawText(..., false)` = no
  ellipsis/shrink, so an over-length label clips against the button's paint
  clip rather than shrinking).

## Non-blocking observations
- `fromBarEditor_`/`toBarEditor_` are digit-restricted (`setInputRestrictions(5,
  "0123456789")`) but the panel does not client-side-validate `fromBar >= 1` or
  `fromBar <= toBar` before calling `onSaveRoutine`; a blank field reads as `0`
  via `getIntValue()`, which the pre-existing `takeBeatOfBar` helper
  (`RoutineSlice.cpp:445`) was not written to expect for bar 0 and would produce
  a negative beat offset rather than a refusal. This is not a regression this
  lane introduced — the same server-side call is already reachable unmodified
  via `/api/routine/save`'s `useBars` path — and the plan does not ask for new
  client-side bar validation, so it is a suggestion, not a blocker. A future
  pass could clamp `fromBarEditor_`/`toBarEditor_` to `>= 1` at read time.
- `saveRoutineBtn_`'s tooltip ("Saves the chosen bars of the loaded take onto the
  first empty pad") does not mention the full-bank refusal path; accurate for the
  happy path, just incomplete — cosmetic.

## SLIM
No `EXCESS_DEAD`/`EXCESS_VESTIGIAL`/`EXCESS_DUP`/`EXCESS_SPEC` found.
`tool_routine_strip_snapshot.cpp` is deliberately excluded from `catch_discover_tests`
per the plan's own SHOTS requirement (§7 LANE 3) — `JUSTIFIED_KEEP reason="sanctioned
manual screenshot tool for a gate the plan explicitly requires; never runs under
ctest, adds no CI cost"`. All new fields/branches (`RoutineBankView::Pad::firing`,
all 3 `Tone` values, the empty-bank else-branch) are exercised by
`test_routine_bank_model.cpp`.

## Verdict rationale
Every checklist item verified on disk against the pinned worktree and the plan
section it was gated on. No blocking issues found; two cosmetic/non-blocking
observations noted above for the record.

METADATA: reviewer=reviewer-routine-strip, builder_packet=routine-strip-r1, date=2026-09-26T00:00:00Z
