# Attack: plan-video.md — GL/threading/FFmpeg seat

VERDICT: The ring protocol is sound (F14/R-3 verified, 11 headless cases +
TSan stress), but the plan has one MUST correctness gap in its "never shown"
signal that crosses Pitfall 53 / its own "never black, never wrong" claim,
and it is UNGATEABLE — no probe row exercises GL context loss, the one
scenario that triggers it. A SHOULD undermines R-7's own idle claim. X1
mutex scope, close()/join avoidance, and memory-ordering pairing all check
out against current main.

## MUST
1. **`judge()`'s `shownBefore` = GL-texture existence, not "ever decoded a
   frame" — context loss flips a legitimate Held/Late hold into Pending.**
   4.4 calls `judge(false, texture_ != 0, playing_, ...)` (plan-video.md
   :513); the spec treats `!shownBefore` as "never shown" -> Pending (:415).
   `releaseGL()` (`src/media/VideoPlayer.cpp:369-377`, verified: `texture_ =
   0; textureCreated_ = false;`) fires on EVERY `openGLContextClosing()`
   (`Renderer.cpp:1120-1125`) for every LIVE player, not just retired ones.
   A clip mid-catch-up (Held/Late, no pick yet) that survives a context
   recreation (output window add/remove/resize) next reports `texture_==0`
   and is judged Pending instead of Held. Per F9 (`CompositorEngine.h
   :113-116`, C1 :383-395) Pending pauses the crossfade and feeds
   `notePendingImage` (the render_frame gate) — a hold silently becomes
   "nothing to show," crossing Boris's "keep playing" ruling
   (BORIS_DECISIONS.md:354-355) and the plan's own w3b "NEVER BLACK, NEVER
   WRONG" row (:609). UNGATEABLE: no row in 4.6 closes/reopens an output
   window or resizes mid-hold, so this self-forbidden regression has no
   witness.
   Fix: an `everShown_` bool set once on first upload, untouched by
   `releaseGL()` (only `lastUploadedSeq_` resets there, as :463 already
   does); `judge()` reads that, not `texture_ != 0`. Add a w9 row: mid-GOP
   retrigger, force context close+reopen during the hold, assert no Pending.

## SHOULD
2. **The decode thread's ring-full wait loop isn't idle-aware, contradicting
   R-7's "idles after 250 ms without a draw request."** The inner `while
   ((s = ring_.acquireWrite()) < 0 && !threadShouldExit()) thread_.wait(20);`
   (:494) checks only `threadShouldExit()`, never the 250 ms idle clock.
   Rule 15 stops calling `uploadToTexture`/`pick`/`release` for an
   off-screen clip, so if the ring saturates right as a deck goes off-
   screen, the thread parks here forever polling every 20 ms instead of
   `wait(-1)`. Not a decode call (no B2 violation by the letter) but also
   not the "costs nothing" idle thread R-2/R-7 claim; no row (w4 tests
   return, not saturate-then-leave) catches the steady wakeup across many
   off-screen decks. Fix: re-check the idle condition inside the inner wait,
   or fall to `wait(-1)` after N failed acquires while off-screen.

## NIT
3. R-3 pins `acquireWrite`'s CAS to `acq_rel` but doesn't state the ordering
   for `pick()`'s Ready->Free CASes (:189) — say `acq_rel` explicitly so the
   builder doesn't default to `seq_cst`/`relaxed` by omission.

## Strongest counterargument to my own position
Both findings need a rare coincidence (context loss during a hold;
saturate-then-leave) and neither is a hot-path Sacred Rule violation — no
lock, allocation, or FFmpeg call ever reaches the GL thread either way. A
chair optimizing for highest-value gates might defer both as named levers
(R-13 style) rather than block commit 3. I still rank #1 MUST: it silently
breaks a Boris-adjacent contract with zero test coverage — worse than a slow
path the diagnostics would surface on their own.
