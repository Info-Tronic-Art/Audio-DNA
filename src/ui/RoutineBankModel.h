#pragma once
#include "recording/RoutineEngine.h"
#include <juce_core/juce_core.h>
#include <cmath>
#include <string>

// RoutineBankModel -- s-rta-0926 lane 3 (plan-routines-s1-final.md section 5.4). What is left of the
// Record panel's routine model after s-rta-0927 moved the pad row to the deck (src/ui/RoutineDeckView.h):
// the bar length and the Save Routine row's bar-range clamp. juce_core only, pinned by
// tests/test_routine_bank_model.cpp.

// Bars are always 4 beats in this app (RoutineSlice.cpp's own local kBeatsPerBar); the deck pad's
// "5/8" reading is the running routine's 1-based bar within its own cycle.
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
