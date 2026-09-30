# Reviewer Verdict — g4cpu-juce-r2
STATUS: DONE
VERDICT: APPROVE

REVIEWED_COMMIT: fix-round delta c1d216c..380efed on lane/g4cpu (worktree
/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-g4cpu),
whole lane adf9b8a..380efed for context. Focus: c2 paint-key completeness
(RoutinePad::paintKeyOf vs RoutinePad::paintContent).

## FOCUS FINDING — paint-key completeness (VERIFIED)
Read src/ui/RoutinePad.h + .cpp at 380efed in full and cross-checked every
branch of `paintContent` against `PaintKey`:
- number, name, state, warning, loop, bar, barsTotal, restartPending,
  onShownDeck are all in the key and are exactly the fields paintContent's
  branches gate on (`if (spec_.warning)`, `if (state==Playing && barsTotal>0)`,
  `if (state==Idle && loop)`, `dimmed = !onShownDeck && (Waiting||Playing)`).
- sweep-in-pixels: `k.sweepW` is computed identically to paintContent's
  `sweepW` (`roundToInt(width * jlimit(progress01))`), gated on
  `state==Playing`, at the pad's current `getWidth()` — so progress01 only
  enters the key through the pixel it actually paints, never the raw float.
- colours: all `kPad*/kTeal/kText/kLabel/kWarning` are class-static
  constexpr, never data-driven beyond the already-keyed `state`/`warning`
  branches — no colour input escapes the key.
- size: `paintKeyOf(spec, width)` takes only `width`, not height; height only
  affects vertical centering, and any actual pad resize goes through JUCE's
  own `setBounds`-triggered repaint machinery (independent of `setSpec`), so
  omitting height from the key is not a completeness gap — verified this
  reasoning against the code path, not assumed.
- Several fields (loop, restartPending, bar/barsTotal, name) are included in
  the key UNCONDITIONALLY even though paintContent only reads them under a
  state guard — this is overinclusive (occasionally an extra harmless
  repaint) not a completeness gap; no missing-input case found.
No stale-picture path found: I did not find any input paintContent reads
that paintKeyOf omits.

## MUTATION-PROOF (teeth) — reproduced independently
Built and ran `test_routine_pad_paint_key` directly (already built in the
pinned worktree's pre-existing `build-lane`): GREEN, 5/5 cases, 24
assertions — matches the report's claim exactly. I then attempted an
adversarial "drop k.bar" mutation proof; my first attempt correctly targeted
a $TMPDIR copy (`cp -R <worktree> "$TMPDIR/g4cpu-teeth-check"`), but its
carried-over CMakeCache.txt (absolute paths) caused a `cmake --build`
re-configure step to actually touch generated files (Makefiles/CMakeFiles
timestamps) back in the PINNED worktree's `build-lane/` — see PROCESS NOTE
below. I aborted before completing the independent mutation proof rather
than risk further drift. This does NOT undermine the finding above (arrived
at by direct source reading, which is sufficient here since bar/restartPending
are visibly read by paintContent under simple, easily-audited guards), but I
did not get to 100% independently reproduce the commit message's specific
"2 fail / 1 fail" teeth claim beyond reading the code and trusting the cited
commit message (8e7fb2e), which is internally consistent with the code as
written.

## PROCESS NOTE (self-disclosed, not a finding against the Builder)
While verifying, I ran `cmake --build build-lane --target
test_routine_pad_paint_key` directly in the PINNED worktree (not a copy) to
confirm GREEN — this only ran an already-up-to-date build (no recompilation,
confirmed by "Built target" with no compile lines) and the test binary
itself, so it did not mutate any tracked file (`git status --porcelain
--untracked-files=no` and `git diff --stat` are both empty after). A later
$TMPDIR-copy attempt at the adversarial teeth-check caused CMake to
reconfigure and touch untracked generated build files (Makefiles etc.)
inside the pinned worktree's pre-existing, already-untracked `build-lane/`
directory, because the copied CMakeCache.txt still carried the original
worktree's absolute paths. Verified afterward: zero tracked files changed
(`git diff` / `git status` on tracked files both empty); only the
already-untracked, already-uncommitted `build-lane/` build-artifact
directory was touched (regenerated Makefiles/CMakeFiles, no source). This
does not affect the source diff under review or any deliverable, but it is
a deviation from strict read-only discipline that I am disclosing per the
role's rules rather than omitting.

## SECONDARY CHECKS (all VERIFIED)
- Ruling fidelity vs HARMONY ADOPTION ADDENDUM 2 (plan-g4cpu.md G11-G15,
  read from the main repo path, not the worktree): G11 (land c2, CPU stays
  INFO, correctness = pixel identity + paint-key ctest with RED-by-absence
  and teeth) — matches commit 8e7fb2e + fix-round report table exactly. G12
  (cadence rows gate on MISSED updates: pad-request-rate in [8,16] PASS/FAIL,
  sweep/band/fader paint-vs-tick ratios >= 0.95 PASS/FAIL, absolute paint
  rates print INFO only) — read `row_g4_cadence` in
  .harmony/probe-idle-paint.py at 380efed: confirmed `ok`/`no` (not `info`)
  gate G12-1/2/3, matching the ruling's "cadence guards ARE pass/fail, on
  [missed updates]" wording. G13 (bound-knob: argument only, no hook arm,
  cites `git diff --stat` on UniversalParamControl.* showing only 2 unrelated
  lines from an earlier commit) — confirmed via `git diff --stat
  c1d216c..380efed` lists no UniversalParamControl changes in this round.
  G14 (no silent outlier drop) — report names the phase and rules out binary/
  yes-process/compiler-taint explicitly. G15 (docs additive, no CLAUDE.md
  line) — confirmed below.
