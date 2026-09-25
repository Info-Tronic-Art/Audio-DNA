# Finalize truncation ("header N frames < framesWritten N+512") -- mechanism proof + fix plan

Architect (Fable), s-rta-0924b, 2026-09-25. Read-only analysis of main @ HEAD (3279148).
Repo: /Users/boriskarpman/projects/RealTimeAudio. Confidence labels: VERIFIED = read in
source at the cited line; INFERRED = follows from cited source + the live numbers; ASSUMED =
not checked.

QUESTION: Why do ~9% of REST record/stop cycles on the built-in mic end with
`[Recorder] asset <id>: truncated (header N frames < framesWritten N+512)` -- always exactly one
512-frame device block short -- and why is `GET /api/perf/status` `lastError` empty afterwards?
Then: the minimal RT-safe fix, a deterministic fail-first unit test, status visibility, a
probe-step3 row, and a live N=40 check.

VERDICT (sentence one): `AudioTap::stopInternal()` clears the audio thread's only gate,
`running_`, LAST (AudioTap.cpp:499), after it has already nulled the writer (:445), waited out
the in-flight push (:466), drained the spill buffer (:473-493) and destroyed the writer (:495);
a device callback that lands inside that window still passes `push()`'s gate (:190), counts its
512 frames into `framesWritten_` (:226), fails the real write (:398-399, writer null), spills
into `pendingStorage_` (:310) -- and nobody ever drains that spill (the drain already ran; the
writer is gone; `start()` zeroes it at :139). Finalize then compares the WAV header (N) with
the counter (N+512) and flags truncation (AudioStore.cpp:211-218). `lastError` is empty because
`RecorderHost::disarm()` keeps `fin.error` in a local (`finalizeError`, RecorderHost.cpp:306-311)
and never assigns `lastError_`; `publishStatus()` (:719) therefore publishes the value `arm()`
cleared at :171. Fix: flip `armed_`/`running_` FIRST in `stopInternal()` (seq_cst) and make
`push()`'s `running_` load seq_cst, so any post-gate callback returns 0 before counting; add
`Status::lastFinalizeError` + cumulative `Status::finalizeErrors` and mirror into `lastError_`.

---------------------------------------------------------------------------------------------

## 1. MECHANISM -- proof from source

### 1.1 The tap counts BEFORE the block lands (count-then-write) -- VERIFIED
- `AudioTap::push()` (src/recording/AudioTap.cpp:150-221): busy flag RAII at :159; arm capture
  :180-188; **the one gate** `if (!running_.load(relaxed)) return 0;` at :190-191; then
  `writeFrames(ch, chans, numSamples)` at :219.
- `writeFrames()` :223-269: `framesWritten_.fetch_add(numSamples)` at **:226**, BEFORE the data is
  handed to `writeBlockRetrying()` (:231 / :266). Same shape in `writeSilenceFrames()` (:274).
- `writeBlockRetrying()` :285-311: `tryRealWrite()` (:302) -> `activeWriter_.load(seq_cst)` and
  `w != nullptr && w->write(...)` at **:398-399**; on `false` the whole block goes to
  `spillIntoPending()` (:310 -> :343-353, `pendingFrames_ += accepted`).
- Count-first is honest ONLY because a spilled block is later drained: by the next `push()`
  (:289-290 -> `flushPendingNonBlocking()` :313-319) or by `stopInternal()` (:473-479).

### 1.2 `stopInternal()` clears the gate LAST -- VERIFIED (AudioTap.cpp:440-500)
```
:445  activeWriter_.store(nullptr, seq_cst)          -- push() can no longer reach the writer
:466  while (audioThreadBusy_.load(seq_cst)) yield  -- waits out ONE in-flight push()
:469  if (threadedWriter_) {
:473    while (pendingFrames_ > 0) threadedWriter_->write(pending...)   -- drains a spill made BEFORE :466 passed
:486    while (silenceDebtFrames_ > 0) ...
:495    threadedWriter_.reset()      -- BLOCKING: JUCE ~Buffer removeTimeSliceClient (may wait out an
                                        in-progress background write: juce_TimeSliceThread.cpp:59-78),
                                        synchronous FIFO drain (juce_AudioFormatWriter.cpp:235-242,
                                        273-316), ~WavAudioFormatWriter -> writeHeader() seek+write
                                        (juce_WavAudioFormat.cpp:1626-1629, 1687-1699), ~FileOutputStream
                                        flushBuffer()+close (juce_FileOutputStream.cpp:47-51)
      }
:498  armed_.store(false)
:499  running_.store(false)          -- <<< the ONLY thing push() checks, and it flips LAST
```
`stop()` is reached from `RecorderHost::disarm()` (RecorderHost.cpp:285) on the message thread
(REST: ApiServer.cpp:1240-1253 `callAsync(onPerfStop)` -> MainComponent.cpp:2040 -> `perfStop()`
:5127-5148). The CoreAudio callback keeps firing every 512 frames (10.67 ms at 48 kHz) throughout;
`CombinedCallback::audioDeviceIOCallbackWithContext` calls `audioTap_.push()` unconditionally
every block (src/audio/CombinedCallback.h:129).

### 1.3 The lossy window, step by step -- VERIFIED ordering, INFERRED timing
Classify a device callback's `push()` by where it lands relative to `stopInternal()`:

