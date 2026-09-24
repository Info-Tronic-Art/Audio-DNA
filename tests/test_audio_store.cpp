// test_audio_store -- s-rta-0923 Lane R28: AudioStore, the shared audio
// store (take format v3). All headless (ctest); temp dirs under
// juce::File::tempDirectory, RAII-deleted. Own tiny helpers -- the
// FakeAudioIODevice/TempFolder helpers in test_audio_tap_sync.cpp are in an
// anonymous namespace and are NOT shared. AudioTap is driven DIRECTLY via
// prepare/start/push/stop (all public) with this file's own `delivered`
// counter -- CombinedCallback's counter identity is already proven
// elsewhere and is not re-proven here.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "recording/AudioStore.h"
#include "recording/AudioTap.h"
#include "recording/Take.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_cryptography/juce_cryptography.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

using Catch::Approx;

namespace
{
    struct TempDir
    {
        juce::File dir;
        explicit TempDir(const juce::String& tag)
            : dir(juce::File::getSpecialLocation(juce::File::tempDirectory)
                      .getChildFile("audiodna_store_test_" + tag + "_"
                                    + juce::String(juce::Random::getSystemRandom().nextInt64())))
        {
            dir.createDirectory();
        }
        ~TempDir() { dir.deleteRecursively(); }
    };

    // Writes a synthetic 16-bit PCM WAV via AudioFormatWriter::write(const
    // int**) with v << 16 (spec section 3 step 3's >>16 recovery, VERIFIED
    // against juce_AudioDataConverters.h -- see AudioStore.cpp's
    // fingerprint()).
    void writeSynthWav(const juce::File& path, double rate, int channels, uint64_t frames,
                        const std::function<int16_t(uint64_t frame, int channel)>& gen)
    {
        // FileOutputStream APPENDS at the end of an already-existing file
        // (it does not truncate) -- delete first so re-writing the same
        // path (the resolve()-verdict test's "replace the WAV" cases)
        // actually overwrites rather than leaving the OLD header+data
        // untouched at offset 0 with new bytes appended after it.
        path.deleteFile();
        juce::WavAudioFormat format;
        auto* stream = new juce::FileOutputStream(path);
        std::unique_ptr<juce::AudioFormatWriter> writer(
            format.createWriterFor(stream, rate, static_cast<unsigned int>(channels), 16, {}, 0));
        REQUIRE(writer != nullptr);

        std::vector<std::vector<int>> chans(static_cast<size_t>(channels));
        std::vector<const int*> ptrs(static_cast<size_t>(channels));
        for (int c = 0; c < channels; ++c)
        {
            chans[static_cast<size_t>(c)].resize(static_cast<size_t>(frames));
            for (uint64_t f = 0; f < frames; ++f)
                chans[static_cast<size_t>(c)][static_cast<size_t>(f)] = static_cast<int>(gen(f, c)) << 16;
            ptrs[static_cast<size_t>(c)] = chans[static_cast<size_t>(c)].data();
        }
        REQUIRE(writer->write(ptrs.data(), static_cast<int>(frames)));
        writer.reset();   // flush + finalize the header
    }

    // Hand-computed fp1 (spec section 3), independent of AudioStore's own
    // fingerprint() implementation -- built directly from the generator
    // function used to write the file, not by re-reading it.
    std::string handFp1(uint64_t N, int R, int C, int B,
                         const std::function<int16_t(uint64_t frame, int channel)>& gen)
    {
        const juce::String header = "adna-fp1|frames=" + juce::String(static_cast<juce::int64>(N))
            + "|rate=" + juce::String(R) + "|channels=" + juce::String(C) + "|bits=" + juce::String(B) + "|";
        juce::MemoryBlock bytes;
        bytes.append(header.toRawUTF8(), header.getNumBytesAsUTF8());

        constexpr uint64_t W = AudioStore::kFingerprintWindowFrames;
        auto appendWindow = [&](uint64_t start, uint64_t count)
        {
            for (uint64_t f = 0; f < count; ++f)
                for (int c = 0; c < C; ++c)
                {
                    const int16_t v = gen(start + f, c);
                    const uint16_t uv = static_cast<uint16_t>(v);
                    const uint8_t le[2] = { static_cast<uint8_t>(uv & 0xFF), static_cast<uint8_t>((uv >> 8) & 0xFF) };
                    bytes.append(le, 2);
                }
        };
        if (N <= W * 2)
            appendWindow(0, N);
        else
        {
            appendWindow(0, W);
            appendWindow(N - W, W);
        }
        return std::string("fp1:") + juce::SHA256(bytes.getData(), bytes.getSize()).toHexString().toStdString();
    }
}

// === 1: [format] version constants are 3/3 ===

TEST_CASE("Take format version constants are 3/3", "[format]")
{
    REQUIRE(Take::kFormatVersion == 3);
    REQUIRE(Take::kMinReader == 3);

    Take take;
    const juce::var v = take.toVar();   // keep alive -- obj points into it
    auto* obj = v.getDynamicObject();
    REQUIRE(obj != nullptr);
    REQUIRE(static_cast<int>(obj->getProperty("version")) == 3);
}

