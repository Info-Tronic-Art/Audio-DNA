# Plan: downbeatDetected reader cadence (roadmap item 4) -- diagnosis correction, guard tests, REST witness, live probe

Author: Architect (Fable), s-rta-0925, 2026-09-25. Repo HEAD read: e501b00 (main).
Read-only on source; every codebase claim is `path:line`-cited and binned VERIFIED / INFERRED / ASSUMED.
Prior art read: `.harmony/.reports/s-rta-0924b/plan-onset-render.md`, `critic-onset-render.md`, commit 4532779,
`src/features/OnsetPulse.h`, CLAUDE.md Common Pitfall 30.

QUESTION: `downbeatDetected` was flagged (plan-onset-render.md section 6, CLAUDE.md Pitfall 30's last
sentence, this dispatch) as "the same one-hop pulse class as onsets": a render/UI reader of the
always-latest FeatureBus would LOSE downbeats at 60 fps and DUPLICATE them above ~93.75 fps. Apply the
OnsetPulse pattern: monotonic counter in FeatureSnapshot, per-reader-thread pulse, every consumer switched,
Catch2 cadence test, live probe.

VERDICT / APPROACH (stated first): **The premise is false, and the counter the task asks for already
exists.** `downbeatDetected` is a beat-long LEVEL, not a one-hop pulse: `BPMTracker` assigns
`downbeatDetected_ = (beatCounter_ == 0)` only at a beat event and never clears it per hop, so the flag
reads true for the WHOLE first beat of the bar (one beat period: 300 ms at 200 BPM .. 1 s at 60 BPM) and
false for beats 2-4. No 15-120 Hz reader can miss it, and re-reading a level is not duplication. "New
bar" is its rising edge, which `BPMTracker::updatePhrase` already edge-detects into `totalBarCount`
(S168: monotonic, never reset, published every hop, exposed on `/api/bpm`) -- that IS the OnsetPulse-class
counter for downbeats; `OnsetPulse` applied to it gives an exact one-pulse-per-bar at ANY cadence, which
matters only for readers slower than a beat (a sluggish REST poller), where a level consumer really does
lose bars. There is NO render-side consumer at all (no shader uniform, no signal, no autopilot, no
recorder marker, no REST field), and the only two UI reads are a dead write-only copy (`TopBar.cpp:232`,
the dispatch's "known reader") and the hidden v1 `AudioReadoutPanel`. The comments that misled three
successive readers ("true on the hop where beat 1 lands", `FeatureSnapshot.h:47` / `BPMTracker.h:219`)
are the actual defect. Deliverables that CLOSE the item with evidence instead of code churn:
(1) two deterministic Catch2 tests that pin the level semantics and model 60 Hz / 120 Hz / 0.9 s readers
against the real tracker (RED form = the task's premise, fails with the real numbers);
(2) `downbeatDetected` added to `/api/bpm` (same coherent snapshot as `totalBarCount`) so the level can
be witnessed live; (3) the misleading comments/docs corrected + a new Common Pitfall; (4) a live probe
(manual BPM, no audio content needed) whose oracle discriminates level from pulse and shows the counter
contract at a slow cadence; (5) a written 3-line recipe for any FUTURE consumer that wants a bar pulse.
No new FeatureSnapshot field, no analysis-thread change, no consumer rewired.

## 0. Facts established (VERIFIED unless marked)

### 0.1 `downbeatDetected` is a level -- every assignment site (VERIFIED, `src/analysis/BPMTracker.cpp`)
- `:241` `advancePredictedBeat()`: `downbeatDetected_ = (beatCounter_ == 0);` -- called only on a
  predicted phase wrap (`:219-222`, manual mode / silence / locked-but-no-raw-BPM: `:77-82`, `:92-94`,
  `:103-108`).
- `:337` `scoreBeat()` locked branch: same assignment, only on a hop where aubio flagged a beat
  (`:308-311`, gated `beatDetected_ && lockedBPM_ > 0 && !predictedBeatRegime_`).
- `:355` `scoreBeat()` not-yet-locked branch: `downbeatDetected_ = false;`
- `:436` `analyzeDownbeatPosition()` initial lock: same assignment (once, at lock).
- NOTHING per hop: `process()` `:46-61` / `runPipeline()` `:68-72` set only `rawBPM_/confidence_/
  beatDetected_` each hop; `updatePhase` `:191-223`, `updateBarPhase` `:441-457`, `resetBeatPhase`
  `:534-540`, `setManualBPM` `:522-532` never touch it. The declaration comment `BPMTracker.h:219`
  "true on the hop where beat 1 lands" is wrong; `beatDetected_` (`:72`) is the per-hop flag, not this.
- Proof the author knew it is a level: `updatePhrase()` `:471-472` does explicit rising-edge detection
  `bool newBar = downbeatDetected_ && !prevDownbeatDetected_;` -- pointless for a one-hop pulse -- and
  `:474-478` increments `barCount_` AND `totalBarCount_` exactly on that edge. `feedDownbeatFeatures`
  (`:295-318`) runs `updatePhrase` every hop (`:317`), called unconditionally per hop by
  `AnalysisThread.cpp:196-197`; `:204` publishes the level, `:206` the counter, in every snapshot.
- Consequence: at the tracker's BPM range (`BPMTracker.h:40-41`, 60-200 BPM) the level lasts 28-94 hops
  (300 ms-1 s). Readers: TopBar 15 Hz (`src/ui/TopBar.cpp:221`), AudioReadoutPanel 30 Hz
  (`src/ui/AudioReadoutPanel.cpp:12`), Renderer 60-118 fps (`.harmony/gotchas.md:235,:376`). The slowest
  is 4.5x faster than the shortest level: loss needs a > 300 ms stall, not a cadence.
- Existing test `tests/test_downbeat_detector.cpp:112-138` ("fires only on beat 0") samples the flag only
  right after each beat event, so it never distinguished level from pulse -- consistent with both.

### 0.2 The monotonic counter already exists: `totalBarCount` (VERIFIED)
`FeatureSnapshot.h:51-56` (uint32, "monotonic bars since transport start, NEVER rewound"),
`BPMTracker.h:112-115, :228-234`, incremented only at `BPMTracker.cpp:477` on the `newBar` edge; the three
places that reset `barCount_` leave it alone (`:464`, `:496`, `:518`). Published every hop
(`AnalysisThread.cpp:206`), exposed on `/api/bpm` (`src/api/ApiServer.cpp:612`), injectable in test mode
(`src/test/TestServer.cpp:468` -- defaults to 0 when absent, i.e. a backwards jump for any pulse
consumer). Already consumed as a never-backwards phase basis by `OscillatorSignal.h:54`,
`EnvelopeSignal.h:53`, `ConnectionShaper.cpp:17-21`. Its delta == number of downbeat rising edges since
the reader's previous look, by construction (`:471-477`). Relock corner (`:402-406`: `beatCounter_`
re-aligned without touching the level) can merge/skip one edge -- level and counter agree with each other
there, so no consumer can disagree with the tracker.

### 0.3 Exhaustive consumer inventory (`grep -rn downbeatDetected src/ tests/`; `grep -in downbeat`
in shaders, ui, signal, routing, model, recording, api, test, midi, osc, connect -- VERIFIED 2026-09-25)
| # | Site | Thread / cadence | What it does with the flag | Action |
|---|---|---|---|---|
| 1 | `src/ui/TopBar.cpp:232` `displaySnap_.downbeatDetected = snap.downbeatDetected;` (timer 15 Hz `:221`, `timerCallback` `:224-241`) | message | WRITE-ONLY: `paintBeatWheel` uses `beatInBar` `:369` and `beatPhase` `:389`; `paintBarPhraseDisplay` uses `barCount`/`phrasePhase` `:423,:432`; no other `downbeat` token in the file | dead copy; delete (4.4) |
| 2 | `src/ui/AudioReadoutPanel.cpp:43` same copy | message, 30 Hz | WRITE-ONLY (paint uses `beatInBar` `:346`, `i == 0` `:357`, `downbeatFlash_` `:366`) | dead copy; delete (4.4) |
| 3 | `src/ui/AudioReadoutPanel.cpp:70-74` `if (snap.downbeatDetected) downbeatFlash_ = 1.0f; else *= 0.85f;` | message, 30 Hz | LEVEL consumer: holds the beat-1 box at full alpha while beat 1 is current (`:363-367`), decays during beats 2-4 when the box is drawn as a dim outline anyway (`:376-380`). Coherent with the "current beat = bright" rule for the other boxes (`:369-373`). The panel is HIDDEN in v2: `src/MainComponent.cpp:2490-2491` "Hide v1 panels removed from v2 layout" `audioReadoutPanel_.setVisible(false)` (also `:2412`); no `setVisible(true)` anywhere | keep as-is (4.6 gives the 3-line pulse variant if the panel is ever revived) |
| 4 | `src/test/TestServer.cpp:481-482` | HTTP (test mode) | inject writer | untouched |
| 5 | `src/analysis/AnalysisThread.cpp:204` | analysis | publisher | untouched |
| -- | Shaders: `grep -in downbeat src/render/EmbeddedShaders.h` = 0 hits (no `u_downbeat*` uniform; the 3 uploaders never read the field) | | | none |
| -- | `SignalRegistry`/`MappingEngine`/`connect/`: 0 hits; they use `beatPhase`+`beatInBar`+`totalBarCount` phases (`OscillatorSignal.h:55`, `EnvelopeSignal.h:54`, `ConnectionEngine.cpp:95-96`) | | | none |
| -- | `Autopilot`/`Layer`: 0 hits; beat crossings via `beatPhase` wrap (`Autopilot.cpp:44-45`), bar snap via `beatInBar == 0` (`Layer.h:303-309`) | | | none (adjacent finding in RISKS) |
| -- | `RecorderHost`/`PerfState*`: 0 hits (take-clock `beat` is transport-derived, `RecorderHost.cpp:327,:587`) | | | none |
| -- | `ApiServer`: 0 hits -- `/api/bpm` exposes `beatInBar/barCount/totalBarCount` (`:606-612`) but NOT the level; `/api/features` (`:639-690`) neither | | | add to `/api/bpm` (4.1) |
| -- | `midi/`, `osc/`: 0 hits | | | none |

