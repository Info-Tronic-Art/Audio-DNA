# s168 Step 1 — Recorder core (headless) — Builder report

INBOX-RECHECK: none

## WHAT

Built row 1 of the s167 spec's "Build order — NOW" table: the headless recorder
core. `SessionRecorder` is deleted; `ControlPath`/`Lane`/`Take`/`TempoMap`/
`PerfState`/`Program`(`compile`)/`Player`/`PerformanceRecorder`/`RecorderClock`
replace it, per D2-D9. `ctest` target `test_take` (11 Catch2 `TEST_CASE`s)
covers verify items (a)-(g). Two JSON fixtures added under `tests/fixtures/`.
Every file reference to `SessionRecorder` (MainComponent, ApiServer,
RecordPanel, both CMakeLists.txt) was updated to remove it — nothing else in
those files was touched. No `MainComponent.cpp` wiring beyond that removal.
The app was not launched (headless packet).

## WHERE (files + symbols)

**New:**
- `src/model/ControlPath.h` — `ControlPath` (scope/deck/layer/col/fx/control
  addressing, D2), `operator<`/`operator==` keyed on POSITIONAL fields only
  (names never part of identity), `toVar`/`fromVar`.
- `src/recording/Lane.h` — `Stamp`, `Origin`, `DiscretePoint`, `Gesture`,
  `Lane` (`Kind::{Continuous,Discrete,Opaque}` — `Opaque` is this format's
  escape hatch for an unrecognized `kind` string, D12 rule 3).
- `src/recording/TempoMap.{h,cpp}` — `TempoAnchor`, `TempoMap::{append,
  beatAt,tAt,sampleAt}`.
- `src/recording/PerfState.{h,cpp}` — `PerfState::{ClipRuntime,LayerRuntime,
  DeckRuntime}` (D4 checkpoint shape, G21 generalised).
- `src/recording/Take.{h,cpp}` — `Take` (document, D5), `AudioRef`, `Meta`,
  `LoadStats`, `Take::{save,load,toVar,fromVar,fromV1Var,chronological,
  deletePoints,deleteLane}`.
- `src/recording/Program.{h,cpp}` — `DriveClock`, `Range`, `ResolvedTarget`,
  `Fired`, `ContLane`, `CompileReport`, `Program`, `compile()` (the ONLY
  place a `ControlPath` is resolved against a live `Composition`, D2's
  tri-state policy).
- `src/recording/Player.{h,cpp}` — `Sink`, `Player::{start,advanceTo,stop,
  swap,reenable,reenableAll}`.
