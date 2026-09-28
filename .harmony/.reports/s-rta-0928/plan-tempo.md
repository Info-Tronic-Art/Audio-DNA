# PLAN -- the take start-tempo race: a tempo command sent just before Record must be in the take's start

Architect (Fable), 2026-09-28, against main @ 6db8d67 (code = 233eae7). Every `path:line` below was read on
6db8d67 today. Input: `.harmony/.reports/s-rta-0927/tempo0-diag.md` (verified by trace). Its MainComponent line
numbers have drifted: its `MainComponent.cpp:5285-5287` is `:5276-5282` today. A builder reads ONLY this file.
Labels: VERIFIED (read at file:line, or derived from code read today), INFERRED (the best model the evidence
allows), ASSUMED (not checked; the builder checks where marked).

QUESTION: when a tempo command (REST/OSC set_bpm, Resync, Tap, manual BPM, Link, a replayed tempo point) and
Record arrive within about one analysis hop, the take's "start" anchor can carry the old tempo (bpm 0 when
unlocked, 4/34 witness runs), and its bar grid can be unknown or stale (9/34). How should Record's t = 0 be
defined so that every command sent before Record is in the take's start and bar grid, without breaking the
unmetered start, the fallback when there is no audio, audio alignment, old takes, or the real-time rules?