// === 2: [format] v1 and v2 fixtures still load under a v3 reader ===

TEST_CASE("v1 and v2 fixtures still load under a v3 reader", "[format]")
{
    {
        juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v1.json");
        REQUIRE(fixture.existsAsFile());
        LoadStats stats;
        auto take = Take::load(fixture, stats);
        REQUIRE(take.has_value());
        REQUIRE_FALSE(stats.refused);
        REQUIRE(stats.wasV1);

        ControlPath layer0;
        layer0.scope = ControlPath::Scope::Layer;
        layer0.deckRelative = true;
        layer0.layer = 0;
        layer0.control = "activeClip";
        REQUIRE(take->lanes.count(layer0) == 1);
        REQUIRE(take->lanes.at(layer0).points.size() == 2);
    }
    {
        juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v2_future.json");
        REQUIRE(fixture.existsAsFile());
        LoadStats stats;
        auto take = Take::load(fixture, stats);
        REQUIRE(take.has_value());
        REQUIRE_FALSE(stats.refused);
        REQUIRE(std::find(stats.unknownFeatures.begin(), stats.unknownFeatures.end(), "quantumFlux")
                != stats.unknownFeatures.end());
    }
}

// === 3: [format] v3 fixture loads; resolve against an EMPTY store is Missing and names the id ===

TEST_CASE("v3 fixture loads; resolve against an empty store is Missing and names the id", "[format]")
{
    juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v3_audio.json");
    REQUIRE(fixture.existsAsFile());

    LoadStats stats;
    auto take = Take::load(fixture, stats);
    REQUIRE(take.has_value());
    REQUIRE_FALSE(stats.refused);
    REQUIRE_FALSE(stats.legacyInFolderAudio);
    REQUIRE(take->audio.segments.size() == 1);
    REQUIRE(take->audio.segments[0].id == "0123456789abcdef0123456789abcdef");

    TempDir emptyStore("empty3");
    AudioStore store(emptyStore.dir);
    auto res = store.resolve(take->audio);
    REQUIRE(res.status == AudioStore::Status::Missing);
    REQUIRE(res.reason.find("0123456789abcdef0123456789abcdef") != std::string::npos);
    REQUIRE(res.reason.find(emptyStore.dir.getFullPathName().toStdString()) != std::string::npos);
}

// === 4: [format] legacy v2 in-folder audio is flagged, round-trips, and resolves as Legacy ===

TEST_CASE("legacy v2 in-folder audio is flagged, round-trips, and resolves as Legacy", "[format]")
{
    juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v2_legacy_audio.json");
    REQUIRE(fixture.existsAsFile());

    LoadStats stats;
    auto take = Take::load(fixture, stats);
    REQUIRE(take.has_value());
    REQUIRE_FALSE(stats.refused);
    REQUIRE(stats.legacyInFolderAudio);
    REQUIRE(take->audio.segments.size() == 1);
    REQUIRE(take->audio.segments[0].id.empty());
    REQUIRE(take->audio.segments[0].file == "audio.wav");

    // Keep the round-tripped var ALIVE across statements (its internal
    // DynamicObjects are reference-counted and owned only by this var --
    // navigating a temporary's chain and stashing a raw pointer past the
    // full expression would dangle).
    const juce::var roundTripped = take->toVar();
    auto* segObj = roundTripped.getDynamicObject()->getProperty("audio").getDynamicObject()
                       ->getProperty("segments").getArray()->getReference(0).getDynamicObject();
    REQUIRE(segObj != nullptr);
    REQUIRE(segObj->getProperty("file").toString() == "audio.wav");
    REQUIRE_FALSE(segObj->hasProperty("id"));
    REQUIRE_FALSE(segObj->hasProperty("sha1Head"));

    TempDir emptyStore("legacy4");
    AudioStore store(emptyStore.dir);
    auto res = store.resolve(take->audio);
    REQUIRE(res.status == AudioStore::Status::Legacy);
}

// === 5: [store] beginAsset creates the folder and sets the active id; finalize on a
//        synthetic 16-bit WAV writes the sidecar last ===

