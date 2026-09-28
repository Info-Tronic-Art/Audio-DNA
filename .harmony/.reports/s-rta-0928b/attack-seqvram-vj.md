# Attack — plan-seqvram.md — VJ performer / gate skeptic

VERDICT: NOT SHOW-READY AS GATED — the RED/GREEN rows prove the policy on isolated, single-sequence, cut-only
scenes; they never exercise the concurrency a real set produces (many decks, crossfades, beat-sync, reverse), and
the one shared resource that will actually break under load — the 3-thread decoder pool — is never contended by
more than one sequence in any gate.

## MUST

1. **Single shared 3-thread decoder, never contended by >1 sequence in any gate.** `Renderer.h:285`:
   `ImageDecode::Decoder imageDecoder_{3}` is ONE pool for every ImageSequence in the whole app (F9 confirms 3
   low-priority threads app-wide, not per-sequence). A real show runs several decks/layers of long or 4K
   sequences simultaneously (persistent layers, F1 line 57-60 — a fade alone doubles live sequences on ONE
   layer). Every probe row (v1-v7) drives at most one actively-decoding sequence at a time (v6's second sequence
   only starts after the first is fully resident; v7 gates the off-screen deck to zero decode via rule 15). No
   row ever makes 3+ sequences request frames in the same second, so the exact failure mode this design exists
   to bound — decode throughput, not VRAM — is untested. `seq_late_frames` can read 0 in every gate while a real
   4-layer 4K show starves the pool. FIX: add a row with >=3 concurrently-decoding sequences (mixed 1080p/4K,
   at least one past its window) and assert `seq_late_frames` growth stays bounded, or explicitly document the
   decoder pool as a second, ungated bottleneck.

2. **No probe row ever crossfades two sequences, though the plan's own F1 says a fade holds two live
   `ImageSequence` instances competing for the same 1 GiB budget.** Fixture line ("Layers: `transitionSpeed 0`
   (cuts)") forces every row to a cut. §4.3's grant math (`seqResidentTotal_ - min(..., mine)`) is computed once
   per frame at scan time and handed out per-clip in draw order (plan admits overshoot "by at most that frame's
   uploads", line 315) — but that's only checked for the steady multi-second case, never for the transient where
   BOTH chains of a fade are long/4K and each expects the OTHER's residency to still be free. This is exactly the
   scenario named in the dispatch ("crossfades") and is a Boris-visible failure class (double judder mid-fade) —
   currently ungateable because it's never staged. FIX: a v8 row — two long/4K sequences on the same layer mid
   crossfade (`transitionSpeed` nonzero) — asserting `seq_late_frames` delta == 0 through the fade.

3. **Many-sequence floor overrun (R5, self-flagged) has no gate at all**, only a stats field. `kMinWindowFrames`=8
   floor x 31.64 MiB (4K) = 253 MiB per sequence; 5 concurrent 4K sequences already exceed the 1 GiB budget before
   any eviction can help (`SeqVram.h:201-202`'s `allowance()` clamps to the floor, never below it — by design it
   CANNOT enforce the budget once floors alone exceed it). `seq_over_budget` is reported but no row's PASS
   criterion ever drives the count of live sequences high enough to set it, so "many sequences, budget
   physically impossible" degrades to *undefined* rather than *tested-and-accepted*. FIX: a row with 5+ live 4K
   sequences that intentionally sets `seq_over_budget` nonzero, and a documented statement of what the user sees
   (worse: whose floor gets violated, or does none?) — right now `allowance()`'s floor guarantee vs. the global
   `kBudgetBytes` guarantee are two promises that can both be violated simultaneously and nothing says which wins.

## SHOULD

4. **Reverse toggling and BPM-synced fps changes — both named in this seat's brief — are proven only in the pure
   ctest simulation (`tests/test_seq_vram.cpp` cases 1-11), never live.** `Renderer::syncMedia` (`Renderer.cpp`
   :1674-1691 per plan) can push a sequence's fps arbitrarily high via BPM sync (`fps = frames /
   (beatDivision * 60 / bpm)`); F9's "sustainable" throughput (~30 fps @1080p, ~10 fps @4K) is a fixed-rate
   assumption baked into `kLookAhead=4`. No probe row drives BPM sync or toggles `reverse` mid-play (all v-rows
   use static `speed 1.0, reverse false`, F7 quote). A tempo push past the sustainable rate — trivially achievable
   on a real set — produces exactly the lateFrames this design is meant to catch, and no gate can fail on it.
   FIX: a live row that ramps BPM (or sets a small `beatDivision`) on a 4K sequence and asserts `seq_late_frames`
   stays within a stated bound, plus a live reverse-toggle row (R-3 "accepted... a few late frames" is currently
   an assertion, not a measurement).

5. **Frame-time spike from the trim itself is never measured.** `scanSequenceVram` (§4.3) does its GL deletes
   (`shrink`/`trimToMinimum`) while holding `imageSeqMutex_`, at the frame top, only "under budget pressure" —
   v2(d)/v3(d) check `peak_frame_time_ms <= 16.7` during *steady* playback, never at the moment several idle
   sequences get trimmed at once (e.g., several deck switches back-to-back in a real transition-heavy set).
   `glDeleteTextures` on many slots in one frame is exactly the kind of GL-thread spike the 8ms render budget
   (CLAUDE.md latency table, row 10 Shader render 1-3ms) has no headroom for. FIX: a row that forces several
   sequences over budget in the same frame and checks `peak_frame_time_ms` across that specific frame, not just
   the steady window.

## NIT

6. Cross-deck transitions (CLAUDE.md: "cross-deck transitions with 3 blend modes") aren't exercised with
   sequences on both decks simultaneously — v7 uses a static `warm.png` on the other deck, so the shared-budget
   interaction between two *decks'* worth of sequences (not just two layers on one deck) is untested.

## Self-counterargument

The plan is explicit (§7 R1, R5, R8; §9 open questions Q1-Q3) that these are exactly the tradeoffs it is
deferring to Harmony/Boris, and its counters (`seq_over_budget`, `seq_late_frames`) are designed so a real show
that hits these cases is *visible after the fact*, not silently corrupted — arguably sufficient for a first
landing given the unbounded-VRAM failure it replaces is strictly worse (R1's own argument: two 300-frame loops
today can exhaust an 8-16 GB machine outright). But "visible in a counter Harmony can look at later" is not the
same as "gated RED-then-GREEN before ship," and this plan's own methodology (RED-first, §0) is the standard I'm
holding it to — the gaps above are inside that self-declared bar, not outside it.
