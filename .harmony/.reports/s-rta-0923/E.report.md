# Lane E report — RecorderClock periodic anchor + Player backwards seek + L4 Latch-refusal rider

## STATUS: DONE_WITH_CONCERNS (self-check only — Harmony's gate/Reviewer are authoritative)

## Worktree
`/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b6c30536-17a-3`
(`git rev-parse --show-toplevel` == this path; `git log -1 --oneline` == `bfc3907 chore(harmony): take back this repo's own review verdicts from the Harmony primary`, descends from main HEAD `e3504be` per the amended spec.)

## Patch / report paths
- Diff: `/private/tmp/rta-patches/E.patch` (5 files changed, 322 insertions(+), 12 deletions(-))
- This report: `/private/tmp/rta-patches/E.report.md`

## Summary

Implemented Lane E from the amended spec (`s-rta-0923-ruling28-audio-store.v2.md` §8, §9, §10.2) plus the L4 rider
carried from `s-rta-0923-review-fixes-plan.md`:

1. **RecorderClock periodic tempo anchor** (review fix c, §8). Added `kPeriodicAnchorBeats = 32.0`, `lastAnchorBeat_`,
   and a private `anchor(t, beat, sample, bpm, why)` helper that both appends to the map and records where the last
   anchor landed. All 5 existing `tempo_.append` call sites now route through `anchor()`. In the metered branch, after
   the existing bpm-change check, if no anchor was written this tick and `beatNow - lastAnchorBeat_ >= 32.0`, a
   `why:"periodic"` anchor is written. No periodic anchors while unmetered (documented limitation, per spec).

2. **Player backwards seek** (HANDOFF addendum item 2, §9). Added `Player::seek(double pos, Sink& sink)`: for every
   lane currently `inGesture`, retains the grip if the CURRENT gesture still covers `pos` (no release, no fresh
   touch — the next `advanceTo` re-evaluates the curve), otherwise releases (unless already displaced) and resets
   the cursor. Then re-seats `nextDiscrete_`/`gestureIndex` exactly as `start(at)` does — no state synthesis.
   `advanceTo(pos, sink)`: if `pos < pos_`, calls `seek(pos, sink)` first, then runs its normal forward pass — a
   backwards jump is never a stall; a caller wanting a deliberate forward jump WITHOUT catch-up must call `seek()`
   explicitly (documented on both methods).

3. **L4 rider — `Player::Override::Latch` refuses loudly** (review addendum 4b). Replaced the old
   `void setOverride(Override o) { override_ = o; }` with `[[nodiscard]] bool setOverride(Override o);` +
   `Override overrideMode() const`. The `.cpp` implementation refuses any request other than `Touch`: logs via
   `juce::Logger::writeToLog`, returns `false`, leaves the mode unchanged. Confirmed via grep that nothing else in
   the codebase called `setOverride`/read `override_` before this change, so no other call sites needed touching.

## Files changed (owned list — nothing outside it)
- `src/recording/RecorderClock.h` (+12/-0)
- `src/recording/RecorderClock.cpp` (+32/-6 net, i.e. touched lines route through the new `anchor()` helper)
- `src/recording/Player.h` (+36/-4)
- `src/recording/Player.cpp` (+61/-0)
- `tests/test_take.cpp` (+193/-0) — 6 new `TEST_CASE`s appended at EOF, 11 → 17 (+6, matches §10.2 acceptance)

`Program.cpp` was NOT touched (out of scope, owned by another lane); no CMake file was touched (Lane E appends
nothing to `tests/CMakeLists.txt` per spec §10.2/§4.3).

## Fail-first proof

Baseline confirmed at HEAD `e3504be` state before any edit — sha256 of the 4 pre-change source files matched the
spec's §10.5 pins exactly:
```
3104e1c0457b6d227d96f588efb753f8ba1c182ed122463f0996ae780145d066  src/recording/RecorderClock.h
9b0fe0803624fe85e3b04c9775048bd3036a7f5e60b05faba55b226ec3fcfeab  src/recording/RecorderClock.cpp
b46ed9bef4eced3ded045981f35cc8dab68dfa65f44b18445cff13479dd5985c  src/recording/Player.h
fe4e5c34e889d3a1d081e5e4b394f95812e62bb1495419dd4b94dcf6c2b52b72  src/recording/Player.cpp
```

**Whole-TU proof (as predicted by the spec — "does not compile"):** with all 6 new tests present against the
pre-change `RecorderClock.*`/`Player.*`, the build fails: `error: no member named 'seek' in 'Player'` and
`error: no member named 'overrideMode' in 'Player'` (11 errors total) — exactly the "Pre-change: does not compile"
outcome the spec states for the L4 test and implicitly for the `seek()`-calling test.

