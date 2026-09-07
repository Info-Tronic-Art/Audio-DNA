# Reviewer Verdict — s168-review-lane-b
STATUS: DONE
VERDICT: APPROVE

REPO: /Users/boriskarpman/projects/RealTimeAudio (confirmed real repo via `git rev-parse --show-toplevel`, not the stale "copy").

FILES REVIEWED (git diff, working tree vs HEAD):
- src/signal/OscillatorSignal.h
- src/analysis/FeatureSnapshot.h
- src/analysis/BPMTracker.h / .cpp
- src/analysis/AnalysisThread.cpp
- tests/test_oscillator_bar_fold.cpp

## 1. MONOTONICITY (MEASURED)
Grepped every write to `totalBarCount_`/`totalBarCount` (patterns: `totalBarCount_`, `totalBarCount(`, `totalBarCount\b`) across BPMTracker.h/.cpp and AnalysisThread.cpp. Exactly one increment site: `++totalBarCount_;` inside `updatePhrase()`'s `newBar` branch, immediately after `++barCount_;`. Every zeroing site for `barCount_` (no-lock early return in `updatePhrase()`, the structural-reset branch in `updatePhrase()`, and `resetPhrase()`) leaves `totalBarCount_` untouched — confirmed by reading each of the three sites directly, not just the added comments. Also checked `advancePredictedBeat()` and `resetBeatPhase()` (P23/P24 predicted-beat paths): neither touches `barCount_` or `totalBarCount_`. No constructor/`prepareToPlay`/`setSampleRate`/device-change reset path exists for either counter (grepped `barCount_|totalBarCount_|::reset|prepareToPlay|setSampleRate|releaseResources` in BPMTracker.cpp/.h — nothing else). Genuinely monotonic for the life of one BPMTracker instance.

## 2. barCount PRESERVED? (MEASURED, with a real gap)
Enumerated every non-`totalBarCount` consumer of `barCount`/`barCount_` repo-wide (grep `\bbarCount\b` across src/+tests/, then hand-filtered). All existing consumers (MappingEngine's `BarCount` mapping source, TopBar UI display, Autopilot's modulo-N pattern-trigger snap, Layer::processPendingTrigger, ApiServer/TestServer JSON glue) are unchanged and still correct against `barCount_`'s original "bars since last phrase reset" semantics — none silently changed behavior.
**Gap, not a regression in this diff**: two other consumers run the *exact same* pre-fix fold formula (`beatPhase + beatInBar + 4*barCount`) that OscillatorSignal just moved off of, and were NOT migrated or given the opt-in:
  - `src/signal/EnvelopeSignal.h::getValue` (line with `4.0f * static_cast<float>(snapshot.barCount)`) — its own comment literally says "same fold-across-bars fix... as OscillatorSignal" but it still reads `barCount` unconditionally, so an EnvelopeSignal with `beatDuration_ > 4` still gets yanked backward on a real drop.
  - `ConnectionShaper::beatsNow()` (src/connect/ConnectionShaper.cpp), called from `ConnectionEngine.cpp:~95` for **every enabled connection with an LFO/shape source** — a much larger blast radius than one Oscillator instance (macro/mapping system-wide).
  Both should arguably have gotten the same `totalBarCount`-backed treatment (or at least a tracked followup). Worth a S168-followup ticket; does not block this diff since the task was explicitly scoped to oscillators and these files are outside the reviewed set.

## 3. OVERFLOW + PRECISION (MEASURED/INFERRED)
`totalBarCount_` is `uint32_t`: wraps at ~4.29B bars, i.e. centuries at any real tempo — not reachable. Real limit is float mantissa precision in `OscillatorSignal::getValue`'s `static_cast<float>(snapshot.totalBarCount)`: exact integers only up to 2^24 (16,777,216). At 90–180 BPM (bar = 1.33–2.67s) that's ~260–520 days of one BPMTracker instance's continuous uptime (never destroyed/reconstructed) before consecutive bars start rounding to the same float — a stutter (missed phase advance for one bar), not a backward jump, since float rounding of a monotonically-increasing integer sequence is itself monotonically non-decreasing. Real for a permanent installation, irrelevant for a festival set (hours). Pre-existing `barCount_` (uint16_t) wraps at 65536 bars (~1–1.5 days) but that's unchanged legacy behavior, out of scope here.

