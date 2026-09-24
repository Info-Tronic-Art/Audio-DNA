# s-rta-0923 — Ruling 28: the shared audio store (take format v3) — AMENDED SPEC v2

Author: Architect (Fable), 2026-09-23, secondary lane for RealTimeAudio. Read-only analysis; this file is the dispatch-named
scratch output (Harmony persists to `.harmony/specs/`, A54). Supersedes `s-rta-0923-ruling28-audio-store.md` (v1).
Repo @ HEAD e3504be; every lane file below is byte-identical to a50788b (`git log -- src/recording tests/… CMakeLists.txt`,
VERIFIED); `ctest -N --test-dir build` = 306 (VERIFIED this session). Sha-256 pins for mutation restores are in §10.5.
Confidence labels: VERIFIED = read on disk at the cited line this session; INFERRED = derived from cited code; ASSUMED = stated,
not checked. JUCE citations are into `build/_deps/juce-src/modules/` (module-relative paths).

## What changed from v1 (the critique's 4 BLOCKING findings, all fixed; 5-8 folded; 9-15 dispositioned in §14)

1. **Crash-readable WAV (B1).** v1's premise "the WAV is readable up to the last flush" was FALSE: `ThreadedWriter` only flushes when
   `samplesPerFlush > 0` (`juce_audio_formats/format/juce_AudioFormatWriter.cpp:304`, default `0` at `:341`, VERIFIED) and `AudioTap`
   never calls `setFlushInterval` (the whole of `src/recording/AudioTap.cpp` read; the writer is built at `:126-127`, VERIFIED). The WAV
   header is written in the writer ctor with 0 frames and again only in the dtor (`juce_audio_formats/codecs/juce_WavAudioFormat.cpp:
   1622-1629`, VERIFIED); the reader takes length from the data chunk with no file-size fallback (`:1348-1352`, VERIFIED). Fix: one
   line in `AudioTap::start` (§4.4, D-A10), a bounded-loss statement, a test that fails pre-change, and `AudioTap.*` moved into Lane R28.
2. **The reference is ALWAYS written (B2).** `finalize()` now always returns an `AudioAsset` (from the header when readable, from the
   capture facts otherwise) and `take.audio` always references the id. A fingerprint-less reference resolves as `ResolvedUnverified`
   (header-checked), never as "no audio". The take is the sidecar's backup: `CaptureFacts::fromAudioRef` makes a later repair real.
   `resolve()` step (1) no longer dereferences a missing segment.
3. **Working-tree consistency (B3).** Files are created on disk BEFORE any CMake line references them, in one commit; Lane E and the
   fixes-plan lanes configure the SAME tree in their own `-B` dirs, so a CMake reference to a missing source would break their configure.
   Step order rewritten (§10.1).
4. **`tests/CMakeLists.txt` ownership (B4).** Per Harmony's lane re-cut there is NO scaffold commit; every lane APPENDS its own target
   block(s) at the END of `tests/CMakeLists.txt` and never edits an existing block. R28 appends two targets; Lane E appends none
   (it extends `tests/test_take.cpp`). Root `CMakeLists.txt` is R28's alone.
5. Folded (N5-N8): per-tick `popGap` drain (the 64-slot FIFO holds 63 markers, §5.1); `finalize()` detects header-vs-`framesWritten`
   truncation (§4.1); a self-stopped tap (mid-take rate/channel change) is recorded as `unreliableFrom = firstSample + frames` (§5.1);
   the overdub clock domain is pinned with the exact JUCE formula (§5.2).
6. **Lane re-cut (Harmony):** Lane R28 = v1 Lane A + Lane B + AudioTap flush fix + review-fix L1 (v1 `TransportChange` bridge, same
   `Take.cpp`). Lane E = `RecorderClock` + `Player` (+ review-fix L4 Latch-refusal rider) + `tests/test_take.cpp`.

QUESTION: Boris ruled (`.harmony/binding-decisions.md:436-466`, VERIFIED) that the recorded audio is the ANCHOR and many takes share one
audio. Today `AudioTap` writes `audio.wav` wherever `AudioTap::start(file)` is pointed (`AudioTap.cpp:105-144`, VERIFIED) and the only
caller points it INSIDE a take folder (`tests/test_audio_tap_sync.cpp:357`, VERIFIED; no production caller — grep `AudioTap|AudioStore`
under `src/` finds only `src/recording/AudioTap.*` and `src/audio/CombinedCallback.h`, VERIFIED). Design the shared store, the take-side
reference, the v3 format, migration, the missing-audio policy, the GC policy; fold in review fix (c) (`RecorderClock` periodic anchor)
and HANDOFF addendum item 2 (`Player::advanceTo` backwards seek, `.harmony/HANDOFF.md:2521-2526`, VERIFIED); carry L1 and L4 from the
sibling plan into these lanes.

APPROACH (verdict first): **an app-managed store `~/Documents/Audio-DNA/Audio/<id>.adna-audio/{audio.wav, audio.json}`, keyed by a
random id minted at record-start, self-described by an `audio.json` sidecar written LAST (its presence is the "complete" flag),
fingerprinted by a cheap deterministic head+tail+length SHA-256 (`fp1`) recomputed on EVERY load (~10 ms), referenced from the take by
`{id, fingerprint, firstSample, frames, rate, channels}`; take format `version 3 / minReader 3`; the take folder holds `take.json`
ONLY.** The WAV header is re-patched every 10 s of audio so a crash mid-show loses at most ~18 s, not the file. The take ALWAYS carries
the reference, even when finalize fails; missing or mismatched audio REFUSES play-with-audio with a reason and offers wall-clock replay
only as an explicit choice; a never-fingerprinted reference plays with a warning. Nothing is deleted by code except the store's own
never-finalized, just-minted asset on a failed arm (GC is a query; deletion is a Boris question). Fix (c) = a `why:"periodic"` anchor
every 32 beats while metered. Item 2 = a backwards `pos` is a defined SEEK, plus an explicit `Player::seek()`. Two builder lanes,
disjoint files: R28 (store + format + tap + L1) and E (clock + player + L4).

---

## 0. Scope, non-goals, R8

IN: `src/recording/AudioStore.{h,cpp}` (new); `AudioRef` + `Take` v3 + `LoadStats` flags; the v1 `TransportChange` bridge (L1); the
`AudioTap` header-flush line; three fixtures; `RecorderClock` periodic anchor; `Player` seek + Latch refusal (L4); tests; CMake.
All headless (ctest), no app launch.

OUT (named so nobody "helpfully" adds them): MainComponent/RecordPanel wiring of record→store→take (spec step 3 — §5 is the CONTRACT it
must follow and §5.6 lists what it MUST do); review fixes (b) exact stamps in `Program::compile` (fixes-plan L2), (d) RecordPanel
(L5), addendum 4a double-touch (L3), addendum 5 EnvelopeSignal siblings (L6) — all still owed, in the sibling plan's lanes, not here;
multi-segment audio (`audio-2.wav` on a mid-take rate change — NOT built; the end of audio is recorded, §5.1, and `size() > 1` is
refused at resolve); any deletion beyond §7's one narrow exception; a store-location preference; importing external WAVs; a
full-content hash (door open, §3); a periodic `Take::save` checkpoint (step 3 MUST, §5.6, finding 13).

R8: nothing here defines or touches `manualWrite`, `AutomationCurve`, or the connection engine. No file in either lane lives under
`src/connect/` (VERIFIED against the file lists in §10).

---

## 1. Decisions (each with the reason and the strongest counter)

**D-A1 — Identity is a random id, not a path and not a hash.** `id = juce::Uuid().toString()` — 32 lowercase hex chars
(`juce_core/misc/juce_Uuid.h:86-88` + `juce_Uuid.cpp:96,99` → `String::toHexString`, digits `0123456789abcdef` at
`juce_core/text/juce_String.cpp:1929`, VERIFIED). Entropy is ≤ 48 bits, not 128: `Uuid()` draws from a fresh `juce::Random`
(`juce_Uuid.cpp:38-40`) seeded from time/ticks/`this` (`juce_core/maths/juce_Random.cpp:42-45,66-76`) and stepped by a 48-bit LCG
(`:88`) — VERIFIED. Adequate for a per-user store of hundreds of assets; `beginAsset()` re-mints if the folder already exists
(bounded, 3 tries). Folder `<root>/<id>.adna-audio/`. A take references the id; the store root is re-derived at runtime, so moving the
whole `Audio-DNA` folder keeps every reference valid. Counter: "reference by relative path" — a path is what breaks when the user
reorganises; loses. Counter: "id = content hash" — the id must exist at record-START, the content does not exist until STOP; loses.

**D-A2 — Content check is `fp1`, a cheap deterministic fingerprint, verified at every load.** Definition in §3. Cost ≈ 2 MiB read +
one SHA-256 ≈ 10 ms, so it runs synchronously at finalize AND at every resolve. Counter (strongest): "Boris said content hash; hash the
whole file." JUCE's `SHA256` is one-shot, non-incremental (`juce_cryptography/hashing/juce_SHA256.h:64-102` — no `update()`,
VERIFIED); a 2.76 GB night is ~10-30 s (ASSUMED throughput) which cannot run at every load on the message thread. `fp1` catches every
realistic failure (wrong/moved/replaced/truncated/length-edited file); the sidecar is versioned so a `sha256` field can be ADDED later.
Loses on cost/benefit now; honestly named `fp1`.

**D-A3 — The sidecar is written last, atomically; its presence means "complete"; an incomplete asset is repairable FROM ITS TAKE.**
`File::replaceWithText` goes through `TemporaryFile` + rename (`juce_core/files/juce_File.cpp:798-802`, VERIFIED). A crash mid-show
leaves `audio.wav` + no sidecar = INCOMPLETE. With D-A10's header flush the WAV is readable up to the last flush; the take written by
§5.1 carries `{id, frames, rate, channels, firstSample, gaps, unreliableFrom}`, so `finalize(id, CaptureFacts::fromAudioRef(take.audio))`
rebuilds the sidecar later. That is why incomplete assets are NEVER auto-deleted (§7). (v1's claim that the WAV was already readable
after a crash was false — see "What changed" 1.)

