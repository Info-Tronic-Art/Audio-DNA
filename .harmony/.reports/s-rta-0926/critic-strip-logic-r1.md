# Critic: Routines strip — logic, round 1

**Item**: Record tab "Routines" strip (8 pads + Save Routine row), s-rta-0926 Lane 3.
**Pinned commit**: `1832e5e8a275fc39df93550dd59ed22d27f0d992` (worktree `wf_b3f13eab-924-2`).
**Files read at that commit**: `src/ui/RoutineBankModel.h`, `src/ui/RecordPanel.h`, `src/ui/RecordPanel.cpp`,
`src/recording/RoutineEngine.h`, `src/model/Routine.h`, `src/MainComponent.cpp` (`perfRoutineSave`),
`.harmony/.reports/s-rta-0926/plan-routines-s1-final.md` §7 LANE 3.
**Shots judged**: 1-empty.png, 2-saved.png, 3-playing.png (all three, as supplied).

**Verdict: PASS** — no MUST found.

---

## MUST
None.

## SHOULD

1. **Save Routine controls never reflect model state — breaks the panel's own stated contract.**
   `RecordPanel.cpp`'s `applyRoutines()` only touches the 8 pads (`applyPad`) and `routineNoticeLabel_`;
   `saveRoutineBtn_`, `fromBarEditor_`, `toBarEditor_`, `routineNameEditor_` are configured once in the
   constructor and never re-visited by `refresh()`/`applyRoutines()`. Every other control in this panel
   (Record/Play/Load/Reveal/Repair, the two toggles, the name field) is driven by
   `deriveRecordPanelView()` each refresh per the file's own header comment ("A THIN VIEW: every button's
   text/enabled/tooltip/tone... comes from the model"). The Save Routine row is the one exception: it is
   full-opacity and clickable in 1-empty.png even though the panel's own status line reads "Ready. No take
   loaded." — clicking it there produces a correct refusal ("No take is loaded. Use Load Take... first.")
   but only after the click, not as an advance visual cue the way every sibling control in this same tab
   behaves. Same gap if the bank is full (no pad is free) or a bar range is out of the take's range —
   the button gives no dimmed/disabled signal in any of these situations, unlike Record/Play/Load which
   grey to `kDisabledAlpha` when unusable.
   *Fix scope*: either route the Save row through the (already-fetched) `RoutineEngine::Status`/take state
   each `applyRoutines()` call the same way `applyView()` does for the take-lifecycle buttons, or drop the
   "thin view" claim for this row specifically — as written, the two rows of the same panel follow two
   different contracts and a performer sees one button family dim in advance of failure and the other not.

2. **A looping / "start from now" routine is visually identical to a one-shot / restore routine on the
   pad.** `RoutineEngine::Status::Slot` carries `loop` and `restoreState`, but
   `deriveRoutineBankView()`'s running-state branch only reads `s.name` and `s.position` — the pad text is
   `"N: <name> (bar N)"` and the tone is flat `Playing` (green) regardless of `loop`. The judge brief's own
   state list ("empty, waiting for the next bar, playing, looping...") asks this to be distinguishable at a
   glance, and today it is not: a pad mid-cycle gives no way to tell whether it will hold/stop at the end of
   its length or keep going. This is consistent with the plan (`plan-routines-s1-final.md` §7 LANE 3 scopes
   this strip to fire/stop + name/fromBar/toBar save only, explicitly "cut without loss") and the panel also
   gives no way to *set* loop/restoreState when saving — so this is a real but low-frequency gap: it only
   surfaces for a routine authored with `loop:true` via REST/OSC/binding elsewhere, not for anything created
   from this panel itself (every panel-authored save takes the `sliceRoutine()` defaults, `loop=false`,
   `restoreState=true`, since `onSaveRoutine` only forwards name/fromBar/toBar). Not a MUST because it
   matches the plan's explicit, reviewed scope cut and never produces a *wrong* reading, only an incomplete
   one for a minority case.

## NICE

- No indication, before pressing Save, of which pad the routine will land on (per plan, "the first empty
  pad"); the performer only learns this after the fact by seeing which pad populates. A short hint (even
  reusing the existing tooltip's wording inline) would remove the one guess left in "save without
  instructions."
- `fromBarEditor_`/`toBarEditor_` take any digits with no live validation (e.g. To bar < From bar, or a
  blank field parsing to 0); the resulting refusal is correct and worded well
  (`RoutineEngine::saveRefusalText`), but it only appears after the round trip to the engine rather than as
  immediate field feedback.

---

## What checked out (no issue)

- **State→tone→text mapping is internally consistent** across all four `RoutineEngine::Status::Slot`
  states (`empty`/`idle`/`pending`/`running`) in `deriveRoutineBankView()`: empty=disabled/Neutral,
  idle=enabled/Neutral/"press to fire", pending=enabled/Warning/"(next bar)"/fires→Stop,
  running=enabled/Playing(green)/"(bar N)"/fires→Stop. Matches all three shots exactly (1: all 8 "N: Empty"
  greyed; 2: pad 1 "1: Drop 1" idle/Neutral; 3: pad 1 "1: Drop 1 (bar 2)" on green).
- **Fire-vs-Stop routing is correct**: `RecordPanel.cpp`'s pad `onClick` reads
  `lastRoutineView_.pads[i].firing` (true only in `idle`/`empty`) to choose `onFireRoutine` vs
  `onStopRoutine` — matches each state's own tooltip ("Press to fire" / "Press to stop").
  Disabled (`empty`) pads never reach `onClick` at all (standard JUCE `TextButton` behavior), so there is no
  path to firing a pad with nothing saved on it.
- **Bar arithmetic is correct**: the "(bar N)" arithmetic in `deriveRoutineBankView()`
  (`floor(position / 4) + 1`) matches the
  ctest-pinned case and the builder's cited live-probe reading (`position=5.65` → same bar-2 result as the
  pinned `position=5.0` case) — no drift between the two beats-per-bar conventions in this file.
- **Refused-save-with-reason is genuinely shown**: `runRoutineAction()` writes any non-empty return of
  `onSaveRoutine`/`onFireRoutine`/`onStopRoutine` into `routineNotice_` and `applyRoutines()` renders it into
  `routineNoticeLabel_`, laid out directly under the Save Routine row with clear vertical room in all three
  shots (nothing below it competes for space) — a refusal is not fighting the take-notice line for
  visibility since it's a separate label (`routineNotice_`/`noticeKey_` are kept apart from the take
  lifecycle's `notice_`/`noticeKey_` on purpose, per the header comment).
- **Layout fix for text clipping is real and holds**: 2-pads-per-row (not the original 4) gives
  `"1: Drop 1 (bar 2)"` visible full-width room in 3-playing.png at the app's fixed 14pt
  non-ellipsized `drawButtonText` — confirmed by reading the commit message's own account of catching the
  4-per-row clip via the snapshot tool before committing, and the shipped `resized()` comment matches
  (`kPadsPerRow = 2`, "so a running pad's ... suffix has room").
- **Whole-word / dark-theme / tooltip conventions** all hold: no abbreviations in any pad text, label,
  placeholder, or tooltip; tone reuse (`Playing`=green, `Warning`=yellow, `Neutral`=app default) matches the
  vocabulary already established by `RecordPanelView::Tone` one struct up in the same file, so "Playing"
  means the same green in the take-lifecycle row and the Routines row.

---

*Round 1 of the logic pass. Scope: control/state meaning consistency only — visual/spacing polish and UX
copy are separate critic passes per the workflow.*
