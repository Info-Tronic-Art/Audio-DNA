# ATTACK — plan-g4cpu.md (JUCE 8/macOS peer, CALayer, retina, coalescing lens)

VERDICT: c2 (RoutinePad paint-key) and c1's attribution are mechanically sound — verified
`Slider::setValue`/`constrainedValue` (juce_Slider.cpp:201-239: snap then compare to
`lastCurrentValue`, repaint only on change) matches the plan's claim exactly. c3 (TopBar into its
own CALayer via `NativeLayerHost`) reuses a proven mechanism but is the FIRST time it wraps an
interactive, focus-taking, text-editing widget, and the plan's own gates never exercise that
surface. Ship c1/c2 as spec'd; gate c3 on MUST-1/2 before it lands.

## MUST
1. **c3 has zero automated input-passthrough coverage.** `ADNANativeLayerView::hitTest` returns
   `nil` unconditionally, `interceptsMouseClicks(false,false)` (`NativeLayerHost.mm:19,82`) — proven
   for SignalBar/WaveformDisplay, which have no buttons. TopBar has play/pause/stop, `outputsButton_`
   (`MainComponent.cpp:576-582`), tap-tempo, Link toggle, and a focus-grabbing `TextEditor`
   (`bpmEditField_.grabKeyboardFocus()`, `TopBar.cpp:116`). Every c3 gate (v2t, v1/v3/v3p, LOOK 3.4)
   is a static-state pixel/endpoint check; none simulates a click, and 3.4's LOOK list names
   Outputs/Manual+BPM/Tap's tooltip but never Play/Pause/Stop/Link. A hitTest/intercepts regression
   ships silently: transport goes dead while every pixel gate stays green. Fix: add
   Play/Pause/Stop/Link clicks to 3.4, plus one automated probe click-through assertion on
   `outputsButton_` post-c3.
2. **The c3 gap arithmetic (2.4 S2, 3.3) doesn't net out c3's benefit to the i1 arm.** S2 claims
   "c3 (locked case) -> 0" i.e. zeroes its GAP share. But c3 also shaves TopBar's isolated 15 Hz
   pass at i1 (§0's own fact: "a TopBar-only pass 0.73 ms ... 0.49 ms of JUCE paint" — ~4 ms/s at
   14.9/s). The true gap reduction is (Δg4 − Δi1), not Δg4 alone; 3.3 freezes the bar from this
   uncorrected table BEFORE deciding whether c2/c3 land, and "never moves" it — a wrong bar (too
   tight -> false STOP/RED; too loose -> a real regression passing) is locked in regardless of
   which levers actually get built. Fix: c1's ranking must compute (Δg4 − Δi1) for every
   idle-shared lever (c3, c4) before 3.3 freezes.

## SHOULD
3. Retina/2x is never exercised (`grep -rn "scaleFactor|backingScale|Retina" probe-idle-paint.py`
   = no hits; `NativeLayerHost.mm` never touches `layer.contentsScale`, relies on AppKit's implicit
   sync). Inherited from prior work, so not new — but this is the first layer to carry live TEXT
   (BPM digits/labels), where 2x hinting/AA diffs are most visible and K2's 1/255 tolerance least
   forgiving. Run one v2t pass forced to a 2x backing scale.
4. c1 classifies a pass by which named rects its UNION intersects (§2.2), not by which counters
   actually fired that tick. A pass unioning two rects for an unrelated reason (hover, resize)
   would be misattributed to routine cost, and 3.3's bar is frozen from one such sample and never
   re-derived. Cross-check: only count a pass toward S1-S4 if its matching counter
   (routinePadRepaints / faderRepaints / topBarWheelRepaints) moved in the same tick.

## NIT
5. §2.4's precondition grep for popups is correct and verified clean (Outputs menu:
   `MainComponent.cpp:580`, `withParentComponent(getTopLevelComponent())`, routed through
   `OverlayWatch` so the layer falls back synchronously under the popup) — worth citing as
   evidence this specific z-order trap is already handled, not just a checklist line.

## Strongest counterargument
Both MUSTs concern c3, which is CONDITIONAL on c1's own measurement (§2.4: ">= 8 coincident
passes/s"). If c1 shows the coincidence is rare, c3 never lands and neither finding bites. I hold
them anyway: (a) §1 already predicts the locked case as likely and treats c3 as "ACCEPTED
CONDITIONALLY," not a remote branch; (b) MUST-2 corrupts the bar's arithmetic even if c3 is
ultimately skipped, since 3.3 freezes the bar from the SAME precomputed table before the c2/c3
decision is made.

REPORT_FILE: .harmony/.reports/s-rta-0929/attack-g4cpu-juce.md