**D-A4 — The take keeps the v2 `audio` SHAPE and ADDS `id` + `fingerprint` per segment; `file` becomes read-only legacy; `sha1Head`
is dropped** (never written by shipped code — only `Take.h:27` / `Take.cpp:15,29` mention it, VERIFIED). D12 "ADD, never REDEFINE"
(`.harmony/specs/s167-performance-log-and-routines.md:670`, VERIFIED). `segments[]` stays an array; `resolve()` refuses `size() > 1`.

**D-A5 — `version 3 / minReader 3`, constant.** A store-referenced take must not be half-read by a v2 reader (it would find
`file == ""`, open nothing, and degrade to "no audio" silently — what D12 forbids). No v2 reader exists outside this repo's history and
no real take was ever recorded (`HANDOFF.md:2434`, VERIFIED); the bump costs nothing. The refusal at `Take.cpp:192-200` keeps working:
v2 files (minReader 2) load; the v1 bridge (`:189-190`, `:299-402`) is touched only by L1 (§4.2).

**D-A6 — `Take::load` stays pure JSON; resolution against the store is a separate call.** `Take` is a value type used by
filesystem-free tests (`Take.h:100-104`, VERIFIED). `AudioStore::resolve(take.audio)` does the disk checks; the loader only FLAGS.

**D-A7 — Missing/mismatched audio: refuse play-with-audio, loudly; wall-clock replay only as an explicit choice.** D11 #1 sets this
principle for render (`s167 spec:639-641`, VERIFIED). One graded exception, D-A12.

**D-A8 — GC: never delete automatically; this step ships the QUERY only** (one narrow exception, D-A13). An unreadable `take.json`
under the scanned roots voids the orphan verdict. The verdict is relative to the roots scanned and says so (finding 9).

**D-A9 — Sample domains.** Inside ONE take every `sample` stamp, `audio.gaps[].sample` and `unreliableFrom` is in the take's clock
domain (the delivered-sample counter, `src/audio/CombinedCallback.h:105,134`, VERIFIED). `firstSample` is the take-clock value at
asset frame 0, so `assetFrame = sample − firstSample` (`AudioTap.cpp:178` + T1's `w + firstSample == exactSample`,
`test_audio_tap_sync.cpp:274`, VERIFIED). The SIDECAR stores gaps/unreliableFrom in the ASSET-FRAME domain; `finalize()` converts in,
`referencing()` converts out (integer-exact). A fresh take against stored audio gets a clock whose `sample` IS the asset frame
(`firstSample = 0`) — §5.2 pins the formula step 3 must use.

**D-A10 [new] — The tap patches the WAV header every 10 s of audio.** `threadedWriter_->setFlushInterval(int(rate_ * 10.0))` right
after `AudioTap.cpp:127`. The flush runs on the writer's `TimeSliceThread` inside `writePendingData` (`juce_AudioFormatWriter.cpp:
304-312`), NOT the audio thread; `WavAudioFormatWriter::flush()` seeks back, rewrites the fixed-size header, seeks forward
(`juce_WavAudioFormat.cpp:1667-1679`; fixed size because of the JUNK pad chunk, `:1729-1745`), and `FileOutputStream::setPosition`
flushes its buffer before each seek (`juce_core/files/juce_FileOutputStream.cpp`, `setPosition` → `flushBuffer()`, VERIFIED) — so
after a flush the header on disk counts exactly the frames on disk. The counter starts at 0, so the FIRST drain also flushes
(`:307-311`: `0 − n ≤ 0`). Bounded loss on a PROCESS crash: ≤ 10 s of audio not yet counted + whatever sat in the 8 s writer FIFO
(typically < 2 s, the drain chunk `fifo.getTotalSize()/4`, `:275`) ≈ 12 s typical, 18 s worst. Kernel panic / power loss (OS cache
not fsync'd) is out of scope. 16-bit frames are even-sized, so `writeHeader`'s odd-byte pad (`:1689-1690`) never fires. Counter:
"flush every second" — 10× the seeks for a 9 s better bound nobody asked for; loses. Counter: "do it in step 3" — the tap is the only
place that owns the writer, and a take that loses its whole audio to a crash is exactly ruling 28's failure; loses.

**D-A11 [new] — The take ALWAYS carries the reference.** `finalize()` never returns "nothing": when the WAV is unreadable it returns an
asset built from the `CaptureFacts` (fingerprint `""`, `frames = framesWritten`) and says so; when only the sidecar write fails, the
asset is complete (fp1 computed) and the store is merely Incomplete until repaired. §5.1 references the asset in every branch. Counter
(v1's text): "blank `take.audio` and repair later" — that destroys the only link between the take and its audio; the facts die with the
`AudioTap`; loses outright.

**D-A12 [new] — A reference with an empty fingerprint resolves as `ResolvedUnverified`.** Header (frames/rate/channels/bits) and
sidecar are still checked; only content identity is unverified. Playback is allowed WITH the reason shown (a warning, not a refusal):
the file is app-managed, the length matches what the tap wrote, and the alternative (refuse) makes D-A11's repair story useless.
Counter: "refuse — D-A7" — D-A7 is about the audio being wrong or gone; here it is present and length-verified; loses.

**D-A13 [new] — One narrow delete: `abandonAsset(id)` on a failed arm.** Preconditions: `id == activeAssetId_` (minted by THIS
instance, never finalized) AND no sidecar exists. `AudioTap::start` can leave a 0-byte `audio.wav` when `createWriterFor` fails after
the stream opened (`AudioTap.cpp:115-121`, VERIFIED), and creates the parent itself (`:112-113`; `File::createDirectory` is recursive,
`juce_File.cpp:531-546`, VERIFIED), so a failed arm always leaves a folder. `deleteRecursively`. A new process has an empty
`activeAssetId_`, so a crashed show's asset can never be abandoned — it is Incomplete, repairable (D-A3).

---

## 2. On-disk layout

```
~/Documents/Audio-DNA/                       (convention: MainComponent.cpp:1954 settings, Renderer.cpp:1962 Snapshots — VERIFIED)
├── Audio/                                   AudioStore::defaultRoot()   ← the shared store (this spec)
│   └── <id>.adna-audio/
│       ├── audio.wav                        written by AudioTap during the show; header re-patched every 10 s (D-A10)
│       └── audio.json                       sidecar, written LAST by AudioStore::finalize; absent ⇒ incomplete (repairable from any take)
└── Takes/                                   default home for Name.adna-take/ (step 3/4 picks names; not this spec)
    └── Friday-take-3.adna-take/
        └── take.json                        ONLY. No audio.wav, ever again.
```

### 2.1 Sidecar `audio.json` (sidecar format v1)
```jsonc
{ "format": "audiodna-audio", "version": 1,
  "id": "3f2a9c0e1b7d4e5f8a6b2c1d0e9f8a7b",           // == folder stem; find() refuses a mismatch
  "fingerprint": "fp1:<64 lowercase hex>",             // §3; NEVER empty in a sidecar (finalize computes it or does not write)
  "frames": 173145600, "rate": 48000, "channels": 2, "bits": 16,
  "recordedAt": "2026-09-23T21:14:02.000+02:00",       // juce::Time::getCurrentTime().toISO8601(true) — carries the LOCAL offset
                                                       // (juce_core/time/juce_Time.cpp:427-437, VERIFIED); not converted to Z
  "app": "0.1.0",
  "mode": "input",                                     // how the AUDIO was captured ("input" | "file", D10.1) — not the re-do session's mode
  "gapDetection": true,                                // R14 flag — ASSET-level fact
  "gaps": [ { "frame": 9826816, "n": 1024 } ],         // ASSET-FRAME domain (frame = sample − firstSample)
  "unreliableFrom": null }                             // asset-frame domain or null; MAY equal "frames" (= the capture ended before the
                                                       // performance did, §5.1) or be < frames (a FIFO overrun or a truncated file, §4.1)
```

### 2.2 Take `take.json` v3 — only the envelope and `audio` change
```jsonc
{ "format": "audiodna-take", "version": 3, "minReader": 3,
  "features": ["lanes", "tempoMap", "checkpoint0", "audio"],          // unchanged list (Take.cpp:146-153); the version says the rest
  "audio": {
    "segments": [ { "id": "3f2a9c0e1b7d4e5f8a6b2c1d0e9f8a7b", "fingerprint": "fp1:…",      // fingerprint "" ⇒ ResolvedUnverified (D-A12)
                    "firstSample": 3584, "frames": 173145600, "rate": 48000, "channels": 2 } ],
    "mode": "input", "gapDetection": true,
    "gaps": [ { "sample": 9830400, "n": 1024 } ],                     // TAKE-clock domain, as today
    "unreliableFrom": null },
  … everything else exactly as v2 (Take.cpp:139-176) … }
