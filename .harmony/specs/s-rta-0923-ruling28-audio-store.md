# s-rta-0923 — Ruling 28: the shared audio store (take format v3), plus the two fixes it makes load-bearing

Author: Architect (Fable), 2026-09-23, secondary lane for RealTimeAudio. Read-only analysis.
INTENDED PATH (dispatch-named, refused by the read-only write fence — Harmony persists per A54):
`/Users/boriskarpman/projects/RealTimeAudio/.harmony/specs/s-rta-0923-ruling28-audio-store.md`
Confidence labels: VERIFIED = read on disk at the cited line; INFERRED = derived from cited code; ASSUMED = stated, not checked.

QUESTION: Boris ruled (`.harmony/binding-decisions.md:436-466`) that the recorded audio is the ANCHOR and many takes share
one audio. Today `AudioTap` writes `audio.wav` wherever `AudioTap::start(file)` is pointed (`src/recording/AudioTap.cpp:105-144`,
VERIFIED) and the only shipped caller points it INSIDE the take folder (`tests/test_audio_tap_sync.cpp:357`, VERIFIED — no
production caller exists yet; grep `audioTap|AudioTap` in `src/` outside `src/recording` and `src/audio` returns nothing,
VERIFIED). Design the shared store, the take-side reference, the v3 format, the migration story, the missing-audio policy,
the GC policy; fold in review fix (c) (`RecorderClock` periodic anchor, `.harmony/.reports/s168-review-lane-a.md:207-209`)
and HANDOFF addendum item 2 (`Player::advanceTo` backwards seek, `.harmony/HANDOFF.md:2521-2526`).

APPROACH (verdict first): **an app-managed store `~/Documents/Audio-DNA/Audio/<id>.adna-audio/{audio.wav, audio.json}`,
keyed by a random 128-bit id minted at record-start, self-described by an `audio.json` sidecar written LAST (its presence is
the "complete" flag), fingerprinted by a cheap deterministic head+tail+length SHA-256 (`fp1`) that is recomputed and verified
on EVERY load (~10 ms), referenced from the take by `{id, fingerprint, firstSample, frames, rate, channels}`; take format
`version 3 / minReader 3`; the take folder holds `take.json` ONLY.** Missing or mismatched audio REFUSES play-with-audio with
a reason and offers wall-clock replay only as an explicit choice. Nothing is ever deleted by code in this step (GC is a
query; deletion is a Boris question). Fix (c) = a `why:"periodic"` anchor every 32 beats (8 bars of 4) while metered. Item 2
= a backwards `pos` is a defined SEEK (re-seat cursors, release gestures no longer covering `pos`, events re-fire on re-pass),
plus an explicit `Player::seek()` for deliberate forward repositioning. Three builder lanes, disjoint files: A (store +
format), B (tap→store proof, depends on A), E (clock + player, independent).

---

## 0. Scope, non-goals, R8

IN: `src/recording/AudioStore.{h,cpp}` (new); `AudioRef` + `Take` version bump; two fixtures; `RecorderClock` periodic
anchor; `Player` seek; tests; CMake. All headless — proven the way s168 proved everything (ctest), no app launch required.

OUT (named so nobody "helpfully" adds them): MainComponent/RecordPanel wiring of the record→store→take sequence (spec step 3
— §5 below is the CONTRACT that wiring must follow, and Lane B's test is its executable form); review fixes (a) v1
TransportChange, (b) exact per-breakpoint stamps in `Program::compile`, (d) RecordPanel inert buttons — all still owed, not
here; multi-segment audio (`audio-2.wav` on a mid-take rate change — still NOT built, now refused loudly at resolve time);
any deletion code; a store-location preference; importing external WAVs into the store; record-over-with-seeks semantics
for the recorder clock; a full-content hash (door left open, §3).

R8: nothing in this spec defines or touches `manualWrite`, `AutomationCurve`, or the connection engine. The recorder still
HOOKS `manualWrite` (step 3) and never defines it. VERIFIED no file in this spec's lanes lives under `src/connect/`.

---

## 1. Decisions (each with the reason and the strongest counter)

**D-A1 — Identity is a random id, not a path and not a hash.** `id = juce::Uuid().toString()` (32 hex chars,
`build/_deps/juce-src/modules/juce_core/misc/juce_Uuid.h:86-88` VERIFIED). Folder `<root>/<id>.adna-audio/`. A take
references the id; the store root is re-derived at runtime, so moving the whole `Audio-DNA` folder to another machine keeps
every reference valid. Counter: "reference by relative path `../Audio/x.wav`" — a path is exactly what breaks when the user
reorganises; loses. Counter: "id = content hash" — the id must exist at record-START (the tap needs a file to write into)
and the content does not exist until record-STOP; loses.

**D-A2 — Content check is `fp1`, a cheap deterministic fingerprint, verified at every load.** Definition in §3. Cost ≈ 2 MiB
read + one SHA-256 ≈ 10 ms, so it runs synchronously at finalize AND at every load/resolve. Counter (strongest): "Boris said
content hash; hash the whole file." A whole-file SHA-256 of a 4-hour stereo 16-bit WAV (2.76 GB, `s167 spec:622`) with
JUCE's one-shot, non-incremental `SHA256(InputStream&)` (`juce_SHA256.h:82`, VERIFIED — no `update()` API exists) is
~10-30 s (ASSUMED throughput 100-300 MB/s), which cannot run on the message thread at every load and would need async
plumbing (thread, completion, a window where the take is saved before the hash exists) at finalize. `fp1` catches every
realistic failure — wrong file, moved, replaced, truncated, length-edited — and the sidecar is versioned, so a full
`sha256` field can be ADDED later without a format change. Loses on cost/benefit now; honestly named `fp1`, not "sha256".

**D-A3 — The sidecar is written last, atomically; its presence means "complete".** `juce::File::replaceWithText` goes
through `TemporaryFile` + rename (`juce_File.cpp:798-800`, VERIFIED). A crash mid-show leaves a folder with `audio.wav`
and no sidecar = an INCOMPLETE asset. JUCE's `ThreadedWriter` calls `writer->flush()` periodically
(`juce_AudioFormatWriter.cpp:306-311`, VERIFIED) and the WAV writer rewrites its header on every flush
(`juce_WavAudioFormat.cpp:1670,1687`, VERIFIED), so a crashed recording's WAV is readable up to the last flush. That is why
incomplete assets are NEVER auto-deleted (§7): `finalize()` run later on the orphan is the whole repair.

**D-A4 — The take keeps the v2 `audio` section SHAPE (segments[], mode, gapDetection, gaps, unreliableFrom) and ADDS
`id` + `fingerprint` per segment; `file` becomes read-only legacy; `sha1Head` is dropped (never written by any shipped code
— only `Take.h:27`/`Take.cpp:15,29` mention it, VERIFIED grep).** D12 "ADD, never REDEFINE" (`s167 spec:670`). `segments[]`
stays an array so the not-built multi-segment case remains expressible; `resolve()` refuses `size() > 1` loudly.

**D-A5 — `version 3 / minReader 3`, constant.** A store-referenced take must not be readable by a v2 reader: it would find
`file == ""`, open the take folder itself as a WAV, fail, and degrade to "no audio" SILENTLY — exactly what D12 forbids.
No v2 reader exists outside this repo's git history and no real take was ever recorded (`HANDOFF.md:2434`, VERIFIED), so
the bump costs nothing. Counter: "minReader 2 for no-audio takes" — a branch for a reader that does not exist; loses.

**D-A6 — `Take::load` stays pure JSON; resolution against the store is a separate call.** `Take` is a value type used by
filesystem-free tests (`Take.h:100-104`, VERIFIED). `AudioStore::resolve(take.audio)` does the disk checks and returns a
status + reason; the loader only FLAGS (`LoadStats.legacyInFolderAudio`).

**D-A7 — Missing/mismatched audio: refuse play-with-audio, loudly; wall-clock replay only as an explicit choice.** D11 #1
already sets this principle for render ("explicit option … never a silent fallback", `s167 spec:639-641`, VERIFIED).
Ruling 28 makes the audio the master; playing the knobs against nothing without saying so is the lie.

**D-A8 — GC: never delete anything automatically; this step ships the QUERY only.** A take whose `take.json` fails to
parse blocks the orphan report entirely (an unreadable take might reference the "orphan"). Deletion policy → Boris (§13).

**D-A9 — Sample domains.** Inside ONE take, every `sample` stamp and every `audio.gaps[].sample` /
`unreliableFrom` is in the take's clock domain (today: the delivered-sample counter, `CombinedCallback.h:105,134`,
VERIFIED). `firstSample` is the take-clock value at asset frame 0, so `assetFrame = sample − firstSample`
(`AudioTap.h:66-67` "WAV frame k ⇔ delivered sample firstSample + k", VERIFIED). The SIDECAR stores gaps/unreliableFrom in
the ASSET-FRAME domain (shared by all takes); `finalize()` converts in, `referencing()` converts out. A fresh take recorded
against stored audio must be given a clock whose `sample` IS the asset frame (then `firstSample = 0`) — that is step 3's
job and the one invariant it must honour (§12 R3).

