# Reviewer Verdict — g4cpu-gates-r2
STATUS: DONE
VERDICT: APPROVE

REVIEWED: 42e3c93ae76df9782a5448e99319db60183bec1b..c1d216c630a46cb3eb814c1f7279095124eb7ef2
(lane/g4cpu, worktree .claude/worktrees/rta0929-g4cpu; commits 2df6876 "test(...) v5_routine_identity"
+ c1d216c "docs(...) fix-round report"). This is the g4cpu-fix round answering critic-g4cpu-r1.md +
review-g4cpu-gates-r1.md + review-g4cpu-juce-r1.md (round 1, all three already APPROVE/PASS on 42e3c93).

SCOPE NOTE (re-verified independently): git diff --stat 42e3c93..c1d216c touches exactly 8 files:
3 source files (MainComponent.cpp, ApiServer.cpp, UiPaintCounters.h — 1 line each), probe-idle-paint.py/.json,
APP-INVENTORY.md, and two report .md files. c2/c3/c4 (the CPU-reducing levers) are still NOT in this tree —
`git diff --stat ad35de1..42e3c93` independently re-confirms 0 code files, 1 doc file — so most of the FOCUS
list (ctest paint-key compile-RED, g4 CPU gate rewrite, cadence-on-paint() wiring, c4 stale-strip gate) has
no artifact to review in THIS diff: the lane stopped after c1 per plan rule 3.3(iii) in the prior round
(RED margin 7.7 < 8, g4cpu.md:6,144) and that STOP was already reviewed/approved (review-g4cpu-gates-r1.md).
This round's job is narrower: did the fix honestly answer the two MUSTs (false CPU claim; missing v5 pair)
without fabricating a CPU-reduction claim or a toothless test.

VERIFIED independently (read-only; disk-cited):
- MUST 1 (false "CPU drops" claim) is correctly REFRAMED, not asserted differently: grep for "BORIS_DECISIONS"
  across g4cpu-fix.md, g4cpu.md, probe-idle-paint.py returns nothing (G5 respected); the new claim in
  g4cpu-fix.md section 1 is explicitly "c1 + g4cpu-fix change nothing Boris sees", never "CPU drops". No
  code in this diff reduces CPU (all 3 source hunks are 1-line `previewRect`/`preview_rect` plumbing under
  `#if AUDIODNA_TEST_SERVER`, confirmed by reading the surrounding guard blocks in all three files — the
  guard is open from MainComponent.cpp's `recordUiGeometry()` function start, from ApiServer.cpp:1678
  (no `#endif` before `handleDebugUiPaint` at :1756), and inside UiPaintCounters.h's existing TEST_SERVER
  struct region).