```
Legacy v2 segment as read from an old file: `{ "file": "audio.wav", "firstSample": …, "frames": …, "rate": …, "channels": …,
"sha1Head": "" }` → `Segment.id == ""`, `Segment.file == "audio.wav"`, `sha1Head` ignored; re-emitted with `file` (and no `id`) so a
legacy take round-trips; flagged by the loader (`LoadStats.legacyInFolderAudio`); refused by `resolve()` as `Legacy`.

---

## 3. `fp1` — the fingerprint, exactly

Inputs: the WAV opened with `juce::WavAudioFormat::createReaderFor(new juce::FileInputStream(wav), true)`; `N = reader->lengthInSamples`,
`R = roundToInt(reader->sampleRate)`, `C = reader->numChannels`, `B = reader->bitsPerSample` (`juce_audio_formats/format/
juce_AudioFormatReader.h:234-243`, VERIFIED). `W = 262144` frames. **Defined for integer-PCM WAVs only**: `reader->usesFloatingPointData`
(`:246`) → `fingerprint()` returns nullopt (the int read of a float file is reinterpreted bits; the tap never writes float,
`AudioTap.cpp:119` writes 16-bit, VERIFIED).

1. `header = "adna-fp1|frames=" + N + "|rate=" + R + "|channels=" + C + "|bits=" + B + "|"` (ASCII, no trailing newline).
2. Windows: if `N <= 2W` → one window `[0, N)`; else two windows `[0, W)` and `[N−W, N)`.
3. For each window, read via the INT overload `reader->read(int* const* dest, C, start, count, false)` (`:139-143`, VERIFIED) into `C`
   int buffers; emit frames interleaved, channel `0..C−1`, each sample as PCM16 little-endian `int16 v = (int16_t)(sample >> 16)`.
   **VERIFIED, no longer inferred:** the 16-bit WAV read path is `ReadHelper<AudioData::Int32, AudioData::Int16, LittleEndian>`
   (`juce_WavAudioFormat.cpp:1531`) and `AudioData::Int16::getAsInt32LE` is `(uint16 stored) << 16`
   (`juce_audio_basics/buffers/juce_AudioDataConverters.h:171`); the write path `WriteHelper<Int16, Int32, LE>` (`:1646`) stores
   `(uint16)(value >> 16)` (`juce_AudioDataConverters.h:173`). So `>> 16` recovers the stored int16 exactly, and a test that writes
   `v << 16` through `AudioFormatWriter::write(const int**)` (`juce_AudioFormatWriter.h:125`) stores exactly `v`. A 24-bit file
   fingerprints on its top 16 bits (documented).
4. `fingerprint = "fp1:" + juce::SHA256(bytes.getData(), bytes.getSize()).toHexString()` (`juce_SHA256.h:74,102`, VERIFIED).
   `bytes` is one `juce::MemoryBlock` ≤ header + 2·W·C·2 bytes (≈ 2 MiB stereo) — bounded, no 2.8 GB read.
5. `N == 0` is allowed (header only).

Requires `juce::juce_cryptography` (depends only on `juce_core`, `juce_cryptography/juce_cryptography.h:54`, VERIFIED) — CMake is R28's.

---

## 4. Code surface

### 4.1 `src/recording/AudioStore.h` (new, Lane R28) — signatures are the contract
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
    std::string fingerprint;        // "fp1:" + 64 hex; "" ONLY in a finalize() result whose wav was unreadable (never in a sidecar)
    uint64_t frames = 0;
    double rate = 0.0;
    int channels = 0;
    int bits = 16;
    std::string recordedAt;         // Time::toISO8601(true) -- local offset, see spec 2.1
    std::string app;
    std::string mode;               // how the AUDIO was captured: "input" | "file"
    bool gapDetection = false;
    std::vector<std::pair<uint64_t, uint32_t>> gaps;   // ASSET-FRAME domain
    std::optional<uint64_t> unreliableFrom;            // asset-frame domain; may == frames (capture ended early)

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

    // --- recording a new asset (sequence in spec 5.1) ---
    // Mints an id (re-minting up to 3 times if the folder already exists)
    // and CREATES the folder so the asset exists as a unit from the first
    // instant (isIncomplete/listIncompleteAssetIds semantics; the R15 free-
    // space check in AudioTap::start runs on THIS volume). Records it as the
    // active asset (see activeAssetId/abandonAsset). nullopt if the root or
    // the folder cannot be created.
    std::optional<std::string> beginAsset();
    std::optional<std::string> activeAssetId() const;        // set by beginAsset(), cleared by finalize()/abandonAsset()

    // The ONE delete in this class (spec 7, D-A13): removes the folder of
    // the asset THIS instance minted and never finalized (a failed arm --
    // AudioTap::start may have left a 0-byte audio.wav). Refuses (false,
    // nothing touched) unless id == activeAssetId() AND no sidecar exists.
    bool abandonAsset(const std::string& id);

    struct CaptureFacts
    {
        std::string mode;                                    // "input" | "file"
        bool gapDetection = false;                           // AudioTap::gapDetectionSupported()
        uint64_t firstSample = 0;                            // AudioTap::firstSample()
        uint64_t framesWritten = 0;                          // AudioTap::framesWritten() -- truncation check + stub when the wav is unreadable
        double rate = 0.0;                                   // device rate/channels at AudioTap::start() time (used ONLY when the wav is unreadable)
        int channels = 0;
        std::vector<std::pair<uint64_t, uint32_t>> gapsInTakeClock;   // drained via AudioTap::popGap EVERY TICK (spec 5.1), take-clock domain
        std::optional<uint64_t> unreliableFromInTakeClock;   // AudioTap::unreliableFrom(), or firstSample+framesWritten if the tap self-stopped (spec 5.1)
        std::string app;
        // Repair path (D-A3/D-A11): the take is the sidecar's backup. Requires
        // ref.segments.size() == 1; copies mode/gapDetection/gaps/unreliableFrom
        // and seg.{firstSample, frames->framesWritten, rate, channels}.
        static std::optional<CaptureFacts> fromAudioRef(const AudioRef& ref, std::string app);
    };

    struct FinalizeResult
    {
        AudioAsset asset;            // ALWAYS filled (D-A11): from the WAV header when readable, else from `facts` with fingerprint ""
        bool wavReadable = false;
        bool sidecarWritten = false;
        std::string error;           // "" on full success; otherwise names the id and what failed (unreadable wav / sidecar write / truncation)
    };
    // Reads the finished WAV header, computes fp1, converts gaps/unreliableFrom
    // to the asset-frame domain (saturating: frame = sample - firstSample, 0
    // if sample < firstSample), then -- if the header's lengthInSamples <
    // facts.framesWritten (AudioTap::stopInternal's "give up rather than spin"
    // branches, AudioTap.cpp:453,467, leave frames unwritten without setting
    // unreliableFrom) -- sets asset.unreliableFrom = min(existing,
    // lengthInSamples) and notes it in `error`. Writes the sidecar LAST via
    // File::replaceWithText. Clears the active id. NEVER writes a sidecar
    // with an empty fingerprint.
    FinalizeResult finalize(const std::string& id, const CaptureFacts& facts);

    // --- lookup ---
    std::optional<AudioAsset> find(const std::string& id) const;   // "complete" == sidecar parses AND sidecar.id == id (== folder stem)
    bool isIncomplete(const std::string& id) const;                 // audio.wav exists, audio.json does not, and id != activeAssetId()
    std::vector<std::string> listAssetIds() const;                  // complete only (definition above)
    std::vector<std::string> listIncompleteAssetIds() const;        // excludes activeAssetId() (the one being recorded right now)

    // --- resolution for a take (never silent) ---
    enum class Status { NoAudio, Resolved, ResolvedUnverified, Missing, Incomplete, Mismatch, Legacy, MultiSegment };
    struct Resolution
    {
        Status status = Status::NoAudio;
        std::string reason;          // human-readable, names the id, the root searched, and what differed; "" for NoAudio/Resolved
        juce::File wav;              // valid when Resolved or ResolvedUnverified
        AudioAsset asset;            // valid when Resolved or ResolvedUnverified
        uint64_t firstSample = 0;    // the take's segment firstSample (assetFrame = sample - firstSample)
    };
    Resolution resolve(const AudioRef& ref) const;

    // --- referencing (a fresh take against an existing asset; ruling 16/28) ---
    // firstSample: the take-clock value at asset frame 0 (0 when the take's
    // clock IS the asset frame counter -- the overdub case, spec 5.2).
    static AudioRef referencing(const AudioAsset& asset, uint64_t firstSample);

    // --- fp1 (spec 3) ---
    static constexpr uint32_t kFingerprintWindowFrames = 262144;
    static std::optional<std::string> fingerprint(const juce::File& wav);   // "fp1:<hex>"; nullopt if unreadable or floating-point

    // --- GC: QUERY ONLY (spec 7). ---
    struct ReferenceScan
    {
        std::vector<juce::File> roots;              // what was scanned -- every verdict is relative to these (finding 9)
        std::vector<std::string> referencedIds;     // union over every readable take under `roots`
        std::vector<juce::File> unreadableTakes;    // take.json missing/unparseable -- if non-empty, no orphan verdict is valid
    };
    static ReferenceScan scanTakes(const juce::File& takesRoot);   // recursive "*.adna-take" search; parses take.json, reads ONLY "audio.segments[].id"
    // Complete assets referenced by no readable take under takesRoot. nullopt (refuses a verdict) if scan.unreadableTakes is non-empty.
    // Any text derived from this MUST read "not referenced by any take under <roots>".
    std::optional<std::vector<std::string>> unreferencedAssetIds(const juce::File& takesRoot) const;

private:
    juce::File root_;
    std::optional<std::string> activeAssetId_;
};
```
`resolve()` order (each step returns on failure with `reason`):
(1) `segments.empty()`: `mode.empty()` → `NoAudio`; else `Mismatch` ("audio mode '<mode>' but no segment — malformed take").
(2) `segments.size() > 1` → `MultiSegment` ("multi-segment audio is not built; re-record").
(3) `seg.id.empty()`: `!seg.file.empty()` → `Legacy` ("recorded in-folder by a pre-v3 build; re-record"); else `Mismatch` ("no id").
(4) folder or wav absent → `Missing` (names id + root); wav present, no sidecar → `Incomplete` ("repair from this take: finalize(id,
    CaptureFacts::fromAudioRef)"); sidecar unparseable or `sidecar.id != id` → `Mismatch`.