(a) push() already in flight (busy==true) when :466 is evaluated -- or its busy store precedes
    the :466 load in the seq_cst total order: the wait catches it. Its block lands in the
    writer's FIFO (loaded `activeWriter_` before :445) or in `pendingStorage_` (after :445), and
    :473 writes the spill. The busy flag's seq_cst store/load pair (AudioTap.cpp:16-23, :466) gives
    the happens-before that makes the non-atomic `pendingFrames_` read at :473 sound. NO LOSS.

(b) push() starts AFTER the :466 load observed busy==false and BEFORE :499:
    :190 `running_` is still TRUE -> proceeds
    :226 `framesWritten_ += 512`                          <- COUNTED
    :398 `activeWriter_` is null -> `tryRealWrite` false
    :310 `spillIntoPending(data, 512)` -> `pendingFrames_ = 512`
    ... meanwhile the message thread has already passed :473 (which saw 0), destroys the writer
    at :495, then sets `running_=false` at :499. The 512 spilled frames are never written; the
    next `start()` zeroes `pendingFrames_` at :139 (and `framesWritten_` at :137).
    RESULT: WAV = N frames, `framesWritten()` = N+512. Exactly the live message.
    (Also a latent data race: :473-493 read/write `pendingFrames_`/`silenceDebtFrames_` on the
    message thread while a window-(b) push writes them on the audio thread -- non-atomic ints,
    AudioTap.h:234/:245. Not the observed symptom, but the same window.)

Why the counted block is really lost and not just "late": the writer's own `lengthInSamples`
counts only what `write()` received (juce_WavAudioFormat.cpp:1663), the final header is written
from that in `~WavAudioFormatWriter` (:1626-1629), and the spill's only two drains are :289-290
(no further push() writes once `running_` is false) and :473 (already executed).

### 1.4 What finalize reads, and why the take still plays -- VERIFIED
- `RecorderHost::disarm()` RecorderHost.cpp:284-305: `endedEarly = !tap.isRunning()` (before
  stop), `tap.stop()` (:285), then `facts.framesWritten = tap.framesWritten()` (**:295**, the
  N+512 counter) -> `store_.finalize(assetId_, facts)` (:305).
- `AudioStore::finalize()` AudioStore.cpp:194-202 opens the WAV and takes `asset.frames =
  reader->lengthInSamples` (**:202**, = N); :211 `if (asset.frames < facts.framesWritten)` ->
  `unreliableFrom = min(existing, N)` (:213-215) and the error string (:216-217). The sidecar is
  still written (fingerprint ok, :220-223, :245-248) with `frames = N`.
- `take.audio = referencing(fin.asset, firstSample)` (RecorderHost.cpp:312) -> segment.frames = N,
  `unreliableFrom = firstSample + N`. Sidecar N == take N == WAV N, so `resolve()` (AudioStore.cpp
  :401, :421) is consistent -> Resolved -> "Audio: ready", playback fine. The only marks: the
  stderr line (MainComponent.cpp:2013 via `dispatch.notify`), the "Saved: ..., but its audio had
  a problem: ..." notice (MainComponent.cpp:5140-5147), and a spurious `unreliableFrom` on a take
  whose audio up to N is perfect. The lost content is the final 10.67 ms.

### 1.5 Why exactly one block, and why ~9% -- INFERRED
Window (b) spans :467..:499, dominated by `threadedWriter_.reset()` (:495): a FIFO drain of at
most a block or two (the TimeSliceThread is `notify()`ed on every write, juce_AudioFormatWriter.cpp
:264), a header seek+write and a `close()`. That is a few ms at most -- shorter than one 10.67 ms
callback period -- so 0 or 1 callbacks land in it: 0 or 512 frames, never 1024. A 2/23 hit rate
implies a window of roughly 0.9 ms (10.67 ms x 0.087). If a slow disk ever stretched the window
past one period you would see 1024; the fix removes the dependency on window length entirely.

### 1.6 Alternative explanations considered -- and why they lose
- The "give up rather than spin" breaks at :478/:492 (the cause AudioStore.cpp:207-210 attributes
  truncation to): require the 8-second ThreadedWriter FIFO (:126-127) to be FULL at stop -- a
  disk stalled for seconds. Contradicted by: the take plays fine, loss is exactly one device
  block, hit rate ~9% of otherwise healthy 2-60 s takes.
- Header re-patch every 10 s (`setFlushInterval`, :131): `flush()` (juce_WavAudioFormat.cpp
  :1667-1673) rewrites the header from the writer's own count and the destructor rewrites it
  again; the header always reflects what the writer received. The discrepancy is on the
  counter side, not the header side.
- A block "in flight" during stop (window a): fully handled by the busy-wait + :473 drain.
  The s168 review fix closed the UAF; it did not move the gate, so window (b) survived it.

## 2. WHY `lastError` IS EMPTY AFTER THE STOP -- VERIFIED
- RecorderHost.cpp:305-311: `fin.error` -> local `finalizeError` + `dispatch.notify(fin.error)`.
  `lastError_` is NOT assigned. `lastError_` is written only at :171 (cleared at arm), :379
  (rate change) and :419 (tap self-stop).
