#include "recording/AudioStore.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_cryptography/juce_cryptography.h>
#include <algorithm>
#include <memory>
#include <set>

// === AudioAsset (sidecar format v1, spec 2.1) ===

juce::var AudioAsset::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("format", juce::String("audiodna-audio"));
    obj->setProperty("version", kSidecarVersion);
    obj->setProperty("id", juce::String(id));
    obj->setProperty("fingerprint", juce::String(fingerprint));
    obj->setProperty("frames", static_cast<juce::int64>(frames));
    obj->setProperty("rate", rate);
    obj->setProperty("channels", channels);
    obj->setProperty("bits", bits);
    obj->setProperty("recordedAt", juce::String(recordedAt));
    obj->setProperty("app", juce::String(app));
    obj->setProperty("mode", juce::String(mode));
    obj->setProperty("gapDetection", gapDetection);

    juce::Array<juce::var> gapArr;
    for (const auto& [frame, n] : gaps)
    {
        auto* g = new juce::DynamicObject();
        g->setProperty("frame", static_cast<juce::int64>(frame));
        g->setProperty("n", static_cast<int>(n));
        gapArr.add(juce::var(g));
    }
    obj->setProperty("gaps", gapArr);

    obj->setProperty("unreliableFrom",
        unreliableFrom.has_value() ? juce::var(static_cast<juce::int64>(*unreliableFrom)) : juce::var());

    return juce::var(obj);
}

std::optional<AudioAsset> AudioAsset::fromVar(const juce::var& v)
{
    auto* obj = v.getDynamicObject();
    if (obj == nullptr)
        return std::nullopt;
    if (obj->getProperty("format").toString() != "audiodna-audio")
        return std::nullopt;

    AudioAsset a;
    a.id = obj->getProperty("id").toString().toStdString();
    if (a.id.empty())
        return std::nullopt;

    a.fingerprint = obj->getProperty("fingerprint").toString().toStdString();
    a.frames = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("frames")));
    a.rate = static_cast<double>(obj->getProperty("rate"));
    a.channels = static_cast<int>(obj->getProperty("channels"));
    a.bits = obj->hasProperty("bits") ? static_cast<int>(obj->getProperty("bits")) : 16;
    a.recordedAt = obj->getProperty("recordedAt").toString().toStdString();
    a.app = obj->getProperty("app").toString().toStdString();
    a.mode = obj->getProperty("mode").toString().toStdString();
    a.gapDetection = static_cast<bool>(obj->getProperty("gapDetection"));

    if (auto* gapArr = obj->getProperty("gaps").getArray())
    {
        for (const auto& gv : *gapArr)
        {
            if (auto* gObj = gv.getDynamicObject())
            {
                a.gaps.emplace_back(
                    static_cast<uint64_t>(static_cast<juce::int64>(gObj->getProperty("frame"))),
                    static_cast<uint32_t>(static_cast<int>(gObj->getProperty("n"))));
            }
        }
    }

    if (obj->hasProperty("unreliableFrom") && !obj->getProperty("unreliableFrom").isVoid())
        a.unreliableFrom = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("unreliableFrom")));

    return a;
}

// === AudioStore ===

AudioStore::AudioStore(juce::File root) : root_(std::move(root)) {}

juce::File AudioStore::defaultRoot()
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Audio-DNA").getChildFile("Audio");
}

juce::File AudioStore::assetFolder(const std::string& id) const
{
    return root_.getChildFile(juce::String(id) + kAssetSuffix);
}

juce::File AudioStore::wavFile(const std::string& id) const
{
    return assetFolder(id).getChildFile("audio.wav");
}

juce::File AudioStore::sidecarFile(const std::string& id) const
{
    return assetFolder(id).getChildFile("audio.json");
}