### 0.4 Cadence arithmetic (VERIFIED constants, derived numbers)
Hop = 512/48000 s = 10.667 ms (`AnalysisThread.h:47,:50`). At 120 BPM a beat is 46.875 hops; the test
drives use 47 hops/beat (`lround`, the `feedRealOnsets` formula `tests/test_bpm_stabilization.cpp:42-43`),
i.e. a 2.0053 s bar. A 60 Hz reader sees the 0.5013 s level on ~30 consecutive reads per bar, a 120 Hz
reader on ~60; both see exactly one rising edge per bar. A 0.9 s poller sees a level true on ~half the
bars only (grid enumeration: 8 of 16 lost in test A, 7-8 of 16 in test B) while the `totalBarCount`
delta still sums to every bar and advances on exactly one read per bar (0.9 s < one bar, so no read
spans two bars). For a genuine one-hop pulse the same readers would show ~0.64 x bars (60 Hz) and
~1.26 x bars (120 Hz) true reads -- the numbers the RED form of the test asks for and will NOT get.

### 0.5 Layout (VERIFIED `FeatureSnapshot.h:126-137`)
`onsetCount` sits at 316-319, `bandValidMask` at 312, so 3 spare bytes remain (313-315): a uint32
`downbeatCount` would grow the struct to 384 (FeatureBus `kSnapshotWords` 80->96, 4 static_asserts). Not
needed: `totalBarCount` is that counter (0.2).

## 1. TRADEOFFS CONSIDERED
- Correct the diagnosis, pin it with tests, expose the level on REST, document, probe (CHOSEN): the only
  option that fixes the actual defect (three readers trusted a wrong comment) and leaves no code that
  pretends a cadence bug exists.
- Add `downbeatCount` to FeatureSnapshot + analysis increment -- REJECTED: byte-for-byte duplicate of
  `totalBarCount` (0.2), and no uint32 fits the 3 spare bytes (0.5).
