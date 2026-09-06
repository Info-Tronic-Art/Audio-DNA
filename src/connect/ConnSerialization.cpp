#include "connect/ConnSerialization.h"

namespace
{
    // Mirrors PresetManager.cpp's kCurveNames exactly (order == MappingCurve
    // enum order) -- see the header comment re: why this is a duplicate, not
    // a shared table, for now.
    constexpr const char* kCurveNames[] = {
        "Linear", "Exponential", "Logarithmic", "SCurve", "Stepped",
        "CircularIn", "CircularOut", "CircularInOut",
        "BackIn", "BackOut", "BackInOut",
        "ElasticIn", "ElasticOut", "ElasticInOut",
        "BounceIn", "BounceOut", "BounceInOut",
        "CubicIn", "CubicOut", "CubicInOut",
        "SineIn", "SineOut", "SineInOut",
        "Hold"
    };
    constexpr int kNumCurveNames = static_cast<int>(sizeof(kCurveNames) / sizeof(kCurveNames[0]));

    const char* lfoShapeName(ConnSource::Lfo::Shape s)
    {
        switch (s)
        {
            case ConnSource::Lfo::Shape::Sine:       return "sine";
            case ConnSource::Lfo::Shape::SawUp:      return "sawUp";
            case ConnSource::Lfo::Shape::Triangle:   return "triangle";
            case ConnSource::Lfo::Shape::Square:     return "square";
            case ConnSource::Lfo::Shape::SampleHold: return "sampleHold";
        }
        return "sine";
    }

    ConnSource::Lfo::Shape lfoShapeFromName(const juce::String& name, bool* ok)
    {
        if (ok) *ok = true;
        if (name == "sine")       return ConnSource::Lfo::Shape::Sine;
        if (name == "sawUp")      return ConnSource::Lfo::Shape::SawUp;
        if (name == "triangle")   return ConnSource::Lfo::Shape::Triangle;
        if (name == "square")     return ConnSource::Lfo::Shape::Square;
        if (name == "sampleHold") return ConnSource::Lfo::Shape::SampleHold;
        if (ok) *ok = false;
        return ConnSource::Lfo::Shape::Sine;
    }

    const char* playbackName(ConnShape::Playback p)
    {
        switch (p)
        {
            case ConnShape::Playback::Forward:  return "forward";
            case ConnShape::Playback::Backward: return "backward";
            case ConnShape::Playback::PingPong: return "pingPong";
        }
        return "forward";
    }

    ConnShape::Playback playbackFromName(const juce::String& name)
    {
        if (name == "backward") return ConnShape::Playback::Backward;
        if (name == "pingPong") return ConnShape::Playback::PingPong;
        return ConnShape::Playback::Forward;
    }

    const char* interpName(Breakpoint::Interp i)
    {
        switch (i)
        {
            case Breakpoint::Interp::Linear: return "linear";
            case Breakpoint::Interp::Hold:   return "hold";
            case Breakpoint::Interp::Smooth: return "smooth";
        }
        return "linear";
    }

    Breakpoint::Interp interpFromName(const juce::String& name)
    {
        if (name == "hold")   return Breakpoint::Interp::Hold;
        if (name == "smooth") return Breakpoint::Interp::Smooth;
        return Breakpoint::Interp::Linear;
    }
}

const char* ConnSerialization::curveName(uint8_t curve)
{
    return (curve < static_cast<uint8_t>(kNumCurveNames)) ? kCurveNames[curve] : kCurveNames[0];
}

uint8_t ConnSerialization::curveFromName(const juce::String& name)
{
    for (int i = 0; i < kNumCurveNames; ++i)
        if (name == kCurveNames[i])
            return static_cast<uint8_t>(i);
    return 0;   // Linear
}