(5) `sidecar.{frames,rate,channels}` ≠ `seg.{…}` → `Mismatch` listing both.
(6) WAV header `{lengthInSamples, rate, channels, bits}` ≠ sidecar → `Mismatch`.
(7) `seg.fingerprint.empty()` → `ResolvedUnverified` (reason: "this take was saved before its audio was fingerprinted; the store's is
    `fp1:…`; verified by length/rate/channels only"). Else `fingerprint(wav)` ≠ `seg.fingerprint` → `Mismatch` ("content differs from
    what this take was recorded against"); ≠ `sidecar.fingerprint` → `Mismatch` ("the store's sidecar disagrees with its own file").
(8) `Resolved`.

### 4.2 `src/recording/Take.h` / `Take.cpp` (Lane R28)
v3 format:
- `AudioRef::Segment`: ADD `std::string id;` and `std::string fingerprint;`; KEEP `file` (comment: legacy v2 in-folder path, read-only,
  never set by v3 code); REMOVE `sha1Head`.
- `Segment::toVar` (`Take.cpp:7-17`): emit `id` and `fingerprint` (even when `""`) when `id` is non-empty; emit `file` only when
  non-empty; never emit `sha1Head`. `Segment::fromVar` (`:19-32`): read `id`, `fingerprint`, `file` (missing → `""`).
- `Take::kFormatVersion = 3; kMinReader = 3;` (`Take.h:83-84`).
- `LoadStats`: ADD `bool legacyInFolderAudio = false;` set in `Take::fromVar` after `AudioRef::fromVar` when any segment has
  `id.empty() && !file.empty()`.
- `knownTopLevelSections`/`knownFeatures` (`Take.cpp:121-136`): unchanged.

L1 — review fix (a), v1 `TransportChange` bridge (carried verbatim from the sibling plan L1, now R28's because it is the same file):
- `Take.h` `LoadStats`: add `std::map<std::string, int> v1Dropped;` — "v1 events with no v2 equivalent, counted and named (D12: never
  silently dropped); key `<EventType>:<detail>`". Update the private `fromV1Var` comment (`Take.h:117-122`): TransportChange
  play/pause/stop faithful, speed/reverse dropped+counted.
- `Take.cpp` `fromV1Var`: move the seq mint below the switch (`p.s.seq = take.nextSeq++;` after the switch, `p.s = { 0, t, 0 }` before
  it, `:323-327`) so a dropped event mints no seq. Replace the `TransportChange` case (`:374-379`, VERIFIED it scales `value*1000` into
  `p.v` under `Scope::Comp`/`"audio"`) with:
  ```cpp
  case V1Type::TransportChange:
  {
      // Faithful (D12 rule 5): v2's `audio` control is action-valued play/pause/stop
      // (PerfState::audioAction, PerfState.h:74; spec D3) and those carry no value --
      // the legacy value*1000 scaling produced an int the v2 dispatcher would misread.
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

### 4.3 CMake (Lane R28 owns both files; the ORDER of edits is part of the contract — see §10.1)
- Root `CMakeLists.txt`: add `src/recording/AudioStore.h` + `src/recording/AudioStore.cpp` after line 245 (the AudioTap pair; list
  VERIFIED at `:229-247`); add `juce::juce_cryptography` to the app's module list (`:528-538`, VERIFIED).
- `tests/CMakeLists.txt`: APPEND, after the final `endif()` of the syphon-check block (line 673 — the current end of file, VERIFIED),
  two blocks with unique header comments. Never edit an existing block (in particular NOT `test_audio_tap_sync` at `:325-358` and NOT
  `test_take` at `:274-311`):
  (i) `test_audio_store`: sources `test_audio_store.cpp`, `${SRC_DIR}/recording/AudioStore.cpp`, `${SRC_DIR}/recording/AudioTap.cpp`,
      `${SRC_DIR}/recording/Take.cpp`, `${SRC_DIR}/recording/TempoMap.cpp`, `${SRC_DIR}/recording/PerfState.cpp` (the same
      Take-without-model list `test_audio_tap_sync` links at `:328-331`, VERIFIED to link); links `Catch2::Catch2WithMain juce::juce_core
      juce::juce_events juce::juce_audio_basics juce::juce_audio_devices juce::juce_audio_formats juce::juce_cryptography`; compile
      definitions as `:343-349` MINUS `AUDIODNA_AUDIOTAP_TEST_HOOKS` (not needed) PLUS `TEST_FIXTURES_DIR` as `:301`; the same
      `-Wno-*` options; `apply_sanitizers`; `catch_discover_tests`.
  (ii) `test_take_v1_transport`: sources `test_take_v1_transport.cpp`, `${SRC_DIR}/model/Clip.cpp`, `${SRC_DIR}/model/Layer.cpp`,
      `${SRC_DIR}/connect/ConnSerialization.cpp`, `${SRC_DIR}/effects/EffectLibrary.cpp`, `${SRC_DIR}/effects/Effect.cpp`,
      `${SRC_DIR}/recording/TempoMap.cpp`, `${SRC_DIR}/recording/PerfState.cpp`, `${SRC_DIR}/recording/Take.cpp`,
      `${SRC_DIR}/recording/Program.cpp` (test_take's list at `:274-288` minus Player/RecorderClock/PerformanceRecorder — the compile
      test needs `Program::compile` + `Composition`); links/definitions/options exactly as `test_take` (`:290-309`).
- Lane E appends NOTHING (its tests live in `tests/test_take.cpp`, an existing target). Fixes-plan lanes that need targets append their
  own blocks at the end, after R28's; Harmony serializes COMMITS of `tests/CMakeLists.txt` (R28 first); before committing, the R28
  builder proves `git diff tests/CMakeLists.txt` contains ONLY its two appended blocks.

### 4.4 `src/recording/AudioTap.h` / `AudioTap.cpp` (Lane R28) — D-A10
- `AudioTap.h`, public constants next to `kMinFreeBytes` (`:132`): `static constexpr double kHeaderFlushSeconds = 10.0;` with a 4-line
  comment (crash-readable WAV; runs on the writer's TimeSliceThread, never the audio thread; bounded loss ≈ 12 s typical / 18 s worst).
- `AudioTap.cpp`, immediately after line 127 (`threadedWriter_ = std::make_unique<…>(writer, flushThread_, …)`):
  `threadedWriter_->setFlushInterval(static_cast<int>(rate_ * kHeaderFlushSeconds));` (`juce_AudioFormatWriter.h:247`, public, VERIFIED).
- Nothing else in the tap changes. `push()`'s RT path is untouched (the flush is inside `Buffer::writePendingData`, which only the
  TimeSliceThread calls, `juce_AudioFormatWriter.cpp:268-270`, VERIFIED). `kMinFreeBytes` stays 2 GB (Boris Q2, §13).

### 4.5 `RecorderClock` (Lane E) — §8. `Player` (Lane E) — §9.

---

## 5. Sequences (the contract step 3 wires; R28 proves 5.1 and 5.4 headlessly)

### 5.1 Live recording (audio switch ON — ruling 19, `binding-decisions.md:353-355`, VERIFIED)
```
AudioStore store(AudioStore::defaultRoot());
auto id = store.beginAsset();                                  // folder exists now; store.activeAssetId() == id
if (!id)                        → refuse to arm: "cannot create <root>"; no take starts
if (!tap.start(store.wavFile(*id)))                            // R15 (<2 GB on the STORE's volume, AudioTap.cpp:109) or writer failure
                                → refuse to arm, surface it, store.abandonAsset(*id)  (D-A13 -- deletes the folder incl. any 0-byte wav)
recorder.start(comp, clock, takeFolder);
take.save(takeFolder) PROVISIONALLY with audio = referencing(stub{id, "", 0, rate, ch}, 0)   // step 3 MUST (5.6): the id is on disk from arm time
… show …  on EVERY 120 Hz tick: while (tap.popGap(g)) facts.gapsInTakeClock.push_back(g);   // NOT only at stop: gapFifo_{64} holds 63
                                                                                              // markers (juce_AbstractFifo.cpp:74 clamps to
                                                                                              // freeSpace-1) and DROPS the 64th+ silently
                                                                                              // (AudioTap.cpp:386-388, VERIFIED)
const bool endedEarly = tap.wasStarted && !tap.isRunning();    // AudioTap::prepare self-stops on a mid-take rate/channel change
                                                               // (AudioTap.cpp:54-65, VERIFIED) and nothing restarts it; stop() is then a no-op
tap.stop();                                                    // flushes + closes; header patched (AudioTap.cpp:415-475)
facts = { mode, tap.gapDetectionSupported(), tap.firstSample(), tap.framesWritten(), rateAtStart, channelsAtStart,
          gapsDrainedPerTick, tap.unreliableFrom(), appVersion };
if (endedEarly) facts.unreliableFromInTakeClock = min(facts.unreliableFromInTakeClock, tap.firstSample() + tap.framesWritten());
auto fin = store.finalize(*id, facts);                         // sidecar written LAST; fin.asset ALWAYS usable (D-A11)
Take take = recorder.stop(comp);
take.audio = AudioStore::referencing(fin.asset, tap.firstSample());   // EVERY branch -- the reference is never blanked
if (!fin.error.empty()) surface fin.error                      // "wav unreadable" (fingerprint "" -> ResolvedUnverified later) /
                                                               // "sidecar not written" (Incomplete; repair = finalize from this take) /
                                                               // "truncated: header N < framesWritten M" (unreliableFrom set)
take.save(takeFolder);                                         // take.json only
```
Invariant R28 asserts: `fin.asset.frames == tap.framesWritten()` for a clean stop. When they differ, `fin.asset.unreliableFrom` is set
(finalize's truncation rule) — this replaces v1's incorrect "unreliableFrom says why" (the `:453` give-up never set it, VERIFIED).
`firstSample`, `framesWritten`, `gapDetectionSupported`, `unreliableFrom` all remain readable after a self-stop (`stopInternal` resets
none of them; only `start()` does, `:130-137`, VERIFIED).

### 5.2 A FRESH take against existing audio (the re-do)
```
auto asset = store.find(id);   if (!asset) → refuse with reason
take.audio = AudioStore::referencing(*asset, /*firstSample*/ 0);
```
`firstSample = 0` because the recorder clock MUST be fed the ASSET FRAME as `deliveredSamples`. The app plays files through
`AudioEngine::transportSource_` with the reader's rate passed as `sourceSampleRateToCorrectFor` (`src/audio/AudioEngine.cpp:48-50`,
VERIFIED), so `AudioTransportSource::getNextReadPosition()` returns DEVICE-rate frames — `positionableSource->getNextReadPosition()
× (sampleRate / sourceSampleRate)` under `ScopedLock (callbackLock)` (`juce_audio_devices/sources/juce_AudioTransportSource.cpp:
189-198`, VERIFIED). Therefore, on the MESSAGE THREAD only (the 120 Hz tick, G12; never the audio thread):
`assetFrame = llround(double(transport.getNextReadPosition()) * asset.rate / deviceRate)` — exact when the rates are equal, ±1 frame
otherwise (JUCE truncates its own ratio). With a 44.1 kHz asset on a 48 kHz device the naive counter would be 8.8 % off (finding 8).
`readerSource_->getNextReadPosition()` is NOT the play position (the 32768-frame `BufferingAudioSource` reads ahead, `:73-81`).
Scrubbing while armed makes `sample` non-monotonic — R3 (§12) stands: step 3 disables scrubbing while armed or treats it as
record-over (ruling 18), and says which. `mode` copied by `referencing()` describes how the AUDIO was captured, not this session.

### 5.3 Fork / "save as a new file connected to the same audio" (ruling 16, `binding-decisions.md:326-333`, VERIFIED)
`Take copy = original; copy.meta.recordedAt = now; copy.save(newFolder);` — the reference is a value; no bytes of audio move.

### 5.4 Load + play
```
auto take = Take::load(folder, stats);      if (stats.legacyInFolderAudio) surface it
auto r = store.resolve(take->audio);
NoAudio            → replay on DriveClock::Wall as today
Resolved           → play with audio (DriveClock::Sample; pos = assetFrame + r.firstSample, assetFrame per 5.2)
ResolvedUnverified → play with audio AND show r.reason (warning, D-A12)
anything else      → REFUSE play-with-audio; show r.reason; offer "Play without audio (wall clock)" as an explicit action;
                     for Incomplete also offer "Repair audio" = store.finalize(id, *CaptureFacts::fromAudioRef(take->audio, app))
```

### 5.5 Audio switch OFF
Skip 5.1's store/tap lines entirely; `take.audio` stays default (mode "", no segments) — unchanged from today.

### 5.6 Step 3 MUST (named here so the wiring packet cannot miss them; none is built in this spec)
1. Write a provisional `take.json` at ARM time carrying the asset id (5.1) and checkpoint `Take::save` periodically (every ~60 s;
   `replaceWithText` is atomic). Today `PerformanceRecorder::start` stores the folder and never writes
   (`src/recording/PerformanceRecorder.cpp:12-22`, VERIFIED); a crash mid-show would lose the whole take, not just the audio facts
   (finding 13). Ruling 24: the log is the product.
2. Drain `popGap` every tick (5.1). 3. Read `isRunning()` before `stop()` (5.1). 4. Feed the clock the asset frame per 5.2 and pin
   the scrub-while-armed rule (R3). 5. Call `advanceTo` vs `seek` per §9's caller contract (R8).

---

## 6. Missing / moved audio — refuse loudly, degrade only by explicit choice
| Status | Meaning | Play with audio? |
|---|---|---|
| `NoAudio` | switch was off | n/a — wall clock, not an error |
| `Resolved` | id + sidecar + header + fp1 all agree | yes |
| `ResolvedUnverified` | as Resolved but the take never got a fingerprint (finalize saw an unreadable wav) | yes, with the reason shown |
| `Missing` | folder or wav gone (names id + root) | refuse |
| `Incomplete` | wav but no sidecar (a crashed show, or the sidecar write failed) | refuse; offer Repair (5.4) |
| `Mismatch` | id/frames/rate/channels/bits/fp1 differ, or a malformed reference | refuse |
| `Legacy` | pre-v3 in-folder audio | refuse; re-record |
| `MultiSegment` | not built | refuse |
No status ever degrades to wall clock on its own. Rationale D-A7/D-A12.

---

## 7. Garbage collection policy
1. **Never auto-delete** orphans, incomplete assets (D-A3: a crashed show's audio; the repair is `finalize` from any take that
   references it), or anything on take deletion. The single exception is `abandonAsset` (D-A13): this instance's just-minted,
   never-finalized asset after a failed arm — nothing was ever recorded into it.
2. This step ships `scanTakes` + `unreferencedAssetIds` only. Any unreadable `take.json` under the scanned roots voids the verdict
   (nullopt). The verdict is relative to `ReferenceScan.roots`; a take saved elsewhere (a tour SSD, the Desktop) makes its asset LOOK
   unreferenced — every derived UI text must say "under <roots>". Multi-root scanning or a per-asset back-reference file is the LATER
   option if Boris wants deletion at all.
3. Proposed LATER UI: "Clean up unused audio…" listing id, recordedAt, size, and which takes reference it; deletion only after
   confirmation; incomplete assets shown separately with "Repair" before "Delete"; the asset being recorded never listed
   (`listIncompleteAssetIds` excludes it). Whether deletion is ever offered is Boris's call (§13).

---

## 8. Fix (c) — `RecorderClock` periodic tempo anchor (Lane E)
Measured problem (`.harmony/.reports/s168-review-lane-a.md:35-54`, VERIFIED): `RecorderClock::tick` writes anchors only on a bpm change
> 0.05, resync "reset", lock/unmetered edges and "start" (`RecorderClock.cpp:16,28,39,56,64`, VERIFIED). A steady-tempo take has ONE
anchor for its whole span; `TempoMap::beatAt` extrapolates at that anchor's bpm (`TempoMap.cpp:63-70`, VERIFIED) — 0.03 BPM bias ×
40 min ≈ 1.2 beats. Worse: with a single anchor `TempoMap::sampleAt` has NO rate source (both branches at `:90-104` need two anchors,
VERIFIED) so it returns `anchor.sample` for every `t` — every `DriveClock::Sample` compile of such a take collapses every continuous
gesture to `x0 == x1` (until fixes-plan L2 lands; even after it, edits need the map). Ruling 28 (2-4 hour takes) makes both load-bearing.

Change (`RecorderClock.h/.cpp` only):
- `static constexpr double kPeriodicAnchorBeats = 32.0;` (8 bars × 4 beats — `FeatureSnapshot.h:44` `beatInBar 0-3`, VERIFIED; spec D1
  "at least every 8 bars", `s167 spec:121-122`, VERIFIED).
- `double lastAnchorBeat_ = 0.0;` + private `void anchor(double t, double beat, uint64_t sample, float bpm, const char* why)` that does
  `tempo_.append(...)` AND `lastAnchorBeat_ = beat`. Route all five existing `tempo_.append` sites through it.
- In the metered branch (`RecorderClock.cpp:41-65`), after the existing bpm-change check, if no anchor was written this tick and
  `beatNow − lastAnchorBeat_ >= kPeriodicAnchorBeats` → `anchor(t, beatNow, deliveredSamples, snap.bpm, "periodic")`.
- No periodic anchors while unmetered (bpm == 0): keeps `TempoMap::tAt`'s documented unmetered semantics (`TempoMap.h:41-45`,
  VERIFIED) — several equal-beat anchors would move its canonical answer. Documented limitation: a long unmetered stretch has only its
  edge anchors.
Cost: 4 h at 128 BPM → 960 anchors ≈ 100 KB of JSON. Fine.

---

## 9. Addendum item 2 — `Player` backwards seek (Lane E)
Problem (`Player.cpp:36-45`, VERIFIED): `advanceTo` sets `pos_ = pos` and only ever moves `nextDiscrete_`/gesture cursors forward; a
smaller `pos` silently skips everything already passed, forever, and keeps calling `set(curve.eval(pos))` on a gesture `pos` is no
longer inside. Ruling 28's cleanup loop scrubs over the audio, and D10.2 drives `pos` from the audio player's position.

Change (`Player.h/.cpp` only):
- NEW `void seek(double pos, Sink& sink);` — running only (no-op otherwise, like `advanceTo`). Semantics: for every cursor `inGesture`:
  if the current gesture still covers `pos` (`g.x0 <= pos < g.x1`) keep `inGesture`/`displaced` as they are (the grip is retained; the
  next `advanceTo` re-evaluates the curve at the new `pos`); else `release()` if not displaced and reset the cursor. Then re-seat
  exactly as `start(at)` does (`Player.cpp:17-33`): `nextDiscrete_` = first event with `at >= pos`; each `gestureIndex` = first gesture
  with `x1 > pos`. `pos_ = pos`. NO state synthesis (no "fire the last event ≤ pos" preamble) — same contract as today's mid-take
  `start()` (`Player.h:36-40`), disclosed. Consequences, stated: an event exactly AT the seek target re-fires on the next
  `advanceTo(pos') , pos' >= at`; a seek landing INSIDE an earlier gesture re-touches it on the next `advanceTo` (as `start()` does).
- `advanceTo(pos)`: if `pos < pos_` → `seek(pos, sink)` first (a backwards jump is never a stall), then the existing forward pass. A
  FORWARD jump through `advanceTo` keeps exactly-once catch-up semantics (D3; the 400 ms stall test) — callers that reposition forward
  deliberately MUST call `seek()`; documented on both methods. No epsilon: any decrease is a seek.
- L4 rider (review addendum 4b, carried verbatim from the sibling plan): `Player.h:59-63` → replace with
  ```cpp
  // D8's LATCH ("your value holds until the lane's next gesture / re-enable") is
  // LATER (D14). Requesting it today is REFUSED, not silently mapped to Touch:
  // returns false, logs, leaves the mode unchanged. Touch (D8's default) is
  // what row 1 implements and tests.
  [[nodiscard]] bool setOverride(Override o);
  Override overrideMode() const { return override_; }
  ```
  (`overrideMode`, not `override` — a contextual keyword.) `Player.cpp`: `bool Player::setOverride(Override o) { if (o != Override::Touch)
  { juce::Logger::writeToLog("Player::setOverride: Latch override is not implemented (spec D8, LATER); staying in Touch"); return false; }
  override_ = o; return true; }`. `Player.cpp` never reads `override_` today (grep, VERIFIED), so nothing else moves.
- `swap()`/`stop()`/`start()` unchanged.

---

## 10. Builder lanes — DISJOINT files, own `-B` dir each (HANDOFF rig fact `:2493`, VERIFIED)

Conventions for BOTH lanes: `cmake -B build-<lane> -DCMAKE_BUILD_TYPE=Release`, never `build/`; build and run only your targets, then
`ctest --test-dir build-<lane>` for your targets; `git add -f` for anything under `.harmony/` and verify every commit with `git show
--stat HEAD` (`HANDOFF.md:2490-2492`); mutation checks restore byte-identical and prove it with `shasum -a 256` against §10.5. Shared
working tree rule (B3): the tree must CONFIGURE at every instant for every lane — create a file BEFORE the CMake line that references
it; remove the reference BEFORE deleting a file; never a whole-file rewrite of a shared file. Counts are RELATIVE per lane; the absolute
count is stated only by Harmony's final gate on ONE clean forced rebuild in ONE directory.

### 10.1 Lane R28 — store + format v3 + tap flush + L1 (`cmake -B build-r28`)
Owns: `src/recording/AudioStore.h`, `src/recording/AudioStore.cpp` (new); `src/recording/AudioTap.h`, `src/recording/AudioTap.cpp`;
`src/recording/Take.h`, `src/recording/Take.cpp`; `CMakeLists.txt`; `tests/CMakeLists.txt` (append-only, §4.3); `tests/test_audio_tap_sync.cpp`;
`tests/test_audio_store.cpp`, `tests/test_take_v1_transport.cpp` (new); `tests/fixtures/take_v3_audio.json`,
`tests/fixtures/take_v2_legacy_audio.json`, `tests/fixtures/take_v1_transport.json` (new); `CLAUDE.md` (source-tree entries for
AudioStore + a 6-line "Audio store" note under the recording section). Must NOT touch `test_take.cpp`, `RecorderClock.*`, `Player.*`,
`Program.*`, `TempoMap.*`, `PerformanceRecorder.*`, anything under `src/connect/` or `src/ui/`.

Steps (three commits, in this order — each leaves the tree configurable and green for the other lanes):
1. **Tap flush.** `AudioTap.h/.cpp` per §4.4. In `tests/test_audio_tap_sync.cpp`: DELETE the folder test at `:343-416` (its assertion
   `folder.dir.getChildFile("audio.wav").existsAsFile()` at `:393` is the inverse of ruling 28; nothing else in the file references
   `Segment::file`) and ADD, in its place, the `[audiotap][flush]` case below. No CMake change. Build `test_audio_tap_sync`; run;
   mutation-check (revert the one `setFlushInterval` line → the flush case fails by poll timeout; restore; sha256). Commit.
2. **Format v3 + L1 + fixtures.** `Take.h/.cpp` per §4.2; write the three fixtures (§10.4). No CMake change. Build `test_audio_tap_sync`
   (still green — nothing in it references `file` after step 1). `test_take` is Lane E's file but links `Take.cpp`: build it too as
   evidence that `Take.h` stays source-compatible; if it fails to compile for a reason inside `tests/test_take.cpp` (Lane E's edit in
   flight), REPORT it, never touch that file. Commit.
3. **Store + targets.** Create ON DISK, in this order: `AudioStore.h`, `AudioStore.cpp`, `tests/test_audio_store.cpp`,
   `tests/test_take_v1_transport.cpp`. THEN edit root `CMakeLists.txt` (+2 sources, +1 module) and APPEND the two blocks to
   `tests/CMakeLists.txt` (§4.3). Configure `build-r28`, build the three targets, run them; `CLAUDE.md` note. Commit (see §4.3 for the
   `git diff tests/CMakeLists.txt` proof).

Tests — `tests/test_audio_tap_sync.cpp` (existing harness `:299-341`):
- `[audiotap][flush] the WAV header is patched while recording, so a crashed show is readable up to the last flush` — `start()`; push 4
  blocks; then poll for ≤ 5 s (10 ms sleeps), each iteration opening a FRESH `WavAudioFormat` reader on the file (a torn read returns
  null — keep polling), until `reader && reader->lengthInSamples > 0`; REQUIRE it happened; REQUIRE `lengthInSamples <=
  tap.framesWritten()`; `stop()`; REQUIRE a fresh reader's `lengthInSamples == tap.framesWritten()`. **Fails on pre-change code**: the
  header stays 0 until the writer's destructor → the poll times out. Deterministic: the first `writePendingData` after `start()` flushes
  (`flushSampleCounter` starts at 0, `juce_AudioFormatWriter.cpp:307-311`), and the TimeSliceThread must run for `stop()` to complete at
  all.

Tests — `tests/test_audio_store.cpp` (all headless; temp dirs under `juce::File::tempDirectory`, RAII-deleted; own tiny helpers — the
`FakeAudioIODevice`/`TempFolder` helpers in `test_audio_tap_sync.cpp` are in an anonymous namespace and are NOT shared; the tap is
driven DIRECTLY via `AudioTap::prepare/start/push/stop` (`AudioTap.h:64,73,94,104`, all public, VERIFIED) with the test's own
`delivered` counter — `CombinedCallback`'s counter identity is already proven by T1 and is not re-proven here):
1. `[format] version constants are 3/3` — `Take::kFormatVersion == 3`, `kMinReader == 3`, `Take{}.toVar()` carries `"version": 3`.
   **Fails pre-change (2/2).**
2. `[format] v1 and v2 fixtures still load under a v3 reader` — `take_v1.json` → `stats.wasV1`, layer-0 lane has 2 points;
   `take_v2_future.json` → not refused, `quantumFlux` still reported.
3. `[format] v3 fixture loads; resolve against an EMPTY store is Missing and names the id` — `take_v3_audio.json`;
   `legacyInFolderAudio == false`; `resolve` → `Missing`, reason contains the id AND the root path.
4. `[format] legacy v2 in-folder audio is flagged, round-trips, and resolves as Legacy` — `take_v2_legacy_audio.json` →
   `stats.legacyInFolderAudio == true`; `toVar()` re-emits `file`, no `id`, no `sha1Head`; `resolve` → `Legacy`.
5. `[store] beginAsset creates the folder and sets the active id; finalize on a synthetic 16-bit WAV writes the sidecar last` — write a
   3-second stereo WAV via `AudioFormatWriter::write(const int**)` with `v << 16`; `finalize` with gaps in take-clock domain
   `{firstSample+1000, 64}` → sidecar gap `{1000, 64}`; `fin.wavReadable && fin.sidecarWritten && fin.error.empty()`;
   `activeAssetId()` empty after; `find()` frames/rate/channels/bits equal the header; `isIncomplete` false after, true once the
   sidecar is deleted.
6. `[store] fp1 is deterministic and specified` — same file twice → same string; expected value computed in the test by hand (header
   string + interleaved PCM16LE of the SAME `v` values written, SHA-256 via `juce::SHA256`) and REQUIRED equal for (a) a 144,000-frame
   stereo file (one window) and (b) a 600,000-frame MONO file (two windows `[0,W)` + `[N−W,N)`); a 32-bit FLOAT WAV → nullopt.
7. `[store] resolve verdicts` — `Resolved` on the finalized asset; `Mismatch` after replacing the WAV with an equal-length file of
   different content (fp1 differs); `Mismatch` after replacing it with a shorter one (frames differ, before fp1); `Missing` after
   deleting the folder; `Incomplete` after deleting only the sidecar (reason names the repair); `MultiSegment` with two segments;
   `NoAudio` for a default `AudioRef`; `Mismatch` for `mode == "input"` with no segments (no crash — B2's deref); `ResolvedUnverified`
   for the v3 reference with `fingerprint = ""` against the complete asset, reason contains the sidecar's fp1.
8. `[store] finalize with an unreadable wav still yields a complete reference` — `beginAsset`; NO wav written; `finalize` with facts
   `{framesWritten 2560, rate 48000, channels 2, firstSample 512}` → `!fin.wavReadable`, `!fin.sidecarWritten`, `error` non-empty,
   `asset.id == id`, `asset.fingerprint == ""`, `asset.frames == 2560`; `referencing(fin.asset, 512)` → `segments[0].id == id`,
   `frames == 2560`; no sidecar exists (never written with an empty fingerprint).
9. `[store] truncation: header shorter than framesWritten sets unreliableFrom` — 1000-frame WAV, `facts.framesWritten = 1500`,
   `facts.unreliableFromInTakeClock = nullopt` → sidecar `unreliableFrom == 1000`, `error` mentions truncation, `sidecarWritten`;
   with `unreliableFromInTakeClock = firstSample + 400` → sidecar `unreliableFrom == 400` (min wins).
10. `[store] repair from the take rebuilds the sidecar` — after test 5's finalize: `take.audio = referencing(asset, 3584)`; delete the
    sidecar; `CaptureFacts::fromAudioRef(take.audio, "0.1.0")` → `finalize` → `find()` returns the SAME fp1 and the same gaps;
    `resolve(take.audio)` → `Resolved`.
11. `[store] active asset is excluded from the incomplete list; abandonAsset is narrow` — `beginAsset`; create a 0-byte `audio.wav` in
    it; `listIncompleteAssetIds()` does NOT contain it, `isIncomplete(id) == false`; `abandonAsset(id)` → true, folder gone, active id
    cleared; a second `abandonAsset(id)` → false; `abandonAsset` on a COMPLETE asset → false and nothing deleted; on a non-active
    incomplete id → false.
12. `[store] fork shares one asset` — `referencing()` into take A; `Take B = A; B.save(otherFolder)`; both load and resolve to the SAME
    `wav` path; the store lists exactly one asset; neither take folder contains `audio.wav`.
13. `[store] referencing round-trips the domain` — sidecar gap frame 1000 + `firstSample` 3584 → take gap sample 4584;
    `unreliableFrom` likewise; `fromAudioRef` inverts both exactly.
14. `[gc] scanTakes + unreferencedAssetIds` — assets X, Y complete, Z incomplete; takes root with two takes → X and one garbage
    `take.json` → verdict nullopt, `unreadableTakes.size() == 1`, `roots == {takesRoot}`; remove the garbage → unreferenced == {Y}; Z
    only in `listIncompleteAssetIds`.
15. `[audiotap][store] a live recording lands in the store, not in the take folder` — `AudioTap tap; tap.prepare(48000, 2, 512)`;
    `beginAsset`; `tap.start(store.wavFile(id))` succeeds (the folder exists → R15 runs on a real volume); push 10 blocks (own
    `delivered` counter advanced by `512 + gapReturned`, `hostTimeNs` advanced one block per push, `popGap` drained after EVERY push);
    `stop()`; `finalize` → `asset.frames == tap.framesWritten()`, `error.empty()`; `take.audio = referencing(fin.asset,
    tap.firstSample())`; `take.save(takeFolder)`; REQUIRE the take folder has NO `audio.wav`; `Take::load` + `resolve` → `Resolved`,
    `r.wav == store.wavFile(id)`, `r.firstSample == tap.firstSample()`, `segments[0].id == id`, `segments[0].file.empty()`,
    `gapDetection == tap.gapDetectionSupported()` (HANDOFF item 3: the R14 flag reaches the take). **Fails pre-change**: does not
    compile (no `AudioStore`); behaviourally the deleted folder test asserted the inverse — the builder notes both.
16. `[audiotap][store] more than 63 gaps survive only when drained per tick` — 80 blocks, each with a 3-block `hostTimeNs` jump (one
    `2×512` gap per block, as T1's drop at `test_audio_tap_sync.cpp:203-211`); run A drains `popGap` after every push → 80 gaps in
    `facts`, sidecar has 80 with `frame == sample − firstSample`; run B drains only after `stop()` → exactly 63 come out
    (`juce_AbstractFifo.cpp:74`), documenting why 5.1 drains per tick.
17. `[audiotap][store] moving the asset folder makes resolve refuse` — rename the folder → `Missing`; move it back → `Resolved` (nothing
    cached).
18. `[audiotap][store] a self-stopped tap is recorded as audio ending early` — `prepare(48000,2,512)`; start; push 5 blocks; `prepare
    (44100, 2, 512)` (rate change → `stopInternal`, `AudioTap.cpp:54-65`); `isRunning() == false`, `framesWritten() == 2560`;
    apply 5.1's rule → `facts.unreliableFromInTakeClock == firstSample + 2560`; `stop()` (no-op); `finalize` → sidecar
    `unreliableFrom == 2560`; `referencing` → `take.audio.unreliableFrom == firstSample + 2560`; `resolve` → `Resolved`.

Tests — `tests/test_take_v1_transport.cpp` (L1, verbatim from the sibling plan, tags `[take][v1][transport]`):
1. "v1 TransportChange bridges play/pause/stop faithfully and counts speed/reverse as dropped": load `take_v1_transport.json`;
   `stats.wasV1`; the Comp/"audio" lane has exactly 3 points, actions play/pause/stop at t 1.0/3.0/4.0, `v == 0` each;
   `stats.v1Dropped == {{"TransportChange:speed",1},{"TransportChange:reverse",1}}`; `take->nextSeq == 5`.
2. "compile of a bridged v1 take dispatches only faithful audio points": `Composition comp; comp.initDefault();` `compile(*take, comp,
   DriveClock::Wall)`; `discrete.size() == 4`; every Fired with `key.control == "audio"` has `p.v == 0` and action in
   {play,pause,stop}; none has speed/reverse; `report.unresolved.empty()`; `report.resolvedCount == 1`; `report.reboundByPosition.size()
   == 1` (the v1 activeClip lane is deck-relative with an empty layerName → PositionOnly, `Program.cpp:50-51` per the sibling plan).
   Pre-change: the TU does not compile (no `v1Dropped`) → behavioural mutation check: keep the `Take.h` field, revert only the
   `Take.cpp` hunk → test 1 fails (5 audio points, v 0/1500/0/1000/0), test 2 fails (`discrete.size() == 6`, a "speed" Fired); restore.

Acceptance (R28): `test_audio_tap_sync` (7 cases: 7 − 1 + 1), `test_audio_store` (18), `test_take_v1_transport` (2) all green in
`build-r28`; relative delta **+19** over the 306 baseline for R28 alone (306 − 1 + 1 + 18 + 2 = 326 if R28 lands first; state the
number you MEASURE); ThreadSanitizer: `cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=Debug -DADNA_SANITIZE=thread`
(`cmake/Sanitizers.cmake:13-14`, VERIFIED; prior run `.harmony/.reports/s168-fix-blocking.md:50`), `--target test_audio_tap_sync`, run
`[concurrency]` and `[flush]` → zero diagnostics (the flush adds a TimeSliceThread-side seek/write against a file the test thread reads —
prove it, do not assume it); every commit `git show --stat HEAD`.

### 10.2 Lane E — long takes and scrubbing (`cmake -B build-laneE`; independent of R28)
Owns: `src/recording/RecorderClock.h`, `src/recording/RecorderClock.cpp`, `src/recording/Player.h`, `src/recording/Player.cpp`,
`tests/test_take.cpp`. Must NOT touch `TempoMap.*`, `Program.*`, `Take.*`, `AudioTap.*`, any CMake file. Lane E appends nothing to
`tests/CMakeLists.txt`.
Tests (append to `tests/test_take.cpp`, same helpers `makeSnap`/`FakeSink`/`layerKey` at `:23-73`, VERIFIED):
- `[recorderclock][long] 40 minutes at a 0.03 BPM reporting bias stays within 0.05 beats and one block of the map` — tick at 120 Hz for
  2400 s: true phase advances `128/60/120` per tick (double accumulator, `fmod` 1.0, cast to float for the snap), `snap.bpm = 127.97f`
  constant, `deliveredSamples += 400`. After the loop: `now.beat ≈ 5120 (margin 0.5)`; `|tempo().beatAt(now.t) − now.beat| < 0.05`
  (**pre-change ≈ 1.2 → fails**); `|sampleAt(now.t) − now.sample| <= 400` (**pre-change returns the start sample → fails by ~115 M**);
  count of `why == "periodic"` anchors in [150, 170] (5120/32 = 160); consecutive anchors' `beat` deltas ≤ 32.5; `tempo().a` sorted by
  `t`. Error bound: 0.03/60 × 15 s ≈ 0.0075 beats (re-derived; the critic concurred).
- `[recorderclock][long] no periodic anchors while unmetered` — 60 s of `bpm == 0` after a lock: anchor count grows by exactly the one
  "unmetered" edge.
- `[recorderclock] existing monotonic/resync case (:226-272) still green` — unchanged, re-run.
- `[player][seek] a backwards advanceTo re-seats and events re-fire on re-pass` — discrete at 1.0/2.0/3.0, gesture [1.5, 2.5] ramp 0→1.
  `start(0)`; `advanceTo(2.2)` → fired {1,2}, 1 touch; `advanceTo(1.2)` → 1 release, no new fire; `advanceTo(2.2)` → fired {1,2,2},
  2 touches; `advanceTo(3.5)` → fired size 4, 2 releases. **Pre-change: fired size stays 3 and touches 1 → fails.** (Hand-traced against
  `Player.cpp:12-89` and the §9 semantics this session; every count matches.)
- `[player][seek] seeking within a gesture keeps the grip and re-evaluates` — `advanceTo(2.0)` then `advanceTo(1.8)`: no release, last
  `set` value (0.3) < the previous (0.5), still exactly 1 touch.
- `[player][seek] explicit forward seek skips, advanceTo catches up` — from 1.2 `seek(2.9)` → no fire; `advanceTo(3.1)` → only event 3.
  Separately from 1.2 `advanceTo(3.1)` → events 2 and 3 (catch-up unchanged; the `:384-443` stall case still green).
- `[player][override] Latch is refused loudly` (L4) — `Player player(std::make_shared<Program>()); REQUIRE(player.overrideMode() ==
  Player::Override::Touch); REQUIRE_FALSE(player.setOverride(Player::Override::Latch)); REQUIRE(player.overrideMode() ==
  Player::Override::Touch); REQUIRE(player.setOverride(Player::Override::Touch));` Pre-change: does not compile (void return, no
  getter) — contract test; behavioural mutation check = store unconditionally and return true → fails; restore.
Acceptance (E): `test_take` green in `build-laneE`, 11 → 17 cases (**+6**); the "fails on pre-change" cases mutation-checked against
a stash of the pre-change `RecorderClock.cpp`/`Player.cpp` (the s168 method, `HANDOFF.md:2422-2423`), source restored byte-identical
(§10.5 pins).

### 10.3 Order of execution and the final gate
R28 and E run in parallel (disjoint files; shared tree rule in §10). Fixes-plan lanes: L2 (`Program.*`), L3 (`PerformanceRecorder.*`),
L5 (`RecordPanel.*`), L6 (`test_oscillator_bar_fold.cpp`) stay in the sibling plan and never touch these lanes' files; L0 no longer
exists; L1 and L4 are absorbed here. Final gate (Harmony, not a builder): ONE clean forced rebuild in ONE directory with every lane
merged; expected `ctest` = 306 + 19 (R28) + 6 (E) + whatever the sibling lanes add — RUN it, never inherit it.

### 10.4 Fixture contents (R28 writes these verbatim)
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
`tests/fixtures/take_v1_transport.json` (L1, exact):
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

### 10.5 Pre-change sha-256 pins (computed this session at HEAD e3504be; mutation restores must match)
```
830aa03ddd2590d278a65e9020335e41f9863e371508a269e0e1b56a8eaf59b7  src/recording/AudioTap.cpp
acdee8b7b50b9ba809fad05ad753163d7d2f4794fed2549cd33926140a31ced4  src/recording/AudioTap.h
92885fd661aafa61e3f391ff0a4f9bc62686631b464c1abd441b69fd879795a3  src/recording/Take.cpp
0e638bd96a20d50c63b379a7dc15af3716c832028496ddcd5f9ceedf602c0539  src/recording/Take.h
fe4e5c34e889d3a1d081e5e4b394f95812e62bb1495419dd4b94dcf6c2b52b72  src/recording/Player.cpp
b46ed9bef4eced3ded045981f35cc8dab68dfa65f44b18445cff13479dd5985c  src/recording/Player.h
9b0fe0803624fe85e3b04c9775048bd3036a7f5e60b05faba55b226ec3fcfeab  src/recording/RecorderClock.cpp
3104e1c0457b6d227d96f588efb753f8ba1c182ed122463f0996ae780145d066  src/recording/RecorderClock.h
b87c448343b95aa2a8243ac8388098048010e8a516dc410a13f9d34e4807d9cf  tests/test_audio_tap_sync.cpp
291d1887bdea302a93cb732a83649cf9e423e4b782a83d845316e22227807f9f  tests/test_take.cpp
90072790cc349f0c9e4b540bf666b878edaa599db7336f993bce4e8436d47a2e  tests/CMakeLists.txt
a4eb097604f8fa78c5ea44e8be2020908886b9deb4b5497bd24d4ad80417d2bc  CMakeLists.txt
```
(`Take.cpp` and `Player.cpp` match the sibling plan's pins — the files are unchanged since s168.)

---

## 11. Tradeoffs considered
- Store per-asset FOLDER vs flat `<id>.wav`+`<id>.json` — folder chosen: the sidecar-last "complete" flag and a future FLAC transcode
  live beside the WAV; one `deleteRecursively` per asset. Flat rejected: two files per identity, no atomic unit.
- Human-readable folder names vs bare id — bare id chosen: direct-path resolution, no scan; the sidecar carries `recordedAt`.
- Keep `gaps`/`unreliableFrom` only in the take vs also in the sidecar — both: a fresh take against an asset needs them (they describe
  the audio, not the knobs); the take doubles as the sidecar's backup (D-A3). Cost: a tiny list duplicated; conversion tested.
- Hard-links / APFS clones to dedupe in-folder audio — rejected: break on exFAT/USB/other-OS copies; leave ownership open.
- Fix the `:453` give-up inside `AudioTap::stopInternal` (set `unreliableFrom`, bump `droppedFrames_`) vs detect it in `finalize()` —
  `finalize()` chosen: one header-vs-`framesWritten` rule catches that path, the `:467` path, and any future one, with no threading
  reasoning; the tap change would be a second, partial signal. The tap's `droppedFrames_` staying unbumped on that path is a documented
  limit, not a lie: the sidecar says where the audio stops.
- Refuse a fingerprint-less reference vs `ResolvedUnverified` — see D-A12.
- `abandonAsset` vs "leave the failed-arm folder for the cleanup UI" — a 0-byte asset that can never be repaired is cruft the store
  itself created seconds ago; the preconditions make it impossible to reach anything recorded. See D-A13.
- Interpolating `TempoMap::beatAt` between anchors instead of periodic anchors — rejected for now: with one anchor there is nothing to
  interpolate; a 32-beat segment bounds the bias to ≈ 0.0075 beats (a 1/4-beat replay snap, ruling 15, is 30× coarser).
- `jassert(pos >= pos_)` instead of seek semantics — rejected: ruling 28 makes backwards `pos` routine.
- Flush interval as a runtime setting — rejected: nothing asks for it; a constant is one line and one test.

---

## 12. RISKS — strongest counterarguments, and what to verify before/while building
R1. **`fp1` is partial by design.** A corrupted middle of a 4-hour file is undetected. Mitigation: app-managed, immutable after
    finalize; versioned sidecar. Verify: nothing — a stated limit.
R2. **`>> 16` PCM16 extraction — now VERIFIED** (`juce_AudioDataConverters.h:171,173`). The hand-computed fp1 test still pins it; if it
    ever fails the builder reads the converters, not the test.
R3. **One sample domain per take is an invariant step 3 must honour** (5.2). Scrubbing while armed ⇒ non-monotonic `sample` stamps
    unless disabled or treated as record-over. Not this spec; named so step 3 cannot miss it.
R4. **`kMinReader = 3` refuses every v3 file for any external tool written against v2.** None exists (VERIFIED by absence).
R5. **The flush test's poll is timing-tolerant, not timing-dependent**: it waits for the writer thread to run (≤ 5 s), which the same
    thread must do for `stop()` to return at all. Under ThreadSanitizer the reader/writer file access is on different threads through
    the OS — prove zero diagnostics (§10.1 acceptance).
R6. **`AudioTap::start` R15 check runs on the STORE's volume** (`AudioTap.cpp:109`) — correct. A CI box with < 2 GB free fails R28's
    test 15 loudly. Also: `kMinFreeBytes` (2 GB) is BELOW one 4-hour night (2.76 GB) — Boris Q2, not changed here.
R7. **Fix (b) — exact stamps in `Program::compile` — is still owed** (fixes-plan L2, `Program.cpp:174-183`). Periodic anchors shrink the
    error but the compile path still reconstructs `x` from the map until L2 lands.
R8. **Forward `seek()` vs `advanceTo` catch-up is a caller contract.** If step 3 only ever calls `advanceTo`, a deliberate forward scrub
    fires every skipped event as a burst. Documented on both methods; 5.6 item 5.
R9. **`juce_cryptography` link is ASSUMED to configure cleanly** on this FetchContent JUCE (module present; depends only on `juce_core`,
    VERIFIED). R28's first configure proves it.
R10. **Periodic anchors and `tAt` inside metered segments** — anchors are strictly increasing in `beat` while metered, so
    `bracketByBeat` (`TempoMap.cpp:51-60`) stays unambiguous. Verify with the unmetered case.
R11. **The R14 flag glue (HANDOFF item 3)** is satisfied by 5.1's `CaptureFacts.gapDetection`, proven headlessly by test 15; the
    production call is still step 3's.
R12. **Shared working tree, two builders.** The B3 rule (create before reference) protects configure; it does not protect a lane from
    the OTHER lane's mid-edit compile errors in files it does not own (e.g. R28 building `test_take` while E is mid-edit). Rule: report,
    never touch, never stash another lane's file.
R13. **Bounded loss on crash is ≈ 12-18 s, not 0.** If Boris wants tighter, it is one constant (`kHeaderFlushSeconds`), not a design change.

Counter-argument to the whole approach, and why it loses: "a store is infrastructure; keep `audio.wav` in the take folder and let Save-As
copy it — disks are cheap." Ruling 28's own arithmetic (`binding-decisions.md:452-455,463-464`): five re-dos of one night = ~14 GB of
identical bytes, and every re-do is the PRODUCT. A store is ~350 lines and one CMake module; the copy model gets uglier with every real
take. `HANDOFF.md:2391-2392`: do it before anyone records anything real.

---

## 13. Questions for Boris (product calls; none block the lanes — defaults chosen and named)
1. **Deleting audio.** Should the app EVER delete stored audio — only via an explicit "Clean up unused audio" that lists what would go,
   or never (leave it to the Finder)? When the last take referencing an audio is deleted, prompt to delete the audio, or leave it?
   Default built: never deletes (except its own failed-arm folder); query only.
2. **Where the store lives, and the free-space floor.** Default `~/Documents/Audio-DNA/Audio` (≈ 2.8 GB per 4-hour night). On macOS
   `~/Documents` may be iCloud "Desktop & Documents" with Optimize Storage, which can evict a 2.8 GB `audio.wav` to a stub — at resolve
   time indistinguishable from deletion (`Missing`). Do you want the store on an external/tour SSD from Preferences? And the tap's
   floor is 2 GB free (`AudioTap.h:132`) — below one night; it would arm on a 2.5 GB-free disk and hit the overrun path at hour ~3.6.
   Raise to ~4 GB, or make it "one night at the current rate"? Default built: fixed path; floor unchanged.
3. **Naming.** Assets are identified by id + date. Name an audio at record time ("Friday Berlin main set"), or is "date + the takes that
   use it" enough? Default built: sidecar has room for a name; nothing asks for one yet.

---

## 14. Critique disposition (every finding, one line each; 1-8 are applied in the text above)
| # | Verdict | Where |
|---|---|---|
| 1 | ACCEPTED (blocking) — flush line + test + ownership | D-A10, §4.4, §10.1 step 1 |
| 2 | ACCEPTED (blocking) — reference always written; `ResolvedUnverified`; `fromAudioRef` repair; step (1) fixed | D-A11/12, §4.1, §5.1, §5.4 |
| 3 | ACCEPTED (blocking) — create-before-reference, three commits | §10.1, shared-tree rule in §10 |
| 4 | ACCEPTED (blocking) — resolved by Harmony's re-cut: no L0; R28 appends two blocks at EOF; E appends none | §4.3, §10.3 |
| 5 | ACCEPTED — drain `popGap` every tick; capacity is 63 (`juce_AbstractFifo.cpp:74`), test 16 | §5.1, §10.1 |
| 6 | ACCEPTED — `framesWritten` in `CaptureFacts`; `finalize` truncation rule; v1's "unreliableFrom says why" text removed | §4.1, §5.1, test 9 |
| 7 | ACCEPTED — `isRunning()` before `stop()`; `unreliableFrom = firstSample + framesWritten`; test 18 | §5.1 |
| 8 | ACCEPTED — exact formula from `AudioEngine.cpp:48-50` + `juce_AudioTransportSource.cpp:189-198`; message thread only | §5.2 |
| 9 | ACCEPTED — `ReferenceScan.roots`; "under <roots>" wording mandatory | §4.1, §7.2 |
| 10 | ACCEPTED — `activeAssetId_`; excluded from incomplete lists; `abandonAsset` | §4.1, D-A13, test 11 |
| 11 | ACCEPTED — (a) reason corrected (statfs walks 5 ancestors; tap creates the parent), (b) ≤ 48-bit entropy stated + re-mint, (c) `deleteRecursively` via `abandonAsset` | D-A1, D-A13 |
| 12 | ACCEPTED — (a) int write `v << 16`, float WAVs refused, `>> 16` now VERIFIED; (b) TSan recipe cited; relative counts | §3, §10.1 |
| 13 | ACCEPTED as a step-3 MUST (out of scope to build here) — provisional `take.json` at arm + periodic save | §5.6 |
| 14 | ACCEPTED as Boris Q2 (iCloud eviction; 2 GB floor < one night) — no constant changed | §13 |
| 15 | ACCEPTED — `mode` semantics, `recordedAt` carries the local offset (`juce_Time.cpp:427-437`), "complete" defined | §2.1, §4.1 |

REPORT_FILE: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/7e364303-6335-4445-955e-49321e60ded4/scratchpad/s-rta-0923-ruling28-audio-store.v2.md
STATUS: DONE
