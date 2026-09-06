# WORK PACKET — s168 Step 1: the recorder core model (headless)

PROJECT: ~/projects/RealTimeAudio (Audio-DNA, C++20/JUCE)
DEPARTMENT: engineering
SPEC (authoritative, READ IT): `.harmony/specs/s167-performance-log-and-routines.md`
  — §2 D1 (timebase), D2 (ControlPath), D3 (lanes), D5 (Take vs Program), D6 (parameter moves),
    D7 (curve sharing with Lane 2), D8 (override), §3 (model sketches), §5 Build order row 1.

## TASK
Build the headless recorder core ONLY. Row 1 of the spec's "Build order — NOW":
- `src/model/ControlPath.h`
- `src/recording/{Lane.h, Take.{h,cpp}, TempoMap.{h,cpp}, PerfState.{h,cpp}, Program.{h,cpp},
   Player.{h,cpp}, PerformanceRecorder.{h,cpp}, RecorderClock.{h,cpp}}`
- DELETE `src/recording/SessionRecorder.{h,cpp}` and every reference to it (see BOUNDARIES).
- ctest target `test_take` covering verify items (a)-(g) of spec §5 row 1.

## HARD CONSTRAINTS (violating any of these fails the review)
1. **DO NOT create a new AutomationCurve.** It ALREADY EXISTS at `src/connect/AutomationCurve.h`
   (landed in Lane 2, commit d8562d8). REUSE it. If it lacks something the recorder needs,
   ADD to it — never fork it, never freeze `Envelope::points` back to a bare pair-vector (spec R8).
2. **`manualWrite` is owned by the CONNECTION lane, not you.** Ruling R8. Do NOT define,
   move, or re-implement `manualWrite`. Step 3 (a LATER packet) hooks it. Stay headless.
3. **No `MainComponent.cpp` wiring in this packet.** Model + tests only. Step 3 does the wiring.
4. **No pointer hand-out** (spec R1): no `advance()` in `src/recording/` may return
   `const Event*` / `const DiscretePoint*`. Return values, not pointers into a mutating vector.
5. **No clip-id keying** (spec R4): `ControlPath` addresses positionally + by name, never `clip.id`.
6. **Message-thread only** (spec R7): `PerformanceRecorder` asserts the thread. Nothing on the
   GL or audio thread in this packet.
7. **Only CHANGES are recorded** (Boris ruling 23a): a knob moved and parked records the move
   then nothing. Lanes are sparse; silence means "unchanged", not "missing".
8. **Coalescing bounded** (verify item e): 100 `set` events in 50 ms must produce a bounded
   number of points, with begin/end exact.

## SUCCESS CRITERIA
- `cmake --build build --config Release` exits 0.
- `cd build && ctest` — ALL tests pass, count STRICTLY GREATER than 285 (the current baseline),
  and no previously-passing test regresses.
- `test_take` exists and covers (a) JSON round-trip lane-by-lane with stable seqs; (b) a v1
  fixture loads to lanes with `wallOnly` and correct counts; (c) a "future" fixture round-trips
  unknown control/section/feature BYTE-PRESERVED and reports them; (d) `RecorderClock` with a
  scripted `beatPhase` incl. a resync drop yields monotonic `beat` + anchors + an unmetered
  segment at `bpm=0`; (e) coalescing + `AutomationCurve::eval` linear/hold; (f) compile reports
  unresolved / rebound-by-position / rebound-by-name correctly; (g) `Player` under scripted dt
  (8.3 ms fixed, seeded jitter, one 400 ms stall) fires the identical discrete sequence each
  >= its `at`; a continuous gesture emits touch/set(interp)/release with values on the curve;
  `swap` mid-gesture re-seats and releases orphans; `stop` releases all; TOUCH refusal marks the
  lane displaced for that gesture only.
- Fixtures committed under `tests/fixtures/`.

## BOUNDARIES
- Touch ONLY: `src/model/ControlPath.h`, `src/recording/*` (except `VideoRecorder.*` — DO NOT
  TOUCH), `tests/`, `CMakeLists.txt` (to register the new sources + test target),
  `src/connect/AutomationCurve.h` (additive only), and whatever files reference `SessionRecorder`
  and must be updated for its deletion.
- DO NOT touch: `src/connect/ConnectionEngine.*`, `src/connect/ConnectionShaper.*`,
  `src/MainComponent.cpp` beyond the minimum needed to remove `SessionRecorder` references,
  `src/audio/*`, `src/render/*`, `.harmony/*`.
- DO NOT commit. DO NOT push. Report and stop.

## REPO GOTCHAS (each cost a prior session real time)
- Line numbers in this repo DRIFT. Re-grep by anchor text; never trust a cited line number.
- A negative grep is only as strong as its pattern. Grep for the SYMBOL, not a guess at it.
- `.harmony/` is gitignored with force-tracked files; `git add` there prints an "ignored" WARNING
  and still stages, which BREAKS `&&` chains — use `;`.
- Confirm you are in `~/projects/RealTimeAudio`, NOT `~/projects/RealTimeAudio copy` (stale, Jul 11).
- Do NOT launch the app. This packet is headless.

## REPORT FORMAT (compact, 8 fields)
WHAT / WHERE (files+symbols) / HOW VERIFIED (exact commands + counts) / WHAT IS NOT VERIFIED /
ASSUMPTIONS MADE / DEVIATIONS FROM THE SPEC (with reason) / RISKS LEFT OPEN / STATUS
(DONE | DONE_WITH_CONCERNS | BLOCKED).
Report the ctest count BEFORE and AFTER. Never inherit a count — run it.

## REPORT TO DISK — MANDATORY
Write your full report to `.harmony/.reports/s168-lane-a-recorder-core.md` BEFORE you return.
If your message is lost in transit, that file is the record. Then also return it in your reply.