- Make `downbeatDetected` a true one-hop pulse in the tracker so it matches its comment -- REJECTED: it
  would MANUFACTURE the onset-class loss for every future reader, and the two UI readers want a level
  (beat-1 highlight while beat 1 is current).
- Switch `AudioReadoutPanel`'s flash to an `OnsetPulse` on `totalBarCount` now -- REJECTED for this
  lane: the panel is hidden (0.3 #3), and with the pulse the current beat-1 box would fade to 0.3 alpha
  while the other boxes' "current" fill stays constant -- a visible regression if the panel is revived.
  Recipe kept in 4.6.
- Add a render-side `u_downbeatDetected`/`u_barPulse` uniform now -- REJECTED (scope): no shader or
  source declares one; adding it means 3 uploaders + test-mode `totalBarCount` mirror + docs. Recipe in
  4.6; the OnsetPulse pattern makes it a small change when a shader author asks.
- Leave the two dead `displaySnap_.downbeatDetected` copies -- REJECTED (barely): `TopBar.cpp:232` is
  the exact line that seeded this false lead; two-line deletion, zero behaviour change (4.4).
- Live probe via click WAV + recorder (as `probe-onset-render.sh`) -- REJECTED: the downbeat detector
  needs an accented 4/4 to lock (`scoreBeat` `:323-325`), which `gen-click-wav.py` cannot produce, and
  the take side effects are unnecessary. Manual BPM via `/api/set_bpm` (`MainComponent.cpp:1919-1922`
  -> `applyTempoCommand("link")` `:5016-5022` -> `setManualMode(true)+setManualBPM`) drives the
  predicted-beat regime deterministically with no audio content -- already proven live by
  `.harmony/probe-tempo-silence.sh:33-47` (barCount +3..5 per 8 s at 120 BPM).

## 2. DECISION / SPEC

### 2.1 REST witness: `downbeatDetected` on `/api/bpm` (`src/api/ApiServer.cpp:597-614`)
After `:607` (`barCount`) and before the S168 comment at `:608`, add:
```cpp
    // s-rta-0925: the downbeat LEVEL (held for the whole first beat -- see
    // FeatureSnapshot::downbeatDetected), read in the SAME snapshot as
    // totalBarCount below so a poller can compare its own rising-edge count
    // with the counter delta from one coherent read (probe-downbeat-level.sh).
    obj->setProperty("downbeatDetected", snap.downbeatDetected);
```
`/api/features` (`:639-690`) is left alone (the probe needs level + counter in ONE read, which is
`/api/bpm`; `/api/features` already omits `beatInBar`/`barCount`, so it is not a "full dump" anyway).

### 2.2 Tests (RED-first) -- `tests/test_downbeat_detector.cpp` (existing target, links BPMTracker + Aubio,
`tests/CMakeLists.txt:237-255`; no CMake change). Reuses the file's helpers `lockBPM` `:12-16`,
`feedBeatWithFeatures` `:20-31` (conf 1.0 -> phase hard-reset on the beat hop, `BPMTracker.cpp:209-212`),
`feedNonBeatHops` `:34-41` (conf 0.3 >= `kConfidenceThreshold` 0.1, so the real-onset regime stays
`predictedBeatRegime_ = false`, `:112-120,:187` -- `scoreBeat` is the only beat incrementer).
Add after the includes: `#include "features/OnsetPulse.h"`, `#include <cstddef>`, `#include <cstdint>`.

Shared model (file-scope, after the existing static helpers):
```cpp
// ============================================================================
// Downbeat LEVEL semantics + reader-cadence model (s-rta-0925, roadmap item 4)
// ============================================================================
// downbeatDetected is NOT a one-hop pulse: BPMTracker assigns downbeatDetected_ = (beatCounter_ == 0)
// only at a beat event (scoreBeat / advancePredictedBeat / initial lock) and never clears it per hop,
// so it is HELD for the whole first beat of the bar (300 ms at 200 BPM .. 1 s at 60 BPM).
// updatePhrase() rising-edge-detects it and advances totalBarCount exactly once per bar. Pinned here:
//   * a reader at any UI/render cadence (15 Hz .. 120 Hz) never misses a downbeat: it sees the level on
//     many consecutive reads and its rising edge exactly once per bar;
//   * a reader slower than one beat (a sluggish REST poller) DOES lose rising edges; the totalBarCount
//     delta (OnsetPulse -- the generic monotonic-counter consumer of the onset render-path fix) sees
//     every bar exactly once at ANY cadence. Live twin: .harmony/probe-downbeat-level.sh.
namespace
{
constexpr double kHopSec = 512.0 / 48000.0;   // AnalysisThread::kHopSize / kSampleRate

struct CadenceReader
{
    double     periodSec;
    double     nextRead   = 0.0;
    bool       prevLevel  = false;   // what a LEVEL consumer must keep to see "new bar"
    int        readsTrue  = 0;       // reads on which downbeatDetected was true
    int        edges      = 0;       // rising edges seen: level && !prevLevel
    int        pulseReads = 0;       // reads on which the totalBarCount delta was > 0
    uint32_t   deltaSum   = 0;       // sum of totalBarCount deltas
    OnsetPulse barPulse;             // generic monotonic-counter delta, here on totalBarCount

    void prime(bool level, uint32_t count)          // the reader was already looking before bar 0
    {
        prevLevel = level;
        (void) barPulse.consume(count);             // baseline only
        nextRead = periodSec;
    }
    void catchUp(double tPub, bool level, uint32_t count)   // always-latest bus: read the latest
    {
        while (nextRead <= tPub)
        {
            readsTrue += level ? 1 : 0;
            if (level && !prevLevel) ++edges;
            prevLevel = level;
            const uint32_t d = barPulse.consume(count);
            deltaSum += d;
            if (d > 0u) ++pulseReads;
            nextRead += periodSec;
        }
    }
};

struct HopTruth { int hopsTrue = 0; int hopEdges = 0; bool prevLevel = false; double tPub = 0.0; };

template <size_t N>
void publishHop(const BPMTracker& t, HopTruth& truth, CadenceReader (&readers)[N])
{
    const bool     level = t.downbeatDetected();
    const uint32_t count = t.totalBarCount();
    truth.hopsTrue += level ? 1 : 0;
    if (level && !truth.prevLevel) ++truth.hopEdges;
    truth.prevLevel = level;
    truth.tPub += kHopSec;
    for (auto& r : readers) r.catchUp(truth.tPub, level, count);
}

// GREEN contract shared by both regimes. readers[0] = 60 Hz, [1] = 120 Hz, [2] = 0.9 s poller.
template <size_t N>
void requireLevelContract(const HopTruth& truth, const CadenceReader (&readers)[N],
                          uint32_t barsAdvanced, int kBars, int hopsPerBeatLo, int hopsPerBeatHi)
{
    INFO("barsAdvanced=" << barsAdvanced << " hopsTrue=" << truth.hopsTrue << " hopEdges=" << truth.hopEdges
         << " | 60Hz true=" << readers[0].readsTrue << " edges=" << readers[0].edges << " pulse=" << readers[0].pulseReads
         << " | 120Hz true=" << readers[1].readsTrue << " edges=" << readers[1].edges << " pulse=" << readers[1].pulseReads
         << " | 0.9s true=" << readers[2].readsTrue << " edges=" << readers[2].edges << " pulse=" << readers[2].pulseReads
         << " delta=" << readers[2].deltaSum);
    // Hop-level truth: one bar per 4 beats; the level held for the whole first beat.
    REQUIRE(barsAdvanced == static_cast<uint32_t>(kBars));
    REQUIRE(truth.hopEdges == kBars);
    REQUIRE(truth.hopsTrue >= kBars * hopsPerBeatLo);
    REQUIRE(truth.hopsTrue <= kBars * hopsPerBeatHi);
    // UI/render cadence: the level is seen on many reads per bar (a level, not a pulse), its rising
    // edge exactly once per bar (no loss, no duplication), and the counter agrees.
    for (int i = 0; i < 2; ++i)
    {
        REQUIRE(readers[i].readsTrue > 10 * kBars);
        REQUIRE(readers[i].edges == kBars);
        REQUIRE(readers[i].pulseReads == kBars);
        REQUIRE(readers[i].deltaSum == static_cast<uint32_t>(kBars));
    }
    // Slower than one beat (0.9 s > 0.5 s): rising-edge detection on the level LOSES bars (about half
    // on this grid); the totalBarCount delta still sees every bar exactly once.
    REQUIRE(readers[2].edges < kBars);
    REQUIRE(readers[2].pulseReads == kBars);
    REQUIRE(readers[2].deltaSum == static_cast<uint32_t>(kBars));
}
} // namespace
```