TEST_CASE("beginAsset creates the folder and sets the active id; finalize writes the sidecar last", "[store]")
{
    TempDir root("finalize5");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    REQUIRE(store.assetFolder(*id).isDirectory());
    REQUIRE(store.activeAssetId() == id);

    constexpr double rate = 48000.0;
    constexpr int channels = 2;
    constexpr uint64_t frames = 144000;   // 3 s @ 48 kHz
    auto gen = [](uint64_t f, int c) -> int16_t { return static_cast<int16_t>(((f * 3 + c * 7) % 2000) - 1000); };
    writeSynthWav(store.wavFile(*id), rate, channels, frames, gen);

    AudioStore::CaptureFacts facts;
    facts.mode = "input";
    facts.gapDetection = true;
    facts.firstSample = 0;
    facts.framesWritten = frames;
    facts.rate = rate;
    facts.channels = channels;
    facts.gapsInTakeClock = { { 1000, 64 } };   // take-clock == asset-frame here (firstSample 0)
    facts.app = "0.1.0";

    auto fin = store.finalize(*id, facts);
    REQUIRE(fin.wavReadable);
    REQUIRE(fin.sidecarWritten);
    REQUIRE(fin.error.empty());
    REQUIRE(fin.asset.gaps.size() == 1);
    REQUIRE(fin.asset.gaps[0].first == 1000);
    REQUIRE(fin.asset.gaps[0].second == 64);

    REQUIRE_FALSE(store.activeAssetId().has_value());

    auto found = store.find(*id);
    REQUIRE(found.has_value());
    REQUIRE(found->frames == frames);
    REQUIRE(found->rate == Approx(rate));
    REQUIRE(found->channels == channels);
    REQUIRE(found->bits == 16);

    REQUIRE_FALSE(store.isIncomplete(*id));
    store.sidecarFile(*id).deleteFile();
    REQUIRE(store.isIncomplete(*id));
}

// === 6: [store] fp1 is deterministic and specified ===

TEST_CASE("fp1 is deterministic and specified", "[store]")
{
    TempDir root("fp1test6");

    SECTION("144,000-frame stereo file -- one window")
    {
        auto gen = [](uint64_t f, int c) -> int16_t { return static_cast<int16_t>(((f * 11 + c * 5) % 4000) - 2000); };
        const juce::File wav = root.dir.getChildFile("stereo.wav");
        writeSynthWav(wav, 48000.0, 2, 144000, gen);

        const auto fp = AudioStore::fingerprint(wav);
        REQUIRE(fp.has_value());
        REQUIRE(*fp == handFp1(144000, 48000, 2, 16, gen));

        const auto fp2 = AudioStore::fingerprint(wav);
        REQUIRE(fp2 == fp);   // deterministic
    }

    SECTION("600,000-frame mono file -- two windows")
    {
        auto gen = [](uint64_t f, int /*c*/) -> int16_t { return static_cast<int16_t>((f % 3000) - 1500); };
        const juce::File wav = root.dir.getChildFile("mono.wav");
        writeSynthWav(wav, 48000.0, 1, 600000, gen);

        const auto fp = AudioStore::fingerprint(wav);
        REQUIRE(fp.has_value());
        REQUIRE(*fp == handFp1(600000, 48000, 1, 16, gen));
    }

    SECTION("a 32-bit float WAV -> nullopt")
    {
        const juce::File wav = root.dir.getChildFile("float.wav");
        juce::WavAudioFormat format;
        auto* stream = new juce::FileOutputStream(wav);
        std::unique_ptr<juce::AudioFormatWriter> writer(
            format.createWriterFor(stream, 48000.0, 1u, 32, {}, 0));
        REQUIRE(writer != nullptr);
        std::vector<float> ch(100, 0.25f);
        const float* ptrs[1] = { ch.data() };
        REQUIRE(writer->writeFromFloatArrays(ptrs, 1, 100));
        writer.reset();

        REQUIRE_FALSE(AudioStore::fingerprint(wav).has_value());
    }
}

// === 7: [store] resolve verdicts ===

TEST_CASE("resolve verdicts", "[store]")
{
    TempDir root("resolve7");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    constexpr double rate = 48000.0;
    constexpr uint64_t frames = 48000;
    auto gen = [](uint64_t f, int c) -> int16_t { return static_cast<int16_t>(((f + c) % 1000) - 500); };
    writeSynthWav(store.wavFile(*id), rate, 2, frames, gen);

    AudioStore::CaptureFacts facts;
    facts.mode = "input"; facts.gapDetection = false; facts.firstSample = 0;
    facts.framesWritten = frames; facts.rate = rate; facts.channels = 2; facts.app = "0.1.0";
    auto fin = store.finalize(*id, facts);
    REQUIRE(fin.wavReadable);
    REQUIRE(fin.sidecarWritten);

    AudioRef ref = AudioStore::referencing(fin.asset, 0);

    SECTION("Resolved on the finalized asset")
    {
        auto res = store.resolve(ref);
        REQUIRE(res.status == AudioStore::Status::Resolved);
    }

    SECTION("Mismatch after replacing the WAV with an equal-length file of different content")
    {
        auto gen2 = [](uint64_t f, int c) -> int16_t { return static_cast<int16_t>(((f + c + 1) % 1000) - 500); };
        writeSynthWav(store.wavFile(*id), rate, 2, frames, gen2);
        auto res = store.resolve(ref);
        REQUIRE(res.status == AudioStore::Status::Mismatch);
    }

    SECTION("Mismatch after replacing it with a shorter one (frames differ, before fp1)")
    {
        writeSynthWav(store.wavFile(*id), rate, 2, frames / 2, gen);
        auto res = store.resolve(ref);
        REQUIRE(res.status == AudioStore::Status::Mismatch);
    }

    SECTION("Missing after deleting the folder")
    {
        store.assetFolder(*id).deleteRecursively();
        auto res = store.resolve(ref);
        REQUIRE(res.status == AudioStore::Status::Missing);
    }

    SECTION("Incomplete after deleting only the sidecar (reason names the repair)")
    {
        store.sidecarFile(*id).deleteFile();
        auto res = store.resolve(ref);
        REQUIRE(res.status == AudioStore::Status::Incomplete);
        REQUIRE(res.reason.find("repair") != std::string::npos);
    }

    SECTION("MultiSegment with two segments")
    {
        AudioRef two = ref;
        two.segments.push_back(ref.segments[0]);
        auto res = store.resolve(two);
        REQUIRE(res.status == AudioStore::Status::MultiSegment);
    }

    SECTION("NoAudio for a default AudioRef")
    {
        AudioRef empty;
        auto res = store.resolve(empty);
        REQUIRE(res.status == AudioStore::Status::NoAudio);
    }

    SECTION("Mismatch for mode == 'input' with no segments (no crash -- B2's deref)")
    {
        AudioRef modeOnly;
        modeOnly.mode = "input";
        auto res = store.resolve(modeOnly);
        REQUIRE(res.status == AudioStore::Status::Mismatch);
    }

    SECTION("ResolvedUnverified for the v3 reference with fingerprint = \"\" against the complete asset")
    {
        AudioRef unverified = ref;
        unverified.segments[0].fingerprint.clear();
        auto res = store.resolve(unverified);
        REQUIRE(res.status == AudioStore::Status::ResolvedUnverified);
        REQUIRE(res.reason.find(fin.asset.fingerprint) != std::string::npos);
    }
}

