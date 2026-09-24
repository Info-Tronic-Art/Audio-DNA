# s-rta-0923 — Recorder-core carried review fixes: PLAN (Architect, secondary lane)

INTENDED PATH: /Users/boriskarpman/projects/RealTimeAudio/.harmony/specs/s-rta-0923-review-fixes-plan.md
(the architect's PreToolUse hook refused that path — read-only agents may write only memory/.reports/,
memory/wip/ or a tmp scratch path; Harmony copies this file there, A54.)

Date: 2026-09-23. Repo: /Users/boriskarpman/projects/RealTimeAudio @ HEAD f5ae847 (recorder files last
touched at a50788b — nothing under src/recording/, src/ui/RecordPanel.*, tests/test_take.cpp or
tests/test_oscillator_bar_fold.cpp has changed since s168, `git log` verified).
Scope: review fixes (a), (b), (d) + HANDOFF post-close addenda 4 and 5.
OUT of scope (another architect owns them — do NOT build here): Ruling-28 audio-store format change,
review fix (c) RecorderClock periodic tempo anchor, Player backwards-seek guard (addendum 2).

## QUESTION
Plan the remaining carried recorder-core fixes so a Builder can execute each as an independent lane
with disjoint file ownership and a test that fails on the pre-change code.

## APPROACH (stated first)
Seven lanes. L0 is a 20-minute test-scaffolding lane that owns `tests/CMakeLists.txt` and creates one
empty test TU per fix, so every later lane owns exactly its source files plus its own test file and
nobody touches the shared `tests/test_take.cpp`. The five fix lanes are small (6–40 lines of source
each) and the design choice in every one is the *minimal honest* option:
- (a) the v1 bridge stops inventing data: `play/pause/stop` convert faithfully (v=0, no scaling);
  `speed/reverse` are dropped AND counted in `LoadStats` — never emitted as an `audio` point.
- (b) `Program::compile` reads `Gesture::stamps[i]` through the `pickAt` lambda that already exists
  for discrete points; the tempo map becomes a *reported* fallback only.
- (d) all five RecordPanel buttons disabled with whole-word "coming" tooltips; status label honest.
- addendum 4: `touch()` closes an open gesture exactly (shared helper with `release()`/`stop()`)
  instead of discarding it; `Player::setOverride` returns `[[nodiscard]] bool` and refuses `Latch`.
- addendum 5: three default-mode (`totalBarCount`) sibling blocks for the EnvelopeSignal cases,
  mutation-checked against a reverted `EnvelopeSignal.h`.

## VERIFICATION OF EVERY CLAIM AGAINST CURRENT SOURCE (all read this session)