std::optional<std::string> AudioStore::beginAsset()
{
    if (!root_.exists() && !root_.createDirectory())
        return std::nullopt;

    // D-A1: re-mint up to 3 times if the folder already exists (bounded --
    // Uuid's entropy is <= 48 bits, adequate for a per-user store of
    // hundreds of assets, but a collision is not impossible).
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        const std::string id = juce::Uuid().toString().toStdString();
        juce::File folder = assetFolder(id);
        if (folder.exists())
            continue;
        if (!folder.createDirectory())
            continue;
        activeAssetId_ = id;
        return id;
    }
    return std::nullopt;
}

std::optional<std::string> AudioStore::activeAssetId() const
{
    return activeAssetId_;
}

bool AudioStore::abandonAsset(const std::string& id)
{
    // D-A13: refuse unless this instance minted `id` (still active) AND it
    // was never finalized (no sidecar).
    if (!activeAssetId_.has_value() || *activeAssetId_ != id)
        return false;
    if (sidecarFile(id).existsAsFile())
        return false;

    const bool ok = assetFolder(id).deleteRecursively();
    activeAssetId_.reset();
    return ok;
}

std::optional<AudioStore::CaptureFacts> AudioStore::CaptureFacts::fromAudioRef(const AudioRef& ref, std::string app)
{
    if (ref.segments.size() != 1)
        return std::nullopt;

    const auto& seg = ref.segments[0];
    CaptureFacts facts;
    facts.mode = ref.mode;
    facts.gapDetection = ref.gapDetection;
    facts.firstSample = seg.firstSample;
    facts.framesWritten = seg.frames;
    facts.rate = seg.rate;
    facts.channels = seg.channels;
    facts.gapsInTakeClock = ref.gaps;
    facts.unreliableFromInTakeClock = ref.unreliableFrom;
    facts.app = std::move(app);
    return facts;
}

namespace
{
    // D-A9: sample (take-clock) -> asset-frame, saturating at 0 if the
    // stamp somehow predates firstSample.
    uint64_t toAssetFrame(uint64_t sample, uint64_t firstSample)
    {
        return sample >= firstSample ? sample - firstSample : 0;
    }
}

AudioStore::FinalizeResult AudioStore::finalize(const std::string& id, const CaptureFacts& facts)
{
    FinalizeResult result;
    AudioAsset asset;
    asset.id = id;
    asset.recordedAt = juce::Time::getCurrentTime().toISO8601(true).toStdString();
    asset.app = facts.app;
    asset.mode = facts.mode;
    asset.gapDetection = facts.gapDetection;

    for (const auto& [sample, n] : facts.gapsInTakeClock)
        asset.gaps.emplace_back(toAssetFrame(sample, facts.firstSample), n);
    if (facts.unreliableFromInTakeClock.has_value())
        asset.unreliableFrom = toAssetFrame(*facts.unreliableFromInTakeClock, facts.firstSample);

    const juce::File wav = wavFile(id);
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(wav), true));

    if (reader != nullptr)
    {
        result.wavReadable = true;
        asset.frames = static_cast<uint64_t>(reader->lengthInSamples);
        asset.rate = reader->sampleRate;
        asset.channels = static_cast<int>(reader->numChannels);
        asset.bits = static_cast<int>(reader->bitsPerSample);

        // The `:453`/`:467` "give up rather than spin" branches inside
        // AudioTap::stopInternal leave frames unwritten without setting
        // unreliableFrom -- catch it here, one rule for every such path
        // (spec 4.1, "Tradeoffs considered").
        if (asset.frames < facts.framesWritten)
        {
            const uint64_t truncatedAt = asset.frames;
            asset.unreliableFrom = asset.unreliableFrom.has_value()
                ? std::min(*asset.unreliableFrom, truncatedAt) : truncatedAt;
            result.error = "asset " + id + ": truncated (header " + std::to_string(asset.frames)
                + " frames < framesWritten " + std::to_string(facts.framesWritten) + ")";
        }

        if (const auto fp = fingerprint(wav); fp.has_value())
        {
            asset.fingerprint = *fp;
        }
        else
        {
            if (!result.error.empty()) result.error += "; ";
            result.error += "asset " + id + ": wav could not be fingerprinted (floating-point or open failure)";
        }
    }
    else
    {
        // D-A11: the take ALWAYS carries the reference -- build the asset
        // from the capture facts, fingerprint "" (never written to a
        // sidecar).
        result.wavReadable = false;
        asset.fingerprint.clear();
        asset.frames = facts.framesWritten;
        asset.rate = facts.rate;
        asset.channels = facts.channels;
        asset.bits = 16;
        result.error = "asset " + id + ": wav unreadable";
    }

    // D-A3/D-A11: NEVER write a sidecar with an empty fingerprint.
    if (!asset.fingerprint.empty())
    {
        const bool wrote = sidecarFile(id).replaceWithText(juce::JSON::toString(asset.toVar()));
        result.sidecarWritten = wrote;
        if (!wrote)
        {
            if (!result.error.empty()) result.error += "; ";
            result.error += "asset " + id + ": sidecar not written";
        }
    }
    else
    {
        result.sidecarWritten = false;
    }

    result.asset = asset;

    if (activeAssetId_.has_value() && *activeAssetId_ == id)
        activeAssetId_.reset();

    return result;
}

