# Critic: plan-downbeat.md (s-rta-0925, roadmap item 4)

VERDICT: PASS (no blocking defects found)

## Method
Independently re-derived every `path:line` claim in the plan against HEAD (e501b00) source:
`BPMTracker.cpp/.h` (all 4 `downbeatDetected_` assignment sites, `updatePhrase`/`totalBarCount_`
edge detection, `advancePredictedBeat`, manual-mode branch of `runPipeline`), `AnalysisThread.cpp`
publish site, `ApiServer.cpp` (`handleGetBpm`/`handleSetBpm`), `TopBar.cpp`/`AudioReadoutPanel.cpp`
(both `downbeatDetected` reads), `TestServer.cpp` inject site, `FeatureSnapshot.h` layout/comment,
`OnsetPulse.h`, `OscillatorSignal.h`/`EnvelopeSignal.h`/`ConnectionShaper.cpp` (`totalBarCount`
consumers), `tests/test_downbeat_detector.cpp` + `tests/test_bpm_stabilization.cpp` (existing
helpers/precedent), `tests/CMakeLists.txt` (include dirs), `CLAUDE.md`, `.harmony/APP-INVENTORY.md`,
and the Autopilot/Layer adjacent-finding lines in RISKS. Also hand-traced the new Test A/B math
(hop cadence, beatCounter_ alignment at lock time, manual-mode wrap count, CadenceReader/OnsetPulse
window arithmetic) rather than trusting the plan's arithmetic on assertion.

## Findings

### Core claim: downbeatDetected is a LEVEL, not a pulse — VERIFIED, no blocking issue
All 4 assignment sites (`BPMTracker.cpp:241,337,355,436`) are beat-event-only; no per-hop clear
exists anywhere in the file. `updatePhrase()`'s own `prevDownbeatDetected_` rising-edge detector
(`:471-472`) only makes sense if the flag is a level. `totalBarCount_` (`BPMTracker.h:112-115`,
`:234`) is exactly the monotonic counter the task asked to invent — confirmed as already consumed
by `OscillatorSignal.h:54`, `EnvelopeSignal.h:53`, `ConnectionShaper.cpp:17-21`. The plan's
refutation of the task's premise is correct and well-evidenced, not merely asserted.

### Consumer inventory — VERIFIED complete
Independent `grep -rln downbeatDetected src/ tests/` and `grep -rin downbeat` across
connect/routing/model/midi/osc/recording returned exactly the same file set the plan's table
claims (TopBar.cpp, AudioReadoutPanel.cpp, AnalysisThread.cpp, BPMTracker.cpp/.h,
TestServer.cpp, test_downbeat_detector.cpp, TESTING.md) plus two unrelated comment/enum hits in
Autopilot.h/Composition.h that don't reference the field. No missed consumer, no missed shader
uniform (0 hits in EmbeddedShaders.h confirmed), no missed signal/routing/autopilot/recorder
reader. The two `displaySnap_.downbeatDetected` copies (TopBar.cpp:232, AudioReadoutPanel.cpp:43)
are independently confirmed write-only (grepped for all reads of `displaySnap_.downbeatDetected`
in both files: zero).

### Test math — hand-traced, checks out
- Test A: `feedBeatWithFeatures`/`feedNonBeatHops` helpers match verbatim (existing file,
  unmodified); `kHopsPerBeat=47` matches `lround(48000*60/(120*512))=46.875→47`, same formula as
  the existing precedent in `test_bpm_stabilization.cpp:42-43`.
- Traced `beatCounter_` alignment through the 8-bar lock loop by hand: `totalBeatsScored_`
  increments in lockstep with the loop's `beat` index every call (scoreBeat always fires since
  `beatDetected_=true` and `!predictedBeatRegime_` holds for every `feedBeatWithFeatures` call),
  so once `analyzeDownbeatPosition()` locks `lockedDownbeatPos_=0`, `beatCounter_` re-derives to
  exactly the loop's beat index and tracks it forever after. This independently confirms the
  plan's `REQUIRE_FALSE(tracker.downbeatDetected())` immediately after the lock loop (which ends
  on `beat==3`) — not just an assumed value, a derivable one.