APPROACH (the ruling): neither fix A nor fix B. Use an EXACT handshake (the dispatch's better option, refined):
- `BPMTracker` numbers its requests. Every request raises a sequence number (release) after its own write.
  `runPipeline` latches it (acquire) as its first statement, before it reads any request. `AnalysisThread`
  publishes the latched value as `FeatureSnapshot::trackerRequestSeq`.
- `perfRecord` passes `BPMTracker::postedRequestSeq()` into `ArmOptions`.
- `RecorderHost` delays only the CLOCK's first tick (t = 0) until a snapshot has `trackerRequestSeq` >= that
  value. It gives up 100 ms after the first tick after arm. `meta.startBeatInBar` is read from that same
  snapshot. The audio tap and the provisional save stay at arm.
- A Stop before t = 0 starts the take from the last tick it saw.
- The result is exact. It covers every path, including future callers of the request API, and t = 0 moves ONLY
  when a command is in flight at Record. Every other take, and every existing probe, starts on the same tick as
  today.
- TempoMap::sampleAt: FILED, not fixed here (section 8).

--------------------------------------------------------------------------------------------------------------
## 0. What the code does today (re-derived on 6db8d67)

### 0.1 The mechanism, both symptoms (VERIFIED)
- **A tempo command is only a REQUEST.** `MainComponent::applyTempoCommand` (MainComponent.cpp:5154-5207)
  calls `setManualBPM` / `followExternalTempo` / `setManualMode` / `requestResync` (:5159-5184). Each one only
  posts:
  - `postTempoRequest` does a CAS on one atomic word (BPMTracker.cpp:586-598);
  - `requestResync` does `resyncRequests_.fetch_add` (:555-558);
  - `setManualMode` does `manualMode_.store` (:625-628).
- **The analysis thread applies them on its next hop:**
  - the tempo word at the start of `runPipeline` (:74-79);
  - the manual flag right after (:94);
  - a Resync at the END of `feedDownbeatFeatures` (:352-357).
  - The hop copies the tracker fields (AnalysisThread.cpp:199-209) and publishes (:343).
  - A hop runs only when 512 samples are available (:86-90, `sleep(1)` otherwise). The period is 10.67 ms.
- **Symptom 1: the start has the stale tempo.** The take clock starts on the first 120 Hz tick after arm
  (RecorderHost.cpp:433-438). The first-tick branch copies `snap.bpm` into the "start" anchor as-is
  (RecorderClock.cpp:16-29). If that tick reads the bus before the applying hop publishes, "start" carries the
  old tempo (0 when unlocked), and a "lock"/"bpm" anchor follows about 10 ms later.
- **Symptom 2: the bar grid is unknown or stale.** `perfRecord` reads the bus ONCE at arm
  (MainComponent.cpp:5276-5282) and writes `startBeatInBar` only if the tracker is LOCKED. That read comes right
  after the command's handler on the same thread, so it almost always predates the applying hop.
  - If the tracker was unlocked, the grid is UNKNOWN.
  - If it was locked (Tap/Resync before Record), the grid is STALE: it holds the pre-command bar position.
    The diag did not measure this case; it is INFERRED from the same ordering.
- **Harm** (VERIFIED on real takes by the diag; code re-read):
  - `checkMetered`'s second loop refuses any cut whose range contains the bpm-0 "start" anchor at beat 0
    (RoutineSlice.cpp:66-68).
  - An unknown `startBeatInBar` makes bar 1 = beat 0 (RoutineSlice.cpp:457-458).
  - After a Resync, a stale grid counts bars from the wrong downbeat.

### 0.2 Every path that changes tempo or phase (VERIFIED at the cited lines)
| path | entry | thread | tracker request | applied |
|---|---|---|---|---|
| REST `/api/set_bpm` | ApiServer.cpp:755-776 callAsync -> MainComponent.cpp:1896-1899 `"link"` | message | setManualMode(true) + followExternalTempo | runPipeline start |
| REST `/api/resync` | ApiServer.cpp:778-790 -> MainComponent.cpp:1900-1903 `"resync"` | message | requestResync | end of feedDownbeatFeatures |
| OSC `/audiodna/bpm`, `/audiodna/resync` | OscHandler.cpp:170-181 (MessageLoopCallback, OscHandler.h:29-30) -> MainComponent.cpp:2157-2164 | message | as REST | as REST |
| MIDI Tap / Resync bindings | MidiHandler.cpp:70-100 callAsync -> handleBindingAction MainComponent.cpp:7068; Tap :7236-7254, Resync :7258-7260 | message | setManualBPM (realign) / requestResync | runPipeline / end of hop |
| keyboard bindings | same handleBindingAction | message (INFERRED: JUCE key events) | same | same |
| TopBar Tap / typed BPM / auto / Resync | MainComponent.cpp:587-589, 596-598, 600-602 | message | setManualBPM / setManualMode(+followExternalTempo) / setManualMode(false) / requestResync | as above |
| Ableton Link | timerCallback MainComponent.cpp:3762-3768, 30 Hz, only when enabled (never in a default build, :3761) | message | setManualMode(true) + followExternalTempo, every tick | runPipeline start |
| replayed tempo point | MainComponent.cpp:1958-1962 (Origin::Replay) | message | as its action | as above |

Every row reaches the tracker only through `applyTempoCommand`. Outside BPMTracker, the four request methods are
called only at MainComponent.cpp:5159-5184 (grep, VERIFIED). Every row is applied by an analysis hop. No path
writes tracker state directly.

### 0.3 Who arms a take (VERIFIED)
- Only the Record panel (MainComponent.cpp:1545-1552) and REST `/api/perf/record` (ApiServer.cpp:1401-1429 ->
  MainComponent.cpp:2082) arm a take. Both run `perfRecord` on the message thread.
- The ToggleRecording binding and the Output menu item drive the VIDEO recorder, not a take
  (MainComponent.cpp:7366-7372, :6628).
- So the race needs either a scripted client (REST, or OSC tempo + REST record), or a message thread about 10 ms
  late with both messages queued (the diag's run 03). A human does not press Record within 10 ms of a tap
  (INFERRED).

### 0.4 What t = 0 is tied to (VERIFIED)
- The audio tap starts at arm (RecorderHost.cpp:228) and captures `firstSample` at its first push
  (AudioTap.h:67).
- Events and audio line up through absolute device-sample stamps:
  - WithAudio replay places points by `s.sample` (Program.cpp:431-433) against
    `playFirstSample_ + asset frames` (RecorderHost.cpp:523-532);
  - T2 pairs `marker.sample - firstSample` (probe-step3.sh:505-529).
- The only code that reads an anchor's `sample` is `TempoMap::sampleAt`, called from Program.cpp:451 (grep).
- So moving t = 0 later moves nothing relative to the audio.

--------------------------------------------------------------------------------------------------------------
## 1. Ruling

### 1.1 Chosen: the exact handshake (section 2 has the details)
Reasons:
1. **Exact.** It waits for the one snapshot that carries every command sent before Record. There is no hop
   arithmetic and no guess.
2. **Covers every path in 0.2**, and any future caller of the request API, because the raise lives inside the
   request API itself.
3. **t = 0 moves only for raced takes**, i.e. when a command is in flight at Record. Every other take, and every
   existing recording probe (section 7), starts on the same tick as today.
4. **It is the codebase's own counter pattern** for "a slower reader must not miss an analysis-side event":
   `onsetCount` (Pitfall 30), `totalBarCount` (32), `totalBeatCount` (42).
5. **Cheap.** It costs 4 bytes in FeatureSnapshot's free tail (sizeof stays 384), one acquire load per hop and
   one release RMW per command. It adds no allocation and no lock on the audio or analysis threads.

Kept from A: `startBeatInBar` from the t = 0 snapshot (the same instant as beat 0), a wall fallback, and the
gate on the clock only.

### 1.2 Rejected: A, `firstTickNotBefore = armSnap.timestamp + 2 * kHopSize` plus a 50 ms fallback
- It cannot tell whether a command is in flight, so it moves t = 0 by 11-21 ms for EVERY take. Every recording
  probe window and every take duration shifts, and every baseline has to be redone for no gain.
- It is a heuristic tied to where each request is consumed today (the start or end of a hop). If a request moves
  elsewhere later, A breaks silently. C names that dependency and tests it.
- It does arithmetic on 48 kHz analysis-domain timestamps (Pitfall 29 territory). In test mode the TestServer
  writes the bus with injected timestamps (MainComponent.cpp:1837-1848), so A would hit its fallback on every
  take there.
- 50 ms is shorter than the legitimate wait with 2048-frame device buffers (about 53 ms, section 2.2), so A would
  fall back routinely on such devices (INFERRED).

### 1.3 Rejected: B, `/api/set_bpm` answers only after a snapshot shows the new bpm (REST only)
- It covers REST set_bpm only. It misses:
  - OSC (UDP, so there is no answer to hold back);
  - MIDI and keyboard Tap/Resync, the TopBar, Link and replay;
  - REST `/api/resync`, which changes no bpm and so needs a different predicate.
- The bpm-equality predicate is wrong in two cases: folded values (240 is applied as 120, BPMTracker.cpp:607)
  and re-sending the same tempo.
- It holds an httplib worker up to 50 ms per call and changes every REST client's latency.
- It fixes the witness script, not the recorder: two concurrent clients (OSC tempo + REST record) still race.

### 1.4 Also considered and rejected
- **Arm later (retry `perfRecord` until the command is applied).** The tap and the provisional save would start
  late: the take would lose its first ~20 ms of audio, and the provisional save would no longer happen at arm.
- **Block the message thread in `perfRecord` until the hop.** It stalls the UI for up to one hop, and hangs
  until the fallback when there is no device.
- **Start on the first tick, then rewrite the "start" anchor when the command lands.** This invents beats the
  tracker never published between the tick and the hop, and stamps already taken disagree with the rewritten
  map.
- **A value predicate (snapshot bpm == requested).** It fails like B's second bullet, and Tap/Resync/auto carry
  no value to check.
- **Relax `checkMetered` for a zero-beat unmetered segment.** It hides symptom 1 for slicing only, leaves the
  grid unknown, and changes the designed unmetered refusal. The diag agrees.

--------------------------------------------------------------------------------------------------------------
## 2. Design

### 2.1 The invariant and its three ends (memory order)
- **Post** (any thread; in the app it is always the message thread): the existing relaxed write of the request,
  THEN `requestSeq_.fetch_add(1, release)`.
- **Latch** (analysis thread, FIRST statement of `runPipeline`): `appliedRequestSeq_ = requestSeq_.load(acquire)`.
  After that come the existing reads: the tempo word (:74), the manual flag (:94), and later the resync counter
  (:352).
  - If the latch reads N, every request write sequenced before raise N happens-before the reads that follow the
    latch. Coherence then forces those reads to see the write or a newer value.
  - So a snapshot published with N reflects every request whose raise brought the sequence to <= N.
  - A Resync posted between :74 and :352 of the same hop is applied but not claimed. That is conservative: the
    recorder waits one more hop.
- **Publish:** `AnalysisThread` copies `appliedRequestSeq()` next to the other tracker fields (:199-209).
- **Read at Record:** `perfRecord` reads `postedRequestSeq()` on the message thread. That is its own latest post,
  because all posts happen on that thread (program order).
- **Compare:** `static_cast<int32_t>(snap.trackerRequestSeq - awaited) >= 0` (C++20 conversion is modular). The
  sequence wraps after 2^32 requests; Link's 2 raises x 30 Hz takes about 2.3 years to wrap.

### 2.2 The start gate (RecorderHost, message thread)
- If the clock has started, tick it as today.
- Otherwise call `startDue(snap, wallNow, sample)`. It returns true when any of these holds:
  - (a) nothing is awaited (nullopt);
  - (b) the snapshot has reached the awaited sequence;
  - (c) `wallNow` - (first waiting tick) >= `kStartWaitFallbackSeconds` (0.1 s). In this case it writes one
    stderr line starting "take start:".
  Otherwise it remembers this tick (for a Stop) and returns false.
- `startClock` calls `clock_.tick(...)` (t = 0 and the "start" anchor) and sets
  `startBeatInBar_ = LOCKED ? beatInBar + beatPhase : -1`, both from the same snapshot.
- **Nothing that stamps with the clock runs before t = 0:** onset markers (baseline at t = 0, Pitfall 30),
  synthesized Decaying ends, and the early-tempo and periodic saves.
- **Tap bookkeeping runs from the first tick after arm, as today:** the self-stop edge, the gap drain and
  framesWritten.
- **Captures between Record and t = 0** (human or REST writes) are stamped with the unstarted clock's defaults
  {t 0, beat 0, sample 0}. That is exactly what already happens in today's arm -> first-tick window
  (PerformanceRecorder.cpp:31-36 uses `clock_->now()`). They replay at the take's start. Only the window's length
  changes, and only when a command is in flight.
- **Stop before t = 0:** `disarm` first starts the clock from the last waiting tick. A take that saw a tick
  therefore always has t = 0.
- **Why 100 ms.** The legitimate wait is at most one device-buffer period + about 2 ms (the `sleep(1)` hop pull)
  + one tick. That is about 21 ms at 512 frames or fewer, about 53 ms at 2048, and about 95 ms at 4096 @ 44.1 kHz
  (INFERRED from AnalysisThread.cpp:86-90 and the 120 Hz tick, MainComponent.h:318). Beyond that the fallback
  reproduces today's stale start, and stderr says so.
- **Test mode:** the analysis thread never starts (MainComponent.cpp:1837-1841), so `perfRecord` passes nothing
  and nothing waits. Eyes behaviour is unchanged.

### 2.3 `startBeatInBar` moves from MainComponent (arm-time bus read) to RecorderHost (the t = 0 snapshot)
- The rule is unchanged: LOCKED -> `beatInBar + beatPhase`, else -1. It now describes the same instant as beat 0.
- `ArmOptions::startBeatInBar` is deleted. Its only writer is MainComponent.cpp:5281 and its only reader is
  RecorderHost.cpp:175 (grep); no test sets it.
- The recording layer tests LOCKED through a new `FeatureSnapshot::kTrackerLocked`. RecorderHost's test targets
  do not link aubio (tests/CMakeLists.txt:1342-1390), so they cannot include BPMTracker.h. A static_assert in
  AnalysisThread.cpp ties the constant to `BPMTracker::STATE_LOCKED`.

--------------------------------------------------------------------------------------------------------------
## 3. Exact changes, by file:line

### 3.1 `src/analysis/FeatureSnapshot.h`
After :137 (`totalBeatCount`):
```cpp
    // s-rta-0928 take start (Pitfall 48): BPMTracker::appliedRequestSeq() of the hop that published this
    // snapshot. Every tempo / Tap / Resync / manual-mode request whose post raised the tracker's request
    // sequence to <= this value is reflected in bpm, beatPhase, totalBeatCount, trackerState and beatInBar
    // here. Monotonic, wraps at 2^32 -- compare with a signed difference. 0 before any request and in test
    // mode (no analysis thread). A sync token, not an audio feature: no UI readout.
    uint32_t trackerRequestSeq = 0;

    // trackerState value meaning LOCKED (== BPMTracker::STATE_LOCKED; static_assert in AnalysisThread.cpp): the
    // recording layer tests the lock without including BPMTracker.h (aubio).
    static constexpr uint8_t kTrackerLocked = 2;
```
After :181, add:
`static_assert(offsetof(FeatureSnapshot, trackerRequestSeq) == 328, "trackerRequestSeq must immediately follow totalBeatCount (offset 324 + 4 bytes) in the free tail tier -- if this fails, re-audit the layout");`

Keep :182-184 `sizeof == 384`. Change its message to "...the next fields are free from offset 332 up to 384 --
FeatureBus::kSnapshotWords must be 96". FeatureBus.h is unchanged; its static_asserts still hold.

### 3.2 `src/analysis/BPMTracker.h` / `.cpp`
In `.h`, public, after :165:
```cpp
    // s-rta-0928 (Pitfall 48): the request SEQUENCE. Every request above -- setManualBPM, followExternalTempo,
    // setManualMode, requestResync -- raises it by one (release) AFTER its own write; runPipeline() latches it
    // (acquire) as its FIRST statement, BEFORE it reads any request. A snapshot published with
    // appliedRequestSeq() == N therefore reflects every request that raised the sequence to <= N (a later one
    // may be reflected too -- never the reverse). "Everything sent before X" = FeatureSnapshot::trackerRequestSeq
    // >= postedRequestSeq() read at X (signed difference; wraps at 2^32).
    uint32_t postedRequestSeq() const { return requestSeq_.load(std::memory_order_relaxed); }   // any thread
    uint32_t appliedRequestSeq() const { return appliedRequestSeq_; }                           // analysis thread
```
In `.h`, private, after :294:
```cpp
    // === s-rta-0928: request sequence (see postedRequestSeq) ===
    std::atomic<uint32_t> requestSeq_{0};   // raised by every request, release, AFTER its write
    uint32_t appliedRequestSeq_ = 0;        // analysis thread: latched at the start of runPipeline
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "the request sequence must be lock-free");
    void raiseRequestSeq() { requestSeq_.fetch_add(1, std::memory_order_release); }
```
In `.cpp`:
- :69-73 `runPipeline`: new FIRST statement `appliedRequestSeq_ = requestSeq_.load(std::memory_order_acquire);`
  with the comment "BEFORE any request is read -- never move it below (Pitfall 48)".
- :555-558 `requestResync`: `raiseRequestSeq();` after the `fetch_add`.
- :586-598 `postTempoRequest`: `raiseRequestSeq();` after the CAS loop. This covers setManualBPM and
  followExternalTempo. Their `bpm <= 0` early returns post nothing and raise nothing.
- :625-628 `setManualMode`: `raiseRequestSeq();` after the store.
- Nothing else changes: application points, coalescing and realign rules stay as they are.

### 3.3 `src/analysis/AnalysisThread.cpp`
- After :21: `static_assert(BPMTracker::STATE_LOCKED == FeatureSnapshot::kTrackerLocked, "RecorderHost reads trackerState through FeatureSnapshot::kTrackerLocked");`
- After :208: `snap->trackerRequestSeq = bpmTracker_->appliedRequestSeq();   // s-rta-0928: the requests this snapshot reflects (Pitfall 48)`

### 3.4 `src/recording/RecorderClock.h`
After :50, add `bool started() const { return haveTicked_; }   // s-rta-0928: false until the first tick (t = 0)`.
Nothing else in the clock changes (RoutineEngine's clock is unaffected).

### 3.5 `src/recording/RecorderHost.h` / `.cpp`
**`.h` changes:**
- `ArmOptions` :110-113: delete `startBeatInBar` and its comment. Add:
```cpp
        // s-rta-0928 take start (Pitfall 48): BPMTracker::postedRequestSeq() read at Record. The take's t = 0 --
        // its "start" anchor, beat 0 and meta.startBeatInBar -- is the first tick whose snapshot carries every
        // tempo / Tap / Resync / manual-mode request sent before Record (FeatureSnapshot::trackerRequestSeq >=
        // this), or kStartWaitFallbackSeconds after the first tick. nullopt = the first tick (headless tests,
        // test mode). The tap and the provisional save stay at arm.
        std::optional<uint32_t> startAfterTrackerRequest;
```
- `tick()` doc :142-155: (1) becomes "the start gate (startAfterTrackerRequest, else the fallback), then
  clock.tick". (2) gains "onset markers, Decaying ends and saves only once the clock has started".
- :274, next to `kCheckpointSeconds`: `static constexpr double kStartWaitFallbackSeconds = 0.1;   // s-rta-0928: the longest t = 0 waits after the first tick`
- :287-293 `takeForSave` doc: "startBeatInBar (from the t = 0 snapshot; -1 before t = 0)".
- New private members:
  `bool startDue(const FeatureSnapshot& snap, double wallNow, uint64_t sample);`
  `void startClock(const FeatureSnapshot& snap, double wallNow, uint64_t sample);`
- :316: rename `armedStartBeatInBar_` to `startBeatInBar_`, with the comment "s-rta-0928: where beat 0 sits in
  its bar, from the t = 0 snapshot (LOCKED only); -1 until t = 0 or unknown". It is used at 3 sites.
- After :326:
```cpp
    // s-rta-0928 take start (Pitfall 48; ArmOptions::startAfterTrackerRequest). Message thread only.
    std::optional<uint32_t> startAfterTrackerRequest_;
    std::optional<double> startWaitSinceWall_;           // wallNow of the first waiting tick
    std::unique_ptr<FeatureSnapshot> startWaitSnap_;     // the last waiting tick, for a Stop before t = 0
    bool startWaitSeen_ = false;
    double startWaitWall_ = 0.0;
    uint64_t startWaitSample_ = 0;
```
  A `unique_ptr` keeps FeatureSnapshot forward-declared (:17). The destructor is already out of line
  (RecorderHost.cpp:112). Do NOT hold a FeatureSnapshot by value: `alignas(64)` would over-align RecorderHost
  and MainComponent.
- :347-378 `onsetCountBaseline_` comment: the baseline is the snapshot at t = 0 (the clock's first tick), not
  "the first snapshot after arm". The KNOWN LIMITATION's window is now "Record -> t = 0": one tick, or 1-2 hops
  when a tempo command was in flight.

**`.cpp` changes:**
- Add `#include <iostream>`.
- `takeForSave` :160 now uses `startBeatInBar_`.
- `arm` :175 becomes:
  `startBeatInBar_ = -1.0; startAfterTrackerRequest_ = opts.startAfterTrackerRequest; startWaitSinceWall_.reset(); startWaitSeen_ = false;`
- `arm` :268-271 comment: the provisional save has an empty tempo map and an unknown startBeatInBar until t = 0.
  The early tempo save at the start tick writes both when the start is metered (:498-515).
- `tick` :433-516, recording block:
```cpp
    if (recording_)
    {
        // Fix plan F1: the clock ticks only while a take records -- arm() re-creates it.
        // s-rta-0928 take start (Pitfall 48): its FIRST tick (t = 0, the "start" anchor, meta.startBeatInBar)
        // waits for the snapshot that carries every tracker request sent before Record -- or the fallback.
        if (clock_.started())
            clock_.tick(snap, wallNow, sampleForClock);
        else if (startDue(snap, wallNow, sampleForClock))
            startClock(snap, wallNow, sampleForClock);

        if (tapWasStarted_) { /* :440-466 unchanged -- tap facts are clock-free and run from arm */ }

        // Everything below stamps with the clock: nothing before t = 0 (a marker or a synthesized end would
        // carry the unstarted clock's sample 0).
        if (clock_.started())
        {
            /* :468-492 onset markers, :494 synthesizeIdleDecayingEnds, :496-515 saves -- bodies unchanged */
        }
    }
```
- New functions:
```cpp
bool RecorderHost::startDue(const FeatureSnapshot& snap, double wallNow, uint64_t sample)
{
    if (!startAfterTrackerRequest_.has_value())
        return true;                                            // nothing awaited: the first tick is t = 0
    // Signed difference: the sequence wraps at 2^32 (C++20: the conversion is modular).
    if (static_cast<int32_t>(snap.trackerRequestSeq - *startAfterTrackerRequest_) >= 0)
        return true;                                            // carries every request sent before Record
    if (!startWaitSinceWall_.has_value())
        startWaitSinceWall_ = wallNow;
    if (wallNow - *startWaitSinceWall_ >= kStartWaitFallbackSeconds)
    {
        std::cerr << "[RecorderHost] take start: tracker request " << *startAfterTrackerRequest_
                  << " not in the analysis snapshot after "
                  << static_cast<int>(kStartWaitFallbackSeconds * 1000.0) << " ms (it carries "
                  << snap.trackerRequestSeq << "); the take starts from the latest snapshot" << std::endl;
        return true;
    }
    if (!startWaitSnap_)
        startWaitSnap_ = std::make_unique<FeatureSnapshot>();
    *startWaitSnap_ = snap;                                     // for a Stop before t = 0 (disarm)
    startWaitWall_ = wallNow;
    startWaitSample_ = sample;
    startWaitSeen_ = true;
    return false;
}

void RecorderHost::startClock(const FeatureSnapshot& snap, double wallNow, uint64_t sample)
{
    clock_.tick(snap, wallNow, sample);   // the first tick: t = 0 and the "start" anchor (RecorderClock.cpp:16-29)
    // plan-routines 3.6's rule, now from the SAME snapshot as beat 0 (it was MainComponent's arm-time read).
    startBeatInBar_ = snap.trackerState == FeatureSnapshot::kTrackerLocked
        ? static_cast<double>(snap.beatInBar) + static_cast<double>(snap.beatPhase)
        : -1.0;
}
```
- `disarm`, directly after the `!recording_` check (:297-301), so the clock exists before
  `recorder_.stop` / `takeForSave` read it:
```cpp
    // s-rta-0928: a take stopped while its t = 0 still waited starts from the last tick it saw -- a take that
    // saw a tick always has a "start" anchor (one stopped before any tick keeps an empty map, as before).
    if (!clock_.started() && startWaitSeen_ && startWaitSnap_)
    {
        std::cerr << "[RecorderHost] take start: stopped before tracker request " << startAfterTrackerRequest_.value_or(0)
                  << " reached the analysis snapshot; started from the last tick" << std::endl;
        startClock(*startWaitSnap_, startWaitWall_, startWaitSample_);
    }
```
  Next to :380-387, reset `startWaitSeen_ = false; startWaitSinceWall_.reset();`.

### 3.6 `src/MainComponent.cpp`
Replace `perfRecord` :5276-5282 (the arm-time bus read) with:
```cpp
    // s-rta-0928 take start (Pitfall 48): every tempo / Tap / Resync / manual-mode command that ran before this
    // Record is a BPMTracker request the analysis thread applies at its next hop. The take's t = 0 -- and
    // meta.startBeatInBar (plan-routines 3.6: where beat 0 sits in its bar), read from the same snapshot --
    // waits for the snapshot that carries them (RecorderHost::tick). Test mode never starts the analysis
    // thread (the TestServer writes the bus): nothing to wait for there.
    if (!testMode_)
        if (auto* tracker = analysisThread_.getBpmTracker())
            armOpts.startAfterTrackerRequest = tracker->postedRequestSeq();
```
`testMode_` is at MainComponent.h:570. Nothing else in MainComponent changes: `applyTempoCommand`, including its
tempo-point capture, is untouched, because the sequence lives in the tracker.

### 3.7 Tests
- New file `tests/test_tempo_start.cpp` (section 5).
- `tests/CMakeLists.txt`: append after :2460. The notebook's "CMake append conflicts" rule applies: keep both
  blocks.
```cmake
# --- test_tempo_start (s-rta-0928 take start race, .harmony/.reports/s-rta-0928/plan-tempo.md): a tempo / Tap /
# Resync command sent just before Record must be in the take's "start" anchor and bar grid -- the
# FeatureSnapshot::trackerRequestSeq handshake (Pitfall 48). test_recorder_host's link set (:1343-1359) +
# RoutineSlice.cpp + the REAL BPMTracker (aubio) that scripts the race; two [law] cases scan
# AnalysisThread.cpp and MainComponent.cpp, which no ctest compiles.
add_executable(test_tempo_start
    test_tempo_start.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/Program.cpp
    ${SRC_DIR}/recording/Player.cpp
    ${SRC_DIR}/recording/RecorderClock.cpp
    ${SRC_DIR}/recording/PerformanceRecorder.cpp
    ${SRC_DIR}/recording/AudioTap.cpp
    ${SRC_DIR}/recording/AudioStore.cpp
    ${SRC_DIR}/recording/RecorderHost.cpp
    ${SRC_DIR}/recording/PerfStateCapture.cpp
    ${SRC_DIR}/recording/RoutineSlice.cpp
    ${SRC_DIR}/analysis/BPMTracker.cpp
)
target_include_directories(test_tempo_start PRIVATE ${SRC_DIR})
target_link_libraries(test_tempo_start PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_cryptography
    Aubio::Aubio
)
target_compile_definitions(test_tempo_start PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
    AUDIODNA_SRC_DIR="${SRC_DIR}"
)
if(NOT MSVC)
    target_compile_options(test_tempo_start PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_tempo_start)
catch_discover_tests(test_tempo_start)
```
  If a link error appears, add what `test_routine` links (tests/CMakeLists.txt:885-898).
- `tests/fixtures/take_v3_start_bpm0.json`: copy
  `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tempo0/fixtures/harmony-tempo.adna-take/take.json`.
  It exists today (4399 B) and holds a real raced take from 2026-09-27 20:48:51 with no private paths.
  - If the file is gone, hand-build it: format "audiodna-take", version 3, minReader 3,
    features ["lanes","tempoMap","checkpoint0"], meta {recordedAt, app "0.1.0", duration 1.546147208326147,
    durationBeats 3.0720001347363}, audio {segments []}, and tempoMap:
    `[{"t":0.0,"beat":0.0,"sample":128512,"bpm":0.0,"why":"start"},{"t":0.01203954165976,"beat":0.0,"sample":129536,"bpm":120.0,"why":"lock"}]`.
  - Leave markers and lanes empty.

### 3.8 `.harmony/probe-tempo-start.sh` (new, section 6)
### 3.9 Docs (texts in section 12)
- `docs/claude/pitfalls.md`: add Pitfall 48 after :103.
- `CLAUDE.md`: index line 48 after :229.
- `docs/claude/recording.md`: a "Take start" paragraph after :11.
- `docs/claude/architecture.md`: a row after :29.
- `docs/claude/analysis.md`: a row after :39.

--------------------------------------------------------------------------------------------------------------
## 4. MUST HOLD -> how it holds
- **A take started while the tracker is truly unlocked stays an unmetered start.**
  - With nothing in flight, the gate opens on the first tick, as today (G1).
  - A command in flight that does not lock (a Resync while unlocked, or "auto") starts after it, on a snapshot
    that is still unmetered, so the start is bpm 0 as designed.
  - The gate waits for the COMMAND, never for a lock.
  - RecorderClock's unmetered rules (RecorderClock.cpp:30-52) are untouched.
- **Device absent, stalled audio or no hops still yield t = 0.**
  - The fallback starts the take 100 ms after the first tick (G3).
  - A Stop before t = 0 starts it from the last waiting tick (G4).
  - A take stopped before any tick keeps an empty map, as today.
- **audio:true stays aligned.**
  - The tap still starts at arm (RecorderHost.cpp:228).
  - Events and audio align through absolute sample stamps (0.4).
  - Only the unused offset between the WAV's first frame and the "start" anchor grows, and only when a command
    is in flight.
  - Overdub keeps its asset-frame sample domain (RecorderHost.cpp:425-430).
- **The provisional save at arm is unchanged** (RecorderHost.cpp:268-288). It now carries an unknown
  startBeatInBar. It already had an empty tempo map, and the early tempo save at the start tick writes both
  when the start is metered (:498-515).
- **Old takes load and replay identically.** Take, TempoMap, Program, Player, RoutineSlice and RoutineEngine do
  not change (format v3). F1 pins a real raced take's load and slice behaviour.
- **Zero allocation and no locks on the audio or analysis threads.**
  - The audio thread is untouched.
  - The analysis thread gains one acquire load per hop and one plain store.
  - The only allocation is `startWaitSnap_`, on the message thread, once per host.
- **Pitfall 29:** the handshake does no sample arithmetic (a sequence number plus tick wall time). Anchors keep
  device-domain samples.
- **Pitfall 30:** the onset baseline moves to t = 0 and still consumes `onsetCount` deltas (G6). No marker is
  written before t = 0, where it would carry sample 0.
- **Pitfall 32:** `downbeatDetected` is not read. `startBeatInBar` reads `beatInBar + beatPhase`, as today.
- **Pitfall 42:** the clock still integrates `totalBeatCount + beatPhase`. Beat 0 is the raw value of the start
  snapshot, so after a Resync beat 0 IS the new downbeat (E2).

--------------------------------------------------------------------------------------------------------------
## 5. ctests -- RED first, driving real code (all in `tests/test_tempo_start.cpp`)

**Helpers.** Copy these from test_recorder_host.cpp:37-150: TempDir, makeComposition, a minimal FakeDispatch
(capturePerfState + notify), and a plain `AudioTap tap;`. Load the saved take with
`Take::load(folder/"take.json", stats)`. Slice with `EffectLibrary lib; lib.registerDefaults();` as the diag's
scratch test did.

- `publishHop(BPMTracker& tr, uint64_t& ts)` mirrors AnalysisThread.cpp:199-209:
  - `tr.processRawBPM(0,0,false)`, then `tr.feedDownbeatFeatures(0,0,0,0)`, then `ts += 512`;
  - copy bpm, beatPhase, trackerState, beatInBar, barPhase, downbeatDetected, barCount, totalBarCount,
    resyncBarOrigin, totalBeatCount and phrasePhase into a `clear()`ed snapshot.
  - GREEN (commit 4) adds one line: `s.trackerRequestSeq = tr.appliedRequestSeq();`
- `armLikePerfRecord(host, comp, tap, folder, busAtArm, tr)` builds the ArmOptions (audio false, takeFolder) the
  way perfRecord does.
  - RED body (today's rule, MainComponent.cpp:5276-5282):
    `if (busAtArm.trackerState == BPMTracker::STATE_LOCKED) o.startBeatInBar = busAtArm.beatInBar + busAtArm.beatPhase;`
  - GREEN body: `o.startAfterTrackerRequest = tr.postedRequestSeq();`
  - These two helper lines are the test's mirror of the two production wiring lines that no ctest compiles. L1
    and L2 pin those production lines in source.
- `drive(...)` runs hops every 512/48000 s and ticks every 1/120 s, in time order, up to a given wall time.
  `delivered` = 4,800,000 + round((wall - w0) * 48000).
- `snapOf(seq, bpm, locked, beatInBar, phase, onsetCount = 0)` builds synthetic snapshots for the G cases.

### 5.1 Commit 1 -- RED on UNMODIFIED 6db8d67 (these cases use only today's API)
Use CHECK, not REQUIRE, so one run prints every RED value.

**E1 `[tempo-start][race] a set_bpm sent just before Record is in the take's start anchor and bar grid`**
- Set up `BPMTracker tr(512, 1024, 48000)` and publish three unlocked hops (bpm 0, SEARCHING).
- Message thread, back to back: `tr.setManualMode(true); tr.followExternalTempo(120.0f);` (the `"link"` action),
  then `armLikePerfRecord`.
- `host.tick(bus, 100.0005)`: a stale tick, run 03's order.
- `bus = publishHop(...)`: the applying hop (bpm 120, LOCKED).
- `host.tick(bus, 100.0088333)`, then `host.tick(bus, 100.0171667)`.
- CHECK `host.status().t == Approx(1/120).margin(1e-6)`. RED: 0.0166667.
- `drive` to 102.0, `disarm`, load. Then CHECK:
  - `tempo.a.size() == 1` (RED: 2, start + lock);
  - `a[0].why == "start"`;
  - `a[0].bpm == Approx(120)` (RED: 0);
  - `meta.startBeatInBar == Approx(512.0/24000.0)` (RED: -1);
  - `sliceRoutine(take, {0.0, 4.0, "r", true, folder}, lib).error.empty()` (RED: "this stretch has no beat; the
    tempo was unknown while it was recorded");
  - `takeBeatOfBar(take, 1) == Approx(4.0 - 512.0/24000.0)` (RED: 0).

**E2 `[tempo-start][race] a Resync sent just before Record makes beat 0 the new downbeat`**
- Manual 120 (`setManualMode(true)` + `followExternalTempo(120)`). Publish hops until `tr.beatInBar() == 2` and
  `tr.beatPhase() >= 0.55`; this is deterministic in the manual regime (BPMTracker.cpp:94-99, :255-271).
- `tr.requestResync()`, then `armLikePerfRecord`.
- `host.tick(bus, 200.0005)` (stale), then `bus = publishHop(...)` (the resync hop: beatInBar 0, beatPhase 0,
  BPMTracker.cpp:560-572), then `host.tick(bus, 200.0088333)`.
- CHECK `host.status().beat == Approx(0).margin(1e-9)`. RED: about 0.4 (1 - the stale phase).
- `drive` to 202.0, `disarm`, load. Then CHECK:
  - `meta.startBeatInBar == Approx(0).margin(1e-9)` (RED: about 2.6);
  - `takeBeatOfBar(take, 1) == Approx(0).margin(1e-9)` (RED: about 1.4).

**F1 `[tempo-start][old-take] a raced take recorded before the fix still loads and behaves identically`**
- Load `take_v3_start_bpm0.json`. CHECK:
  - the two anchors are exactly as in 3.7;
  - `meta.startBeatInBar == -1`;
  - `tempo.beatAt(0.5) == Approx((0.5 - 0.01203954165976) * 2)`;
  - `tempo.tAt(1.0) == Approx(0.51203954165976)`;
  - slice [0,2) refused with the unmetered message (designed; the fix is forward-only);
  - slice [1e-6,2) accepted;
  - `takeBeatOfBar(take, 1) == 0`.
- GREEN on main and after. It is a guard; its teeth are any accidental change to load, TempoMap or RoutineSlice.

Expected at commit 1: E1 and E2 FAIL with the values above, and F1 PASSES. Record the output verbatim.

### 5.2 Commit 3 -- the analysis side (written first; compile-RED, then GREEN)
**B1 `[bpm][request-seq] every request raises the posted sequence; the next hop latches it`**
- Start: posted 0 / applied 0.
- `followExternalTempo(120)`: posted 1, applied 0, `bpm() == 0`. Then a hop: applied 1, bpm 120.
- `setManualMode(true)`: posted 2. Then a hop: applied 2.
- `setManualBPM(100)`: posted 3. Then a hop: applied 3, bpm 100, beatPhase == 512/28800 (a realign, then one
  manual advance).
- `requestResync()`: posted 4. Then `processRawBPM`: applied 4. Then `feedDownbeatFeatures`: beatInBar 0,
  beatPhase 0.
- `setManualMode(false)`: posted 5. Then a hop: applied 5.

**B2 `[bpm][request-seq] a request posted after the hop's latch is claimed by the next hop, never this one`**
- `processRawBPM` (latch L), then `followExternalTempo(90)`, then `feedDownbeatFeatures`: `applied == L` and the
  bpm is unchanged. The next `processRawBPM` gives applied L+1 and bpm 90.
- `processRawBPM` (latch M), then `requestResync()`, then `feedDownbeatFeatures`: the resync IS applied
  (beatInBar 0) but `applied == M` (conservative).

**L1 `[tempo-start][law] AnalysisThread publishes the request sequence before it publishes the snapshot`**
- Load `AUDIODNA_SRC_DIR "/analysis/AnalysisThread.cpp"`.
- The offset of `snap->trackerRequestSeq = bpmTracker_->appliedRequestSeq();` must be > 0 and less than the
  offset of `featureBusWriter_.publishWrite();`.

### 5.3 Commit 4 -- the recorder side
First add ONLY the API: the `ArmOptions::startAfterTrackerRequest` field (ignored), delete
`ArmOptions::startBeatInBar` and set `startBeatInBar_ = -1` at arm, add `kStartWaitFallbackSeconds`, and replace
the perfRecord lines. Write the cases below, run them, and record the RED. Then implement 3.4/3.5 and get GREEN.
Commit once.

**G1 `[tempo-start][gate] nothing in flight -> t = 0 on the first tick; startBeatInBar from that tick`**
- Section A: `startAfterTrackerRequest` is nullopt.
- Section B: `startAfterTrackerRequest = 7`, and the first tick has seq 7.
- In both: `tick(snapOf(seq, 120, locked, 3, 0.84), 5.0)`, then `tick(.., 5.05)`, then
  `status().t == Approx(0.05)`. After disarm: `a[0] == {start, 120}` and `startBeatInBar == Approx(3.84)`.
- RED on the stub: startBeatInBar -1.

**G2 `[tempo-start][gate] waits for the snapshot that carries the request`**
- `startAfterTrackerRequest = 5`.
- `tick(snapOf(4, 0, unlocked), 10.000)`, then `tick(.., 10.008)`: `status().t == 0`. RED: 0.008.
- `tick(snapOf(5, 120, locked, 1, 0.25), 10.017)`, then `tick(.., 10.025)`: `status().t == Approx(0.008)`.
- After disarm: `a.size() == 1`, `a[0] == {start, 120}`, `startBeatInBar == Approx(1.25)`.

**G3 `[tempo-start][fallback] no hop carries the request -> t = 0 by the fallback`**
- `startAfterTrackerRequest = 5`. Ticks with seq 4, bpm 0 at 10.0, 10.05 and 10.09: `status().t == 0` after
  each (RED: 0.05).
- The tick at 10.125 (seq 4) starts the take by the fallback. Never test the exact 0.1 s boundary in doubles:
  10.1 - 10.0 < 0.1.
- A tick at 10.135 with seq 5, bpm 120, locked writes "lock".
- After disarm: `a == [start(t 0, bpm 0), lock(t Approx(0.010), bpm 120)]` and `startBeatInBar == -1`.

**G4 `[tempo-start][stop] a Stop before t = 0 still gives the take its start`**
- `startAfterTrackerRequest = 5`. Ticks with seq 4, bpm 128, locked, beatInBar 2, phase 0.5 at 20.000 and 20.008.
  Then disarm.
- CHECK `a.size() == 1`, `a[0] == {start, bpm 128}`, `startBeatInBar == Approx(2.5)`, `meta.duration == 0`.
- RED on the stub: duration 0.008 and startBeatInBar -1.

**G5 `[tempo-start][wrap] the sequence compare survives 2^32`**
- `startAfterTrackerRequest = 0xFFFFFFFE`.
- A tick with seq 0xFFFFFFFD at 30.000, then another at 30.008: `status().t == 0`.
- A tick with seq 1 (wrapped) at 30.016 starts the take. The tick at 30.024 gives `t == Approx(0.008)`.

**G6 `[tempo-start][onset] onset markers count from t = 0 (Pitfall 30)`**
- `onsetMarkers = true`, `startAfterTrackerRequest = 5`.
- `tick(seq 4, onsetCount 10, 40.000)`, then `tick(seq 5, bpm 120, locked, onsetCount 11, 40.008)` (the start;
  baseline 11), then `tick(seq 5, onsetCount 12, 40.016)`.
- After disarm: `markers.size() == 1`, `markers[0].s.t == Approx(0.008)`, and `markers[0].s.sample` is the sample
  passed at 40.016 (not 0). RED on the stub: 2 markers.

**L2 `[tempo-start][law] perfRecord arms with the request sequence, not an arm-time bus read`**
- Take MainComponent.cpp's text between `std::string MainComponent::perfRecord(` and
  `std::string MainComponent::perfStop(`.
- It must contain `postedRequestSeq()` and `testMode_`, and must contain neither `startBeatInBar` nor
  `getFeatureBus().read()`.

Commit 4 also swaps the two helper lines, turning E1 and E2 GREEN.

### 5.4 Teeth (after GREEN; each mutation reverted; record the RED)
- T1: make `startDue` `return true;` first. E1, E2 and G2-G6 go RED.
- T2: delete the latch line in `runPipeline`. B1 goes RED. In E1 the gate opens only at the fallback, so the
  `status().t == 1/120` check goes RED.
- T3: delete the AnalysisThread copy. L1 goes RED.
- T4: delete the perfRecord block. L2 goes RED.
- NOT testable single-threaded: raising the sequence BEFORE the write, or latching after a request read.
  Comments at both ends and Pitfall 48 are the only guard (section 10, R2).

--------------------------------------------------------------------------------------------------------------
## 6. Live -- `.harmony/probe-tempo-start.sh [OUTDIR]`

**Conventions.** Production mode (NO --test-mode), port 7070. Copy probe-resync.sh:1-80 and
probe-beatclock.sh:1-60:
- the lock gate (`/tmp/audiodna-live.lock/owner`, `AUDIODNA_LOCK_OWNER`);
- `adna_pids` / `adna_running` / `adna_kill` (ucomm-based);
- `open -g --stdout/--stderr` (clear the logs first) and the health loop;
- ok/no counters, the final "PASS n / FAIL m" line, and exit 1 on any FAIL;
- osascript quit first, `adna_kill` only as the last resort.

It never opens the Output window (no output endpoint is used) and takes no screen capture. It records only
`audio:false` takes and deletes exactly the takes it created (`tstart-<runid>-*` under
`~/Documents/Audio-DNA/Takes/`, MainComponent.cpp:5216-5220, :5263). It writes no composition file.

**Env:**
- `TEMPOSTART_APP` (full bundle path) or `TEMPOSTART_BUILD_DIR` (default build-lane);
- `TEMPOSTART_CYCLES` (default 20);
- `TEMPOSTART_STALL_MS` (default 40; 0 = tight pair only);
- `TEMPOSTART_WITNESS_RUNS` (default 0).

**Client** (python3 stdlib, embedded): pre-connected raw sockets to 127.0.0.1:7070, with `Connection: close`,
JSON bodies and Content-Length. The PAIR:
1. Optionally, `POST /api/debug/stall_message_thread {"ms": S}` and wait for its answer.
2. `POST` the command and wait for its answer. ApiServer queues the callAsync BEFORE it answers
   (ApiServer.cpp:768-775, :784-789, :1592), so the answer proves the command is queued.
3. At once, `POST /api/perf/record {"name": N, "audio": false}`.

Print the command -> record gap in ms.

**Mode.** At start, POST the stall hook with `{"ms": 1}`. The hook is TEST-ONLY, compiled with
`AUDIODNA_BUILD_TEST_SERVER=ON` (ApiServer.cpp:283-288), and main's build has it (build/CMakeCache.txt:31).
- If it answers ok, the mode is **stall-assisted**. Both requests queue behind a 40 ms message-thread sleep and
  run back to back after it: the diag's run-03 order, made nearly certain.
- If it returns 404, the mode is **tight pair** (sub-millisecond gap, natural timing).

**Per cycle:**
1. The pair.
2. `sleep 0.3`.
3. `POST /api/perf/stop {}`.
4. Poll `/api/perf/status` until `recording` is false (at most 3 s).
5. Read `take.json`.

A cycle takes about 0.5 s; W1-W4 take about 1 minute including launch.

**Rows:**
- **W1** First take of a fresh launch, at the witness's timing (+2 s after health). Precondition: `/api/bpm` bpm
  is 0; otherwise W1 is SKIP (the tracker was already metered). Run Pair(set_bpm 120) and stop after 0.5 s.
  PASS iff:
  - tempoMap[0] == {why "start", bpm 120 +- 0.05};
  - there is no "lock" anchor;
  - meta.startBeatInBar is in [0, 4);
  - then `POST /api/perf/load {"folder": <take>}` and
    `POST /api/routine/save {"name":"TStart","fromBeat":0,"toBeat":4,"slot":7}` give `/api/routine/status`
    `lastSaved.slot == 7` and `lastError == ""` within 1 s. This is the user-facing harm; on the old app
    lastError is "this stretch has no beat; ...".
- **W1b liveness.** `/api/bpm` bpm == 120 and totalBeatCount advances by >= 1 in 1 s. Otherwise FAIL "no analysis
  hops (no audio device?)": every other row would be meaningless.
- **W2** 20 cycles of set_bpm alternating 90 / 150 (never the current value). PASS iff tempoMap[0].bpm equals
  the value sent, 20/20. Print per cycle: the gap, the start bpm, and whether a "bpm" anchor exists at t < 0.05
  (the old correction signature).
- **W3** set_bpm 120, sleep 1, then 20 cycles of Pair(resync). PASS iff meta.startBeatInBar is in [0, 0.25) and
  tempoMap[0].bpm == 120, 20/20. 0.25 allows 11 hops of tick lateness at 120 BPM; the fixed app gives 0, 0.021
  or 0.043.
- **W4** `grep -c "take start:"` on the app's stderr == 0. This proves the handshake, not the fallback, started
  every take.
- **W6** (opt-in, `TEMPOSTART_WITNESS_RUNS=20`) is the canonical witness, repeated per run: quit, fresh `open -g`,
  health, sleep 2, curl set_bpm 120, curl record audio:false, sleep 1.5, stop, sleep 1, read the final
  take.json. Count start-bpm-0 (tempoMap[0].bpm == 0) and unknown grids (meta.startBeatInBar absent). PASS iff
  both are 0/N. About 15 s per run.

**RED on the CURRENT app (mandatory, BEFORE the fix):**
- Build 6db8d67 unmodified in the lane worktree FIRST, configured like main (`-DCMAKE_BUILD_TYPE=Release
  -DAUDIODNA_BUILD_TEST_SERVER=ON`). Copy the bundle aside and run W1-W4 on it with `TEMPOSTART_APP`.
- Expected per-cycle failure rates (INFERRED from 0.1: 120 Hz ticks, 10.67 ms hops, FIFO message queue):

  | mode | W1 | W2 | W3 |
  |---|---|---|---|
  | stall-assisted | fails: grid unknown ~100%, start bpm 0 ~85% | ~85% | ~95% |
  | tight pair | not modelled | ~60% | ~95% |

  - W2 (stall): after the stall the first tick lands within ~1 ms of arm, and the applying hop 0-10.7 ms later.
  - W3: the arm-time read always precedes the applying hop, and a stale bar position lands in [0, 0.25) only
    ~6% of the time.
- **Credibility rule.** A row counts as a gate only if it FAILED >= 5 of 20 on the unmodified app. A row that did
  not is reported as "no refutation power", never as a pass.
- **Power.** Even at a pessimistic 15% per-cycle rate, P(20/20 pass on the old app) = 0.85^20 = 4%. At the
  modelled rates it is < 1e-10.
- W6 is not re-run on the old app: the diag's 34 runs are its RED (4 start-bpm-0, 9 unknown grids).

**GREEN on the lane app:** W1 PASS, W2 20/20, W3 20/20, W4 0. Run W1-W4 three times in stall mode, plus once with
`TEMPOSTART_STALL_MS=0` (natural timing). W6 with 20 runs: 0/20 start-bpm-0 and 0/20 unknown grids.

--------------------------------------------------------------------------------------------------------------
## 7. Existing probes to re-run -- t = 0 moves for NONE of them (VERIFIED)
| probe | records? | tempo command in flight at Record? | why re-run | must still show |
|---|---|---|---|---|
| probe-routines.sh (ROUTINES_RECORD_PAUSE 0 and 1.8) | yes, :637 | no -- set_bpm at :620, >= 5.3 s earlier | take/slice/routine path; startBeatInBar's new source | every row PASS: "tempoMap has N anchor(s), all with a tempo", "meta.startBeatInBar present and in [0, 4)", "every scheduled move went out within 80 ms", grid / loop / stack / from-now windows unchanged |
| probe-step3.sh | yes, :318-321 and later sections | no -- set_bpm(128) is mid-take, :343 | recording lifecycle | provisional take.json at arm + 2 s; tempoMap first anchor "start" + a 128 anchor; T2 mean offset in [0, 60] ms, drift bound, p95 <= 15 ms, >= 90% coverage. T2 is computed in the sample domain (`marker.sample - firstSample`, :505-529), so it cannot see t = 0 anyway |
| probe-beatclock.sh, probe-downbeat-level.sh, probe-resync.sh | no | -- | BPMTracker's request path and AnalysisThread's publish changed | every row as before: b1/b2/b3 beat continuity; the level contract and totalBarCount edges; the first poll >= 60 ms after /api/resync is the new downbeat, and the LFO restarts |
| probe-manual-bpm.sh | yes, :117 -- BEFORE any tempo command (:204) | no | the request path (S rows: a tempo value never realigns; R rows: Resync) | all rows as before |
| probe-finalize-loop.sh, probe-onset-render.sh | yes, no tempo commands | no | the restructured arm/tick/disarm | as before |
| Harmony's tempo-witness.sh (scratchpad) | yes | YES -- this is the race | -- | GREEN every run |

For every re-run, also grep its app stderr for "take start:" and expect 0. A hit means a gate stayed closed
where nothing was in flight.

--------------------------------------------------------------------------------------------------------------
## 8. TempoMap::sampleAt -- FILED, not fixed in this lane
**Defect** (VERIFIED, TempoMap.cpp:81-108):
- A single-anchor map has no rate: rate 0, so `sample(t)` stays at the anchor's sample.
- Otherwise it extrapolates the last segment's delivered-sample slope, which a short segment makes wrong (the
  diag's 12 ms start/lock pair gives 85,050 samples/s).

**Reachability** (VERIFIED by reading):
- Its only caller is Program.cpp:451, the Sample-clock fallback for a gesture WITHOUT parallel stamps
  (Program.cpp:502-511).
- The recorder writes a stamp for every breakpoint (PerformanceRecorder.cpp:100-107, 137-138).
- The v1 importer writes discrete points only (Take.cpp:432-435).
- Routines drop stamps but compile on the Beat clock, where the fallback is the identity (Program.cpp:449, 566,
  601).
- So no take the app writes can reach it. Only a hand-edited take.json, or a future lane editor, replayed
  WithAudio could.

**Why file it instead of fixing it here:**
1. It is independent of the race. This plan changes which anchors a raced take has, not sampleAt. After the fix,
   a raced take is a single-anchor map, exactly like every take shorter than 32 beats already is.
2. The right fix needs its own ruling on two points:
   - where the fallback rate comes from: the resolved asset's rate (always present on the Sample clock, since
     WithAudio refuses unresolved audio, RecorderHost.cpp:689-697), a whole-map slope, or a rate carried by
     anchors (a format change);
   - a minimum segment span.
3. Today it has no user-visible effect, and it sits in replay code (Program.cpp) that this lane otherwise does not
   touch.

**Ledger entry (Harmony: add to the HANDOFF loose-ends ledger):**
- The fix: `compile()` passes the resolved asset rate into `convertBeatX`. `sampleAt(t, nominalRate)` uses a
  segment's own slope only when the segment spans >= 1 s of t, and `nominalRate` otherwise.
- The RED: a one-anchor map {t 0, sample 150016, bpm 120} plus a stampless gesture at beats {0, 4, 8} must
  compile on DriveClock::Sample to x = {150016, 246016, 342016}. Today all three are 150016.
- Also update tests/test_program_stamps.cpp:31-35, which documents rate 0.

--------------------------------------------------------------------------------------------------------------
## 9. Must not change
1. The audio callback, the ring buffer, AudioTap and AudioStore (the tap starts at arm).
2. FeatureBus: `sizeof(FeatureSnapshot)` stays 384 and `kSnapshotWords` 96; the offsets 316/320/324 stay.
3. Where BPMTracker applies each request (BPMTracker.cpp:74-79, :94, :352-357), coalescing (:586-598), the
   realign rule (:248-253), and `applyTempoCommand` (MainComponent.cpp:5154-5207), including its tempo-point
   capture.
4. RecorderClock's integration and anchor rules (only `started()` is added).
5. The Take format (v3). meta.startBeatInBar keeps its meaning (LOCKED -> beatInBar + beatPhase, else absent).
   TempoMap, Program, Player, RoutineSlice (checkMetered, takeBeatOfBar) and RoutineEngine stay unchanged.
6. The provisional save at arm. The early-tempo and 60 s periodic save rules (now gated behind t = 0, which they
   already implied).
7. The REST, OSC and MIDI surface: no new endpoint or field; `/api/bpm` and `/api/perf/status` are unchanged.
8. Test-mode behaviour (no wait there).
9. No existing probe row or threshold is loosened.

--------------------------------------------------------------------------------------------------------------
## 10. Risks
- **R1 Strongest counterargument:** "A is recorder-local and simpler; C changes a cross-thread contract
  (FeatureSnapshot + BPMTracker) to fix a race only scripts hit." Why it loses:
  - A is not local either. It silently depends on `AnalysisThread::kHopSize`, on snapshot timestamps and on
    where each request is consumed. C makes that dependency explicit, named and tested.
  - A shifts t = 0 for 100% of takes and re-baselines every probe. C moves it only for raced takes.
  - The extra cost is about 20 lines and 4 bytes.
- **R2 Memory order cannot be unit-tested single-threaded.** An edit that raises the sequence before the write,
  or latches after a request read, breaks exactness silently, but only within one hop and only in a rare
  interleaving (no worse than today). Guards: comments at both ends, Pitfall 48, B2's conservative-direction case.
- **R3 Captures between Record and t = 0 are stamped t = 0, beat 0, sample 0.** This class already exists: today's
  arm -> first-tick window, and perfRecord's own arm-time "audio play" point (MainComponent.cpp:5292-5293). The
  window grows (from <= 8 ms to <= ~21 ms, or up to the fallback in a stall) only when a command is in flight.
  WithAudio fires such points on its first advance.
- **R4 In a stall the fallback reproduces today's stale start.** stderr says "take start: ...".
- **R5 A message-thread stall between arm and the applying hop delays t = 0 until the stall ends.** This is the
  same class as today's late first tick, and the tap keeps recording through it.
- **R6 The live RED rates are modelled, not measured.** The credibility rule in section 6 decides.
- **R7 The probe oracle assumes JUCE `callAsync` is FIFO** (INFERRED). The diag's trace shows set_bpm's handler
  running before record's in the lost race.
- **R8 The fix is forward-only.** Takes already raced keep their bpm-0 start and unknown grid; F1 pins that they
  still load and slice as before. Repairing them is out of scope ("old takes replay identically").
- **R9 checkpoint0.bpm is still the arm-time bus bpm** (MainComponent.cpp:2007). It is informational only:
  PerfState.bpm is read only by its serializer (PerfState.cpp:204), and replay restore does not restore tempo
  (recording.md:11). Left as is.
- **R10 Link (off in a default build) posts every 33 ms.** When it is on, about 1 in 3 Records wait 1-2 hops. That
  is harmless.
- **R11 CMake append conflicts with parallel lanes** (the notebook's rule): keep both blocks.

--------------------------------------------------------------------------------------------------------------
## 11. Commit sequence (lane branch off 6db8d67) and "done"
0. Build 6db8d67 UNMODIFIED in `build-lane` (Release, TEST_SERVER ON). Copy `Audio-DNA.app` aside: this is the
   RED app.
1. `test(recording): RED -- a tempo / Resync command sent just before Record misses the take's start anchor and
   bar grid`. Includes the target, E1, E2, F1 and the fixture. ctest shows exactly E1 and E2 failing; quote the
   values.
2. `probe(tempo-start): .harmony/probe-tempo-start.sh`. `bash -n`, plus shellcheck if installed. Run W1-W4 on the
   RED app and record the counts (the credibility rule applies).
3. `feat(analysis): FeatureSnapshot::trackerRequestSeq -- BPMTracker raises a request sequence after every post
   and latches it before any read; published every hop`. Covers 3.1-3.3, B1, B2 and L1. ctest: all GREEN
   except E1 and E2.
4. `fix(recording): a take starts on the snapshot that carries every tracker request sent before Record;
   startBeatInBar from that snapshot; 100 ms fallback; a Stop before t = 0 starts it`. Covers 3.4-3.6, G1-G6,
   L2 and the E1/E2 helper swap. The RED-against-the-stub is recorded inside the commit's work. ctest 100%.
5. `docs: Pitfall 48; recording.md take start; FeatureSnapshot rows; CLAUDE.md index 48`.

After the commits:
- Record the teeth T1-T4 (not committed).
- Run the full ctest.
- Run the live GREEN (section 6) and the re-runs (section 7).
- Write the lane report: RED, GREEN and teeth verbatim, plus the probe counts.

**Done looks like:**
- ctest is 100%: every existing test plus test_tempo_start's 13 cases.
- E1 and E2 were RED on unmodified 6db8d67, with the values quoted.
- On the lane app the probe gives W1 PASS, W2 20/20, W3 20/20, W4 0, and W6 0/20 + 0/20. On the RED app W2 and W3
  each failed >= 5/20.
- Every section-7 probe is green, with no "take start:" lines.
- sampleAt is filed.
- Harmony: APP-INVENTORY gains 1 Catch2 target (13 cases) and 1 probe; the REST surface is unchanged.
- Notebook line: "A two-request message-ordering race becomes nearly certain live when both requests queue behind
  the TEST-ONLY message-thread stall hook. Count a row as a gate only if it fails >= 5/20 on the unmodified app."

--------------------------------------------------------------------------------------------------------------
## 12. Doc texts

### Pitfall 48 (`docs/claude/pitfalls.md`, after :103)
48. **A tempo command reaches the analysis snapshot only at the tracker's NEXT hop -- "everything sent before X"
is `FeatureSnapshot::trackerRequestSeq >= BPMTracker::postedRequestSeq()` read at X, never the snapshot read at X
and never a fixed delay**: every tempo / phase command -- REST and OSC set_bpm and Resync, Tap and Resync pads and
keys, the TopBar's Tap / typed BPM / manual-auto switch / Resync, Link, a replayed tempo point -- goes through
`MainComponent::applyTempoCommand` into a `BPMTracker` REQUEST that the analysis thread applies up to one hop
later (~10.7 ms; a device-buffer period with large buffers): the tempo word at the start of `runPipeline`, the
manual flag right after, a Resync at the end of `feedDownbeatFeatures`. A FeatureBus read right after the command
-- and any 120 Hz tick in the next ~10 ms -- still sees the old tempo and phase. Record lost that race (s-rta-0927
tempo0-diag: 4/34 witness takes started at bpm 0, so no routine could be cut from beat 0 or bar 1; 9/34 had an
unknown bar grid). Every request raises `requestSeq_` (release) AFTER its own write; `runPipeline` latches it
(acquire) as its FIRST statement, BEFORE it reads any request; `AnalysisThread` publishes the latch as
`trackerRequestSeq` -- a snapshot never claims a request it has not read (it may reflect a later one). Never move
the raise before the write or the latch after a request read: nothing single-threaded can catch it.
`RecorderHost` starts a take (t = 0, the `"start"` anchor, and `meta.startBeatInBar` from the same snapshot) on
the first tick whose snapshot reaches `ArmOptions::startAfterTrackerRequest` (a signed 32-bit difference -- the
sequence wraps), or `kStartWaitFallbackSeconds` (0.1 s) after the first tick (stderr `take start:`); nothing in
flight = the first tick, as before; the tap and the provisional save stay at arm. Test mode never starts the
analysis thread, so `perfRecord` passes no sequence there. The same counter pattern as `onsetCount` (30),
`totalBarCount` (32), `totalBeatCount` (42). Guards: `tests/test_tempo_start.cpp`; live:
`.harmony/probe-tempo-start.sh` (the TEST-ONLY stall hook makes the old race nearly certain).

Index line (`CLAUDE.md`, after :229):
`48. A tempo command reaches the analysis snapshot only at the next hop -- before acting on "everything sent before X" (a take's t = 0 and bar grid), compare FeatureSnapshot::trackerRequestSeq with BPMTracker::postedRequestSeq() read at X.`

### `docs/claude/recording.md` (new paragraph after :11)
**Take start (s-rta-0928).** A take's t = 0 -- its `"start"` tempo anchor, beat 0 and `meta.startBeatInBar` (where
beat 0 sits in its bar, read from the same analysis snapshot; written only while the tracker is LOCKED, else
unknown) -- is the first 120 Hz tick after Record whose snapshot already carries every tempo command sent before
Record: Tap, Resync, set_bpm over REST/OSC, the typed manual BPM and the manual/auto switch, Link, a replayed tempo
point (`FeatureSnapshot::trackerRequestSeq` against `BPMTracker::postedRequestSeq()` read at Record; Pitfall 48).
With nothing in flight that is the first tick, as before; right after a command it is one or two analysis hops
later (~10-20 ms), so a set_bpm or Resync sent just before Record is in the take's start and bar grid. If no
snapshot carries it within 100 ms of the first tick (no audio device, a stalled device), the take starts from the
latest snapshot anyway (one `take start:` line in the app's stderr); a take stopped before its start is started
from the last tick it saw. The audio still starts at Record; moves captured between Record and t = 0 are stamped
t = 0. A take started while the tracker is truly unlocked is an unmetered start (a bpm-0 `"start"` anchor, then
`"lock"` when a tempo arrives), and a routine cut starting at its beat 0 is refused -- by design. Takes recorded
before this fix keep the start they have. Test mode (no analysis thread) does not wait. Live:
`.harmony/probe-tempo-start.sh`.

### `docs/claude/architecture.md` (row after :29)
`| trackerRequestSeq | uint32_t | count | s-rta-0928: the BPMTracker request sequence this snapshot reflects -- every tempo / Tap / Resync / manual-mode request that raised the sequence to <= this is in bpm / beatPhase / trackerState / beatInBar here; wraps at 2^32 (signed-difference compare); 0 in test mode. Record's t = 0 waits for it (Pitfall 48) |`

### `docs/claude/analysis.md` (row after :39)
`| Tracker Request Seq | BPM tracker (s-rta-0928): latched at the start of each hop's runPipeline, before any request is read; every request raises the posted sequence after its write. >= BPMTracker::postedRequestSeq() read at X = the snapshot reflects every command sent before X (Pitfall 48) | uint32 | trackerRequestSeq |`

--------------------------------------------------------------------------------------------------------------
## COMPACT
- **Ruling:** an EXACT handshake, not A (2 hops + 50 ms) and not B (REST-only wait).
  - Every tracker request raises `requestSeq_` (release) after its write.
  - `runPipeline` latches it (acquire) before any read.
  - It is published as `FeatureSnapshot::trackerRequestSeq` (offset 328; sizeof stays 384).
  - `perfRecord` passes `postedRequestSeq()`.
  - `RecorderHost` holds only the clock's first tick until the snapshot reaches it, with a 100 ms fallback, and a
    Stop starts a waiting take.
  - startBeatInBar comes from the t = 0 snapshot. The tap and the provisional save stay at arm.
  - Test mode does not wait.
- **Why:**
  - exact;
  - covers every path (REST, OSC, MIDI, keyboard, TopBar, Link, replay all go through the request API;
    MainComponent.cpp:5154-5207, BPMTracker.cpp:555-628);
  - t = 0 moves only for raced takes, so no existing probe's t = 0 moves (section 7, VERIFIED);
  - the codebase's own counter pattern;
  - zero allocation and no locks on the RT threads.
- **A loses:** it shifts every take's t = 0 by 11-21 ms, is a heuristic tied to where requests are consumed, and
  mis-waits in test mode. **B loses:** REST set_bpm only, a wrong predicate for folded or same tempos and for
  Resync, and it blocks httplib workers.
- **Tests:** test_tempo_start, 13 cases.
  - E1 and E2 are a deterministic RED of both symptoms on UNMODIFIED 6db8d67: the real BPMTracker, RecorderHost,
    RecorderClock/TempoMap, Take save/load and RoutineSlice.
  - F1 is an old raced take (a guard).
  - B1, B2 and L1 cover the analysis side; G1-G6 and L2 the recorder side; T1-T4 are the teeth.
- **Live:** probe-tempo-start.sh.
  - The stall-assisted pair makes the old race nearly certain: ~85-95% per cycle, by the model.
  - The gate: W1, W2 20/20, W3 20/20, W4 0 fallbacks, W6 witness x20 at 0/20 + 0/20.
  - A row counts only if it failed >= 5/20 on the unmodified app.
- **sampleAt:** FILED. It is unreachable by any take the app writes, independent of the race, and needs its own
  ruling on the rate source. A spec and RED are given in section 8.
- **Files:** FeatureSnapshot.h, BPMTracker.h/.cpp, AnalysisThread.cpp (2 lines), RecorderClock.h (1 line),
  RecorderHost.h/.cpp, MainComponent.cpp (perfRecord only), tests/test_tempo_start.cpp + CMake + 1 fixture,
  .harmony/probe-tempo-start.sh, 5 doc files. Take format unchanged. Five commits.

STATUS: COMPLETE -- builder-executable plan written; no product code changed; open items: (1) file TempoMap::sampleAt in the loose-ends ledger (section 8); (2) the live RED rates are modelled and must be measured on the unmodified app before any GREEN claim (section 6 credibility rule).

--------------------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (s-rta-0928, 09:35) — OVERRIDES THE BODY WHERE THEY DIFFER
Authorship note: this plan was written by an OPUS architect standing in for Fable (the Fable quota was exhausted this
session; recorded tier deviation). The header's "Architect (Fable)" is wrong; read it as opus.
Blind council: attack-tempo-gates.md (SOUND_WITH_FIXES), attack-tempo-realtime.md (SOUND_WITH_FIXES). Both verified the
handshake's memory order, the FeatureSnapshot layout (offset 328, sizeof 384) and that no existing probe records with a
tempo command in flight. ADOPTED: the exact handshake (§1.1) as written, with these rulings:
- A1 (realtime MUST) FALLBACK: kStartWaitFallbackSeconds = 0.25 s, not 0.1 s. The plan's own §2.2 arithmetic puts the
  legitimate wait at ~95 ms for 4096-frame buffers @ 44.1 kHz; 0.1 s leaves ~5 ms for message-thread jitter. 0.25 s gives
  > 2x margin; it only matters when a command is in flight AND hops stop (no device / a stall). If RecorderHost already
  knows the device buffer period without new plumbing, use max(0.25, 2 * period + 0.05); otherwise the constant. G-case
  for the fallback uses the constant by name.
- A2 (gates MUST 1) The live RED is STALL-ASSISTED, mandatory. If /api/debug/stall_message_thread answers 404 on the RED
  or the lane app, the probe prints "BLOCKED: stall hook unavailable" and exits non-zero for W1-W3 (no silent fall back to
  tight pair). Tight pair (TEMPOSTART_STALL_MS=0) is an extra natural-timing run on the lane app only, reported, never the
  RED. Main's and the lane's builds have TEST_SERVER ON. Harmony's own baseline on main (08:29, witness x20, load 5-7):
  start_bpm0 4/20, unknown grid 5/20 — W6 on the RED app is therefore not needed.
- A3 (gates MUST 2) L1/L2 are STRUCTURAL LINTS (source-text scans), labelled "[lint]" in their names, never counted as
  behaviour proof. The MainComponent arm -> postedRequestSeq wire and the startBeatInBar source are proven ONLY by the
  live W1-W3 on the lane app; the report says so. Harmony re-runs W1-W4 + W6 x20 herself at the gate.
- A4 (gates SHOULD) Replace the heap unique_ptr<FeatureSnapshot> startWaitSnap_ with a small POD holding exactly the
  fields startClock reads (timestamp/sample, bpm, beatPhase, beatInBar, trackerState, totalBeatCount — whatever §3.5 uses).
- A5 (realtime SHOULD) No onset lost to the wait: an onset (onsetCount delta) observed between arm and t = 0 is not
  dropped — keep the onset baseline at the FIRST tick after arm (as today) and emit waiting-window onsets stamped t = 0
  (the same rule §2.2 already applies to captures), or show with a G-case that none can occur; whichever is smaller.
  Synthesized Decaying ends: same principle (nothing a performer did between arm and t = 0 is lost).
- A6 Re-runs (§7) use the battery env: ROUTINES_BUILD_DIR=<lane build> ROUTINES_RECORD_PAUSE=1.8 (and 0) probe-routines;
  STEP3_BUILD_DIR; RESYNC_BUILD_DIR; DOWNBEAT_BUILD_DIR; MANUALBPM_BUILD_DIR; probe-beatclock.sh <outdir> (it takes the
  build under test from its own env — read its header); probe-finalize-loop / probe-onset-render per their headers.
- A7 TempoMap::sampleAt stays FILED (§8) — it is Program's stampless Sample-clock fallback only; add it to the lane
  report's found_not_fixed with the §8 text.
- A8 Pitfall number: check docs/claude/pitfalls.md for the next free number at build time (47 is the last on main);
  CLAUDE.md index line must fit the 25,000-byte cap (pay by moving text to docs/claude if needed).