// === 8: [store] finalize with an unreadable wav still yields a complete reference ===

TEST_CASE("finalize with an unreadable wav still yields a complete reference", "[store]")
{
    TempDir root("unreadable8");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    // No wav written at all.

    AudioStore::CaptureFacts facts;
    facts.mode = "input"; facts.gapDetection = true;
    facts.firstSample = 512; facts.framesWritten = 2560;
    facts.rate = 48000.0; facts.channels = 2; facts.app = "0.1.0";

    auto fin = store.finalize(*id, facts);
    REQUIRE_FALSE(fin.wavReadable);
    REQUIRE_FALSE(fin.sidecarWritten);
    REQUIRE_FALSE(fin.error.empty());
    REQUIRE(fin.asset.id == *id);
    REQUIRE(fin.asset.fingerprint.empty());
    REQUIRE(fin.asset.frames == 2560);

    AudioRef ref = AudioStore::referencing(fin.asset, 512);
    REQUIRE(ref.segments.size() == 1);
    REQUIRE(ref.segments[0].id == *id);
    REQUIRE(ref.segments[0].frames == 2560);

    REQUIRE_FALSE(store.sidecarFile(*id).existsAsFile());
}

// === 9: [store] truncation: header shorter than framesWritten sets unreliableFrom ===

TEST_CASE("truncation: header shorter than framesWritten sets unreliableFrom", "[store]")
{
    TempDir root("truncation9");
    AudioStore store(root.dir);

    auto writeAndFinalize = [&](const std::optional<uint64_t>& unreliableFromInTakeClock) -> AudioStore::FinalizeResult
    {
        auto id = store.beginAsset();
        REQUIRE(id.has_value());
        auto gen = [](uint64_t f, int) -> int16_t { return static_cast<int16_t>(f % 100); };
        writeSynthWav(store.wavFile(*id), 48000.0, 1, 1000, gen);

        AudioStore::CaptureFacts facts;
        facts.mode = "input"; facts.gapDetection = false;
        facts.firstSample = 0; facts.framesWritten = 1500;
        facts.rate = 48000.0; facts.channels = 1; facts.app = "0.1.0";
        facts.unreliableFromInTakeClock = unreliableFromInTakeClock;
        return store.finalize(*id, facts);
    };

    SECTION("no prior unreliableFrom -- truncation point wins")
    {
        auto fin = writeAndFinalize(std::nullopt);
        REQUIRE(fin.sidecarWritten);
        REQUIRE(fin.asset.unreliableFrom.has_value());
        REQUIRE(*fin.asset.unreliableFrom == 1000);
        REQUIRE(fin.error.find("truncat") != std::string::npos);
    }

    SECTION("prior unreliableFrom at 400 -- min wins")
    {
        auto fin = writeAndFinalize(400);   // firstSample 0, so asset-frame == 400
        REQUIRE(fin.sidecarWritten);
        REQUIRE(fin.asset.unreliableFrom.has_value());
        REQUIRE(*fin.asset.unreliableFrom == 400);
    }
}

// === 10: [store] repair from the take rebuilds the sidecar ===