- :332 `res.error = finalizeError` -> `MainComponent::perfStop()` (:5127-5148) folds it into a
  notify and returns `{}`; the REST handler discards even that (ApiServer.cpp:1240-1253 answers
  `jsonOk()` before `onPerfStop` runs; MainComponent.cpp:2040 `[this] { perfStop(); }` drops the
  string). So REST has NO channel for a finalize problem.
- :355 `publishStatus()` -> :719 `s.lastError = lastError_` == "" -> `perfStatusVar()`
  (MainComponent.cpp:5241-5297, :5253) serialises "". Hence probe-step3.sh:301-302's
  "lastError empty" row passes vacuously and `.harmony/probe-step3.sh` cannot detect this.
- The Record panel is unaffected either way: RecordPanelModel.h:222 shows `lastError` only while
  `recording` is true.

---------------------------------------------------------------------------------------------

## 3. TRADEOFFS CONSIDERED (fix)
- **A. Move the gate: flip `armed_`/`running_` first, seq_cst, then null the writer, then
  busy-wait (RECOMMENDED).** After the wait no push() can touch the audio-thread-only fields, so
  the drains and the writer teardown run with exclusive access by construction; the latent
  `pendingFrames_` race disappears too. Cost on the audio thread: `push()`'s `running_` load
  goes relaxed -> seq_cst (one `ldar` on Apple Silicon per block, next to the two seq_cst busy
  stores and the seq_cst `activeWriter_` load push() already does). No lock/alloc/syscall.
- B. Keep the order, add a second busy-wait + re-drain after `running_=false`, move `reset()`
  after it. Works, but two waits, and the first drain still races a concurrent push on the
  non-atomic fields. Rejected: more lines, weaker invariant.
- C. Count after a successful write (count-after). Rejected: spilled blocks are legitimately
  owed and later written; `unreliableFrom` math depends on count-first (AudioTap.cpp:374-378);
  and it would not drain the orphaned spill anyway.
- D. Make `finalize()` tolerate a one-block deficit. Rejected: hides real loss; the check is the
  safety net that caught this. Keep AudioStore.cpp:211-218 unchanged.
- Visibility: dedicated `lastFinalizeError` (per take) + cumulative `finalizeErrors` counter,
  AND mirror into `lastError_`. Mirroring alone would be enough for probe row :302 but the
  counter lets a live loop check "any stop in the run" with no read window (D12: additive only).

## 4. DECISION / SPEC

### F1. AudioTap ordering fix (src/recording/AudioTap.cpp, src/recording/AudioTap.h) -- the fix
1. `stopInternal()` (:440): insert BEFORE the existing `activeWriter_.store(nullptr, seq_cst)`:
```cpp
    // s-rta-0924b finalize-truncation fix: close the gate FIRST. Any push() that starts after
    // these two stores returns 0 at its running_ check without touching framesWritten_,
    // pendingFrames_ or silenceDebtFrames_. A push() already past that check is in flight
    // (audioThreadBusy_ == true) and is waited out below; its block lands either in the
    // writer's FIFO (it loaded activeWriter_ before the null store) or in pendingStorage_
    // (after it), and the pending drain below writes it. Everything after the wait therefore
    // runs with EXCLUSIVE access to the audio-thread-only fields. seq_cst on both stores: this
    // is a store-buffering (Dekker) pattern with push()'s busy store + running_ load, and the
    // "both threads read stale" outcome is excluded only when all four operations are seq_cst.
    armed_.store(false, std::memory_order_seq_cst);
    running_.store(false, std::memory_order_seq_cst);
```
   KEEP the trailing `armed_.store(false)` / `running_.store(false)` at :498-499. They are still
   load-bearing: a push() that read `armed_==true` at :180 just before the early store and then
   stored `running_=true` at :183 AFTER the early `running_=false` leaves the tap "running" with
   no writer; that push is in flight (busy) and is waited out, so the trailing stores, which run
   after the wait, are the ones that end that state. Add a one-line comment saying so.
2. `push()` :190: `if (!running_.load(std::memory_order_relaxed))` ->
   `if (!running_.load(std::memory_order_seq_cst))` with a comment pointing at the stopInternal
   comment above (why relaxed is not enough: SB pattern).
3. AudioTap.h :97-104 `stop()` doc: replace "Synchronously waits out any push()..." with the new
   order (disarm+stop first -> null writer -> wait -> drain -> teardown) so the header stops
   describing the pre-fix ordering.
4. Nothing else in push()'s call graph changes. RT audit for the Builder/Reviewer: the diff to
   the audio-thread path is exactly one memory_order token on one load.