std::optional<AudioAsset> AudioStore::find(const std::string& id) const
{
    const juce::File sidecar = sidecarFile(id);
    if (!sidecar.existsAsFile())
        return std::nullopt;

    auto asset = AudioAsset::fromVar(juce::JSON::parse(sidecar.loadFileAsString()));
    if (!asset.has_value() || asset->id != id)
        return std::nullopt;
    return asset;
}

bool AudioStore::isIncomplete(const std::string& id) const
{
    if (activeAssetId_.has_value() && *activeAssetId_ == id)
        return false;
    return wavFile(id).existsAsFile() && !sidecarFile(id).existsAsFile();
}

namespace
{
    // Strips kAssetSuffix off a folder name; empty if it doesn't match.
    std::string idFromAssetFolderName(const juce::File& dir)
    {
        const std::string name = dir.getFileName().toStdString();
        const std::string suffix = AudioStore::kAssetSuffix;
        if (name.size() <= suffix.size())
            return {};
        if (name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0)
            return {};
        return name.substr(0, name.size() - suffix.size());
    }
}

std::vector<std::string> AudioStore::listAssetIds() const
{
    std::vector<std::string> out;
    if (!root_.exists())
        return out;
    for (const auto& dir : root_.findChildFiles(juce::File::findDirectories, false, juce::String("*") + kAssetSuffix))
    {
        const auto id = idFromAssetFolderName(dir);
        if (!id.empty() && find(id).has_value())
            out.push_back(id);
    }
    return out;
}

std::vector<std::string> AudioStore::listIncompleteAssetIds() const
{
    std::vector<std::string> out;
    if (!root_.exists())
        return out;
    for (const auto& dir : root_.findChildFiles(juce::File::findDirectories, false, juce::String("*") + kAssetSuffix))
    {
        const auto id = idFromAssetFolderName(dir);
        if (!id.empty() && isIncomplete(id))
            out.push_back(id);
    }
    return out;
}

