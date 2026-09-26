# Visual/Graphic-Design Critique — Routines strip (Record tab) — Round 1

**Artifact judged**: worktree `.claude/worktrees/wf_b3f13eab-924-2`, branch `worktree-wf_b3f13eab-924-2`,
commit `1832e5e8a275fc39df93550dd59ed22d27f0d992`.
Shots: `1-empty.png`, `2-saved.png`, `3-playing.png` (paths under that worktree's
`.harmony/.reports/s-rta-0926/strip-visual/`). Source read only at that commit in that worktree
(`src/ui/RoutineBankModel.h`, `src/ui/RecordPanel.h/.cpp`, `src/recording/RoutineEngine.h`,
`src/api/ApiServer.h`, `src/MainComponent.cpp` wiring block).

**Verdict: PASS**

No MUST-level finding. One SHOULD, one NICE.

---

## What was judged, and how it reads

**Empty (1-empty.png)**: 8 pads, 2-per-row, each "N: Empty", uniformly dimmed (alpha, not a color
swap) below the enabled "Save Routine" row. Whole words throughout ("Empty", "From bar", "To bar",
"Save Routine"). Reads immediately as "nothing saved yet, here's how you'd save one" — a performer
does not need instructions to understand this screen.

**Saved/idle (2-saved.png)**: pad 1 flips to "1: Drop 1" at full opacity in the app's normal
neutral button color, the other 7 pads stay dimmed "Empty". The contrast between "this one is real"
and "these are placeholders" is immediate and doesn't depend on reading the text.

**Playing (3-playing.png)**: pad 1 goes solid green, "1: Drop 1 (bar 2)" fits on one line at the
app's fixed 14pt button font with no clipping. The build log says the first pass was 4-per-row and
clipped this exact string, was caught by the builder's own snapshot tool, and was replaced with the
2-per-row layout before commit — that is exactly the right process (render the real worst-case
string, not a placeholder, before shipping), and the shot confirms the fix holds: "1: Drop 1 (bar 2)"
has visible margin on both sides inside the pad, not a near-miss.

Cross-checking the states that aren't in the three shots against `RoutineBankModel.h` (pure,
Catch2-pinned, so this is a source read of the actual mapping, not a guess):
- **Pending ("waiting for the next bar")**: yellow (`kMeterYellow`) background, black text,
  `"N: <name> (next bar)"`, tooltip "Starting on the next bar. Press to stop before it starts." —
  a third, distinct color from idle's neutral and playing's green, so all three live states are
  colour-coded, not just text-coded.
- **Refused save with its reason**: `routineNoticeLabel_` sits directly under the Save Routine row
  and is populated with the funnel's exact refusal string (`routineNotice_`) for `kNoticeSeconds`,
  same mechanism the take side of the panel already uses for `onRecord`/`onLoad` refusals. Same
  place a performer's eye already goes after pressing a button in this panel — consistent, not a
  new pattern to learn.

Color/tone reuse is disciplined: Playing pads reuse the exact `kMeterGreen`/`kBackground` pair the
Play Take button already uses; Warning reuses the same yellow/black pairing pattern; Empty/disabled
reuses the panel's existing `kDisabledAlpha` dimming rather than inventing a fourth colour. A
performer who already knows the Record tab's vocabulary reads the Routines row for free.

Layout/spacing: pad grid sits under a bold "Routines" label with a small gap, row gaps are uniform,
the Save Routine row below it (From bar / To bar / name / button) matches the height and inset
rhythm of the take-management rows above it. Nothing overlaps, nothing is flush to an edge, nothing
in the three shots is cut off by the panel bounds.

## SHOULD

- **Looping is not a glanceable state, and isn't settable from this panel.** `RoutineEngine::Status::Slot`
  carries a real `loop` bool (`src/recording/RoutineEngine.h`), but `deriveRoutineBankView()` never
  reads it — a looping routine and a one-shot routine render identically ("N: Name (bar N)", same
  green) while running. The Save Routine row also has no loop control: `RecordPanel::onSaveRoutine`
  only forwards `name`/`fromBar`/`toBar`, so `ApiServer::RoutineSaveOpts::loop` (an
  `std::optional<bool>`) is left unset for every routine saved from this panel — a performer using
  only this UI can't choose loop-or-not, and can't tell after the fact which one they got. The
  bar-in-cycle counter (`(bar N)`) does let an attentive performer *infer* looping by watching it
  wrap back to bar 1 after a few seconds, so this isn't invisible, just not "at a glance." This
  reads as an intentional scope line (the fence for this slice is literally "From bar / To bar
  numeric editors, a name field, a Save button" per the builder's own summary), not a bug in what
  was built — recommend either a short `"loop"` badge/suffix on a looping pad plus a checkbox on
  the Save Routine row in the next slice, or an explicit note to Boris that loop is REST/engine-only
  for now.

## NICE

- Consider capturing a 4th shot (pending/"next bar" yellow tone) in a future round for full visual
  coverage — the state exists correctly in code but wasn't screenshotted this round, so it's
  verified by source read here rather than by pixels.

---

**MUST**: none.
**SHOULD**: loop is engine-tracked but neither shown on a pad nor settable from Save Routine.
**NICE**: capture the pending/"next bar" tone in a future shot round.
