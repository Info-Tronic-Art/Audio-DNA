#include "recording/Take.h"
#include <algorithm>
#include <set>

// === AudioRef ===

juce::var AudioRef::Segment::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("file", juce::String(file));
    obj->setProperty("firstSample", static_cast<juce::int64>(firstSample));
    obj->setProperty("frames", static_cast<juce::int64>(frames));
    obj->setProperty("rate", rate);
    obj->setProperty("channels", channels);
    obj->setProperty("sha1Head", juce::String(sha1Head));
    return juce::var(obj);
}

AudioRef::Segment AudioRef::Segment::fromVar(const juce::var& v)
{
    Segment s;
    if (auto* obj = v.getDynamicObject())
    {
        s.file = obj->getProperty("file").toString().toStdString();
        s.firstSample = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("firstSample")));
        s.frames = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("frames")));
        s.rate = static_cast<double>(obj->getProperty("rate"));
        s.channels = static_cast<int>(obj->getProperty("channels"));
        s.sha1Head = obj->getProperty("sha1Head").toString().toStdString();
    }
    return s;
}

juce::var AudioRef::toVar() const
{
    auto* obj = new juce::DynamicObject();

    juce::Array<juce::var> segArr;
    for (const auto& s : segments) segArr.add(s.toVar());
    obj->setProperty("segments", segArr);

    obj->setProperty("mode", juce::String(mode));
    obj->setProperty("gapDetection", gapDetection);

    juce::Array<juce::var> gapArr;
    for (const auto& [sample, n] : gaps)
    {
        auto* g = new juce::DynamicObject();
        g->setProperty("sample", static_cast<juce::int64>(sample));
        g->setProperty("n", static_cast<int>(n));
        gapArr.add(juce::var(g));
    }
    obj->setProperty("gaps", gapArr);

    obj->setProperty("unreliableFrom",
        unreliableFrom.has_value() ? juce::var(static_cast<juce::int64>(*unreliableFrom)) : juce::var());

    return juce::var(obj);
}

AudioRef AudioRef::fromVar(const juce::var& v)
{
    AudioRef a;
    if (auto* obj = v.getDynamicObject())
    {
        if (auto* segArr = obj->getProperty("segments").getArray())
            for (const auto& sv : *segArr)
                a.segments.push_back(Segment::fromVar(sv));

        a.mode = obj->getProperty("mode").toString().toStdString();
        a.gapDetection = static_cast<bool>(obj->getProperty("gapDetection"));

        if (auto* gapArr = obj->getProperty("gaps").getArray())
        {
            for (const auto& gv : *gapArr)
            {
                if (auto* gObj = gv.getDynamicObject())
                {
                    a.gaps.emplace_back(
                        static_cast<uint64_t>(static_cast<juce::int64>(gObj->getProperty("sample"))),
                        static_cast<uint32_t>(static_cast<int>(gObj->getProperty("n"))));
                }
            }
        }

        if (obj->hasProperty("unreliableFrom") && !obj->getProperty("unreliableFrom").isVoid())
            a.unreliableFrom = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("unreliableFrom")));
    }
    return a;
}

// === Meta ===

juce::var Meta::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("recordedAt", juce::String(recordedAt));
    obj->setProperty("app", juce::String(app));
    obj->setProperty("duration", duration);
    obj->setProperty("durationBeats", durationBeats);
    return juce::var(obj);
}

Meta Meta::fromVar(const juce::var& v)
{
    Meta m;
    if (auto* obj = v.getDynamicObject())
    {
        m.recordedAt = obj->getProperty("recordedAt").toString().toStdString();
        m.app = obj->getProperty("app").toString().toStdString();
        m.duration = static_cast<double>(obj->getProperty("duration"));
        m.durationBeats = static_cast<double>(obj->getProperty("durationBeats"));
    }
    return m;
}

// === Take ===

