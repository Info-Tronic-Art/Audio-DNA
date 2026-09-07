# Reviewer Verdict — L1b-media-leak-delta-s-rta-0904
STATUS: DONE
VERDICT: PASS

Repo: /Users/boriskarpman/projects/RealTimeAudio. Reviewed `git diff -- src/ tests/`
against a SETTLED tree (see METHOD NOTE below — the tree moved mid-review; all
citations below are from the confirmed-stable final state, verified stable
across 3 consecutive md5 checks ~6-8s apart).

## VERDICT SUMMARY
All three named defects are FIXED, correctly, with proof. Undo correctness
holds for both new paths, no double-dispose bug. The retire-list drain is a
shared helper, not duplicated. Both PRODUCTION call sites pass the REAL
dispose hook, not a noop — the lane is NOT decorative. Test quality is
genuinely strong: falsifiable exact-count assertions exist for both MAJOR
fixes, plus two NEW adversarial guard-refusal tests that specifically catch
the "dispose fired even though the erase guard refused" bug class. One LOW,
non-blocking suggestion below (test-fixture completeness, not a code defect).

## DEFECT 1 — RemoveLayerCmd never disposed: FIXED
DeckCommands.h:485-514 (`execute()`). The erase (line 497) and the dispose
loop (line 508-511, `for (const auto& c : removed_.clips) if (c.has_value())
disposeHook_(*c);`) are now INSIDE the same `if (deck->layers.size() > 1 &&
...)` guard block (opened line 494, braced 496/512) — dispose only fires when
the layer actually was removed. `removed_.clips` is the flat per-column cell
vector for the WHOLE layer, so the loop covers every occupied cell in it, not
just the first. Undo (516-532) reconnects via `mediaHook_` over
`deck->layers[at].clips` (528-530) — already correct per the prior review,
unchanged here.

Test: `tests/test_undo_commands.cpp` "RemoveLayerCmd: remove restores the
full layer on undo (deep-equal)" — real counting hooks, `disposeCalls == 1`
on execute (clip 77), unchanged + `reconnectCalls == 1` on undo, `disposeCalls
== 2` on redo. PLUS a new dedicated test "RemoveLayerCmd: refuses when only 1
layer remains, does not dispose" — drives the deck to 1 layer, puts a clip in
it, and asserts `disposeCalls == 0` when the guard refuses the erase. This is
exactly the falsifiable test the prescribed fix needed and it exists.

## DEFECT 2 — RemoveDeckCmd had no media hooks at all: FIXED
DeckCommands.h:700-783. Ctor now threads both `mediaHook_` and `disposeHook_`
(703-704, 707-708). `execute()`'s dispose loop (753-757) is a NESTED loop —
`for (const auto& layer : removed_.layers) for (const auto& c : layer.clips)
if (c.has_value()) disposeHook_(*c);` — covering every occupied cell across
EVERY layer of the removed deck, and it is INSIDE the same guard block
(734-758) as the erase (737), so a refused removal (only 1 deck left) does
NOT dispose. `undo()` (762-783) reconnects with the mirror-image nested loop
over `comp->decks[at].layers` (778-781).

Production wiring: `MainComponent.cpp:4141-4144` — `RemoveDeckCmd` is
constructed with `makeClipMediaHook(), makeClipMediaDisposeHook()`, both the
real production hooks (same functions wired into RemoveColumnCmd elsewhere).
Confirmed via `grep -n "makeClipMediaDisposeHook()" src/MainComponent.cpp`
(7 hits, includes this site) and `grep -n "RemoveDeckCmd>" src/MainComponent.cpp`
(1 hit, line 4141, immediately followed by the real-hook line) — **NOT a
noop**.