### F2. Test hooks (AudioTap.h/.cpp, under `#if defined(AUDIODNA_AUDIOTAP_TEST_HOOKS)` only)
AudioTap.h, next to the existing debug hooks (:144-178):
```cpp
    // TEST ONLY -- pauses the NEXT stop()/stopInternal() after its pending/silence-debt drains
    // and immediately BEFORE threadedWriter_.reset(): the part of the stop window a device
    // callback can land in after the drains already decided nothing was pending. The test
    // thread plays the audio thread and calls push() while stop() is paused on another thread.
    void debugArmStopPauseBeforeWriterTeardown() { debugPauseStopForTest_.store(true, std::memory_order_release); }
    void debugReleasePausedStop()                { debugReleaseStopForTest_.store(true, std::memory_order_release); }
    bool debugWaitUntilStopPaused(int maxSpins = 2'000'000)
    {
        for (int i = 0; i < maxSpins; ++i)
        {
            if (debugStopIsPausedForTest_.load(std::memory_order_acquire)) return true;
            std::this_thread::yield();
        }
        return false;
    }
    // TEST ONLY -- adds `n` to framesWritten_ with no write: the exact EFFECT of a block counted
    // at writeFrames() that never reached the WAV (s-rta-0924b). Lets RecorderHost tests force
    // AudioStore::finalize's truncation verdict deterministically, independent of F1.
    void debugAddPhantomFramesForTest(uint32_t n) { framesWritten_.fetch_add(n, std::memory_order_relaxed); }
```
plus three `std::atomic<bool>` members (`debugPauseStopForTest_`, `debugStopIsPausedForTest_`,
`debugReleaseStopForTest_`) in the existing `#if` block at :262-270.
AudioTap.cpp `stopInternal()`, inside `if (threadedWriter_)`, AFTER both drain loops and
immediately BEFORE `threadedWriter_.reset()`:
```cpp
#if defined(AUDIODNA_AUDIOTAP_TEST_HOOKS)
        if (debugPauseStopForTest_.exchange(false, std::memory_order_acq_rel))
        {
            debugStopIsPausedForTest_.store(true, std::memory_order_release);
            while (!debugReleaseStopForTest_.load(std::memory_order_acquire))
                std::this_thread::yield();
            debugStopIsPausedForTest_.store(false, std::memory_order_relaxed);
            debugReleaseStopForTest_.store(false, std::memory_order_relaxed);
        }
#endif
```
The hook must be placed at this SAME point in the pre-fix and post-fix code (it is; F1 only adds
lines at the top of the function) so the RED run below is meaningful.

### F3. Test A -- deterministic fail-first for the ordering (tests/test_audio_tap_sync.cpp)
Target already defines `AUDIODNA_AUDIOTAP_TEST_HOOKS=1` (tests/CMakeLists.txt:364-370). Append:
```cpp
// s-rta-0924b: one device block lost at stop ("truncated (header N frames < framesWritten
// N+512)"). Deterministic: stop() is held on a background thread at the test-only pause point
// right before the writer teardown (after the drains), and THIS thread plays the audio thread
// by calling push() once inside that window. Oracle == AudioStore::finalize's truncation rule
// (AudioStore.cpp :211): the WAV header must equal framesWritten().
TEST_CASE("AudioTap::stop() -- a push() landing after the busy-wait, before the writer teardown, is not counted (s-rta-0924b truncation)", "[audiotap][concurrency][truncation]")
{
    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    AudioTap tap;
    tap.prepare(rate, 2, blockSize);
    TempWavFile wav("stopwindow");
    REQUIRE(tap.start(wav.file));

    std::vector<float> inL(static_cast<size_t>(blockSize), 0.25f), inR(static_cast<size_t>(blockSize), 0.25f);
    const float* ch[2] = { inL.data(), inR.data() };
    uint64_t delivered = 0, hostTimeNs = 1'000'000'000ULL;
    auto pushOne = [&]()
    {
        hostTimeNs += static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
        juce::AudioIODeviceCallbackContext ctx;
        ctx.hostTimeNs = &hostTimeNs;
        const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
        delivered += static_cast<uint64_t>(blockSize) + gap;
        return gap;
    };
    for (int b = 0; b < 10; ++b) REQUIRE(pushOne() == 0);
    REQUIRE(tap.isRunning());
    REQUIRE(tap.framesWritten() == 10 * blockSize);

    tap.debugArmStopPauseBeforeWriterTeardown();
    std::thread stopThread([&]() { tap.stop(); });
    REQUIRE(tap.debugWaitUntilStopPaused());          // stop() is inside the window now

    const uint64_t countedBefore = tap.framesWritten();
    CHECK(pushOne() == 0);                             // the device callback that lands in the window
    const uint64_t countedInWindow = tap.framesWritten() - countedBefore;

    tap.debugReleasePausedStop();
    stopThread.join();
    REQUIRE_FALSE(tap.isRunning());

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(wav.file), true));
    REQUIRE(reader != nullptr);
    // THE oracle (fails pre-fix as 5120 == 5632): what finalize compares.
    CHECK(static_cast<uint64_t>(reader->lengthInSamples) == tap.framesWritten());
    // And the reason: the in-window block was never counted (pre-fix: 512).
    CHECK(countedInWindow == 0);
    CHECK(static_cast<uint64_t>(reader->lengthInSamples) == 10 * blockSize);
}
```
Expected RED on pre-fix code: `lengthInSamples (5120) == framesWritten (5632)` fails and
`countedInWindow (512) == 0` fails -- the exact live shape. GREEN after F1. The existing UAF test
(:624-686) must stay green (F1 does not touch the busy-wait).

### F4. Status visibility (RecorderHost.h/.cpp, MainComponent.cpp) + Test B
1. RecorderHost.h `Status` (:188-229), additive:
```cpp
        // s-rta-0924b: AudioStore::finalize's verdict for the LAST stop ("" = clean), per take
        // (cleared at arm). `finalizeErrors` counts every stop whose finalize reported a problem
        // since this host was created and is NEVER reset -- a probe compares it before/after any
        // number of record/stop cycles with no read window to miss.
        std::string lastFinalizeError;
        int finalizeErrors = 0;
```
   members (near :275 `lastError_`): `std::string lastFinalizeError_; int finalizeErrors_ = 0;`
