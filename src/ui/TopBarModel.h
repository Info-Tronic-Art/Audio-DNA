#pragma once
#include <juce_core/juce_core.h>
#include <cstdint>

// TopBarModel -- s-rta-0926b plan3-final.md section 3 (ITEM B): a PURE function from the bar
// count to the TopBar's one-line readout. Boris: "top bar count should go 1-2-3-4-1 etc" -- the
// bar within a 4-bar group, restarting at a Resync and, in live tracking, at a structural
// transition (same edge the routines' 2 Bar / 4 Bar quantize consults, RoutineEngine::dueNow).
// Header-only, juce_core only -- the RoutineBankModel.h posture -- so this is pinned by a Catch2
// test that links juce_core alone, never juce_gui_basics.
inline constexpr int kBarsPerCount = 4;

inline juce::String barReadoutText(bool hasBpm, uint16_t barCount)
{
    return hasBpm ? "Bar " + juce::String(barCount % kBarsPerCount + 1) : juce::String("Bar -");
}