| # | Claim (from review / handoff) | Status today | Evidence |
|---|---|---|---|
| a | v1 `TransportChange` resolves as Comp scope with value*1000 | **LIVE, VERIFIED** | `src/recording/Take.cpp:374-379` sets `Scope::Comp`, `control="audio"`, `p.v = value*1000`; `src/recording/Program.cpp:98-99` returns `ExactMatch` for every Comp key before reading any other field. No fixture covers it: `tests/fixtures/take_v1.json` has only type 1/2 events. |
| a′ | "no real v1 file has a TransportChange" | **VERIFIED** | `git grep recordTransportChange 3736f02^ -- src/` → zero callers outside SessionRecorder itself (matches spec G4). The v1 event shape is `{"t","type":4,"action","value"}` (`git show 3736f02^:src/recording/SessionRecorder.cpp:194-195`); actions were play/pause/stop/speed/reverse (`SessionRecorder.h:52`). |
| a″ | v2 `audio` vocabulary exists in code | **VERIFIED** | `src/recording/PerfState.h:74` `audioAction` = "play" \| "pause" \| "stop"; spec D3 (`.harmony/specs/s167-performance-log-and-routines.md:183`) "audio (comp; play/pause/stop/seek)". No `applyAudioTransport`/`Sink` implementation exists yet in src/ (grep). |
| b | compile discards exact stamps | **LIVE, VERIFIED** | `Program.cpp:174-183` `convertBeatX`; `:226-231` loop reads only `bp.x`; zero `.stamps` references in Program.cpp/Player.cpp. `pickAt(const Stamp&, double beat)` at `:158-167` already does the right per-clock pick and is used only for discrete points (`:208`). |
| b′ | worse than reported: single-anchor Sample drive collapses | **VERIFIED** | `src/recording/TempoMap.cpp:90-107`: with ONE anchor both rate branches are skipped → `rate = 0` → `sampleAt(t) == anchor.sample` for every t. `RecorderClock.cpp:16` writes exactly one anchor for a steady-tempo take (anchors only at start/unmetered/lock/reset/bpm>0.05, `:16,28,39,56,64`). So today a Sample-clock compile of a steady take gives every continuous breakpoint x = 0. |
| d | RecordPanel Save/Load/Play inert and silent | **LIVE, VERIFIED** | `src/ui/RecordPanel.cpp:29-39`: Play calls `onPlayRecording` (never assigned: `grep onPlayRecording src/MainComponent.cpp` → 0 hits; only `BrowserPanel.h:59` instantiates the panel and wires nothing); Save/Load have no `onClick`; Record/Stop flip "Recording..."/"Stopped" (`:8-27`) with no recorder behind them. |
| 4a | `touch()` overwrites an open gesture | **LIVE, VERIFIED** | `src/recording/PerformanceRecorder.cpp:57` `openGestures_[key] = std::move(og)` unconditionally; no production caller of `PerformanceRecorder` exists outside comments (grep src/, excluding src/recording/). |
| 4b | `Override::Latch` accepted, ignored | **LIVE, VERIFIED** | `src/recording/Player.h:63` stores; `:84` member; `Player.cpp` never reads `override_` (grep). Spec D8 (`s167 spec:450`) defines LATCH as "hold until the lane's next gesture begins / re-enable" and lists it LATER (`:765`). |
| 5 | EnvelopeSignal cases have no default-mode siblings | **LIVE, VERIFIED** | `tests/test_oscillator_bar_fold.cpp:306-334, 336-366, 368-387` each `setResetPhaseOnStructural(true)` with no `totalBarCount` sibling; header comment `:38-42` states this. `src/signal/EnvelopeSignal.h:51-53` is the switch; getter at `:142`. |
| — | ctest baseline | **VERIFIED** | `ctest -N` in `build/` → "Total Tests: 306". |
| — | `CompileReport::invalid` is unused | **VERIFIED** | declared `Program.h:70`; zero writers/readers in src/ or tests/ (grep). Reused by L2 as the stamps-fallback bucket. |
| — | `report.unknown` is never written | **VERIFIED** (adjacent gap, not planned) | grep `unknown[` in Program.cpp → 0; `Program.cpp:189-190` skips Opaque lanes with no report entry. |

Pinned sha256 of the files this plan changes (pre-change, for mutation-check restores):
```
5a36ef13798067f03af3101bac53416095762ce0fb07e738c5469fbfb88564e3  src/signal/EnvelopeSignal.h   (L6 mutation target — NOT owned, must be restored)
92885fd661aafa61e3f391ff0a4f9bc62686631b464c1abd441b69fd879795a3  src/recording/Take.cpp
c8e201813998e15b5a2febdc7147c3f54d7fc1cded5e187fb143ec5f3ef33964  src/recording/Program.cpp
fcff7b7b9e734a0373ffbfedefb97fe22a9e94b6a112bb114b91f5e76efd2cba  src/recording/PerformanceRecorder.cpp
fe4e5c34e889d3a1d081e5e4b394f95812e62bb1495419dd4b94dcf6c2b52b72  src/recording/Player.cpp
7e9765b796c64a1592eeda6c85ee11c37422017c48e710c72a1f71e92b511370  src/ui/RecordPanel.cpp
eb4f092c811af1c134967f1811c4c3ad157b7488efd41d9b1a5cefa5ca2abe7f  tests/test_oscillator_bar_fold.cpp
```

## TRADEOFFS CONSIDERED