2. RecorderHost.cpp `arm()` :171: add `lastFinalizeError_.clear();` (NOT the counter).
3. `disarm()` :306-311 becomes:
```cpp
        if (!fin.error.empty())
        {
            finalizeError = fin.error;
            lastError_ = fin.error;            // status().lastError carries it (REST/probe row :302)
            lastFinalizeError_ = fin.error;
            ++finalizeErrors_;
            if (dispatch.notify) dispatch.notify(fin.error);
        }
```
4. `publishStatus()` after :719: `s.lastFinalizeError = lastFinalizeError_; s.finalizeErrors = finalizeErrors_;`
5. MainComponent.cpp `perfStatusVar()` after :5253:
   `obj->setProperty("lastFinalizeError", juce::String(s.lastFinalizeError));`
   `obj->setProperty("finalizeErrors", s.finalizeErrors);`
6. Optional O1 (same shape, 2 lines): at :327-329 also `lastError_ = "could not save take.json at stop";`
   -- pre-existing gap where a save failure is likewise invisible to REST. Not required for this
   defect; include only if the Reviewer agrees it is in scope.
7. tests/CMakeLists.txt:1076-1082 (`test_recorder_host` compile definitions): add
   `AUDIODNA_AUDIOTAP_TEST_HOOKS=1`. Safe: that target compiles its own AudioTap.cpp object
   (:1065), so the define reaches only this test binary, never the app.
8. Test B (tests/test_recorder_host.cpp, uses the file's `TempDir`/`FakeDispatch`/`pushCleanBlocks`
   helpers :77-144, arm options as in :372-378):
```cpp
TEST_CASE("RecorderHost disarm -- a finalize problem (truncated asset) reaches status(): lastError, lastFinalizeError, finalizeErrors; the take still references the audio", "[host][stop][finalize]")
{
    TempDir storeRoot("fin_store"); TempDir takeFolder("fin_take"); TempDir takeFolder2("fin_take2");
    AudioStore store(storeRoot.dir); RecorderHost host(store);
    Composition comp = makeComposition(); FakeDispatch fake; fake.wire(host, &comp);
    AudioTap tap; tap.prepare(48000.0, 2, 512);
    RecorderHost::ArmOptions opts; opts.takeFolder = takeFolder.dir; opts.audio = true;
    opts.audioMode = "input"; opts.deviceRate = 48000.0; opts.deviceChannels = 2; opts.appVersion = "test";
    REQUIRE(host.arm(comp, tap, opts).ok);
    uint64_t delivered = 0, hostTimeNs = 1'000'000'000ULL;
    host.tick(makeSnap(), 0.0, delivered, tap, std::nullopt, 48000.0);
    pushCleanBlocks(tap, 48000.0, 512, 10, delivered, hostTimeNs);
    host.tick(makeSnap(), 0.2, delivered, tap, std::nullopt, 48000.0);
    CHECK(host.status().lastError.empty()); CHECK(host.status().finalizeErrors == 0);

    tap.debugAddPhantomFramesForTest(512);   // a block counted but never written -- the s-rta-0924b effect
    const auto stopRes = host.disarm(comp, tap);
    CHECK(stopRes.ok);                                            // D-A11: finalize problems never fail the stop
    REQUIRE(stopRes.error.find("truncated (header 5120 frames < framesWritten 5632)") != std::string::npos);
    const auto after = host.status();
    CHECK_FALSE(after.recording);
    CHECK(after.lastError == stopRes.error);                      // RED pre-F4: empty
    CHECK(after.lastFinalizeError == stopRes.error);
    CHECK(after.finalizeErrors == 1);
    LoadStats stats; auto loaded = Take::load(takeFolder.dir, stats); REQUIRE(loaded.has_value());
    REQUIRE(loaded->audio.segments.size() == 1);
    CHECK(loaded->audio.segments[0].frames == 5120);              // the header, not the counter
    REQUIRE(loaded->audio.unreliableFrom.has_value());
    CHECK(*loaded->audio.unreliableFrom == tap.firstSample() + 5120);
    CHECK(store.resolve(loaded->audio).status == AudioStore::Status::Resolved);   // still plays

    opts.takeFolder = takeFolder2.dir;                            // per-take vs cumulative
    REQUIRE(host.arm(comp, tap, opts).ok);
    CHECK(host.status().lastFinalizeError.empty()); CHECK(host.status().lastError.empty());
    CHECK(host.status().finalizeErrors == 1);
    pushCleanBlocks(tap, 48000.0, 512, 2, delivered, hostTimeNs);
    CHECK(host.disarm(comp, tap).error.empty());
    CHECK(host.status().finalizeErrors == 1);
}
```
   Hook-free fallback if the Reviewer objects to the phantom hook: `store.wavFile(assetId).deleteFile()`
   before `disarm()` forces the "wav unreadable" branch (AudioStore.cpp:231-242) -- also reaches
   `lastError`, but changes the disk assertions (no sidecar, Incomplete). Prefer the hook: it
   reproduces the observed message verbatim, which is what the probe rows key on.

