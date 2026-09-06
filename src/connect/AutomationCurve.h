#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <cstdint>

// AutomationCurve: the shared curve type behind BOTH a hand-drawable
// per-control curve and a recorded performance take (s167-l2 architect
// ruling, received after the s166 spec was written: one drawn, one
// captured, sharing one struct / one evaluator / one editor -- see this
// session's report for the full reasoning on why a recorded lane is a
// WRITER, not a connection Kind, and is therefore NOT modeled here).
//
// x is normalized along whatever domain the owner defines (Envelope::Clock:
// beats-since-cycle-start or clip position, both already folded to [0,1]
// before eval() is called); y is the [0,1] output value.
struct Breakpoint
{
    double x = 0.0;
    float y = 0.0f;

    // The interpolation used for the SEGMENT LEAVING this point (i.e.
    // between this point and the next). Linear ramps; Hold steps (the shape
    // a strobe-like curve wants -- genuinely needed, not speculative);
    // Smooth eases via smoothstep.
    enum class Interp : uint8_t { Linear, Hold, Smooth };
    Interp interp = Interp::Linear;

    // s167 step 1 (recorder core): additive -- a recorded lane's gesture
    // (src/recording/Lane.h) stores its curve as this shared type (D7) and
    // needs to put it in a take.json. Enums are strings (D12 "ADD, never
    // REDEFINE"); an unrecognised string on load falls back to Linear
    // rather than failing the point.
    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("x", x);
        obj->setProperty("y", static_cast<double>(y));
        const char* name = "linear";
        switch (interp)
        {
            case Interp::Hold:   name = "hold";   break;
            case Interp::Smooth: name = "smooth"; break;
            case Interp::Linear: default:          break;
        }
        obj->setProperty("interp", juce::String(name));
        return juce::var(obj);
    }

    static Breakpoint fromVar(const juce::var& v)
    {
        Breakpoint bp;
        if (auto* obj = v.getDynamicObject())
        {
            bp.x = static_cast<double>(obj->getProperty("x"));
            bp.y = static_cast<float>(static_cast<double>(obj->getProperty("y")));
            auto interpStr = obj->getProperty("interp").toString();
            if (interpStr == "hold")        bp.interp = Interp::Hold;
            else if (interpStr == "smooth") bp.interp = Interp::Smooth;
            else                            bp.interp = Interp::Linear;   // default/unknown -> Linear
        }
        return bp;
    }
};

struct AutomationCurve
{
    std::vector<Breakpoint> pts;   // sorted by x

    double xMin() const { return pts.empty() ? 0.0 : pts.front().x; }
    double xMax() const { return pts.empty() ? 0.0 : pts.back().x; }

    // s167 step 1: additive, same reason as Breakpoint::toVar above.
    juce::var toVar() const
    {
        juce::Array<juce::var> arr;
        for (const auto& bp : pts)
            arr.add(bp.toVar());
        return arr;
    }

    static AutomationCurve fromVar(const juce::var& v)
    {
        AutomationCurve c;
        if (auto* arr = v.getArray())
            for (const auto& e : *arr)
                c.pts.push_back(Breakpoint::fromVar(e));
        return c;
    }

    // Evaluates at x; clamps to the first/last point outside [xMin, xMax].
    // Empty -> 0.0f.
    float eval(double x) const
    {
        if (pts.empty())
            return 0.0f;
        if (x <= pts.front().x)
            return pts.front().y;
        if (x >= pts.back().x)
            return pts.back().y;

        for (size_t i = 1; i < pts.size(); ++i)
        {
            if (x <= pts[i].x)
            {
                const Breakpoint& a = pts[i - 1];
                const Breakpoint& b = pts[i];
                double span = b.x - a.x;
                float t = (span > 1e-9) ? static_cast<float>((x - a.x) / span) : 0.0f;
                switch (a.interp)
                {
                    case Breakpoint::Interp::Hold:
                        return a.y;
                    case Breakpoint::Interp::Smooth:
                        t = t * t * (3.0f - 2.0f * t);   // smoothstep
                        return a.y + t * (b.y - a.y);
                    case Breakpoint::Interp::Linear:
                    default:
                        return a.y + t * (b.y - a.y);
                }
            }
        }
        return pts.back().y;
    }
};
