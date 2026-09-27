# PLAN (draft) -- the routine beat clock must not lose time across a stall

Architect, Fable, 2026-09-27, against main @ 1636785. Every `path:line` below was read on this tree today.
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
as its Bar edge. The routine clock stays EXACTLY the tracker's grid -- it never invents wall-time beats.
The second recorded slip (reppre1) is the AUDIO CLOCK pausing (the tracker's grid moved with it, for
everything beat-driven); that is a tracker-level decision for Boris, sketched as a separate slice, not
folded into this fix. A TEST-ONLY message-thread stall hook gives the live rows their RED.

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
  beat gap with a wrap is lost entirely.

Evidence (`routines-timing-evidence/anomalies/loadpost1-grid.json`, my per-sample derivation, 120 BPM):
`clockBeat` = 62.272 for every sample from t=+0.524 s to t=+1.053 s (the message thread published no
status for >= 0.53 s -- `publishStatus()` runs only in `tick()`, `RoutineEngine.cpp:651`); the next sample at
+1.092 s reads 62.445: +0.173 beats over 0.568 s of wall, expected 1.136, **lost 0.963 ~ 1 beat**; the
clock's lag is then -0.536..-0.595 s for the rest of the file (the lane's `analysis.txt` "grid spread
0.506"). Every `clockBeat` delta in both files is a multiple of 0.0213 beats = one 512-sample hop at 120
BPM (`BPMTracker.cpp:217-218` advances the phase per hop) -- the clock is hop-quantised, as designed.

The same rule feeds the take clock (`RecorderHost.cpp:438`) and the routine clock (`RoutineEngine.cpp:519`,
`clock_.tick(snap, wallNow, 0)`); a routine's position is `beat - startBeat` (`RoutineEngine.cpp:561`), so
the lost beat delays every later event of every running routine by that amount, permanently.

### 1b. Class 2 -- the AUDIO CLOCK paused; the grid moved with it (INFERRED, strong; the reppre1 slip)

`anomalies/reppre1-jumploop.json`, samples i=92..98 (t=+3.269..+3.482 s): 0.213 s of wall; `clockBeat`
advanced 0.107 beats = **5 hops where 20 were due**; `clockBeat` was constant for three consecutive samples
(0.10 s), then resumed at the normal ~3 hops per 33 ms sample with **no catch-up burst** (deltas 0.021,
0.043, 0.085, 0.064, 0.064 ...); the lag stays -0.165..-0.175 s to the end. HTTP latency stayed 1-5 ms
throughout (`ts-t`, `tc-t`), so the HTTP threads were fine.

Why this is not Class 1: a message-thread gap of 0.14 s = 0.28 beats is handled EXACTLY by the wrap rule
(for g < 0.5 a wrap always reads as `p1 < p0-0.5`) -- the phase-wrap reader loses nothing here. Why it is not
an analysis-thread stall: the analysis ring holds 16384 floats = 341 ms (`src/MainComponent.h:268`;
`docs/claude/architecture.md:101`) and the loop drains back-to-back (`AnalysisThread.cpp:86-90`), so a
0.14 s starvation would have shown as a ~13-hop burst in the next one or two samples; none is there.
What remains: 15 hops of audio were never delivered -- the device callback did not run for ~0.15 s
(cause unknown; a CoreAudio device restart/default-device change or HAL overload are the usual ones; the
run's app-err.log was not kept, so the device rate is unknown too). Since `BPMTracker::updatePhase`
advances the phase per HOP (`BPMTracker.cpp:217-218`), the beat grid IS the audio clock: bar edges,
quantised clip triggers, `Autopilot` (`src/model/Autopilot.cpp:44`), the TopBar count and the routine clock
all moved 0.15 s later in wall time TOGETHER. The routine clock followed the tracker exactly, which is its
contract. The lane's rows 7/8g measure wall time and so flagged it; that is a measurement of the tracker's
grid, not of the routine clock.

### 1c. RED reproducers that need no load

Unit (ctest, section 5): a synthetic tick stream with a 0.55 s gap (1.1 beats) -> today's `RecorderClock`
is short by 1.0 beat; a 0.35 s gap that begins at phase 0.5 -> today frozen, short by 0.7; a 0.6-beat
publication burst between two ticks -> today read as a resync, short by 0.6; a `RoutineEngine` position
across a 1.1-beat gap -> today 1.0 short, and the point inside the gap not fired.
Live (section 5): a TEST-ONLY `POST /api/debug/stall_message_thread {"ms":550}` (build-flag-gated,
`#if AUDIODNA_TEST_SERVER`, main's build has it ON: `build/CMakeCache.txt` `AUDIODNA_BUILD_TEST_SERVER:BOOL=ON`)
sleeps the message thread deterministically; the routine clock must advance 2 x elapsed seconds across it.
Today it advances one beat less -- the loadpost1 signature, on a quiet machine.

--------------------------------------------------------------------------------------------------------
## 2. Design and why it beats the alternatives

### 2a. The principle: one beat grid, the tracker's

Routines are quantised to the tracker's edges (`dueNow`, `RoutineEngine.cpp:218-229`, `barEdge` from
`totalBarCount` :526), their glide windows end on the tracker's predicted boundary (`beatsUntilBoundary`
:240-253), their loop points are meant to land on bar edges. A routine clock that keeps wall time while the
tracker's grid pauses would put every routine OFF the app's own grid (its beat 16 before the fourth bar
edge). So the routine clock must be exactly "tracker beats elapsed" -- and the fix is to stop LOSING those
beats on the message thread, not to replace them with wall time.

