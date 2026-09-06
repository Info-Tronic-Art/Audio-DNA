# WORK PACKET — s168 Lane B: oscillator phase must run off a MONOTONIC beat counter

PROJECT: ~/projects/RealTimeAudio (Audio-DNA, C++20/JUCE)

## THE PROBLEM (verified, and the file's own comment already asks the question)
`src/signal/OscillatorSignal.h` derives its cycle phase from `barCount`, which is
"bars since the last phrase reset". `BPMTracker::updatePhrase` resets `barCount` on structural
transitions (entering a drop, etc.). Consequence: a REAL drop mid-track yanks an 8-beat
oscillator shape BACKWARDS in the middle of a gesture — the visual jumps. The in-file comment at
`OscillatorSignal.h` lines ~27-41 records this as a known S166-L5a trade-off and states that
deriving from `phrasePhase` instead was considered and REJECTED. Read that whole comment first.

Session s167 already fixed the SILENCE half of this class (counters advance from the predicted
beat when audio is silent, `e437872`/`ab7ad06`/`1e79092`). This packet fixes the STRUCTURAL half.

## TASK
Give tempo-locked oscillators a beat timebase that structural events NEVER touch.
1. Introduce a MONOTONIC musical-time counter (a beat/bar count that only ever increases while
   the transport runs and is never rewound by a phrase/structural reset). Put it where the
   existing beat state lives; do NOT invent a parallel clock in the oscillator.
2. Point `OscillatorSignal`'s phase computation at it, replacing the `barCount` derivation.
3. Keep the existing structural `barCount` intact for whatever else legitimately consumes it —
   this is an ADDITION plus a re-point, NOT a redefinition of `barCount`. Grep for every consumer
   of `barCount` before you change anything and list them in your report.
4. **Add a per-oscillator switch `resetPhaseOnStructural` (or the naming already used in this
   file), DEFAULT FALSE** — i.e. the new flow-through behaviour is the default, and the old
   jump-on-drop behaviour remains reachable. Boris has NOT ruled on which he prefers musically;
   the mechanism must support both so his answer is a setting, not a rebuild. Serialize it if
   the oscillator is serialized; if that would break an existing format version, DO NOT change
   the format — report it instead and leave the switch runtime-only.

## HARD CONSTRAINTS
- **DO NOT touch `tests/CMakeLists.txt` or the top-level `CMakeLists.txt`.** Another lane owns
  them this session. Add your test cases to the EXISTING, already-registered
  `tests/test_oscillator_bar_fold.cpp`.
- **DO NOT touch `src/recording/*`, `src/model/ControlPath.h`, `src/connect/*`, or
  `src/MainComponent.cpp`.** Another lane owns those. If your change genuinely requires a
  `MainComponent.cpp` edit, STOP and report BLOCKED with the exact edit needed — do not make it.
- Do NOT introduce a new cross-thread dependency into `FeatureBus`. It is the one component
  hardened to zero TSan suppressions; adding a race there is an automatic reject.
- Do NOT launch the app. Headless only.
- Do NOT commit. Do NOT push. Report and stop.

## SUCCESS CRITERIA
- Build exits 0.
- New assertions in `tests/test_oscillator_bar_fold.cpp` that FAIL against the old behaviour and
  PASS against the new one. At minimum: drive a scripted beat sequence that includes a structural
  phrase reset mid-cycle and assert the oscillator's phase advances MONOTONICALLY across it
  (no backwards jump), while a second case with the switch enabled DOES reset.
- `cd build && ctest` — all pass, count strictly greater than 285, no regression.
  (285/285 is this session's MEASURED baseline on a build that exited 0. Re-run it yourself.)

## REPO GOTCHAS (each cost a prior session real time)
- Line numbers in this repo DRIFT constantly. Re-grep by anchor text; never trust a cited number,
  including the ~27-41 above.
- A negative grep is only as strong as its pattern — a wrong pattern once produced a false
  "no such thing exists anywhere" claim in this repo. Grep for the SYMBOL.
- Confirm you are in `~/projects/RealTimeAudio`, NOT `~/projects/RealTimeAudio copy` (stale, Jul 11).

## REPORT — TO DISK, MANDATORY
Write your full report to `.harmony/.reports/s168-lane-b-oscillator-beat.md` BEFORE returning,
then also return it. Fields: WHAT / WHERE (files+symbols) / EVERY CONSUMER OF `barCount` FOUND /
HOW VERIFIED (exact commands + before/after ctest counts) / WHAT IS NOT VERIFIED / ASSUMPTIONS /
RISKS LEFT OPEN / STATUS (DONE | DONE_WITH_CONCERNS | BLOCKED).