### F5. probe-step3.sh rows (.harmony/probe-step3.sh)
Per-stop, after :302 (section 7) and after :798 (section 11L):
```bash
FIN_ERR="$(perf_field "d.get('lastFinalizeError','NA')")"
[ "$FIN_ERR" = "" ] && ok "perf/status: lastFinalizeError empty after stop (audio.wav header frames == tap framesWritten; no block lost at stop)" \
  || no "perf/status: lastFinalizeError='$FIN_ERR' (s-rta-0924b finalize truncation, or field missing)"
```
("NA(...)" when the key is absent -> red, so a build without F4 cannot pass this row vacuously.)
Disk oracle, section 8 after :330 (independent of the new field):
```bash
UNREL="$(take_field "$TAKE_FOLDER" "d['audio'].get('unreliableFrom')")"
[ "$UNREL" = "None" ] && ok "take.json audio.unreliableFrom is null (no truncation, no overrun)" || no "take.json audio.unreliableFrom=$UNREL (finalize marked the audio unreliable)"
WAVN="$(python3 -c "import wave; w=wave.open('$ASSET_DIR/audio.wav','rb'); print(w.getnframes())" 2>/dev/null || echo NA)"
[ "$WAVN" = "$SEG_FRAMES" ] && ok "audio.wav frame count == take segment frames ($WAVN)" || no "audio.wav frames $WAVN != take segment frames $SEG_FRAMES"
```
Run-level, at the top of section 13 (BEFORE the graceful quit at :897 -- the app must still answer):
```bash
FIN_COUNT="$(perf_field "d.get('finalizeErrors','NA')")"; FIN_COUNT="$(normnum "$FIN_COUNT")"
[ "$FIN_COUNT" = "0" ] && ok "no stop in this run reported a finalize problem (perf/status finalizeErrors == 0)" || no "perf/status finalizeErrors == $FIN_COUNT (some stop in this run truncated or failed to finalize)"
TRUNC_LOG="$(grep -c 'truncated (header' /tmp/adna-step3-err.log 2>/dev/null || echo 0)"
[ "$TRUNC_LOG" = "0" ] && ok "stderr: no '[Recorder] asset ...: truncated' line this run" || no "stderr: $TRUNC_LOG truncated-asset line(s) in /tmp/adna-step3-err.log"
```
Validate with `bash -n .harmony/probe-step3.sh` (and shellcheck if installed), as the header says.