AudioStore::Resolution AudioStore::resolve(const AudioRef& ref) const
{
    Resolution res;

    // (1)
    if (ref.segments.empty())
    {
        if (ref.mode.empty())
        {
            res.status = Status::NoAudio;
            return res;
        }
        res.status = Status::Mismatch;
        res.reason = "audio mode '" + ref.mode + "' but no segment -- malformed take";
        return res;
    }

    // (2)
    if (ref.segments.size() > 1)
    {
        res.status = Status::MultiSegment;
        res.reason = "multi-segment audio is not built; re-record";
        return res;
    }

    const auto& seg = ref.segments[0];

    // (3)
    if (seg.id.empty())
    {
        if (!seg.file.empty())
        {
            res.status = Status::Legacy;
            res.reason = "recorded in-folder by a pre-v3 build; re-record";
        }
        else
        {
            res.status = Status::Mismatch;
            res.reason = "no id";
        }
        return res;
    }

    // (4)
    const juce::File folder = assetFolder(seg.id);
    const juce::File wav = wavFile(seg.id);
    if (!folder.exists() || !wav.existsAsFile())
    {
        res.status = Status::Missing;
        res.reason = "asset " + seg.id + " not found under " + root_.getFullPathName().toStdString();
        return res;
    }

    const juce::File sidecar = sidecarFile(seg.id);
    if (!sidecar.existsAsFile())
    {
        res.status = Status::Incomplete;
        res.reason = "asset " + seg.id + " has no sidecar -- repair from this take: "
            "finalize(id, CaptureFacts::fromAudioRef)";
        return res;
    }

    const auto sidecarAsset = AudioAsset::fromVar(juce::JSON::parse(sidecar.loadFileAsString()));
    if (!sidecarAsset.has_value() || sidecarAsset->id != seg.id)
    {
        res.status = Status::Mismatch;
        res.reason = "asset " + seg.id + ": sidecar is unparseable, or its id does not match the folder";
        return res;
    }

    // (5)
    if (sidecarAsset->frames != seg.frames || sidecarAsset->rate != seg.rate || sidecarAsset->channels != seg.channels)
    {
        res.status = Status::Mismatch;
        res.reason = "asset " + seg.id + ": sidecar {frames=" + std::to_string(sidecarAsset->frames)
            + ", rate=" + std::to_string(sidecarAsset->rate) + ", channels=" + std::to_string(sidecarAsset->channels)
            + "} != take segment {frames=" + std::to_string(seg.frames) + ", rate=" + std::to_string(seg.rate)
            + ", channels=" + std::to_string(seg.channels) + "}";
        return res;
    }

    // (6)
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(wav), true));
    if (reader == nullptr)
    {
        res.status = Status::Mismatch;
        res.reason = "asset " + seg.id + ": wav could not be opened for verification";
        return res;
    }
    if (static_cast<uint64_t>(reader->lengthInSamples) != sidecarAsset->frames
        || reader->sampleRate != sidecarAsset->rate
        || static_cast<int>(reader->numChannels) != sidecarAsset->channels
        || static_cast<int>(reader->bitsPerSample) != sidecarAsset->bits)
    {
        res.status = Status::Mismatch;
        res.reason = "asset " + seg.id + ": wav header does not match its own sidecar";
        return res;
    }

    // (7)
    if (seg.fingerprint.empty())
    {
        res.status = Status::ResolvedUnverified;
        res.reason = "this take was saved before its audio was fingerprinted; the store's is "
            + sidecarAsset->fingerprint + "; verified by length/rate/channels only";
        res.wav = wav;
        res.asset = *sidecarAsset;
        res.firstSample = seg.firstSample;
        return res;
    }

    const auto liveFp = fingerprint(wav);
    if (!liveFp.has_value() || *liveFp != seg.fingerprint)
    {
        res.status = Status::Mismatch;
        res.reason = "asset " + seg.id + ": content differs from what this take was recorded against";
        return res;
    }
    if (*liveFp != sidecarAsset->fingerprint)
    {
        res.status = Status::Mismatch;
        res.reason = "asset " + seg.id + ": the store's sidecar disagrees with its own file";
        return res;
    }

    // (8)
    res.status = Status::Resolved;
    res.wav = wav;
    res.asset = *sidecarAsset;
    res.firstSample = seg.firstSample;
    return res;
}

AudioRef AudioStore::referencing(const AudioAsset& asset, uint64_t firstSample)
{
    AudioRef ref;
    ref.mode = asset.mode;
    ref.gapDetection = asset.gapDetection;

    AudioRef::Segment seg;
    seg.id = asset.id;
    seg.fingerprint = asset.fingerprint;
    seg.firstSample = firstSample;
    seg.frames = asset.frames;
    seg.rate = asset.rate;
    seg.channels = asset.channels;
    ref.segments.push_back(seg);

    for (const auto& [frame, n] : asset.gaps)
        ref.gaps.emplace_back(frame + firstSample, n);
    if (asset.unreliableFrom.has_value())
        ref.unreliableFrom = *asset.unreliableFrom + firstSample;

    return ref;
}

