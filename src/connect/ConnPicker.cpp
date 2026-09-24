#include "connect/ConnPicker.h"
#include <array>
#include <cmath>
#include <sstream>

// Lane C4: the real picker -> model translation (s-rta-0923 lane 3 plan
// section 4.3). Headless -- no JUCE UI types -- so it stays unit-testable
// without a widget (tests/test_conn_picker.cpp).

namespace
{

// The BPM Sync submenu's fixed division set (UniversalParamControl.cpp's
// own divisions[] = {"1/4 Beat","1/2 Beat","1 Beat","2 Beats","4 Beats",
// "8 Beats"}), in the same order the widget's divIdx indexes into.
constexpr std::array<float, 6> kBpmDivisions{ 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };

bool approxEqual(float a, float b) { return std::fabs(a - b) < 0.001f; }

// Inverse of kBpmDivisions -- matches the widget's own division strings
// verbatim (UniversalParamControl.cpp buildSourcePickerMenu's divisions[]).
std::string formatBeats(float cycleBeats)
{
    if (approxEqual(cycleBeats, 0.25f)) return "1/4 Beat";
    if (approxEqual(cycleBeats, 0.5f))  return "1/2 Beat";
    if (approxEqual(cycleBeats, 1.0f))  return "1 Beat";
    if (approxEqual(cycleBeats, 2.0f))  return "2 Beats";
    if (approxEqual(cycleBeats, 4.0f))  return "4 Beats";
    if (approxEqual(cycleBeats, 8.0f))  return "8 Beats";
    // A cycleBeats value outside the picker's fixed set (e.g. loaded from an
    // old preset) -- fall back to a plain numeric label rather than picking
    // the nearest fixed string, so it is obviously not one of the 6 presets.
    std::ostringstream oss;
    oss << cycleBeats << (approxEqual(cycleBeats, 1.0f) ? " Beat" : " Beats");
    return oss.str();
}

// Matches the widget's own shapes[] = {"Sine","Saw","Triangle","Square"}
// display names (UniversalParamControl.cpp buildSourcePickerMenu), not the
// ConnSource::Lfo::Shape enumerator spelling (SawUp).
std::string lfoShapeName(ConnSource::Lfo::Shape shape)
{
    switch (shape)
    {
        case ConnSource::Lfo::Shape::Sine:       return "Sine";
        case ConnSource::Lfo::Shape::SawUp:      return "Saw";
        case ConnSource::Lfo::Shape::Triangle:   return "Triangle";
        case ConnSource::Lfo::Shape::Square:     return "Square";
        case ConnSource::Lfo::Shape::SampleHold: return "Sample Hold";
    }
    return "";
}

} // namespace

ConnSource sourceFromPicker(const PickerChoice& choice)
{
    ConnSource src;   // Kind::None (Manual) by default

    switch (choice.kind)
    {
        case PickerChoice::Kind::Manual:
            return src;

        case PickerChoice::Kind::Signal:
            src.kind = ConnSource::Kind::Signal;
            src.signalName = choice.signalName;
            return src;

        case PickerChoice::Kind::BpmSync:
        {
            src.kind = ConnSource::Kind::Lfo;
            int shapeIdx = choice.shapeIdx;
            if (shapeIdx < 0) shapeIdx = 0;
            if (shapeIdx > 3) shapeIdx = 3;   // widget's menu only offers Sine/Saw/Triangle/Square
            src.lfo.shape = static_cast<ConnSource::Lfo::Shape>(shapeIdx);

            int divIdx = choice.divIdx;
            if (divIdx < 0) divIdx = 0;
            if (divIdx >= static_cast<int>(kBpmDivisions.size()))
                divIdx = static_cast<int>(kBpmDivisions.size()) - 1;
            src.lfo.cycleBeats = kBpmDivisions[static_cast<size_t>(divIdx)];
            return src;
        }

        case PickerChoice::Kind::ClipPosition:
            src.kind = ConnSource::Kind::ClipPosition;
            return src;

        case PickerChoice::Kind::Timeline:
            // "Timeline becomes a REAL 4-beat linear ramp Envelope" (plan
            // section 4.3) -- a plain 0->1 ramp over one 4-beat cycle.
            src.kind = ConnSource::Kind::Envelope;
            src.env.clock = ConnSource::Envelope::Clock::Beats;
            src.env.cycleBeats = 4.0f;
            src.env.curve.pts = {
                Breakpoint{ 0.0, 0.0f, Breakpoint::Interp::Linear },
                Breakpoint{ 1.0, 1.0f, Breakpoint::Interp::Linear },
            };
            return src;

        case PickerChoice::Kind::Macro:
            src.kind = ConnSource::Kind::Macro;
            src.macroIndex = choice.macroIdx;
            return src;
    }
    return src;
}

std::string describeSource(const ConnSource& source)
{
    switch (source.kind)
    {
        case ConnSource::Kind::None:
            return "";
        case ConnSource::Kind::Signal:
            return source.signalName;
        case ConnSource::Kind::Macro:
            return "Macro " + std::to_string(source.macroIndex + 1);
        case ConnSource::Kind::Lfo:
            return lfoShapeName(source.lfo.shape) + " " + formatBeats(source.lfo.cycleBeats);
        case ConnSource::Kind::Envelope:
            // The only origin for Kind::Envelope in this lane is the
            // Timeline picker entry (plan section 4.3) -- a hand-drawn
            // per-parameter curve editor is future work.
            return "Timeline";
        case ConnSource::Kind::ClipPosition:
            return "Clip Position";
    }
    return "";
}
