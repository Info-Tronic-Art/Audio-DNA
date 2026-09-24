#pragma once
#include "recording/Take.h"
#include <juce_core/juce_core.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// AudioStore.h -- s-rta-0923 Ruling 28: the shared audio store. Recorded
// audio is the ANCHOR; many takes reference one asset. Layout:
//   ~/Documents/Audio-DNA/Audio/<id>.adna-audio/{audio.wav, audio.json}
// The sidecar (audio.json) is written LAST -- its presence is the
// "complete" flag; an incomplete asset (a crashed show) is repairable
// FROM ANY TAKE that references it (CaptureFacts::fromAudioRef). Nothing
// is deleted by code except this instance's own just-minted,
// never-finalized asset on a failed arm (abandonAsset, D-A13).
// Message-thread only -- blocking file I/O is fine there (same posture as
// AudioTap::start/stop).

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
    std::string recordedAt;         // Time::toISO8601(true) -- local offset (spec 2.1)
    std::string app;
    std::string mode;               // how the AUDIO was captured: "input" | "file"
    bool gapDetection = false;
    std::vector<std::pair<uint64_t, uint32_t>> gaps;   // ASSET-FRAME domain
    std::optional<uint64_t> unreliableFrom;             // asset-frame domain; may == frames (capture ended early)

    static constexpr int kSidecarVersion = 1;
    juce::var toVar() const;
    static std::optional<AudioAsset> fromVar(const juce::var& v);   // nullopt: not an object / wrong format / no id
};

// AudioStore -- ruling 28's shared audio store. Message-thread only.
class AudioStore
{
public:
    explicit AudioStore(juce::File root);
    static juce::File defaultRoot();                         // ~/Documents/Audio-DNA/Audio
    const juce::File& root() const { return root_; }

    static constexpr const char* kAssetSuffix = ".adna-audio";
    juce::File assetFolder(const std::string& id) const;      // <root>/<id>.adna-audio
    juce::File wavFile(const std::string& id) const;          // <folder>/audio.wav  -- hand THIS to AudioTap::start()
    juce::File sidecarFile(const std::string& id) const;      // <folder>/audio.json

    // --- recording a new asset (sequence in spec 5.1) ---
    // Mints an id (re-minting up to 3 times if the folder already exists)
    // and CREATES the folder so the asset exists as a unit from the first
    // instant. Records it as the active asset (see activeAssetId/
    // abandonAsset). nullopt if the root or the folder cannot be created.
    std::optional<std::string> beginAsset();
    std::optional<std::string> activeAssetId() const;         // set by beginAsset(), cleared by finalize()/abandonAsset()

    // The ONE delete in this class (spec 7, D-A13): removes the folder of
    // the asset THIS instance minted and never finalized (a failed arm --
    // AudioTap::start may have left a 0-byte audio.wav). Refuses (false,
    // nothing touched) unless id == activeAssetId() AND no sidecar exists.
    bool abandonAsset(const std::string& id);

    struct CaptureFacts
    {
        std::string mode;                                    // "input" | "file"
        bool gapDetection = false;                            // AudioTap::gapDetectionSupported()
        uint64_t firstSample = 0;                             // AudioTap::firstSample()
        uint64_t framesWritten = 0;                           // AudioTap::framesWritten() -- truncation check + stub when the wav is unreadable
        double rate = 0.0;                                    // device rate/channels at AudioTap::start() time (used ONLY when the wav is unreadable)
        int channels = 0;
        std::vector<std::pair<uint64_t, uint32_t>> gapsInTakeClock;   // drained via AudioTap::popGap EVERY TICK (spec 5.1), take-clock domain
        std::optional<uint64_t> unreliableFromInTakeClock;    // AudioTap::unreliableFrom(), or firstSample+framesWritten if the tap self-stopped (spec 5.1)
        std::string app;

        // Repair path (D-A3/D-A11): the take is the sidecar's backup.
        // Requires ref.segments.size() == 1; copies mode/gapDetection/gaps/
        // unreliableFrom and seg.{firstSample, frames->framesWritten,
        // rate, channels}.
        static std::optional<CaptureFacts> fromAudioRef(const AudioRef& ref, std::string app);
    };

    struct FinalizeResult
    {
        AudioAsset asset;            // ALWAYS filled (D-A11): from the WAV header when readable, else from `facts` with fingerprint ""
        bool wavReadable = false;
        bool sidecarWritten = false;
        std::string error;           // "" on full success; otherwise names the id and what failed
    };
    // Reads the finished WAV header, computes fp1, converts gaps/
    // unreliableFrom to the asset-frame domain (saturating: frame = sample
    // - firstSample, 0 if sample < firstSample), then -- if the header's
    // lengthInSamples < facts.framesWritten (a truncation) -- sets
    // asset.unreliableFrom = min(existing, lengthInSamples) and notes it in
    // `error`. Writes the sidecar LAST via File::replaceWithText. Clears
    // the active id. NEVER writes a sidecar with an empty fingerprint.
    FinalizeResult finalize(const std::string& id, const CaptureFacts& facts);

    // --- lookup ---
    std::optional<AudioAsset> find(const std::string& id) const;    // "complete" == sidecar parses AND sidecar.id == id (== folder stem)
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
        std::vector<juce::File> roots;               // what was scanned -- every verdict is relative to these (finding 9)
        std::vector<std::string> referencedIds;      // union over every readable take under `roots`
        std::vector<juce::File> unreadableTakes;     // take.json missing/unparseable -- if non-empty, no orphan verdict is valid
    };
    static ReferenceScan scanTakes(const juce::File& takesRoot);   // recursive "*.adna-take" search; parses take.json, reads ONLY "audio.segments[].id"
    // Complete assets referenced by no readable take under takesRoot. nullopt (refuses a verdict) if scan.unreadableTakes is non-empty.
    // Any text derived from this MUST read "not referenced by any take under <roots>".
    std::optional<std::vector<std::string>> unreferencedAssetIds(const juce::File& takesRoot) const;

private:
    juce::File root_;
    std::optional<std::string> activeAssetId_;
};