**Isolated behavioral proof (per-hunk, mutation-style):** to see the RecorderClock and backwards-seek tests fail
*behaviorally* rather than being masked by an unrelated compile error, I temporarily wrapped the two tests that
reference the brand-new API surface (`seek()` used explicitly, `overrideMode()`/`setOverride()`) in `#if 0 ... #endif`
(never touching `RecorderClock.*`/`Player.*` themselves), rebuilt against the reverted sources, and ran the
remaining 15 test cases:
```
RecorderClock stays within 0.05 beats and one block of the map over a long steady take
  FAILED: std::fabs(clock.tempo().beatAt(now.t) - now.beat) < 0.05
  with expansion: 1.19995115662732132 < 0.05        <- matches spec's predicted "pre-change ~= 1.2"

Player: a backwards advanceTo re-seats and events re-fire on re-pass
  FAILED: sink.releases.size() == 1
  with expansion: 0 == 1                             <- pre-change advanceTo never releases on a backward pos

test cases:  15 |  13 passed | 2 failed
assertions: 138 | 136 passed | 2 failed
```
(The other 2 new tests — "no periodic anchors while unmetered" and "seeking within a gesture keeps the grip" — pass
even pre-change, as expected: the spec does not claim they discriminate pre/post-change behavior; they are guard/
contract tests, not fail-first probes.)

Restore: copied the 4 source files and `tests/test_take.cpp` back from the pre-edit backup
(`/private/tmp/claude-501/.../scratchpad/lane-e-backup/`), then verified sha256 byte-identical to the post-change
state captured immediately after implementation (all 5 hashes matched exactly, including `tests/test_take.cpp`).
Rebuilt clean, ran the full suite: **all 17 test cases pass, 480 assertions, 0 failures.**

## Test counts

- `test_take`: 11 → 17 TEST_CASEs (+6), matching §10.2's acceptance exactly.
- Ran via both the raw binary (`./build-lane/tests/test_take`) and `ctest --test-dir build-lane -R "Player|RecorderClock|Take|Automation|Program"` (16/16 matched tests passed; the 17th, `PerformanceRecorder coalesces...`, is unaffected by this lane and passed in the full-binary run).
- Rebuild of only the touched `.o` files produced zero compiler warnings or errors.

## Build

`cmake -S . -B build-lane -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_JUCE=... -DFETCHCONTENT_SOURCE_DIR_CATCH2=... -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=... -DFETCHCONTENT_SOURCE_DIR_SYPHON=...` configured cleanly (the `MELATONIN_INSPECTOR` source-dir override was silently unused — that FetchContent dep isn't referenced by this target, harmless). Only `test_take` was built (this lane's only test target; no app-source changes, so `AudioDNA` was not built).

## NOT done / out of scope (named, not built here)

- `AudioTap.*`, `Take.*`, `AudioStore.*`, `tests/CMakeLists.txt`, root `CMakeLists.txt` — Lane R28's, untouched.
- `Program.cpp` — review-fix L2's (exact stamps in `Program::compile`); still owed per spec R7. This lane's periodic
  anchors shrink the error but the compile path still reconstructs `x` from the map until L2 lands.
- ThreadSanitizer run — the spec's `[concurrency]`/`[flush]` TSan requirement (§10.1 acceptance) is R28's
  (`test_audio_tap_sync`), not this lane's; `test_take` has no threading of its own to sanitize.
- Step 3 wiring (MainComponent/RecordPanel calling `seek()` vs `advanceTo()` per the caller contract, R8) — explicitly
  out of scope for this spec; named so it isn't missed later.

## Risks (named in the spec, still true after this lane)

- **R8**: `seek()` vs `advanceTo`'s catch-up is a caller contract. If step 3 only ever calls `advanceTo`, a
  deliberate forward scrub will fire every skipped discrete event as a burst — documented on both methods in
  `Player.h`, but enforcement is step 3's job, not this lane's.
- **R10**: periodic anchors are strictly increasing in `beat` while metered (verified by the long-take test's
  `anchors[i].beat - anchors[i-1].beat <= 32.5` assertion across all ~161 anchors), so `TempoMap::bracketByBeat`
  stays unambiguous; the unmetered case is separately covered.
- The long-take test runs 288,000 `RecorderClock::tick()` calls; it completed in ~10 ms in this build (no perf
  concern), but it is by far the heaviest test in `test_take` — noting this in case CI timing budgets are tight.

## Deviations from the packet

None. Implementation follows §8/§9/L4 verbatim, including the exact log string and refusal semantics for
`setOverride`, the exact `overrideMode()` naming (not `override()`, a contextual keyword), and the `[[nodiscard]]`
annotation.

## Tester checklist (for Harmony's gate / independent Reviewer)

1. Confirm `git diff --stat` for this lane touches only `src/recording/RecorderClock.{h,cpp}`,
   `src/recording/Player.{h,cpp}`, `tests/test_take.cpp` — nothing else.
2. Rebuild `test_take` in a clean directory; confirm 17 TEST_CASEs, 0 failures.
3. Spot-check the periodic-anchor math independently: at 128 BPM, 32 beats = 15 s wall-clock; the long-take test
   should show `periodicCount` close to `5120/32 = 160`.
4. Spot-check `Player::seek`'s retain/release logic against §9's prose directly (the "grip retained if
   `g.x0 <= pos < g.x1`" rule) rather than trusting only the tests.
5. Confirm `Player::setOverride(Override::Latch)` really returns `false` and leaves `overrideMode()` at `Touch`
   (not just that the test asserts it — read the one-line `if` in `Player.cpp`).
