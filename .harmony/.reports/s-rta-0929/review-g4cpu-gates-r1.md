# Reviewer Verdict — g4cpu-gates-r1
STATUS: DONE
VERDICT: APPROVE

REVIEWED: adf9b8a..42e3c93ae76df9782a5448e99319db60183bec1b (lane/g4cpu, commits ad35de1 "test(...) attribution" + 42e3c93 "docs(...) lane report")

SCOPE NOTE: only c1 (TEST-ONLY attribution witnesses + a1 probe row) landed. c2/c3/c4/c2b/v5/c5-docs were
correctly NOT built — the lane stopped per the plan's own pre-registered rule 3.3 (ii)/(iii): residual 40.3 > 30
(rule ii) and, after an uncommitted/reverted c2 experiment re-derived the bar per G2 (bar=min(36,round(28.6+6))=35),
G_red 42.7 - bar 35 = 7.7 < 8 (rule iii) -> STOP. Arithmetic independently re-checked and correct. Because c2+ never
landed, most FOCUS items (ctest paint-key, g4 CPU gate rewrite, cadence-on-paint-counts wiring, v5, c4 stale-strip
gate) have no artifact to review in this diff — their absence is the disclosed, justified outcome, not a drop.

VERIFIED independently (read-only, in place; also re-checked file contents/DeckView.h std::min via a $TMPDIR copy):
- Full incremental build at HEAD (42e3c93) succeeds (nothing to rebuild -> already green); ctest --test-dir
  build-lane -j1 -> 100% tests passed, 0 failed, 920/920 (matches the lane report's claim).
- No ctest/CMakeLists.txt changes in this diff (confirms "ctest paint-key" gate genuinely doesn't exist yet —
  consistent with c2 not landing).
- probe-idle-paint.py/.json: only adds row a1_attribution (INFO-only, never PASS/FAIL) and an opt-in `passes=`
  PassLog param (default False at every pre-existing call site) — g4_routine's own PASS/FAIL logic body is
  byte-for-byte untouched; no existing threshold moved (g1/g2/g4/i1/i2/v* all unchanged).
- No `BORIS_DECISIONS` citation anywhere in the diff (G5 respected).
- CLAUDE.md untouched, 24,980 B (<=25,000; G9/Q4 respected).
- All new instrumentation is TEST-ONLY (`#if AUDIODNA_TEST_SERVER`) or a relaxed-atomic counter bump beside an
  already-existing repaint()/setValue() call; no new mutex, no new timer, no new heap allocation on any hot path;
  nothing touches the audio callback or render/GL thread (all edits are message-thread UI code).
  `handleDebugUiPasses` is correctly registered/defined inside the pre-existing `#if AUDIODNA_TEST_SERVER` block in
  ApiServer.cpp (new route + handler both gated).
- Working tree clean beyond the reviewed commits; no stray `.venv`, no env-var hook, no leftover instrumentation
  outside TEST_SERVER; build-lane/__pycache__ are pre-existing gitignored artifacts.

FILE: src/ui/UiPaintCounters.h
  [OK] Patterns: new counters/ring buffers follow the exact idlepaint-era shape (relaxed atomics, fixed-size ring,
       no allocation); `Src` bit-stamping (G3) is a clean, minimal addition.

FILE: src/ui/RoutinePad.cpp / .h
  [OK] Spec fidelity: `paintContent`'s sweep formula is untouched (2.7 MUST-NOT-CHANGE); `samePad` (the RED
       behaviour) is deliberately still present since c2 wasn't built.
  [NIT] DRY: `sweepWidthOf()` (the witness helper) duplicates the inline sweepW formula inside `paintContent`
        rather than paintContent calling the helper. Plan 2.2 explicitly specifies this duplication for c1
        ("the tick counter compares ... inline"), and it's TEST-ONLY, so not blocking — worth collapsing when/if
        c2 lands and paintKeyOf subsumes both.

FILE: src/ui/TopBar.cpp / .h
  [OK] `getWheelRepaintBounds()` reproduces the prior inline rect expression exactly — refactor-only, verified by
       diff (no behavior change).

FILE: src/ui/DeckView.h
  [OK] `getStripColumnBounds()`/`getRoutinePadRowBounds()` are pure geometry helpers, no side effects.
  [NIT] `std::min` is used without an explicit `<algorithm>` include (relies on transitive include via JUCE headers
        already used in this codebase's style elsewhere) — compiles clean today (verified), flag only if a header
        reorg ever drops the transitive include.

FILE: src/ui/SignalBar.cpp / SignalStrip.cpp/.h
  [OK] `updateValue` now returns bool but the caller (`SignalBar::timerCallback`) discards it for repaint decisions
       and still calls the bar-wide `repaint()` — matches the report's claim "the bar still repaints whole" (c4 not
       built). No behavior change.

FILE: src/api/ApiServer.cpp/.h, src/ui/NativeLayerHost.mm, LayerInspector.cpp, UniversalParamControl.cpp,
      ClipInspector.cpp, MainComponent.cpp/.h
  [OK] Straightforward, consistent instrumentation; `paintOverChildren` correctly fires even when JUCE skips
       `paint()` for opaque-covered clips (this is what let the report catch the "pad-only pass still costs ~1.2ms"
       finding — a genuinely useful diagnostic result).

FILE: .harmony/.reports/s-rta-0929/g4cpu.md
  [OK] Honest, disk-cited report; STOP rationale matches the plan's pre-registered rule verbatim; risks (R1-R4),
       the stray `yes` process, and the outlier launches (R2) are disclosed rather than smoothed over.
  [SHOULD] Measuring c2 "as an experiment" after rule (ii) already said STOP is a mild process deviation (rule ii
       says stop AFTER c1 and report, not "try one more lever off the books first"). It was disclosed, never
       committed, and reverted, and it materially informs Harmony's ruling (section 6) rather than shipping
       anything — so not blocking, but Harmony may want to tighten "STOP" wording in future plans if this pattern
       recurs.

SUMMARY: 13 files reviewed (12 source + 1 report/probe-tooling set), 0 blocking issues, 3 suggestions (1 SHOULD
process note, 2 NITs). No defect ships; the absent gates (ctest paint-key, g4 CPU gate, v5, c4 stale-strip) are a
correctly-justified STOP, not a drop — arithmetic independently re-verified. ctest 920/920 and clean build
independently reproduced.
METADATA: reviewer=reviewer-g4cpu-gates-r1, builder_packet=g4cpu round1, date=2026-09-29
