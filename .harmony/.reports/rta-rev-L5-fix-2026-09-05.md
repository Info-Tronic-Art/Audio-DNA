# Reviewer Verdict — rta-rev-L5-fix-2026-09-05 (RealTimeAudio, L5 Quantize fix round)
STATUS: DONE
VERDICT: PASS_WITH_FINDINGS

FENCE: confirmed — diff touches exactly src/model/Layer.h, src/model/Deck.h,
src/core/DeckCommands.h, src/MainComponent.cpp, tests/test_undo_commands.cpp
(`git diff --stat`, 322 ins / 23 del). graphify-out/* dirty, ignored per
instructions. Machine claims re-verified independently: `ctest` 218/218 on a
fresh in-place rebuild; diff stat matches exactly after my own build-and-revert
experiments below (nothing left dirty).

## Adjudication of the three sent-back items

### 1. Deck-switch cancellation (Fix 1) — CORRECT where wired, but DEMONSTRABLY INCOMPLETE
`handleDeckSwitch` (MainComponent.cpp:4429) now cancels every layer's pending
trigger on the deck being LEFT, guarded on an actual index change. Traced all
6 callers of `handleDeckSwitch` (deckView tab click :638, the undo-pushing
lambda :1372, `apiServer_->onSwitchDeck` :1734/REST, `oscHandler_->onSwitchDeck`
:1764/OSC, genre auto-switch :1454 (routes through `handleDeckSwitch`, confirmed
by its own comment "Routed through handleDeckSwitch... so this gets the same
... every other switch path gets"), and the MIDI `SwitchDeck` binding action
:5715) — **all 6 correctly inherit the live cancel**, because it lives inside
the shared function, not at each call site. "Autopilot deck advance" does not
exist as a real path — `Autopilot.cpp` only ever calls `layer.triggerClip(...)`
(column-level, within a deck), never touches `activeDeckIndex` (grep-verified,
3 call sites, none deck-switching) — so it is correctly a non-issue, not an
uncovered path. "Removing the active deck" is also safe: `RemoveDeckCmd` only
ever erases the active deck itself (asserted invariant, DeckCommands.h:719-729)
and captures the whole `Deck` **by value** before erasing it — the pending
trigger goes away with the Layer objects it lived on; there is nothing left to
freeze. Undo/redo of a switch: traced and test-confirmed correct (see below).

**But two OTHER deactivation paths in the very file under review bypass
`handleDeckSwitch` entirely and reproduce the exact bug Fix 1 was written to
close:**
- **`AddDeckCmd::execute()`** (DeckCommands.h:635-663, wired from `kDeckNew` /
  "Add Deck" menu action, MainComponent.cpp:4661-4672): unconditionally sets
  `comp->activeDeckIndex = addedIndex_;` — deactivating whatever deck was
  previously active — with **no cancellation of that deck's pending trigger**.
  Queue a Quantize trigger on Deck A, click "Add Deck" mid-set, and Deck A's
  layer freezes with the trigger still pending, exactly as Fix 1 describes,
  firing arbitrarily late whenever the performer switches back to A.
- **The "Load Deck File" append action** (MainComponent.cpp:2762,
  `composition_.activeDeckIndex = composition_.appendDeck(std::move(incoming));`)
  — same gap: appends a new deck and activates it, deactivating the current
  deck without touching its pending trigger.
Both are ordinary live-performance actions (adding a deck, loading a saved
deck file mid-set), not obscure edge cases, and both sit in files already
inside this review's fence. This is a real, concrete "deactivation path still
uncovered" of the exact kind the packet asked me to name — Fix 1 is correct in
shape but not complete in coverage.

### 2. `clearActiveClip()` cancellation (Fix 2) — right location, but the motivating repro doesn't actually reach it, and it has ZERO test coverage
The cancel lines land inside `clearActiveClip()` itself (Layer.h:303-325), so
every caller inherits it structurally — the right call, matching the packet's
explicit instruction. Enumerated all 6 non-test callers:
1. `Layer::triggerClip`'s empty-cell branch (Layer.h:206) — pre-existing
   behavior, unchanged in effect (it already manually reset
   `pendingTriggerColumn = -1` here before this diff; the diff just moved that
   reset into the shared helper). Desirable, not new.
2. X-button clear, `onLayerClearClip` (MainComponent.cpp:706) — the packet's
   own motivating case. Desirable: an explicit "clear this layer" should not
   leave a trigger silently armed.
3/4. "Clear Deck Clips" / "Clear Layer Clips" (MainComponent.cpp:4710, 4787) —
   ALL clips in the layer are wiped first (`clips.clear(); ensureColumns(...)`),
   so a pending trigger's target is guaranteed gone anyway. Desirable, and
   incidentally fixes a second, independent bug: **pre-fix, a pending trigger
   surviving one of these clears would later fire via `triggerClipImmediate`
   against a column with `has_value() == false`**, setting `activeClipColumn`
   to point at an empty cell (Layer.h:274-300 does not guard on
   `clipOpt.has_value()` before falling through to `shouldTrigger`/
   `triggerClipImmediate`) — a "phantom active clip" bug, separate from and
   pre-dating the Quantize feature. Worth knowing this fix is doing double duty.
5. Cell clear on the active column, MainComponent.cpp:4970 (guarded on
   `layer->activeClipColumn == cell.column`) — see the blanket-cancel note
   below.
6/7. MIDI Momentary release, `TriggerClip`/`TriggerColumn` (MainComponent.cpp:
   5627, 5663), both guarded on `layer->activeClipColumn == resolvedColumn`.

**Traced whether the packet's own motivating scenario for Fix 2 — "hold a
Momentary pad under Quantize, release before the beat, the queued trigger
still fires" — is actually reachable, and it is not, by construction.**
Queuing only happens when `column != activeClipColumn`
(`triggerClip`, Layer.h:216); nothing changes `activeClipColumn` while a
trigger is pending (only `clearActiveClip()`/`triggerClipImmediate` do, and
both always clear `pendingTriggerColumn` first). So whenever
`pendingTriggerColumn == resolvedColumn` is true, `activeClipColumn ==
resolvedColumn` is *necessarily* false — the release branch's own guard
(`if (layer->activeClipColumn == resolvedColumn)`) can never be true for the
column that's actually pending, so `clearActiveClip()` is never invoked in
that exact scenario. The fix is not wrong to exist (it correctly fixes cases
3/4/5 above and is harmless here), but the concrete MIDI repro named in the
original finding does not reproduce as described — worth telling whoever runs
the packet's human-verification item #2 so they don't spend time chasing a
repro that structurally cannot occur, and worth re-checking my reasoning by
hand at the machine before fully retiring finding #2 (I did not have a live
build/GUI to click through; this is a static trace of `Layer.h`/
`MainComponent.cpp`, not a runtime observation).

**Zero test coverage for Fix 2, empirically confirmed, not just inferred:**
I reverted only the two new lines in `clearActiveClip()`
(`pendingTriggerColumn = -1; pendingTriggerSnapOverride = Off;`), rebuilt
`test_undo_commands`, and reran the 5 new tests — **all 5 still passed, 36/36
assertions**, with Fix 2 completely undone. Restored the file and reconfirmed
36/36 green with the fix back in. None of the 5 new tests, nor any
pre-existing test, calls `clearActiveClip()` with a pending trigger already
queued and asserts it gets cancelled.

**A real, undiscussed side effect of the blanket cancel, worth a product
decision (not blocking, flagging for the record):** `Layer` has exactly ONE
`pendingTriggerColumn` slot (not per-column), so "cancel the pending trigger"
inside `clearActiveClip()` cancels whatever is queued, regardless of whether
it targets the same column being cleared. Concretely: layer has clip A active
(column 3) and clip B queued for next beat (column 7, unrelated); performer
releases a Momentary pad bound to column 3 (the currently-active one) →
`activeClipColumn(3) == resolvedColumn(3)` → `clearActiveClip()` fires → B's
completely unrelated queued trigger is silently dropped too. Same shape at the
cell-clear site (:4970): clearing the active cell at column 3 also cancels an
unrelated queued trigger for column 7. This is arguably consistent with the
pre-existing "one pending slot per layer, last write wins" model (queuing a
second trigger already silently drops a first one, unfixed by this diff and
not new), so I'm not calling it a defect — but it is a real behavior change
introduced by this round that nobody tested or documented, and a human should
decide if it's the intended semantics.

### 3. The five tests — REAL, but only ONE of five exercises this fix round's code
All 5 build and pass (`test_undo_commands "[quantize]"` → 36/36 assertions, no
placeholder values, all assert on real production entry points —
`Layer::triggerClip`, `Layer::processPendingTrigger`, `Deck::triggerColumn`,
`TriggerClipCmd`, `SwitchDeckCmd` — none reach past a caller to poke internals
directly). The granularity test (test 2) genuinely distinguishes Beat from Bar
— it queues Beat on column 2 and fires it at `beatInBar=2` (non-downbeat,
proving Beat ignores bar position), then separately queues Bar on column 3 and
proves it does NOT fire at `beatInBar=2` but DOES fire at `beatInBar=0` — two
different values are exercised, not collapsed to one.

**But I built and ran an empirical before/after for each, not just a code
trace, and the result matters more than the raw count of 5:**
- Tests 1-4 (`triggerClip` queues, `processPendingTrigger` granularity,
  `Deck::triggerColumn` fan-out, undo/redo of `pendingTriggerSnapOverride`) all
  exercise code that was **already present and already correct before this fix
  round** — the queuing/override/undo-snapshot machinery that the *original*
  L5 review had already manually verified via code trace (its own report,
  section "Packet's false claim — checked, did NOT propagate into a defect").
  None of these four touch `clearActiveClip()` or `SwitchDeckCmd`. I confirmed
  this empirically above: reverting Fix 2 left all 5 green. They are honest,
  well-built tests that close the *original* "zero tests" gap for round-1's
  functionality — but they are not regression tests for either of the two
  fixes actually sent back this round.
- **Only test 5 (`SwitchDeckCmd` cancel/undo/redo) exercises fix-round code.**
  It would fail against pre-fix code even harder than a runtime assertion — it
  calls the new 6-argument `SwitchDeckCmd` constructor
  (`..., std::move(cancelled)`), which does not exist pre-fix, so it would fail
  to *compile* against the code as it stood when the original review ran.
- **Test 4, the one called out as highest-value, does genuinely catch the
  brace-list trap** — I reproduced this by hand: dropped
  `layer.pendingTriggerSnapOverride` from `captureLayerRuntime`'s return list,
  rebuilt, and reran — test 4 failed exactly as expected
  (`REQUIRE( after.pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar )` →
  `0 == 2`), the other four tests stayed green. Restored the file and
  reconfirmed 36/36. This is a genuinely load-bearing regression guard for a
  real historical bug shape in this codebase, not a decorative assertion.

**Net: "five tests, as required" is technically true and none of them are
fake, but the test remediation does not actually cover the fix that most
needed it.** Fix 2 (`clearActiveClip` cancellation, one of the two HIGH items
sent back) has no automated coverage at all; Fix 1 has exactly one test,
covering only the single call site (`SwitchDeckCmd`) that pushes undo history,
not the live cancellation inside `handleDeckSwitch` itself (which IS what all
6 real call sites depend on) — though that gap is more forgivable since
`handleDeckSwitch`'s body isn't independently unit-testable without linking
MainComponent, consistent with this codebase's established
zero-MainComponent test boundary.

### 4. `handleClipTrigger`'s post-trigger stale-clip read (packet's INFERRED item #7) — ignored, not worsened
Diffed the exact hunk: the only change inside `handleClipTrigger` is replacing
`layer->triggerClip(column);` with the `forcedSnap` computation +
`layer->triggerClip(column, forcedSnap);` (MainComponent.cpp:3708-3721). The
block at :3730-3757 that calls `layer->getActiveClip()` *after* the trigger
call, and seeks the player to the live beat phase, is byte-for-byte untouched.
So: this fix round neither addressed nor worsened the original review's
INFERRED concern — it remains exactly as risky (or not) as it was, still
unverified at runtime, still worth a human check (see below). I did not have
a live build/GUI in this pass to resolve it either way.

## Things that did NOT regress (re-checked in light of the new cancellation code)
- **Exactly-once firing**: `SwitchDeckCmd::apply`'s cancel/restore loop always
  writes `-1`/`Off` on execute regardless of prior state (idempotent), so a
  redo-without-intervening-undo double-apply is a no-op, not a double-cancel
  bug. `Autopilot::processFrame`'s beat-crossing/drain logic is untouched by
  this diff.
- **BPM-unknown degradation**: `quantizeModeToForcedSnap` (MainComponent.cpp:
  22-30) is untouched by this fix round.
- **Threading**: grep-confirmed `Autopilot.cpp` never calls `clearActiveClip()`
  (only `triggerClip`, which already ran on the GL thread pre-fix for the
  empty-cell case — the 2 new field writes inside `clearActiveClip()` land on
  the exact same thread as the 3 field writes it already did before this diff,
  no new cross-thread surface). `SwitchDeckCmd::apply`'s new loop runs on the
  message thread, same as the rest of `apply()`.

## What only a human at the machine can verify
1. Queue a Quantize trigger on Deck A, click "Add Deck" (not switch-deck-tab)
   before the beat lands, then switch back to Deck A later — confirm the
   trigger fires unexpectedly (finding 1's gap, AddDeckCmd path). Same test
   with "Load Deck File" instead of "Add Deck."
2. Bind a MIDI pad to TriggerClip Momentary, queue it under Quantize, release
   before the beat — per my trace this should NOT reactivate the layer either
   way (the guard structurally prevents `clearActiveClip()` from firing here),
   so confirm my trace is right rather than assuming the fix "worked" for this
   specific repro.
3. With clip A active and clip B separately queued on the same layer, release
   a Momentary pad for A (or clear A's cell) and confirm whether B's queue
   silently drops — decide if that's the intended behavior (finding under
   item 2's "blanket cancel" note).
4. Re-run the original review's own 7-item human-only list plus its 3
   finding-specific repros (queue-then-switch-decks, MIDI-hold-release,
   beat-snap-on-cold-trigger) — none of that was superseded by this fix round.

METADATA: reviewer=rev-L5, builder_packet=rta-rev-L5-fix-2026-09-05, date=2026-09-05T00:00:00Z, machine=live-build-and-revert-verified
