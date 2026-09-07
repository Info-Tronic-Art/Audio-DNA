# Reviewer Verdict — rta-rev-L5-final-2026-09-05 (RealTimeAudio, L5 Quantize completion round)
STATUS: DONE
VERDICT: PASS_WITH_FINDINGS

FENCE: confirmed exactly src/model/Layer.h, src/model/Deck.h, src/core/DeckCommands.h,
src/MainComponent.cpp, tests/test_undo_commands.cpp (graphify-out/* dirty, tooling,
ignored). Machine independently re-verified: `cmake --build . --target test_undo_commands`
clean (no recompile needed — matches team-lead's "no file changed" claim), full
`./tests/test_undo_commands` 438/438 assertions / 67/67 cases, `ctest` 219/219 including
the new named `AddDeckCmd: cancels a pending trigger...` case. Ran two adversarial
build-revert-rebuild-restore cycles (below); repo diff --stat matches the pre-review
state byte-for-byte afterward (confirmed via diff + md5 on the restored test file).

## 1. Are BOTH paths A and B covered? YES, both, empirically confirmed
**Path A (`AddDeckCmd::execute()`, DeckCommands.h:749/715):** first-do branch now
calls `cancelledOnAdd_ = cancelPendingTriggers(comp->decks[priorActiveIndex_])`
BEFORE `comp->activeDeckIndex = addedIndex_`; redo branch calls
`applyPendingTriggerCancellation(..., cancelledOnAdd_, false)`. I reverted only the
first-do cancel call, rebuilt, and reran the new test: it FAILED exactly as expected
(`REQUIRE( ...pendingTriggerColumn == -1 )` → `5 == -1`); restored the file, rebuilt,
confirmed 438/438 green again. Real, load-bearing.

**Path B (`appendDeckFromFile`, MainComponent.cpp:2762-2764):** now calls
`cancelPendingTriggers(*leavingDeck)` immediately before
`composition_.activeDeckIndex = composition_.appendDeck(...)`, correctly discarding
the cancelled list (this path is a raw, non-undo-tracked model swap — there is
nothing to restore on undo because there is no undo command here, matching
`loadComposition`'s own no-undo-tracking precedent). Confirmed by trace only (this
path needs a live GUI file-drop to exercise at runtime, consistent with the prior
review's own human-verification carve-out for GUI-only paths) — the code shape is
identical to path A's cancel call and correctly ordered before the reassignment.

## 2. Is there a FOURTH such path? YES — found one, empirically reproduced
Swept every write to `activeDeckIndex` across `src/` (13 sites). Eleven are either
(a) already covered (the two SwitchDeckCmd::apply / handleDeckSwitch / AddDeckCmd
sites), or (b) genuinely safe because the OLD active deck is destroyed wholesale in
the same operation (RemoveDeckCmd::execute()'s clamp — the erased deck's Layer
objects, and any pending trigger they held, are destroyed with it, same reasoning
the prior review already validated for this exact command), or (c) building a
brand-new `Composition` object that then wholesale-replaces `composition_`
(`loadComposition`'s `composition_ = std::move(incoming)`, `Composition::initDefault`,
`fromVar`/deserialize, `CompositionLoad.h`'s repair) — none of these leave a live
deck "abandoned in place," they destroy it, so there is nothing to freeze.

**But `RemoveDeckCmd::undo()` (DeckCommands.h:877) is a real, unfixed 4th path:**
`comp->activeDeckIndex = priorActiveIndex_;` reactivates the deck being restored,
deactivating WHATEVER deck happens to be active at the moment undo runs — with no
call to `cancelPendingTriggers`/`applyPendingTriggerCancellation`. Repro (reachable
via the ordinary "Remove Deck" menu action + Ctrl+Z, both linear-undo-tracked):
remove the active deck (clamps active to some other existing deck), arm a Quantize
trigger on that now-active deck, then Undo the removal — the deck being deactivated
by the undo keeps its `pendingTriggerColumn` live. I wrote a temporary adversarial
TEST_CASE reproducing exactly this (appended to tests/test_undo_commands.cpp,
built, ran, then fully reverted — file diff now byte-identical to pre-probe,
confirmed via md5): it PASSED asserting the bug (`comp.decks[1]...pendingTriggerColumn
== 5` after undo, i.e. NOT cancelled), 438/438 assertions total including the probe.
This is pre-existing code, untouched by this diff (not a regression this round
introduced), but it is squarely the "is there a fourth path" question the packet
asked — worth a follow-up fix using the same two helpers, same shape as path A's fix.

## 3. Undo correctness for AddDeckCmd — CORRECT, traced against the actual code
Confirmed `priorActiveIndex_ < addedIndex_` always holds (AddDeckCmd only appends),
so neither the redo-branch's `insert()` (shifts indices >= `at` right, `priorActiveIndex_`
is always below `at`) nor undo's `erase(addedIndex_)` (only removes an index strictly
above `priorActiveIndex_`) ever moves the deck `cancelledOnAdd_`/`applyPendingTriggerCancellation`
operate on — the indexing stays valid across execute → undo → redo. Undo restores
the cancelled trigger; redo re-cancels it. Verified test-side too: the new test's own
NOTE about never caching a `Layer&`/`Deck&` across push_back/insert/erase (re-resolving
`comp.decks[0].getLayer(0)` fresh each assertion) is accurate — those calls can
reallocate `comp.decks`, and the test correctly avoids the dangling-reference trap.

## 4. The helper — correctly placed, correctly used, no wrong-call caveat found
`cancelPendingTriggers`/`applyPendingTriggerCancellation` (DeckCommands.h, new) is a
clean capture/restore pair, no blanket-cancel side effect this round (contrast the
FIX round's `clearActiveClip()` blanket-cancel finding — irrelevant here: this
helper acts per-layer, on the deck actually being left, only for layers that
already had `pendingTriggerColumn >= 0`, never cancelling an unrelated layer or
column). All three call sites (SwitchDeckCmd, AddDeckCmd, appendDeckFromFile) are
genuine deck-DEACTIVATION points — I found no caller among the three, nor among the
eleven other `activeDeckIndex` writes, for which cancelling is semantically wrong.

## 5. The new test — REAL
Drives the production `AddDeckCmd` command directly via `mgr.perform(...)`, asserts
concrete values (column 5, `BeatSnapMode::Bar`), never reaches past the command into
a helper. Empirically confirmed it fails pre-fix: reverted only the first-do cancel
call, rebuilt, reran — failed with `5 == -1` at the exact assertion the packet cares
about; restored, reconfirmed green.

## 6. Regression check — clean
Full suite 438/438 (was presumably 437 pre-round; team-lead's ctest delta 218→219
matches one new registered test). `ctest` 219/219 independently reproduced. Traced
that this round did not touch `handleDeckSwitch`'s six callers, `clearActiveClip()`,
or the five tests from the prior fix round — diff for `Layer.h`/`Deck.h` in this
round is unchanged from the fix-round diff already reviewed (both prior reports'
findings on those files stand; nothing here supersedes them).

## Not in scope for this round, restating from the FIX review since it's still open
Round 2's own report already flagged: (a) `clearActiveClip()`'s cancel has zero
test coverage, (b) the blanket single-slot cancel can silently drop an unrelated
queued trigger on a different column, (c) the MIDI-Momentary-release repro named in
the original finding cannot structurally reach `clearActiveClip()`. None of that was
touched by this round's diff and none of it is newly wrong — carrying forward, not
re-litigating.

FENCE compliance: only the five named files inspected for behavior; `RemoveDeckCmd`
finding is inside `src/core/DeckCommands.h` (in-fence for review purposes per the
packet's explicit "sweep independently... across src/" instruction) even though the
diff itself doesn't touch that function.

METADATA: reviewer=rev-L5-final, builder_packet=rta-rev-L5-final-2026-09-05, date=2026-09-05, machine=live-build-revert-restore-verified (test_undo_commands 438/438, ctest 219/219, adversarial probe reproduced-then-reverted for both AddDeckCmd fix and the RemoveDeckCmd::undo() gap)