juce::var ConnSerialization::toVar(const ParamConnection& c)
{
    auto* obj = new juce::DynamicObject();

    auto* srcObj = new juce::DynamicObject();
    switch (c.source.kind)
    {
        case ConnSource::Kind::None:
            srcObj->setProperty("kind", "none");
            break;
        case ConnSource::Kind::Signal:
            srcObj->setProperty("kind", "signal");
            srcObj->setProperty("name", juce::String(c.source.signalName));
            break;
        case ConnSource::Kind::Macro:
            srcObj->setProperty("kind", "macro");
            srcObj->setProperty("index", c.source.macroIndex);
            break;
        case ConnSource::Kind::Lfo:
            srcObj->setProperty("kind", "lfo");
            srcObj->setProperty("shape", lfoShapeName(c.source.lfo.shape));
            srcObj->setProperty("cycleBeats", static_cast<double>(c.source.lfo.cycleBeats));
            srcObj->setProperty("phase", static_cast<double>(c.source.lfo.phaseOffset));
            srcObj->setProperty("pulseWidth", static_cast<double>(c.source.lfo.pulseWidth));
            break;
        case ConnSource::Kind::Envelope:
        {
            srcObj->setProperty("kind", "envelope");
            srcObj->setProperty("clock", c.source.env.clock == ConnSource::Envelope::Clock::Beats
                                        ? "beats" : "clipPosition");
            srcObj->setProperty("cycleBeats", static_cast<double>(c.source.env.cycleBeats));
            juce::Array<juce::var> pts;
            for (const auto& bp : c.source.env.curve.pts)
            {
                auto* ptObj = new juce::DynamicObject();
                ptObj->setProperty("x", bp.x);
                ptObj->setProperty("y", static_cast<double>(bp.y));
                ptObj->setProperty("interp", interpName(bp.interp));
                pts.add(juce::var(ptObj));
            }
            srcObj->setProperty("points", pts);
            break;
        }
        case ConnSource::Kind::ClipPosition:
            srcObj->setProperty("kind", "clipPosition");
            break;
    }
    obj->setProperty("src", juce::var(srcObj));

    auto* shapeObj = new juce::DynamicObject();
    shapeObj->setProperty("min", static_cast<double>(c.shape.outMin));
    shapeObj->setProperty("max", static_cast<double>(c.shape.outMax));
    shapeObj->setProperty("invert", c.shape.inverted);
    shapeObj->setProperty("playback", playbackName(c.shape.playback));
    shapeObj->setProperty("loop", c.shape.loop);
    shapeObj->setProperty("curve", curveName(c.shape.curve));
    shapeObj->setProperty("inMin", static_cast<double>(c.shape.inMin));
    shapeObj->setProperty("inMax", static_cast<double>(c.shape.inMax));
    shapeObj->setProperty("smoothMs", static_cast<double>(c.shape.smoothingMs));
    obj->setProperty("shape", juce::var(shapeObj));

    obj->setProperty("enabled", c.enabled);
    return juce::var(obj);
}

