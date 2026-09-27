# PLAN (final) -- the routine beat clock must not lose time across a stall

Architect (chair), Fable, 2026-09-27, against main @ dc7adf9 (the draft said 1636785; every `path:line`
below was RE-READ on dc7adf9 today). Supersedes `plan-beatclock-draft.md`. Three blind seats attacked
the draft; section 0 verifies and rules on every point. A builder reads ONLY this file.
Confidence labels: VERIFIED (read/derived from the files or the recorded samples), INFERRED (the best
explanation the evidence allows), ASSUMED (not checked; the builder verifies where marked).

QUESTION: how must Audio-DNA integrate "beats elapsed" on the message thread so that a routine (and a
take being recorded) is never late by the length of a stall -- and what is, and is not, the routine
clock's fault in the two recorded slips (loadpost1 +0.5 s, reppre1 +0.155 s)?

APPROACH (stated first): carry an UNWRAPPED whole-beat counter, `FeatureSnapshot::totalBeatCount`,
written by `BPMTracker` on the analysis thread (one per `beatPhase` wrap, plus one for a realign that
lands in the second half of a beat), and make `RecorderClock` integrate `totalBeatCount + beatPhase` as
continuous beat time instead of detecting wraps of the sawtooth. A tick gap of ANY length and an
analysis catch-up burst of ANY size then add exactly the beats the tracker published. This is the
codebase's own pattern for exactly this loss class on the always-latest bus (`onsetCount`, Pitfall 30;
`totalBarCount`, Pitfall 32). `RoutineEngine`'s Beat edge becomes "the count changed", the same shape
as its Bar edge. Because the clock now keeps every beat, a looping routine can see a gap longer than a
cycle for the first time: it folds every whole cycle at once with ONE restore (a 5-line change with its
own RED). The routine clock stays EXACTLY the tracker's grid -- it never invents wall-time beats. The
second recorded slip (reppre1) is the AUDIO CLOCK pausing (the tracker's grid moved with it, for
everything beat-driven); that is a tracker-level decision for Boris, filed, not folded into this fix.
A TEST-ONLY message-thread stall hook gives the live rows their RED. Nothing else grows: no analysis-
thread hook, no `analysisSeconds` field (both dropped on the seats' evidence, section 0).

--------------------------------------------------------------------------------------------------------
## 0. Council adjudication (every point verified in the source on dc7adf9)

### Seat 1 (thread and data safety) -- verdict REVISE -> all five points ACCEPTED
- MUST "reader/writer split is sound": ACCEPT (confirmation). VERIFIED: every write to the count sits in
  `BPMTracker::updatePhase` (`src/analysis/BPMTracker.cpp:208-240`), `applyResync` (`:547-559`),
  `applyTempoRequest` (`:587-602`), `resetBeatPhase` (`:604-610`) -- member writes only, no allocation, no
  lock; `FeatureBus::read()` copies the whole 384-byte struct under one seqlock (`src/features/FeatureBus.h:
  106-146`), so `totalBeatCount` and `beatPhase` can never be torn against each other; `MainComponent::
  tickFeaturePipeline` takes ONE `snap` and feeds it to `recorderHost_.tick` (`src/MainComponent.cpp:3630`)
  and `routineEngine_.tick` (`:3639`).
- MUST "the analysis-thread stall hook fills the SPSC ring and drops audio": ACCEPT -- the hook and probe
  row b4 are DELETED. VERIFIED: `RingBuffer::push` truncates to the free space and returns the count
  written (`src/audio/RingBuffer.h:28-41`: `toWrite = min(count, available)`), so once the 16384-sample
  ring (`src/MainComponent.h:265`; 341 ms at 48 kHz) fills, the audio callback's samples are silently
  discarded; the analysis loop backs off only on an EMPTY ring (`src/analysis/AnalysisThread.cpp:86-90`).
  A sleeping analysis thread is therefore NOT the "starved then bursts" scenario -- it is a lossy-audio
  scenario the design table never claimed to cover. The burst class is pinned by the ctest case 5.2c
  (a 0.6-beat publication burst between two ticks), which needs no live hook.
- SHOULD "clamp the analysis route": ACCEPT by deletion (moot).
- SHOULD "the new reader avoids torn/unsigned-wrap bugs": ACCEPT (confirmation); the comparison is on
  doubles (section 3.4).
- NIT "layout claim verified": ACCEPT. VERIFIED `src/analysis/FeatureSnapshot.h:158-169` (resyncBarOrigin
  at 320, sizeof 384), `src/features/FeatureBus.h:127-133` (`kSnapshotWords = 96`, untouched).

### Seat 2 (musical timing) -- verdict REVISE -> all five points ACCEPTED, two change the plan
- MUST "catch-up fires every due event with zero spacing; never weighed as a musical choice": ACCEPT the
  gap; the plan now weighs it and DECIDES (section 2e): default = catch up (fire every missed event, in
  order, in the resume tick), flagged for Boris in plain words. Why it is the right default, VERIFIED:
  (i) it is the codebase's existing ruling for the identical situation in the set replay -- `Player.h:
  44-51` documents the burst as D3's intended default and `seek()` as the explicit opt-out; a routine is
  a piece of a take. (ii) today's code ALSO fires every event inside a gap -- it never drops one: it
  fires them a beat late (the lossy clock reaches their `at` a beat later) and then everything after is
  late for good. The fix changes WHEN the gap's events fire (at resume, in one 8 ms tick) and puts
  everything after back on the grid; no event's fate changes. (iii) "hold" (drop what fell in the gap)
  leaves the routine in the WRONG state until its next event on that layer, possibly bars away -- a
  missing scene change is worse than a scene change that lands 0.5 s late. (iv) discrete events are
  set-state (`Fired.p.v`: a clip column, a flag, play/pause -- `src/recording/Program.h:40-47`), so a
  burst's end state equals the programmed state; the only visible cost is the burst itself.
- MUST "risk 3 (loop re-fire under a long stall) is this failure at loop scale and is deferred": ACCEPT --
  the k-cycle fold is IN this plan (3.5b, tests 5.3e, commits 5-6). Decisive fact, VERIFIED by derivation
  from `RecorderClock.cpp:57-75`: today's reader can never advance a whole beat in ONE tick (for a gap g
  in [1, 2) it adds g-1 or freezes; the sawtooth carries only g mod 1), so the one-fold-per-tick path at
  `RoutineEngine.cpp:576-581` is UNREACHABLE today and becomes reachable ONLY through this fix. A fix
  must not open a new pathological path. And it is not exotic: a 1-beat looping stutter routine at
  120 BPM under a loadpost1-sized stall (0.55 s = 1.1 beats) folds twice -> two preambles in 16 ms.
- SHOULD "the multi-event gap is untested": ACCEPT -- 5.3c fires three points inside one gap.
- SHOULD "the TopBar citation is wrong; two real lossy readers are uncounted": ACCEPT. VERIFIED:
  `src/MainComponent.cpp:4132-4142` is `setClipFitMode`; `src/ui/TopBar.cpp:313-321` copies `beatPhase`/
  `barCount` for display with no accumulator (no loss). The exhaustive grep of `beatPhase <` in `src/`
  gives the real wrap readers: `src/recording/RoutineEngine.cpp:527` (fixed here), `src/model/Autopilot.
  cpp:44`, `src/render/Renderer.cpp:407`, `src/MainComponent.cpp:3880` (`advanceSlideshow`) and `:4041`
  (`beatSyncRandomize`; both from the 30 Hz `timerCallback`, `:423`). The last four are beat-CROSSING
  consumers (one fewer crossing under a >= 0.5-beat stall, not an accumulating clock); out of scope,
  named in Pitfall 42 as the follow-up list.
- NIT "the count arithmetic is sound": ACCEPT (confirmation).

### Seat 3 (blast radius) -- verdict REVISE -> all five points ACCEPTED, one ASSUMED resolved
- MUST "`/api/bpm.analysisSeconds` is out-of-scope API growth for a defect this plan defers": ACCEPT --
  dropped, with probe row b5. Attribution of a Class 2 event needs no new field: the draft's own section
  1b identified reppre1 from `/api/routine/status.clockBeat` deltas alone (5 hops where 20 were due,
  constant for three samples, no catch-up burst), and after this fix `clockBeat` deltas stay hop-
  quantised (verify item (d)), so the same signature analysis remains available. The Class 2 slice, if
  it ever ships, adds its own instrumentation.
- MUST "the optional analysis-thread hook is internally inconsistent and touches the hot loop with no
  evidence behind it": ACCEPT -- dropped (see seat 1). `AnalysisThread.cpp` is touched by ONE line: the
  publish of the count (3.3).
- SHOULD "the touched-file count is justified": ACCEPT (confirmation). VERIFIED: `RecorderClock` is the
  take clock (`src/recording/RecorderHost.cpp:438`) AND the routine clock (`src/recording/RoutineEngine.
  cpp:519`, member `clock_` `RoutineEngine.h:168`); `RoutineEngine::tick` has its OWN second lossy reader
  for the Beat edge (`:527`). One counter fixes both; two local heuristics would not.
- SHOULD "the stall hook's one-tick-on-wake behaviour is ASSUMED": RESOLVED to VERIFIED. The tick is
  `MappingTickTimer`, a plain `juce::Timer` (`src/MainComponent.h:306-313`, `startTimerHz(kMappingTickHz)`
  `src/MainComponent.cpp:427`, `kMappingTickHz = 120` `MainComponent.h:317`). JUCE's timer thread posts at
  most ONE `CallTimersMessage` while the message thread is busy and re-posts once after a 300 ms wait
  (`build/_deps/juce-src/modules/juce_events/timers/juce_Timer.cpp:120-139`); `callTimers()` fires a due
  timer ONCE and resets its `countdownMs = timerPeriodMs` (`:154-163`), so the second queued message finds
  `countdownMs > 0` and breaks (`:158-159`). Exactly one catch-up tick per timer per wake -- a deliberate
  sleep inside one message IS the recorded single-gap signature.
- NIT "hook hygiene is right": ACCEPT.

### Draft errors found while verifying (fixed below)
HEAD is dc7adf9; the TopBar citation (above); `handleGetBpm`'s last field is `resyncBarOrigin` at
`ApiServer.cpp:744` (the draft said insert after `:741`); pitfalls 40 and 41 exist in `docs/claude/
pitfalls.md:89,91` but `CLAUDE.md`'s index stops at 39 (`CLAUDE.md:221`) -- the index gets 40, 41, 42.

--------------------------------------------------------------------------------------------------------
## 1. Mechanism, with evidence

### 1a. Class 1 -- the reader integrates a WRAPPED phase (VERIFIED; the loadpost1 whole-beat slip)

`RecorderClock::tick` (`src/recording/RecorderClock.cpp:53-75`) sees only `snap.beatPhase` in [0,1):
```
if (phase < lastPhase_ - 0.5)       wholeBeats_ += 1        // "one wrap"            (:57-61)
else if (phase < lastPhase_)        freeze: offset absorbs   // "a resync"            (:62-71)
else                                advance by phase-last    // forward               (:72-75)
```
Between two ticks separated by g beats (tracker running), with phase p0 -> p1 = frac(p0 + g):
- g in [0.5, 1) and the gap contains a wrap (p0 >= 1-g): p1 = p0+g-1 >= p0-0.5, so the wrap is read as a
  RESYNC -> the beat FREEZES for the whole gap: **loses g** (and writes a bogus "reset" TempoMap anchor).
- g in [1, 1.5): one wrap -> p1 >= p0, read as "forward", advance g-1; two wraps -> p1 < p0-0.5, read as ONE
  wrap, advance g-1. Either way **loses exactly 1 beat**.
- In general the sawtooth carries only g mod 1; every whole beat inside a gap is lost, and a half-to-one
  beat gap with a wrap is lost entirely. The per-tick advance is ALWAYS < 1 beat (why 3.5b is new ground).

Evidence (`.harmony/.reports/s-rta-0927/routines-timing-evidence/anomalies/loadpost1-grid.json`, per-sample
derivation, 120 BPM): `clockBeat` = 62.272 for every sample from t=+0.524 s to t=+1.053 s (the message thread
published no status for >= 0.53 s -- `publishStatus()` runs only in `tick()`, `RoutineEngine.cpp:651`); the
next sample at +1.092 s reads 62.445: +0.173 beats over 0.568 s of wall, expected 1.136, **lost 0.963 ~ 1
beat**; the clock's lag is then -0.536..-0.595 s for the rest of the file (the lane's "grid spread 0.506").
Every `clockBeat` delta in both files is a multiple of 0.0213 beats = one 512-sample hop at 120 BPM
(`BPMTracker.cpp:217-218` advances the phase per hop) -- the clock is hop-quantised, as designed.

The same rule feeds the take clock (`RecorderHost.cpp:438`) and the routine clock (`RoutineEngine.cpp:519`);
a routine's position is `beat - startBeat` (`RoutineEngine.cpp:561`), so the lost beat delays every later
event of every running routine by that amount, permanently. `RoutineEngine`'s own Beat edge (`:527`) is a
second reader of the same shape: a Beat-quantised start misses any edge swallowed by a gap.

### 1b. Class 2 -- the AUDIO CLOCK paused; the grid moved with it (INFERRED, strong; the reppre1 slip)

`anomalies/reppre1-jumploop.json`, samples i=92..98 (t=+3.269..+3.482 s): 0.213 s of wall; `clockBeat`
advanced 0.107 beats = **5 hops where 20 were due**; constant for three consecutive samples (0.10 s), then
resumed at the normal ~3 hops per 33 ms sample with **no catch-up burst**; the lag stays -0.165..-0.175 s to
the end. HTTP latency stayed 1-5 ms throughout, so the HTTP threads were fine. Not Class 1: a 0.14 s gap
(0.28 beats) is handled exactly by the wrap rule. Not an analysis-thread stall: the ring holds 341 ms and
the loop drains back-to-back (`AnalysisThread.cpp:86-90`), so starvation would have shown as a ~13-hop burst.
What remains: 15 hops of audio were never delivered -- the device callback did not run for ~0.15 s (cause
unknown; the run's app-err.log was not kept). Since `BPMTracker::updatePhase` advances the phase per HOP
(`:217-218`), the beat grid IS the audio clock: bar edges, quantised clip triggers, `Autopilot`, the TopBar
count and the routine clock all moved 0.15 s later in wall time TOGETHER. The routine clock followed the
tracker exactly, which is its contract. That is a measurement of the tracker's grid, not of the clock.

### 1c. RED reproducers that need no load
Unit (ctest, section 5) and live (a TEST-ONLY `POST /api/debug/stall_message_thread {"ms":550}`, build-flag-
gated `#if AUDIODNA_TEST_SERVER`; main's build has it ON: `build/CMakeCache.txt:31`
`AUDIODNA_BUILD_TEST_SERVER:BOOL=ON`) -- one catch-up tick on wake (section 0, seat 3), so a 550 ms sleep on
a quiet machine reproduces loadpost1 deterministically: today the clock advances one beat less.

