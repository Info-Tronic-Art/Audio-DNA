# s168 Lane B — Oscillator monotonic beat report

### STATUS
DONE

### RESULT
OscillatorSignal now folds its phase across a new monotonic `totalBarCount`
by default; the old jump-on-structural-reset behaviour survives as an
explicit per-oscillator opt-in (`resetPhaseOnStructural_ = true`). Build
exits 0; ctest 286/286 (baseline 285/285, my delta +1 test case / +N
assertions inside it). No fenced file touched.

### FACTS
- `src/signal/OscillatorSignal.h:23-57` — `getValue()` now reads
  `snapshot.totalBarCount` when `resetPhaseOnStructural_` is false
  (default), `snapshot.barCount` when true.
- `src/signal/OscillatorSignal.h:99-105,112` — new
  `getResetPhaseOnStructural()`/`setResetPhaseOnStructural(bool)` and
  `resetPhaseOnStructural_ = false` field.
- `src/analysis/FeatureSnapshot.h:49-55` — new `uint32_t totalBarCount = 0`
  field, added directly after `barCount`.
- `src/analysis/BPMTracker.h:111-115,225-232` — new `totalBarCount_` field
  and `totalBarCount()` accessor, alongside `barCount_`/`barCount()`.
- `src/analysis/BPMTracker.cpp:461-517` — `++totalBarCount_` added next to
  `++barCount_` in `updatePhrase()`'s newBar branch; the three places that
  zero `barCount_` (no-lock early return, structural-transition branch,
  `resetPhrase()`) each got a comment but no code change — `totalBarCount_`
  is never written anywhere except that one increment.
- `src/analysis/AnalysisThread.cpp:179` — `snap->totalBarCount =
  bpmTracker_->totalBarCount();` added next to the existing `snap->barCount
  = bpmTracker_->barCount();` line, so the live app actually publishes the
  field (without this the feature would be inert in production).
- `tests/test_oscillator_bar_fold.cpp` — 104 lines added, 0 removed
  (`git diff --numstat`). One new `TEST_CASE` (S168); one
  `osc.setResetPhaseOnStructural(true);` line added to each of the 4
  pre-existing `OscillatorSignal` test cases that manipulate `barCount`
  directly (their original numeric assertions are untouched).
- `cd build && ctest`: 286/286 passed (`ctest` output, `build/` dir),
  baseline was 285/285 per the packet. `./tests/test_oscillator_bar_fold`
  run directly: "All tests passed (243 assertions in 9 test cases)".