void ConnSerialization::fromVar(ParamConnection& c, const juce::var& v, int* unknownKindCount)
{
    // Fresh defaults every time -- "undo/redo and preset load clear all
    // grips" (s166 spec section 2.3); a load must never inherit a live
    // gesture or timing state from whatever was previously in `c`.
    c.source = ConnSource{};
    c.shape = ConnShape{};
    c.enabled = true;
    c.grip = ParamConnection::Grip{};
    c.state = ParamConnection::State{};

    auto* obj = v.getDynamicObject();
    if (obj == nullptr)
        return;   // absent/malformed -> stays None, already reset above

    if (auto* srcObj = obj->getProperty("src").getDynamicObject())
    {
        juce::String kind = srcObj->getProperty("kind").toString();
        if (kind == "signal")
        {
            c.source.kind = ConnSource::Kind::Signal;
            c.source.signalName = srcObj->getProperty("name").toString().toStdString();
        }
        else if (kind == "macro")
        {
            c.source.kind = ConnSource::Kind::Macro;
            c.source.macroIndex = static_cast<int>(srcObj->getProperty("index"));
        }
        else if (kind == "lfo")
        {
            c.source.kind = ConnSource::Kind::Lfo;
            bool shapeOk = true;
            c.source.lfo.shape = lfoShapeFromName(srcObj->getProperty("shape").toString(), &shapeOk);
            if (!shapeOk && unknownKindCount)
                ++*unknownKindCount;
            if (srcObj->hasProperty("cycleBeats"))
                c.source.lfo.cycleBeats = static_cast<float>(static_cast<double>(srcObj->getProperty("cycleBeats")));
            if (srcObj->hasProperty("phase"))
                c.source.lfo.phaseOffset = static_cast<float>(static_cast<double>(srcObj->getProperty("phase")));
            if (srcObj->hasProperty("pulseWidth"))
                c.source.lfo.pulseWidth = static_cast<float>(static_cast<double>(srcObj->getProperty("pulseWidth")));
        }
        else if (kind == "envelope")
        {
            c.source.kind = ConnSource::Kind::Envelope;
            c.source.env.clock = (srcObj->getProperty("clock").toString() == "clipPosition")
                ? ConnSource::Envelope::Clock::ClipPosition : ConnSource::Envelope::Clock::Beats;
            if (srcObj->hasProperty("cycleBeats"))
                c.source.env.cycleBeats = static_cast<float>(static_cast<double>(srcObj->getProperty("cycleBeats")));
            c.source.env.curve.pts.clear();
            if (auto* pts = srcObj->getProperty("points").getArray())
            {
                for (const auto& pv : *pts)
                {
                    if (auto* ptObj = pv.getDynamicObject())
                    {
                        // Current form: {"x":..,"y":..,"interp":".."}.
                        Breakpoint bp;
                        bp.x = static_cast<double>(ptObj->getProperty("x"));
                        bp.y = static_cast<float>(static_cast<double>(ptObj->getProperty("y")));
                        bp.interp = interpFromName(ptObj->getProperty("interp").toString());
                        c.source.env.curve.pts.push_back(bp);
                    }
                    else if (auto* pair = pv.getArray(); pair != nullptr && pair->size() >= 2)
                    {
                        // Old flat [x,y] pair form (pre-Breakpoint) -- loads
                        // as Interp::Linear, matching what that shape always
                        // meant (s167-l2 team-lead correction).
                        Breakpoint bp;
                        bp.x = static_cast<double>((*pair)[0]);
                        bp.y = static_cast<float>(static_cast<double>((*pair)[1]));
                        bp.interp = Breakpoint::Interp::Linear;
                        c.source.env.curve.pts.push_back(bp);
                    }
                }
            }
        }
        else if (kind == "clipPosition")
        {
            c.source.kind = ConnSource::Kind::ClipPosition;
        }
        else if (kind == "none")
        {
            c.source.kind = ConnSource::Kind::None;
        }
        else
        {
            // Unrecognized kind (a future format, e.g. Timeline, or a
            // corrupt file) -- loads as None and is counted, never
            // misinterpreted as one of today's kinds (s166 spec section 2.7).
            c.source.kind = ConnSource::Kind::None;
            if (unknownKindCount)
                ++*unknownKindCount;
        }
    }

    if (auto* shapeObj = obj->getProperty("shape").getDynamicObject())
    {
        if (shapeObj->hasProperty("min"))
            c.shape.outMin = static_cast<float>(static_cast<double>(shapeObj->getProperty("min")));
        if (shapeObj->hasProperty("max"))
            c.shape.outMax = static_cast<float>(static_cast<double>(shapeObj->getProperty("max")));
        if (shapeObj->hasProperty("invert"))
            c.shape.inverted = static_cast<bool>(shapeObj->getProperty("invert"));
        if (shapeObj->hasProperty("playback"))
            c.shape.playback = playbackFromName(shapeObj->getProperty("playback").toString());
        if (shapeObj->hasProperty("loop"))
            c.shape.loop = static_cast<bool>(shapeObj->getProperty("loop"));
        if (shapeObj->hasProperty("curve"))
            c.shape.curve = curveFromName(shapeObj->getProperty("curve").toString());
        if (shapeObj->hasProperty("inMin"))
            c.shape.inMin = static_cast<float>(static_cast<double>(shapeObj->getProperty("inMin")));
        if (shapeObj->hasProperty("inMax"))
            c.shape.inMax = static_cast<float>(static_cast<double>(shapeObj->getProperty("inMax")));
        if (shapeObj->hasProperty("smoothMs"))
            c.shape.smoothingMs = static_cast<float>(static_cast<double>(shapeObj->getProperty("smoothMs")));
    }

    if (obj->hasProperty("enabled"))
        c.enabled = static_cast<bool>(obj->getProperty("enabled"));
}