### F6. Live check Harmony runs: `.harmony/probe-finalize-loop.sh` (new file, complete)
```bash
#!/bin/bash
# probe-finalize-loop.sh -- s-rta-0924b finalize-truncation live check.
# N record/stop cycles on the PRODUCTION app (built-in mic / input mode by default -- the config
# the defect was observed in; the mechanism is source-mode independent) and FAIL if ANY stop
# reports a finalize problem ("truncated (header N frames < framesWritten N+512)": one device
# block lost at stop, AudioTap::stopInternal ordering) via three INDEPENDENT oracles:
#   (1) /api/perf/status: lastFinalizeError per stop, finalizeErrors run total (F4)
#   (2) disk: each take's audio.unreliableFrom is null AND audio.json frames == audio.wav frames
#   (3) stderr: no "[Recorder] asset ...: truncated" line in the app log
# Rig rules exactly as probe-step3.sh: IPv4 127.0.0.1:7070, launch via `open` with stdout/stderr
# logs, bracket pgrep 'MacOS/Audio-DN[A]', graceful osascript quit, SCREEN-SAFETY (no endpoint
# used here reaches the Output window). Ruling 28: the N takes/assets it creates are NOT deleted
# (names finloop-<stamp>-<i>; the asset ids are printed at the end for Boris).
# Usage: bash .harmony/probe-finalize-loop.sh [N=40]
#   FINLOOP_BUILD_DIR=build-gate   FINLOOP_MODE=input|file (file: STEP3_CLICK_WAV, default
#   /tmp/click_48k.wav, generated if missing)   FINLOOP_HOLD_S=1.0 (base record length; a random
#   0-0.9 s is added per cycle so the stop's phase against the 10.7 ms callback grid varies)
# Statistics: at the observed ~2/23 (8.7%) rate a PRE-FIX run of 40 shows >=1 truncation with
# ~97% probability (0.913^40 = 2.7%); a POST-FIX 0/40 bounds the residual rate below ~7.2% (95%,
# rule of three). Use N=100 when time allows (0/100 -> <3%). The deterministic unit test
# (test_audio_tap_sync "[truncation]") is the proof; this loop is the live smoke check.
set -u
N="${1:-40}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${FINLOOP_BUILD_DIR:-build-gate}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
A='http://127.0.0.1:7070'
MODE="${FINLOOP_MODE:-input}"
HOLD="${FINLOOP_HOLD_S:-1.0}"
CLICK_WAV="${STEP3_CLICK_WAV:-/tmp/click_48k.wav}"
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
AUDIO_DIR="$HOME/Documents/Audio-DNA/Audio"
ERRLOG=/tmp/adna-finloop-err.log
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
perf_field(){ curl -s --max-time 5 "$A/api/perf/status" | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin); v = d.get('$1', 'NA'); print('' if v is None else v)
except Exception as e:
    print('NA(%s)' % e)"; }
wait_field(){ # $1 field $2 expected(python truthiness word: true|false) $3 max tenths
  local i=0; while [ $i -lt "$3" ]; do v="$(perf_field "$1")"; case "$2:$v" in true:True|true:true|false:False|false:false) return 0;; esac; sleep 0.1; i=$((i+1)); done; return 1; }

echo "$N" | grep -Eq '^[1-9][0-9]*$' || { echo "REFUSE: N must be a positive integer (got '$N')"; exit 64; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (FINLOOP_BUILD_DIR overrides)"; exit 64; }
if [ "$MODE" = "file" ] && [ ! -f "$CLICK_WAV" ]; then
    python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV" --interval 24000 >/dev/null || { echo "REFUSE: click WAV generation failed"; exit 64; }
fi

: > "$ERRLOG"
open --stdout /tmp/adna-finloop-out.log --stderr "$ERRLOG" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: /api/health never answered (mic-permission prompt? screencapture -x and LOOK)"; exit 1; }
ok "app launched, /api/health answered"
sleep 3
FIN0="$(perf_field finalizeErrors)"
[ "$FIN0" = "0" ] && ok "perf/status finalizeErrors == 0 at start" || no "perf/status finalizeErrors == '$FIN0' at start (field missing = build without F4)"

STAMP="$(date +%H%M%S)"; TRUNC=0; CYCLES_OK=0; ASSETS=""
for i in $(seq 1 "$N"); do
    NAME="finloop-$STAMP-$i"
    if [ "$MODE" = "file" ]; then BODY="{\"name\":\"$NAME\",\"audio\":true,\"audioFile\":\"$CLICK_WAV\"}"; else BODY="{\"name\":\"$NAME\",\"audio\":true}"; fi
    curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' -d "$BODY" >/dev/null
    if ! wait_field recording true 50; then no "cycle $i: recording never became true"; continue; fi
    AM="$(perf_field audioMode)"
    if [ "$i" = "1" ]; then
        [ "$AM" = "$MODE" ] && ok "cycle 1: audioMode == $MODE" || no "cycle 1: audioMode == '$AM', expected '$MODE' (switch the app's audio source, or set FINLOOP_MODE)"
    fi
    sleep "$(awk -v h="$HOLD" -v r="$RANDOM" 'BEGIN{printf "%.3f", h + (r % 900) / 1000.0}')"
    FW="$(perf_field framesWritten)"
    curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null
    if ! wait_field recording false 50; then no "cycle $i: recording never became false after stop"; continue; fi
    FIN="$(perf_field lastFinalizeError)"; LERR="$(perf_field lastError)"
    ASSET="$(perf_field assetId)"; ASSETS="$ASSETS $ASSET"
    TF="$TAKES_DIR/$NAME.adna-take"
    DISK="$(python3 - "$TF/take.json" "$AUDIO_DIR/$ASSET.adna-audio" <<'PY'
import json, sys, wave, os
try:
    t = json.load(open(sys.argv[1])); seg = t['audio']['segments'][0]
    a = json.load(open(os.path.join(sys.argv[2], 'audio.json')))
    w = wave.open(os.path.join(sys.argv[2], 'audio.wav'), 'rb'); n = w.getnframes()
    print("segframes=%d sidecarframes=%d wavframes=%d unreliableFrom=%s" % (seg['frames'], a['frames'], n, t['audio'].get('unreliableFrom')))
except Exception as e:
    print("NA(%s)" % e)
PY
)"
    if [ -n "$FIN" ]; then
        TRUNC=$((TRUNC+1)); echo "  cycle $i: TRUNCATION  lastFinalizeError='$FIN' lastError='$LERR' framesWritten(before stop)=$FW  $DISK"
    else
        case "$DISK" in
            *"unreliableFrom=None"*) SF="${DISK#*segframes=}"; SF="${SF%% *}"; WF="${DISK#*wavframes=}"; WF="${WF%% *}"
                if [ "$SF" = "$WF" ] && [ "${FW:-0}" != "0" ] && [ "$FW" != "NA" ]; then CYCLES_OK=$((CYCLES_OK+1)); echo "  cycle $i: clean  $DISK"; else no "cycle $i: status clean but disk/frames disagree ($DISK framesWritten=$FW)"; fi;;
            *) no "cycle $i: status clean but disk says $DISK";;
        esac
    fi
done

FIN_END="$(perf_field finalizeErrors)"
[ "$TRUNC" = "0" ] && ok "$N record/stop cycles, 0 stops reported a finalize problem (oracle 1: lastFinalizeError)" || no "$N record/stop cycles, $TRUNC truncated stop(s) (oracle 1)"
[ "$FIN_END" = "$TRUNC" ] && ok "perf/status finalizeErrors ($FIN_END) agrees with the per-cycle count ($TRUNC)" || no "perf/status finalizeErrors ($FIN_END) != per-cycle count ($TRUNC)"
[ "$CYCLES_OK" = "$N" ] && ok "disk: all $N takes have unreliableFrom null and take frames == audio.wav frames (oracle 2)" || no "disk: only $CYCLES_OK of $N takes are clean on disk (oracle 2)"
TL="$(grep -c 'truncated (header' "$ERRLOG" 2>/dev/null || echo 0)"
[ "$TL" = "0" ] && ok "stderr: 0 '[Recorder] asset ...: truncated' lines (oracle 3)" || no "stderr: $TL truncated-asset line(s) in $ERRLOG (oracle 3)"

osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1; sleep 2; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING after graceful quit + pkill" || ok "app terminated"
echo; echo "$N cycles, $TRUNC truncations.  $PASS PASS / $FAIL FAIL"
echo "assets created this run (Ruling 28, not deleted):$ASSETS"
echo "takes: $TAKES_DIR/finloop-$STAMP-*.adna-take"
[ "$FAIL" -eq 0 ] || exit 1
```
Fail-first for the loop itself: run it once against the PRE-FIX binary (`FINLOOP_BUILD_DIR=build`
after `cmake --build build`) -- expected >=1 truncation (about 3-4 of 40 at the observed rate);
then against the post-fix build-gate -- expected `40 cycles, 0 truncations`.

