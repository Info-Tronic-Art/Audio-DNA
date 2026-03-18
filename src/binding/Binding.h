#pragma once
#include <string>
#include <cstdint>

// Binding: maps a keyboard key or MIDI note/CC to an application action.
struct Binding
{
    uint32_t id = 0;

    // === Input Source ===
    enum class InputType : uint8_t { Keyboard, MidiNote, MidiCC };
    InputType inputType = InputType::Keyboard;

    int keyCode = 0;         // JUCE key code (for Keyboard)
    bool keyModShift = false;
    bool keyModCmd = false;
    bool keyModAlt = false;

    int midiChannel = 0;     // 0 = any channel
    int midiNote = 0;        // For MidiNote
    int midiCC = 0;          // For MidiCC

    // === Action ===
    enum class Action : uint8_t
    {
        TriggerClip,        // Trigger a specific clip (layerIndex + column)
        TriggerColumn,      // Trigger a column across all layers
        ToggleLayerBypass,
        ToggleLayerSolo,
        ToggleLayerMute,
        ToggleLayerAutopilot,
        ToggleLayerVisible,
        LayerTransport,     // Play/pause/reverse on layer
        ToggleEffectBypass, // Bypass a specific effect
        AdjustMacro,        // Continuous control of a macro knob
        SwitchDeck,
        TapTempo,
        Resync,
        GlobalPlayPause,
        GlobalStop,
        MasterOpacity       // Continuous control of master opacity
    };
    Action action = Action::TriggerClip;

    // === Action Parameters ===
    int targetLayerIndex = 0;
    int targetColumn = 0;
    int targetDeckIndex = 0;
    int targetEffectIndex = 0;
    int targetMacroIndex = 0;

    bool enabled = true;
};