Test A -- real-onset regime (`scoreBeat` is the incrementer):
```cpp
TEST_CASE("Downbeat level (real onsets): held for the whole first beat, totalBarCount once per bar; "
          "60/120 Hz readers lose nothing, a 0.9 s poller needs the counter",
          "[downbeat][level][cadence]")
{
    BPMTracker tracker(512, 1024, 48000);
    lockBPM(tracker, 120.0f);
    for (int bar = 0; bar < 8; ++bar)                 // lock the downbeat (compressed spacing, as above)
        for (int beat = 0; beat < 4; ++beat)
        {
            feedBeatWithFeatures(tracker, 120.0f, beat == 0);
            feedNonBeatHops(tracker, 120.0f, 3);
        }
    REQUIRE(tracker.downbeatLocked());
    REQUIRE_FALSE(tracker.downbeatDetected());       // the lock phase ended on beat 4 (index 3)

    constexpr int kBars = 16;
    constexpr int kHopsPerBeat = 47;                  // lround(48000*60 / (120*512)): real-time spacing
    const uint32_t bars0 = tracker.totalBarCount();

    CadenceReader readers[] = { { 1.0 / 60.0 }, { 1.0 / 120.0 }, { 0.9 } };
    HopTruth truth;
    truth.prevLevel = tracker.downbeatDetected();
    for (auto& r : readers) r.prime(tracker.downbeatDetected(), tracker.totalBarCount());

    for (int bar = 0; bar < kBars; ++bar)
        for (int beat = 0; beat < 4; ++beat)
        {
            feedBeatWithFeatures(tracker, 120.0f, beat == 0);   // the beat hop (conf 1.0)
            publishHop(tracker, truth, readers);
            for (int h = 1; h < kHopsPerBeat; ++h)              // 46 non-beat hops
            {
                feedNonBeatHops(tracker, 120.0f, 1);
                publishHop(tracker, truth, readers);
            }
        }

    requireLevelContract(truth, readers, tracker.totalBarCount() - bars0, kBars,
                         kHopsPerBeat - 1, kHopsPerBeat + 1);   // exact is 16*47 = 752
}
```

Test B -- manual/predicted regime (the regime the live probe drives; `advancePredictedBeat` is the
incrementer, no onsets at all):
```cpp
TEST_CASE("Downbeat level (manual BPM, predicted beats): the same level + counter contract with no "
          "onsets -- the regime /api/set_bpm puts the tracker in",
          "[downbeat][level][cadence][manual]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualMode(true);
    tracker.setManualBPM(120.0f);                     // lockedBPM_ = 120, phase_ = 0, beatCounter_ = 0
    REQUIRE(tracker.isManualMode());

    constexpr int kBars = 16;
    constexpr int kHops = 3060;   // 65 phase wraps at 46.875 hops/beat; downbeats at wraps 4,8,..,64
                                  // (beatCounter_ 0->1->2->3->0), the 16th level window ~hops [3000,3047)
    const uint32_t bars0 = tracker.totalBarCount();
    CadenceReader readers[] = { { 1.0 / 60.0 }, { 1.0 / 120.0 }, { 0.9 } };
    HopTruth truth;
    truth.prevLevel = tracker.downbeatDetected();
    for (auto& r : readers) r.prime(tracker.downbeatDetected(), tracker.totalBarCount());

    for (int h = 0; h < kHops; ++h)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);        // manual branch: predicted wrap drives beats
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f);  // per hop, as AnalysisThread: updatePhrase -> totalBarCount
        publishHop(tracker, truth, readers);
    }

    requireLevelContract(truth, readers, tracker.totalBarCount() - bars0, kBars, 46, 48);
}
```