---

## 2. On-disk layout

```
~/Documents/Audio-DNA/                       (existing convention: Recordings/ MainComponent.cpp:5231-5232, Snapshots/ Renderer.cpp:1961-1962 — VERIFIED)
├── Audio/                                   AudioStore::defaultRoot()   ← the shared store (this spec)
│   └── <id>.adna-audio/
│       ├── audio.wav                        written by AudioTap during the show (unchanged class)
│       └── audio.json                       sidecar, written LAST by AudioStore::finalize; absent ⇒ incomplete
└── Takes/                                   default home for Name.adna-take/ (step 3/4 picks names; not this spec)
    └── Friday-take-3.adna-take/
        └── take.json                        ONLY. No audio.wav, ever again.
```

### 2.1 Sidecar `audio.json` (sidecar format v1)
```jsonc
{ "format": "audiodna-audio", "version": 1,
  "id": "3f2a9c0e1b7d4e5f8a6b2c1d0e9f8a7b",           // == folder stem; find() refuses a mismatch
  "fingerprint": "fp1:<64 lowercase hex>",             // §3
  "frames": 173145600, "rate": 48000, "channels": 2, "bits": 16,
  "recordedAt": "2026-09-23T21:14:02Z", "app": "0.1.0",
  "mode": "input",                                     // "input" | "file" (what the app listened to, D10.1)
  "gapDetection": true,                                // R14 flag — ASSET-level fact
  "gaps": [ { "frame": 9826816, "n": 1024 } ],         // ASSET-FRAME domain (frame = sample − firstSample)
  "unreliableFrom": null }                             // asset-frame domain or null
```

### 2.2 Take `take.json` v3 — only the envelope and `audio` change
```jsonc
{ "format": "audiodna-take", "version": 3, "minReader": 3,
  "features": ["lanes", "tempoMap", "checkpoint0", "audio"],          // unchanged list; the version says the rest
  "audio": {
    "segments": [ { "id": "3f2a9c0e1b7d4e5f8a6b2c1d0e9f8a7b", "fingerprint": "fp1:…",
                    "firstSample": 3584, "frames": 173145600, "rate": 48000, "channels": 2 } ],
    "mode": "input", "gapDetection": true,
    "gaps": [ { "sample": 9830400, "n": 1024 } ],                     // TAKE-clock domain, as today
    "unreliableFrom": null },
  … everything else exactly as v2 (Take.cpp:139-176) … }
```
Legacy v2 segment as read from an old file: `{ "file": "audio.wav", "firstSample": …, "frames": …, "rate": …, "channels": …, "sha1Head": "" }`
→ `Segment.id == ""`, `Segment.file == "audio.wav"`, `sha1Head` ignored; re-emitted with `file` (and no `id`) so a
legacy take round-trips; flagged by the loader; refused by `resolve()` as `Legacy`.

---

## 3. `fp1` — the fingerprint, exactly

Inputs: the WAV opened with `juce::WavAudioFormat::createReaderFor(new juce::FileInputStream(wav), true)`; `N =
reader->lengthInSamples`, `R = roundToInt(reader->sampleRate)`, `C = reader->numChannels`, `B = reader->bitsPerSample`
(`juce_AudioFormatReader.h:240`, VERIFIED). `W = 262144` frames.

