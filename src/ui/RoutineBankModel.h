#pragma once
#include "recording/RoutineEngine.h"
#include <juce_core/juce_core.h>
#include <cmath>
#include <string>

// RoutineBankModel -- s-rta-0926 lane 3 (plan-routines-s1-final.md section 5.4, section 7 LANE 3
// "optional bank strip"). A PURE function from RoutineEngine::Status to everything the Record
// panel's "Routines" pad row shows: each pad's text / enabled / tone / tooltip and whether
// pressing it fires or stops. Same posture as RecordPanelModel.h -- no juce_gui_basics here, so
// the pad matrix is pinned by a Catch2 table test (tests/test_routine_bank_model.cpp) that links
// juce_core only. Every pad is derived from Status::Slot, never from panel memory.

// Bars are always 4 beats in this app (RoutineSlice.cpp's own local kBeatsPerBar); the pad label's
// "(bar N)" reading is the running routine's 1-based bar within its own cycle.
inline constexpr double kRoutineBeatsPerBar = 4.0;

// s-rta-0926 cleanup lane (review-routine-strip-r1.md): the Record panel's raw "From bar"/"To bar"
// editors accept blank (TextEditor::getText().getIntValue() == 0 on an empty field) or a typed 0
// or negative bar, and onSaveRoutine's real signature takes 1-based, inclusive bars -- so an
// unclamped call could hand it fromBar/toBar < 1, or toBar < fromBar (an empty/inverted range).
// PURE: bars are always >= 1, and toBar is always >= fromBar. Call this on the raw editor values
// BEFORE calling onSaveRoutine, never after.
struct RoutineBarRange { int fromBar; int toBar; };
inline RoutineBarRange clampRoutineBarRange(int fromBar, int toBar)
{
    if (fromBar < 1) fromBar = 1;
    if (toBar < 1) toBar = 1;
    if (toBar < fromBar) toBar = fromBar;
    return { fromBar, toBar };
}

struct RoutineBankView
{
    enum class Tone { Neutral, Playing, Warning };
    struct Pad
    {
        juce::String text;
        bool enabled = false;   // false only for an empty pad
        bool firing = true;     // true: pressing calls Fire; false: pressing calls Stop
        Tone tone = Tone::Neutral;
        juce::String tooltip;
    };
    Pad pads[RoutineEngine::kBankSize];
};

inline RoutineBankView deriveRoutineBankView(const RoutineEngine::Status& status)
{
    RoutineBankView v;
    for (int i = 0; i < RoutineEngine::kBankSize; ++i)
    {
        const auto& s = status.slots[i];
        auto& pad = v.pads[i];
        const juce::String number = juce::String(i + 1) + ": ";

        if (s.state == "pending")
        {
            pad.text = number + juce::String(s.name) + " (next bar)";
            pad.enabled = true;
            pad.firing = false;
            pad.tone = RoutineBankView::Tone::Warning;
            pad.tooltip = "Starting on the next bar. Press to stop before it starts.";
        }
        else if (s.state == "running")
        {
            const int bar = static_cast<int>(std::floor(s.position / kRoutineBeatsPerBar)) + 1;
            pad.text = number + juce::String(s.name) + " (bar " + juce::String(bar) + ")";
            pad.enabled = true;
            pad.firing = false;
            pad.tone = RoutineBankView::Tone::Playing;
            pad.tooltip = "Playing. Press to stop.";
        }
        else if (s.state == "idle")
        {
            pad.text = number + juce::String(s.name);
            pad.enabled = true;
            pad.firing = true;
            pad.tone = RoutineBankView::Tone::Neutral;
            pad.tooltip = "Press to fire this routine.";
        }
        else   // "empty"
        {
            pad.text = number + "Empty";
            pad.enabled = false;
            pad.firing = true;
            pad.tone = RoutineBankView::Tone::Neutral;
            pad.tooltip = "No routine is saved on this pad yet. Use Save Routine below.";
        }
    }
    return v;
}
