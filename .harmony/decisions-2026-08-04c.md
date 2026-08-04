# BORIS RULINGS — session 2026-08-04c

## RULING 1: multi-image drop legibility — BADGE, NOT CHOOSER
**Ruled: badge + tooltip + name-truncation fix (~15 lines). The ~450-line transient pill is
KILLED.**

Path to the ruling, worth preserving because the process did real work:
1. Boris asked for a post-drop toggle between "one cell" and "multi cells".
2. Architect designed a two-segment chooser pill (v1, ~500 lines).
3. Harmony pushed back on scope; architect self-revised to a notice-with-action (v2, ~400-450).
4. **Two blind critics, dispatched separately, BOTH returned UNSOUND/FAIL and independently
   converged on the same alternative** — make the cell say what it is, permanently.
5. Boris ruled for the small fix.

**The root cause the critics found — this is the actual finding of the whole exercise:**
`ClipCell.cpp:48-50` paints `MediaType::Video` and `MediaType::ImageSequence` through the SAME
branch. **An image sequence and an .mp4 are visually identical in the grid.** Harmony verified
this directly. That is why Boris did not recognise his own shipped feature — not the drop
behavior, the rendering.

Second cause, also Harmony-verified: the clip name is already correct — `"<parentDir> (N frames)"`
(`MainComponent.cpp:3718-3719`) — but `ClipCell.cpp:106-108` draws it left-justified with
ellipsis into ~84px at 10pt. **"(N frames)", the only part that explains the merge, is exactly
what gets truncated.** The app was telling him and cutting off the sentence.

NOTE / correction to a critic claim: critic-flow stated the name also renders inside the
thumbnail for sequences (`ClipCell.cpp:49,64`). Harmony re-read that branch: the name is drawn
in the thumbnail area ONLY in the `else` fallback when `thumbnail_.isValid()` is false. A
sequence WITH a thumbnail shows the picture and the truncated name bar. The UX critic's finding
stands; the flow critic overstated it. Recorded so the next session does not inherit the error.

Why the pill lost, in one line each (both critics, independently):
- Cells are 90px wide with `kCellGap = 0` — a legible pill needs ~115-130px. **Wider than its
  anchor, with no gutter.** It would occlude the thumbnail, which IS the trigger hit box.
- The grid's existing idiom for "this cell is special" is a small persistent glyph ("SRC" tag,
  lock "L", missing-file "!"), not a floating control. A rounded pill is foreign by construction.
- Its dismiss mechanism (`UndoManager::onHistoryChanged`) is **structurally blind to autopilot
  advances and non-user deck switches** — the two things most likely to happen mid-set — because
  neither creates a command. A safety mechanism blind to its most probable trigger is not one.
- `UndoManager::onHistoryChanged` is a single `std::function` already assigned at
  `MainComponent.cpp:1544`; the pill would have silently broken Edit-menu undo text.
- A 5-second timer makes the escape hatch a race, in the window where the clip is most likely to
  have started playing.

## RULING 2: sequence threshold — 3 OR MORE
**Ruled: 2 images now SPREAD across 2 cells; 3+ become a sequence.** Today the branch is
`images.size() == 1` vs `> 1` (sequence at exactly 2), which is why the surprise happened —
two images does not read as "an animation". Existing decks are unaffected (persistence stores
the resulting clip, not the drop rule).

## CARRIED FORWARD — NOT ruled, do not treat as decided
- Whether "SEQ N" is the right badge token, and whether badge clutter bothers him in a dense
  grid. He should see it and say.
- Mixed-drop spread ordering (images split around videos) — **moot under the badge ruling**, but
  it becomes live again if a spread action is ever added. Both critics called the split layout
  indefensible; do not resurrect it without relocating videos.
- Escape-hatch home if he later wants one: critic-flow recommends a persistent "Spread to cells"
  button in ClipInspector (which is already a panel of exactly such TextButtons) over any
  transient UI. No timer, no snapshots, works ten minutes after the drop.

## INCIDENTAL FINDINGS from the critics — NOT in scope, recorded so they are not lost
- [CRITIC-ONLY, unverified by Harmony] `Renderer::closeMediaForClip` (`Renderer.cpp:916`) has
  **ZERO call sites**. Media for replaced clips is never closed — a permanent leak per replaced
  clip. Worth its own investigation.
- [CRITIC-ONLY] Autopilot calls `Layer::triggerClip` on the GL thread and creates NO command by
  design (`TriggerCommands.h:28-32`), so it is invisible to undo history.
- [CRITIC-ONLY] Only the deck TAB CLICK wraps `SwitchDeckCmd`; genre auto-switch, REST, OSC and
  MIDI all call `handleDeckSwitch` bare with no command and no hook.
- [CRITIC-ONLY] `Clip` has render-thread-written `mutable` fields (`Clip.h:132-133, 156-157`) and
  `SetClipCmd::apply` is an unconditional whole-struct overwrite with no staleness check
  (`ClipCommands.h:76-92`). Any future snapshot-and-restore feature must not stomp those.
