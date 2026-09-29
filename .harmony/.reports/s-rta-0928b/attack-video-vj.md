# Attack: plan-video.md — VJ performer / gate skeptic

VERDICT: NOT SHIP-READY AS GATED — two MUSTs (retrigger-during-crossfade shows a stale/wrong-time
frame with no probe catching it; the decode thread's own idle promise is starvable off-screen),
plus a SHOULD (a new >1s freeze ships with no Boris feel ruling). Citations spot-checked against
main @ 5b78d43 (VideoPlayer.h/.cpp line numbers, Renderer.cpp:1577-1643/:1642/:1717-1726,
CompositorEngine.cpp:383-395, DeckClock.h:16-41, BORIS_DECISIONS.md:354-355) all matched exactly —
this critique is about design/gate coverage, not fabricated source facts.

## MUST 1 — retrigger during an active crossfade: stale frame blended in, ungated
Pitfall 53's C1 pause (`incomingImagePending`, CompositorEngine.cpp:383-395) fires only while a
clip has NEVER shown a frame (R-6: "pending happens only for a broken first frame or a seek
before the first draw"). The same-column retrigger path (MainComponent.cpp:4305-4329,
`player->seekTo(clip->inPoint)`) runs on an ALREADY-SHOWN clip — texture_ != 0 — so it becomes
`Held`, not `Pending` (VideoRing::judge, 4.4). A VJ retriggering the incoming clip of a live
crossfade to a mid-GOP in-point (a routine live move: double-tap the fading-in column) gets NO
pause: the fade keeps blending a FROZEN pre-retrigger frame for up to ~0.3s/1080p, ~1.3s/4K
(R-7) while the real target decodes off-thread — the audience sees a fade land on the wrong
temporal content, not merely a freeze. w6 (4.6) crossfades two freshly-triggered clips, never a
mid-fade retrigger; w3/w3b retrigger with no crossfade running. No row exercises "retrigger the
incoming clip while `crossfadeProgress < 1`," and R-C only names a single-layer hold, never the
fade-blended case. Fix: make `Held` pause the fade too while `crossfadeProgress < 1` (extend the
same predicate ImageSequence's first-frame-pending already uses), and add a probe row: trigger A,
1s in; retrigger A mid-GOP; immediately trigger B; assert the blend never shows A's stale frame.

## MUST 2 — decode thread's "idle after 250ms" is starvable, contradicting Rule 15
R-5's `decodeLoop` (4.4) checks the 250ms idle condition only at the top of the OUTER `while`.
The ring-full retry (`while ((s = ring_.acquireWrite()) < 0 ...) thread_.wait(20);`) is a nested
loop entered mid-iteration. Switch a player off-screen (DeckClock ticks `advanceClock` only — no
`wantTime_`/`notify()`, R-4) while its ring is already full mid-catch-up, and nobody ever calls
`pick()`/`release()` off-screen — the inner wait(20) loop never gets a free slot, so the thread
never reaches the outer idle check and polls forever instead of parking on `wait(-1)`. This
contradicts B2/rule 15 ("no decode... bounded per call," performance-controls.md:49, F10) and the
section-10 doc text ("its thread idles after 250ms"). Untested by w4 (switches away only when NOT
mid-catch-up). Fix: recheck the idle condition inside the ring-full retry, or force `wait(-1)`
once `nowMs()-lastDrawMs_ > 250` fires even mid-wait.

## SHOULD — a new >1s single-layer freeze ships without a Boris feel ruling
BORIS_DECISIONS.md "Playback Behaviour" (:320-360) has no ruling on acceptable hold length; the
nearest is ":354-355" ("keep playing" on deck return). w3b (4.6) accepts up to 200 late-frame
deltas (~1.3-1.8s) at 4K by DEFAULT (`kSkipNonRefInCatchUp` off unless the bar is exceeded, R-13),
and the plan defers the default choice to "OPEN FOR HARMONY" rather than Boris. For a live show
that's the gap between "looks briefly odd" and "visibly stalls for over a second" — exactly the
feel judgment CLAUDE.md's "clarify before coding" and this project's Boris-ruling practice exist
for. Fix: get a feel ruling on hold length / NONREF-by-default-at-4K before merge.

## Strongest counterargument, and why I still hold the verdict
Today's failure is strictly worse (whole output at 14/3.8 fps for 3.4s, F3/S1b) vs one layer
holding up to 1.3s, and R-C/R-7/R-13 already name the hold as an open, measured risk rather than
hide it. Both MUSTs are edge interactions the "both, in sequence" scope never promised to solve,
and the b2 fallback (R-B) wouldn't solve them either — so "not gateable yet" is not "wrong
design." But section 6 claims Pitfall 53 and Rule 15 are kept VERBATIM, and both MUSTs show a
realistic live sequence where the letter of those guarantees breaks. A plan with 9 probe rows
aimed at exactly this class of regression should have a 10th for the one combination (fade +
mid-GOP retrigger) live VJ use makes routine, and Rule 15's idle promise should hold under the
one condition (off-screen mid-catch-up) it doesn't test.