### F7. Docs/comments (same commit)
- AudioStore.cpp:207-210 comment: name the closed stop-window as the other source this rule
  catches, and that F1 closed it.
- CLAUDE.md "Audio Store (Ruling 28)" paragraph: append `lastFinalizeError`, `finalizeErrors` to the
  listed `/api/perf/status` fields.
- .harmony/gotchas.md (Harmony's call): "a stop() that nulls the writer but clears the audio
  thread's gate last leaves a counted-but-never-written block; gate first, then wait."

### Build order (fail-first discipline) and "done"
1. F2 hooks (no app behaviour change) + F3 test A. Build `test_audio_tap_sync`; run
   `ctest -R test_audio_tap_sync` -> RED with `5120 == 5632` and `512 == 0`. Record the output.
2. F1. Re-run -> GREEN; whole `test_audio_tap_sync` GREEN (UAF test :624 included).
3. F4 items 1-5, 7 + Test B (write Test B first if you want its own RED: `after.lastError` empty).
   `ctest -R "test_recorder_host|test_audio_store|test_audio_tap_sync"` GREEN; then full `ctest`
   (438 -> 440).
4. RT grep (CLAUDE.md sacred rules): `git diff src/recording/AudioTap.cpp` on the push() path
   shows only the memory_order token change; no `new/malloc/mutex/printf` anywhere in the diff.
5. F5 + F6; `bash -n` both scripts.
6. Live (Harmony, "the party that builds never verifies"): forced rebuild in build-gate;
   `bash .harmony/probe-finalize-loop.sh 40` -> `40 cycles, 0 truncations`, all oracles PASS
   (N=100 if time allows); then the full `bash .harmony/probe-step3.sh` -> new rows PASS, no FAIL.
   Optional but recommended baseline: the same loop on the pre-fix ./build shows >=1 truncation.
7. F7 docs; commit as one fix lane (mechanism + tests + visibility + probes).

## 5. RISKS
- Strongest counterargument to F1: "the drains at :473/:486 exist precisely to catch a post-null
  push -- just add a second wait + re-drain after `running_=false`". It works, but keeps the
  non-atomic `pendingFrames_` race between the first drain and a concurrent push and needs two
  waits; F1 makes everything after the single wait exclusive by construction with fewer lines.
- `isRunning()` now flips false at the START of stop() rather than the end. Consumers VERIFIED:
  RecorderHost.cpp:284 reads it BEFORE `tap.stop()`; :416 only inside `if (recording_ &&
  tapWasStarted_)` on the tick, never during stop(). No other consumer (grep of src/).
- After F1, the block(s) that arrive during stop() advance `deliveredSamples_`
  (CombinedCallback.h:134) but not `framesWritten_`, so `framesWritten == delivered - firstSample`
  holds only up to the stop instant. That is the honest state (the audio ends there); events
  captured between `tap.stop()` (:285) and `recorder_.stop()` (:316) -- same message-thread call,
  microseconds apart -- may be stamped up to one block past the audio end. Pre-existing, harmless,
  disclosed. Existing tests asserting the equality (test_audio_tap_sync.cpp:588) never push during
  stop, so they stay valid.
- The arm-capture race (:180-183) is why the trailing stores at :498-499 must stay; a Builder
  "cleaning up" the now-duplicate stores would reintroduce a stuck `running_==true` tap in that
  rare start->immediate-stop ordering.
- Test A relies on the hook sitting after the drains and before `reset()`. Placed before the
  drains it would go GREEN pre-fix (the :473 drain would rescue the spill) and prove nothing.
- Statistical power of the live loop: 0/40 only bounds the residual rate below ~7.2% (95%);
  the unit test is the proof. Ask for N=100 if the gate has the time (~5-6 min).
- Mirroring into `lastError_`: probe-step3 :302 and 11L :798 rows now go red on truncation
  (intended); RecordPanelModel.h:222 only shows `lastError` while recording, so the panel is
  unchanged (its notice already says "Saved ..., but its audio had a problem").
- ASSUMED: 40 x ~1.5 s mic takes (~12 MB of assets) are acceptable under Ruling 28's never-delete
  store; the script prints the ids for manual cleanup.
- Out of scope, noted: `res.ok=false` with an empty `res.error` when only `take.save` fails
  (RecorderHost.cpp:327-332 -> "Could not stop the take: "); optional O1 above touches the
  adjacent status gap only.

STATUS: COMPLETE -- mechanism proven from source (AudioTap.cpp:190/:226/:398/:310 vs :445-:499),
lastError gap proven (RecorderHost.cpp:306-311/:719), fix + hooks + 2 fail-first tests + probe rows
+ live loop specified; no open questions for the Builder.
