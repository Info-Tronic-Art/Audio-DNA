# Undo v1 — Build Lane Ledger

Session: 2026-07-19 (secondary, slim boot). Spec: `.harmony/specs/undo-v1-spec.md`
(Boris-ratified Wave-2 MUST #1). Build order §6 steps 1-9. Baseline at lane start:
HEAD b4ee380, source tree clean, tests 114/114 (handoff claim — re-verify at first gate).

## Standing constraints
- NO push to remote (standing rule). Commit per step after gates pass — no batching.
- App launch ONLY via `open build/AudioDNA_artefacts/Release/Audio-DNA.app`; :7070 binds ~12s.
- Verification split: Builder implements → independent Reviewer reads source → Harmony
  runs behavioral gate (build + ctest + app-level checks). Builder never claims verified.
- Spec lines are pre-Wave-0; SYMBOLS authoritative. Known drift: Wave 0 G3 touched menu
  code; Wave 1-D fixed the Clear-Clips bug (spec risk #7 / row #19 note stale); Wave 1-C
  landed toVar fidelity (no dependency either way — v1 uses value copies).

## Step status
| Step | Work | Status |
|---|---|---|
| 1 | Plumbing + GL-fence empirical validation (risk #9) FIRST | DONE — Reviewer CLEAN + behavioral gate PASS; committed 7c8d286 |
| 2 | SetClipCmd + single-cell sites (#4-6,8,10-12) | DONE — review FINDINGS→fix→CLEAN (open()-failure edge verified safe); gate PASS ×2; committed 7921572 |
| 3 | SwapClipsCmd (#3) | DONE — review CLEAN (1 cosmetic nit fixed); gate PASS (ctest 130/130, [swap] 21/21, app health); committed daa9361 |
| 4 | Composites (#7,9,4-multi,25-26,19,23) | queued — NEXT SESSION START HERE (deferred at ~33% gauge per LONG-TASK DEFERRAL; see HANDOFF.md) |
| 5 | Layer ops (#13-18,20) + GL fence | queued |
| 6 | Deck ops (#21-24) | queued |
| 7 | Effect stacks (#27-29) performEdit + scope descriptor | queued |
| 8 | Trigger cmds (#1-2) + merge | queued |
| 9 | Tests + manual e2e checklist | queued |

## Decision log
- 2026-07-19: GL-fence validation outcome gates `withDeckDetached` usage in steps 5-6.
  If fence deadlocks or is unreliable → fallback per spec: plain unsynchronized writes
  (status-quo risk profile) for structure commands + escalate as spec deviation.
- 2026-07-19: **GL-fence VALIDATED — PASS.** 100/100 blocking fences from message thread
  under live ~110fps rendering; roundtrip max 15.6ms / mean 2.8ms; no deadlock; app
  responsive after. Spec risk #9 resolved → `withDeckDetached` GREENLIT for steps 5-6.
  Evidence: /tmp/undo-fence-validation.log (+ -final copy).
- 2026-07-19: Step-1 behavioral gate (Harmony, independent): diff scope == report (9 mod
  + 3 new, src/tests/CMake only); harness residue grep CLEAN; ctest re-run 118/118
  (114 baseline + 4 [undo]); app launch via `open` → /api/health ready ~10s. PASS.
- 2026-07-19: Builder deviations accepted pending review: (1) guarded jassert (headless
  Catch2 aborts on bare form — identical enforcement in app); (2) +juce_events on test
  target; (3) syncAfterModelChange as two overloads (nested-struct default-arg compile
  error); (4) withDeckDetached restores activeDeck_ by RE-RESOLVE not saved pointer
  (survives decks-vector reallocation — strictly safer than spec sketch).
- 2026-07-19: New gotcha captured: transient first-`open` CoreAudio/TCC stall
  (gotchas.md 2026-07-19 entry).
- 2026-07-19: Step-1 review CLEAN (all 8 focus areas; deviations corroborated).
  Committed 7c8d286 (local, no push). Reviewer carry-forwards folded into lane:
  (a) step-4 composites MUST check CompositeCommand::isEmpty() before perform;
  (b) UndoService resolve* unit tests — added to step-2 packet.
- 2026-07-19: Step 2 dispatched to warm builder undo-s1 (R3 round 2; respawn
  fresh if round 3 needed or context bloats). Packet adds: operator== field-
  completeness statement required (post-Wave-1-C Clip fields), risk-#4 reconnect
  guard in SetClipCmd apply path, single-cell #4 with optional early composite.
- 2026-07-20: Step-2 review cycle: FINDINGS (1 real bug — video replace-undo
  silent no-op) → fix (MediaReconnect file-compare) → targeted re-review CLEAN
  (open()-failure edge safe: failed reopen dies on discarded temporary; locking
  consistent; immunity claims for images/sequences source-verified). Committed
  7921572. Carry-forwards: (a) VideoPlayer::getFile() unlocked — message-thread
  only, comment rider in step-3 packet; (b) SwapClipsCmd numColumns snapshot
  closes the column-growth gap for the MOVE op (drop/FX column growth still
  waits for step 4).
- 2026-07-20: Step 3 → FRESH builder undo-s3 (undo-s1 retired: 3 R3 rounds,
  context carrying 2 packets + fix loop). Budget watch: ~25% at dispatch;
  drain off-ramp ~40% — reassess after step 3 lands; steps 4+ likely defer to
  next session per LONG-TASK DEFERRAL.

## Queued non-lane items (from handoff, deferred while build lane occupies build dir)
- Syphon.framework install + rebuild -DAUDIODNA_BUILD_SYPHON=ON (verify real publish)
- 10s UI eyeball (menus / 3-tab Prefs / new Sources rows) — ADD: Edit menu shows
  dynamic "Undo <desc>"/"Redo <desc>" with correct greyed/enabled state (native-menu
  probe was TCC-blocked headlessly); quick Cmd+Z after a drop = restores cell;
  drag-move a clip (incl. to a FAR empty column) → Cmd+Z → both cells + column
  count restore (step-3 gesture path, not unit-testable headlessly).

## Known gaps until later steps (accepted, tracked)
- numColumns growth from multi-FX/edge drops NOT undone until step-4 column ops land
  (extra empty columns persist after undo — builder risk (a), step-2 report).
- ~~Same-id replaceContent-of-video undo: player keeps new file~~ RECLASSIFIED
  2026-07-19 by reviewer: NOT a §8.4 boundary — silent wrong-video on undo (worst
  class: quiet failure). Fix in flight (file-compare in media hook + Renderer
  loaded-file query). Step-9 manual e2e MUST include: video→video replace → undo →
  OLD video visibly plays.
- Undo with no cell selected clears clip inspector (safety over UX) — surface to Boris
  at wrap if UX feels off.
