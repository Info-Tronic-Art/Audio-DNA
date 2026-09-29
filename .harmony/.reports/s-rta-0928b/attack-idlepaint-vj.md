# Attack: plan-idlepaint.md — VJ/UX/gate-skeptic seat

VERDICT: Mechanism and cited lines check out (spot-verified: LayerStrip.cpp:733-746,531-560; SignalBar.cpp:
34,111-123,199-213; WaveformDisplay.cpp:9,58; TopBar.cpp:305,325-326,569-573; ClipInspector.cpp:958-980;
MainComponent.cpp:2224,2348; MainComponent.h:294,396,406-407; DeckView.cpp — LayerStrips are gridContent_
children in gridViewport_). But identity + perf are proved only at IDLE/test-mode; by its own admission
(R1,R2) the two riskiest behaviours — overlay fallback timing and the untouched routine-band hairline — are
completely unmeasured under a real show, the exact condition this dispatch asks about.

## MUST
1. **Overlay-fallback pixel proof (v2) never runs under load.** v2_identity_fallback is "AFTER only, test
   mode, card" (plan:491) — one idle launch. R1 (plan:603-607) already flags the un-fallback/hide timing as
   INFERRED CA behaviour and admits a "ONE-frame glitch...cannot be captured" by v2's 1s poll. Under a real
   show — several LayerStrips ticking, a routine playing across layers (BORIS_DECISIONS.md:342-348: a
   routine must show in every layer it plays), a recording callback marshalling — the message thread is
   busier, so `AsyncUpdater::triggerAsyncUpdate()`'s "next turn" (NativeLayerCache.h:161,182) can slip past
   more than one vblank. A VJ opening the Outputs menu over the meter bar mid-set could see a longer, more
   visible stale composite than an idle-only test can ever catch. Fix: add a v2b arm with routine/multi-layer
   load, and bound the check by frame count, not 1s wall clock.

2. **The exact defect class survives, unmeasured, inside the fix's own MUST-NOT-CHANGE list.** The routine
   band hairline (`LayerStrip::timerCallback`, LayerStrip.cpp:743-746) stays untouched (plan 2.10). It is an
   unconditional in-peer `repaint()` at 30Hz per strip with a Playing band. A multi-layer routine (Boris
   ruling above) means several simultaneously-animating rects at different window edges — the same C1/C2
   union mechanism diag-idle blamed, re-scoped from "always" to "while a routine plays." No gate tests this:
   i1/i2 are idle-only, g2 uses one seq24 strip, not a multi-layer routine. The plan's 8ms/150ms/s thresholds
   and 75-115ms/s prediction are idle numbers; nothing states, even INFERRED, the CPU/stall profile with a
   routine playing across N strips. Fix: add a g4 arm — trigger a routine spanning >=3 layers, measure like
   i1 — before this lane is called done.

## SHOULD
3. Idle fixtures (card/many16) don't state a driven audio source; v1 explicitly runs "analysis off: meters
   and waveform static, bpm 0" (plan:490). The two panels this whole plan is about are pixel-checked only in
   the state where they DON'T animate — a native-layer artifact visible only mid-animation (a torn async
   layer commit racing `setNeedsDisplayInRect:`, or an alpha issue only visible with real waveform data)
   would be caught only by the human LOOK checklist (3.4), not CI. Add a burst-capture v1 variant with a
   real/synthetic signal driving both panels, diffed frame-by-frame.
4. R3's margin (75-115 predicted vs 150ms/s ceiling) assumes the diag's near-idle signal; the plan's own 3.3
   notes the "repaint only strips whose value changed" lever is NOT implemented in this lane, so a real
   show's continuously-moving meters could push recording cost past the stated margin with no gate to catch
   it before ship.

## NIT
5. Window-capture title-bar height is hardcoded "28pt, ASSUMED... verify once" (plan:497-498) rather than
   measured per run; a wrong value silently shifts every v1-v3 allowed-diff region and could hide a real
   regression instead of failing loud.

## Self-counterargument
The plan is explicit about R1/R2 and offers the LOOK checkpoint (3.4) plus Q4's stated contract ("behaviour
identical, perf identical only while no overlay is up") as the backstop for exactly these cases. If Harmony
accepts that framing, findings 1-2 are priced-in accepted risk, not oversights. My disagreement: that
acceptance shouldn't extend to leaving the routine-band case entirely un-gated, since it is cheap to test
(g4) and is the one scenario this dispatch names by name.
