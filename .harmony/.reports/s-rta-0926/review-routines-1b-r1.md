# Reviewer Verdict — routines-1b-r1 (round 1)
STATUS: DONE
VERDICT: APPROVE

Scope: lane 1b of s-rta-0926 (routines slice 1), diff `main...worktree-wf_91d4473b-9fb-1`
(commits a137a6f..7951be5). Read-only review; no build, no launch, no debugger. All claims
below are VERIFIED by reading the diff on disk unless marked INFERRED.

## LANE 2 reviewer greps (plan-routines-s1-final.md :685-689) -- all PASS

1. `grep -rn "Origin::Routine" src/` -> only `Lane.h:58,68` (enum-string round trip, vocabulary
   only). No capture-path reference. PASS.
2. `grep -n "routines\|routineBank" src/model/Composition.h` -> both cleared together in
   `fromVar` (`Composition.h:611-612` "routines.clear(); routineBank.clear();"). PASS.
3. Every `firePreamble` call in `RoutineEngine.cpp` is gated on `restore`:
   `startNow` (`:172-177`, `if (r.restore)`) and the loop-restart path (`:255-260`, `if (r.restore)`).
   PASS.
4. `grep -n mutex` -> the only `std::mutex` in `RoutineEngine.{h,cpp}` is `statusMutex_`, used only
   inside `publishStatus()` (`:450`) and `status()` (`:456`). PASS.
5. `git diff main...HEAD -- src/recording/Player.cpp src/connect/ManualWrite.cpp` -> empty. PASS.
6. `grep -n "routineEngine_\.dispatch\." src/MainComponent.cpp` -> the `notify` lambda body
   (`MainComponent.cpp:2055-2058`) does not name `routineEngine_`; every OTHER hit is a caller
   invoking `dispatch.notify(...)` from outside the lambda (`:5533,5624,5653,5665,5676,5691,5695`),
   not a lambda body naming itself. No re-entrancy. PASS.
7. `Binding::Action` diff (`Binding.h`) is a pure append: `TriggerRoutine` follows `MasterSignal`
   directly, no enumerator inserted earlier; `targetRoutineSlot` is a new field, append-only.
   Confirmed by `Binding.h` diff and pinned by test 12
   (`CHECK(TriggerRoutine == MasterSignal + 1)`, plus the legacy-JSON-without-the-key case). PASS.
