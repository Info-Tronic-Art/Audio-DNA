# UX Critique — Routines Strip (Record Panel), Round 1

**Reviewer role:** UX critic (usability, clarity, live-performance safety), read-only.
**Artifact judged:** worktree `.claude/worktrees/wf_b3f13eab-924-2`, branch
`worktree-wf_b3f13eab-924-2`, commit `1832e5e8a275fc39df93550dd59ed22d27f0d992`.
**Shots judged:** `.harmony/.reports/s-rta-0926/strip-visual/{1-empty,2-saved,3-playing}.png`
(inside that worktree at that commit).
**Source read:** only at the pinned commit, via `git -C <worktree> show <commit>:<path>`.

## Verdict: PASS (0 MUST findings)

The strip is legible at a glance, uses whole words throughout, matches the dark VJ theme, and
the one real bug the builder found (4-per-row text clipping) was caught and fixed before this
commit — shot 3 confirms the fix (`"1: Drop 1 (bar 2)"` renders in full on the green Playing pad
in a 2-per-row layout). Empty/idle/playing states are each visually distinct (dim grey /
neutral-enabled / green). Nothing here blocks showing Boris.

That said, there are real usability gaps against this panel's *own* established conventions,
listed below as SHOULD/NICE.

---

## SHOULD

### S1 — Pending/running pads never say "Stop"; only colour + suffix signal it
`src/ui/RoutineBankModel.h:41-53` — a pending pad's text is `"N: Name (next bar)"` and a running
pad's is `"N: Name (bar N)"`; only `tone` (Warning/Playing) and the tooltip
(`"Press to stop."`) tell the performer the next press stops it, never the button's own label.

Compare this same panel's two lifecycle buttons, which explicitly relabel on state change
(`src/ui/RecordPanelModel.h:169`: `"Record Take"` → `"Stop Recording"`; `:187`: `"Play Take"` →
`"Stop Playback"`). The Routines row breaks that pattern: it is the only stateful control group
in RecordPanel where "what does pressing this do right now" is never spelled out in the label,
only inferred from colour. Under stage lighting, at a glance, a green pad reading a routine name
is a reasonable "it's running" signal (and pad-grid conventions elsewhere in the app — Launchpad
MIDI feedback — do use colour-only state), but it is a real inconsistency with the panel's own
established word-based affordance, and the task's "distinguishable at a glance" bar is stricter
for a stop action than a status readout: mis-reading a lit pad as "not yet fired" and pressing it
again mid-set would stop a routine live. Not blocking (colour + the "(bar N)"/"(next bar)" suffix
do communicate state), but worth a text-level "Stop" affordance to match the rest of the tab.

### S2 — Save Routine's four controls never disable, unlike every other control in this panel
`src/ui/RecordPanel.cpp` (`applyRoutines`, ~line 425-437) only ever touches the 8 pads and the
notice label; `fromBarEditor_`, `toBarEditor_`, `routineNameEditor_`, and `saveRoutineBtn_` are
never passed through `.setEnabled()`/`.setAlpha()` anywhere in the file (confirmed by grep — only
`applyButton`/`applyPad` and the take-side controls call `setEnabled`/`setAlpha`). They stay fully
opaque and clickable even in the exact state shot 1 captures: `"Ready. No take loaded."`

Every *other* control in this same panel proactively greys out with an explanatory tooltip when
it cannot succeed — e.g. `RecordPanelModel.h:193`: `v.play = {"Play Take", false, "Load a take
first.", ...}`, dimmed via `kDisabledAlpha` (`RecordPanel.cpp:252`). Save Routine has no such
guard: a performer can type bars, press Save, and only learn "no take loaded" (or "bank full",
etc.) from a transient cyan notice *after* pressing — the reason is still shown (satisfies "a
refused save with its reason is distinguishable"), but the *proactive* signal the rest of the tab
gives before the fact is missing here. Given Save Routine reads a currently-loaded take (per the
plan: it slices bars from the loaded take), it is plausible to want it disabled+dimmed with a
tooltip when `lastStatus_` has no take loaded, exactly like Play Take.

### S3 — Loop and "start from now" are not reachable from this strip at all
`src/model/Routine.h` (per plan section 3.1) and `src/recording/RoutineEngine.h` (`Status::Slot`)
carry `loop` (default false) and `restoreState` (default true = restore; false = "start from
now") per routine, and the REST surface exposes them
(`POST /api/routine/set {"loop":true,"restoreState":false,...}` per the plan's section 5). The
Save Routine row in this panel (`RecordPanel.cpp`, "Row E"/"Row F") only exposes From bar / To
bar / Name — no Loop toggle, no Restore/"start from now" toggle, no quantize control anywhere in
the UI.

This matches the plan's explicit scoping (`plan-routines-s1-final.md` §5.4: "a 'Save Routine' row:
`From bar` / `To bar` editors + name + button" — loop/restoreState/quantize are deliberately
REST-only for this lane), so it is not a defect against this PR's stated scope. But the task
description frames "loop / start-from-now" as part of what a performer should be able to do from
this strip, and today a performer with only the app UI cannot set either — they get whatever
default the save funnel applies (loop=false/once, restore=true) with no in-panel way to change it
for a saved routine. Flagging as a scope note, not a bug: the panel is usable and honest about
what it does, it just does noticeably less than "loop / start-from-now" implies.

## NICE

### N1 — Save always targets "the first empty pad" silently
The Save button's own tooltip says *"Saves the chosen bars of the loaded take onto the first
empty pad"* — informative, but there is no on-screen indication of *which* pad number that will
be before pressing, and no way to target a specific pad from the panel (REST-only, per the plan).
A one-line preview (e.g. "Will save to pad 3") would remove the guesswork.

### N2 — No tooltip/hint for "bank full"
Nothing in the Save Routine row's copy anticipates the 8/8-full case; the performer would learn
it only from the post-press refusal notice. Minor, since the refusal-with-reason path exists and
is consistent with how every other refusal in this panel is surfaced.

---

## What's genuinely good (for context, not scored)

- Whole-word labels everywhere ("From bar", "To bar", "Save Routine", "Empty") — matches the
  project's UI Text Rules; no abbreviations found.
- Three states (Empty/dim, Idle/neutral, Playing/green, and Pending/yellow — not in the shots but
  present in `RoutineBankModel.h`) map to three visually distinct tones reused from the app's
  existing palette (`kMeterGreen`, `kMeterYellow`), so no new colour-contrast risk was introduced;
  the Playing tone reuses the exact bg/text combo already vetted for the Play button
  (`kMeterGreen` bg / `kBackground` text, `RecordPanel.cpp` `applyButton`/`applyPad`).
- The clipping bug (4-per-row text overflow) was caught with the project's own snapshot tool
  before commit and fixed by switching to 2-per-row — shot 3 confirms the fix holds with the
  longest realistic label (`"1: Drop 1 (bar 2)"`).
- Save Routine's row placement, spacing, and typography (label above field, bold section headers)
  are consistent with the rest of the Record tab's existing rows.

---
*Findings above are this critic's own read of the pinned commit's source; no code was modified.*