- `sizeof(FeatureSnapshot)` unchanged at 320 bytes — proved by a scratch
  compile before editing (existing 12 bytes of `alignas(64)` tail padding
  absorbed the new `uint32_t`), and re-proved by the real build: `test_feature_bus`
  (owns the `static_assert(sizeof(FeatureSnapshot) == 320, ...)` in
  `src/features/FeatureBus.h`) built and its 9 tests (`ctest -N` #30-38) +
  the integration test (#71) all passed.

### METHOD
Read `src/signal/OscillatorSignal.h`'s existing S166-L5a comment in full
first, per the packet's instruction. Grepped every `barCount` occurrence in
the repo (`grep -rn "barCount" --include=*.h --include=*.cpp .`) before
touching anything, to find where "the existing beat state lives" and every
consumer (list below). Verified the FeatureSnapshot size question with a
standalone scratch compile (`clang++ -std=c++20 -Isrc`) before editing the
real header, since `src/features/FeatureBus.h` seqlocks the struct through a
`static_assert(sizeof(FeatureSnapshot) == 320)` and the packet explicitly
forbids adding risk to FeatureBus (zero TSan suppressions). Built with
`cmake --build build --config Release -j$(sysctl -n hw.ncpu)` (full build,
then again targeted at `test_oscillator_bar_fold`), ran
`./tests/test_oscillator_bar_fold` directly, then `cd build && ctest`.

### CONFIDENCE+VERIFY
HIGH on the OscillatorSignal-level contract (new TEST_CASE drives a scripted
beat-reset sequence and asserts monotonic-by-default / jump-when-switched,
both proved green in this run). MEDIUM on BPMTracker's own
`totalBarCount_` bookkeeping in isolation: I did not add a BPMTracker-level
assertion of it specifically (see UNKNOWNS) — confidence rests on code
reading (the increment is the literal same `if (newBar)` statement that
already drives `barCount_`, which existing `test_bpm_stabilization.cpp`
cases do exercise) plus the fact that AnalysisThread/FeatureSnapshot wiring
compiled and the seqlock static_asserts still hold. To re-verify: `cd
/Users/boriskarpman/projects/RealTimeAudio && cmake --build build --config
Release -j$(sysctl -n hw.ncpu) && cd build && ctest`.

### UNKNOWNS-NOT-DONE
- No BPMTracker-level test asserts `totalBarCount()` directly survives a
  structural reset while `barCount()` zeroes (e.g. in
  `tests/test_bpm_stabilization.cpp`, which already has the "structural
  transition into drop does not reset barCount" pattern to mirror). Not
  added because the packet/team-lead message explicitly restricts new test
  cases to `tests/test_oscillator_bar_fold.cpp`. Flagging as a residual gap.
- `src/api/ApiServer.cpp` (`handleGetBpm`, `handleInjectFeatures`) and
  `src/test/TestServer.cpp` (JSON snapshot builder) are NOT wired for
  `totalBarCount` — the S166-L5a fix was previously verified live via
  `/api/signals` (ApiServer.cpp's own comment says so); the same manual
  verification path is not available yet for S168. Not required by the
  packet's success criteria (unit-test only, headless), left out per
  minimal-scope discipline. Easy follow-up if Boris wants live verification.
- `src/mapping/MappingEngine.cpp`'s `MappingSource::BarCount` was not
  extended with a `TotalBarCount` mapping source — not required, not
  requested.
- `EnvelopeSignal.h` (`src/signal/EnvelopeSignal.h:38-44`) has the exact
  same barCount-fold pattern and thus the same backward-jump exposure, but
  the packet named only `OscillatorSignal` as the target — left untouched.
- Did not run the TSan build (`build-tsan`) — the packet's build/test
  commands didn't call for it, and I did not touch `FeatureBus.h` or
  anything cross-thread, so I judge this low-risk, but it is not
  independently re-proven here.

### NUANCE
`resetPhrase()` (called on manual Resync, `src/MainComponent.cpp:595,5860`
— read-only, not edited) also zeroes `barCount_`. I read this as a
"phrase reset" per its own name/comment and left `totalBarCount_` untouched
there too, i.e. even a manual Resync does not rewind the monotonic counter —
this is the strictest, most literal reading of "never rewound by a
phrase/structural reset" in the packet. This was a judgment call (packet
doesn't explicitly mention Resync); flagging it explicitly since it affects
oscillator behaviour across a user-initiated Resync, not just automatic
structural transitions.

Serialization: grepped for any OscillatorSignal/Signal persistence
(ValueTree/JSON/PresetManager) and found none — `SignalRegistry.cpp`
constructs "Mod 1"/"Mod 2" etc. hard-coded in `initDefaults()`; no save/load
path touches Signal fields at all. So "serialize it if the oscillator is
serialized" resolves to: nothing to serialize into currently exists;
`resetPhaseOnStructural_` is runtime-only by necessity, not by a versioning
concession. Noted in the OscillatorSignal.h comment.

### HANDOFF-NEEDS
None for me. For Harmony's gate / the independent reviewer: please confirm
the `resetPhrase()`/Resync interpretation above reads correctly, and decide
whether the ApiServer/TestServer JSON-observability gap and the
BPMTracker-level `totalBarCount()` test gap are worth a follow-up packet.

---

## WHAT
Gave tempo-locked oscillators (`OscillatorSignal` only, per packet) a beat
timebase (`FeatureSnapshot::totalBarCount`, sourced from
`BPMTracker::totalBarCount_`) that structural events never rewind, and made
using it the default. The old barCount-driven jump-on-reset behaviour is
preserved, reachable per-oscillator via `resetPhaseOnStructural_ = true`.

## WHERE (files + symbols)
- `src/analysis/FeatureSnapshot.h` — `+uint32_t totalBarCount`
- `src/analysis/BPMTracker.h` — `+uint32_t totalBarCount_`,
  `+uint32_t totalBarCount() const`
- `src/analysis/BPMTracker.cpp` — `BPMTracker::updatePhrase()` (`+
  ++totalBarCount_` next to `++barCount_`; comments at the 3 reset sites),
  `BPMTracker::resetPhrase()` (comment only, no code change)
- `src/analysis/AnalysisThread.cpp` — `AnalysisThread::run()`, one line:
  `snap->totalBarCount = bpmTracker_->totalBarCount();`
- `src/signal/OscillatorSignal.h` — `OscillatorSignal::getValue()`
  re-pointed; `+resetPhaseOnStructural_`,
  `+getResetPhaseOnStructural()/setResetPhaseOnStructural()`
- `tests/test_oscillator_bar_fold.cpp` — header comment addendum; 1 line
  added to 4 pre-existing `OscillatorSignal` test cases; 1 new `TEST_CASE`
  ("S168: default oscillator phase is monotonic across a structural reset;
  switch=true still jumps")

## EVERY CONSUMER OF barCount FOUND
(`grep -rn "barCount" --include=*.h --include=*.cpp .`, excluding `/build*`)
- `src/analysis/FeatureSnapshot.h:49` — the field itself (untouched, kept
  intact per packet §3)
- `src/analysis/BPMTracker.h`/`.cpp` — owner: `barCount_`, `barCount()`,
  increment in `updatePhrase()`, resets in `updatePhrase()` (no-lock branch
  + structural branch) and `resetPhrase()` — all untouched in behaviour,
  only commented
- `src/analysis/AnalysisThread.cpp:179` — copies `bpmTracker_->barCount()`
  into the published snapshot (untouched, `totalBarCount` copy added next
  to it)
- `src/mapping/MappingEngine.cpp:87` — `MappingSource::BarCount` exposes
  `barCount` as a routable float (untouched, out of scope)
- `src/ui/TopBar.cpp:234,423` — displays "Bar N" (phrase-relative, by
  design; untouched)
- `src/model/Layer.h:278-309` (`processPendingTrigger`) and
  `src/model/Autopilot.cpp:57` — trigger scheduling on `barCount % 2`/`% 4`
  (untouched, out of scope — legitimately wants phrase-relative count)
- `src/api/ApiServer.cpp:552,634-640` — `/api/bpm`, `/api/signals` inject —
  JSON get/set of `barCount` (untouched — see UNKNOWNS)
- `src/test/TestServer.cpp:461` — JSON-driven test snapshot builder
  (untouched — see UNKNOWNS)
- `src/connect/ConnectionShaper.cpp/.h`, `src/connect/ConnectionEngine.cpp`
  — `beatsNow()` folds `barCount` for the connection-routing shaper
  (FENCED, not touched, not read for editing purposes beyond a name grep)
- `src/signal/EnvelopeSignal.h:44` — identical fold pattern, different
  Signal subclass (untouched — packet named only OscillatorSignal)
- `tests/test_bpm_stabilization.cpp`, `tests/test_connection.cpp` — existing
  test coverage of `barCount`/`beatsNow` (untouched, still green)

## HOW VERIFIED (exact commands + before/after ctest counts)
```
cd /Users/boriskarpman/projects/RealTimeAudio
cmake --build build --config Release -j$(sysctl -n hw.ncpu)   # exit 0, full rebuild incl. all test targets
cmake --build build --config Release -j$(sysctl -n hw.ncpu) --target test_oscillator_bar_fold  # exit 0
cd build && ./tests/test_oscillator_bar_fold      # "All tests passed (243 assertions in 9 test cases)"
cd build && ctest                                  # "100% tests passed, 0 tests failed out of 286"
```
Baseline stated in the packet: 285/285. Observed after my change: 286/286
(my delta: +1 `TEST_CASE` registered with ctest; the 4 pre-existing
`OscillatorSignal` cases I touched gained lines but did not gain new
`TEST_CASE`s, so they don't add to the ctest count). I did not see the other
lane's test additions reflected in this count at the time I ran it — 286 is
exactly 285 + my own delta of 1, so lane-a-recorder's tests were not yet
present in the tree/build at this snapshot.

## WHAT IS NOT VERIFIED
See UNKNOWNS-NOT-DONE above (BPMTracker-level direct assertion of
`totalBarCount()`, ApiServer/TestServer JSON wiring, TSan build). Also not
run: the actual app (forbidden by the packet — headless only).

## ASSUMPTIONS
- `resetPhrase()` (manual Resync) also does not rewind `totalBarCount_` —
  see NUANCE above; this was the literal reading of "never rewound by a
  phrase/structural reset," not an explicit packet instruction.
- `totalBarCount` is `uint32_t` (not `uint16_t` like `barCount`) to keep
  wraparound outside any realistic session length (~272 years at 120 BPM
  4/4 vs. `barCount`'s ~36 hours) since "monotonic" implied avoiding a
  practically-reachable overflow rewind.
- The pre-existing 4 `OscillatorSignal` test cases in
  `test_oscillator_bar_fold.cpp` were reasonably re-scoped to explicitly pin
  the legacy (`resetPhaseOnStructural_ = true`) path via one added line each,
  rather than rewriting every `FeatureSnapshot` construction in them to also
  set `.totalBarCount` — this kept their original, reviewed numeric
  assertions byte-for-byte intact while still being accurate about what they
  now test (comment added at each site + a file-header addendum explaining
  the split).

## RISKS LEFT OPEN
- The Resync/`resetPhrase()` judgment call above (NUANCE) — if Boris wants
  Resync to also snap `totalBarCount_` back in line with `barCount_` (i.e.
  only true structural transitions should be exempt, not a manual reset),
  that's a small follow-up in `BPMTracker::resetPhrase()`.
- `EnvelopeSignal.h` carries the identical backward-jump exposure and was
  not fixed (out of scope per packet).
- No live/API-level way to exercise or observe `totalBarCount` from outside
  the process yet (see UNKNOWNS) — the previous S166-L5a bug of this same
  class was originally diagnosed via `/api/signals`; that path isn't
  extended for S168.

### STATUS
DONE

INBOX-RECHECK: 0 addenda folded
