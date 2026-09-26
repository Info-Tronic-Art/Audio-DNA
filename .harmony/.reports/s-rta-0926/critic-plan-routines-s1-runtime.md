# Blind critic — RUNTIME CORRECTNESS lens — plan-routines-s1.md (s-rta-0926)

VERDICT: SOUND WITH FIXES. The threading/ordering/grip-chain claims I could check against
disk all VERIFIED true. But the plan has one silent, uncaught quantization-parity bug
(different bar counter than the rest of the app uses for the identical setting name), and
one implementation-hazard the plan's own pseudocode walks past without comment (container
mutation mid-iteration). Neither is fatal to the design; both are concrete, fixable defects
a builder following this plan literally would ship.

## Findings

### MUST — TwoBar/FourBar routine quantize uses a different bar counter than every other
TwoBar/FourBar quantize in the app; they will silently disagree after any structural transition.

Plan section 4.2 step 3 (line 323-325) computes bar-edge parity for a pending routine as:
"(TwoBar && barEdge && totalBarCount % 2 == 0)" / "(FourBar && barEdge && totalBarCount % 4 == 0)",
explicitly said to be "the same arithmetic as Layer::processPendingTrigger... but on
totalBarCount (never rewound)".

That parenthetical is the bug: Layer::processPendingTrigger does NOT use totalBarCount.
VERIFIED, src/model/Layer.h:296-306:
    case Clip::BeatSnapMode::TwoBar:
        shouldTrigger = (beatInBar == 0 && (barCount % 2) == 0);
    case Clip::BeatSnapMode::FourBar:
        shouldTrigger = (beatInBar == 0 && (barCount % 4) == 0);
and the caller passes snapshot.barCount, not snapshot.totalBarCount
(VERIFIED src/model/Autopilot.cpp:56-57: layer.processPendingTrigger(snapshot.beatInBar, snapshot.barCount)).

FeatureSnapshot::barCount and FeatureSnapshot::totalBarCount are two DIFFERENT counters
(VERIFIED src/analysis/FeatureSnapshot.h:56-57: barCount = "bars since last phrase reset";
totalBarCount = "monotonic bars since transport start"). They diverge every time the
structural detector resets the phrase — VERIFIED src/analysis/BPMTracker.cpp:474: on a
phrase reset, "barCount_ = 0; // totalBarCount_ intentionally NOT touched (S168)". CLAUDE.md's
own Structural table documents phrase resets on buildup/drop/breakdown transitions, i.e. this
happens routinely in a live set, not as an edge case.

Consequence: a routine set to quantize "2bar" or "4bar" and an ordinary clip on the SAME
layer set to the identical BeatSnapMode::TwoBar/FourBar can fire on different bars for the
same wall-clock instant, because one is keyed on barCount % N and the other on
totalBarCount % N, and those two counters' parity relationship is not preserved across a
phrase reset. This is silent (no counted/disclosed divergence in the plan's Status fields) and
distinct from the two already-disclosed risks R2 (GL-thread beat-crossing vs message-thread bar
edge, ~16ms) and R3 (Resync short-bar) — both of those are about WHEN a boundary is seen, not
about WHICH bar counts as "even". A routine with TwoBar/FourBar quantize is the one config this
plan actually adds new arithmetic for (Bar/Off/Beat reuse existing edges faithfully); it is also
the one arithmetic path nobody in the 14-case test suite pins against barCount specifically —
tests 9/11 exercise Off/Beat/Bar/TwoBar but construct totalBarCount directly in the fixture
(makeSnap), so a test written to the plan's own spec would pass while being wrong relative to
the rest of the app's convention.

Fix: either (a) use snap.barCount % N for TwoBar/FourBar parity (matching the existing
convention exactly, at the cost of routines re-aligning with the layer's own phrase-relative
grid — arguably the semantically correct choice since routines and clips share the deck), or
(b) if totalBarCount is intentionally preferred for its never-rewound monotonicity, disclose
the parity divergence as a named risk (R13) the same way R2/R3 are disclosed, and decide which
counter "2 Bar" means for a routine going forward (a product call, not just an engine detail).

### SHOULD — running_ vector erase during iteration in the "once" branch is unaddressed
container-mutation-during-traversal, in a plan otherwise scrupulous about lifetime/aliasing.

Plan section 4.2 step 3 (line 328-330): "once: player->stop(*sink) -> erase r (state idle; the
look holds)" is written as one step inside "For each Running r: ...". If the builder implements
this literally as a range-for over running_ that erases the current element mid-loop, the erase
invalidates the iterator/reference for the remainder of that pass and skips or double-visits the
next element on the next iteration (std::vector::erase invalidates all iterators at/after the
erase point). This is exactly the class of bug CLAUDE.md pitfall 33 warns about for a
structurally different but analogous case (erasing/reallocating a vector while something holds a
reference into it) — here the "something" is the loop itself.

