# Reviewer Verdict — rta-rev-L5-test7-2026-09-05
STATUS: DONE
VERDICT: PASS_WITH_FINDINGS
FILES: tests/test_undo_commands.cpp:2669-2691, src/model/Layer.h:303-325
METADATA: reviewer=rev-L5 (subagent), builder_packet=rta-rev-L5-test7-2026-09-05, date=2026-09-05, repo=/Users/boriskarpman/projects/RealTimeAudio

## Empirical steps taken
1. Baseline: `./tests/test_undo_commands "*clearActiveClip*"` -> 2 test cases, 14 assertions, all PASSED.
2. Recorded pre-state: `git status --porcelain src/model/Layer.h` empty (clean vs HEAD);
   md5(src/model/Layer.h) = 00dcc172eaa85be3501a8885d695c732.
3. Neutralized the fix: commented out the two lines at Layer.h:323-324
   (`pendingTriggerColumn = -1;` / `pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;`)
   inside `clearActiveClip()`. Left everything else (including the explanatory comment block)
   untouched.
4. Rebuilt `test_undo_commands` target, then full `AudioDNA` app target (Layer.h is a shared
   header; both consume it).
5. Ran full `ctest` (222 tests): 221/222 passed, exactly 1 failure:
   `192 - Layer::clearActiveClip: cancels a pending trigger too, even one queued on an
   unrelated column (Failed)`. No other test in the 222-test suite regressed.
6. Verbose failure output for the new test with the fix neutralized:
   - All prior REQUIREs in the test pass (activeClipColumn==0, clips[0]->playing==true,
     pendingTriggerColumn==2, pendingTriggerSnapOverride==Bar, activeClipColumn==-1,
     clips[0]->playing==false).
   - First failure: `tests/test_undo_commands.cpp:2689: FAILED: REQUIRE(
     L.pendingTriggerColumn == -1 )` with expansion `2 == -1`. Catch2 aborts the test case at
     the first failed REQUIRE, so the following assertion (line 2690,
     pendingTriggerSnapOverride) was not reached in this run — the fields are set together by
     the same two lines removed, so this does not weaken the finding.
7. Restored `src/model/Layer.h` from a pre-edit copy. Verified byte-identical:
   md5 after restore = 00dcc172eaa85be3501a8885d695c732 (matches step 2); `git diff
   src/model/Layer.h` and `git status --porcelain src/model/Layer.h` both empty.
8. Rebuilt `test_undo_commands` and reran the filtered suite: 14/14 assertions PASSED again
   (green). Rebuilt the full `AudioDNA` app target too, so the build tree matches its
   pre-review state.
9. Final `git status --porcelain` in the RTA repo shows only the same files as at the start
   of this review (`tests/test_undo_commands.cpp` + unrelated `graphify-out/*` artifacts) —
   the tree was left exactly as found.

## Answers

**Does the new test FAIL when the fix is neutralized?**
Yes. Confirmed empirically (step 6). It is NOT theatre — it is load-bearing.

**Do any OTHER tests fail when the fix is neutralized?**
No. 221/222 passed with only the new test failing. This CONFIRMS the prior reviewer's finding:
before this test existed, deleting the two-line fix was invisible to the suite; now it is
caught by exactly one test, which is the intended gap-closer.

**Does the test drive the real `Layer::clearActiveClip()`, or does it reach past it?**
It drives the real method directly: a real `Layer` object (`L`), real `triggerClip()` /
`clearActiveClip()` calls, no mocks or fakes involved. Confirmed by reading the test body and
by the fact that editing the production `.h` changed the test's outcome.

**Does the test's "even one queued on an unrelated column" claim hold, and is that behavior
correct or merely current?**
The claim holds exactly as stated: `Layer` has a single `pendingTriggerColumn` /
`pendingTriggerSnapOverride` pair (Layer.h:157-163), not one slot per column, so
`clearActiveClip()` clearing column 0 also drops a queued trigger armed for the wholly
unrelated column 2. The test's own inline comment ("The queued trigger on column 2 — never
itself cleared — is dropped too") is accurate and doesn't overclaim.

Whether this is CORRECT rather than merely CURRENT is a real open question, not a settled
design decision — flagging per the task's request:
- The lane's own commit message (29019fe) frames the fix as "a queued trigger stops outliving
  its context," and enumerates four abandonment paths it closed (deck switch,
  `clearActiveClip()`, `AddDeckCmd`/append, `RemoveDeckCmd::undo()`). All four are about a
  trigger surviving past a change to *the same* clip/column's context (e.g., a clip that gets
  cleared or deactivated). None of the four describe — and the commit message never discusses —
  a queued trigger for a *different, still-untouched* column being collaterally cancelled.
  That specific cross-column side effect is a consequence of the pre-existing single-slot
  field design, not something this lane's stated intent explicitly reasons about.
- Concretely: a performer with clip A live on column 0 who cues clip B on column 2 to fire on
  the next bar, then stops A (e.g. releases a momentary MIDI pad bound to column 0 — the exact
  scenario named in Layer.h's own comment at line 318), would silently lose their queued cue
  for B, which had nothing to do with A. That is very plausibly surprising to a performer in a
  live-performance tool, and it is a real, previously-undiscussed behavior change introduced
  by this lane (the cancellation is new; the single-slot field is not).
- The test pins this down as an assertion (REQUIRE pendingTriggerColumn == -1 after clearing
  an unrelated column), which means: if a future change scopes pending-triggers per-column
  instead of per-layer (arguably the more intuitive fix for the surprise above), this test
  will fail and force an explicit decision at that time. That's a reasonable thing for a test
  to lock in provisionally, but the test name and the code comment both assert this as if it
  were an intended feature ("even one queued on an unrelated column") rather than flagging it
  as an accepted side effect of a known architectural limitation (one pending slot per layer).
  Recommend: file this as a product/UX question (should pending-trigger be per-column?) rather
  than treat the test's assertion as the final word on desired behavior.

## VERDICT: PASS_WITH_FINDINGS
The new test is genuinely load-bearing: it fails when the fix is removed, nothing else in the
221-test remainder does, and it drives the real production method with no mocking. The one
non-blocking finding is a product-facing behavior question (cross-column trigger cancellation
inside a single-pending-trigger-slot `Layer`) that the test silently locks in as correct
without it having been explicitly decided — worth a product call, not a reason to reject the
test itself.