## 4. SEQLOCK (MEASURED — compiled isolated repro, not read-by-eye)
Copied FeatureSnapshot.h at HEAD and at the working-tree version into an isolated scratch dir and compiled tiny `sizeof`/`offsetof` probes (g++ -std=c++20, no project build/cmake touched). Confirmed: `sizeof(FeatureSnapshot)` == 320 in BOTH versions (builder's claim holds). Root cause: `alignas(64)` forces the struct's size to a multiple of 64; the OLD layout's last real member (`reeseBass`) ended at offset 304, leaving 16 bytes of compiler-inserted TAIL padding to reach 320. The new 4-byte `totalBarCount` (inserted between `barCount` and `phrasePhase`, offset 112, itself 4-byte aligned with zero padding needed there) shifts every later member by +4 and consumes 4 of those 16 padding bytes — new tail slack is 12 bytes (reeseBass now ends at 308). So the size held by consuming real (if incidental) alignment slack, not blind luck, and there's room for roughly one more 4-byte / two 2-byte fields before the struct must grow to 384. `totalBarCount` lands entirely inside one 4-byte atomic word (no straddling), and `FeatureBus` transports the whole 320-byte struct as opaque words with seq-checked whole-struct copies (not per-field atomicity), so no new tearing/aliasing class is introduced — same discipline as every other field. `std::is_trivially_copyable_v` holds (verified in the same compile).

## 5. TEST STRENGTH (MEASURED — reproduced by hand, not eyeballed)
The new `TEST_CASE("S168: default oscillator phase is monotonic...")` is a genuine regression test, not a tautology. Manually recomputed `getValue()`'s arithmetic for its 4 scripted snapshots (beatDuration_=32): default-oscillator sequence is strictly non-decreasing (0, 0.625, 0.703, 0.75) using `totalBarCount`. Recomputing the SAME snapshots against `barCount` instead (i.e., simulating pre-S168 code) gives (0, 0.625, **0.078**, 0.125) — the comment's claimed "~0.078" is exactly right, and `REQUIRE(d2 >= d1 - 0.0001f)` would fail against it. The legacy-opt-in assertions (`l1 - l2 > 0.3f`, `l3 > l2`) also check out arithmetically. The 4 pre-existing tests' `setResetPhaseOnStructural(true)` calls are legitimate adaptation (they explicitly test the old barCount-driven trade-off, which still exists and is still reachable) — not weakened, not a redefinition. One real coverage gap: the new DEFAULT path (false) is only exercised by the one new S168 test case; the older 4 tests no longer cover it at all (opted out via `true`). Acceptable given the new test is well-targeted, but worth naming.

## 6. THREAD SAFETY OF PUBLISH (MEASURED)
`AnalysisThread.cpp`: `snap->totalBarCount = bpmTracker_->totalBarCount(); // S168` sits between `snap->barCount = ...` and `snap->phrasePhase = ...`, writing into the same writer-private `staging_` snapshot (`FeatureBus::Writer::acquireWrite()`) that every other field in that ~200-line pipeline fills before a single `publishWrite()` call. Same discipline as its neighbor, no bypass.

## REQUIRED FIXES
None in the 6 reviewed files.

## NICE-TO-HAVE
1. File a followup to migrate `EnvelopeSignal::getValue` and `ConnectionShaper::beatsNow`/`ConnectionEngine.cpp` off unconditional `barCount` — same bug class, much larger blast radius (all LFO-sourced connections) than OscillatorSignal.
2. Consider whether the 4 adapted legacy tests should get a `false`-mode sibling assertion each, so the new default isn't covered by only one test case.
3. `resetPhaseOnStructural_` is a runtime-only, non-serialized flag (verified: no OscillatorSignal field is persisted anywhere in ApiServer/TestServer) — fine today, but flag for whoever eventually adds Oscillator persistence.

## WHAT I COULD NOT CHECK WITHOUT BUILDING
- Runtime TSan-clean behavior of the actual seqlock under real concurrent readers/writer (static structural analysis only — did not build/run tests per instructions).
- Actual measured wall-clock bar rate at a real device sample rate / hop size in production (used illustrative BPM-based estimates for the overflow/precision section).
- Whether any UI/preset path outside grep's reach (e.g. reflection-based serialization) persists OscillatorSignal state; grepped for `toVar|fromJson|toJson|setProperty|DynamicObject` co-occurring with `OscillatorSignal` and found none.