- MUST 2 / SHOULD 5 (no v5 pair; frame doesn't show a routine playing) FIXED with real teeth, verified by
  reading the row's logic (`.harmony/probe-idle-paint.py` `row_v5`/`v5_shoot`), not just trusting the printed
  log:
  - Honest SKIP: the position-landed check (`abs(float(p) - want[k]) > c["positionTol"]`) runs for BOTH the
    BEFORE and AFTER arm BEFORE any ok/no call and `return`s immediately on failure — the row cannot silently
    fall through to a PASS on clock drift.
  - K2 identity (P1, P2): BEFORE vs AFTER masked only by `fps_mask` + `signalbar_rect` (same masking
    convention as v1/v3/v4) — real 0 px / 0 px measured in both the RED (main-vs-main, establishing the
    comparator's own noise floor at 0) and GREEN (build-lane-vs-main) runs (g4cpu-fix.md section 2, verbatim
    logs cross-checked against the row's code path).
  - Teeth has real bite: `cl` (violation clusters between this build's own P1 vs P2) is required truthy —
    an accidentally-frozen/stale routine render (no paint update between P1/P2) yields `cl == []`, which is
    falsy, so the gate correctly FAILs that failure mode, not just a happy-path re-assertion. Stray clusters
    outside `pad_row_rect`/`strip_col_rect` (widened +/-2 px) also FAIL. The preview region is masked OUT of
    the stray check (a second `identity()` call with the complement mask reports it as INFO only) — this is
    the intentional "the layers' opacity render legitimately changes, don't let it count as noise" design,
    confirmed by re-deriving the complement-rect math (`outside_pv`) by hand.
  - `cue_px`'s hue/sat/value decode (68-100 deg, sat>0.5, value>0.6, `mx>153`) is a verbatim match of
    `probe-routine-display.sh`'s existing d9 decoder (`grep -n hue .harmony/probe-routine-display.sh` line
    130-135) — the "reused from d9" claim is TRUE, not just asserted. `kRoutineCue = 0xffb4ff2e` (LookAndFeel.h:36)
    hue-converts to 81.5 deg, sat 0.82, val 1.0 — inside the decoder's window, confirming the color-claim math.
  - `minCuePx=300` vs measured 12213-14973: a loose but harmless sanity floor (existence check only; the
    shift itself is proven by the separate teeth check, not this line) — consistent with d9's own +300
    convention, not a new weakness introduced here.
- Existing rows never re-thresholded: `probe-idle-paint.json` diff adds only the new `"v5": {...}` block;
  the `identity`/`v3p`/`v4` blocks are unchanged context lines. `g4_routine`'s PASS/FAIL body is untouched
  (confirmed: no diff hunk touches `row_g4`/`def row_g4` in this round).
- No new ctest: `tests/CMakeLists.txt` has 0 diff lines in this round; g4cpu-fix.md's own claim "ctest 920/920,
  count unchanged" is consistent with that.
- CLAUDE.md untouched, 24,980 B (<=25,000 cap respected). No audio-callback/render/GL-thread file touched;
  all edits are message-thread UI/test-tooling. No new mutex, no new timer, no heap allocation added.
- Screen-safety and stray-artifact claims (0 Output windows, no `.venv` symlink, no TEMPORARY hook left) are
  consistent with the diff containing no such artifacts.

FILE: src/ui/UiPaintCounters.h
  [OK] `previewRect[4]` follows the exact existing idiom (`deckRect`/`padRowRect`/etc.), correctly inside the
       pre-existing `#if AUDIODNA_TEST_SERVER` struct region. Not a duplicate mechanism (EXCESS_DUP) — a
       distinct mask target the v5 row genuinely needs.

FILE: src/MainComponent.cpp
  [OK] `put(c.previewRect, previewPanel_.getBounds())` — one line inside `recordUiGeometry()`, whose entire
       body is `#if AUDIODNA_TEST_SERVER`-gated; `previewPanel_` is a pre-existing, unconditionally-declared
       member (MainComponent.h:301) — no new dependency, no behavior change outside the guard.

FILE: src/api/ApiServer.cpp
  [OK] `preview_rect` property added inside `handleDebugUiPaint`, itself inside the `#if AUDIODNA_TEST_SERVER`
       block opened at :1678 with no intervening `#endif` — correctly test-only.

FILE: .harmony/probe-idle-paint.py / .json
  [OK] New default row `v5_routine_identity` is additive only (registered in `ALL_ROWS`, dispatched in the
       row-selection `if/elif` chain); no existing row's dispatch or threshold touched.
  [OK] Docstring header (v5 description) accurately matches the implementation (checked line-by-line against
       `row_v5`/`v5_shoot`/`cue_px`/`regions_png`) — no overclaiming vs what the code does.

FILE: .harmony/.reports/s-rta-0929/g4cpu-fix.md, g4cpu.md (appended), APP-INVENTORY.md
  [OK] Findings table (1 MUST/2 MUST/3 SHOULD/4 SHOULD/5 SHOULD) is a correct synthesis of THREE round-1
       verdicts (critic-g4cpu-r1.md MUST 1/MUST 2/SHOULD "frame"; review-g4cpu-gates-r1.md SHOULD "c2 after
       STOP"; review-g4cpu-juce-r1.md SHOULD "G3 window empirical") — cross-checked against all three source
       files; not fabricated, and each verdict/action cell matches its source finding's substance.
  [OK] Bound-knob non-completion (section 3) is disclosed with a concrete, checked reason (no REST/composition
       selection path; grepped ApiServer routes / DeckView::selectLayer myself) rather than silently dropped —
       matches UNKNOWNS-NOT-DONE. STATUS: DONE_WITH_CONCERNS is the right header for an honest partial.
  [OK] No BORIS_DECISIONS citation anywhere in the new/changed docs (G5).

SUMMARY: 8 files reviewed (3 one-line source hunks, 2 probe-tooling files, 3 report/doc files), 0 blocking
issues, 0 suggestions beyond what round 1 already filed (unaddressed but non-blocking: the RoutinePad.cpp DRY
NIT and DeckView.h std::min NIT from review-g4cpu-gates-r1.md — untouched by this diff, so out of scope here).
Confidence: VERIFIED for all claims above (diff read directly, guard blocks read directly, hue math
re-derived by hand, teeth/SKIP logic traced by hand against the printed RED/GREEN logs) — no claim taken on
report-recall alone. The one thing NOT independently re-run: an actual `ctest`/build execution (the source
change is a single trivial getter-bounds call under an already-exercised TEST_SERVER path; risk of a compile
regression here is judged negligible and the report's own 920/920 claim is consistent with everything else
checked).
METADATA: reviewer=reviewer-g4cpu-gates-r2, builder_packet=g4cpu-fix, date=2026-09-29