namespace
{
    const std::set<std::string>& knownTopLevelSections()
    {
        static const std::set<std::string> s = {
            "format", "version", "minReader", "features", "meta", "audio",
            "tempoMap", "checkpoint0", "checkpointEnd", "markers", "lanes"
        };
        return s;
    }

    const std::set<std::string>& knownFeatures()
    {
        static const std::set<std::string> s = {
            "lanes", "tempoMap", "checkpoint0", "audio", "markers", "wallOnly"
        };
        return s;
    }
}

juce::var Take::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("format", juce::String("audiodna-take"));
    obj->setProperty("version", kFormatVersion);
    obj->setProperty("minReader", kMinReader);

    juce::Array<juce::var> featArr;
    featArr.add(juce::String("lanes"));
    featArr.add(juce::String("tempoMap"));
    featArr.add(juce::String("checkpoint0"));
    if (!audio.mode.empty()) featArr.add(juce::String("audio"));
    if (!markers.empty())    featArr.add(juce::String("markers"));
    for (const auto& f : unknownFeatures) featArr.add(juce::String(f));
    obj->setProperty("features", featArr);

    obj->setProperty("meta", meta.toVar());
    obj->setProperty("audio", audio.toVar());
    obj->setProperty("tempoMap", tempo.toVar());
    obj->setProperty("checkpoint0", checkpoint0.toVar());
    obj->setProperty("checkpointEnd", checkpointEnd.toVar());

    juce::Array<juce::var> markerArr;
    for (const auto& m : markers) markerArr.add(m.toVar());
    obj->setProperty("markers", markerArr);

    juce::Array<juce::var> laneArr;
    for (const auto& [key, lane] : lanes)
        laneArr.add(lane.toVar());
    obj->setProperty("lanes", laneArr);

    // D12 rule 3: whatever this reader didn't understand on load is
    // re-emitted verbatim, unexamined.
    for (const auto& [name, v] : unknownTopLevel)
        obj->setProperty(juce::Identifier(juce::String(name)), v);

    return juce::var(obj);
}

std::optional<Take> Take::fromVar(const juce::var& root, LoadStats& stats)
{
    auto* obj = root.getDynamicObject();
    if (!obj)
    {
        stats.refused = true;
        stats.refusalReason = "take.json is not an object";
        return std::nullopt;
    }

    // Legacy v1 detection (G1): no "format" key, bare {"version":1,"events":[...]}.
    if (!obj->hasProperty("format") && static_cast<int>(obj->getProperty("version")) == 1)
        return fromV1Var(root, stats);

    const int minReader = obj->hasProperty("minReader")
        ? static_cast<int>(obj->getProperty("minReader")) : 1;
    if (minReader > kFormatVersion)
    {
        stats.refused = true;
        stats.refusalReason = "minReader " + std::to_string(minReader)
            + " > this reader's " + std::to_string(kFormatVersion);
        return std::nullopt;
    }

    Take take;

    for (const auto& nv : obj->getProperties())
    {
        const std::string name = nv.name.toString().toStdString();
        if (knownTopLevelSections().count(name) == 0)
        {
            take.unknownTopLevel[name] = nv.value;
            stats.unknownTopLevelSections.push_back(name);
        }
    }

    if (auto* featArr = obj->getProperty("features").getArray())
    {
        for (const auto& fv : *featArr)
        {
            const auto f = fv.toString().toStdString();
            if (knownFeatures().count(f) == 0)
            {
                take.unknownFeatures.push_back(f);
                stats.unknownFeatures.push_back(f);
            }
        }
    }

    take.meta = Meta::fromVar(obj->getProperty("meta"));
    take.audio = AudioRef::fromVar(obj->getProperty("audio"));
    take.tempo = TempoMap::fromVar(obj->getProperty("tempoMap"));
    take.checkpoint0 = PerfState::fromVar(obj->getProperty("checkpoint0"));
    take.checkpointEnd = PerfState::fromVar(obj->getProperty("checkpointEnd"));

    if (auto* markerArr = obj->getProperty("markers").getArray())
        for (const auto& mv : *markerArr)
            take.markers.push_back(DiscretePoint::fromVar(mv));

    uint64_t maxSeq = 0;
    if (auto* laneArr = obj->getProperty("lanes").getArray())
    {
        for (const auto& lv : *laneArr)
        {
            std::string unknownKind;
            Lane lane = Lane::fromVar(lv, &unknownKind);
            if (lane.kind == Lane::Kind::Opaque)
            {
                stats.unknownKindLanes++;
                if (!unknownKind.empty())
                    stats.unknownKindNames.push_back(unknownKind);
            }
            for (const auto& p : lane.points)
                maxSeq = std::max(maxSeq, p.s.seq);
            for (const auto& g : lane.gestures)
                for (const auto& st : g.stamps)
                    maxSeq = std::max(maxSeq, st.seq);

            take.lanes[lane.key] = std::move(lane);
        }
    }
    take.nextSeq = maxSeq + 1;

    return take;
}

