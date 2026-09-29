# ATTACK — plan-idlepaint.md (JUCE/AppKit/CoreAnimation seat)

VERDICT: Core diagnosis and F4/F6 check out against pinned JUCE source, but OverlayWatch's fallback transition
has a real ordering bug that defeats the one thing (E) exists for, and F4 doubles exposure to an already-flagged
cross-thread race in a way that can freeze (not just glitch) a playhead. Neither is caught by the stated gates.

## MUST 1 — Native→Fallback transition can hide the very overlay it's meant to reveal (untested)
`setFallback(true)` (2.2, lines 173-178) sets `FallbackPending`, calls `component_.repaint()`, and only fires
`sink_.layerHidden(true)` one `AsyncUpdater` turn LATER, after `paint()` sees a full-widget clip. But
`OverlayWatch` reacts to the overlay's own `setVisible(true)` via `ComponentListener` in the SAME call that makes
it visible, so the overlay's first paint and the widget's fallback-repaint land at the SAME next vblank
(`juce_NSViewComponentPeer_mac.mm:1089-1123`). At that vblank the peer's backing store gets fresh overlay + fresh
widget pixels, but `ADNANativeLayerView`'s own CALayer — a separate sibling ABOVE the peer's layer (plan's own
section-1 admission) — is never told to redraw (`invalidate()` in FallbackPending returns `true` WITHOUT calling
`sink_.layerNeedsDisplay`, 2.2 lines 164-170: that only fires when `mode_==Native`). The stale native layer keeps
visually covering the freshly-drawn overlay for one more message-loop turn — a popup over SignalBar/Waveform is
briefly INVISIBLE on appearance, the exact defect (E) is built to prevent. v2 (line 491) can't catch this: it
polls `signalbar_mode==2` to steady state BEFORE capturing B, excluding the transient frame by construction. R1
(line 603) names only the REVERSE transition ("the un-fallback frame") as a risk; this forward glitch is unnamed.
FIX: hide the NSView synchronously when `paint()` sees the full clip (R1's own stated fallback) instead of via
`triggerAsyncUpdate()`; add a timing gate that captures mid-transition, not after polling to steady state.

## MUST 2 — F4 doubles reads of a known GL-thread race; can freeze, not just glitch, a playhead
`LayerStrip::transportViewOf` reads `clip->playheadPosition` to DECIDE whether to repaint (2.5). That field is
`mutable double` (`src/model/Clip.h:229`), written non-atomically from the render path (`src/model/Layer.h:268,
282`), and institutionally flagged as a live hazard: `src/connect/ConnectionEngine.h:12-13` — "GL-written
`mutable double` — s166 spec section 8 names two acceptable read strategies." Today's `paint()` reads it once and
ALWAYS repaints, so a torn read is cosmetic-only (self-corrects next tick). F4 reuses the same racy field to gate
whether to repaint AT ALL: if a torn read happens to equal `lastTransportView_`, the tick's repaint is skipped —
the playhead visibly FREEZES until the next differing read, a new failure mode, not merely doubled exposure, and
not one of s166's two sanctioned strategies. g2 (line 486) only checks an aggregate count (`>=40` per 5s); one
dropped compare among ~140 expected repaints is invisible to that threshold.
FIX: read `playheadPosition` once per tick into a local and reuse it for both compare and paint; resolve against
s166's read strategies before shipping, don't add a third ad hoc one.

## SHOULD 1 — invariant (i)'s clipObscuredRegions citation is right by accident, not by the stated reason
2.2 invariant (i) (lines 186-189) says WaveformDisplay's rounded corners are covered by "the parent's background
painted by the parent itself," citing `juce_Component.cpp:1719-1727` (opaque-sibling exclusion inside
paintComponentAndChildren's child loop). Verified separately: `clipObscuredRegions` (called at
`juce_Component.cpp:1688`, same function) is what actually excludes an OPAQUE child's rect from the PARENT's own
`paint()` call; a non-opaque child (Waveform) is not excluded, so `MainComponent::paint`'s `fillAll`
(`MainComponent.cpp:2350`) does repaint under it whenever MainComponent itself repaints. This holds ONLY because
nothing dirties MainComponent at idle by this plan's own design — that's the real reason, never stated. A future
full-window repaint path that assumes the citation (not the idle-invariant) could leave stale pixels under the
waveform's corners.

## NIT — peer layer-backing precondition stated as unconditional fact
(A) assumes the peer NSView is layer-backed (`juce_NSViewComponentPeer_mac.mm:220-226`), gated on
`windowRequiresSynchronousCoreGraphicsRendering` being unset. Verified true today (`Main.cpp`'s DocumentWindow
never sets it) but the plan (line 50) states it as unconditional rather than a checked precondition; no test
names it, so a future window-style change would silently break (A).

## Strongest counterargument to this attack
Both MUST findings are narrow races (~16ms vblank window; one torn-double compare) in a UI that already
tolerates an unsynchronized playhead read today, and Boris's LOOK checklist (3.4) exercises exactly the
popup-over-bar and playing-strip cases a human would notice. I still hold both: MUST 1 defeats (E)'s stated
purpose at the one moment it matters, and MUST 2 turns a self-healing cosmetic bug into a silent stall in a
latency-sensitive playback app — and no gate in section 3 can fail on either regression today.