RED-first procedure (record the numbers in the commit message; this is the fail-first evidence that the
premise is wrong, not a pre/post-fix RED -- no production logic changes):
1. Write the model + Test A with, in place of `requireLevelContract`, the task's premise as CHECKs (CHECK,
   not REQUIRE, so all numbers print):
   ```cpp
   const uint32_t barsAdvanced = tracker.totalBarCount() - bars0;
   CHECK(truth.hopsTrue == truth.hopEdges);                              // "one hop true per downbeat"
   CHECK(readers[0].readsTrue < static_cast<int>(barsAdvanced));         // "60 fps loses"
   CHECK(readers[0].edges     < static_cast<int>(barsAdvanced));
   CHECK(readers[1].readsTrue > static_cast<int>(barsAdvanced) && readers[1].readsTrue < 2 * static_cast<int>(barsAdvanced)); // "120 fps duplicates ~1.26x"
   ```
   Expected FAIL expansions (scratch build dir, `./test_downbeat_detector "[cadence]"`): `752 == 16`,
   `481 < 16` (may read 480-482), `16 < 16`, `962 > 16 && 962 < 32`.
2. Replace the CHECK block with `requireLevelContract(...)`; add Test B. Both GREEN. ctest 445 -> 447.
3. Keep the RED numbers in the commit body (precedent: 4532779's message).

### 2.3 Comment / doc corrections (exact text)
- `src/analysis/FeatureSnapshot.h:47` -- replace `// true on the hop where beat 1 lands` with:
  ```cpp
    // LEVEL, not a pulse (s-rta-0925): BPMTracker assigns this = (beatCounter_ == 0) only at a beat
    // event and never clears it per hop, so it reads true for the WHOLE first beat of the bar (one
    // beat period, 300 ms at 200 BPM .. 1 s at 60 BPM) and false for beats 2-4. No 15-120 Hz reader
    // can miss it and re-reading it is not duplication. "New bar" is its rising edge, which
    // totalBarCount below counts exactly (BPMTracker::updatePhrase) -- a consumer slower than a
    // beat, or wanting one pulse per bar at any cadence, diffs totalBarCount (OnsetPulse) instead.
    bool    downbeatDetected = false;
  ```
- `src/analysis/BPMTracker.h:219` -- replace the trailing comment with
  `// LEVEL: (beatCounter_ == 0), assigned at beat events only (scoreBeat / advancePredictedBeat /`
  `// initial lock), never cleared per hop -- held for the whole first beat; updatePhrase() edge-detects`
  `// it into barCount_/totalBarCount_. NOT "true on the hop where beat 1 lands".`
- `CLAUDE.md:61` FeatureSnapshot table row -> `| downbeatDetected | bool | level | Held true for the
  WHOLE first beat of the bar (assigned at beat events, never cleared per hop -- not a one-hop pulse);
  its rising edge is what totalBarCount counts |`. (Optional: add a `totalBarCount` row; the table
  currently lacks one.)
- `CLAUDE.md:1101` Pitfall 30, last sentence: replace "`downbeatDetected` has the same one-hop class
  and is NOT yet covered." with "`downbeatDetected` is NOT in this class -- it is a beat-long level
  (Pitfall 32)."
- `CLAUDE.md` new Pitfall 32 after 31: "**`downbeatDetected` is a beat-long LEVEL, not a pulse -- never
  read a true value as 'a downbeat happened on this read'**: `BPMTracker` assigns
  `downbeatDetected_ = (beatCounter_ == 0)` only at beat events (`scoreBeat`, `advancePredictedBeat`,
  the initial lock) and never clears it per hop, so the flag is true for the whole first beat
  (300 ms-1 s) -- no 15-120 Hz reader can miss it and re-reading it is not duplication (s-rta-0925 lane
  re-derived this after three readers trusted the old 'true on the hop where beat 1 lands' comment).
  'New bar' is its rising edge, which `BPMTracker::updatePhrase` already counts into `totalBarCount`
  (monotonic, never reset, published every hop, on `/api/bpm` next to the level). A consumer that wants
  one pulse per bar at ANY cadence -- including REST pollers slower than a beat, which DO lose rising
  edges -- diffs `totalBarCount` with `OnsetPulse` (a generic monotonic-counter delta), one instance per
  reader thread, exactly like `onsetCount`. Do not add a `downbeatCount` field: it would duplicate
  `totalBarCount`. Guards: `tests/test_downbeat_detector.cpp` `[level][cadence]`; live:
  `.harmony/probe-downbeat-level.sh`."
- `.harmony/APP-INVENTORY.md:151` row 10 -> `bpm, beatPhase, barPhase, phrasePhase, beatInBar, barCount,
  totalBarCount, downbeatDetected (level)`.
- `tests/visual/TESTING.md`: no change (inject field list `:126-127` already names `downbeatDetected`;
  it stays a plain bool in test mode).

### 2.4 Dead copies (recommended, 2 lines, zero behaviour change; Harmony's call)
Delete `src/ui/TopBar.cpp:232` and `src/ui/AudioReadoutPanel.cpp:43` (both write `displaySnap_.
downbeatDetected`, which no paint routine reads -- 0.3 #1-2). Nothing else references the member.

### 2.5 Files touched, sequence, counts
Production: `src/api/ApiServer.cpp` (+5 lines), `src/analysis/FeatureSnapshot.h` (comment),
`src/analysis/BPMTracker.h` (comment); optional `src/ui/TopBar.cpp` (-1), `src/ui/AudioReadoutPanel.cpp`
(-1). Deliberately UNTOUCHED: `AnalysisThread`, `BPMTracker.cpp`, `FeatureSnapshot` layout (sizeof 320
static_asserts unchanged), `Renderer`/`OutputWindow`/uploaders/shaders, `TestServer`, `OnsetPulse.h`,
`RecorderHost`, `Autopilot`/`Layer`.
Tests: `tests/test_downbeat_detector.cpp` (+ model + 2 TEST_CASEs). ctest 445 -> 447.
Live: `.harmony/probe-downbeat-level.sh` (new; `git add -f`; `bash -n` + shellcheck if installed).
Docs: `CLAUDE.md` (row 61, Pitfall 30 sentence, Pitfall 32), `.harmony/APP-INVENTORY.md:151`.
Builder sequence (scratch build dir or lane worktree, never `./build`; reuse deps via
`-DFETCHCONTENT_SOURCE_DIR_<NAME>`): (1) test model + Test A RED form -> run -> record numbers;
(2) GREEN forms + Test B -> `ctest` 447/447; (3) ApiServer line; (4) comments/docs; (5) probe script,
`bash -n`; (6) commit (`git add -f` under `.harmony/`, check `git show --stat HEAD`); (7) Harmony runs
section 5 on the lane build in PRODUCTION mode.

### 2.6 Follow-up recipe (NOT this lane): a consumer that wants one pulse per bar
- Message-thread or REST-side reader: `OnsetPulse barPulse_;` (one per reader thread), pulse =
  `barPulse_.consume(snap.totalBarCount) > 0u`; `reset()` where the reader starts looking afresh.
  `AudioReadoutPanel` variant = replace `:71 if (snap.downbeatDetected)` with
  `if (barPulse_.consume(snap.totalBarCount) > 0u)` + member next to `onsetPulse_` (`AudioReadoutPanel.h:30`)
  -- and revisit the beat-1 box alpha rule (`:366`) so a faded pulse does not read as "not current".
- Render side: derive once per frame on the GL thread next to `Renderer.cpp:230`, reset at `:113`,
  own instance in `OutputRenderer` (`OutputWindow.cpp:104`); carry it to the three uploaders as a NEW
  1-byte field in the frame copy (fits the 3 spare bytes at 313-315 without changing sizeof 320 --
  analysis never sets it) and upload as `u_barPulse`; keep `downbeatDetected` a level. Test-mode
  injection must then mirror `totalBarCount` (`TestServer.cpp:468` defaults it to 0 = backwards jump).

## 3. THREADING / SACRED RULES (nothing on a hot path changes)
- Audio callback, analysis thread, FeatureBus, FeatureSnapshot layout: untouched.
- `ApiServer::handleGetBpm`: HTTP thread; reads the bus via the existing `featureBus_.read()` (`:599`)
  and adds one property to the same heap `DynamicObject` its neighbours use -- identical discipline to
  `:606-612`. No new state, no lock, no atomic.
- Tests: single-threaded, drive `BPMTracker` directly (precedent `tests/test_downbeat_detector.cpp`,
  `tests/test_bpm_stabilization.cpp:31-53`); `OnsetPulse` is an 8-byte POD.
- Probe: external HTTP only; `/api/set_bpm` marshals to the message thread (`ApiServer.cpp:629-633`).

## 4. FAIL-FIRST SUMMARY (what discriminates)
- Test A RED form fails with `752 == 16`, `481 < 16`, `16 < 16` -- the premise's predictions against
  the real tracker; GREEN pins level + counter at 60/120 Hz and the 0.9 s loss/counter contrast.
- Test B proves the same in the predicted regime (manual BPM), tying the unit model to the live probe.
- Live: on a pre-fix build `/api/bpm` has no `downbeatDetected` -> the probe FAILs "cannot observe"
  (the REST-field RED, same shape as `probe-onset-render.sh`'s NA). Post-fix the duty/run-length
  oracle refutes "one-hop pulse" live (0.25 / ~500 ms vs ~0.03 / <= 45 ms) and the 0.9 s subsample
  shows the counter-vs-level contrast.

## 5. LIVE PROBE (Harmony's live-app agent runs it -- NOT the Architect, NOT the Builder)
File: `.harmony/probe-downbeat-level.sh` (conventions mirror `probe-onset-render.sh`: `ok/no/info/jget`,
`open` launch with logs CLEARED first (rig rule: `open --stderr` appends), bracketed pgrep, production
mode, graceful quit, Output-window check, screenshot). PASS iff all verdict lines pass.
```bash
#!/bin/bash
# probe-downbeat-level.sh -- live witness for s-rta-0925 roadmap item 4: downbeatDetected is a
# beat-long LEVEL (not a one-hop pulse), totalBarCount is its exact per-bar counter, a UI-cadence
# poller loses nothing, and a poller slower than a beat loses rising edges while the counter does
# not. Recipe: .harmony/.reports/s-rta-0925/plan-downbeat.md section 5.
# Written by the Builder, NOT run by the Builder. Validate: bash -n; shellcheck (if installed).
#
# ORACLE (no audio content needed): /api/set_bpm puts BPMTracker in manual mode; its predicted
# phase wraps cycle beatCounter_, so downbeatDetected_ = (beatCounter_ == 0) is held for one whole
# beat (500 ms at 120 BPM) and totalBarCount advances once per 4 wraps (2.0 s). A ~25 Hz poller of
# /api/bpm (level + counter from ONE coherent snapshot per poll) must see: duty ~0.25 with true-runs
# ~500 ms (LEVEL -- a one-hop pulse would read duty ~0.03 and runs <= 45 ms); rising edges ==
# totalBarCount delta (no loss at UI cadence); downbeatDetected == (beatInBar == 0) on every poll.
# The same stream subsampled at 0.9 s (slower than a beat) must LOSE rising edges while its
# totalBarCount delta still counts every bar. Pre-fix build: /api/bpm has no downbeatDetected -> FAIL.
#
# RIG FACTS (same as probe-onset-render.sh -- do not re-derive):
#   * ApiServer = http://127.0.0.1:7070 (IPv4 ONLY), PRODUCTION mode, NO --test-mode (test mode never
#     starts AnalysisThread; /api/bpm would read zeros forever -- probe-tempo-silence.sh's lesson).
#   * Analysis needs hops flowing: the default MicInput device must be open (TCC prompt on the first
#     launch after a rebuild). If beatPhase never moves, screencapture -x and LOOK.
#   * Launch via `open`; every pgrep uses the bracket trick 'MacOS/Audio-DN[A]'.
#   * SCREEN-SAFETY LAW: never open the Output window; graceful osascript quit first, pkill last.
set -u
OUT="${1:-/tmp/audiodna-downbeat-level}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${DOWNBEAT_BUILD_DIR:-build-lane}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
VENV_PY="${DOWNBEAT_VENV_PY:-$ROOT/.venv/bin/python}"
A='http://127.0.0.1:7070'
BPM=120
POLL_S="${DOWNBEAT_POLL_S:-30}"
FAST_SLEEP=0.04     # ~25 Hz: the UI reader class (TopBar 15 Hz, AudioReadoutPanel 30 Hz)
SLOW_PERIOD=0.9     # slower than one beat (0.5 s at 120 BPM): where a LEVEL consumer does lose bars
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
info(){ echo "INFO  $1"; }
jget(){ # $1 = URL  $2 = python expr against `d`; "NA(...)" on any error
  curl -s --max-time 5 "$1" | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($2)
except Exception as e:
    print('NA(%s)' % e)
"
}
isint(){ echo "$1" | grep -Eq '^-?[0-9]+$'; }

# --- 1. preconditions + production launch ----------------------------------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set DOWNBEAT_BUILD_DIR to override)"; exit 64; }
: > "$OUT/adna-out.log"; : > "$OUT/adna-err.log"      # open --stdout/--stderr APPEND: clear first
open --stdout "$OUT/adna-out.log" --stderr "$OUT/adna-err.log" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (TCC mic prompt? screencapture -x and LOOK)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
[ -n "$PID" ] || { echo "FAIL: health answered but no Audio-DNA process was found"; exit 1; }
ok "app launched (production), /api/health answered, PID=$PID"
sleep 10
B1="$(jget "$A/api/bpm" "d.get('totalBarCount','NA')")"
isint "$B1" && ok "analysis thread live: totalBarCount readable ($B1)" || no "totalBarCount not readable ($B1) -- analysis thread not running?"

# --- 2. the field under test (RED on a pre-fix build) -----------------------
HAS="$(jget "$A/api/bpm" "isinstance(d.get('downbeatDetected'), bool)")"
[ "$HAS" = "True" ] && ok "/api/bpm exposes downbeatDetected (bool)" \
  || no "/api/bpm has no bool downbeatDetected ($HAS) -- pre-fix build? the level cannot be observed"

# --- 3. manual tempo: predicted beats, no audio content needed ---------------
curl -s --max-time 6 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d "{\"bpm\":$BPM}" >/dev/null
sleep 1
B="$(jget "$A/api/bpm" "round(d['bpm'], 1)")"
[ "$B" = "$BPM.0" ] && ok "manual tempo took (bpm=$B)" || no "manual tempo did not take (bpm=$B)"

# --- 4. poll ~25 Hz for POLL_S seconds; verdicts computed in python ----------
if [ "$HAS" = "True" ]; then
POLL_S="$POLL_S" FAST_SLEEP="$FAST_SLEEP" SLOW_PERIOD="$SLOW_PERIOD" BPM="$BPM" OUT="$OUT" \
python3 - "$A" > "$OUT/verdicts.txt" <<'PY'
import json, os, statistics, sys, time, urllib.request
A = sys.argv[1] + '/api/bpm'
poll_s = float(os.environ['POLL_S']); fast_sleep = float(os.environ['FAST_SLEEP'])
slow_period = float(os.environ['SLOW_PERIOD']); bpm = int(os.environ['BPM']); out = os.environ['OUT']
def get():
    with urllib.request.urlopen(A, timeout=2) as r:
        return json.load(r)
polls = []
t_end = time.monotonic() + poll_s
while time.monotonic() < t_end:
    t = time.monotonic(); d = get()
    polls.append((t, bool(d['downbeatDetected']), int(d['beatInBar']), int(d['totalBarCount']), float(d['beatPhase'])))
    time.sleep(fast_sleep)
n = len(polls)
gaps = [polls[i][0] - polls[i-1][0] for i in range(1, n)]
mean_ms = 1000 * statistics.mean(gaps); max_ms = 1000 * max(gaps)
duty = sum(1 for p in polls if p[1]) / n
runs = []; start = None
for t, lvl, _, _, _ in polls:
    if lvl and start is None: start = t
    elif not lvl and start is not None: runs.append(t - start); start = None
run_median_ms = 1000 * statistics.median(runs) if runs else -1
edges_fast = sum(1 for i in range(1, n) if polls[i][1] and not polls[i-1][1])
delta_fast = polls[-1][3] - polls[0][3]
mismatch = sum(1 for p in polls if p[1] != (p[2] == 0))
phase_values = len(set(round(p[4], 2) for p in polls))
slow = []; nxt = polls[0][0]
for p in polls:
    if p[0] >= nxt:
        slow.append(p)
        while nxt <= p[0]: nxt += slow_period
edges_slow = sum(1 for i in range(1, len(slow)) if slow[i][1] and not slow[i-1][1])
delta_slow = slow[-1][3] - slow[0][3]
pulse_slow = sum(1 for i in range(1, len(slow)) if slow[i][3] > slow[i-1][3])
with open(os.path.join(out, 'poll.json'), 'w') as f:
    json.dump({'n': n, 'mean_ms': mean_ms, 'max_ms': max_ms, 'duty': duty, 'runs_ms': [1000 * r for r in runs],
               'edges_fast': edges_fast, 'delta_fast': delta_fast, 'mismatch': mismatch, 'phase_values': phase_values,
               'slow_n': len(slow), 'edges_slow': edges_slow, 'delta_slow': delta_slow, 'pulse_slow': pulse_slow}, f)
def v(cond, msg): print(('PASS  ' if cond else 'FAIL  ') + msg)
v(n >= 400 and max_ms < 250, f"poller healthy: {n} polls, mean {mean_ms:.1f} ms, max gap {max_ms:.1f} ms (need >= 400 polls, max gap < 250 ms)")
v(phase_values > 5, f"analysis hops flowing: beatPhase took {phase_values} distinct values")
v(12 <= delta_fast <= 17, f"bars advance at the manual tempo: totalBarCount +{delta_fast} in {poll_s:.0f} s (expect ~15 at {bpm} BPM)")
v(0.18 <= duty <= 0.32 and 350 <= run_median_ms <= 700, f"LEVEL: downbeatDetected duty {duty:.3f} (expect ~0.25), true-run median {run_median_ms:.0f} ms over {len(runs)} runs (expect ~500 ms) -- a one-hop pulse would read duty ~0.03 and runs <= 45 ms")
v(edges_fast == delta_fast, f"no loss at ~25 Hz: rising edges {edges_fast} == totalBarCount delta {delta_fast}")
v(mismatch == 0, f"coherent: downbeatDetected == (beatInBar == 0) on every poll ({mismatch} mismatches)")
v(pulse_slow == delta_slow and 0 <= delta_fast - delta_slow <= 1, f"counter is cadence-independent: the 0.9 s subsample ({len(slow)} reads) saw {pulse_slow} advancing reads, delta {delta_slow} (fast delta {delta_fast})")
v(edges_slow < delta_slow, f"level edge-detection at 0.9 s LOSES bars: {edges_slow} edges vs {delta_slow} bars (expect roughly half) -- the counter above did not")
PY
  cat "$OUT/verdicts.txt"
  PASS=$((PASS + $(grep -c '^PASS ' "$OUT/verdicts.txt"))); FAIL=$((FAIL + $(grep -c '^FAIL ' "$OUT/verdicts.txt")))
else
  no "poll skipped: downbeatDetected not exposed"
fi

# --- 5. teardown (SCREEN-SAFETY LAW) ------------------------------------------
osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then
    echo "graceful osascript quit did not clear the process -- falling back to pkill (last resort)"
    pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
fi
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING AFTER graceful quit + pkill fallback" || ok "app terminated, no process remains"
if [ -x "$VENV_PY" ]; then
    W="$("$VENV_PY" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))
           and 'Output' in str(w.get('kCGWindowName',''))]))" 2>/dev/null)"
    if [ "$W" = "0" ] || [ -z "$W" ]; then ok "0 Audio-DNA Output windows in the FULL window list (SCREEN-SAFETY LAW held)"
    else no "$W Audio-DNA Output window(s) were open during the run -- screen-safety breach"; fi
else
    no "Output-window check: $VENV_PY (pyobjc/Quartz) unavailable -- set DOWNBEAT_VENV_PY to the main checkout's .venv/bin/python"
fi
screencapture -x "$OUT/downbeat-level-eos.png" 2>/dev/null \
  && echo "screenshot: $OUT/downbeat-level-eos.png (read it before concluding -- no dialog expected)" \
  || echo "screencapture failed (no display attached / headless run)"
echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
```
Interpretation notes for the runner: a noisy room lets aubio beats (conf >= 0.5) hard-reset the phase
in manual mode (`BPMTracker.cpp:209-212`), which stretches individual runs but not the duty or the
edge/counter equality -- the median and the [350, 700] ms band absorb it; if `phase_values` is small the
mic is not feeding analysis (TCC), not a downbeat problem. Expected ~15 bars, ~15 runs, 25 Hz ~750 polls.

## 6. RISKS
- Strongest counterargument to my recommendation: "three documents (prior plan section 6, Pitfall 30,
  this dispatch) say one-hop pulse -- maybe some path DOES make it one." It loses on the source: all six
  assignment sites are beat-event-only (0.1), no per-hop clear exists anywhere in `BPMTracker.cpp`, and
  the tracker's own `prevDownbeatDetected_` edge detector (`:237`, `:471`) only makes sense for a level.
  The cheapest disconfirming test is Test A's RED form (`752 == 16`), and the probe's duty/run oracle
  is its live twin. Every one of the three documents traces to the same wrong comment.
- Second counterargument: "even so, apply the pattern now so future consumers cannot get it wrong." No
  consumer exists to switch (0.3), the hidden panel's level-hold is the coherent visual (1), and the
  contract is now pinned by tests + Pitfall 32 + the 3-line recipe (2.6); shipping unused pulse plumbing
  is the YAGNI the karpathy rules forbid.
- Behaviour change: none in production (REST gains one read-only bool; comments; optional dead-line
  deletions). Boris sees nothing different.
- Test brittleness: `hopsTrue` bands assume `feedNonBeatHops`' conf 0.3 keeps the real-onset regime
  (VERIFIED `:112-120`) and 47 hops/beat; a future helper change moves the numbers, not the contract.
  Float phase accumulation in Test B drifts < 0.001 hop per wrap (INFERRED) -- `kHops = 3060` leaves a
  13-hop margin past the 16th level window.
- Probe: needs hops flowing (mic open); the counter-vs-level verdicts do not depend on room noise, the
  run-length band tolerates aubio phase resets; `delta_fast - delta_slow <= 1` allows a bar landing
  after the last 0.9 s mark.
- Adjacent finding, NOT this lane (INFERRED): `Autopilot.cpp:44-57` fires `Layer::processPendingTrigger`
  on the frame after a `beatPhase` wrap with the CURRENT `beatInBar`, but `beatInBar` advances on the
  beat EVENT (`scoreBeat`) while the phase wraps on the free-running clock; in the real-onset regime the
  two can differ by a few hops, so a Bar/TwoBar/FourBar snap (`Layer.h:303-309`) may evaluate the
  previous beat's index. Worth its own probe; `totalBarCount` deltas would be the robust bar clock there.
- Test-mode injection publishes `totalBarCount = 0` when the field is absent (`TestServer.cpp:468`): any
  future pulse consumer sees a backwards jump and re-baselines -- the same rule `onsetCount` needed
  (`InjectedOnsetCount`); document at that time.
- Learning worth keeping (not logged: project-repo boot, disk fence): a field's comment is not evidence
  of its semantics -- for any "pulse vs level" question read every assignment site and look for an
  edge detector on the consumer side (its presence proves a level). The prior plan, the critic, and
  Pitfall 30 all inherited one comment; the cheapest refutation was a 40-line tracker-driven test.

STATUS: COMPLETE -- plan is Builder-executable with no open questions; the premise is refuted with cited evidence, the counter already exists (totalBarCount), and the live probe is Harmony's to run (no app launch by Architect).