bool Take::save(const juce::File& folder) const
{
    if (!folder.exists())
        folder.createDirectory();
    auto target = folder.getChildFile("take.json");
    return target.replaceWithText(juce::JSON::toString(toVar()));
}

std::optional<Take> Take::load(const juce::File& folder, LoadStats& stats)
{
    // A directory means the real `.adna-take/take.json` contract (D10); a
    // bare file lets tests point straight at tests/fixtures/*.json.
    juce::File target = folder.isDirectory() ? folder.getChildFile("take.json") : folder;

    if (!target.existsAsFile())
    {
        stats.refused = true;
        stats.refusalReason = "file not found: " + target.getFullPathName().toStdString();
        return std::nullopt;
    }

    const auto text = target.loadFileAsString();
    const auto parsed = juce::JSON::parse(text);
    if (parsed.isVoid())
    {
        stats.refused = true;
        stats.refusalReason = "invalid JSON in " + target.getFullPathName().toStdString();
        return std::nullopt;
    }

    return fromVar(parsed, stats);
}

// === v1 -> v2 bridge (D12 rule 5, G1's SessionRecorder shape) ===

std::optional<Take> Take::fromV1Var(const juce::var& root, LoadStats& stats)
{
    stats.wasV1 = true;
    Take take;
    take.unknownFeatures.push_back("wallOnly");   // beat/sample are absent -- t only (D12 rule 5)

    auto* obj = root.getDynamicObject();
    auto* eventArray = obj ? obj->getProperty("events").getArray() : nullptr;
    if (!eventArray)
        return take;   // an empty-but-valid v1 file is not an error

    enum class V1Type : int
    {
        ParameterChange = 0, ClipTrigger = 1, ColumnTrigger = 2, MacroChange = 3,
        TransportChange = 4, EffectToggle = 5, CuepointJump = 6
    };

    for (const auto& ev : *eventArray)
    {
        auto* e = ev.getDynamicObject();
        if (!e) continue;

        const double t = static_cast<double>(e->getProperty("t"));
        const auto type = static_cast<V1Type>(static_cast<int>(e->getProperty("type")));
        const uint64_t seq = take.nextSeq++;

        ControlPath key;
        DiscretePoint p;
        p.s = { seq, t, 0 };
        p.beat = 0.0;   // wallOnly: no tempo/phase data in a v1 file
        p.bpm = 0.0f;
        p.origin = Origin::Human;

        switch (type)
        {
            case V1Type::ClipTrigger:
                // Faithful conversion (D12 rule 5): the common real-world v1
                // shape ("clip-trigger-only sessions", July spec).
                key.scope = ControlPath::Scope::Layer;
                key.deckRelative = true;
                key.layer = static_cast<int>(e->getProperty("layer"));
                key.control = "activeClip";
                p.v = static_cast<int>(e->getProperty("column"));
                break;

            case V1Type::ColumnTrigger:
                // Best-effort (D12 rule 5): a real per-layer expansion needs
                // a Composition (how many layers, which ignore column
                // triggers) that Take::load is never given. Recorded as one
                // Comp-scope marker point carrying the column so the data
                // is not lost; step 3's live capture replaces this with
                // proper per-layer activeClip points going forward.
                key.scope = ControlPath::Scope::Comp;
                key.control = "columnTrigger";
                p.v = static_cast<int>(e->getProperty("column"));
                break;

            case V1Type::ParameterChange:
                // Best-effort: targetId was overloaded ("clip ID or layer
                // index", the old model) and is not reliably resolvable
                // here. Left unresolved-by-position on purpose (D2: no
                // path silently misfires) rather than guessed at.
                key.scope = ControlPath::Scope::Clip;
                key.control = "param";
                key.param = static_cast<int>(e->getProperty("paramIndex"));
                p.v = static_cast<int>(static_cast<double>(e->getProperty("value")) * 1000.0);
                break;

            case V1Type::MacroChange:
                key.scope = ControlPath::Scope::Macro;
                key.control = "macro";
                key.macro = static_cast<int>(e->getProperty("macroIndex"));
                p.v = static_cast<int>(static_cast<double>(e->getProperty("value")) * 1000.0);
                break;

            case V1Type::TransportChange:
                key.scope = ControlPath::Scope::Comp;
                key.control = "audio";
                p.action = e->getProperty("action").toString().toStdString();
                p.v = static_cast<int>(static_cast<double>(e->getProperty("value")) * 1000.0);
                break;

            case V1Type::EffectToggle:
                key.scope = ControlPath::Scope::Clip;
                key.control = "enable";
                key.fxName = e->getProperty("effect").toString().toStdString();
                p.v = static_cast<bool>(e->getProperty("enabled")) ? 1 : 0;
                break;

            case V1Type::CuepointJump:
                key.scope = ControlPath::Scope::Clip;
                key.control = "cue";
                p.v = static_cast<int>(e->getProperty("cuepointIndex"));
                break;
        }

        auto& lane = take.lanes[key];
        lane.key = key;
        lane.kind = Lane::Kind::Discrete;
        lane.points.push_back(std::move(p));
    }

    return take;
}

