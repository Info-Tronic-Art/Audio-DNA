# WORK PACKET — s168 Lane B2: finish the monotonic-beat sweep, and make it live-verifiable

PROJECT: ~/projects/RealTimeAudio (Audio-DNA, C++20/JUCE)

## WHY THIS EXISTS
Lane B moved `OscillatorSignal` off the resettable `barCount` onto a new monotonic
`totalBarCount`, so a structural drop no longer yanks a tempo-locked shape backwards mid-gesture.
An INDEPENDENT REVIEWER then enumerated every consumer of `barCount` and found the fix was
applied to ONE SITE of a bug that has THREE. **In this repo the defect is repeatedly a SHAPE, not
a SITE** — session s167 found the identical frame-rate coupling in six separate places, two of
them inside code that had been reviewed and gated green earlier the same day. Do not assume the
list below is complete either: re-derive it.

## TASK — four items, in this order

### 1. [THE BIG ONE] `ConnectionShaper::beatsNow()` — `src/connect/ConnectionShaper.cpp`
It runs the same pre-fix fold (`beatPhase + beatInBar + 4*barCount`) and is called from
`ConnectionEngine` for EVERY enabled connection with an LFO/shape source. Blast radius is the
whole macro/mapping/connection path, not one oscillator instance. Move it to `totalBarCount`.

### 2. `EnvelopeSignal::getValue` — `src/signal/EnvelopeSignal.h`
Same fold. **Its own in-file comment claims it already received "the same fold-across-bars fix as
OscillatorSignal" — that comment is FALSE.** It reads `barCount` unconditionally, so an
`EnvelopeSignal` with `beatDuration_ > 4` still jumps backwards on a real drop. Fix the code AND
fix the lying comment. A comment that claims a fix that is not there is worse than no comment.

### 3. Make it LIVE-VERIFIABLE — expose `totalBarCount`
`src/api/ApiServer.cpp` (`handleGetBpm` / `handleInjectFeatures` / the `/api/signals` JSON) and
`src/test/TestServer.cpp`'s snapshot builder do not carry `totalBarCount`. The S166-L5a fix was
proven live through `/api/signals`; the same path must exist for this one, or the fix is
unit-proven only and no behavioural gate can reach it. Add the field wherever `barCount` is
already published. Additive only — do not rename or remove `barCount` anywhere.

### 4. Close the two test gaps the review named
   (a) `tests/test_bpm_stabilization.cpp` — add a BPMTracker-LEVEL case asserting that a
       structural transition into a drop ZEROES `barCount()` while `totalBarCount()` keeps
       increasing across the same event. That file already contains the "structural transition
       does not reset barCount" pattern to mirror. This is the assertion nobody has written yet:
       today the monotonicity claim rests on code reading plus one oscillator-level test.
   (b) `tests/test_oscillator_bar_fold.cpp` — the four pre-existing cases were adapted by opting
       IN to the legacy mode (`setResetPhaseOnStructural(true)`), which is legitimate, but it
       leaves the new DEFAULT path guarded by exactly one test case. Add a default-mode (`false`)
       sibling assertion to each of the four.

## HARD CONSTRAINTS
- **Additive + re-point only. Never redefine `barCount`.** Its existing consumers (MappingEngine's
  `BarCount` source, the TopBar display, Autopilot's modulo-N pattern snap,
  `Layer::processPendingTrigger`, the ApiServer/TestServer JSON glue) legitimately want "bars
  since the last phrase reset" and must keep it. Enumerate them yourself before you touch
  anything and list what you found in your report.
- Follow the pattern Lane B already established for the opt-in: where a caller might legitimately
  want the old jump-on-drop behaviour, expose it as a switch DEFAULTING to the new monotonic
  behaviour, named consistently with `resetPhaseOnStructural` in `OscillatorSignal.h`. Read that
  file first so the two look like one design, not two.
- **Do NOT add a cross-thread dependency to `FeatureBus`** — it is the one component hardened to
  zero TSan suppressions. `FeatureSnapshot` already carries `totalBarCount` (added by Lane B);
  `sizeof(FeatureSnapshot)` is 320 and is held by a `static_assert` in `src/features/FeatureBus.h`.
  There are ~12 bytes of tail slack left. **If you add any field to that struct, prove the size
  with a scratch compile BEFORE editing the real header** — that is how Lane B avoided breaking
  the seqlock. You should not need to add one.
- Do NOT touch `src/recording/*`, `src/model/ControlPath.h`, `src/ui/RecordPanel.*`, or the
  `Take`/`Player`/`PerformanceRecorder` classes — a different lane owns the recorder.
- Do NOT launch the app. Do NOT commit. Do NOT push.

## SUCCESS CRITERIA
- Build exits 0.
- `cd build && ctest` — all pass, count strictly greater than 296, no regression.
  **296/296 is the MEASURED baseline on a clean forced rebuild. Re-run it; never inherit a count.**
- The new BPMTracker-level test FAILS if `++totalBarCount_` is moved inside the structural-reset
  branch (i.e. it actually pins monotonicity, it is not a tautology). Say in your report how you
  convinced yourself of that.
- Every `barCount` consumer you did NOT migrate is listed in the report with one line on why it
  correctly stays on the resettable counter.

## REPO GOTCHAS (each cost a prior session real time)
- Line numbers DRIFT here. Re-grep by anchor text; never trust a cited number.
- A negative grep is only as strong as its pattern. State the pattern; try an alternative spelling
  before believing a "does not exist anywhere" result. A wrong pattern once produced a false
  "no resolution lock exists anywhere" claim in this repo (the symbol was `setLockedResolution`,
  the grep looked for `lockResolution`).
- `.harmony/` is gitignored with force-tracked files; `git add` there warns and still stages,
  which breaks `&&` chains — use `;`.
- Confirm you are in `~/projects/RealTimeAudio`, NOT `~/projects/RealTimeAudio copy` (stale Jul 11).

## REPORT — TO DISK, MANDATORY
Write to `.harmony/.reports/s168-lane-b2-beat-sweep.md` before returning, then also return it.
Fields: WHAT / WHERE (files + symbols) / EVERY `barCount` CONSUMER FOUND, MIGRATED OR NOT, WITH
REASON / HOW VERIFIED (exact commands, before/after ctest counts) / HOW I PROVED THE NEW TEST IS
NOT A TAUTOLOGY / WHAT IS NOT VERIFIED / ASSUMPTIONS / RISKS LEFT OPEN / STATUS.
