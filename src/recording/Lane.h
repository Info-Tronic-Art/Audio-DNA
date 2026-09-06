#pragma once
#include "model/ControlPath.h"
#include "connect/AutomationCurve.h"
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include <cstdint>

// Lane.h -- s167 D3: a take is a map<ControlPath, Lane>, not a flat log. A
// Lane is a sorted vector of points (discrete) or gestures (continuous);
// every point/gesture carries all three clocks from ONE RecorderClock
// (D1). Header-only by design (this packet's TARGET FILES list Lane.h with
// no .cpp) -- the existing repo already keeps comparable model complexity
// header-only (Composition.h, Deck.h).

// Stamp: the three-clock reading D1 requires on every point. `seq` is the
// global capture sequence -- monotonic across the whole take, minted at
// capture, unique; it IS the point's id (never a pointer -- R1).
struct Stamp
{
    uint64_t seq = 0;
    double t = 0.0;
    uint64_t sample = 0;

    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("seq", static_cast<juce::int64>(seq));
        obj->setProperty("t", t);
        obj->setProperty("sample", static_cast<juce::int64>(sample));
        return juce::var(obj);
    }

    static Stamp fromVar(const juce::var& v)
    {
        Stamp s;
        if (auto* obj = v.getDynamicObject())
        {
            s.seq = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("seq")));
            s.t = static_cast<double>(obj->getProperty("t"));
            s.sample = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("sample")));
        }
        return s;
    }
};

// D6's Origin rule: the recorder records Human and Engine; Replay is never
// recorded; Routine records the trigger AND its expanded (`via`-tagged)
// children; Preamble is a slice-time synthesis (LATER, D4).
enum class Origin : uint8_t { Human, Replay, Routine, Engine, Preamble };

inline const char* originToString(Origin o)
{
    switch (o)
    {
        case Origin::Human:    return "human";
        case Origin::Replay:   return "replay";
        case Origin::Routine:  return "routine";
        case Origin::Engine:   return "engine";
        case Origin::Preamble: return "preamble";
    }
    return "human";
}

inline Origin originFromString(const juce::String& s)
{
    if (s == "replay")   return Origin::Replay;
    if (s == "routine")  return Origin::Routine;
    if (s == "engine")   return Origin::Engine;
    if (s == "preamble") return Origin::Preamble;
    return Origin::Human;   // default/unknown -> Human (never silently "replay", D12)
}

// DiscretePoint: one instantaneous command that leaves the control in
// state `v` (or `action` + `v` for action-valued controls like tempo/audio,
// D3). `raw` carries the point's original var on load -- reserved for a
// future field this reader predates (D12 rule 3); nothing sets it back
// into a KNOWN point's re-save today because no lossy field exists yet.
struct DiscretePoint
{
    Stamp s;
    double beat = 0.0;
    float bpm = 0.0f;
    Origin origin = Origin::Human;
    int v = 0;
    std::string action;
    bool retrigger = false;
    uint64_t group = 0;
    uint64_t via = 0;
    juce::var raw;

    juce::var toVar() const
    {
        juce::var sv = s.toVar();                   // keep alive -- obj points into it
        auto* obj = sv.getDynamicObject();           // seq/t/sample already on it
        obj->setProperty("beat", beat);
        obj->setProperty("bpm", static_cast<double>(bpm));
        obj->setProperty("origin", juce::String(originToString(origin)));
        obj->setProperty("v", v);
        if (!action.empty())   obj->setProperty("action", juce::String(action));
        if (retrigger)         obj->setProperty("retrigger", true);
        if (group != 0)        obj->setProperty("group", static_cast<juce::int64>(group));
        if (via != 0)          obj->setProperty("via", static_cast<juce::int64>(via));
        return sv;
    }

    static DiscretePoint fromVar(const juce::var& v)
    {
        DiscretePoint p;
        p.raw = v;
        p.s = Stamp::fromVar(v);
        if (auto* obj = v.getDynamicObject())
        {
            p.beat = static_cast<double>(obj->getProperty("beat"));
            p.bpm = static_cast<float>(static_cast<double>(obj->getProperty("bpm")));
            p.origin = originFromString(obj->getProperty("origin").toString());
            p.v = static_cast<int>(obj->getProperty("v"));
            if (obj->hasProperty("action"))    p.action = obj->getProperty("action").toString().toStdString();
            if (obj->hasProperty("retrigger")) p.retrigger = static_cast<bool>(obj->getProperty("retrigger"));
            if (obj->hasProperty("group"))     p.group = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("group")));
            if (obj->hasProperty("via"))       p.via = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("via")));
        }
        return p;
    }
};