- Test B: `setManualBPM` sets `phase_=0` and leaves `beatCounter_` at its (freshly-constructed)
  0; increment/wrap math (`512/24000=0.021333`/hop, 46.875 hops/wrap) confirms `kHops=3060`
  gives >=65 wraps with the claimed ~13-hop margin, and downbeats land at wraps 4,8,...,64 (16
  bars) — matches `kBars=16`.
- `CadenceReader`/`publishHop` windowing: no read of a bar edge is ever dropped or double-counted
  at the loop's end (I traced the 0.9s-poller's final-window boundary explicitly, since a
  window that closes exactly at the truth stream's end is the standard place for this class of
  test to silently miss the last edge — it doesn't here, bar 16's edge lands mid-loop, not at the
  boundary).
- `#include "features/OnsetPulse.h"` resolves under the existing `target_include_directories(...
  PRIVATE ${SRC_DIR})` for `test_downbeat_detector` (`tests/CMakeLists.txt:242`) — no CMake change
  needed, matching precedent in `test_integration_pipeline.cpp:16`. `CadenceReader`'s aggregate-init
  with only `periodSec` specified is legal (C++14+ relaxed aggregate rules, default member
  initializers on the rest, no virtuals/private members) under the project's C++20 target.

### REST/thread-safety — no RT-rule violation
`ApiServer::handleGetBpm` runs on the HTTP thread (not audio/analysis/render); the new line reuses
the same `featureBus_.read()` snapshot and the same `DynamicObject` its neighbours already write to
(`ApiServer.cpp:606-612` precedent) — no new lock, no new atomic, no hot-path touch. Confirmed
`/api/set_bpm` → `onSetBpm` → `applyTempoCommand("link", ...)` → `setManualMode(true)` +
`setManualBPM(bpm)` (`MainComponent.cpp:1919-1922` lambda, `:5016-5023` branch — plan cites
`:5016-5022`, off by one line, trivial) is the exact code path the live probe drives; this is a
real, already-wired path, not a probe-invented one.

### Minor citation drift (non-blocking, worth a one-line fix at build time)
- `.harmony/APP-INVENTORY.md:151` — the actual row-10 line is **152**, not 151 (`grep -n "GET |
  /api/bpm"` → `152:| 10 | GET | /api/bpm | ...`). Trivial to find; not a blocking defect.
- `MainComponent.cpp:5016-5022` for the `applyTempoCommand("link", ...)` branch — actual branch
  body is `5016-5023` (closing brace one line later). Cosmetic.
Both are citation-precision nits in an otherwise painstakingly exact plan; neither affects any
Builder deliverable (REST line, comment text, or test code) since those are given as literal diffs,
not derived from these two line numbers.

### One unstated (non-blocking) edge case in the live probe's "coherent" oracle
`downbeatDetected_ == (beatInBar_ == 0)` can transiently be FALSE right after `/api/set_bpm` is
called, if the tracker was previously in the un-locked, real-onset `scoreBeat()` branch
(`BPMTracker.cpp:355`, which forces `downbeatDetected_ = false` even when
`totalBeatsScored_ % 4 == 0` sets `beatInBar_ = 0`) — that stale `beatInBar_` persists until the
first predicted-mode phase wrap re-syncs both fields together via `advancePredictedBeat()`
(`:241`), since `setManualBPM` does not reset `beatCounter_`/`beatInBar_`. In practice the probe's
existing `sleep 1` after `set_bpm` plus one more `sleep 1`+curl round-trip before polling starts
(section 5, steps 3→4) already exceeds one beat period (500 ms @ 120 BPM) in every case, so the
transient window is retired before the 25 Hz poll loop begins — I could not construct a realistic
sequence where it survives into the polled window. Flagging as a documented near-miss for
completeness, not a blocking defect: it doesn't invalidate the oracle as written, and the plan's
own interpretation notes already call out the harder case (aubio phase-reset stretching individual
runs) that dominates any real timing noise.

## What would make this FAIL
None of the above rises to "blocking" — every core claim (level-not-pulse semantics, exhaustive
consumer list, existing-counter reuse, REST/threading discipline, test math) is independently
re-derived from source and holds. The plan is unusually rigorous: every `path:line` citation I
checked was either exact or off by at most one line in a non-load-bearing spot.

STATUS: DONE
