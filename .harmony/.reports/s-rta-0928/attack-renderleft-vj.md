# Attack -- plan-renderleft (R1-R5), VJ-visible-behaviour + measurement-skeptic seat

VERDICT: SOUND_WITH_FIXES -- the async-decode architecture is right, but the plan has
three real gaps a VJ or an Eyes run would hit, none of which its own probe rows can catch
as written.

## MUST 1 -- a crossfade onto a cold image freezes, then JUMPS, not just "1 frame late"
`advanceCrossfade(layer, dt)` runs unconditionally every layer every frame
(`CompositorEngine.cpp:969`, before the clip-texture fetch), so crossfadeProgress keeps
advancing by wall-clock time even on a frame where the plan's hold path skips
renderLayerStages entirely (plan lines 193-196, 509-524). The picture freezes at the last
resident frame while progress silently advances underneath it; when the texture lands the
layer jumps to the now-later progress instead of resuming smoothly. This is a beat-synced
dissolve-in, the most common VJ action, and it is worse than "held picture" -- it drops
frames of motion on resume. i2_fade (plan line 755) only checks dbox at the SETTLED state
1.5s after trigger, never mid-fade; R-2 (plan line 856) names only "staggered by 1 frame"
for column triggers, not this jump. No row could ever go RED for it.
Fix: pause the crossfade clock while the incoming clip is pending, or add a mid-fade
(30-60% of a 1s dissolve) smoothness row.

## MUST 2 -- the capture completeness gate silently swallows live Save-Snapshot, not just Eyes
`processPendingCapture` (`Renderer.cpp:2124-2179`) is the single GL-side handler for every
captureFrame caller. `takeSnapshot()` calls captureFrame at `Renderer.cpp:2197` with no
distinguishing flag, wired to the live Save-Snapshot UI action, OSC, and REST
(`MainComponent.cpp:1894,2199,6624,7360`, `ApiServer.cpp:720`). R1-g's gate (plan lines
245-249, 545-551) is motivated purely for Eyes/probe determinism but lives at the shared
GL layer with no caller discrimination -- any pending image anywhere defers every capture.
A VJ pressing Save Snapshot the same second they trigger a cold 4K cue now waits up to 5s
or gets a silent "Failed to save snapshot" log line with no UI feedback, a regression from
today's always-answers-this-frame behaviour. R1-h's 8 MiB/frame throttle (lines 250-255)
widens the window further on swap-triggered prefetch. Nothing in PROBE ROWS or DONE LOOKS
LIKE exercises takeSnapshot under a pending image.
Fix: bound takeSnapshot's wait (capture-as-is fallback, as today) or add a best-effort
flag that only render_frame sets.

## MUST 3 -- R1-g's sequence coverage claim isn't wired, and no test could show it
R1-g claims the gate covers "sequences with nothing to show" (line 246), but the counter
it reads (framePendingImages) is only incremented inside the new getKeyTexture (line 498,
R1.2 section 3), the Image-clip path. R1.4's sequence section (lines 634-651) never states
getCurrentTexture bumps that same counter when *pending is set for lastShown_ below 0 --
it only feeds the separate hold/skip counters (line 522). If this ships as written, a
render_frame right after triggering a brand-new ImageSequence can capture an
empty/undefined frame instead of the first real one, the exact nondeterminism R1-g exists
to prevent. There is no i3-equivalent zero-wait capture row for a fresh sequence; i6
(line 760) waits 1.6s, generous enough to hide the gap, so the gate structurally cannot go
RED even if the bug ships.
Fix: name the shared counter explicitly in R1.4 and add a zero-wait sequence capture row.

## SHOULD -- i1/i2's cut-vs-dissolve split undertests Mask and persistent-layer holds
i4m (line 758) is a counter-delta check only, never a dbox/picture check that a held stale
mask still keys correctly against a live, changing layer beneath it.

## Strongest counterargument to this attack
i3's gate covers the most common case: a still-image trigger, hard cut or steady state.
The gaps above are the crossfade transient and non-compositor pending sources -- narrower
slices, not a rejection of the approach. A careful builder might wire the sequence counter
correctly by inference and notice the takeSnapshot conflation while implementing. I hold
the findings anyway: the plan's own bar is "RED-first rows" and "a gate that cannot fail"
is exactly what this seat was asked to hunt for, and for two of three, no row exists that
could ever catch a regression even if the builder gets it right today.