TEST_CASE("repair from the take rebuilds the sidecar", "[store]")
{
    TempDir root("repair10");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    auto gen = [](uint64_t f, int c) -> int16_t { return static_cast<int16_t>(((f * 7 + c) % 500) - 250); };
    writeSynthWav(store.wavFile(*id), 48000.0, 2, 48000, gen);

    AudioStore::CaptureFacts facts;
    facts.mode = "input"; facts.gapDetection = true;
    facts.firstSample = 3584; facts.framesWritten = 48000;
    facts.rate = 48000.0; facts.channels = 2; facts.app = "0.1.0";
    facts.gapsInTakeClock = { { 3584 + 1000, 64 } };
    auto fin = store.finalize(*id, facts);
    REQUIRE(fin.wavReadable);
    REQUIRE(fin.sidecarWritten);
    const std::string originalFingerprint = fin.asset.fingerprint;

    Take take;
    take.audio = AudioStore::referencing(fin.asset, 3584);

    store.sidecarFile(*id).deleteFile();
    REQUIRE(store.isIncomplete(*id));

    auto repairedFacts = AudioStore::CaptureFacts::fromAudioRef(take.audio, "0.1.0");
    REQUIRE(repairedFacts.has_value());
    auto repairedFin = store.finalize(*id, *repairedFacts);
    REQUIRE(repairedFin.sidecarWritten);

    auto found = store.find(*id);
    REQUIRE(found.has_value());
    REQUIRE(found->fingerprint == originalFingerprint);
    REQUIRE(found->gaps.size() == 1);
    REQUIRE(found->gaps[0].first == 1000);

    auto res = store.resolve(take.audio);
    REQUIRE(res.status == AudioStore::Status::Resolved);
}

// === 11: [store] active asset is excluded from the incomplete list; abandonAsset is narrow ===

TEST_CASE("active asset is excluded from the incomplete list; abandonAsset is narrow", "[store]")
{
    TempDir root("abandon11");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    store.wavFile(*id).create();   // 0-byte audio.wav, as a failed arm would leave

    const auto incompleteAfterBegin = store.listIncompleteAssetIds();
    REQUIRE(std::find(incompleteAfterBegin.begin(), incompleteAfterBegin.end(), *id) == incompleteAfterBegin.end());
    REQUIRE_FALSE(store.isIncomplete(*id));

    REQUIRE(store.abandonAsset(*id));
    REQUIRE_FALSE(store.assetFolder(*id).exists());
    REQUIRE_FALSE(store.activeAssetId().has_value());

    REQUIRE_FALSE(store.abandonAsset(*id));   // second call -- no longer active

    // A complete asset -- abandonAsset refuses.
    auto id2 = store.beginAsset();
    REQUIRE(id2.has_value());
    auto gen = [](uint64_t f, int) -> int16_t { return static_cast<int16_t>(f % 50); };
    writeSynthWav(store.wavFile(*id2), 48000.0, 1, 4800, gen);
    AudioStore::CaptureFacts facts2;
    facts2.mode = "input"; facts2.firstSample = 0; facts2.framesWritten = 4800;
    facts2.rate = 48000.0; facts2.channels = 1; facts2.app = "0.1.0";
    auto fin2 = store.finalize(*id2, facts2);
    REQUIRE(fin2.sidecarWritten);
    REQUIRE_FALSE(store.abandonAsset(*id2));
    REQUIRE(store.assetFolder(*id2).exists());

    // A non-active incomplete id -- abandonAsset refuses (this instance
    // never minted it as active; activeAssetId_ is empty here).
    auto id3 = store.beginAsset();
    REQUIRE(id3.has_value());
    store.wavFile(*id3).create();
    // Mint a NEW active asset so id3 is no longer the active one.
    auto id4 = store.beginAsset();
    REQUIRE(id4.has_value());
    REQUIRE_FALSE(store.abandonAsset(*id3));
    REQUIRE(store.assetFolder(*id3).exists());
}

// === 12: [store] fork shares one asset ===

TEST_CASE("fork shares one asset", "[store]")
{
    TempDir root("fork12");
    TempDir takesA("takesA12"), takesB("takesB12");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    auto gen = [](uint64_t f, int c) -> int16_t { return static_cast<int16_t>(((f + c * 3) % 700) - 350); };
    writeSynthWav(store.wavFile(*id), 48000.0, 2, 48000, gen);
    AudioStore::CaptureFacts facts;
    facts.mode = "input"; facts.firstSample = 0; facts.framesWritten = 48000;
    facts.rate = 48000.0; facts.channels = 2; facts.app = "0.1.0";
    auto fin = store.finalize(*id, facts);
    REQUIRE(fin.sidecarWritten);

    Take a;
    a.audio = AudioStore::referencing(fin.asset, 0);
    a.save(takesA.dir);

    Take b = a;
    b.save(takesB.dir);

    LoadStats sa, sb;
    auto loadedA = Take::load(takesA.dir, sa);
    auto loadedB = Take::load(takesB.dir, sb);
    REQUIRE(loadedA.has_value());
    REQUIRE(loadedB.has_value());

    auto resA = store.resolve(loadedA->audio);
    auto resB = store.resolve(loadedB->audio);
    REQUIRE(resA.status == AudioStore::Status::Resolved);
    REQUIRE(resB.status == AudioStore::Status::Resolved);
    REQUIRE(resA.wav == resB.wav);

    REQUIRE(store.listAssetIds().size() == 1);
    REQUIRE_FALSE(takesA.dir.getChildFile("audio.wav").existsAsFile());
    REQUIRE_FALSE(takesB.dir.getChildFile("audio.wav").existsAsFile());
}