This is not a design flaw (the fix is a one-line index-based loop with erase-and-reassign, or a
mark-then-compact pass) but it is a genuine implementation hazard the plan's own pseudocode does
not flag, unlike every other lifetime-sensitive step in the document (which routinely calls out
"R1: no pointer survives past it", "orphans", etc.). Given the plan elsewhere treats mid-tick
vector safety as load-bearing enough to name explicitly (R1, R7, R8), its silence here
specifically inside the one step that erases mid-loop is worth flagging before the builder
writes it, not after ctest is green on a fixture with a single running slot that happens not to
exercise "two once-mode routines finish in the same tick."

## What I verified TRUE (runtime-correctness claims that held up)

- Grip-chain rank/tie-break: VERIFIED src/connect/ManualWrite.h:20-24,60-63 — Hand rank
  order and "equal rank: latest writer wins" both match the plan's D9 stacking claim (section 4.3).
- Preamble mechanics: VERIFIED src/recording/Player.cpp:84-103 — discrete preamble entries
  fire in full before any continuous entry, and a continuous entry only releases when BOTH
  touch and set succeed; the plan's section 3.1 ordering rule (R5) rides on real behavior, not an
  assumption.
- Origin/immediate reuse: VERIFIED src/MainComponent.cpp:1945-1953 — every discrete replay
  handler is called with the LITERAL Origin::Replay, independent of f.p.origin; only
  immediate depends on f.p.origin == Preamble. The plan's section 4.4 claim that a routine's
  writes are indistinguishable from an ordinary replay's writes at the capture/undo gates is
  correct, and its reuse of "the SAME four lambdas" (section 4.1) is therefore coherent, not
  hand-wavy.
- totalBarCount monotonicity and the manual-BPM predicted-phase path used by the probe:
  VERIFIED src/analysis/BPMTracker.cpp:487,541 (never rewound) and :77-82,545-554
  (setManualBPM -> STATE_LOCKED + predicted phase). The probe's "no audio device needed"
  claim (section 6.2) is real.
- Tick ordering: VERIFIED src/MainComponent.cpp:3398 (recorderHost_.tick) precedes
  :3418 (connectionEngine_.tick), matching section 4.5's placement instruction for the new
  routineEngine_.tick call between them.
- RecorderClock's TempoMap growth bound for a routine-engine-owned, app-lifetime instance:
  spot-checked kBpmChangeThreshold = 0.05 BPM (RecorderClock.h:70) — a tight threshold that
  looked like a possible unbounded-anchor risk for an instance that (unlike RecorderHost's,
  which is recreated per take) never resets. Traced through: BPMTracker::bpm() returns
  lockedBPM_, which is 0 (frozen/unmetered) until lock and then only changes on hysteresis-
  gated median re-evaluation (BPMTracker.cpp:131-162), not continuously — so the periodic
  32-beat re-anchor dominates growth (~1 anchor / 16s at 120 BPM), matching the plan's
  "~1000 anchors per 4-hour set" INFERRED estimate. No finding; noted only because the plan's
  own "INFERRED" label on this number turned out to be checkable and correct.

## Self-critique
The two findings above are both about arithmetic/implementation details inside a single new
method (RoutineEngine::tick), not about the plan's architecture, threading model, or grip
semantics — which is where a "runtime correctness" review is supposed to be hardest to defend
against, and where this plan is in fact well-defended (every threading and ordering claim I
checked against the current HEAD was accurate, including several I expected to catch drifted
line numbers on and didn't). The strongest counter to my TwoBar/FourBar finding is that it may
be an intentional, undocumented product choice (a routine's own "2 Bar" quantize could
legitimately mean "since fire," not "since the last phrase reset," and the plan's author may
simply have picked the more robust counter without noticing the naming collision with the
existing per-clip setting) — in which case the fix is a one-line disclosure, not a redesign.
I hold the finding at MUST rather than downgrading it, because the plan explicitly asserts
equivalence ("the same arithmetic as Layer::processPendingTrigger") where none exists, and an
unannounced semantic difference under an identical UI label ("2 Bar"/"4 Bar") is exactly the
kind of quiet failure the project's own operating discipline (layer0-block.md rule 3, "Effort
follows risk... quiet failures get the most effort") singles out.