1. `header = "adna-fp1|frames=" + N + "|rate=" + R + "|channels=" + C + "|bits=" + B + "|"` (ASCII, no trailing newline).
2. Windows: if `N <= 2W` → one window `[0, N)`; else two windows `[0, W)` and `[N−W, N)`.
3. For each window, read via the INT overload `reader->read(int* const* dest, C, start, count, false)`
   (`juce_AudioFormatReader.h:139-143`, VERIFIED signature) into `C` int buffers; emit frames interleaved, channel
   0..C−1, each sample as PCM16 little-endian: `int16 v = (int16_t)(sample >> 16)` (JUCE left-justifies integer PCM in
   the 32-bit int, so for a 16-bit file this is the exact stored value — INFERRED from the reader contract; the builder
   proves it with the fixture test in Lane A: a synthetic 16-bit WAV of known samples produces the expected bytes).
4. `fingerprint = "fp1:" + juce::SHA256(bytes.getData(), bytes.getSize()).toHexString()` (`juce_SHA256.h:74,102`,
   VERIFIED). `bytes` is one `juce::MemoryBlock` of ≤ header + 2·W·C·2 bytes (≈ 2 MiB stereo) — bounded, no 2.8 GB read.
5. `N == 0` is allowed (header only). A 24-bit file fingerprints on its top 16 bits (documented; the tap writes 16-bit,
   `AudioTap.cpp:119`, VERIFIED).

Requires linking `juce::juce_cryptography` (module present in the fetched JUCE tree,
`build/_deps/juce-src/modules/juce_cryptography`, VERIFIED) — CMake is Lane A's.

---

## 4. Code surface