**(a) where to fix — bridge vs compiler.**
- Reviewer option "fix `resolveKey` so bogus Comp controls aren't blanket-ExactMatch" — REJECTED. The
  control vocabulary is free-form BY DESIGN (`src/model/ControlPath.h:41-44`: "only the dispatcher
  needs to recognise it"); the future fixture's `quantumFlux` control at Layer scope resolves
  ExactMatch by the same design. A Comp whitelist in Program.cpp would be a second vocabulary to
  maintain and would collide with lane L2's file. The defect is the bridge emitting a KNOWN control
  (`audio`) with WRONG data — fix the bridge.
- "Drop every v1 TransportChange" (3 lines) — REJECTED, narrowly. D12 rule 5 (spec `:712-718`) says
  best-effort conversion; `play/pause/stop` have an exact v2 equivalent already pinned in code
  (`PerfState.h:74`) and carry no value in v1 (`recordTransportChange(action, value = 0.0f)`), so a
  faithful conversion costs 6 lines and no semantic guess. `speed`/`reverse` (the audio player's
  rate/direction) have no v2 control — D3's `speed` is a per-clip continuous control, not the same
  thing — so those are dropped and COUNTED (D12: never silently).
- Route TransportChange through `Scope::Clip` with deck=-1 like ParameterChange — REJECTED: makes a
  Comp-level event report "deck not found", a lie in the report.

**(b) exact stamps.** Only one design: use `stamps[i]`; keep the map as fallback. The question was
whether the fallback should be silent — NO: it goes into the already-declared, never-used
`CompileReport::invalid` bucket, one Issue per lane with the gesture count. Refusing to compile a
stamp-less gesture was considered and rejected (a hand-built or future-edited take must still play;
D12 "never half-refused silently" is satisfied by the report entry).

**(d) how honest.** Disable-with-tooltip (D14 `:755-757` pattern) vs hiding the tab vs wiring a
"save what exists" path. Hiding contradicts D14 ("disabled with a tooltip, not hidden"); wiring
anything is step 3/4 (spec build-order rows 3-4) and would pull `PerformanceRecorder` into UI before
its capture sites exist. Record/Stop are included beyond the dispatch's literal "Save/Load/Play"
because the review's required fix #2 (`.harmony/.reports/s168-review-lane-a.md:196-201`) names
them as the minimum: "Recording..." in red with nothing captured is the exact lying control. The
D14 "Render… (coming)" combo rename is step 4 work and is NOT included (surgical scope).

**(4a) double-touch: close-and-reopen vs idempotent no-op.** Idempotent would silently swallow a
grip change (held → decaying) — the one legitimate reason for a re-touch. Close-and-reopen keeps
every captured breakpoint, records the grip change truthfully as two adjacent gestures, and reuses
the exact-end logic `stop()` already has. No production caller exists to be surprised.

**(4b) Latch: implement vs refuse vs delete the enum value.** Implementing is NOT minimal: under the
current `Sink` contract the row-1 "Touch" (`Player.cpp:80-86`, displaced until the gesture ends)
is already LATCH-shaped per D8's table, and D8's real TOUCH ("glide back when the hand lets go")
needs the grip engine (step 3) — the two modes are not distinguishable inside Player today.
Deleting `Latch` from the enum is the loudest refusal but changes the API shape the spec declares
(`s167 spec:828`). Runtime refusal with a `[[nodiscard]] bool` + log line is loud at compile time
(unused-result warning) AND at runtime, identical in Debug and Release. No `jassert` — a legitimate
test must be able to exercise the refusal without assertion noise.

## DECISION / SPEC — LANES

Conventions for EVERY lane: build in your own directory (`cmake -B build-<lane> -DCMAKE_BUILD_TYPE=Release`
— RIG FACT, two builders in one `build/` produce plausible-but-wrong results); run only your target(s)
plus a final `ctest --test-dir build-<lane>`; new files under `.harmony/` need `git add -f` and
every commit is verified with `git show --stat HEAD` (RIG FACT: `git add` silently refuses new
`.harmony/` paths and still exits 0). Mutation checks restore the file byte-identical and prove it
with `shasum -a 256` against the pins above. UI text: whole words, no abbreviations.

### L0 — test scaffolding (owns tests/CMakeLists.txt) — ~20 min
Files owned: `tests/CMakeLists.txt`; NEW empty TUs `tests/test_take_v1_transport.cpp`,
`tests/test_program_stamps.cpp`, `tests/test_recorder_double_touch.cpp`, `tests/test_player_override.cpp`;
conditionally NEW `tests/test_record_panel.cpp`. Ownership of each stub transfers to its lane the
moment L0 lands.
1. Append the four stub `.cpp` files to the `add_executable(test_take ...)` source list
   (`tests/CMakeLists.txt:274-288`). They inherit test_take's link set, `TEST_FIXTURES_DIR`, warnings.
   Each stub = a 3-line header comment naming its lane + `#include <catch2/catch_test_macros.hpp>`
   (a Catch2 TU with zero cases is legal; count stays 306).
2. GUI-headless spike (time-box 20 min, in the scratchpad dir, NOT in the repo): a Catch2 TU with
   `#include <juce_gui_basics/juce_gui_basics.h>`, `juce::ScopedJuceInitialiser_GUI init;`
   `juce::TextButton b("x"); b.setEnabled(false); REQUIRE_FALSE(b.isEnabled());`, linked against
   `juce::juce_gui_basics` in a throwaway build dir, run via `ctest --timeout 60`.
   PASS (exit 0, no hang) → add target `test_record_panel`: sources `test_record_panel.cpp` +
   `${SRC_DIR}/ui/RecordPanel.cpp`; links `Catch2::Catch2WithMain juce::juce_core juce::juce_events
   juce::juce_graphics juce::juce_gui_basics`; same compile definitions/warning flags as
   `test_thumbnail_cache` (`tests/CMakeLists.txt:476-490`); `apply_sanitizers`; `catch_discover_tests`;
   stub TU as in step 1. `AudioDNALookAndFeel` constants are `static constexpr` (`src/ui/LookAndFeel.h:16,19`)
   so `LookAndFeel.cpp` need not be linked.
   FAIL → do not add the target; leave a 3-line comment beside the `test_thumbnail_cache` note
   recording the spike result and date; tell Harmony so L5 uses its fallback acceptance.
Acceptance: `cmake --build build-l0` exits 0; `ctest --test-dir build-l0 -N` reports exactly 306.

### L1 — review fix (a): v1 TransportChange bridge — ~45 min
Files owned: `src/recording/Take.cpp`, `src/recording/Take.h`, NEW `tests/fixtures/take_v1_transport.json`,
`tests/test_take_v1_transport.cpp` (from L0).
**FILE CONFLICT:** `Take.h`/`Take.cpp` are on the Ruling-28 audio-store architect's touch list.
Serialize: run this lane strictly before or after that lane, never concurrently; rebase on it.
Source changes:
- `Take.h` `LoadStats`: add `std::map<std::string, int> v1Dropped;` with comment "v1 events with no
  v2 equivalent, counted and named (D12: never silently dropped); key `<EventType>:<detail>`".
  Update the private `fromV1Var` comment (`:117-122`): TransportChange play/pause/stop faithful,
  speed/reverse dropped+counted.
- `Take.cpp` `fromV1Var`: move the seq mint below the switch (`p.s.seq = take.nextSeq++;` after the
  switch, `p.s = { 0, t, 0 }` before it) so a dropped event mints no seq. Replace the
  TransportChange case with:
  ```cpp
  case V1Type::TransportChange:
  {
      // Faithful (D12 rule 5): v2's `audio` control is action-valued play/pause/stop
      // (PerfState::audioAction, spec D3) and those carry no value -- the legacy
      // value*1000 scaling produced an int the v2 dispatcher would misread.
      // v1 "speed"/"reverse" (the audio player's rate/direction) have no v2 control:
      // counted in stats.v1Dropped, never emitted as an `audio` point.
      const auto action = e->getProperty("action").toString().toStdString();
      if (action != "play" && action != "pause" && action != "stop")
      {
          stats.v1Dropped["TransportChange:" + (action.empty() ? std::string("<empty>") : action)]++;
          continue;
      }
      key.scope = ControlPath::Scope::Comp;
      key.control = "audio";
      p.action = action;
      p.v = 0;
      break;
  }
  ```
Fixture `tests/fixtures/take_v1_transport.json` (exact):
```json
{
    "version": 1,
    "events": [
        { "t": 0.5, "type": 1, "layer": 0, "column": 2 },
        { "t": 1.0, "type": 4, "action": "play",    "value": 0.0 },
        { "t": 2.5, "type": 4, "action": "speed",   "value": 1.5 },
        { "t": 3.0, "type": 4, "action": "pause",   "value": 0.0 },
        { "t": 3.5, "type": 4, "action": "reverse", "value": 1.0 },
        { "t": 4.0, "type": 4, "action": "stop",    "value": 0.0 }
    ]
}
```
Tests (2 TEST_CASEs, tags `[take][v1][transport]`):
1. "v1 TransportChange bridges play/pause/stop faithfully and counts speed/reverse as dropped":
   load fixture via `Take::load(juce::File(TEST_FIXTURES_DIR).getChildFile(...))`; `stats.wasV1`;
   Comp/"audio" lane has exactly 3 points, actions play/pause/stop at t 1.0/3.0/4.0, `v == 0` each;
   `stats.v1Dropped == {{"TransportChange:speed",1},{"TransportChange:reverse",1}}`;
   `take->nextSeq == 5` (four kept events, contiguous seqs 1..4).
2. "compile of a bridged v1 take dispatches only faithful audio points": `Composition comp;
   comp.initDefault();` (activeDeckIndex 0, `Composition.h:156`); `compile(*take, comp,
   DriveClock::Wall)`; `discrete.size() == 4`; every Fired with `key.control == "audio"` has
   `p.v == 0` and action in {play,pause,stop}; no Fired has action speed/reverse;
   `report.unresolved.empty()`; `report.resolvedCount == 1` (the audio lane) and
   `report.reboundByPosition.size() == 1` (the v1 activeClip lane is deck-relative with an empty
   layerName → PositionOnly, `Program.cpp:50-51`).
Pre-change: the TU does not compile (no `v1Dropped`) — so run the behavioural mutation check:
keep the Take.h field, revert only the Take.cpp hunk → test 1 fails (5 audio points, v 0/1500/0/1000/0)
and test 2 fails (`discrete.size() == 6`, a "speed" Fired present); restore; sha256.
Acceptance: `test_take` green in `build-l1`; ctest total 308 (306 + 2); mutation evidence in report.

### L2 — review fix (b): exact per-breakpoint stamps in Program::compile — ~45 min
Files owned: `src/recording/Program.cpp`, `src/recording/Program.h` (comments only),
`tests/test_program_stamps.cpp` (from L0). No external conflict (Program.* is not on the other
architect's list).
Source change (`Program.cpp:220-243` continuous branch):
```cpp
int stampMismatches = 0;
for (const auto& g : lane.gestures)
{
    if (g.curve.pts.empty()) continue;
    // D1/D7: every breakpoint carries its own exact {t,sample} reading
    // (Lane.h Gesture::stamps, size == pts.size(), enforced by
    // PerformanceRecorder::set/release). Use it. The tempo map is a
    // FALLBACK for a gesture with no parallel stamps (hand-built, or an
    // edited take whose editor dropped them) -- reported, never silent.
    const bool exact = g.stamps.size() == g.curve.pts.size();
    if (!exact) ++stampMismatches;

    ContLane::G cg;
    cg.grip = g.grip;
    for (size_t i = 0; i < g.curve.pts.size(); ++i)
    {
        Breakpoint converted = g.curve.pts[i];
        converted.x = exact ? pickAt(g.stamps[i], g.curve.pts[i].x)
                            : convertBeatX(g.curve.pts[i].x);
        cg.curve.pts.push_back(converted);
    }
    /* x0/x1/range/target push unchanged */
}
if (stampMismatches > 0)
    program->report.invalid.push_back({ key, std::to_string(stampMismatches)
        + " gesture(s) without parallel stamps: x reconstructed from the tempo map (inexact)" });
```
`pickAt` (`:158-167`) is reused unchanged — Wall→`s.t`, Beat→`beat`, Sample→`s.sample`. Update the
comment at `:169-173` and `Program.h:87-93` (x comes from each breakpoint's own stamp; the map is the
reported fallback) and `Program.h:70` (`invalid`: gestures whose x had to be reconstructed).
Tests (2 TEST_CASEs, tags `[program][compile][stamps]`), both against `Composition comp; comp.initDefault();`
and a lane key {Layer scope, deck 0 "Deck 1", layer 0 "Layer 1", control "scalar", scalar "opacity"}
(ExactMatch). Tempo map = exactly what RecorderClock writes for a steady take:
`take.tempo.a = {{0.0, 0.0, 0, 120.0f, "start"}}`. Gesture: `curve.pts x(beat) = {0.0, 4.0, 8.0}`,
y = {0, 0.5, 1}; `stamps = {{1, 0.0, 0}, {2, 2.03, 97440}, {3, 4.07, 195360}}` (the live clock
drifted vs the 120-BPM map, where tAt(4)=2.0, tAt(8)=4.0).
1. "compile takes each breakpoint's x from its own stamp, not the tempo map": Wall → x ==
   {0.0, 2.03, 4.07} (Approx 1e-9), `x0 == 0.0`, `x1 == 4.07`, `program->length == 4.07`;
   Sample → x == {0, 97440, 195360}; Beat → x == {0, 4, 8}; `report.invalid.empty()` in all three.
   Pre-change: Wall gives {0, 2.0, 4.0}; Sample gives {0, 0, 0} (TempoMap.cpp:90-107 single-anchor
   rate 0) — fails on both.
2. "compile falls back to the tempo map and reports it when a gesture has no parallel stamps":
   same take with `stamps = {}`; Wall → x == {0.0, 2.0, 4.0}; `report.invalid.size() == 1`, its
   `key == laneKey`, reason contains "stamps". Pre-change: `invalid` is empty — fails.
Acceptance: `test_take` green in `build-l2`; ctest total 308 (306 + 2); mutation check = revert the
Program.cpp hunk → both fail; restore; sha256. Also confirm no existing test relied on map
reconstruction: `tests/test_take.cpp` never compiles a continuous lane (its Player cases build
Programs by hand; its compile case is discrete-only, `:334-378`) — VERIFIED by reading.

### L3 — addendum 4a: PerformanceRecorder::touch() must not discard an open gesture — ~30 min
Files owned: `src/recording/PerformanceRecorder.cpp`, `src/recording/PerformanceRecorder.h`,
`tests/test_recorder_double_touch.cpp` (from L0). No external conflict.
Source change:
- `.h` private: `void finishGesture(const ControlPath& key, OpenGesture& og, const ClockStamp& stamp);`
  — appends the exact end breakpoint (`pts.back().y` at `stamp.beat`, stamp `{nextSeq++, stamp.t,
  stamp.sample}`) if `pts` is non-empty, then moves `og.g` into `take_.lanes[key]` (kind Continuous).
  Does NOT erase from `openGestures_` — callers do.
- `release()` (`:113-129`) → `finishGesture(key, it->second, clock_->now()); openGestures_.erase(it);`
- `stop()` (`:139-152`) → per entry `finishGesture(key, og, stamp)` (keep the
  `clock_ != nullptr ? clock_->now() : ClockStamp{}` guard); then `openGestures_.clear()`.
- `touch()` (`:44-58`): after `const auto stamp = clock_->now();` insert
  `if (auto it = openGestures_.find(key); it != openGestures_.end()) { finishGesture(key, it->second, stamp); openGestures_.erase(it); }`
  and update the `.h` comment on `touch()`: "A second touch() on a key whose gesture is still open
  closes that gesture EXACTLY at the current stamp (as release() would) before opening the new one —
  captured breakpoints are never discarded; a grip change (held → decaying) records as two adjacent
  gestures."
Test (1 TEST_CASE, tag `[performancerecorder][doubletouch]`), scripted clock as in
`tests/test_take.cpp:289-330` (seed tick, `makeSnap(120,0)`, wall/samples advanced by hand):
touch(k,"held") @0.0; set 0.1 @0.1; set 0.5 @0.3 (both outside the 50 ms window); touch(k,"decaying")
@0.4 with NO release; set 0.9 @0.5; release @0.7; `stop()`. Assert `lanes[k].gestures.size() == 2`;
g0: grip "held", `pts.size() == 3`, `pts.back().y == 0.5f`, `stamps.size() == 3`,
`stamps.back().t == Approx(0.4)`; g1: grip "decaying", `pts.size() == 2` (0.9 + synthesized end),
`stamps.front().t == Approx(0.5)`; seqs strictly increasing across g0 then g1.
Pre-change: `gestures.size() == 1` (g0 discarded) — fails.
Acceptance: `test_take` green in `build-l3`; ctest 307; existing coalescing case (`test_take.cpp:289`)
still green (the refactor must not change release()/stop() output — that case pins it).

### L4 — addendum 4b: Player::Override::Latch refuses loudly — ~20 min
Files owned: `src/recording/Player.h`, `src/recording/Player.cpp`, `tests/test_player_override.cpp` (from L0).
**FILE CONFLICT:** `Player.h`/`Player.cpp` are also the backwards-seek architect's files.
Recommended: hand this ~15-line diff to that lane's builder as a rider; otherwise serialize.
Source change:
- `Player.h:59-63` → replace with
  ```cpp
  // D8's LATCH ("your value holds until the lane's next gesture / re-enable") is
  // LATER (D14). Requesting it today is REFUSED, not silently mapped to Touch:
  // returns false, logs, leaves the mode unchanged. Touch (D8's default) is
  // what row 1 implements and tests.
  [[nodiscard]] bool setOverride(Override o);
  Override overrideMode() const { return override_; }
  ```
  (`overrideMode`, not `override` — the latter is a contextual keyword.)
- `Player.cpp`: `bool Player::setOverride(Override o) { if (o != Override::Touch) {
  juce::Logger::writeToLog("Player::setOverride: Latch override is not implemented (spec D8, LATER); staying in Touch");
  return false; } override_ = o; return true; }`
Test (1 TEST_CASE, tag `[player][override]`): `Player player(std::make_shared<Program>());`
`REQUIRE(player.overrideMode() == Player::Override::Touch); REQUIRE_FALSE(player.setOverride(Player::Override::Latch));
REQUIRE(player.overrideMode() == Player::Override::Touch); REQUIRE(player.setOverride(Player::Override::Touch));`
Pre-change: does not compile (void return, no getter) — a contract test; behavioural mutation check =
make `setOverride` store unconditionally and return true → fails; restore; sha256.
Acceptance: `test_take` green in `build-l4`; ctest 307.

### L5 — review fix (d): RecordPanel honest interim — ~30 min (+30 if the GUI test target exists)
Files owned: `src/ui/RecordPanel.cpp`, `src/ui/RecordPanel.h`, `tests/test_record_panel.cpp` (only if
L0 shipped the target). No external conflict.
Source change (exact strings; whole words, no abbreviations):
- In the constructor, for each button call `setComponentID(...)`, `setEnabled(false)`, `setTooltip(...)`:
  - recordBtn_: id "record", tooltip "Recording is coming in a later build. Nothing is captured yet."
  - stopBtn_:   id "stop",   tooltip "Recording is coming in a later build."
  - playBtn_:   id "play",   tooltip "Playback of a recorded performance is coming in a later build."
  - saveBtn_:   id "save",   tooltip "Saving a recorded performance is coming in a later build."
  - loadBtn_:   id "load",   tooltip "Loading a recorded performance is coming in a later build."
- `statusLabel_` initial text: "Performance recorder coming in a later build" (replaces "Ready", which
  is also a lie today). Leave the Record/Stop lambdas in place (unreachable while disabled; step 4
  rewrites the panel and removes the G25 shadow bool) — do not extend scope into step 4.
- Replace the comments at `RecordPanel.h:5-9` and `RecordPanel.cpp:35-37`: "disabled with tooltips
  until spec steps 3/4 wire it; nothing is captured or played; MainComponent's TooltipWindow
  (`MainComponent.cpp:471`) shows the tooltips."
- NOT included: the D14 "Render… (coming)" combo rename and the format selector — step 4 (spec row 4).
Test (1 TEST_CASE, tag `[ui][recordpanel]`, only if the target exists): `juce::ScopedJuceInitialiser_GUI init;
RecordPanel panel;` for id in {record, stop, play, save, load}: `auto* c = panel.findChildWithID(id);
REQUIRE(c); REQUIRE_FALSE(c->isEnabled()); auto* tc = dynamic_cast<juce::SettableTooltipClient*>(c);
REQUIRE(tc); REQUIRE(tc->getTooltip().isNotEmpty());`. Pre-change: `findChildWithID` returns null
(no ids) and play/save/load are enabled — fails.
Fallback acceptance (no GUI target): app target builds (`cmake --build build-l5 --target AudioDNA`);
`grep -n 'setTooltip' src/ui/RecordPanel.cpp` shows the five lines above; `grep -c 'setEnabled(false)'`
rose from 3 to at least 8; Boris (Tier 4) at the next launch: Browser → Record tab shows five grey
buttons and hovering shows the sentence. No screenshot claim without a launch.
Acceptance: as above; ctest 306 + (1 if target).

### L6 — addendum 5: EnvelopeSignal default-mode siblings — ~30 min
Files owned: `tests/test_oscillator_bar_fold.cpp` only. No dependencies; can run first.
Change: in each of the three EnvelopeSignal cases append a sibling block in the file's established
in-case pattern (the OscillatorSignal siblings at `:98-142`), using `EnvelopeSignal envDefault(...)`,
`REQUIRE_FALSE(envDefault.getResetPhaseOnStructural());`, and snapshots that set `totalBarCount`
(leaving `barCount` at 0):
- "completes a full cycle over 8 beats" (`:306-334`): start (tbc 0) → 0.0; peak (beatPhase 0,
  beatInBar 0, tbc 1) → 1.0 ± 0.01; nearEnd (0.99, 3, tbc 1) < 0.1.
- "16-beat option" (`:336-366`): start → 0.0; peak at tbc 2 → 1.0; nearEnd (0.99, 3, tbc 3) < 0.1.
- "default duration 4.0 unchanged" (`:368-387`): sweep tbc {0,1,2,5,10} → equal to baseline ± 0.0005.
  Comment that this sibling is a no-op guard, not a discriminator (at duration 4 either counter is a
  whole number of cycles).
- Correct the header comment `:38-42` (it currently states the siblings do not exist).
Mutation check (touches a file this lane does NOT own — restore is mandatory): change
`src/signal/EnvelopeSignal.h:51-53` locally to `float barsElapsed = static_cast<float>(snapshot.barCount);`,
rebuild `test_oscillator_bar_fold` → the 8-beat and 16-beat siblings FAIL (peak reads 0.0, barCount
is 0 in those snapshots), the duration-4 sibling still passes (by design), the legacy blocks still
pass; restore; `shasum -a 256 src/signal/EnvelopeSignal.h` == 5a36ef13… (pin above); `git status`
shows only the test file modified.
Acceptance: `test_oscillator_bar_fold` green; ctest count unchanged (306, siblings live inside
existing cases; still 9 TEST_CASEs in the file); mutation evidence in the report.

## ORDER OF EXECUTION
- Wave 1 (parallel): L0, L6.
- Wave 2 (parallel, after L0): L2, L3, L5.
- Wave 3 (serialized against the OTHER architect's lanes, never concurrent with them): L1 (Take.*),
  L4 (Player.* — preferably folded into the backwards-seek lane as a rider).
- Final gate (Harmony, not a builder): ONE clean forced rebuild in ONE directory with every lane
  merged; expected `ctest` = 306 + 2 (L1) + 2 (L2) + 1 (L3) + 1 (L4) + (1 if `test_record_panel`
  shipped) = **312, or 313** — RUN it, never inherit it. Then `git show --stat HEAD` per commit.

## RISKS (and the strongest counterargument to this plan)
1. **File conflicts with the Ruling-28 / backwards-seek architect** (L1: Take.h/.cpp; L4: Player.h/.cpp).
   Not a design risk, a scheduling one: serialize, rebase, or hand L4 over as a rider. If the audio-
   store lane changes `LoadStats`, L1's `v1Dropped` field must be re-merged by hand.
2. **GUI headless test feasibility is ASSUMED** (JUCE `ScopedJuceInitialiser_GUI` under ctest on this
   Mac). L0's spike decides it; L5 has a fallback so nothing blocks. Do not let the spike exceed 20 min.
3. **(a) presumes step 3's `audio` Fired shape** (action string, `v` unused for play/pause/stop). If
   step 3 later gives `v` a meaning for "play" (e.g. a start sample), 0 is the benign value. G4 proves
   no real v1 TransportChange data exists, so the practical exposure is nil either way.
4. **(b) does not fix tempo-map sparsity** — that is fix (c) (RecorderClock periodic anchor, the other
   architect). After (b) the map is no longer load-bearing for row-1 replay, only for later edit ops.
   Strongest counterargument: "then (c) is unnecessary" — no: edits (move/retime, D5) re-derive one
   domain from another and need a dense map; (b) removes today's silent hole, (c) keeps tomorrow's.
5. **(4a) changes recorded shape for a naive caller that calls touch() per MIDI message** (it would now
   get N adjacent gestures instead of one overwritten one). No such caller exists (step 3 is unbuilt);
   and N adjacent exact gestures lose nothing, whereas today's overwrite loses everything.
6. **L6's mutation check edits a file the lane does not own.** Restore + sha256 is mandatory; the
   lane report must quote the hash.
7. **Parallel builders in one `build/`** and **`git add` silently refusing new `.harmony/` paths** —
   both s168 RIG FACTS; repeated in the lane conventions above.
8. **Strongest counterargument to (a)'s faithful conversion:** "drop all TransportChange, 3 lines, zero
   semantic claims." It loses convertible data that D12 rule 5 says to convert, for a 3-line saving.
   If Harmony prefers the drop-all variant, only the `if` and the three assignments change; the
   fixture and both tests stay, with test 1's audio-lane expectation becoming "absent" and
   `v1Dropped` gaining play/pause/stop entries.

## ADJACENT GAPS FOUND, NOT PLANNED (surface to Harmony; each is a one-lane candidate)
- `Program::compile` skips Opaque lanes with NO report entry (`Program.cpp:189-190`) and nothing
  anywhere writes `CompileReport::unknown` — an unknown-kind lane is reported at load
  (`LoadStats.unknownKindLanes`) but is invisible at compile/play time.
- Spec D12 (`:717`) says "the compiler refuses beat/sample drive [of a wallOnly take] with a clear
  message"; `compile()` never reads `take.unknownFeatures`/"wallOnly" — a bridged v1 take compiled
  with `DriveClock::Beat` gets every point at beat 0 silently.
- `RecordPanel`'s Record/Stop lambdas and the G25 shadow bool remain (dead once disabled); step 4.

## BORIS
No product decision is needed for these five fixes; every choice above is technical and reversible.
D8's open feel call (after letting go, come back right away = Touch, or hold until the lane next moves
that control = Latch) remains his and remains deferred by design — L4 only makes the deferral audible.

REPORT_FILE: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/7e364303-6335-4445-955e-49321e60ded4/scratchpad/s-rta-0923-review-fixes-plan.md
STATUS: COMPLETE