// === Derived views / edit ops (D3/D5, NOW subset) ===

std::vector<Take::ChronoEntry> Take::chronological() const
{
    std::vector<ChronoEntry> out;
    for (const auto& [key, lane] : lanes)
    {
        if (lane.kind == Lane::Kind::Discrete)
        {
            for (const auto& p : lane.points)
                out.push_back({ p.s.t, p.s.seq, key, "point" });
        }
        else if (lane.kind == Lane::Kind::Continuous)
        {
            for (const auto& g : lane.gestures)
            {
                if (g.stamps.empty()) continue;
                out.push_back({ g.stamps.front().t, g.stamps.front().seq, key, "touch" });
                out.push_back({ g.stamps.back().t, g.stamps.back().seq, key, "release" });
            }
        }
        // Opaque lanes contribute nothing -- their content is unparsed.
    }
    std::sort(out.begin(), out.end(), [](const ChronoEntry& a, const ChronoEntry& b) {
        if (a.t != b.t) return a.t < b.t;
        return a.seq < b.seq;
    });
    return out;
}

void Take::deletePoints(std::span<const uint64_t> seqs)
{
    if (seqs.empty()) return;
    const std::set<uint64_t> toDelete(seqs.begin(), seqs.end());
    for (auto& [key, lane] : lanes)
    {
        if (lane.kind != Lane::Kind::Discrete) continue;
        lane.points.erase(
            std::remove_if(lane.points.begin(), lane.points.end(),
                [&](const DiscretePoint& p) { return toDelete.count(p.s.seq) != 0; }),
            lane.points.end());
    }
}

void Take::deleteLane(const ControlPath& key)
{
    lanes.erase(key);
}
