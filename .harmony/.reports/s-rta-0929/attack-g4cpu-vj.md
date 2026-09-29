# ATTACK — plan-g4cpu (VJ / live-performer seat)

VERDICT: The plan's mechanism (c2 pad paintKeyOf) is sound and pixel-proven; its weakest link is c4
(SignalBar per-strip repaint), which has NO end-to-end pixel-identity gate at all, and the plan
misattributes a rejection rationale to a Boris ruling that does not exist in BORIS_DECISIONS.md.
The g4 stall-window bound is also loosened past what idlepaint just won. Approve c1/c2/c3 as gated;
require a real v1c-style live pixel gate before c4 ships, and fix the citation.

## MUST

1. **c4 (SignalBar per-strip repaint) has zero end-to-end pixel-identity coverage — exactly the
   surface (Beat/Bar/Mod live readouts) the persona cares most about.** Every K2 pixel gate the plan
   lists (v1, v1b, v3, v3p, v5) either explicitly excludes the whole SignalBar rect ("outside the fps
   mask + the SignalBar rect (Mod 1 is live, v1 S2's rule)", §2.4/§2.5) or only compares a FROZEN
   state (v1b, "unchanged"), which never exercises the changed/unchanged branch c4 adds. The only
   thing standing behind c4's correctness is a Catch2 unit test of `SignalStrip::updateValue()`'s
   return boolean (`test_signal_strip_painted.cpp`, §3.1.2) — never a screenshot proving the
   per-strip-repaint bar looks identical to the always-repaint bar while live. Verified against
   source: `SignalStrip::paintNormal` draws the meter fill and the 2-decimal text straight from
   `displayValue_` (SignalStrip.cpp:139-155), and the exact-float `Painted` key mirrors that — so the
   unit test can be right and a real rendering bug (stale strip a frame late, a layer-cache dirty-
   union miss) still ships invisibly to Boris during a show. Fix: add a v-row that diffs SignalBar
   pixels frame-to-frame across two LIVE (non-frozen, injected-changing) ticks, unmasked, BEFORE vs
   AFTER c4, PASS only if identical within K2 tolerance — the same shape v5 already does for the
   routine pad/strip column.

2. **Plan line 120 fabricates a Boris ruling.** It rejects V-fill quantization "(rejected by
   BORIS_DECISIONS 'how they look must not change')". `grep -in "look must not change\|pixel" 
   BORIS_DECISIONS.md` returns nothing — no such phrase, or a close paraphrase, exists anywhere in
   the file. The nearest actual ruling (BORIS_DECISIONS.md:350-351) is about the preview/output
   window's aspect ratio, unrelated to a fader's anti-aliased edge. The engineering call (don't
   change the look) may still be right, but it must stand on its own, not on an invented citation —
   this is exactly the kind of unverified claim Layer 0 forbids graduating to fact by repetition. Fix:
   cite it as the architect's own judgment, or find and cite the real ruling if one exists elsewhere.

## SHOULD

3. **g4's window-max PASS bound is loosened past what idlepaint already achieved.** §3.2 sets
   `window max median <= 8.0 ms` for the g4 gate. But §0's own re-derived facts say this exact
   fixture reads 5.2-5.6 ms on the current build (idlepaint.md r1/fix rounds), and the idle floor
   bar (Pitfall 57) is 4.5-5.5 ms. A future regression to ~7.9 ms — 40%+ worse than today's measured
   number — would still PASS this gate untouched. Tighten to ~6.0-6.5 ms so the stall gate actually
   protects the win idlepaint just landed, rather than merely re-stating a stale, generous ceiling.

4. **The cadence sub-checks (routine_pad_repaints/s, band/fader repaints/s, §3.2 item 3) measure
   repaint()-call-site invocations, not actual `paint()` executions**, so they can diverge from what
   Boris's window actually redraws whenever a sibling's larger dirty-rect union still sweeps the
   pad/strip in on the same tick (the very union-growth mechanism S1/S2 this plan targets) — the
   counters are all local, per-call-site guards (`RoutinePad::setSpec`, `LayerStrip::syncFromModel`,
   `UiPaintCounters.h`), with no witness for "how many times did `paint()` actually run." The CPU
   delta (item 2, the bar) and the raw `ui_passes` per-pass log (a1, INFO only) are the only true
   ground truth. Recommend the cadence checks be documented as secondary/diagnostic, not
   independently gating evidence of "the fix worked," or add an actual-paint-call witness alongside
   the existing repaint()-request counters.

## NIT

5. §0's fact table says TopBar "repaints ... every tick unconditionally" (`TopBar.cpp:306,314-331`).
   Verified: the repaint call is guarded by `if (!beatWheelBounds_.isEmpty())` (TopBar.cpp:325) —
   unconditional only once layout has run once. Immaterial to any gate, but the word choice invites
   over-trust in a claim §0 marks VERIFIED.

6. `SignalStrip::paintMinimized` never draws peak or flash at all (SignalStrip.cpp:~85-115: only a
   name + a fill rect from `displayValue_`), yet c4's `Painted` key still includes peak/flash, so a
   Minimized-size strip will register spurious "changed" from peak/flash decay noise the user never
   sees in that mode — no visual risk, just slightly overstates c4's savings outside the default
   Normal size the plan measures.

## Strongest counter to this attack, and why it doesn't move the verdict
The plan's own gate structure (§3.3 rule ii: STOP after c1 if residual leaves no teeth; c3/c4 built
only conditionally on measured c1 numbers) already defers the riskiest levers to measurement rather
than blind commitment, and c2 — the only unconditional lever — has the strongest pixel proof in the
whole plan (§2.3(e), a real `createComponentSnapshot` pixel compare). One could argue MUST-1 is
therefore premature: c4 may never be built if c1's arithmetic doesn't clear >= 5 ms/s. I still rank
it MUST because the plan's commit list (§4, c4) and its gate table (§3.2, g1 lower-bound row) are
written as if c4 WILL land and only need a numeric go/no-go, with no pixel gate drafted for that
path at all — by the time c1's numbers justify building c4, the missing gate is not a one-line
addition the builder can improvise correctly under commit pressure; it belongs in the plan now.

REPORT_FILE: .harmony/.reports/s-rta-0929/attack-g4cpu-vj.md