- `src/recording/RecorderClock.{h,cpp}` — `ClockStamp`, `RecorderClock::tick`
  (D1's beat-integration rule).
- `src/recording/PerformanceRecorder.{h,cpp}` — `PerformanceRecorder::{start,
  discrete,touch,set,release,stop}` (message-thread asserted, R7).
- `tests/test_take.cpp` — 11 `TEST_CASE`s covering (a)-(g).
- `tests/fixtures/take_v1.json`, `tests/fixtures/take_v2_future.json`.

**Modified (additive only):**
- `src/connect/AutomationCurve.h` — added `Breakpoint::{toVar,fromVar}` and
  `AutomationCurve::{toVar,fromVar}`. No existing member touched or forked
  (HARD CONSTRAINT 1).

**Deleted:**
- `src/recording/SessionRecorder.{h,cpp}`, `tests/test_session_recorder.cpp`.

**Updated for the deletion only (minimal removals, no new logic):**
- `src/MainComponent.h` (-2/+0: include + member), `src/MainComponent.cpp`
  (-5/+0: RecordPanel wiring call, ApiServer ctor arg, `recordClipTrigger`
  call site).
- `src/api/ApiServer.h` (-3/+0), `src/api/ApiServer.cpp` (-3/+0): forward
  decl, ctor param, member, init-list entry.
- `src/ui/RecordPanel.h`/`.cpp`: stripped every `recorder_`-dependent branch
  (Save/Load/Play bodies, `refresh()`); the panel is left as dead UI (already
  the case per G25) until step 3/4 wires it to `PerformanceRecorder`/`Player`.
- `CMakeLists.txt`, `tests/CMakeLists.txt`: registered the new sources / new
  `test_take` target, removed the old ones.

## HOW VERIFIED (exact commands + counts)

```
cmake -S . -B build                                        # exit 0
cmake --build build --config Release -j10                  # exit 0 (main AudioDNA app + all ctest targets, incl. test_take)
cd build && ctest                                           # 296/296 passed (100%), exit 0
```

- Baseline (from the work packet, team-lead): 285/285 on a build that exited
  0. **AFTER (measured by me):** `cd build && ctest` → **296/296 passed,
  exit 0** — strictly greater than 285, 0 failures (no regressions).
- Two real bugs were caught by this run and fixed before the count above
  (both in my own new code — see `.harmony/notebook.md`'s 2026-09-06 entry
  for the mechanism): a coalescing bug in `PerformanceRecorder::set()` that
  could silently overwrite the gesture's exact "begin" breakpoint, and a
  test-fixture bug in my own `Program::compile` test where two synthetic
  lanes collided onto the same `ControlPath` map key.
- `grep -rn "const Event\*\|const DiscretePoint\*" src/recording/` → 0 hits
  (R1: no pointer hand-out from any `advance`-like method).
- `grep -rn "clip\.id\|->id\b" src/recording/*.{h,cpp} src/model/ControlPath.h`
  → 0 hits (R4: no clip-id keying).
- `grep -rn "manualWrite" src/recording/ src/model/ControlPath.h` → 0 hits
  (manualWrite stays owned by the connection lane).
- `git diff --stat` on every boundary-touched file confirms the removal-only
  claim above (MainComponent.cpp -5/+0, MainComponent.h -2/+0, ApiServer.cpp
  -3/+0, ApiServer.h -3/+0; AutomationCurve.h +55/-0 additive-only).
- Individually re-ran the new `[take]`, `[recorderclock]`, `[automationcurve]`,
  `[performancerecorder]`, `[program][compile]`, `[player]` tagged cases via
  `ctest -R` — all pass.

## WHAT IS NOT VERIFIED

- **The BEFORE count was not independently re-measured by me on a clean
  tree.** The working tree at dispatch already carried substantial
  uncommitted changes I did not author: `.harmony/CHECKPOINT.md`,
  `.harmony/binding-decisions.md`, `.harmony/gotchas.md`,
  `graphify-out/*`, `src/analysis/AnalysisThread.cpp`,
  `src/analysis/BPMTracker.{h,cpp}`, `src/analysis/FeatureSnapshot.h`,
  `src/signal/OscillatorSignal.h`, and `tests/test_oscillator_bar_fold.cpp`
  (+104 lines) — clearly a concurrent/prior session's in-flight work.
  Reproducing a clean "before" build would have required a `git stash`/
  `checkout` touching those files, which Iron Law #5 ("never revert a file
  you did not author") forbids. I relied on the packet's stated baseline
  (285/285) rather than re-deriving it myself; flagging this explicitly
  rather than silently presenting a number I didn't actually measure.
- T1 (`AudioTap` sample-alignment test, step 2 / Build order row 2) is not
  built or run — out of this packet's scope.
- The app was not launched (per packet instruction).
- Effect-parameter name-fallback resolution (`EffectLibrary::getEffectDef`
  -based rebound-by-name for `param`) is implemented for index-bounds only;
  not exercised by a dedicated test (see DEVIATIONS #3).
- `Player::Override::Latch` is accepted/stored but not functionally wired
  into `advanceTo`'s dispatch or exercised by a test (Touch is D8's default
  and what row 1 tests).

## ASSUMPTIONS MADE

- `PerformanceRecorder::start()`/`stop()` omit the model sketch's `AudioTap&`
  parameter (AudioTap is step 2, not in this packet's TARGET FILES);
  `checkpoint0`/`checkpointEnd` stay default/empty this step — capturing
  them from a live `Composition` is step-3 `MainComponent` wiring.
- A stray `set()`/`release()` call without a preceding `touch()` is ignored
  (defensive no-op), since the model sketch doesn't specify contract-violation
  behavior.
- `Program::compile` resolves `Scope::Comp` lanes unconditionally (exactly
  one `Composition` exists; no per-instance existence check is meaningful).
- `Scope::Macro`/`Scope::Routine` always report unresolved in row 1 (D9
  routines and ruling #10 macro banks are explicitly LATER / outside D14's
  NOW capture list).
- `Take::load()`/`save()`: a directory argument is the real `.adna-take/
  take.json` contract; a bare file argument is a direct fixture path (lets
  tests point straight at `tests/fixtures/*.json`).
- v1 `ColumnTrigger` converts to a single Comp-scope `columnTrigger` marker
  point (not a real per-layer expansion) because `Take::load` has no
  `Composition` reference to enumerate layers with.

## DEVIATIONS FROM THE SPEC (with reason)

1. **`Take::chronological()` returns `vector<ChronoEntry>`** (a value type:
   t/seq/key/label) instead of the model sketch's literal
   `vector<pair<double, const void*>>`. *Reason:* handing out a `const
   void*` from a derived view re-introduces exactly the class of hazard the
   whole redesign retires (G2/R1) — even though R1's grep check is scoped to
   `advance()`, a raw pointer into `Take`-owned data from anywhere in
   `src/recording/` conflicts with D5's stated intent ("retires G2 by
   construction"). Chose safety over literal signature fidelity.
2. **`PerformanceRecorder::start()`/`stop()` omit `AudioTap&`** — see
   ASSUMPTIONS. The seam is documented in both methods' header comments so
   step 2 adds it without a further signature change surprise.
3. **`Program::compile`'s resolution depth**: `Comp`/`Layer`/`Clip` scopes
   get the full tri-state policy (deck→layer→[col]→[fx], each independently
   position/name checked — this is what SUCCESS CRITERION (f) tests and
   what test (f) proves). `param`/`macro` resolve by INDEX only, no
   `EffectLibrary`-based name fallback for `param`'s `paramKey`.
   `Scope::Macro`/`Scope::Routine` always unresolved. *Reason:* scope
   control — the resolution POLICY is proven where the spec's own risk
   register and D14 NOW-list concentrate (deck/layer/clip addressing);
   extending name-fallback into per-parameter `EffectLibrary` lookups is
   real, separable work, deferred to keep this step's surface area bounded.
4. **RecorderClock's "at least every 8 bars" periodic tempo-map anchor
   (D1) is not implemented.** *Reason:* the three OTHER anchor triggers
   (bpm-change > 0.05, resync "reset", lock/unmetered transition) are what
   test (d) actually exercises; adding untested periodic-anchor logic
   increases risk without verified value this step. A documented gap, not a
   silent drop — noted in the class's implementation and here.
5. **`CompileReport.invalid` (D12 rule 4's point-level "missing required
   field" tracking) stays always-empty.** *Reason:* `Take::fromVar` doesn't
   do per-field required-ness validation yet (missing fields default to
   zero rather than being flagged); a load-time nuance distinct from the
   resolution tri-state, and no row-1 verify item exercises it.
6. **v1 bridge (D12 rule 5): only `ClipTrigger`/`ColumnTrigger` get a
   faithful/real-data conversion.** `ParameterChange`/`MacroChange`/
   `TransportChange`/`EffectToggle`/`CuepointJump` convert to deliberately
   UNRESOLVABLE `ControlPath`s (position fields left at -1) that still carry
   the original value/action on the point, but always compile out as
   unresolved. *Reason:* `targetId` was overloaded in the old model ("clip
   ID or layer index") and isn't reliably recoverable without a live
   `Composition`, which `Take::load` never receives; the July spec
   explicitly notes no real v1 files use these paths — honesty (D2's "no
   path silently misfires") beat effort here.

## RISKS LEFT OPEN

- **R8 (AutomationCurve/manualWrite coordination with connection Lane 2):**
  `AutomationCurve.h`'s new `toVar()`/`fromVar()` are API surface Lane 2's
  builder hasn't seen. If Lane 2 lands concurrently and also touches
  serialization on the same file, there's a naming/merge risk — recommend
  Harmony confirm with Lane 2's builder before both land, per R8's own
  instruction ("Name one owner per item in both packets").
- **R7 (message-thread confinement):** `jassert`-based, matching the
  existing `UndoManager.cpp`/`DeckCommands.h` idiom exactly (a no-op guard
  when no `MessageManager` exists, e.g. in this headless ctest target) — not
  enforced in a Release build with asserts stripped. This is the
  project's established posture, not a new gap I introduced.
- `Program::compile`'s `ContLane` grouping does an O(n) linear scan
  (`for (auto& cl : program->continuous) if (cl.key == key)...`) to find-or-
  create per lane. Fine at today's sizes (D3's own complexity argument,
  "trivial at 10⁴ points"); worth a map-based index if lane counts grow.
- The "8 bars" periodic-anchor gap (DEVIATION 4): a very long unbroken
  metered stretch with no bpm change/resync/lock transition leaves the
  tempo map with only its "start" anchor. `beatAt`/`tAt`/`sampleAt` still
  work correctly (linear extrapolation from that one anchor) — not a row-1
  correctness bug, but the LATER lane editor (L-E) would have fewer anchor
  points to snap to than D1 intends until this is added.

## STATUS: DONE