// === 13: [store] referencing round-trips the domain ===

TEST_CASE("referencing round-trips the domain", "[store]")
{
    AudioAsset asset;
    asset.id = "abcdef0123456789abcdef0123456789";
    asset.fingerprint = "fp1:deadbeef";
    asset.frames = 48000;
    asset.rate = 48000.0;
    asset.channels = 2;
    asset.gaps = { { 1000, 64 } };
    asset.unreliableFrom = 2000;

    constexpr uint64_t firstSample = 3584;
    AudioRef ref = AudioStore::referencing(asset, firstSample);
    REQUIRE(ref.gaps.size() == 1);
    REQUIRE(ref.gaps[0].first == 1000 + firstSample);
    REQUIRE(ref.unreliableFrom.has_value());
    REQUIRE(*ref.unreliableFrom == 2000 + firstSample);

    auto facts = AudioStore::CaptureFacts::fromAudioRef(ref, "0.1.0");
    REQUIRE(facts.has_value());
    REQUIRE(facts->gapsInTakeClock.size() == 1);
    REQUIRE(facts->gapsInTakeClock[0].first == 1000 + firstSample);
    REQUIRE(facts->unreliableFromInTakeClock.has_value());
    REQUIRE(*facts->unreliableFromInTakeClock == 2000 + firstSample);
    // Inverts exactly: (sample - firstSample) recovers the sidecar's frame.
    REQUIRE(facts->gapsInTakeClock[0].first - facts->firstSample == 1000);
    REQUIRE(*facts->unreliableFromInTakeClock - facts->firstSample == 2000);
}

// === 14: [gc] scanTakes + unreferencedAssetIds ===

TEST_CASE("scanTakes + unreferencedAssetIds", "[gc]")
{
    TempDir root("gc14");
    TempDir takesRoot("gctakes14");
    AudioStore store(root.dir);

    auto makeAsset = [&](const std::string& tag) -> std::string
    {
        auto id = store.beginAsset();
        REQUIRE(id.has_value());
        auto gen = [](uint64_t f, int) -> int16_t { return static_cast<int16_t>(f % 40); };
        writeSynthWav(store.wavFile(*id), 48000.0, 1, 4800, gen);
        AudioStore::CaptureFacts facts;
        facts.mode = "input"; facts.firstSample = 0; facts.framesWritten = 4800;
        facts.rate = 48000.0; facts.channels = 1; facts.app = tag;
        auto fin = store.finalize(*id, facts);
        REQUIRE(fin.sidecarWritten);
        return *id;
    };

    const std::string idX = makeAsset("X");
    const std::string idY = makeAsset("Y");

    // Z: incomplete (no sidecar).
    auto idZOpt = store.beginAsset();
    REQUIRE(idZOpt.has_value());
    store.wavFile(*idZOpt).create();
    const std::string idZ = *idZOpt;
    // Mint a fresh active id so Z is no longer "active" for isIncomplete's purposes.
    store.beginAsset();

    // A real take referencing X.
    juce::File takeXDir = takesRoot.dir.getChildFile("takeX.adna-take");
    takeXDir.createDirectory();
    {
        AudioAsset assetX;
        assetX.id = idX;
        AudioRef refX = AudioStore::referencing(*store.find(idX), 0);
        Take t; t.audio = refX; t.save(takeXDir);
    }

    // A garbage take.json.
    juce::File garbageDir = takesRoot.dir.getChildFile("garbage.adna-take");
    garbageDir.createDirectory();
    garbageDir.getChildFile("take.json").replaceWithText("{ not valid json");

    auto scan1 = AudioStore::scanTakes(takesRoot.dir);
    REQUIRE(scan1.unreadableTakes.size() == 1);
    REQUIRE(scan1.roots.size() == 1);
    REQUIRE(scan1.roots[0] == takesRoot.dir);

    auto verdict1 = store.unreferencedAssetIds(takesRoot.dir);
    REQUIRE_FALSE(verdict1.has_value());

    garbageDir.deleteRecursively();

    auto verdict2 = store.unreferencedAssetIds(takesRoot.dir);
    REQUIRE(verdict2.has_value());
    REQUIRE(verdict2->size() == 1);
    REQUIRE((*verdict2)[0] == idY);

    const auto incompleteIds = store.listIncompleteAssetIds();
    REQUIRE(std::find(incompleteIds.begin(), incompleteIds.end(), idZ) != incompleteIds.end());
    const auto completeIds = store.listAssetIds();
    REQUIRE(std::find(completeIds.begin(), completeIds.end(), idZ) == completeIds.end());
}