// Gesture: begin..end on a continuous lane. `curve.pts[i].x` is the BEAT
// offset within the take (D7's closing note); `stamps[i]` is the parallel
// {seq,t,sample} reading for that same breakpoint, so the three-clock rule
// holds per point without bloating the shared curve type.
struct Gesture
{
    std::string grip;              // "held" | "decaying" (D3)
    AutomationCurve curve;
    std::vector<Stamp> stamps;     // stamps.size() == curve.pts.size()
    float bpmAtBegin = 0.0f;
    Origin origin = Origin::Human;
    uint64_t via = 0;

    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("grip", juce::String(grip));
        obj->setProperty("curve", curve.toVar());

        juce::Array<juce::var> stampArr;
        for (const auto& st : stamps)
            stampArr.add(st.toVar());
        obj->setProperty("stamps", stampArr);

        obj->setProperty("bpmAtBegin", static_cast<double>(bpmAtBegin));
        obj->setProperty("origin", juce::String(originToString(origin)));
        if (via != 0) obj->setProperty("via", static_cast<juce::int64>(via));
        return juce::var(obj);
    }

    static Gesture fromVar(const juce::var& v)
    {
        Gesture g;
        if (auto* obj = v.getDynamicObject())
        {
            g.grip = obj->getProperty("grip").toString().toStdString();
            g.curve = AutomationCurve::fromVar(obj->getProperty("curve"));
            if (auto* stampArr = obj->getProperty("stamps").getArray())
                for (const auto& sv : *stampArr)
                    g.stamps.push_back(Stamp::fromVar(sv));
            g.bpmAtBegin = static_cast<float>(static_cast<double>(obj->getProperty("bpmAtBegin")));
            g.origin = originFromString(obj->getProperty("origin").toString());
            if (obj->hasProperty("via"))
                g.via = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("via")));
        }
        return g;
    }
};

// Lane: one control's whole timeline (D3). `Opaque` is this format's own
// escape hatch for a `kind` string a future reader invents that this one
// cannot interpret as either points or gestures (D12 rule 3, "ADD, never
// REDEFINE") -- `raw` then holds the WHOLE original lane var and is
// re-emitted verbatim on save, key included, unparsed.
struct Lane
{
    enum class Kind : uint8_t { Continuous, Discrete, Opaque };

    ControlPath key;
    Kind kind = Kind::Discrete;
    std::vector<DiscretePoint> points;    // kind == Discrete
    std::vector<Gesture> gestures;        // kind == Continuous
    juce::var raw;                        // kind == Opaque: the whole lane, verbatim

    static const char* kindToString(Kind k)
    {
        switch (k)
        {
            case Kind::Continuous: return "continuous";
            case Kind::Discrete:   return "discrete";
            case Kind::Opaque:     return "opaque";
        }
        return "discrete";
    }

    juce::var toVar() const
    {
        if (kind == Kind::Opaque)
            return raw;   // byte-preserved passthrough (D12 rule 3)

        auto* obj = new juce::DynamicObject();
        obj->setProperty("key", key.toVar());
        obj->setProperty("kind", juce::String(kindToString(kind)));

        if (kind == Kind::Discrete)
        {
            juce::Array<juce::var> pts;
            for (const auto& p : points)
                pts.add(p.toVar());
            obj->setProperty("points", pts);
        }
        else
        {
            juce::Array<juce::var> ges;
            for (const auto& g : gestures)
                ges.add(g.toVar());
            obj->setProperty("gestures", ges);
        }
        return juce::var(obj);
    }

    // `unknown` receives the lane's raw `kind` string when it is neither
    // "continuous" nor "discrete" -- the caller (Take::load) uses it to
    // fill LoadStats without Lane.h needing to know about LoadStats itself.
    static Lane fromVar(const juce::var& v, std::string* unknownKindOut = nullptr)
    {
        Lane lane;
        auto* obj = v.getDynamicObject();
        if (!obj)
        {
            lane.kind = Kind::Opaque;
            lane.raw = v;
            return lane;
        }

        lane.key = ControlPath::fromVar(obj->getProperty("key"));
        auto kindStr = obj->getProperty("kind").toString();

        if (kindStr == "discrete")
        {
            lane.kind = Kind::Discrete;
            if (auto* pts = obj->getProperty("points").getArray())
                for (const auto& pv : *pts)
                    lane.points.push_back(DiscretePoint::fromVar(pv));
        }
        else if (kindStr == "continuous")
        {
            lane.kind = Kind::Continuous;
            if (auto* ges = obj->getProperty("gestures").getArray())
                for (const auto& gv : *ges)
                    lane.gestures.push_back(Gesture::fromVar(gv));
        }
        else
        {
            lane.kind = Kind::Opaque;
            lane.raw = v;
            if (unknownKindOut) *unknownKindOut = kindStr.toStdString();
        }
        return lane;
    }
};