std::optional<std::string> AudioStore::fingerprint(const juce::File& wav)
{
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(wav), true));
    if (reader == nullptr)
        return std::nullopt;
    if (reader->usesFloatingPointData)
        return std::nullopt;

    const uint64_t N = static_cast<uint64_t>(reader->lengthInSamples);
    const int R = juce::roundToInt(reader->sampleRate);
    const int C = static_cast<int>(reader->numChannels);
    const int B = static_cast<int>(reader->bitsPerSample);
    const uint64_t W = kFingerprintWindowFrames;

    const juce::String header = "adna-fp1|frames=" + juce::String(static_cast<juce::int64>(N))
        + "|rate=" + juce::String(R) + "|channels=" + juce::String(C) + "|bits=" + juce::String(B) + "|";

    juce::MemoryBlock bytes;
    bytes.append(header.toRawUTF8(), header.getNumBytesAsUTF8());

    auto appendWindow = [&](uint64_t start, uint64_t count)
    {
        if (count == 0 || C <= 0)
            return;
        std::vector<std::vector<int>> chans(static_cast<size_t>(C), std::vector<int>(static_cast<size_t>(count), 0));
        std::vector<int*> ptrs(static_cast<size_t>(C));
        for (int c = 0; c < C; ++c)
            ptrs[static_cast<size_t>(c)] = chans[static_cast<size_t>(c)].data();
        reader->read(ptrs.data(), C, static_cast<juce::int64>(start), static_cast<int>(count), false);

        for (uint64_t f = 0; f < count; ++f)
        {
            for (int c = 0; c < C; ++c)
            {
                const int32_t sample = chans[static_cast<size_t>(c)][static_cast<size_t>(f)];
                const int16_t v = static_cast<int16_t>(sample >> 16);
                const uint16_t uv = static_cast<uint16_t>(v);
                const uint8_t le[2] = { static_cast<uint8_t>(uv & 0xFF), static_cast<uint8_t>((uv >> 8) & 0xFF) };
                bytes.append(le, 2);
            }
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

AudioStore::ReferenceScan AudioStore::scanTakes(const juce::File& takesRoot)
{
    ReferenceScan scan;
    scan.roots.push_back(takesRoot);

    std::set<std::string> ids;
    for (const auto& dir : takesRoot.findChildFiles(juce::File::findDirectories, true, "*.adna-take"))
    {
        const juce::File target = dir.getChildFile("take.json");
        if (!target.existsAsFile())
        {
            scan.unreadableTakes.push_back(target);
            continue;
        }

        // Keep `parsed` ALIVE for the rest of this iteration -- it owns the
        // reference-counted DynamicObject chain below; a raw pointer taken
        // from a temporary var (e.g. JSON::parse(...).getDynamicObject())
        // would dangle the instant this statement ends.
        const juce::var parsed = juce::JSON::parse(target.loadFileAsString());
        auto* obj = parsed.getDynamicObject();
        if (obj == nullptr)
        {
            scan.unreadableTakes.push_back(target);
            continue;
        }

        auto* audioObj = obj->getProperty("audio").getDynamicObject();
        auto* segArr = audioObj != nullptr ? audioObj->getProperty("segments").getArray() : nullptr;
        if (segArr != nullptr)
        {
            for (const auto& sv : *segArr)
            {
                if (auto* sObj = sv.getDynamicObject())
                {
                    const auto id = sObj->getProperty("id").toString().toStdString();
                    if (!id.empty())
                        ids.insert(id);
                }
            }
        }
    }

    scan.referencedIds.assign(ids.begin(), ids.end());
    return scan;
}

std::optional<std::vector<std::string>> AudioStore::unreferencedAssetIds(const juce::File& takesRoot) const
{
    const auto scan = scanTakes(takesRoot);
    if (!scan.unreadableTakes.empty())
        return std::nullopt;

    const std::set<std::string> referenced(scan.referencedIds.begin(), scan.referencedIds.end());
    std::vector<std::string> out;
    for (const auto& id : listAssetIds())
        if (referenced.count(id) == 0)
            out.push_back(id);
    return out;
}