Test: "RemoveDeckCmd: remove active last deck clamps active, undo restores
deck" — real hooks on richDeck's one clip (id 33, layer 0): `disposeCalls ==
1` on execute, unchanged + `reconnectCalls == 1` on undo, `disposeCalls == 2`
on redo. PLUS "RemoveDeckCmd: refuses to remove the last remaining deck" was
extended to give the sole deck a clip and assert `disposeCalls == 0` when the
guard refuses — same adversarial pattern as defect 1's new test.

## DEFECT 3 — openGLContextClosing() didn't drain retire lists: FIXED
Renderer.cpp:756, inside `openGLContextClosing()` (705-783): a single call to
`drainRetiredMedia()` — the SAME function called every frame from
`renderOpenGL()` (line 151). Confirmed NOT duplicated:
`grep -n "retiredVideoPlayers_\|retiredImageSequences_" src/render/Renderer.cpp
src/render/Renderer.h` shows exactly one push_back site (closeMediaForClip,
987/998) and one drain-and-release site (`drainRetiredMedia()`, 1004-1027,
called from the two sites above) — no second copy of the release loop to fall
out of sync.

## PRODUCTION CALL-SITE ANSWER (stated first, unambiguous)
Both new production call sites (`RemoveDeckCmd` MainComponent.cpp:4143,
`RemoveLayerCmd` MainComponent.cpp:4217-4218) pass the REAL
`makeClipMediaDisposeHook()` — confirmed by direct read and by grep. Neither
is a noop. The lane is functional in the shipping app, not decorative.

## TEST-QUALITY ANSWER
- 2 production sites: both real (above).
- Of the test call sites for RemoveLayerCmd/RemoveDeckCmd: the two deep-equal
  tests use REAL hooks with exact falsifiable counts (not "was called
  at-least-once" — exact `== 1`, `== 2` at each cycle phase, tied to a named
  clip id in the comment). The coordinate-shift and stale-index tests
  (layer/deck carries no clip) deliberately use `noopDispose()`, each with an
  inline comment stating why real coverage isn't needed there and pointing to
  the test that does carry it — consistent with the pre-existing `noopMedia()`
  convention audited in the first review.
- NEW since the first review: two guard-refusal tests (RemoveLayerCmd "refuses
  when only 1 layer remains", RemoveDeckCmd "refuses to remove the last
  remaining deck") that give the guarded-refusal case a real playable clip and
  assert `disposeCalls == 0`. This is a falsifiable, adversarial assertion
  that directly targets the "dispose fired unconditionally after a guard that
  may have refused" bug class — see METHOD NOTE.

## COLLATERAL DAMAGE
None found. `git diff --stat -- src/ tests/` shows only the 7 expected files;
every hunk in DeckCommands.h/Renderer.{cpp,h}/MainComponent.{cpp,h} maps to
one of the 3 fixes or their call-site wiring; ClipCommands.h is unchanged
from the mechanism the first review already passed (traps a/b, not
re-litigated here).

## METHOD NOTE — tree moved mid-review (T27)
Early in this review, `DeckCommands.h`'s `RemoveLayerCmd`/`RemoveDeckCmd`
dispose loops were OUTSIDE (after) the erase guard's brace — i.e.
unconditional: if the guard refused (only 1 layer/deck left), dispose would
still have fired on media that was still live in the model. Re-running `git
diff --stat` minutes later showed both files had grown (DeckCommands.h 95→103
lines, test_undo_commands.cpp 180→214 lines); md5 checks then confirmed the
tree settled. The settled state has this gated correctly (guard brace now
wraps both the erase AND the dispose loop) and ships the two new
guard-refusal regression tests described above. All findings in this report
are against the settled state, confirmed stable across 3 consecutive checks.
Flagging for the record since it's exactly the scenario HARMONY_GOTCHAS T27
describes — judge the final state, not a snapshot.

## ONE NON-BLOCKING SUGGESTION (LOW, test-fixture completeness)
The packet asked to confirm "the dispose loop covers EVERY occupied cell (a
deck has multiple layers — a loop that stops at the first layer... is a
partial fix that will gate green)." I verified this BY READING the code — the
nested loops have no `break`/early-return and are not sliced to
`layers[0]` — so the fix itself is correct. But every test fixture that
exercises real dispose counting only ever populates ONE occupied cell
(`richDeck()` sets a clip on layer 0 only; the RemoveLayerCmd deep-equal test
puts one clip in the removed layer). No test currently distinguishes "loops
over ALL layers/cells" from a hypothetical "checks only the first
layer/cell" regression — such a mutant would still pass every test in this
diff. Suggest (not blocking): extend `richDeck()` or add one clip to a second
layer (e.g. layer 2) in the RemoveDeckCmd deep-equal test, and a second
occupied column in the RemoveLayerCmd deep-equal test, asserting
`disposeCalls == 2` instead of `== 1`. Does not affect this verdict since the
source is independently verified correct.

METADATA: reviewer=rev-L1b, builder_packet=L1b-media-leak-delta-s-rta-0904, date=2026-09-05
