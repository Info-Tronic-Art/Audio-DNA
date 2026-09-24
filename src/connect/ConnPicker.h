#pragma once
#include "connect/ParamConnection.h"
#include <string>
#include <cstdint>

// Picker -> model translation, headless (s-rta-0923 lane 3 plan section
// 4.3). Lane C0 (this file) declares the CONTRACT; ships as a stub
// (sourceFromPicker returns a default ConnSource{} == Kind::None,
// describeSource returns "") until Lane C4 implements ConnPicker.cpp for
// real. UniversalParamControl's source-picker popup (Manual/Audio/BPM Sync/
// Oscillator/Envelope/Clip Position/Timeline/Macro) builds one of these from
// the menu id it received and hands it to sourceFromPicker; nothing here
// depends on JUCE UI types, so it is unit-testable without a widget.
struct PickerChoice
{
    enum class Kind : uint8_t { Manual, Signal, BpmSync, ClipPosition, Timeline, Macro } kind = Kind::Manual;
    std::string signalName;
    int shapeIdx = 0;    // BpmSync: ConnSource::Lfo::Shape index (Sine/SawUp/Triangle/Square/SampleHold)
    int divIdx = 2;       // BpmSync: index into {0.25, 0.5, 1, 2, 4, 8} beats-per-cycle
    int macroIdx = -1;    // Macro: 0..MacroBank::kNumMacros-1
};

// Signal(name) | Lfo{Sine/SawUp/Triangle/Square, {0.25,0.5,1,2,4,8}[divIdx]}
// | ClipPosition | Envelope{curve {0,0},{1,1} Linear, Beats, 4} | Macro(i)
ConnSource sourceFromPicker(const PickerChoice& choice);

// "Bass" | "Square 1 Beat" | "Clip Position" | "Timeline" | "Macro 3" | ""
std::string describeSource(const ConnSource& source);