--------------------------------------------------------------------------------------------------------
## 2. Design and why it beats the alternatives

### 2a. The principle: one beat grid, the tracker's
Routines are quantised to the tracker's edges (`dueNow`, `RoutineEngine.cpp:218-229`, `barEdge` from
`totalBarCount` :526), their glide windows end on the tracker's predicted boundary (`beatsUntilBoundary`
:240-253), their loop points land on bar edges. A routine clock that keeps wall time while the tracker's
grid pauses would put every routine OFF the app's own grid. So the routine clock must be exactly "tracker
beats elapsed" -- the fix is to stop LOSING those beats on the message thread, not to replace them.

### 2b. Chosen: `totalBeatCount` in `FeatureSnapshot` (writer decides beats; reader integrates)
Writer (`BPMTracker`, analysis thread, per hop -- every decision is made where the phase is exact):
- `totalBeatCount_ += floor(phase_)` on a wrap (today `updatePhase` :221-223 discards it);
- on every hard realign to phase 0 -- confident detection (:226-229), `applyResync` (:549), a Tap
  (`applyTempoRequest` realign, :600-601), `resetBeatPhase` (:606) -- `+1 if phase_ >= 0.5` ("the beat came
  early: complete it"), else nothing ("the beat restarted"). THE SAME 0.5 rule `RecorderClock.cpp:57/62`
  applies today, moved from a 120 Hz reader (ambiguous across a gap) to the writer (unambiguous per hop);
- unlocked (`lockedBPM_ <= 0`, :210-213): untouched (the reader's unmetered path);
- a tempo VALUE (`followExternalTempo`: typed BPM, REST/OSC `set_bpm`, the Link tick, a replayed value)
  never touches phase or count -- rate only (`BPMTracker.h:149-156`, `:587-601`). Pinned by a test.
- never reset, never rewound; `uint32_t`; published every hop.

Reader (`RecorderClock`): `raw = totalBeatCount + beatPhase` is continuous beat time. `d = raw - lastRaw_`:
`d >= 0` -> the beat advances by d (a 1.1-beat gap adds 1.1; a 0.6-beat burst adds 0.6); `d < 0` -> a realign
that restarted the beat (|d| < 0.5 by the writer's rule, or a writer restart) -> absorbed into the offset
exactly as today's "reset" branch, anchor `"reset"`. Seed, unmetered, lock, bpm and periodic anchors keep
their semantics. `wholeBeats_` and `lastPhase_` disappear; `lastRaw_` replaces them.

`RoutineEngine`: `beatEdge = snap.totalBeatCount != lastTotalBeatCount_` -- identical meaning to today's
wrap test (a realign from >= 0.5 is an edge, from < 0.5 is not) minus the stall loss, the same shape as
`barEdge`. `lastBeatPhase_`/`lastBeatInBar_` stay (they feed the boundary PREDICTION only).

| situation | tracker (writer) | routine / take clock (reader) |
|---|---|---|
| message thread stalls N beats | keeps counting hops | the resume tick adds exactly N; `Player::advanceTo` fires everything due (late by the stall, 2e), later events back on the grid |
| stall longer than a looping routine's cycle | -- | every whole cycle folded in that one tick, ONE restore (3.5b) |
| analysis thread starved then bursts | processes the ring backlog back-to-back; count exact | one tick adds the whole burst -- no resync misread |
| tempo change during a stall | rate changes at the hop the request applies; count integrates hop by hop | exact; a `"bpm"` anchor on the first tick after (as today) |
| Resync / Tap during a stall | realign decided per hop with the exact phase (complete or restart) | forward -> added; backward -> absorbed (D1 continuity) |
| Link | tempo only (`LinkSync`, never realigns) | unaffected |
| manual BPM typed / REST set_bpm | rate only | unaffected (ruling: a typed value never moves the beat) |
| transport paused/stopped (File mode) | device callback still runs; hops continue | keeps time (as today) |
| no device / device restarting | no hops -> phase and count hold | the grid pauses with the audio clock -- Class 2, 2d (as today) |
| device rate change (R13) | resampler reconfigures O(1); tracker never re-created | count continuous; a callback gap is Class 2 |
| deck switch / composition load | -- | the clock is global from app start (`RoutineEngine.h:168`); `stopAll` stops routines, never the clock |
| lock lost / regained (auto) | count holds while unlocked | unmetered freeze, then `"lock"` absorption -- unchanged |

### 2c. Alternatives, and why they lose
- **Wall-time disambiguation inside `RecorderClock`** (k = round(expected wall beats - phase delta)): no
  snapshot change, but it MISREADS a publication burst > 0.5 beat as a resync and mis-rounds a long stall
  with a large tempo change inside it; a new clock-vs-clock inference where the codebase already has the
  counter pattern for this loss class. Rejected.
- **Wall-time integration (beat += dt x bpm/60):** leaves the tracker's grid on every audio-clock pause
  (2a). Rejected -- it would have "fixed" reppre1 by making routines wrong.
- **Derive the count from `totalBarCount*4 + beatInBar`:** `analyzeDownbeatPosition` re-seats `beatCounter_`
  without a wrap (`BPMTracker.cpp:430-463`) -> +-1..3-beat jumps. Rejected.
- **Put the routine clock on the analysis thread:** reset absorption, unmetered freeze and anchors are
  take-level semantics that belong on the message thread with the recorder. Rejected.
- **Fix only the probes:** the loss is real and load-independent (a restore already holds the message
  thread 38-86 ms; at 200 BPM 150 ms is half a beat). Rejected.
- **Hold instead of catch up** (drop the events that fell in a gap): rejected as the DEFAULT, section 2e.

Strongest counterargument to the chosen design: "it needs a snapshot field, a tracker change and updated
test rigs for a defect a 10-line reader heuristic would mostly cover." It loses because the heuristic is
wrong in exactly the situations a stall makes likely (a burst after starvation) and because Sacred Rule 8
(FeatureSnapshot first) plus Pitfalls 30/32 say a message-thread consumer of an analysis event stream reads
a monotonic counter -- the plan makes beats obey the rule bars and onsets already obey.

### 2d. Class 2 (audio-clock pause) -- filed, not fixed here
This plan changes nothing about how the grid behaves through a device gap. That is a tracker-semantics
question for Boris, in his words: "when the audio device hiccups for a fraction of a second while you have
tapped/typed a tempo (or Link), should the beat keep going as if nothing happened, or pause with the
audio?" Recommendation if asked: keep time in the PREDICTED regime only (manual / Link / held silence --
`predictedBeatRegime_`, `BPMTracker.h`), by advancing the phase for the audio-time deficit the analysis
thread can measure (wall elapsed minus audio processed minus audio still buffered; a growing deficit with an
EMPTY ring is a device gap, a backlog is starvation). Separate slice with its own RED and its own
instrumentation; the next live occurrence is still attributable from `clockBeat` deltas alone (section 0).

### 2e. Boris product question -- default picked, flagged
**In plain words:** when the app freezes for a moment (a deck load, a hiccup) while a routine is playing,
the routine now CATCHES UP -- every move that should have happened during the freeze happens the instant
the app comes back (all at once, in the recorded order), and everything after lands back on the beat. If
the freeze was longer than a looping routine's whole cycle, it lands in the right cycle with one restore
and does not replay the skipped cycles. The alternative is to SKIP the missed moves and continue from
where the beat is now. **Default chosen: catch up (skip nothing)** -- a skipped clip change can leave the
wrong picture on screen for bars, while a late one is a half-second stumble; and skipping would be NEW
behaviour (today nothing is skipped either -- it is only late). If Boris prefers skipping: one line in
`RoutineEngine::tick`, `r.player->seek(pos, *r.sink)` before the `advanceTo` when the tick's beat delta
exceeds a threshold (`Player.h:52-66` documents `seek` as exactly that opt-out). FLAG FOR BORIS.

--------------------------------------------------------------------------------------------------------
## 3. Exact changes, by file:line

### 3.1 `src/analysis/FeatureSnapshot.h`
- After `resyncBarOrigin` (`:126`), inside the free tier (offsets 324..383), add:
```cpp
    // s-rta-0927 beat clock: whole beats the tracker has completed since AnalysisThread started -- one per
    // beatPhase wrap, plus one for a realign (Tap / Resync / confident detection) that lands in the SECOND
    // half of a beat (the beat came early and is completed; a realign in the first half restarts the beat
    // and adds nothing). Never reset, never rewound. A tempo VALUE (typed, REST/OSC set_bpm, Link, a
    // replayed value) changes only the rate (s-rta-0926b ruling b). totalBeatCount + beatPhase is
    // continuous beat time with no wrap to detect: a message-thread reader integrates it and loses nothing
    // across a tick gap of any length or an analysis catch-up burst (RecorderClock). Reading beatPhase
    // wraps at 120 Hz misreads any gap longer than half a beat (Pitfall 42). Written only by BPMTracker on
    // the analysis thread; published every hop; /api/bpm publishes it next to totalBarCount.
    uint32_t totalBeatCount = 0;
```
- Next to `:163-166` add `static_assert(offsetof(FeatureSnapshot, totalBeatCount) == 324, "totalBeatCount
  must immediately follow resyncBarOrigin (offset 320 + 4 bytes) -- if this fails, a field was inserted/
  resized somewhere above and the layout needs re-auditing, not just re-numbering this constant");` and
  amend the `sizeof == 384` message (`:167-169`) to "the next fields are free from offset 328 up to 384".
  `sizeof` stays 384, so `FeatureBus.h:127-133` is untouched. `clear()` (`:136-147`) memsets it.

### 3.2 `src/analysis/BPMTracker.h` / `.cpp`
- `.h`: public `uint32_t totalBeatCount() const { return totalBeatCount_; }` beside `totalBarCount()` (`:116`);
  private `uint32_t totalBeatCount_ = 0;` beside `totalBarCount_` (`:265`) with the rule in a comment;
  private `void realignPhaseToZero();` beside `updatePhase` (`:313`).
- `.cpp`, new function (place after `updatePhase`):
```cpp
// s-rta-0927 beat clock: THE one rule for every hard realign (confident detection, Resync, Tap,
// resetBeatPhase). The phase is exact here (per hop), so the "did the beat complete or restart" decision
// RecorderClock.cpp used to make from a 120 Hz sample (its 0.5 rule) is made once, where a tick gap cannot
// fool it: a realign from the second half completes the beat (+1), one from the first half restarts it.
void BPMTracker::realignPhaseToZero()
{
    if (phase_ >= 0.5f)
        ++totalBeatCount_;
    phase_ = 0.0f;
}
```
  `updatePhase` (`:220-229`) becomes:
```cpp
    // Wrap at 1.0 -- s-rta-0927 beat clock: every whole beat the phase crossed is COUNTED, never discarded
    bool wrapped = (phase_ >= 1.0f);
    if (wrapped)
    {
        const float whole = std::floor(phase_);
        totalBeatCount_ += static_cast<uint32_t>(whole);
        phase_ -= whole;
    }

    // Hard reset on high-confidence beat detection from aubio
    if (beat && conf >= kBeatResetConfidence)
        realignPhaseToZero();
```
  `applyResync` (`:549`): `realignPhaseToZero();` in place of `phase_ = 0.0f;` (keep the comment).
  `applyTempoRequest` (`:600-601`): `if (realign) realignPhaseToZero();`.
  `resetBeatPhase` (`:606`): `realignPhaseToZero();` in place of `phase_ = 0.0f;` (test-only utility, same rule).
  The unlocked branch (`:210-213`) is NOT a realign: leave `phase_ = 0.0f;`, no count.
  No allocation, no lock: one integer per hop.

### 3.3 `src/analysis/AnalysisThread.cpp`
- After `:207` (`snap->resyncBarOrigin = ...`): `snap->totalBeatCount = bpmTracker_->totalBeatCount(); // s-rta-0927 beat clock`.
  This is the ONLY change to the analysis loop.

### 3.4 `src/recording/RecorderClock.h` / `.cpp`
- `.h`: replace `double wholeBeats_ = 0.0;` and `double lastPhase_ = 0.0;` (`:61`, `:63`) with
  `double lastRaw_ = 0.0;   // totalBeatCount + beatPhase at the last metered tick`. Rewrite `:22-29`:
  "Beat integration rule (D1, s-rta-0927): raw = snap.totalBeatCount + snap.beatPhase is the tracker's
  continuous beat time -- there is no wrap to detect, so a tick gap of any length or an analysis catch-up
  burst adds exactly the beats the tracker published (Pitfall 42). A DECREASE of raw is a realign that
  restarted the beat (Tap / Resync / confident detection from the first half of a beat; BPMTracker
  completes a beat from the second half by counting it) and is absorbed into an offset so `beat` stays
  monotonic and non-decreasing instead of dipping; an anchor `why:"reset"` is written." Keep the unmetered
  wording (`:27-29`).
- `.cpp` `tick` becomes (complete; the anchor calls are today's):
```cpp
void RecorderClock::tick(const FeatureSnapshot& snap, double wallNow, uint64_t deliveredSamples)
{
    // s-rta-0927 beat clock: the tracker's continuous beat time -- no wrap to detect (Pitfall 42).
    const double raw = static_cast<double>(snap.totalBeatCount) + static_cast<double>(snap.beatPhase);

    if (!haveTicked_)
    {
        haveTicked_ = true;
        startWall_ = wallNow;
        lastRaw_ = raw;
        lastBpm_ = snap.bpm;
        // `beat` counts from Record (D1 "beats since record start"), not from the tracker's count or its
        // last beat line: Record lands mid-beat (s-rta-0926 routine-grid).
        beatOffset_ = -raw;
        anchor(0.0, 0.0, deliveredSamples, snap.bpm, "start");
        current_ = { 0.0, 0.0, deliveredSamples, snap.bpm };
        return;
    }

    const double t = wallNow - startWall_;
    const bool wasUnmetered = (lastBpm_ <= 0.0f);
    const bool isUnmetered = (snap.bpm <= 0.0f);
    const double lastBeat = current_.beat;

    if (isUnmetered)
    {
        if (!wasUnmetered)
            anchor(t, lastBeat, deliveredSamples, 0.0f, "unmetered");
        // Frozen: beatOffset_/lastRaw_ untouched while unmetered. No periodic anchors while unmetered
        // (D1's documented limitation).
    }
    else if (wasUnmetered)
    {
        // Re-locked: absorb the discontinuity so the frozen value carries forward continuously.
        beatOffset_ = lastBeat - raw;
        lastRaw_ = raw;
        anchor(t, lastBeat, deliveredSamples, snap.bpm, "lock");
    }
    else
    {
        bool anchorWrittenThisTick = false;
        if (raw < lastRaw_)
        {
            // A realign that RESTARTED the beat (|d| < 0.5 by BPMTracker's rule) or a writer restart (test
            // injection): absorb into the offset so `beat` never dips (D1). A comparison on doubles --
            // never an unsigned subtraction across the count's wrap.
            beatOffset_ = lastBeat - raw;
            anchor(t, lastBeat, deliveredSamples, snap.bpm, "reset");
            anchorWrittenThisTick = true;
        }
        lastRaw_ = raw;

        const double beatNow = raw + beatOffset_;
        if (std::fabs(snap.bpm - lastBpm_) > kBpmChangeThreshold)
        {
            anchor(t, beatNow, deliveredSamples, snap.bpm, "bpm");
            anchorWrittenThisTick = true;
        }
        // Review fix (c): re-anchor at least every kPeriodicAnchorBeats (unchanged).
        if (!anchorWrittenThisTick && beatNow - lastAnchorBeat_ >= kPeriodicAnchorBeats)
            anchor(t, beatNow, deliveredSamples, snap.bpm, "periodic");
    }

    lastBpm_ = snap.bpm;
    current_.t = t;
    current_.beat = isUnmetered ? lastBeat : raw + beatOffset_;
    current_.sample = deliveredSamples;
    current_.bpm = snap.bpm;
}
```
  Steady state is value-identical to today (pinned by the existing `[recorderclock]` tests with counted rigs,
  5.2). Values differ ONLY across a stall/burst, where today's are wrong.

### 3.5 `src/recording/RoutineEngine.h` / `.cpp` -- the Beat edge
- `.h` after `:170` (`lastTotalBar_`): `uint32_t lastTotalBeatCount_ = 0;   // the Beat edge is its change (the tracker's beat, stall-proof)`;
  change `:172`'s comment to `// the tracker's beatPhase last tick (the boundary PREDICTION only)`.
- `.cpp` `:522-531` becomes:
```cpp
    // Step 2: edges, then the trackers. The Bar edge is totalBarCount's change; the Beat edge (s-rta-0927
    // beat clock) is totalBeatCount's change -- the tracker's beat, the rule a Beat-quantized clip uses,
    // read as its COUNTER so a tick gap swallows no edge (the old beatPhase-wrap test missed every edge
    // inside a gap >= half a beat, Pitfall 42). A realign from the second half of a beat is an edge (the
    // writer completed the beat), one from the first half is not -- the meaning the wrap test had.
    const bool barEdge = haveTicked_ && snap.totalBarCount != lastTotalBar_;
    const bool beatEdge = haveTicked_ && snap.totalBeatCount != lastTotalBeatCount_;
    lastTotalBar_ = snap.totalBarCount;
    lastTotalBeatCount_ = snap.totalBeatCount;
    lastBarCount_ = snap.barCount;
    lastBeatPhase_ = snap.beatPhase;
    lastBeatInBar_ = snap.beatInBar;
```
  Nothing else in the tick changes (position math `:561`, glides, restore untouched).

### 3.5b `src/recording/RoutineEngine.cpp:576-581` -- the loop folds every whole cycle at once
```cpp
            if (r.loop && r.lengthBeats > 0.0)
            {
                // s-rta-0927 beat clock: fold EVERY whole cycle the gap covers at once. The clock now keeps every
                // beat across a stall, so `pos` can exceed one cycle in a single tick (the old lossy clock never
                // advanced a whole beat per tick -- this was unreachable). One restore, for the landing cycle;
                // the skipped cycles' events never fire -- their end state IS the landing cycle's restore. One
                // fold per tick re-fired the preamble once per skipped cycle (Pitfall 42).
                const double cycles = std::floor(pos / r.lengthBeats);   // >= 1 here (pos >= lengthBeats)
                r.startBeat += cycles * r.lengthBeats;   // exact, no drift
                pos -= cycles * r.lengthBeats;
                r.cycle += static_cast<int>(cycles);
                r.player->start(0.0);
                ... `:582-606` unchanged (restore / glidePath / advanceTo(pos) / stepGlides / r.position = pos)
```
  `advanceTo(r.lengthBeats)` at `:565` (the CURRENT cycle's remainder) stays before it. `r.glideCycle`
  (`:628`) compares against the new `cycle`, so the landing cycle schedules its own return glide as today.
  A return glide scheduled before the gap has `t1` in the past: `stepGlides` lands it in one call (`u`
  clamps to 1, `:353`) and releases -- the same path the k = 1 fold takes. Ensure `<cmath>` is included.

### 3.6 `src/api/ApiServer.cpp` (+ `.h`)
- `handleGetBpm` after `:744` (`resyncBarOrigin`):
  `obj->setProperty("totalBeatCount", static_cast<juce::int64>(snap.totalBeatCount));   // s-rta-0927 beat clock`.
- `handleInjectFeatures` after `:875`: accept `"totalBeatCount"` with the `totalBarCount` clamp idiom (`:868-870`).
  (Route registered only in test mode, `:217-224` -- unchanged.)
- `setupRoutes`, after `:282` (`/api/audio/source`):
```cpp
#if AUDIODNA_TEST_SERVER
    // s-rta-0927 beat clock (TEST-ONLY build path: absent from a build without AUDIODNA_BUILD_TEST_SERVER;
    // needs no --test-mode): sleeps the MESSAGE thread for `ms` (1..2000) -- the deterministic stall
    // probe-beatclock.sh and probe-routines row 7s use as their RED. Answers at once.
    server_.Post("/api/debug/stall_message_thread", [this](const httplib::Request& req, httplib::Response& res) { handleDebugStallMessageThread(req, res); });
#endif
```
- `.h` beside `handleAudioSource` (`:208`), and the definition beside its body (`:1540-1564`), both under
  `#if AUDIODNA_TEST_SERVER`:
```cpp
// Sleeps inside ONE message, so the 120 Hz MappingTickTimer fires exactly once on wake (juce_Timer.cpp
// callTimers resets the countdown on that fire) -- the recorded loadpost1 signature, on a quiet machine.
void ApiServer::handleDebugStallMessageThread(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("ms"))
    {
        res.status = 400;
        res.set_content(jsonError("ms (1..2000) required"), "application/json");
        return;
    }
    const int ms = std::clamp(static_cast<int>(json["ms"]), 1, 2000);
    juce::MessageManager::callAsync([ms]() { juce::Thread::sleep(ms); });
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("ms", ms);
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}
```
  The define is target-wide (`CMakeLists.txt:440-442`). VERIFY the route 404s in a
  `-DAUDIODNA_BUILD_TEST_SERVER=OFF` configure (verify item (c)).

### 3.7 `src/test/TestServer.cpp`
- After `:488` (`resyncBarOrigin`): `snap->totalBeatCount = static_cast<uint32_t>(get("totalBeatCount")); // s-rta-0927: absent -> 0`.

### 3.8 Tests (section 5), probes (5.4, 5.5) and docs (section 7)
`tests/test_bpm_stabilization.cpp`, `tests/test_take.cpp`, `tests/test_routine_engine.cpp`;
`.harmony/probe-beatclock.sh` (new), `.harmony/probe-routines.sh` (row 7s); `docs/claude/analysis.md:38`,
`docs/claude/architecture.md:28`, `docs/claude/recording.md:39-41`, `docs/claude/pitfalls.md` (42 after `:91`),
`CLAUDE.md:221` (index lines 40, 41, 42), `.harmony/APP-INVENTORY.md:160` (+ a note after `:202`),
`.harmony/notebook.md` (one line).

--------------------------------------------------------------------------------------------------------
## 4. Must not change (and how the plan keeps it)
- **Glide / Ease / Jump semantics** (`RoutineEngine.cpp:255-394`, `startNow`, `resyncPending`): untouched.
  Only the VALUE of `clock_.now().beat` changes, and only across a stall/burst, where today's is wrong; 3.5b
  changes only how MANY cycles one tick folds (k = 1 is byte-for-byte today's path).
- **Stop = routines only** (`stop`/`stopAll`/`stopOnLayer`): untouched.
- **Take recording stamps + tempo maps**: format unchanged (`TempoMap::toVar/fromVar`); anchor kinds unchanged
  (`start/unmetered/lock/reset/bpm/periodic`); steady-state values identical -- pinned by the 40-minute test
  (`tests/test_take.cpp:585-624`) and the mid-beat seed test (`:654-678`). A take recorded before the fix
  loads unchanged; no `Take`/`Lane`/`Program`/`Player` file is touched.
- **Tracker outputs** `bpm`, `beatPhase`, `trackerState`, `beatInBar`, `barPhase`, `downbeatDetected`,
  `barCount`, `totalBarCount`, `resyncBarOrigin`: unchanged (additive field only -- D12 "ADD, never REDEFINE").
- **Rulings**: a typed/REST/OSC tempo never moves the beat and Link never realigns (`followExternalTempo`
  untouched; the count is untouched by a tempo value -- test 5.1d); Tap and Resync realign (their realign now
  also decides the count, by the same 0.5 rule today's reader applies). VERIFIED in `BPMTracker.h:139-156`.
- **Beat-crossing wrap readers** `Autopilot.cpp:44`, `Renderer.cpp:407`, `MainComponent.cpp:3880`, `:4041`:
  NOT touched (section 0, seat 2); listed in Pitfall 42 as the follow-up.
- **Sacred rules**: no audio-callback change; the analysis thread adds one integer increment per hop and one
  field publish (no allocation, no lock); the render thread is untouched; FeatureSnapshot first (3.1 lands
  before any reader, commit 2 before commit 4).
- **Production REST shape** grows by ONE field (`/api/bpm.totalBeatCount`, parity with `totalBarCount`); the
  stall route is a TEST-ONLY build path.

--------------------------------------------------------------------------------------------------------
## 5. Tests and probe rows -- RED on today's code

Existing rigs synthesise snapshots with `bpm`/`beatPhase` only (`tests/test_take.cpp:35-42` `makeSnap`,
`tests/test_routine_engine.cpp:206-218` `Rig::snap`, `:871-878` `tracker`). After 3.4 lands, a rig that wraps
the phase without advancing the count would read every wrap as a realign; so commit 3 updates these rigs to
publish what the real writer publishes (`totalBeatCount = floor(continuousBeat)`; an explicit count argument
where phases are scripted by hand). The OLD reader ignores the field, so the rig updates are harmless in the
RED commit. `test_recorder_host.cpp` / `test_recorder_double_touch.cpp` hold the phase constant -- no change.

### 5.1 `tests/test_bpm_stabilization.cpp` (links BPMTracker + aubio, `tests/CMakeLists.txt:320-337`) -- `[bpm][beatcount]`
RED = compile failure until 3.1/3.2 exist (additive field; written first, then GREEN in commit 2). Drive the
tracker as `test_downbeat_detector.cpp:190-210` does: `BPMTracker tracker(512, 1024, 48000);
tracker.setManualMode(true); tracker.setManualBPM(120.0f);` then per hop `tracker.processRawBPM(0.0f, 0.0f,
false); tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f);`. One hop = 1/46.875 beat at 120 BPM (0.021333).
- a) **wraps are counted**: 3060 hops -> `totalBeatCount() == 65` (65.28 wraps; `test_downbeat_detector.cpp:197`).
- b) **Tap completes or restarts the beat**: run hops until `beatPhase()` in [0.6, 0.7]; `c0 = totalBeatCount()`;
  `setManualBPM(120.0f)` (a request: applied at the START of the next hop, before its advance, `BPMTracker.cpp:
  74-79`); one hop -> `totalBeatCount() == c0 + 1`, `beatPhase() == Approx(1.0/46.875).margin(1e-4)`.
  Repeat from [0.2, 0.3] -> count unchanged, same phase.
- c) **Resync likewise**: run hops until `beatPhase()` in [0.6, 0.7]; `requestResync()` (applied at the END of
  the next `feedDownbeatFeatures`, after that hop's advance, `:336-344`); one hop -> count +1, `beatPhase() ==
  0.0f`. From [0.2, 0.3] -> count unchanged, phase 0.
- d) **Ruling pin -- a tempo VALUE never moves the beat or the count**: at `p0 = beatPhase()` ~0.63,
  `followExternalTempo(140.0f)`; one hop -> count unchanged, `bpm() == 140`, `beatPhase() == Approx(p0 +
  512.0/(48000.0*60.0/140.0)).margin(1e-4)`.
- e) **Unlocked: no count**: a FRESH tracker (never locked), 500 hops of `processRawBPM(0,0,false)` +
  `feedDownbeatFeatures(0,0,0)` -> `totalBeatCount() == 0`, `beatPhase() == 0.0f`.

### 5.2 `tests/test_take.cpp` -- `[recorderclock][stall]` (RED against the old reader; commit 3)
Helper: `FeatureSnapshot makeSnap(float bpm, float phase, uint32_t count)` (replace `:35-42`; every existing
call site passes the count -- see the rig notes below) and
`struct SyntheticTracker { double beats = 0.0; FeatureSnapshot snap(float bpm = 120.0f) const { return
makeSnap(bpm, float(beats - std::floor(beats)), uint32_t(std::floor(beats))); } };`. Ticks at 1/120 s with
`beats = 2 * t` (120 BPM); seed at t = 0.
- a) **a 1.1-beat gap loses nothing**: ticks to t = 1.0 (beats 2.0); one tick at t = 1.55 (beats 3.1); ticks to
  t = 2.0. After every post-gap tick `REQUIRE(clock.now().beat == Approx(2 * t).margin(1e-6))`.
  Today: phase 0.0 -> 0.1 reads "forward +0.1" -> 2.1: **1.0 short**.
- b) **a 0.7-beat gap that begins at phase 0.5 loses nothing**: ticks to t = 1.25 (beats 2.5); one tick at
  t = 1.6 (beats 3.2) -> `beat == Approx(3.2)`. Today: 0.2 < 0.5 reads "resync", frozen at 2.5: **0.7 short**.
- c) **a 0.6-beat publication burst between two 1/120 s ticks adds 0.6**: ticks to t = 1.35 (beats 2.7); next
  tick at t = 1.35 + 1/120 with `beats = 3.3` -> `beat == Approx(3.3)`. Today: 0.3 < 0.7 reads "resync",
  frozen at 2.7: **0.6 short**.
- d) **realign semantics pinned** (GREEN before AND after -- the writer's rule reproduces the reader's): from
  beats 2.7 a snapshot with count 3, phase 0 -> beat +0.3, no anchor; from beats 2.3 a snapshot with count 2,
  phase 0 -> beat unchanged, exactly one new `"reset"` anchor.
- Rig notes: `:248-283` scripted phases 0.2, 0.4, 0.3 (resync -> count 0), 0.5, 0.9, 0.05 (wrap -> count 1),
  unmetered steps (bpm 0, count 1), relock 0.1 / 0.3 (count 1): outcomes unchanged. `:585-624`: accumulate a
  `trueBeats` double and pass `floor(trueBeats)`. `:626-652`: constant count. `:654-678`: `beats = seedPhase`,
  `+= 1/60` per tick, count = `floor(beats)` -> still `beat == Approx(1.2)`.

### 5.3 `tests/test_routine_engine.cpp` -- `[routine][engine][stall]` (RED against the old reader; commit 3)
- `Rig::snap()` (`:206-218`) adds `s.totalBeatCount = static_cast<uint32_t>(std::floor(beat));`; the `tracker`
  lambda (`:871-878`) takes the continuous beat and sets both fields. Pattern for each case: `rig.tick();`
  (seeds the clock at beat 0 so clock beat == rig.beat), routine from `test7Routine(...)` / `makeRoutine`,
  `rig.forced == Off`.
- a) **position exact across a 1.1-beat gap; the point inside fires once**: `test7Routine(Clip::BeatSnapMode::
  Off)` in slot 0; `rig.tick(); rig.runTo(4.0); rig.fire(0);` (starts now, `startBeat` 4.0); `rig.runTo(4.5)`;
  `rig.beat = 5.6; rig.tick();` -> `slot(0).position == Approx(1.6)`, `fd.firedLanePoints(clipKey, 1) == 1`
  (the point at routine beat 1.0). Today: position 0.6, not fired.
- b) **a Beat-quantised start survives a gap that swallows the edge**: `test7Routine(Clip::BeatSnapMode::Beat)`;
  `rig.tick(); rig.runTo(4.25); rig.fire(0);` -> `state == "pending"`; `rig.beat = 4.5; rig.tick();` -> still
  pending (count 4 == 4); `rig.beat = 5.2; rig.tick();` -> `state == "running"`. Today: 0.2 < 0.0 is false,
  no edge, still pending.
- c) **several points inside one gap all fire, once each, in order, in the resume tick** (seat 2): a routine
  like test7 whose clip lane is `{ point(1, 1.0, 1), point(2, 1.25, 2), point(3, 1.5, 3) }`, quantize Off;
  `rig.tick(); rig.runTo(4.0); rig.fire(0); rig.runTo(4.5); const size_t n = rig.fd.log.size(); rig.beat = 5.6;
  rig.tick();` -> `rig.fd.on(clipKey, n)` is exactly three `Ev::Fire` with `v` 1, 2, 3 in that order;
  `slot(0).position == Approx(1.6)`. Today: none fired.
- d) Bar-edge behaviour unchanged: existing tests 7-12 pass as before (counted rigs).
- e) **a looping routine folds every whole cycle a gap covers in one tick, one restore** (`[routine][engine]
  [stall][loop]`; RED against the post-fix engine WITHOUT 3.5b -- commit 5): test 8's `build(true)` routine
  (`:359-366`; length 4, restore = op1 continuous, point v=3 at 1.0, gesture [0.5, 3.5]) in slot 0;
  `rig.tick(); rig.fire(0); rig.runTo(4.0);` -> running, `cycle == 1`; `rig.runTo(6.0)` (pos 2.0; `firedLanePoints
  (clipKey, 3) == 1`, `count(Ev::Touch, op1) == 1`); `rig.beat = 16.0; rig.tick();` (a 10-beat gap = 2.5 cycles):
  `slot(0).cycle == 4`, `slot(0).position == Approx(0.0).margin(1e-9)`, `count(Ev::Touch, op1) == 2` (ONE more
  restore), `firedLanePoints(clipKey, 3) == 1` (nothing due at pos 0; the skipped cycles' points never fired);
  then `rig.runTo(17.0)` -> `firedLanePoints(clipKey, 3) == 2`, `cycle == 4`. Without 3.5b the resume tick
  gives `cycle == 2`, touches 2, points 2 (and two more folds over the next two ticks).

### 5.4 Live -- `.harmony/probe-beatclock.sh` (new; production mode, port 7070; copy probe-routines' idioms:
the live-lock gate `:61-67`, `adna_pids/adna_running/adna_kill` `:70-72`, `ok`/`no`/`rows` `:89-102`, `now`/`since`
`:592-593`, `open -g` launch + health wait `:607-612`, `POST /api/set_bpm {"bpm":120}` `:617`, the graceful
osascript quit + Quartz 0-Output-window witness (tail); no fixture, no take; ~40 s)
- b0 guard: `POST /api/debug/stall_message_thread {"ms":1}`; on 404 print `SKIP: no TEST-ONLY hook in this
  binary` and exit 0 after b1.
- b1 `/api/bpm.totalBeatCount` advances 6 +- 1 over 3.0 s (today: field absent -> `no ... absent`).
- b2 **550 ms message-thread stall**: read `clockBeat` (`/api/routine/status`, published with no routine running
  -- the clock ticks from app start) and wall; `POST stall {"ms":550}`; sleep 0.8; read again; PASS iff
  `|dBeat - 2*dWall| <= 0.10` (hop quantisation + one stale tick <= 0.04). Today: ~1.0 short -> FAIL
  (deterministic: g in [1, 1.5) always loses 1).
- b3 **350 ms stall started mid-beat**: poll `/api/bpm` at 100 Hz until `beatPhase` in [0.30, 0.55], then stall
  350; same check. Today: 0.7 short -> FAIL (the gap contains the wrap and reads as a resync).
- RED evidence: run against a binary built with ONLY commit 1 -> b2/b3 FAIL by the amounts above; after
  commit 4 -> PASS x3. Both recorded in the lane report.

### 5.5 Live -- `.harmony/probe-routines.sh` row **7s "grid through a stall"** (after row 7, `:725-727`)
`POST /api/routine/fire {"slot":0}` again (Probe Routine, once, no loop; the recorded marks are at +0.6, +2.6,
+4.6, +6.6 s -- `rt.py grid`, `:337-340`; a fifth recorded move at +1.6 is not a mark); `T1s="$(python3 "$RT"
wait 0 running 2.6)"`; sleep until `T1s + 1.0`; `P /api/debug/stall_message_thread '{"ms":550}'` (the window
+1.0..+1.55 s contains NO mark, so a mark inside the stall cannot fail the row for the wrong reason);
`python3 "$RT" sample 8.9 "$OUT/grid-stall.json"` started right after the fire (as `:699`); `rows python3 "$RT"
grid "$T1s" "$OUT/grid-stall.json"` (reuses the `grid` analysis verbatim). Guard like b0: if the stall route
404s, print SKIP for 7s. Today: marks 2-4 arrive +0.50 s late -> 3 FAILs (tolerance 0.35). After: 98 + 7 rows
PASS; run x3 quiet. `P /api/routine/stop '{"all":true}'` after the row.

--------------------------------------------------------------------------------------------------------
## 6. Risks
1. **The 0.5 decision moves from reader to writer.** For a Tap/Resync within ~one tick (8 ms) of the half-beat
   point, today's reader and the new writer can decide differently (complete vs restart) -- a one-beat
   difference in where a RUNNING routine's continuity lands, in a ~1-2 % window per realign. Accepted: the
   writer has the exact phase; the reader was guessing from a sample. Named in the pitfall.
2. **Writers that do not publish the count.** Test-mode injection building a moving `beatPhase` with a constant
   `totalBeatCount` freezes the routine/take clock (every wrap reads as a realign). Both inject handlers
   accept the field (3.6/3.7). VERIFIED: no `tests/visual/*` file mentions routines (grep), so no visual
   sweep runs routines while driving `beatPhase`.
3. **Catch-up burst is the default (2e).** A multi-second freeze resumes with every missed event in one tick.
   Bounded: 3.5b caps a looping routine at one cycle's events + one restore; a once routine fires at most its
   remaining events. The events are set-state (no toggles: `Fired.p.v`), so the end state is right. FLAGGED
   for Boris with the one-line alternative.
4. **`"reset"` anchor density in AUTO mode is unchanged but now understood:** a late confident detection (phase
   0.03 -> 0) is a small backward step today AND after -> a `"reset"` anchor each time. Not this defect.
5. **Class 2 remains by design** (2d): everything beat-driven pauses with the audio clock; after this fix
   routines are consistent with it. A live Class 2 event during the GREEN runs would fail probe-routines' wall-
   time rows and is recognised by its signature (constant `clockBeat` over several samples, no burst, lag
   held to the end) -- triage it as Class 2, not as a regression.
6. **Layout.** 4 bytes at offset 324 keep `sizeof == 384` (static_asserts fail the build otherwise).
7. **Stall hook hygiene.** It sleeps the message thread on purpose; only in a build with
   `AUDIODNA_BUILD_TEST_SERVER=ON` (main's default DEV build -- reachable on 7070 without `--test-mode`, the
   same unauthenticated local trust as `load_composition`), clamped 1..2000 ms; b0/7s guards skip cleanly
   when absent. Pitfall 31 (bodyless POST) does not apply -- it carries a body.
8. **Steady-state regression risk in the reader rewrite** is bounded by the existing long/mid-beat/monotonic
   tests plus 5.2d; run `ctest` fully under the sanitizer config the tests already use.
9. **`cycle` numbering skips after a fold of k > 1** (status shows the landing cycle number). Honest -- the
   skipped cycles did not run. Probe row 8/8g/11j (loop) exercise k = 1 only and stay unchanged.

Verify before/while building: (a) `sizeof(FeatureSnapshot) == 384` still compiles; (b) `grep -n "new\|malloc\|
mutex" src/analysis/BPMTracker.cpp` unchanged; (c) the debug route 404s in a `-DAUDIODNA_BUILD_TEST_SERVER=OFF`
configure; (d) after commit 4, `clockBeat` deltas on `/api/routine/status` remain hop-quantised (0.0213
multiples at 120 BPM) -- proof the reader still follows the tracker and did not start integrating wall time;
(e) probe-routines x3 quiet after commit 4 and again after commit 6; (f) `ctest` 100 % after every commit
except commits 3 and 5, whose named new cases are the ONLY failures.

--------------------------------------------------------------------------------------------------------
## 7. Commit sequence (lane branch; each step's state is checkable)
1. `probe(beatclock): TEST-ONLY POST /api/debug/stall_message_thread; .harmony/probe-beatclock.sh; probe-routines
   row 7s; inventory note` -- 3.6 (route + handler, `#if AUDIODNA_TEST_SERVER`), the two probes, the inventory
   note after `APP-INVENTORY.md:202`. Build; run both probes: **RED recorded** (b2 ~-1.0 beat, b3 ~-0.7, 7s
   three marks +0.5 s late; b1 absent). This is the live RED, on the pre-fix reader.
2. `feat(analysis): FeatureSnapshot::totalBeatCount -- BPMTracker counts wraps and second-half realigns;
   published every hop; /api/bpm + inject paths` -- 3.1, 3.2, 3.3, 3.6 (bpm field, inject), 3.7, 5.1 tests
   (compile-RED then GREEN in this commit), `analysis.md:38` row, `architecture.md:28` row,
   `APP-INVENTORY.md:160` (add `totalBeatCount`; `resyncBarOrigin` is published at `ApiServer.cpp:744` but
   missing from the row -- add it too). ctest 100 %; probe b1 PASS.
3. `test(recording): RED -- RecorderClock and RoutineEngine lose beats across a tick gap or a publication
   burst` -- 5.2 a-d, 5.3 a-c, the rig updates (counts everywhere). ctest shows EXACTLY 5.2a-c and 5.3a-c
   failing (named in the commit message); 5.2d and everything else GREEN.
4. `fix(recording): RecorderClock integrates totalBeatCount + beatPhase; RoutineEngine Beat edge from the count
   -- no beat lost across a stall or burst` -- 3.4, 3.5 only. ctest 100 %; probe-beatclock b2/b3 PASS x3;
   probe-routines 98 + 7 rows PASS x3; verify item (d).
5. `test(recording): RED -- a looping routine spins one catch-up cycle per tick after a gap longer than a
   cycle` -- 5.3e. ctest shows exactly this case failing (`cycle == 2`, touches 2, points 2 at the resume tick).
6. `fix(recording): a loop folds every whole cycle a gap covers in one tick, one restore` -- 3.5b;
   `pitfalls.md` 42 (after `:91`), `CLAUDE.md:221` index lines 40/41/42, `recording.md:39-41` ("a
   `RecorderClock` shared across all running routines -- it integrates `totalBeatCount + beatPhase`, the
   tracker's continuous beat time, so a message-thread stall loses no beats and a loop folds every whole cycle
   a gap covers at once; Pitfall 42"), `RecorderClock.h` comment if not done in 4, `.harmony/notebook.md` one
   line. ctest 100 %; probe-routines x3 quiet (rows 8/8g/11j unchanged). Lane report cites the RED runs of
   steps 1/3/5 and the GREEN runs of 4/6, plus the Boris flag (2e) verbatim.
7. (separate slice, needs Boris) `tracker: keep wall time through an audio-device gap in the predicted regime`
   -- section 2d.

Done looks like: a 550 ms deterministic message-thread stall moves `clockBeat` by exactly 2 x elapsed seconds
at 120 BPM (+-0.1) on the main app; a routine fired before the stall lands its later moves within +-0.35 s of
the recorded grid; a looping routine lands in the right cycle with one restore after a gap of any length;
every existing take/clock/engine test still passes; `/api/bpm` shows `totalBeatCount` running at 2/s.

### Pitfall 42 (text for `docs/claude/pitfalls.md`, after 41; one-line index for `CLAUDE.md`)
42. **A beat-phase sawtooth read at tick rate loses every whole beat inside a tick gap -- a message-thread beat
CLOCK integrates `FeatureSnapshot::totalBeatCount + beatPhase`, never the wrap**: `RecorderClock` (the take clock
and the routine clock) detected beats as `beatPhase < last - 0.5` at 120 Hz; a gap of g beats (a load holding
the message thread -- 0.53 s in s-rta-0927's loadpost1 sample) carries only g mod 1: g in [1, 1.5) lost exactly
one beat, a 0.5..1-beat gap containing a wrap read as a resync and FROZE the clock for the whole gap (and wrote a
bogus `"reset"` anchor) -- every later event of every running routine, and every stamp of a take being
recorded, late by that amount for good. `BPMTracker` now counts whole beats where the phase is exact (per hop):
+1 per wrap, +1 for a hard realign (Tap / Resync / confident detection) from the second half of a beat ("the
beat came early: complete it"), +0 from the first half ("restart"); a tempo VALUE (typed, REST/OSC set_bpm,
Link, replay) never touches it. `RecorderClock` integrates `raw = totalBeatCount + beatPhase` (`raw < lastRaw_`
= a realign that restarted the beat: absorbed as continuity, compared on doubles -- never an unsigned
subtraction); `RoutineEngine`'s Beat edge is the counter's change, the same shape as its Bar edge
(`totalBarCount`, Pitfall 32; `onsetCount`, Pitfall 30 -- the counter pattern for every analysis event a slower
reader consumes). Two things a stall now exposes: (1) `Player::advanceTo` fires every event that fell inside
the gap in the resume tick (D3 exactly-once, `Player.h`; `seek()` is the documented opt-out) -- the events were
never dropped before either, only late; (2) a looping routine folds EVERY whole cycle the gap covers in one
tick with ONE restore (one fold per tick re-fired the preamble per skipped cycle). Still reading the wrap
(beat-CROSSING consumers: one fewer crossing under a stall, not an accumulating clock): `Autopilot.cpp` beat
mode, `Renderer.cpp` projectM playlist, `MainComponent::advanceSlideshow` / `beatSyncRandomize` (30 Hz).
Test-mode injection (`/api/inject_features`, Eyes `inject_features`) must move `totalBeatCount` with `beatPhase`
or the routine/take clock freezes. The AUDIO clock pausing (device callback gap, reppre1) moves the whole
grid -- bars, quantised clips, routines together -- and is a tracker-level decision, not this defect. Guards:
`tests/test_take.cpp [recorderclock][stall]`, `tests/test_routine_engine.cpp [routine][engine][stall]`,
`tests/test_bpm_stabilization.cpp [bpm][beatcount]`; live: `.harmony/probe-beatclock.sh` (TEST-ONLY `POST
/api/debug/stall_message_thread`), `.harmony/probe-routines.sh` row 7s.
Index line: `42. A beat clock integrates totalBeatCount + beatPhase, never the beatPhase wrap -- before reading beatPhase on the message thread as a clock or an edge.`
(Also add the missing index lines: `40. Output windows: normal level, never key, IOSurface frames only -- before touching OutputManager / an output window's GL.` and `41. LayerStrip faders follow the model from the timer -- before caching a model value in a widget.`)

## COMPACT
- Root cause 1 (VERIFIED): `RecorderClock.cpp:57-75` integrates a wrapped phase; a gap >= 0.5 beat with a wrap
  is read as a resync (frozen) and every whole beat inside a gap is lost -> loadpost1 lost exactly 1 beat;
  `RoutineEngine.cpp:527` is a second reader of the same shape (Beat edge).
- Root cause 2 (INFERRED): reppre1 = the audio clock paused ~0.15 s; the tracker's grid moved with it; not a
  routine-clock defect under "one grid"; tracker-level ruling needed (filed, 2d).
- Fix: `FeatureSnapshot::totalBeatCount` (wraps + second-half realigns, written by BPMTracker), `RecorderClock`
  integrates count+phase, `RoutineEngine` Beat edge = count change, loop folds k cycles at once; TEST-ONLY
  message-thread stall hook + probe rows for RED. Dropped on council evidence: analysis-thread hook (fills the
  SPSC ring, drops audio), `/api/bpm.analysisSeconds` (out of scope). Timer wake = one tick (JUCE source).
- Boris flag: after a freeze a routine catches up (fires the missed moves at once) rather than skipping them;
  one-line switch if he prefers skipping.
- Files: FeatureSnapshot.h, BPMTracker.h/.cpp, AnalysisThread.cpp (1 line), RecorderClock.h/.cpp,
  RoutineEngine.h/.cpp, ApiServer.cpp/.h, TestServer.cpp, 3 test files, 2 probes, docs/pitfall 42, inventory,
  CLAUDE.md index. Format of takes unchanged. Six commits + a filed seventh.

STATUS: COMPLETE -- final builder-executable plan written; no app code changed; open decisions for Boris: (1) catch up vs skip after a freeze (default: catch up, flagged 2e); (2) Class 2, the grid through an audio-device gap (separate slice, 2d).