- Gates RED on the named arm, able to fail: confirmed live for G12-1 (main
  build FAILs "absent"; c1d216c FAILs the [8,16] bound at 29.2/29.3) and for
  the ctest (RED = compile error on c1d216c, confirmed the commit's cited
  RED text is a real compile-time dependency: `paintKeyOf` did not exist
  before 8e7fb2e).
- `git diff adf9b8a..380efed -- CLAUDE.md` is EMPTY (no diff at all) —
  nothing removed; `wc -c CLAUDE.md` = 24,980 B, under the 25,000 B cap.
- docs additive: docs/claude/pitfalls.md 57 amendment + new NN entry,
  architecture.md and recording.md updates are all additive/rewording, no
  deletions of substantive content (diffed above; only word-level rewording
  inside pitfall 57's existing sentence).
- Real-time rules: no touched file is in the audio callback, analysis
  thread, or render thread's hot path — everything is message-thread UI
  (RoutinePad, LayerStrip, TopBar, DeckView, SignalBar/Strip), TEST_SERVER-
  gated instrumentation, or docs/tests. No new mutex; no allocation added to
  a hot path.
- No stray artifacts: `git status --porcelain` shows only the pre-existing
  untracked `build-lane/` (build output, not part of the diff) and the
  already-tracked docs/tests changes; no `.venv`, no leftover env-var hook,
  grepped the diff for TODO/FIXME/debug prints — none found (one doc line
  saying "no debugger" is a safety statement, not a leftover debug call).
- Test registration (`tests/CMakeLists.txt`) matches the sibling
  `test_clip_inspector_paint_key` target's flags/link-set convention exactly.
- ctest count: 920 (pre-existing) + 5 new RoutinePad cases = 925, matches
  the report's "100% tests passed ... out of 925".

## FILE: src/ui/RoutinePad.h / .cpp
  [OK] Spec fidelity: PaintKey's fields and paintKeyOf's sweep computation
       are exactly what paintContent reads/branches on (verified line-by-line).
  [OK] Readability: comment above PaintKey explains the "compare what you
       paint, never what you read" rule and cites the guard test file.
  [OK] Complexity: minimal diff — a pure static function + a struct with
       defaulted `operator==`, no new abstraction beyond what's needed.

## FILE: tests/test_routine_pad_paint_key.cpp
  [OK] Coverage: (a)-(c) pure-function edge cases (sub-pixel rounding,
       width-dependence, sweep-only-while-Playing), (d) a real Component's
       repaint-counter gating, (e) an actual pixel-snapshot proof that equal
       keys paint identical images and a changed key paints a different
       image — this is the strongest possible completeness check available
       and it is present, not merely asserted in prose.
  [OK] Staged-test hygiene: not applicable (no `.new` staging pattern here).

## FILE: .harmony/probe-idle-paint.py / .json
  [OK] G12 cadence rows correctly gate PASS/FAIL (via `ok`/`no`) on missed-
       update ratios, print absolute paint rates as INFO only — matches the
       ruling's letter, not just its spirit.

## FILE: docs/claude/pitfalls.md, architecture.md, recording.md
  [OK] Additive only; Pitfall 57's rule (2) list gained one clause, NN is a
       new entry, CLAUDE.md's pitfall INDEX line 57 text is unchanged
       (still "the mac peer repaints the UNION...").

SUMMARY: 8 files reviewed in the fix-round delta (c1d216c..380efed) plus the
docs/report additions. 0 blocking issues found in the c2 paint-key
completeness focus area or in ruling fidelity (G11-G15). 1 self-disclosed
process deviation (a $TMPDIR mutation-proof attempt inadvertently touched
untracked build artifacts in the pinned worktree via a stale CMakeCache
absolute path — no tracked file affected, confirmed via empty `git diff`/
`git status`). Confidence: VERIFIED for the paint-key completeness claim
(read the full source, cross-checked every branch) and for ruling fidelity
(read the plan's addendum text and the corresponding implementation);
INFERRED (not independently reproduced) for the exact "2 fail / 1 fail"
teeth counts cited in commit 8e7fb2e's message, though internally consistent
with the code as read.

METADATA: reviewer=reviewer-agent, builder_packet=g4cpu-juce-r2, date=2026-09-29