### 2b. Chosen: `totalBeatCount` in `FeatureSnapshot` (writer decides beats; reader integrates)

Writer (`BPMTracker`, analysis thread, per hop -- so every decision is made where the phase is exact):
- `totalBeatCount_ += floor(phase_)` on a wrap (today `updatePhase` :221-223 discards it);
- on every hard realign to phase 0 -- confident detection (:226-229), `applyResync` (:549), a Tap
  (`applyTempoRequest` realign, :600-601), `resetBeatPhase` (:606) -- `+1 if phase_ >= 0.5` ("the beat came
  early: complete it"), else nothing ("the beat restarted"). This is THE SAME 0.5 rule `RecorderClock.cpp:57/62`
  applies today, moved from a 120 Hz reader (ambiguous across a gap) to the writer (unambiguous per hop);
- unlocked (`lockedBPM_ <= 0`, :210-213): untouched (the reader's unmetered path);
- a tempo VALUE (`followExternalTempo`: typed BPM, REST/OSC `set_bpm`, the Link tick, a replayed value,
  `MainComponent.cpp:5210-5235`) never touches phase or count -- rate only. Ruling pinned by a test.
- never reset, never rewound; `uint32_t` (wraps after 400 years at 200 BPM); published every hop.

Reader (`RecorderClock`): `raw = totalBeatCount + beatPhase` is continuous beat time.
`d = raw - lastRaw_`: `d >= 0` -> the beat advances by d (a 1.1-beat gap adds 1.1; a 0.6-beat burst adds 0.6);
`d < 0` -> a realign that restarted the beat (|d| < 0.5 by construction, or a writer restart) -> absorbed into
the offset exactly as today's "reset" branch, anchor `"reset"`. Seed, unmetered, lock, bpm and periodic
anchors keep their semantics. `wholeBeats_` and `lastPhase_` disappear; `lastRaw_` replaces them.

`RoutineEngine`: `beatEdge = snap.totalBeatCount != lastTotalBeatCount_` -- identical meaning to today's
wrap test (a realign from >= 0.5 is an edge, from < 0.5 is not) minus the stall loss, and the same shape as
`barEdge`. `lastBeatPhase_`/`lastBeatInBar_` stay (they feed the boundary PREDICTION only).

What happens in each named situation (all follow from the definition; each is a test in section 5):
| situation | tracker (writer) | routine / take clock (reader) |
|---|---|---|
| message thread stalls N beats | keeps counting hops | next tick adds exactly N; `Player::advanceTo` fires everything due (late by the stall, unavoidable), later events back on the grid |
| analysis thread starved then bursts | processes the ring backlog back-to-back; count exact | one tick adds the whole burst -- no resync misread |
| tempo change during a stall (typed/REST/OSC/Link) | rate changes at the hop the request is applied; count integrates hop by hop at each rate | exact; a `"bpm"` anchor is written on the first tick after (as today) |
| Resync / Tap during a stall | realign decided per hop with the exact phase (complete or restart the beat) | forward -> added; backward -> absorbed (continuity, D1) -- a running routine never jumps back |
| Link | tempo only (`LinkSync::getBPM`, `MainComponent.cpp:3740-3745` -> `"link"`), never realigns | unaffected |
| manual BPM typed / REST set_bpm | `followExternalTempo` -- rate only | unaffected (ruling: a typed value never moves the beat) |
| transport paused/stopped (File mode) | the device callback still runs; silence -> hops continue; manual: free-runs; auto: P23 silence hold free-runs (`BPMTracker.h:120-123`) | keeps time (as today) |
| no device / device stopped or restarting (rate change) | no hops -> phase and count hold | the grid pauses with the audio clock -- Class 2, section 2d (as today) |
| device rate change | resampler reconfigures O(1), hop stays 512 @ 48 kHz (R13), tracker never re-created | count continuous; a callback gap during the JUCE device restart is Class 2 |
| deck switch / composition load | -- | the clock is global from app start (`RoutineEngine.h:168`); `stopAll` stops routines, never the clock |
| lock lost / regained (auto mode) | count holds while unlocked | unmetered freeze, then `"lock"` absorption -- unchanged |

### 2c. Alternatives, and why they lose

- **Wall-time disambiguation inside `RecorderClock`** (k = round(expected wall beats - phase delta)): no
  snapshot change and it passes every rig that keeps wall and phase consistent; but it MISREADS a publication
  burst > 0.5 beat as a resync (an analysis starvation of ~300 ms at 120 BPM, ~150 ms at 200 BPM is enough) and
  mis-rounds a long stall with a large tempo change inside it. It is a new clock-vs-clock inference pattern
  where the codebase already has a counter pattern for this loss class. Rejected.
- **Wall-time integration (beat += dt x bpm/60) with the phase as fine correction:** leaves the tracker's grid on
  every audio-clock pause (section 2a). Rejected -- it would have "fixed" reppre1 by making routines wrong.
- **Derive the count from `totalBarCount*4 + beatInBar`:** `analyzeDownbeatPosition` re-seats `beatCounter_`
  without a wrap (`BPMTracker.cpp:430-463`) -> +-1..3-beat jumps of the derived count. Rejected.
- **Put the routine clock on the analysis thread:** the reset absorption, unmetered freeze and anchors are
  take-level semantics that belong on the message thread with the recorder. Rejected.
- **Fix only the probes:** the loss is real and load-independent (a restore already holds the message thread
  38-86 ms, lane finding 2; at 200 BPM 150 ms is half a beat). Rejected.

Strongest counterargument to the chosen design: "it needs a snapshot field, a tracker change and updated
test rigs for a defect a 10-line reader heuristic would mostly cover." It loses because the heuristic is
wrong in exactly the situations a stall makes likely (a burst after starvation) and because Sacred Rule 8
(FeatureSnapshot first) plus Pitfalls 30/32 say a message-thread consumer of an analysis event stream reads
a monotonic counter -- the plan makes beats obey the rule bars and onsets already obey.

### 2d. Class 2 (audio-clock pause) -- what this plan does and does not do

Does: instruments it (`/api/bpm` publishes `analysisSeconds`, the analysis clock, so a probe can diff it
against wall and see a device pause as a step -- the row the lane could not have) and files the decision.
Does not: change how the grid behaves through a device gap. That is a tracker-semantics question for Boris,
in his words: "when the audio device hiccups for a fraction of a second while you have tapped/typed a tempo
(or Link), should the beat keep going as if nothing happened, or pause with the audio?" Recommendation if
asked: keep time in the PREDICTED regime only (manual / Link / held silence -- `predictedBeatRegime_`,
`BPMTracker.h:213-226`), by advancing the phase for the audio-time deficit the analysis thread can measure
(wall elapsed minus audio processed minus audio still buffered in the ring; a growing deficit with an EMPTY
ring is a device gap, a backlog is starvation). It never realigns (rate x deficit), so the rulings hold. RED
for that slice: a `BPMTracker`/`AnalysisThread` test with a simulated 150 ms device gap expecting the phase to
have advanced 0.3 beats at 120 BPM. Out of this plan's commits: tracker scope, needs the ruling, and needs
the deficit detector's own edge cases (a device whose real rate differs from its reported one would drift).

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
- Add `static_assert(offsetof(FeatureSnapshot, totalBeatCount) == 324, "...")` next to `:163-166`; amend the
  `sizeof == 384` message (`:167-169`) to "next fields are free from offset 328 up to 384". `sizeof` stays 384,
  so `src/features/FeatureBus.h:129-133` (`kSnapshotWords = 96`) is untouched. `clear()` (`:136-147`) memsets it.

### 3.2 `src/analysis/BPMTracker.h` / `.cpp`
- `.h`: public accessor `uint32_t totalBeatCount() const { return totalBeatCount_; }` beside `totalBarCount()`
  (`:116`); private member `uint32_t totalBeatCount_ = 0;` beside `totalBarCount_` (`:265`), with the rule
  in a comment; private `void realignPhaseToZero();` beside `updatePhase` (`:313`).
- `.cpp`:
```cpp
void BPMTracker::realignPhaseToZero()   // the one rule for every hard realign
{
    if (phase_ >= 0.5f) ++totalBeatCount_;   // the beat came early: complete it (RecorderClock.cpp's old
    phase_ = 0.0f;                           // reader-side 0.5 rule, now decided per hop where phase is exact)
}
```
  `updatePhase` (`:221-229`): on wrap `const float whole = std::floor(phase_); totalBeatCount_ += static_cast<uint32_t>(whole); phase_ -= whole;`
  then `if (beat && conf >= kBeatResetConfidence) realignPhaseToZero();`.
  `applyResync` (`:549`): `realignPhaseToZero();` in place of `phase_ = 0.0f;`.
  `applyTempoRequest` (`:600-601`): `if (realign) realignPhaseToZero();`.
  `resetBeatPhase` (`:606`): `realignPhaseToZero();` (test-only utility; same rule).
  The unlocked branch (`:210-213`) is NOT a realign: leave `phase_ = 0.0f`, no count.
  No allocation, no lock: one integer per hop.

### 3.3 `src/analysis/AnalysisThread.cpp`
- After `:207` (`snap->resyncBarOrigin = ...`): `snap->totalBeatCount = bpmTracker_->totalBeatCount();`.

### 3.4 `src/recording/RecorderClock.h` / `.cpp`
- `.h`: replace `wholeBeats_`/`lastPhase_` (`:61-63`) with `double lastRaw_ = 0.0;   // totalBeatCount + beatPhase at the last tick`.
  Rewrite the header comment (`:22-29`) to the count rule (keep D1's continuity/unmetered wording).
- `.cpp` `tick` becomes (structure; the anchor calls are today's):
```cpp
    const double raw = static_cast<double>(snap.totalBeatCount) + static_cast<double>(snap.beatPhase);
    if (!haveTicked_) { haveTicked_ = true; startWall_ = wallNow; lastRaw_ = raw; lastBpm_ = snap.bpm;
                        beatOffset_ = -raw;           // beat 0 at Record (D1), whatever the tracker's count/phase
                        anchor(0.0, 0.0, deliveredSamples, snap.bpm, "start"); current_ = {0.0, 0.0, deliveredSamples, snap.bpm}; return; }
    const double t = wallNow - startWall_;
    const bool wasUnmetered = (lastBpm_ <= 0.0f), isUnmetered = (snap.bpm <= 0.0f);
    const double lastBeat = current_.beat;
    if (isUnmetered)            { if (!wasUnmetered) anchor(t, lastBeat, deliveredSamples, 0.0f, "unmetered"); }   // frozen
    else if (wasUnmetered)      { beatOffset_ = lastBeat - raw; lastRaw_ = raw; anchor(t, lastBeat, deliveredSamples, snap.bpm, "lock"); }
    else {
        bool anchorWrittenThisTick = false;
        if (raw < lastRaw_) {   // a realign restarted the beat (or a writer restart): absorb, never dip (D1)
            beatOffset_ = lastBeat - raw; anchor(t, lastBeat, deliveredSamples, snap.bpm, "reset"); anchorWrittenThisTick = true; }
        lastRaw_ = raw;
        const double beatNow = raw + beatOffset_;
        ...bpm anchor (kBpmChangeThreshold) and periodic anchor (kPeriodicAnchorBeats) exactly as :78-89...
    }
    lastBpm_ = snap.bpm; current_.t = t; current_.beat = isUnmetered ? lastBeat : raw + beatOffset_;
    current_.sample = deliveredSamples; current_.bpm = snap.bpm;
```
  Note `raw < lastRaw_` also covers a writer restart with a smaller count (test-mode injection): absorbed
  as continuity, never a 4e9-beat leap -- the comparison is on doubles, no unsigned subtraction.

### 3.5 `src/recording/RoutineEngine.h` / `.cpp`
- `.h` `:172`: add `uint32_t lastTotalBeatCount_ = 0;   // the Beat edge is its change (the tracker's beat, stall-proof)`.
- `.cpp` `:522-531`: `const bool beatEdge = haveTicked_ && snap.totalBeatCount != lastTotalBeatCount_;` and
  `lastTotalBeatCount_ = snap.totalBeatCount;`; keep `lastBeatPhase_ = snap.beatPhase;` (prediction). Update
  the comment: the Beat edge is the tracker's beat, read as its counter's change (a wrap read at 120 Hz
  misses one under a stall).
- Nothing else in the engine changes (position math `:561`, glides, loop, restore untouched).

### 3.6 `src/api/ApiServer.cpp` (+ `.h` for the new handler)
- `handleGetBpm` after `:741`: `obj->setProperty("totalBeatCount", static_cast<juce::int64>(snap.totalBeatCount));`
  and `obj->setProperty("analysisSeconds", snap.wallClockSeconds);` (the analysis clock, `AnalysisThread.cpp:334-335`
  -- audio seconds processed at 48 kHz; diffed against wall by the probe to see an audio-clock pause).
- `handleInjectFeatures` after `:875`: accept `"totalBeatCount"` with the `totalBarCount` clamp idiom (`:868-870`).
- `setupRoutes` (`:137`): TEST-ONLY build path, next to `/api/audio/source` (`:282`):
```cpp
#if AUDIODNA_TEST_SERVER
    // s-rta-0927 beat clock (TEST-ONLY build path: absent from a build without AUDIODNA_BUILD_TEST_SERVER):
    // sleeps the MESSAGE thread for `ms` (1..2000) -- a deterministic stall for probe-beatclock.sh. Answers at once.
    server_.Post("/api/debug/stall_message_thread", [this](const httplib::Request& req, httplib::Response& res) { handleDebugStallMessageThread(req, res); });
#endif
```
  handler: parse `ms`, clamp 1..2000, `juce::MessageManager::callAsync([ms] { juce::Thread::sleep(ms); });`,
  respond `{"ok":true,"ms":N}`. The define is target-wide (`CMakeLists.txt:437-438`; ApiServer.cpp is in the
  target, `:374-375`) -- VERIFY the route 404s in a `-DAUDIODNA_BUILD_TEST_SERVER=OFF` configure.
- OPTIONAL (P2, for the burst class live): `AnalysisThread` gets `std::atomic<int> debugStallMs_{0}` checked
  once per hop after `pullHop` (`AnalysisThread.cpp:86`): `if (int ms = debugStallMs_.exchange(0)) sleep(ms);`
  under `#if AUDIODNA_TEST_SERVER`, exposed as `/api/debug/stall_analysis_thread`. One relaxed load per hop;
  no allocation. Skip if the lane wants the analysis loop untouched -- the ctest burst case covers the logic.

### 3.7 `src/test/TestServer.cpp`
- `:487` (after `totalBarCount`): `snap->totalBeatCount = static_cast<uint32_t>(get("totalBeatCount"));   // absent -> 0`.

### 3.8 Tests (section 5) and docs (section 7): `tests/test_take.cpp`, `tests/test_routine_engine.cpp`,
`tests/test_bpm_stabilization.cpp`; `docs/claude/analysis.md` (tempo table `:38-39`), `docs/claude/architecture.md`
(FeatureSnapshot table, next to `totalBarCount`), `docs/claude/recording.md:39-41`, `docs/claude/pitfalls.md`
(new **42** -- 40 and 41 exist there but are not in CLAUDE.md's index; add 42's index line, report the gap),
`CLAUDE.md` pitfalls index, `.harmony/APP-INVENTORY.md:160` (/api/bpm fields) + a row for the debug route,
`.harmony/probe-beatclock.sh` (new), `.harmony/probe-routines.sh` (row 7s).

--------------------------------------------------------------------------------------------------------
## 4. Must not change (and how the plan keeps it)

- **Glide / Ease / Jump semantics** (`RoutineEngine.cpp:255-394`, `startNow`, loop return `:576-607`, `resyncPending`):
  untouched. Only the VALUE of `clock_.now().beat` changes, and only across a stall/burst, where today's is wrong.
- **Stop = routines only** (`stop`/`stopAll`/`stopOnLayer`): untouched.
- **Take recording stamps + tempo maps**: format unchanged (`TempoMap::toVar/fromVar`, `TempoMap.cpp:110-125`);
  anchor kinds unchanged (`start/unmetered/lock/reset/bpm/periodic`); steady-state values identical -- pinned by
  the existing 40-minute test (`tests/test_take.cpp:585-624`: within 0.5 beat, `beatAt` within 0.05) and the
  mid-beat seed test (`:654-678`). Values differ ONLY across a stall/burst, where today a take gets a frozen beat
  and a bogus `"reset"` anchor. A take recorded before the fix loads unchanged; `Program`/`Player` read stamps
  unchanged. No `Take`/`Lane`/`Program`/`Player` file is touched.
- **Tracker outputs** `bpm`, `beatPhase`, `trackerState`, `beatInBar`, `barPhase`, `downbeatDetected`, `barCount`,
  `totalBarCount`, `resyncBarOrigin`: unchanged (additive field only -- D12 "ADD, never REDEFINE").
- **Rulings**: a typed/REST/OSC tempo never moves the beat and Link never realigns (`followExternalTempo`
  untouched; the count is untouched by a tempo value -- test); Tap and Resync realign (their realign now also
  decides the count, by the same 0.5 rule today's reader applies). NOTE: the dispatch cites these rulings to
  `BORIS_DECISIONS.md` "Playback Behaviour"; that section (`:320-371`) does not contain them -- they are VERIFIED
  in `BPMTracker.h:139-156` and `MainComponent.cpp:5197-5202` ("s-rta-0926b plan3 A / LINK-RAMP ruling b").
- **Autopilot / Renderer / TopBar wrap readers** (`src/model/Autopilot.cpp:44`, `src/render/Renderer.cpp:407`,
  `src/MainComponent.cpp:4134`): NOT touched -- render/UI-thread beat-crossing consumers, not this defect; a
  natural follow-up is to diff `totalBeatCount` there too (named in the pitfall).
- **Sacred rules**: no audio-callback change; the analysis thread adds one integer increment per hop (no
  allocation, no lock); the render thread is untouched; FeatureSnapshot first (3.1 lands before any reader).

--------------------------------------------------------------------------------------------------------
## 5. Tests and probe rows -- RED on today's code

Existing rigs synthesise snapshots with `bpm`/`beatPhase` only (`tests/test_take.cpp:35-42`,
`tests/test_routine_engine.cpp:206-218`, `:871-878`). After 3.4 lands, a rig that wraps the phase without
advancing the count would read every wrap as a realign; so the GREEN commit updates these rigs to publish
what the real writer publishes (`totalBeatCount = floor(continuousBeat)`; an explicit count argument where phases
are scripted by hand, e.g. `test_take.cpp:269-283`). `test_recorder_host.cpp` / `test_recorder_double_touch.cpp`
hold the phase constant -- no change needed.

### 5.1 `tests/test_bpm_stabilization.cpp` (links BPMTracker + aubio, `tests/CMakeLists.txt:320-338`) -- `[bpm][beatcount]`
RED = compile failure until 3.1/3.2 exist (additive field; written first, then made green in the same commit).
- manual 120 BPM (`setManualMode(true); setManualBPM(120)`), 3060 hops of `processRawBPM(0,0,false)` +
  `feedDownbeatFeatures(0,0,0)` per hop (the manual downbeat test's own numbers, `test_downbeat_detector.cpp:186-217`):
  `totalBeatCount() == 65` (46.875 hops/beat).
- Tap from the second half: run hops until `beatPhase()` in [0.6, 0.7], `setManualBPM(120)`, one hop -> count +1,
  phase == one hop's increment (0.0213 +- 1e-4). Tap from the first half ([0.2, 0.3]) -> count unchanged.
- `requestResync()` from each half: same two outcomes (applies at the end of `feedDownbeatFeatures`).
- Ruling pin: at phase ~0.63, `followExternalTempo(140)`, one hop -> count unchanged, phase == 0.63 + the 140-BPM step.
- Lock lost / regained (auto mode via `processRawBPM(0,...)` after a lock): count holds while unlocked.

### 5.2 `tests/test_take.cpp` -- `[recorderclock][stall]` (RED against the old reader, commit 3 of section 7)
Helper: a `SyntheticTracker { double beats; snap(bpm) -> totalBeatCount = floor(beats), beatPhase = frac }`.
- "a tick gap of 1.1 beats loses nothing": ticks at 1/120 s, 120 BPM, to t=1.0; one tick at t=1.55; then to 2.0.
  REQUIRE `beat == Approx(2*t).margin(1e-6)` after the gap. Today: 1.0 short.
- "a 0.7-beat gap that begins at phase 0.5 loses nothing": today frozen (reset path), 0.7 short.
- "a 0.6-beat publication burst between two 1/120 s ticks adds 0.6": today read as a reset, 0.6 short.
- "realign semantics pinned": count+1/phase 0 from 0.7 -> +0.3; count same/phase 0 from 0.3 -> +0, one `"reset"` anchor.
  (GREEN before and after -- pins that the writer-side rule reproduces the reader-side one.)
- Existing `[recorderclock]` tests keep passing with counted rigs (the 40-minute drift test must stay within 0.05/0.5).

### 5.3 `tests/test_routine_engine.cpp` -- `[routine][engine][stall]` (RED against the old reader)
- `Rig::snap()` adds `s.totalBeatCount = static_cast<uint32_t>(std::floor(beat));`.
- "position exact across a 1.1-beat gap; the point inside it fires once": `test7Routine(Off)`; `runTo(4.0)`;
  `fire(0)` (starts now, `startBeat` 4.0); `runTo(4.5)`; `beat = 5.6; tick();` -> `slot(0).position == Approx(1.6)`,
  `fd.count(Ev::Fire, clipKey) == 1` (the point at routine beat 1.0). Today: position 0.6, not fired.
- "a Beat-quantised start survives a gap that swallows the edge": routine quantize Beat, fire at 4.3 (pending),
  `beat = 4.5; tick(); beat = 5.2; tick();` -> `state == "running"`. Today: no edge seen (0.2 !< 0.0), still pending.
- Bar-edge behaviour unchanged: existing tests 7-12 pass as before.

### 5.4 Live -- `.harmony/probe-beatclock.sh` (new; production mode, port 7070, `set_bpm 120`; probe-routines idioms:
live-lock gate, `adna_pids`, `open -g`, `rows`/`ok`/`no`; no take needed; ~40 s)
- b1 `/api/bpm.totalBeatCount` advances 6 +- 1 over 3.0 s (RED-ish today: field absent -> `no ... absent`).
- b2 **550 ms message-thread stall**: read `clockBeat` (`/api/routine/status`, published with no routine running --
  the clock ticks from app start) and wall; `POST /api/debug/stall_message_thread {"ms":550}`; sleep 0.8; read again;
  PASS iff `|dBeat - 2*dWall| <= 0.10`. Today: dBeat is ~1.0 short -> FAIL (deterministic: g in [1,1.5) always loses 1).
- b3 **350 ms stall started in the middle of a beat**: poll `/api/bpm` at 100 Hz until `beatPhase` in [0.30, 0.55],
  then stall 350 ms; same check. Today: 0.7 short -> FAIL (deterministic: the gap contains the wrap and reads as a resync).
- b4 (only with the optional analysis hook) 300 ms analysis stall started at `beatPhase >= 0.45`: today 0.6 short.
- b5 informational (not pass/fail): 20 s of `analysisSeconds` vs wall, prints max step -- a device pause shows as a step.
- b0 guard: if the debug route 404s, print `SKIP: no TEST-ONLY hook in this binary` and exit 0 for b2-b4.
- RED evidence: run against main's CURRENT binary rebuilt with ONLY commit 1 (hook + probe) -> b2/b3 FAIL by the
  amounts above; after commits 2-4 -> PASS x3. Record both in the lane report.

### 5.5 Live -- `.harmony/probe-routines.sh` row **7s "grid through a stall"** (after row 7, `:727`)
Fire Probe Routine again (once, no loop), `T1s = wait 0 running`, at T1s+1.0 s `POST stall 550`, `sample 8.9`
into `grid-stall.json`, `rows python3 "$RT" grid "$T1s" "$OUT/grid-stall.json"` (reuses the `grid` analysis
verbatim, `:335-359`). Today: marks 2-4 arrive +0.5 s late -> 3 FAILs (the loadpost1 signature, quiet machine).
After: 98 + 7 rows PASS; run x3 quiet.

--------------------------------------------------------------------------------------------------------
## 6. Risks

1. **The 0.5 decision moves from reader to writer.** For a Tap/Resync that lands within ~one tick (8 ms) of
   the half-beat point, today's reader and the new writer can decide differently (complete vs restart) -- a
   one-beat difference in where a RUNNING routine's continuity lands, in a ~1-2 % window per realign. Accepted:
   the writer has the exact phase; the reader was guessing from a sample. Named in the pitfall.
2. **Writers that do not publish the count.** Test-mode injection (`TestServer::handleInjectFeatures`,
   `ApiServer::handleInjectFeatures`) building a moving `beatPhase` with a constant `totalBeatCount` would
   freeze the routine/take clock (every wrap reads as a realign). Both handlers accept the field (3.6/3.7);
   VERIFY by grep that no `tests/visual/*` sweep runs routines while driving `beatPhase` (ASSUMED none does).
3. **Long stalls now surface a loop quirk.** `RoutineEngine.cpp:576-579` folds ONE cycle per tick; after a
   stall longer than a routine's length (>= 8 s for the 16-beat probe routine at 120 BPM) a looping routine would
   spin one catch-up cycle per tick (preamble re-fired each). Today's lossy clock hid it. Realistic stalls
   (0.05-1 s) do not reach it for routines >= 2 beats. Follow-up (not here): fold `k = floor(pos/length)` cycles
   in one tick, fire the restore once for the landing cycle.
4. **`"reset"` anchor density in AUTO mode is unchanged but now understood:** a late confident detection
   (phase 0.03 -> 0) is a small backward step today AND after -> a `"reset"` anchor each time. Not this defect;
   a dead-band would change tempo-map content -- separate decision if the lane sees anchor spam in takes.
5. **Class 2 remains by design** (section 2d): everything beat-driven pauses with the audio clock; after this
   fix routines are consistent with it. If Boris wants the grid to keep wall time through a device gap, that
   is the follow-up slice with its own RED; `analysisSeconds` on `/api/bpm` makes the next occurrence attributable.
6. **Layout.** 4 bytes at offset 324 keep `sizeof == 384` (static_asserts at `FeatureSnapshot.h:158-169` and
   `FeatureBus.h:129-133` fail the build otherwise). No `-Wpadded` concern.
7. **Stall hook hygiene.** It sleeps the message thread on purpose; guarded by the build flag AND a 2 s clamp;
   the probe's b0 guard skips cleanly when absent. Pitfall 31 (bodyless POST) does not apply -- it carries a body.
8. **Steady-state regression risk in the reader rewrite** is bounded by the existing long/mid-beat/monotonic
   tests plus 5.2's semantics pin; run `ctest` fully (667 + new) under the sanitizer config the tests already use.

Verify before/while building: (a) `sizeof(FeatureSnapshot) == 384` still compiles; (b) `grep -n new\|malloc\|mutex
src/analysis/BPMTracker.cpp` unchanged; (c) the debug route 404s in a TEST_SERVER=OFF configure; (d) after
commit 4, `clockBeat` deltas on `/api/routine/status` remain hop-quantised (0.0213 multiples at 120 BPM) --
proof the reader still follows the tracker and did not start integrating wall time; (e) probe-routines x3 quiet.

--------------------------------------------------------------------------------------------------------
## 7. Commit sequence (lane branch; each step's state is checkable)

1. `probe(beatclock): TEST-ONLY /api/debug/stall_message_thread (+optional analysis stall); /api/bpm analysisSeconds;
   .harmony/probe-beatclock.sh; probe-routines row 7s` -- ApiServer.cpp/.h (`#if AUDIODNA_TEST_SERVER`), optional
   AnalysisThread hook, probes, APP-INVENTORY rows. Build; run probe-beatclock and probe-routines: **RED recorded**
   (b2 ~-1.0 beat, b3 ~-0.7, 7s marks +0.5 s late). This is the live RED, on the pre-fix reader.
2. `feat(analysis): FeatureSnapshot::totalBeatCount -- BPMTracker counts wraps and second-half realigns; published every hop;
   /api/bpm + inject paths` -- 3.1, 3.2, 3.3, 3.6 (bpm/inject), 3.7, 5.1 tests (compile-RED then GREEN in this
   commit), docs analysis.md/architecture.md. ctest 100 %.
3. `test(recording): RED -- RecorderClock and RoutineEngine lose beats across a tick gap or a publication burst` --
   5.2 and 5.3 added; ctest shows exactly these new cases failing (named in the commit message).
4. `fix(recording): RecorderClock integrates totalBeatCount + beatPhase; RoutineEngine Beat edge from the count --
   no beat lost across a stall or burst` -- 3.4, 3.5, the rig updates (5.x), RecorderClock.h comment, recording.md,
   pitfalls.md #42 + CLAUDE.md index, notebook line. ctest 100 % (667 + new); probe-beatclock b2/b3 PASS x3;
   probe-routines 98 + 7 rows PASS x3. Lane report cites the RED run of step 1 and the GREEN runs here.
5. (separate slice, needs Boris) `tracker: keep wall time through an audio-device gap in the predicted regime` -- section 2d.

Done looks like: a 550 ms deterministic message-thread stall moves `clockBeat` by exactly 2 x elapsed seconds
at 120 BPM (+-0.1) on the main app; a routine fired before the stall lands its later moves within +-0.35 s of
the recorded grid; every existing take/clock/engine test still passes; `/api/bpm` shows `totalBeatCount` running
at 2/s and `analysisSeconds` tracking wall on a quiet machine.

## COMPACT
- Root cause 1 (VERIFIED): `RecorderClock.cpp:57-75` integrates a wrapped phase; a gap >= 0.5 beat with a wrap is
  read as a resync (frozen) and every whole beat inside a gap is lost -> loadpost1 lost exactly 1 beat.
- Root cause 2 (INFERRED): reppre1 = the audio clock paused ~0.15 s (5 of 20 hops arrived, no catch-up); the
  tracker's grid moved with it; not a routine-clock defect under "one grid"; tracker-level ruling needed.
- Fix: `FeatureSnapshot::totalBeatCount` (wraps + second-half realigns, written by BPMTracker), `RecorderClock`
  integrates count+phase, `RoutineEngine` Beat edge = count change; TEST-ONLY stall hook + probe rows for RED.
- Files: FeatureSnapshot.h, BPMTracker.h/.cpp, AnalysisThread.cpp, RecorderClock.h/.cpp, RoutineEngine.h/.cpp,
  ApiServer.cpp/.h, TestServer.cpp, 3 test files, 2 probes, docs/pitfall 42, inventory. Format of takes unchanged.

STATUS: COMPLETE -- plan draft written; no app code changed; open decision for Boris: Class 2 (grid through an audio-device gap).
