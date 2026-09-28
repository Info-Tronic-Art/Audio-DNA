# Attack: plan-seqvram.md — GL/threading/memory-correctness seat

VERDICT: NOT GATEABLE AS WRITTEN — the one failure path the design admits it needs ("plan guarantees room")
has no defined Release-mode behavior, and the crossfade case (two live chains, both budget's sharpest
adversary) is never exercised by any probe row. Do not adopt until MUST-1 and MUST-3 are closed.

## MUST

**MUST-1 — `Act::Full` fallback is a debug-only assert; CMakeLists.txt has no `JUCE_FORCE_DEBUG`/assert
override (verified: `grep -rn "JUCE_FORCE_DEBUG\|CMAKE_BUILD_TYPE" CMakeLists.txt` → no hits), and the rig
builds Release (plan §4.0: "Build Release with AUDIODNA_BUILD_TEST_SERVER=ON").** `jassert` compiles to a
no-op in Release JUCE. Plan §4.1/4.2 (`uploadFrame`, line 267): "Full -> the caller evicted first (never
reached: plan guarantees room; jassert)". If that invariant is ever wrong — and §4.3 lines 315-316 itself
documents a case where two sequences race the same stale `seqResidentTotal_` — `Slots::acquire` returns
`slot=-1`, and the spec's own next call, `bind(slot, tex)` (SeqVram.h line 230), has no guard for a negative
index: an OOB vector write on the GL thread, in Release, with no crash-reporting assert to catch it in CI.
This is a real-time-thread memory-corruption path, the exact class Sacred Rules 1-4 (CLAUDE.md) exist to
forbid for the audio thread and that the render thread should not tolerate either.
FIX: `Slots::acquire` must have a defined Release behavior for the "no room" case (skip this upload and keep
the result in `ready_` for next call, or force one more eviction past the floor before returning Full), not
an assert. Add a ctest that deliberately starves the allowance mid-upload and checks no OOB.

**MUST-2 — `Slots::releaseAll()` / context-loss reset is unspecified for the `Slots` object's own state.**
Today (`ImageSequence.cpp:331-349`) `releaseGL` clears `textures_`, `requested_`, `ready_`, `outstanding_`,
`lastShown_`. Plan §4.2 (line 288) adds "for tex in slots_.releaseAll(): glDeleteTextures; textures_.clear()
... and the rest verbatim" — it never states that `Slots::releaseAll()` also empties the internal `Slot`
vector (SeqVram.h line 239 `std::vector<Slot> slots_`, each carrying `frame`/`w`/`h`). If it doesn't, the next
`acquire()` on the NEW GL context still sees old `frame->slot` mappings and old `w/h`, and can hand back
`Act::Reuse` against a texture name that belonged to the destroyed context — `glTexSubImage2D` into a name
that may now silently mean nothing, or (worse) may have been reissued by the driver to an unrelated object —
a black/garbage frame with no GL error the app checks for. Ctest (10) exercises `shrink`/`releaseAll` return
values but never re-`acquire`s after `releaseAll` to check the object is empty. Verified precedent this
project cares about the exact hazard: `Renderer::openGLContextClosing` comment at `Renderer.cpp` (drainRetiredMedia
block) explicitly calls out reusing a texture ID across a destroyed context as a known bug class.
FIX: spec `releaseAll()` as `slots_.clear()` + return textures; add ctest asserting `size()==0`/`occupied()==0`
and a fresh `acquire()` after `releaseAll()` returns `Act::Create`, never `Reuse`/`Respecify`.

**MUST-3 — the crossfade case (this seat's assigned focus) is never tested, and it is the case that most
stresses the budget.** F1 (plan, verified) two live chains during a fade = two `ImageSequence` instances,
both drawn every frame (`Renderer.cpp:1090` active, `CompositorEngine.cpp:1675`→`getClipTexture` outgoing).
Because both are drawn every frame, R-7's idle-trim (§4.3 scanSequenceVram) never fires for either — the ONLY
correction path during a live fade is each sequence's own per-call eviction (§4.2 step 3) against a shrinking
`allowanceFrames` computed from the stale frame-top total (§4.3 lines 310-316). If both chains grew large
independently before the fade (e.g. two different 300-frame 1080p loops each near a solo allowance), the
combined total can be ~2x budget the instant the fade starts, forcing a same-frame `shrink()` burst of
`glDeleteTextures` on BOTH chains while they are also uploading — no probe row (v1/v2/v3/v4/v6/v7, §4.5) puts
two over-a-fair-share sequences in a REAL crossfade (`transitionSpeed 0` = cuts, per §4.5 "Layers:
`transitionSpeed 0`"). A regression here (stutter/black frame during a fade — a Boris "Playback Behaviour"
item) has zero gate.
FIX: add a probe row with `transitionSpeed > 0` and two large sequences triggered to overlap in an actual
dissolve; assert `peak_frame_time_ms <= 16.7` and no black frame through the fade.

## SHOULD

**SHOULD-1** — §4.3 lines 315-316's own risk bound ("two sequences drawn in one frame can overshoot the
budget by at most that frame's uploads") is written for a single sequence's view and is too optimistic for
two: both grants are computed from the SAME pre-frame `seqResidentTotal_`, so BOTH can believe they have the
other's now-stale headroom simultaneously — the real one-frame overshoot bound is the sum of both sequences'
per-frame upload budgets (still bounded by the shared `UploadBudget`, F3, but not "one frame's uploads"
singular as stated). Restate the bound for N concurrently-drawn sequences, or track a running total updated
within the frame rather than only at the top.

**SHOULD-2** — `currentFrameIndex_` (`ImageSequence.h:114`) is a plain `int`, written on the message thread by
`seekTo` (F1, three callers) and read on the GL thread by the new `cur = clamp(currentFrameIndex_)` (§4.2
step 1) — a pre-existing unsynchronized cross-thread access the plan (R6) downgrades to "tolerates any value
(worst case one extra late frame)". That undersells it: `cur` is now also the index PROTECTED from eviction;
a torn/stale read doesn't just mis-select which texture to show, it could let the frame actually on screen be
evicted this call while a different (racy) `cur` is protected instead. Practically benign on all real targets
(word-aligned int), but the plan should say so explicitly rather than imply the stakes are unchanged from
today's read-only usage.

## NIT
`frameBytesHint_` defaults to a 1080p guess (§4.2 line 254) until the first decoded result arrives; for a 4K
sequence this transiently over-estimates `allowanceFrames` (computes frame count against a too-small byte
size), so a brief startup window can create more large-4K slots than the true budget intends. Self-correcting
next frame once `frameBytesHint_` updates from real dims; not worth gating, just note it in the doc.

## Strongest counterargument to this attack
Every one of MUST-1/2/3 is a "what if the invariant is violated" argument, not a demonstrated failure in the
pure `SeqVram.h` logic itself (which is fully ctest'd, §4.4, cases 1-11) — the plan's own §7 R8 already admits
"Renderer.cpp is in no ctest ... proven live only", and Belady-style trajectory eviction is provably correct
in the model tested. If `plan()`'s arithmetic is in fact airtight (case 11's 2,000-step simulation with random
seeks is a reasonable stress test), MUST-1's OOB path may be genuinely unreachable, and MUST-3 may just cost a
few extra late frames rather than a crash. I hold the ranking anyway because "provably unreachable in a 2,000-
step single-sequence simulation" is not the same claim as "unreachable with two concurrently-drawn sequences
racing a shared stale total during a live crossfade" — that multi-sequence interaction is exactly what the
model in case (11) does not simulate, and it is exactly where a Release-only assert offers zero safety net if
the proof has a gap.