// === 15: [audiotap][store] a live recording lands in the store, not in the take folder ===

TEST_CASE("a live recording lands in the store, not in the take folder", "[audiotap][store]")
{
    TempDir root("live15");
    TempDir takesDir("livetakes15");
    AudioStore store(root.dir);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;

    AudioTap tap;
    tap.prepare(rate, 2, blockSize);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    REQUIRE(tap.start(store.wavFile(*id)));

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    const uint64_t deltaNs = static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
    std::vector<float> inL(blockSize, 0.1f), inR(blockSize, 0.1f);
    const float* ch[2] = { inL.data(), inR.data() };

    for (int b = 0; b < 10; ++b)
    {
        hostTimeNs += deltaNs;
        juce::AudioIODeviceCallbackContext ctx;
        ctx.hostTimeNs = &hostTimeNs;
        const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
        delivered += static_cast<uint64_t>(blockSize) + gap;

        std::pair<uint64_t, uint32_t> g;
        while (tap.popGap(g)) { /* drained every tick, spec 5.1 */ }
    }

    tap.stop();

    AudioStore::CaptureFacts facts;
    facts.mode = "input";
    facts.gapDetection = tap.gapDetectionSupported();
    facts.firstSample = tap.firstSample();
    facts.framesWritten = tap.framesWritten();
    facts.rate = rate;
    facts.channels = 2;
    facts.app = "0.1.0";

    auto fin = store.finalize(*id, facts);
    REQUIRE(fin.asset.frames == tap.framesWritten());
    REQUIRE(fin.error.empty());

    Take take;
    take.audio = AudioStore::referencing(fin.asset, tap.firstSample());
    take.save(takesDir.dir);

    REQUIRE_FALSE(takesDir.dir.getChildFile("audio.wav").existsAsFile());

    LoadStats stats;
    auto loaded = Take::load(takesDir.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE_FALSE(stats.refused);

    auto res = store.resolve(loaded->audio);
    REQUIRE(res.status == AudioStore::Status::Resolved);
    REQUIRE(res.wav == store.wavFile(*id));
    REQUIRE(res.firstSample == tap.firstSample());
    REQUIRE(loaded->audio.segments.size() == 1);
    REQUIRE(loaded->audio.segments[0].id == *id);
    REQUIRE(loaded->audio.segments[0].file.empty());
    REQUIRE(loaded->audio.gapDetection == tap.gapDetectionSupported());
}

// === 16: [audiotap][store] more than 63 gaps survive only when drained per tick ===

TEST_CASE("more than 63 gaps survive only when drained per tick", "[audiotap][store]")
{
    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    constexpr int kBlocks = 80;

    struct RunResult
    {
        std::string id;
        std::vector<std::pair<uint64_t, uint32_t>> gaps;
        uint64_t firstSample = 0;
        uint64_t framesWritten = 0;
    };

    auto runOnce = [&](AudioStore& store, bool drainPerTick) -> RunResult
    {
        auto id = store.beginAsset();
        REQUIRE(id.has_value());
        AudioTap tap;
        tap.prepare(rate, 2, blockSize);
        REQUIRE(tap.start(store.wavFile(*id)));

        uint64_t delivered = 0;
        uint64_t hostTimeNs = 1'000'000'000ULL;
        std::vector<float> inL(blockSize, 0.0f), inR(blockSize, 0.0f);
        const float* ch[2] = { inL.data(), inR.data() };
        std::vector<std::pair<uint64_t, uint32_t>> gaps;

        // Warm-up push at normal (1x) timing -- establishes push()'s
        // haveLastHostTime_ baseline, exactly as T1's own harness has many
        // ordinary blocks before its one gap block. Without this, the
        // FIRST call in the loop below can never register a gap (no prior
        // hostTimeNs to compare against), undercounting by one.
        {
            hostTimeNs += static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
            juce::AudioIODeviceCallbackContext ctx;
            ctx.hostTimeNs = &hostTimeNs;
            const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
            delivered += static_cast<uint64_t>(blockSize) + gap;
        }

        for (int b = 0; b < kBlocks; ++b)
        {
            // A 3-block hostTimeNs jump for 1 delivered block's worth of
            // samples -- a 2*blockSize gap every push (matches T1's own
            // drop pattern, test_audio_tap_sync.cpp's kDropBlock).
            const uint64_t deltaNs = static_cast<uint64_t>((3.0 * blockSize / rate) * 1.0e9);
            hostTimeNs += deltaNs;
            juce::AudioIODeviceCallbackContext ctx;
            ctx.hostTimeNs = &hostTimeNs;
            const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
            delivered += static_cast<uint64_t>(blockSize) + gap;

            if (drainPerTick)
            {
                std::pair<uint64_t, uint32_t> g;
                while (tap.popGap(g)) gaps.push_back(g);
            }
        }

        tap.stop();

        if (!drainPerTick)
        {
            std::pair<uint64_t, uint32_t> g;
            while (tap.popGap(g)) gaps.push_back(g);
        }

        return { *id, gaps, tap.firstSample(), tap.framesWritten() };
    };

    TempDir rootA("gapsA16"), rootB("gapsB16");
    AudioStore storeA(rootA.dir), storeB(rootB.dir);

    const auto resultA = runOnce(storeA, true);
    REQUIRE(resultA.gaps.size() == 80);

    AudioStore::CaptureFacts factsA;
    factsA.mode = "input"; factsA.gapDetection = true;
    factsA.firstSample = resultA.firstSample; factsA.framesWritten = resultA.framesWritten;
    factsA.rate = rate; factsA.channels = 2; factsA.app = "0.1.0";
    factsA.gapsInTakeClock = resultA.gaps;
    auto finA = storeA.finalize(resultA.id, factsA);
    REQUIRE(finA.sidecarWritten);

    auto foundA = storeA.find(resultA.id);
    REQUIRE(foundA.has_value());
    REQUIRE(foundA->gaps.size() == 80);
    for (size_t i = 0; i < foundA->gaps.size(); ++i)
        REQUIRE(foundA->gaps[i].first == resultA.gaps[i].first - resultA.firstSample);

    const auto resultB = runOnce(storeB, false);
    REQUIRE(resultB.gaps.size() == 63);
}

// === 17: [audiotap][store] moving the asset folder makes resolve refuse ===

TEST_CASE("moving the asset folder makes resolve refuse", "[audiotap][store]")
{
    TempDir root("move17");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    auto gen = [](uint64_t f, int) -> int16_t { return static_cast<int16_t>(f % 60); };
    writeSynthWav(store.wavFile(*id), 48000.0, 1, 4800, gen);
    AudioStore::CaptureFacts facts;
    facts.mode = "input"; facts.firstSample = 0; facts.framesWritten = 4800;
    facts.rate = 48000.0; facts.channels = 1; facts.app = "0.1.0";
    auto fin = store.finalize(*id, facts);
    REQUIRE(fin.sidecarWritten);

    AudioRef ref = AudioStore::referencing(fin.asset, 0);
    REQUIRE(store.resolve(ref).status == AudioStore::Status::Resolved);

    const juce::File original = store.assetFolder(*id);
    const juce::File renamed = root.dir.getChildFile("renamed.adna-audio");
    REQUIRE(original.moveFileTo(renamed));

    REQUIRE(store.resolve(ref).status == AudioStore::Status::Missing);

    REQUIRE(renamed.moveFileTo(original));
    REQUIRE(store.resolve(ref).status == AudioStore::Status::Resolved);
}

// === 18: [audiotap][store] a self-stopped tap is recorded as audio ending early ===

TEST_CASE("a self-stopped tap is recorded as audio ending early", "[audiotap][store]")
{
    TempDir root("selfstop18");
    AudioStore store(root.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());

    AudioTap tap;
    tap.prepare(48000.0, 2, 512);
    REQUIRE(tap.start(store.wavFile(*id)));

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    std::vector<float> inL(512, 0.2f), inR(512, 0.2f);
    const float* ch[2] = { inL.data(), inR.data() };
    for (int b = 0; b < 5; ++b)
    {
        hostTimeNs += static_cast<uint64_t>((512.0 / 48000.0) * 1.0e9);
        juce::AudioIODeviceCallbackContext ctx;
        ctx.hostTimeNs = &hostTimeNs;
        const uint32_t gap = tap.push(ch, 2, 512, delivered, ctx);
        delivered += 512 + gap;
    }

    const uint64_t firstSample = tap.firstSample();
    REQUIRE(tap.framesWritten() == 2560);

    tap.prepare(44100.0, 2, 512);   // rate change mid-take -> stopInternal (self-stop)
    REQUIRE_FALSE(tap.isRunning());
    REQUIRE(tap.framesWritten() == 2560);

    AudioStore::CaptureFacts facts;
    facts.mode = "input";
    facts.gapDetection = tap.gapDetectionSupported();
    facts.firstSample = firstSample;
    facts.framesWritten = tap.framesWritten();
    facts.rate = 48000.0; facts.channels = 2; facts.app = "0.1.0";

    const bool endedEarly = true;   // isRunning() was false before stop(), per spec 5.1
    if (endedEarly)
        facts.unreliableFromInTakeClock = firstSample + tap.framesWritten();

    REQUIRE(*facts.unreliableFromInTakeClock == firstSample + 2560);

    tap.stop();   // no-op -- already stopped

    auto fin = store.finalize(*id, facts);
    REQUIRE(fin.sidecarWritten);
    REQUIRE(fin.asset.unreliableFrom.has_value());
    REQUIRE(*fin.asset.unreliableFrom == 2560);

    AudioRef ref = AudioStore::referencing(fin.asset, firstSample);
    REQUIRE(ref.unreliableFrom.has_value());
    REQUIRE(*ref.unreliableFrom == firstSample + 2560);

    auto res = store.resolve(ref);
    REQUIRE(res.status == AudioStore::Status::Resolved);
}
