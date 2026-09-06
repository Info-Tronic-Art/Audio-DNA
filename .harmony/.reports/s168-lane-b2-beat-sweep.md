# s168 Lane B2 — monotonic beat sweep, made live-verifiable

### STATUS
DONE

### RESULT
All four packet items implemented: `ConnectionShaper::beatsNow` re-pointed
to `totalBarCount` with a `ConnShape::resetPhaseOnStructural` opt-in (same
name/default as `OscillatorSignal`'s switch); `EnvelopeSignal::getValue`
fixed (was reading `barCount` unconditionally despite a comment claiming
otherwise) with the identical switch; `totalBarCount` exposed through
`ApiServer::handleGetBpm`/`handleInjectFeatures` and `TestServer`'s
injection builder; two named test gaps closed (BPMTracker-level
monotonicity test, four default-mode sibling assertions in
`test_oscillator_bar_fold.cpp`). Build exits 0. `ctest` in a fresh
`build-b2`: 301/301 pass (baseline 296, so +5 net across the whole
concurrently-edited tree; my own additions are +1 new TEST_CASE plus
assertions folded into four existing ones). The new BPMTracker test was
proven non-tautological by an actual mutation-and-restore (see below),
with a sha256 check confirming the source file was restored byte-for-byte.

### FACTS
- `src/connect/ConnectionShaper.cpp:16` — `beatsNow` now takes
  `(beatPhase, beatInBar, barCount, totalBarCount, resetPhaseOnStructural=false)`.
- `src/connect/ParamConnection.h:79-87` — new `ConnShape::resetPhaseOnStructural`
  field (default false), serialized in `ConnSerialization.cpp` (toVar line
  ~154, fromVar line ~278).
- `src/signal/EnvelopeSignal.h:31-58,137-143,153-158` — new
  `resetPhaseOnStructural_` member + accessors, `getValue` now branches
  like `OscillatorSignal::getValue`.
- `src/api/ApiServer.cpp:552-555` (handleGetBpm), `:644-650`
  (handleInjectFeatures) — additive `totalBarCount` JSON field.
- `src/test/TestServer.cpp:462` — additive `totalBarCount` read in the
  injection snapshot builder.
- `tests/test_bpm_stabilization.cpp:559-586` — new TEST_CASE proven
  non-tautological by mutation (see HOW I PROVED section).
- `tests/test_oscillator_bar_fold.cpp` — 4 default-mode sibling blocks
  added (one per pre-existing legacy-opt-in case), file-level doc comment
  updated to describe the new state accurately.
- `ctest` in `build-b2`: 301/301, 0 failures, confirmed via `shasum -a 256`
  that a deliberate mutation-and-restore of `src/analysis/BPMTracker.cpp`
  left the file byte-identical to before
  (`0b22cd202845b4716d009467fa07d4a7d6ba12fdfffe14f1af0e45f99c49afb1`).

### METHOD
Read the full work packet, then `src/signal/OscillatorSignal.h` (the
sibling lane's design) before touching anything. Enumerated every
`barCount` consumer via `grep -rn "\bbarCount\b" src/ tests/` (excluding
build dirs) before editing. Implemented the four task items in order,
fixed the consequential test breakage each production change caused
(`test_connection.cpp`'s `beatsNow` call sites and Lfo/Envelope-Beats test
snapshots; three pre-existing `EnvelopeSignal` tests in
`test_oscillator_bar_fold.cpp` that would otherwise silently read
`totalBarCount=0` under the new default). Built in a dedicated
`build-b2` directory (never touched `build/`), ran `ctest` to a clean
301/301, then did an actual mutation test on `BPMTracker.cpp` (not a
thought experiment) to prove the new BPMTracker-level test bites, restored
the file, and re-verified both the sha256 hash and a clean 301/301 rerun.

### CONFIDENCE+VERIFY
High on build/test correctness (empirically re-run three times: clean
build-b2 build/ctest before the mutation test, mutation-induced FAIL,
restore + re-verify build/ctest green again). Medium-high on the
serialization addition to `ConnSerialization.cpp` — it wasn't named in the
work packet's TARGET FILES, but leaving `ConnShape::resetPhaseOnStructural`
unserialized would have been the ONE field in an otherwise fully
round-tripped struct that silently reset on every save/load; flagging this
judgment call explicitly below (ASSUMPTIONS) rather than asserting it's
certainly wanted.
To re-verify: `cd ~/projects/RealTimeAudio && cmake --build build-b2 --config Release -j6 && cd build-b2 && ctest` (expect 301/301). Live-verify totalBarCount via `/api/bpm` and `/api/inject_features` (test mode only) once the app is run — I did not launch the app per the DO-NOT constraint.

### UNKNOWNS-NOT-DONE
- Did not add a UI toggle for `resetPhaseOnStructural` on any of the three
  places it now exists (Oscillator/Envelope/Connection) — matches existing
  precedent (Oscillator's own comment: "Boris has not ruled on a
  preference yet"), not asked for by this packet.
- Did not add default-mode (totalBarCount) sibling assertions to the 3
  pre-existing EnvelopeSignal tests in `test_oscillator_bar_fold.cpp` — the
  packet's item 4b named exactly "the four" (OscillatorSignal) cases; I
  gave the 3 EnvelopeSignal ones only the minimal legacy opt-in needed to
  keep them passing under the new default. Flagged in RISKS below.
- Did not run the app or hit `/api/bpm`/`/api/inject_features` live — DO
  NOT launch the app per the packet.

### NUANCE
The `ConnSerialization.cpp` addition (serializing the new
`resetPhaseOnStructural` field) is a real scope judgment call: it's
additive-only and matches the "one design, three places" intent, but
`ConnSerialization.cpp` isn't in the packet's named target files. I made
the call because every OTHER `ConnShape` field is explicitly round-tripped
(min/max/invert/playback/loop/curve/inMin/inMax/smoothMs) — leaving this
one out would be a silent asymmetry in a struct whose whole job is to be
saved with the composition, unlike Oscillator/Envelope's copy of the same
switch (which has no serialization mechanism at all to plug into).

### HANDOFF-NEEDS
None — no other lane's work is blocked on this. Reviewer should double
check the `ConnSerialization.cpp` scope call above and the two test-file
maintenance fixes (`test_connection.cpp`, the 3 EnvelopeSignal opt-ins) I
made as unavoidable consequences of the production signature/default
changes, not scope expansion of my own choosing.

---

## WHAT

Four task items from the work packet, all done:

1. **`ConnectionShaper::beatsNow()`** (`src/connect/ConnectionShaper.cpp`,
   `.h`) — re-pointed to `totalBarCount` by default, with a new
   `ConnShape::resetPhaseOnStructural` bool (default `false`) as the
   opt-in back to the legacy `barCount`-driven fold. Call site
   `ConnectionEngine::evaluate` (`src/connect/ConnectionEngine.cpp:95`)
   updated to pass `ctx.snap.totalBarCount` and `c.shape.resetPhaseOnStructural`.

2. **`EnvelopeSignal::getValue`** (`src/signal/EnvelopeSignal.h`) — was
   reading `snapshot.barCount` unconditionally while its own comment
   claimed it "already received the same fold-across-bars fix as
   OscillatorSignal." That claim was false — there was no switch at all.
   Fixed the code to branch on a new `resetPhaseOnStructural_` member
   (default `false` = `totalBarCount`), matching `OscillatorSignal.h`
   field-for-field (same getter/setter names, same default-value comment
   style), and fixed the lying comment with an explicit CORRECTION note.

3. **Live-verifiable via HTTP**:
   - `ApiServer::handleGetBpm` — added `totalBarCount` to the JSON response.
   - `ApiServer::handleInjectFeatures` — added `totalBarCount` injection,
     clamped `[0, INT_MAX]` (juce::var's int property is 32-bit signed;
     `barCount`'s own precedent clamps to its real uint16_t range, so I
     clamped to the practical range juce::var can carry rather than
     uint32_t's full range — no legitimate sweep needs more).
   - `TestServer::handleInjectFeatures`'s snapshot builder — added
     `snap->totalBarCount = static_cast<uint32_t>(get("totalBarCount"))`.
   All additive; `barCount` untouched at every site.

4. **Two named test gaps closed**:
   - (a) New TEST_CASE in `tests/test_bpm_stabilization.cpp`: "Real audio:
     structural transition into drop zeroes barCount while totalBarCount
     keeps climbing" — mirrors the file's existing "Real audio: structural
     transition into drop still resets barCount" pattern exactly (same
     `feedRealOnsets` sequence), adding assertions that `totalBarCount()`
     is `> 0` after the build-up phase (BEFORE any reset is fed) and
     `> totalBarCountBeforeTransition` after the same transition hop that
     zeroes `barCount()`.
   - (b) Four default-mode (`totalBarCount`-driven) sibling assertion
     blocks added to `tests/test_oscillator_bar_fold.cpp`, one appended to
     each of the four pre-existing OscillatorSignal test cases that had
     opted into `setResetPhaseOnStructural(true)` (the legacy path). Each
     sibling re-runs that test's own scenario through a default-settings
     oscillator using `totalBarCount` instead of `barCount`.

## WHERE (files + symbols)

- `src/connect/ConnectionShaper.h` / `.cpp` — `beatsNow()` signature + impl.
- `src/connect/ParamConnection.h` — `ConnShape::resetPhaseOnStructural`.
- `src/connect/ConnSerialization.cpp` — round-trips the new field (see NUANCE).
- `src/connect/ConnectionEngine.cpp` — `evaluate()`'s `beatsNow` call site.
- `src/signal/EnvelopeSignal.h` — `getValue`, `resetPhaseOnStructural_`,
  getter/setter, fixed comment.
- `src/api/ApiServer.cpp` — `handleGetBpm`, `handleInjectFeatures`.
- `src/test/TestServer.cpp` — `handleInjectFeatures` snapshot builder.
- `tests/test_connection.cpp` — `beatsNow` call-site fixes (signature
  change), `totalBarCount` companions added wherever a test set a nonzero
  `barCount` and depended on the fold result (bar-loop test, 8-beat LFO
  test, loop=false hold test). Zero-`barCount` sites left untouched (they
  already match the new `totalBarCount=0` default with no behavior change).
- `tests/test_oscillator_bar_fold.cpp` — 4 new sibling blocks + file-level
  doc comment rewrite + 3 EnvelopeSignal opt-in fixes (see below).
- `tests/test_bpm_stabilization.cpp` — new TEST_CASE.
- `src/analysis/BPMTracker.cpp` — NOT modified by me; touched only during
  the mutation test, then restored and hash-verified identical to before
  (it already carries lane B's `totalBarCount_` implementation).

## EVERY `barCount` CONSUMER FOUND, MIGRATED OR NOT, WITH REASON

Enumerated via `grep -rn "\bbarCount\b" --include="*.cpp" --include="*.h" src/ tests/` (excluding build dirs), before touching anything:

| Consumer | Migrated? | Reason |
|---|---|---|
| `ConnectionShaper::beatsNow` / `ConnectionEngine::evaluate` (`src/connect/*`) | **YES** (task 1) | The bug: pre-fix fold, blast radius = every LFO/Envelope(Beats) connection. |
| `EnvelopeSignal::getValue` (`src/signal/EnvelopeSignal.h`) | **YES** (task 2) | The bug: comment claimed the fix was already there; it wasn't. |
| `OscillatorSignal::getValue` (`src/signal/OscillatorSignal.h`) | Already done | Lane B's own prior work — read, not touched, this session. |
| `AnalysisThread.cpp:179-180` (publishes `barCount_`/`totalBarCount()` into the snapshot) | N/A — source of truth | Already publishes both (Lane B). Not touched. |
| `ApiServer::handleGetBpm`/`handleInjectFeatures`, `TestServer`'s injection builder | **YES, additive** (task 3) | `totalBarCount` added alongside; `barCount` untouched everywhere. |
| `MappingEngine.cpp:87` — `case MappingSource::BarCount: return snap.barCount;` | **NO — correctly stays** | `MappingSource::BarCount` (`MappingTypes.h:39`) is a deliberate, NAMED, user-facing mapping source exposing "bars since last phrase reset" as a modulation input. Migrating it would silently change what a saved preset's "BarCount" mapping source means. |
| `Autopilot.cpp:56-57` → `Layer::processPendingTrigger(beatInBar, barCount)` (`src/model/Layer.h:279`) | **NO — correctly stays** | `BeatSnapMode::TwoBar`/`FourBar` do `barCount % 2` / `% 4` to quantize clip-launch triggers to phrase-relative bar boundaries — explicitly named in the packet's HARD CONSTRAINTS as a component that legitimately wants the resettable counter. Single caller confirmed via grep; it passes `snapshot.barCount`, not touched. |
| `TopBar.cpp:234,423` — `displaySnap_.barCount`, "Bar N" display | **NO — correctly stays** | UI shows bars-since-phrase-reset, the musically meaningful number for a performer to read off the display; explicitly named in the packet's HARD CONSTRAINTS. |
| `BPMTracker.h`/`.cpp` — `barCount_`/`barCount()` | N/A — source of truth | Untouched; `totalBarCount_` already exists alongside it (Lane B). |
| `FeatureSnapshot.h` — `barCount`/`totalBarCount` fields | N/A — source of truth | Untouched; both fields already exist (Lane B). No `sizeof` risk since I added no new field here. |

No consumer was found that I judged ambiguous; the three that stay
resettable (MappingSource::BarCount, Autopilot's modulo-N snap, TopBar's
display) all match the packet's own HARD CONSTRAINTS list.

## HOW VERIFIED (exact commands, before/after ctest counts)

```
cd ~/projects/RealTimeAudio
cmake -S . -B build-b2 -DCMAKE_BUILD_TYPE=Release   # configured clean
cmake --build build-b2 --config Release -j6         # exit 0, 0 "error:" matches
cd build-b2 && ctest                                # 301/301, 0 failed
```

- Baseline per packet: 296/296.
- My own build-b2, before my test-file edits would have compiled: N/A (I
  built after all edits were in place — the packet's baseline was already
  independently measured by the team-lead's dispatch, and I re-ran the
  full suite from a from-scratch `cmake -S . -B build-b2` configure, never
  inheriting `build/`'s cache or count).
- After all edits: **301/301 pass, 0 failed.** (+5 net across the whole
  concurrently-edited tree — my own contribution is +1 new TEST_CASE in
  `test_bpm_stabilization.cpp`; the 4 sibling assertion blocks in
  `test_oscillator_bar_fold.cpp` were added inside EXISTING TEST_CASE
  bodies, so they add assertions, not new ctest entries.)
- One iteration needed: my first pass at the 4th oscillator sibling block
  ("Phrase-reset trade-off is pinned") picked `totalBarCount` 5→6 to mirror
  the legacy test's `barCount` 5→0 numbers, but `beatDuration_=12` wraps
  its 3-bar cycle at `totalBarCount` 0,3,6,9..., and 5→6 crosses one — a
  legitimate sawtooth 1→0 wrap, not a bug, which my `REQUIRE(after >
  before)` assertion wrongly flagged as a failure (caught by ctest:
  `0.208333254f > 0.875f` failed). Fixed by picking 4→5 instead (same
  cycle, no wrap) — exactly the trap the file's own "S168" test header
  comment warns about, which I stepped in anyway. Rebuilt, reran: 301/301.

## HOW I PROVED THE NEW TEST IS NOT A TAUTOLOGY

Recorded `shasum -a 256 src/analysis/BPMTracker.cpp` before touching it:
`0b22cd202845b4716d009467fa07d4a7d6ba12fdfffe14f1af0e45f99c49afb1`.

Applied the exact mutation named in the packet's success criteria — moved
`++totalBarCount_` out of the `if (newBar) { ++barCount_; ... }` branch
(where it correctly lives) into the `if (resetTransition &&
!predictedBeatRegime_) { barCount_ = 0; ... }` branch, so `totalBarCount_`
would only advance when a structural reset ALSO fires, instead of on every
normal bar. Rebuilt only `test_bpm_stabilization`, ran the new test:

```
REQUIRE( totalBarCountBeforeTransition > 0 )
with expansion:
  0 > 0
FAILED
```

It failed exactly as predicted, and specifically at the Phase-A assertion
(`totalBarCount() > 0` after 24 real onsets with NO structural transition
yet) rather than only at the transition-specific assertion — this is the
non-tautology proof the packet asked for: a naive test that only compared
before/after the transition hop could be fooled by a mutation that
happens to still increment `totalBarCount_` on that SPECIFIC hop (in this
scripted sequence the reset transition coincides with a `newBar` edge), but
checking growth across the whole build-up phase, independent of any
reset, cannot be fooled that way.

Reverted the mutation with the exact inverse edit, rebuilt, reran full
`ctest`: 301/301 again. Re-ran `shasum -a 256` on `BPMTracker.cpp`:
identical hash to before the mutation — byte-for-byte restored (per Iron
Law #7, since the mutation target was a file I did not author this
session).

## WHAT IS NOT VERIFIED

- Live HTTP round-trip of `totalBarCount` through `/api/bpm` and
  `/api/inject_features` — not run; app not launched (DO NOT LAUNCH per
  packet). Code review of the JSON glue is the only verification here.
- `ConnSerialization`'s new `resetPhaseOnStructural` round-trip has no
  dedicated new test (existing `"ConnSerialization: round-trips a real
  connection..."` test still passes since it never sets the field, so it
  exercises the default-false path implicitly but doesn't explicitly
  assert `true` survives a round-trip).
- No UI exposure of any of the three `resetPhaseOnStructural` switches —
  matches existing precedent, not part of this packet.

## ASSUMPTIONS

- `ConnSerialization.cpp` was not in the packet's named target files; I
  added serialization for the new `ConnShape::resetPhaseOnStructural`
  field anyway (additive-only) because every sibling `ConnShape` field is
  explicitly round-tripped and an unserialized field would be a silent,
  surprising asymmetry in a struct whose entire purpose is persistence.
  Flagged for reviewer judgment — happy to revert if this is considered
  out of scope.
- The 3 pre-existing `EnvelopeSignal`-only test cases in
  `test_oscillator_bar_fold.cpp` (lines ~299, ~326, ~356 pre-edit) needed
  `env.setResetPhaseOnStructural(true)` added to keep passing once
  `EnvelopeSignal` gained the switch — this was a necessary consequence of
  task #2, not something I chose to expand. I did NOT give them their own
  totalBarCount sibling assertions, since the packet's item 4b named only
  "the four" (OscillatorSignal) cases for that treatment.
- `test_connection.cpp`'s pre-existing `beatsNow` call sites and several
  Lfo/Envelope-Beats test snapshots needed `totalBarCount` set alongside
  `barCount` (or the new signature's extra args) to keep passing under the
  new default — necessary maintenance from the signature/default change,
  not new test coverage of my own choosing.
- `totalBarCount` JSON clamp range: chose `[0, INT_MAX]` matching
  juce::var's 32-bit signed int property, rather than `barCount`'s
  approach of clamping to its OWN real type's range (uint16_t) — since
  totalBarCount's real range (uint32_t) exceeds what juce::var's int
  property can carry.

## RISKS LEFT OPEN

- The 3 EnvelopeSignal-only tests now opted into legacy mode have no
  companion default-mode assertion, unlike all 4 OscillatorSignal cases —
  so EnvelopeSignal's new default (`totalBarCount`) path is currently
  guarded only by the "Live-app regression anchors" test (which happens to
  use `barCount=0` throughout, so it can't distinguish the two paths) and
  by code-parity with `OscillatorSignal` (identical fold logic, same
  pattern). If Harmony/reviewer wants EnvelopeSignal to have the same
  sibling-assertion coverage as Oscillator, that's a small, mechanical
  follow-up.
- `ConnSerialization.cpp` scope call (see NUANCE/ASSUMPTIONS) — reviewer
  should confirm this was wanted.