8. `grep -n "running_\.erase\|erase_if" src/recording/RoutineEngine.cpp` -> two sites, both
   `std::erase_if`: `:278` (end of `tick()`'s loop, i.e. AFTER it, matching F5) and `:357`
   (`stop()`, a separate function, never called from inside `tick`'s loop). No raw `.erase` inside
   the tick loop. PASS.

## Threading, ordering, and lifecycle (VERIFIED)

- Message-thread confinement: `ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD()` guards every public entry
  point (`RoutineEngine.cpp:196,286,352,363,373,381`), same jassert idiom as `RecorderHost`.
- Tick order: `recorderHost_.tick` (`MainComponent.cpp:3454`) -> `routineEngine_.tick`
  (`:3463`) -> `connectionEngine_.tick` (`:3482`) -- exactly the plan's order, with the comment at
  `:3459-3461` explaining why (replay-then-routine writes last; grip current before
  ConnectionEngine reads it).
- `stopAll()` before every model-invalidating event: TopBar `onStop` (`:704`), `GlobalStop`
  binding (`:7197`), `swapCompositionModel` before the swap (`:2860`, comment cites plan R7),
  `~MainComponent` before `recorderHost_.shutdown` (`:2305`). `routineEngine_` is declared AFTER
  `recorderHost_` in `MainComponent.h:579` (destroyed first) with an explicit comment stating why.
- Missing targets are counted, never silent: `RoutineEngine::Status::Slot` carries `unresolved`,
  `preambleUnresolved`, `preambleRefused`, `skipped`, `yielded` (`RoutineEngine.h:66-70`), all
  populated from `Program::report` / `SlotSink` counters in `publishStatus()`
  (`RoutineEngine.cpp:439-447`) and surfaced in `/api/routine/status` and the start notice
  (`startNow`, `:180-190`).

## F1 stacking / F4 quantize parity / F5 compaction (VERIFIED against code + tests)

- F1 (D9 "later begin wins"): `SlotSink::touch/set/release` (`RoutineEngine.cpp:59-95`) implement
  exactly the plan's rule -- an accepted touch claims `laneOwner_[key]`; a `set` from a
  non-owning slot is refused and counted `yielded`; `release` is a no-op unless this slot is the
  current owner (the ownership entry is deliberately NOT erased on release, matching the header
  comment at `RoutineEngine.h:127-130` and R14's disclosure). Test 10
  (`tests/test_routine_engine.cpp:507-550`) discriminates fire-order from begin-order in three
  sections plus a restore-that-begins-later section, with non-vacuous assertions
  (`REQUIRE_FALSE(later.empty())`, explicit release counts).
- F4 (barCount parity): `dueNow` (`RoutineEngine.cpp:142-153`) uses `lastBarCount_` (the value just
  refreshed from the current snapshot, `:208`) for TwoBar/FourBar parity while the EDGE itself is
  `totalBarCount`'s rising edge (`:205`). Test 11 (`:554-613`) pins the exact discriminating case
  from the plan (totalBarCount even / barCount odd does NOT fire; the next edge, barCount even,
  does).
- F5 (compaction): the tick loop (`:214-277`) is an index loop with no erase inside it; finished
  routines are marked `r.done = true` and compacted in ONE `std::erase_if` immediately after the
  loop (`:278`). Test 15 (`:652-696`) fires two once-mode routines that both cross their end in the
  SAME tick and checks exactly one release each, no duplicate/skip, and that a later `fire()` on
  the same pad starts a genuinely NEW run (not a restart of the removed one).

## Surfaces (VERIFIED against plan section 5)

- REST: all six `/api/routine/*` routes match the plan's shape (503 when unwired, 400-in-words on
  a malformed body, `callAsync` for every mutation, synchronous status read from
  `RoutineEngine::status()`'s mutex-guarded copy -- `ApiServer.cpp:1520-1698`).
- `/api/composition` clip `effects` readback reads `fx.effParam(pi)` (the engine-driven effective
  value, per CLAUDE.md pitfall 33), not the raw `paramValues` field (`ApiServer.cpp` diff
  lines ~27-48).
- OSC: `/audiodna/routine/{slot}` wired at `OscHandler.cpp:179` and `MainComponent.cpp:2165`,
  0-based slot, value > 0 fires.
- Binding: `TriggerRoutine` case in `handleBindingAction` (`MainComponent.cpp:7202-7209`) --
  press fires, Momentary release stops; 8 "Routine 1".."Routine 8" targets added
  (`MainComponent.cpp:6918-6926`, whole words); the three overlay minting sites
  (`BindingOverlay.cpp:197`, `MidiLearnOverlay.cpp:233,280`) and both `findExisting*` switches
  (`BindingOverlay.cpp:292-294`, `MidiLearnOverlay.cpp:339-341`) all carry `routineSlot`.

## Tests (VERIFIED)

8 new `TEST_CASE`s in `tests/test_routine_engine.cpp` (cases 7-12, 15, plus the save-refusal
case), matching the plan's table 6.1 and the builder's own count. The stacking (10), quantize (11)
and compaction (15) cases are the three hardest to get right and all three use non-vacuous
assertions (`REQUIRE_FALSE(...empty())`, explicit counter checks) rather than trusting an empty
list. `tests/CMakeLists.txt`'s new `test_routine_engine` block matches the plan's link set exactly
(Player/RecorderClock/BindingManager + RoutineEngine.cpp on top of the `test_routine` set).

## Live probe (read, not run)

`.harmony/probe-routines.sh` (bash -n clean) + `.harmony/probe-routines.json` cover all 12 rows
from the plan. Notable non-vacuousness: the restore checks (rows 5-6) read `/api/composition` and
`/api/bpm`/`/api/routine/status` independently of each other (restore verified by the actual model
state, not by trusting the engine's own counters); the perturbation (row 4) is asserted visible
(`mad(ref,pert) > threshold`) BEFORE the restore is asserted to remove it; the stacking row (11)
requires non-empty sample windows (`bool(vals) and all(...)`) before declaring PASS, so an empty
sample window cannot silently read as a pass. The embedded Python (lines 76-324) compiles
(`python3 -m py_compile`, confirmed this pass). Fixture load is checked
(`load_composition accepted the fixture`). RED-then-GREEN is not run by this reviewer (no build/
launch), but the builder's reported dev-run progression (23/46 -> 30/40 -> 34/40 -> 73/1 with a
tempo-map workaround) is consistent with a probe that starts RED against the pre-1b binary; Harmony's
live gate is the authority for the actual RED/GREEN transition.

## Deviations reviewed (from builder summary)

All disclosed deviations are consistent with the diff: the `RoutineSnap` local enum plus
`static_assert` mirroring `Clip::BeatSnapMode` (keeps `RoutineEngine.h` juce_core-only, verified --
it only forward-declares `Composition`/`FeatureSnapshot`, and `Program.h`/`Player.h` are
juce_core-only); the extra `beatAvailable` tick parameter; `laneOwner_` never erased on release
(matches D9's "stays displaced for its remainder" reading, and is exactly what test 10's "restore
that begins later" section pins); the commit order (root CMake before REST/OSC before wiring) so
every commit builds. Carried concern (a)'s auto-play capture (`triggerWillAutoPlay` /
`captureAutoPlay`, `MainComponent.cpp:176-180,4403-4414`) is a narrow, well-scoped addition
(records a "resume" playing point only for a clip that will auto-play on first activation) and
does not touch the routines-1b fence itself.

## Nits (non-blocking)

- `captureAutoPlay`'s single-clip-trigger call site (`MainComponent.cpp:4395`) passes `group = 0`
  for its capture group, while the column-trigger call site (`:4494`) passes a shared
  `captureGroup`. Not a correctness issue (group is metadata, not a control key), but worth a
  one-line note if group-0 is ever treated as "no group" elsewhere.
- `RoutineEngine::Running::preambleFired`/`preambleRefused` accumulate across cycles (loop) rather
  than resetting per-cycle; this matches the probe's expectation ("preambleFired grew at the
  loop") and is not a bug, just worth flagging as a deliberate cumulative counter for anyone
  reading `Status::Slot::preambleFired` expecting a per-cycle value.

## Verdict rationale

No blocking correctness issue found. The three hardest mechanisms in the plan (D9 stacking, F4
barCount-parity quantize, F5 same-tick compaction) are implemented exactly as specified and are
each pinned by a non-vacuous test that discriminates the mechanism from its obvious wrong
implementations (fire-order instead of begin-order; totalBarCount instead of barCount parity;
erase-inside-loop instead of mark-then-compact). Every LANE 2 grep passes. Threading, ordering,
and shutdown/swap safety (R7) are all correctly wired and disclosed where relevant (R2, R3, R13,
R14 disclosed, not silently "fixed"). REST/OSC/Binding surfaces match the plan's shape including
the 503/400/callAsync posture and the effParam (pitfall-33) readback. Forbidden files
(Player.cpp, ManualWrite.cpp, Lane.h, ControlPath.h, Program.*, RoutineSlice.*, Routine.h, Take.*)
are untouched, confirmed by empty diffs.

METADATA: reviewer=reviewer-1, builder_packet=routines-1b, date=2026-09-26
