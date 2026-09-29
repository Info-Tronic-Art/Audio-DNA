# Attack — plan-mediaopen.md (VJ performer / gate skeptic seat)

VERDICT: The fence-hold mechanism (4.1) is sound and its citations check out (verified UndoService.cpp:60-92,
Renderer.cpp:357-358/380-461, Clip.cpp:215, ImageSequence.cpp:17-67 all match cited text at 5b78d43). But three gaps
would let a VJ hit a real, un-gated failure mid-show: a stuck "Loading..." label, an unchecked Sacred-Rule-1 audio
claim, and a REST-timing race the plan names but never gates.

## MUST 1 — background video opens are never checked against the Sacred audio-callback budget
CLAUDE.md Sacred Rule 1/12: audio callback must stay <100us. R5 (plan:722-725) admits "up to 8 busy threads on a
10-core M1 Pro during a load" and mitigates only via "m2 records `peak_callback_ms`... if the render callback
suffers, drop the pool" — that stat is `Renderer::peakCallbackMs_`, the GL-thread callback, NOT the OS audio
callback. No probe row (m2/m2b, plan:642-643) or ctest reads an audio-thread stat/XRun/dropout during a 16x4K load —
exactly when a VJ is mixing live audio while loading the next show. A weaker machine under thermal load with 8
concurrent FFmpeg decode threads is a real path to an audio glitch this plan's gates cannot see. Fix: add an audio
budget witness (XRun count / peak audio-callback us) and gate m2/m2b on it, or default MediaOpener to 1 low-priority
thread instead of leaving thread count "ASSUMED (Q4)" decided by a GL-thread metric alone.

## MUST 2 — a canceled staged load leaves "Loading <name>..." stuck forever, with zero gate
`cancelStagedOpen` (plan:487-489) is `mediaOpener_.cancel(); ... staged_.reset();` — no write to `fileLabel_`. The
label is restored only by `finishStagedLoad`'s success LABEL step (plan:463,470-473). Any cancellation trigger the
plan itself lists (a second load/append/duplicate begun before the first lands, or a throwing mutation) clears
`staged_` but leaves the top bar telling the performer a load is still pending — a UI-lies-about-model-state bug, the
exact class Pitfall 41 exists to prevent. None of m1-m8 / `end_hold_witness` (plan:642-653) exercises cancellation, so
this ships with no RED/GREEN signal. Fix: restore `fileLabel_` in `cancelStagedOpen`; add a probe row that cancels a
staged load and asserts the label recovers.

## MUST 3 — R9 (trigger-before-swap) is named, not gated, and is a normal show-control pattern
R9 (plan:733-735) admits a REST load immediately followed by triggering a video cell fires before the swap lands, and
defers to documentation only — no ctest, no probe row, no required witness before a caller acts. For OSC/MIDI/REST
show-control automation (this app's own advertised surfaces) "load, then go" in one script is routine, not an edge
case. The plan's probes dodge it only because "probe-deck-clock sleeps 1 s" (line 734) — a workaround, not a fix.
Fix: a probe row that loads then immediately triggers, asserting a defined behavior, plus require callers to poll
`media.opens_pending` rather than merely suggesting it.

## SHOULD — m3c is self-admittedly not reliably RED
Row m3c: "commit 1: (a) 0-2 (a guard: not reliably RED)" (plan:646). A gate the plan's own author cannot promise will
fail pre-fix is not proof of the regression claimed; should be a repeated-sample assertion, not a single reading,
before counting toward section 4.11's "done."

## SHOULD — the fence's memory-ordering fix is proven only by a flaky live probe
4.1's double-checked-load ordering (fenced_ before deck, re-read once) is correct by inspection, but its only witness
is `fence_black_frames == 0` over ">= 5 runs" (plan:657,740) — probabilistic for a cross-thread ordering bug, the kind
that hides for months then appears once during a real show under different scheduling. No TSan run or stress-repeat
ctest is proposed anywhere in 4.9, despite this being the one piece of the plan with genuine new atomic ordering.

## NIT
`heldFBO`/`heldW`/`heldH` (4.1, plan:286-291) recompute state the next unmodified lines (Renderer.cpp:380-386,
`defaultFBO`/`compW`/`compH`) already compute; harmless since the branch returns first, but should be commented so it
stays in sync if frame setup ever moves.

## Strongest counterargument to this paper
All three MUSTs are omissions in an otherwise carefully re-derived, line-verified plan whose core fix (canvas hold) is
correct against current main; R9/R5 are already named as open risks for Harmony, not hidden. A chair could fold MUST 1
and 3 into "ship commits 1-6, defer commit 7 (Q1)" since both are specific to the asynchronous-load commit, not the
fence fix a VJ needs most urgently. I hold the ranking because section 4.11's "done" bar claims the whole lane
including commit 7, and none of its RED/GREEN rows would catch these three failures even after commit 7 ships.

REPORT_FILE: .harmony/.reports/s-rta-0928b/attack-mediaopen-vj.md