### 4.1 `src/recording/AudioStore.h` (new, Lane A) — signatures are the contract
```cpp
#pragma once
#include "recording/Take.h"
#include <juce_core/juce_core.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// AudioAsset -- the sidecar. Everything a take needs to reference the audio
// and everything a FRESH take against it copies (ruling 28 / ruling 16).
struct AudioAsset
{
    std::string id;                 // 32 lowercase hex == folder stem
    std::string fingerprint;        // "fp1:" + 64 hex
    uint64_t frames = 0;
    double rate = 0.0;
    int channels = 0;
    int bits = 16;
    std::string recordedAt;         // ISO-8601 UTC
    std::string app;
    std::string mode;               // "input" | "file"
    bool gapDetection = false;
    std::vector<std::pair<uint64_t, uint32_t>> gaps;   // ASSET-FRAME domain
    std::optional<uint64_t> unreliableFrom;            // asset-frame domain

    static constexpr int kSidecarVersion = 1;
    juce::var toVar() const;
    static std::optional<AudioAsset> fromVar(const juce::var& v);   // nullopt: not an object / wrong format / no id
};

// AudioStore -- ruling 28's shared audio store. Message-thread only
// (blocking file I/O is fine there; same posture as AudioTap::start/stop).
class AudioStore
{
public:
    explicit AudioStore(juce::File root);
    static juce::File defaultRoot();                         // ~/Documents/Audio-DNA/Audio
    const juce::File& root() const { return root_; }

    static constexpr const char* kAssetSuffix = ".adna-audio";
    juce::File assetFolder(const std::string& id) const;     // <root>/<id>.adna-audio
    juce::File wavFile(const std::string& id) const;         // <folder>/audio.wav  -- hand THIS to AudioTap::start()
    juce::File sidecarFile(const std::string& id) const;     // <folder>/audio.json

    // --- recording a new asset (sequence in spec §5.1) ---
    // Mints an id and CREATES the folder (it must exist before AudioTap::start():
    // AudioTap.cpp:109 checks free space on the wav's parent -- a nonexistent
    // parent reads as 0 bytes free and the tap refuses). nullopt if the folder
    // cannot be created.
    std::optional<std::string> beginAsset();

    struct CaptureFacts
    {
        std::string mode;                                    // "input" | "file"
        bool gapDetection = false;                           // AudioTap::gapDetectionSupported()
        uint64_t firstSample = 0;                            // AudioTap::firstSample()
        std::vector<std::pair<uint64_t, uint32_t>> gapsInTakeClock;   // drained via AudioTap::popGap (take-clock domain)
        std::optional<uint64_t> unreliableFromInTakeClock;   // AudioTap::unreliableFrom()
        std::string app;
    };
    // Reads the finished WAV header, computes fp1, converts gaps/unreliableFrom
    // to the asset-frame domain (saturating: frame = sample - firstSample, 0 if
    // sample < firstSample), writes the sidecar LAST via File::replaceWithText.
    // nullopt (and NO sidecar) if the wav is missing/unreadable or the write fails.
    std::optional<AudioAsset> finalize(const std::string& id, const CaptureFacts& facts);

    // --- lookup ---
    std::optional<AudioAsset> find(const std::string& id) const;   // sidecar parses AND sidecar.id == id
    bool isIncomplete(const std::string& id) const;                 // audio.wav exists, audio.json does not
    std::vector<std::string> listAssetIds() const;                  // complete only
    std::vector<std::string> listIncompleteAssetIds() const;

    // --- resolution for a take (never silent) ---
    enum class Status { NoAudio, Resolved, Missing, Incomplete, Mismatch, Legacy, MultiSegment };
    struct Resolution
    {
        Status status = Status::NoAudio;
        std::string reason;          // human-readable, names the id and what differed; empty for NoAudio/Resolved
        juce::File wav;              // valid only when Resolved
        AudioAsset asset;            // valid only when Resolved
        uint64_t firstSample = 0;    // the take's segment firstSample (assetFrame = sample - firstSample)
    };
    Resolution resolve(const AudioRef& ref) const;

    // --- referencing (a fresh take against an existing asset; ruling 16/28) ---
    // firstSample: the take-clock value at asset frame 0 (0 when the take's
    // clock IS the asset frame counter -- the overdub case, spec §5.2).
    static AudioRef referencing(const AudioAsset& asset, uint64_t firstSample);

    // --- fp1 (spec §3) ---
    static constexpr uint32_t kFingerprintWindowFrames = 262144;
    static std::optional<std::string> fingerprint(const juce::File& wav);   // "fp1:<hex>", nullopt if unreadable

    // --- GC: QUERY ONLY. There is deliberately no delete API in this step (spec §7). ---
    struct ReferenceScan
    {
        std::vector<std::string> referencedIds;     // union over every readable take
        std::vector<juce::File> unreadableTakes;    // take.json missing/unparseable -- if non-empty, no orphan verdict is valid
    };
    static ReferenceScan scanTakes(const juce::File& takesRoot);   // recursive "*.adna-take" search, reads ONLY the "audio" section
    // Complete assets referenced by no readable take. Returns nullopt (refuses a verdict) if scan.unreadableTakes is non-empty.
    std::optional<std::vector<std::string>> unreferencedAssetIds(const juce::File& takesRoot) const;

private:
    juce::File root_;
};
```
`resolve()` order (each step returns on failure with `reason`): (1) `ref.segments.empty() && ref.mode.empty()` → `NoAudio`;
(2) `segments.size() > 1` → `MultiSegment`; (3) `seg.id.empty()`: `!seg.file.empty()` → `Legacy` ("recorded in-folder by a
pre-v3 build; re-record"), else `Mismatch` ("audio reference has no id"); (4) folder or wav absent → `Missing`; wav present,
no sidecar → `Incomplete`; sidecar id ≠ folder id → `Mismatch`; (5) `asset.{frames,rate,channels}` ≠ `seg.{…}` →
`Mismatch` listing both; (6) reader header ≠ asset → `Mismatch`; (7) `fingerprint(wav) != seg.fingerprint` → `Mismatch`
("content differs from what this take was recorded against"); (8) `Resolved`.

### 4.2 `src/recording/Take.h` / `Take.cpp` (Lane A)
- `AudioRef::Segment`: ADD `std::string id;` and `std::string fingerprint;`; KEEP `file` (comment: legacy v2 in-folder
  path, read-only, never set by v3 code); REMOVE `sha1Head` (read path ignores it; nothing else references it — VERIFIED).
- `Segment::toVar` (`Take.cpp:7-17`): emit `id`/`fingerprint` when non-empty; emit `file` only when non-empty; never
  emit `sha1Head`. `Segment::fromVar` (`:19-32`): read `id`, `fingerprint`, `file` (missing → "").
- `Take::kFormatVersion = 3; kMinReader = 3;` (`Take.h:83-84`). The `minReader > kFormatVersion` refusal (`Take.cpp:192-200`)
  keeps working: v2 files (minReader 2) load; v1 bridge (`:189-190`, `:299-402`) untouched.
- `LoadStats`: ADD `bool legacyInFolderAudio = false;` set in `Take::fromVar` after `AudioRef::fromVar` when any segment has
  `id.empty() && !file.empty()`.
- `knownTopLevelSections`/`knownFeatures` (`Take.cpp:121-136`): unchanged.

### 4.3 CMake (Lane A owns both files)
- `CMakeLists.txt`: add `src/recording/AudioStore.h` + `src/recording/AudioStore.cpp` after line 245 (the AudioTap pair,
  VERIFIED list at :229-247); add `juce::juce_cryptography` to the app's module list (:528-537).
- `tests/CMakeLists.txt`: (i) NEW target `test_audio_store` (sources: `test_audio_store.cpp`, `recording/AudioStore.cpp`,
  `recording/Take.cpp`, `recording/TempoMap.cpp`, `recording/PerfState.cpp`, plus the same model/effects .cpp list
  `test_take` links at :274-288 if `Take.h`'s includes need them — the builder checks; links `Catch2::Catch2WithMain
  juce::juce_core juce::juce_events juce::juce_graphics juce::juce_audio_basics juce::juce_audio_formats
  juce::juce_cryptography`; defines `TEST_FIXTURES_DIR` like :296-302; `apply_sanitizers` + `catch_discover_tests`).
  (ii) `test_audio_tap_sync` (:325-358): add `${SRC_DIR}/recording/AudioStore.cpp` and `juce::juce_cryptography` — this is
  what lets Lane B build without touching CMake.

### 4.4 `RecorderClock` (Lane E) — see §8.  `Player` (Lane E) — see §9.

---

## 5. Sequences (the contract step 3 wires; Lane B proves 5.1 + 5.4 headlessly)

### 5.1 Live recording (audio switch ON — ruling 19, `binding-decisions.md:353-355`)
```
AudioStore store(AudioStore::defaultRoot());
auto id = store.beginAsset();                                  // folder exists now
if (!id)                        → refuse to arm: "cannot create <root>"; no take starts
if (!tap.start(store.wavFile(*id)))                            // R15 (<2 GB on the STORE's volume, AudioTap.cpp:109) or writer failure
                                → refuse to arm, surface it, delete the empty asset folder
recorder.start(comp, clock, takeFolder);
… show …
tap.stop();                                                    // flushes + closes; header patched (AudioTap.cpp:415-475)
AudioStore::CaptureFacts f{ mode, tap.gapDetectionSupported(), tap.firstSample(), gapsDrainedViaPopGap,
                            tap.unreliableFrom(), appVersion };  // HANDOFF item 3: this is where the R14 flag finally reaches a take
auto asset = store.finalize(*id, f);                           // sidecar written LAST
Take take = recorder.stop(comp);
if (asset) take.audio = AudioStore::referencing(*asset, tap.firstSample());
else       { take.audio = {}; surface "audio finalize failed" — the WAV stays on disk as an INCOMPLETE asset for later repair; the take is still saved }
take.save(takeFolder);                                         // take.json only
```
Invariant the builder asserts in Lane B: `asset->frames == tap.framesWritten()` for a clean stop (the stop()'s "give up
rather than spin" branches, `AudioTap.cpp:453,467`, are the only way they differ, and then `unreliableFrom` says why).

### 5.2 A FRESH take against existing audio (the re-do)
```
auto asset = store.find(id);   if (!asset) → refuse with reason
take.audio = AudioStore::referencing(*asset, /*firstSample*/ 0);
```
`firstSample = 0` because the recorder's clock MUST be fed the stored audio's playback frame position as `deliveredSamples`
(one sample domain per take, D-A9). That feeding is step 3's job (playback-with-audio drives from `sample`, `s167
spec:568-569`); the format's only requirement is stated here.

### 5.3 Fork / "save as a new file connected to the same audio" (ruling 16, `binding-decisions.md:326-333`)
`Take copy = original; copy.meta.recordedAt = now; copy.save(newFolder);` — the reference is a value; no bytes of audio move.
Lane A test proves two folders → one asset.

### 5.4 Load + play
```
auto take = Take::load(folder, stats);      if (stats.legacyInFolderAudio) surface it
auto r = store.resolve(take->audio);
NoAudio      → replay on DriveClock::Wall as today
Resolved     → play with audio (DriveClock::Sample; pos = playbackFrame + r.firstSample)
anything else→ REFUSE play-with-audio; show r.reason; offer "Play without audio (wall clock)" as an explicit action
```

### 5.5 Audio switch OFF
Skip 5.1's store/tap lines entirely; `take.audio` stays default (mode "", no segments) — unchanged from today.

---

## 6. Missing / moved audio — refuse loudly, degrade only by explicit choice
Verdict table: `Missing` (folder or wav gone), `Incomplete` (wav but no sidecar), `Mismatch` (id/frames/rate/channels/
fingerprint differ), `Legacy` (pre-v3 in-folder audio), `MultiSegment` (not built) → all refuse play-with-audio, all carry a
reason naming the id and the root that was searched. No status ever degrades to wall clock on its own. `NoAudio` is not an
error (the switch was off). Rationale D-A7.

---

## 7. Garbage collection policy
1. **Never auto-delete.** Not orphans, not incomplete assets (D-A3: an incomplete asset is a crashed show's audio; the
   repair is `finalize()`, LATER), not on take deletion.
2. This step ships `scanTakes` + `unreferencedAssetIds` only. Any unreadable `take.json` under the takes root voids the
   orphan verdict (returns nullopt) — a broken take may reference the very asset about to be called unused.
3. Proposed LATER UI: "Clean up unused audio…" listing id, recordedAt, size, and which takes reference it; deletion only
   after confirmation; incomplete assets shown separately with "Repair" (= finalize) before "Delete". Whether deletion is ever
   offered at all, and whether deleting the LAST take of an audio should prompt to delete the audio, are Boris's calls (§13).

---

## 8. Fix (c) — `RecorderClock` periodic tempo anchor (Lane E)
Measured problem (`s168-review-lane-a.md:35-54`, VERIFIED): `RecorderClock::tick` writes anchors only on bpm change > 0.05,
resync "reset", lock/unmetered edges and "start" (`RecorderClock.cpp:16,28,39,56,64`, VERIFIED). A steady-tempo take has ONE
anchor for its whole span; `TempoMap::beatAt` extrapolates at that anchor's bpm (`TempoMap.cpp:63-70`) — 0.03 BPM bias ×
40 min ≈ 1.2 beats. Worse and unreported: with a single anchor `TempoMap::sampleAt` has NO rate source (both branches at
`TempoMap.cpp:91-104` need two anchors) so it returns `anchor.sample` for every `t` (VERIFIED by reading :81-108) — every
`DriveClock::Sample` compile of such a take collapses every continuous gesture to `x0 == x1` (`Program.cpp:180,232-233`).
Ruling 28 (2-4 hour takes; audio-locked replay) makes both load-bearing.

Change (`RecorderClock.h/.cpp` only):
- `static constexpr double kPeriodicAnchorBeats = 32.0;` (8 bars × 4 beats — the app's bar is 4 beats,
  `FeatureSnapshot.h:44` `beatInBar 0-3`, VERIFIED; spec D1 "at least every 8 bars", `s167 spec:121-122`).
- `double lastAnchorBeat_ = 0.0;` + private `void anchor(double t, double beat, uint64_t sample, float bpm, const char* why)`
  that does `tempo_.append(...)` AND `lastAnchorBeat_ = beat`. Route all five existing `tempo_.append` sites through it.
- In the metered branch (`RecorderClock.cpp:41-65`), after the existing bpm-change check, if no anchor was written this tick
  and `beatNow − lastAnchorBeat_ >= kPeriodicAnchorBeats` → `anchor(t, beatNow, deliveredSamples, snap.bpm, "periodic")`.
- No periodic anchors while unmetered (bpm == 0): keeps `TempoMap::tAt`'s documented unmetered semantics
  (`TempoMap.h:41-45`) unchanged — several equal-beat anchors would move its canonical answer. Documented limitation: a long
  unmetered stretch still has only its edge anchors (t↔sample editor conversions there are bounded by the crystal drift the
  spec already accepts, `s167 spec:566-569`).
Cost: 4 h at 128 BPM → 30720/32 = 960 anchors ≈ 100 KB of JSON. Fine.

---

## 9. Addendum item 2 — `Player` backwards seek (Lane E)
Problem (`Player.cpp:36-45`, VERIFIED): `advanceTo` sets `pos_ = pos` and only ever moves `nextDiscrete_`/gesture cursors
forward; a smaller `pos` silently skips everything already passed, forever, and keeps calling `set(curve.eval(pos))` on a
gesture `pos` is no longer inside. Ruling 28's cleanup loop is scrubbing over the audio, and D10.2 drives `pos` from the
audio player's position.

Change (`Player.h/.cpp` only):
- NEW `void seek(double pos, Sink& sink);` — running only (no-op otherwise, like `advanceTo`). Semantics: for every cursor
  `inGesture`: if the current gesture still covers `pos` (`g.x0 <= pos < g.x1`) keep `inGesture`/`displaced` as they are
  (the grip is retained; the next `advanceTo` re-evaluates the curve at the new `pos`); else `release()` if not displaced
  and reset the cursor. Then re-seat exactly as `start(at)` does (`Player.cpp:17-33`): `nextDiscrete_` = first event with
  `at >= pos`; each `gestureIndex` = first gesture with `x1 > pos`. `pos_ = pos`. NO state synthesis (no "fire the last
  event ≤ pos" preamble) — same contract as today's mid-take `start()` (`Player.h:36-40`), disclosed.
- `advanceTo(pos)`: if `pos < pos_` → `seek(pos, sink)` first (a backwards jump is never a stall), then the existing
  forward pass. A FORWARD jump through `advanceTo` keeps exactly-once catch-up semantics (D3; the 400 ms stall test) —
  callers that reposition forward deliberately MUST call `seek()`; documented on both methods. No epsilon: any decrease is
  a seek (audio positions are monotonic while playing).
- `swap()`/`stop()`/`start()` unchanged.

---

## 10. Builder lanes — DISJOINT files, own `-B` dir each (HANDOFF rig fact `:2493`)

### Lane A — store + format v3 (`cmake -B build-laneA`)
Owns: `src/recording/AudioStore.h`, `src/recording/AudioStore.cpp` (new); `src/recording/Take.h`, `src/recording/Take.cpp`;
`CMakeLists.txt`; `tests/CMakeLists.txt`; `tests/test_audio_store.cpp` (new); `tests/fixtures/take_v3_audio.json`,
`tests/fixtures/take_v2_legacy_audio.json` (new); `CLAUDE.md` (source-tree entry + a 6-line "Audio store" note under the
recording section). Must NOT touch `test_take.cpp`, `test_audio_tap_sync.cpp`, `AudioTap.*`, `RecorderClock.*`, `Player.*`.
Steps: (1) CMake + fixtures + `Take` changes (§4.2, §4.3) as ONE commit so Lane B can build the moment it lands; (2)
`AudioStore`; (3) tests; (4) full `ctest` in `build-laneA` — every pre-existing test (306) still green, including
`test_take`'s v1/v2 fixture cases and `test_audio_tap_sync`'s existing folder test (it still passes because a legacy
`file` segment round-trips — that is deliberate and Lane B replaces it).
Tests (`tests/test_audio_store.cpp`, all headless; temp dirs under `juce::File::tempDirectory`, RAII-deleted):
- `[format] version constants are 3/3` — `REQUIRE(Take::kFormatVersion == 3); REQUIRE(Take::kMinReader == 3);` and
  `Take{}.toVar()` carries `"version": 3`. **Fails on pre-change code (2/2).**
- `[format] v1 and v2 fixtures still load` — `take_v1.json` → `stats.wasV1`, same lane counts as `test_take.cpp:138-167`;
  `take_v2_future.json` → not refused, unknown feature/section/kind still reported (mirror `test_take.cpp:172-221`).
- `[format] v3 fixture loads; resolve against an EMPTY store is Missing and names the id` — `take_v3_audio.json`
  (contents in §10.4); `legacyInFolderAudio == false`; `resolve` → `Status::Missing`, reason contains the id.
- `[format] legacy v2 in-folder audio is flagged, round-trips, and resolves as Legacy` — `take_v2_legacy_audio.json` →
  `stats.legacyInFolderAudio == true`; `toVar()` re-emits `file` and no `id`; `resolve` → `Legacy`.
- `[store] beginAsset creates the folder; finalize on a synthetic 16-bit WAV writes the sidecar last` — write a 3-second
  stereo WAV of known int16 samples with `juce::WavAudioFormat` directly (no tap); `finalize` with gaps in take-clock
  domain `{firstSample+1000, 64}` → sidecar gap `{1000, 64}`; `find()` returns frames/rate/channels equal to the header;
  `isIncomplete` false after, true if the sidecar is deleted.
- `[store] fp1 is deterministic and specified` — same file twice → same string; the expected value for a tiny known-sample
  file is computed in the test independently (build the header + PCM16LE bytes by hand from the samples written, SHA-256
  them) and REQUIRED equal — this pins the `>> 16` assumption of §3 step 3.
- `[store] resolve verdicts` — Resolved on the finalized asset; `Mismatch` after overwriting the last 1024 frames with a
  different WAV of equal length; `Mismatch` after truncating (frames differ); `Missing` after deleting the folder;
  `Incomplete` after deleting only the sidecar; `MultiSegment` with two segments; `NoAudio` for a default `AudioRef`.
- `[store] fork shares one asset` — `referencing()` into take A; `Take B = A; B.save(otherFolder)`; both load and resolve
  to the SAME `wav` path; the store lists exactly one asset; neither take folder contains `audio.wav`.
- `[store] referencing round-trips the domain` — sidecar gap frame 1000 + `firstSample` 3584 → take gap sample 4584;
  `unreliableFrom` likewise.
- `[gc] scanTakes + unreferencedAssetIds` — two takes referencing asset X, one asset Y unreferenced, one incomplete Z →
  unreferenced == {Y}, Z listed only by `listIncompleteAssetIds`; add a take folder whose `take.json` is garbage → verdict
  is nullopt and `unreadableTakes.size() == 1`.
Acceptance: all of the above green in `build-laneA`; `ctest` total = 306 + new count on a CLEAN forced rebuild that exits 0;
`git show --stat HEAD` lists every new file (`git add -f` under `.harmony/` and check — HANDOFF `:2490-2492`).

### Lane B — the live tap writes into the store (`cmake -B build-laneB`; depends_on A landed in the tree)
Owns: `tests/test_audio_tap_sync.cpp` ONLY. No `src/` changes (AudioTap needs none — it writes wherever it is pointed).
Work: REPLACE the folder test at `:343-416` with the store flow of §5.1, using the existing `FakeAudioIODevice` +
`CombinedCallback` harness (`:40-104`):
- `[audiotap][store] a live recording lands in the store, not in the take folder` — `store.beginAsset()`;
  `tap().start(store.wavFile(id))` succeeds (the folder exists → R15 check runs on a real volume); drive 10 blocks;
  `stop()`; `finalize` with `CaptureFacts` from the tap; `asset->frames == tap().framesWritten()`; take =
  `referencing(*asset, tap().firstSample())`; `take.save(takeFolder)`; REQUIRE the take folder has NO `audio.wav`;
  `Take::load` + `store.resolve` → `Resolved`, `r.wav == store.wavFile(id)`, `r.firstSample == tap().firstSample()`;
  `segments[0].id == id`, `segments[0].file.empty()`. **Fails on pre-change code**: it does not compile (no `AudioStore`)
  — and, behaviourally, the pre-change test's own assertion `folder.getChildFile("audio.wav").existsAsFile()` (`:393`) is
  the inverse of this one; the builder notes both.
- `[audiotap][store] gaps reach the sidecar in the asset domain` — reuse T1's dropout injection (the existing suite already
  has the hostTimeNs jump) so one gap of 1024 lands; after finalize the sidecar gap frame equals `gap.sample − firstSample`;
  `referencing()` restores `gap.sample`.
- `[audiotap][store] moving the asset folder makes resolve refuse` — rename the asset folder → `Missing`; move it back →
  `Resolved` again (nothing cached).
Acceptance: `test_audio_tap_sync` green in `build-laneB` incl. the T1 sync cases untouched; ThreadSanitizer run of the
concurrency case still clean (unchanged code, but the target's link list changed — prove it, don't assume it).

### Lane E — long takes and scrubbing (`cmake -B build-laneE`; independent of A/B)
Owns: `src/recording/RecorderClock.h`, `src/recording/RecorderClock.cpp`, `src/recording/Player.h`,
`src/recording/Player.cpp`, `tests/test_take.cpp`. Must NOT touch `TempoMap.*`, `Program.*`, `Take.*`, CMake.
Tests (append to `tests/test_take.cpp`, same helpers `makeSnap`/`FakeSink`/`layerKey` at `:23-73`):
- `[recorderclock][long] 40 minutes at a 0.03 BPM reporting bias stays within 0.05 beats and one block of the map` — tick at
  120 Hz for 2400 s: true phase advances `128/60/120` per tick (double accumulator, `fmod` 1.0, cast to float for the snap),
  `snap.bpm = 127.97f` constant, `deliveredSamples += 400`. After the loop: `now.beat ≈ 5120 (margin 0.5)`;
  `|tempo().beatAt(now.t) − now.beat| < 0.05` (**pre-change ≈ 1.2 → fails**); `|sampleAt(now.t) − now.sample| <= 400`
  (**pre-change returns the start sample → fails by ~115 M**); count of `why == "periodic"` anchors in [150, 170]; consecutive
  anchors' `beat` deltas ≤ 32.5; `tempo().a` sorted by `t`.
- `[recorderclock][long] no periodic anchors while unmetered` — 60 s of `bpm == 0` after a lock: anchor count grows by
  exactly the one "unmetered" edge.
- `[recorderclock] existing monotonic/resync case (:226-272) still green` — unchanged, re-run.
- `[player][seek] a backwards advanceTo re-seats and events re-fire on re-pass` — discrete at 1.0/2.0/3.0, gesture
  [1.5, 2.5] ramp 0→1. `start(0)`; `advanceTo(2.2)` → fired {1,2}, 1 touch; `advanceTo(1.2)` → 1 release, no new fire;
  `advanceTo(2.2)` → fired {1,2,2}, 2 touches; `advanceTo(3.5)` → fired size 4, 2 releases. **Pre-change: fired size stays
  3 and touches 1 → fails.**
- `[player][seek] seeking within a gesture keeps the grip and re-evaluates` — `advanceTo(2.0)` then `advanceTo(1.8)`:
  no release, last `set` value < the previous one, still exactly 1 touch.
- `[player][seek] explicit forward seek skips, advanceTo catches up` — from 1.2 `seek(2.9)` → no fire; `advanceTo(3.1)`
  → only event 3. Separately from 1.2 `advanceTo(3.1)` → events 2 and 3 (catch-up unchanged, the `:384-443` stall case
  still green).
Acceptance: `test_take` green in `build-laneE`, the two "fails on pre-change" cases mutation-checked against a stash of
the pre-change `RecorderClock.cpp`/`Player.cpp` (the s168 method, HANDOFF `:2422-2423`), source restored byte-identical.

### 10.4 Fixture contents (Lane A writes these verbatim)
`tests/fixtures/take_v3_audio.json`:
```json
{
    "format": "audiodna-take", "version": 3, "minReader": 3,
    "features": ["lanes", "tempoMap", "checkpoint0", "audio"],
    "meta": { "recordedAt": "2026-09-23T00:00:00Z", "app": "0.1.0", "duration": 10.0, "durationBeats": 21.3 },
    "audio": {
        "segments": [ { "id": "0123456789abcdef0123456789abcdef",
                        "fingerprint": "fp1:0000000000000000000000000000000000000000000000000000000000000000",
                        "firstSample": 3584, "frames": 480000, "rate": 48000, "channels": 2 } ],
        "mode": "input", "gapDetection": true, "gaps": [ { "sample": 100000, "n": 1024 } ], "unreliableFrom": null
    },
    "tempoMap": [ { "t": 0.0, "beat": 0.0, "sample": 3584, "bpm": 128.0, "why": "start" } ],
    "checkpoint0": { "activeDeckIndex": 0, "quantizeMode": 0, "bpm": 128.0, "audioAction": "", "decks": [] },
    "checkpointEnd": { "activeDeckIndex": 0, "quantizeMode": 0, "bpm": 128.0, "audioAction": "", "decks": [] },
    "markers": [],
    "lanes": [
        { "key": { "scope": "layer", "deck": { "i": 0, "rel": true, "name": "" }, "layer": { "i": 0, "id": 0, "name": "Layer 1" }, "control": "activeClip" },
          "kind": "discrete",
          "points": [ { "seq": 1, "t": 0.5, "sample": 27584, "beat": 1.07, "bpm": 128.0, "origin": "human", "v": 3 } ] }
    ]
}
```
`tests/fixtures/take_v2_legacy_audio.json`: the `take_v2_future.json` envelope (`version 2, minReader 2`, features
`["lanes","tempoMap","checkpoint0","audio"]`, no future section, no opaque lane) with
`"audio": { "segments": [ { "file": "audio.wav", "firstSample": 3584, "frames": 480000, "rate": 48000, "channels": 2, "sha1Head": "" } ], "mode": "input", "gapDetection": true, "gaps": [], "unreliableFrom": null }`.

---

## 11. Tradeoffs considered
- Store per-asset FOLDER vs flat `<id>.wav`+`<id>.json` — folder chosen: the sidecar-last "complete" flag and a future FLAC
  transcode live beside the WAV; one `deleteRecursively` per asset. Flat rejected: two files per identity, no atomic unit.
- Human-readable folder names (`2026-09-23_Friday_<id>`) vs bare id — bare id chosen: direct-path resolution, no scan; the
  sidecar carries `recordedAt` and a LATER UI lists by date/take names. Rejected: readable names need a scan per resolve
  and a rename breaks nothing but confuses "is this the same asset".
- Keep `gaps`/`unreliableFrom` only in the take vs also in the sidecar — both: a fresh take against an asset needs them
  (they describe the audio, not the knobs). Cost: a tiny list duplicated; domain conversion specified and tested.
- Hard-links / APFS clones to dedupe in-folder audio — rejected: break on exFAT/USB/other-OS copies and leave the
  "which take owns the audio" question open.
- Interpolating `TempoMap::beatAt` between anchors instead of (or in addition to) periodic anchors — rejected for now:
  with one anchor there is nothing to interpolate; anchors are required regardless, and a 32-beat segment bounds the bias
  error to 0.03/60 × 15 s ≈ 0.008 beats (a 1/4-beat replay snap, ruling 15, is 30× coarser). Interpolation stays a
  LATER refinement, not needed to ship.
- `jassert(pos >= pos_)` instead of seek semantics — rejected: ruling 28 makes backwards `pos` routine, so the assert fires
  constantly in debug and is silent in release — the exact failure class D2/D12 forbid.
- A full-content hash — see D-A2; door open via the versioned sidecar.

---

## 12. RISKS — strongest counterarguments, and what to verify before/while building
R1. **`fp1` is partial by design.** A corrupted middle of a 4-hour file is undetected. Mitigation: the store is app-managed and
    immutable after finalize; the sidecar is versioned so a `sha256` field can be added later. Verify: nothing — a stated limit.
R2. **`>> 16` PCM16 extraction is INFERRED**, not read in JUCE's int-read implementation. Lane A's hand-computed
    fingerprint test is the proof; if it fails, the builder reads `juce_AudioFormatReader.cpp` and fixes the extraction, not
    the test.
R3. **One sample domain per take is an invariant step 3 must honour.** A fresh take recorded against stored audio while the
    user scrubs will produce non-monotonic `sample` stamps unless the recorder clock is driven by the playback frame and
    scrubbing is disabled while armed (or treated as record-over, ruling 18 — unspecified). Not this spec; named so step 3
    cannot miss it.
R4. **`kMinReader = 3` refuses every v3 file for any external tool written against v2.** None exists (VERIFIED by absence:
    no real take was ever recorded, HANDOFF `:2434`). If one appears before this lands, the refusal is the correct loud answer.
R5. **Lane A lands `Take` changes before Lane B rewrites the s168 folder test.** Mitigated by keeping `file` as a legacy
    round-trip field, so `test_audio_tap_sync.cpp:403` keeps passing in between. Verify in Lane A's full ctest.
R6. **`AudioTap::start` R15 check now runs on the STORE's volume** (`AudioTap.cpp:109`) — correct, and it is why
    `beginAsset()` must create the folder first. A CI box with < 2 GB free fails Lane B's test loudly, not silently.
R7. **Fix (b) — exact stamps in `Program::compile` — is still owed** (`s168-review-lane-a.md:202-206`). Periodic anchors
    shrink the error but the compile path still reconstructs `x` from the map (`Program.cpp:174-183`). Not folded in on
    purpose ("keep it minimal"); schedule it as its own small lane (it touches `Program.cpp`, disjoint from all three here).
R8. **Forward `seek()` vs `advanceTo` catch-up is a caller contract.** If step 3 only ever calls `advanceTo`, a deliberate
    forward scrub fires every skipped event as a burst. Documented on both methods; step 3's packet must name the call site.
R9. **`juce_cryptography` link is ASSUMED to configure cleanly** on this FetchContent JUCE (the module directory exists,
    VERIFIED). Lane A's first configure proves it; the module has no external deps.
R10. **Periodic anchors and `tAt` inside metered segments** — anchors are strictly increasing in `beat` while metered, so
    `bracketByBeat` (`TempoMap.cpp:51-60`) stays unambiguous. Unmetered stretches get no periodic anchors precisely to keep
    `tAt`'s existing canonical answer (§8). Verify with the `[long] no periodic anchors while unmetered` case.
R11. **The R14 flag glue (HANDOFF item 3)** is satisfied by §5.1's `CaptureFacts.gapDetection`, but only once step 3 calls
    it. Lane B proves the value reaches the sidecar and the take headlessly; the production call is still step 3's.

Counter-argument to the whole approach, and why it loses: "a store is infrastructure; just keep `audio.wav` in the take
folder and let Save-As copy it — disks are cheap." Ruling 28's own arithmetic (`binding-decisions.md:452-455,463-464`):
five re-dos of one night = ~14 GB of identical bytes, and every re-do is the PRODUCT (ruling 28(d): editing is the point of
recording). A store is ~300 lines and one CMake module; the copy model is a format decision that gets uglier with every real
take recorded. The handoff's own instruction (`HANDOFF.md:2391-2392`): do it before anyone records anything real.

---

## 13. Questions for Boris (product calls; none block the lanes — defaults chosen and named)
1. **Deleting audio.** Should the app EVER delete stored audio — only via an explicit "Clean up unused audio" that lists
   what would go, or never (leave it to the Finder)? And when the last take referencing an audio is deleted, prompt to
   delete the audio, or leave it? Default built: never deletes; query only.
2. **Where the store lives.** `~/Documents/Audio-DNA/Audio` by default (≈ 2.8 GB per 4-hour night). Do you want to point it
   at an external/tour SSD from Preferences? Default built: fixed default path; the whole `Audio-DNA` folder is movable as one
   unit because takes reference audio by id, not path.
3. **Naming.** Assets are identified by id + date. Do you want to name an audio at record time ("Friday Berlin main set") so
   the future picker reads well, or is "date + the takes that use it" enough? Default built: sidecar has room for a name;
   nothing asks for one yet.

REPORT_FILE: /tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/7e364303-6335-4445-955e-49321e60ded4/scratchpad/s-rta-0923-ruling28-audio-store.md
STATUS: DONE
