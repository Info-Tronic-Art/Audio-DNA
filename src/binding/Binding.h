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
        AdjustLayerOpacity, // Continuous control of layer opacity
        SwitchDeck,
        TapTempo,
        Resync,
        GlobalPlayPause,
        GlobalStop,
        MasterOpacity,      // Continuous control of master opacity
        Snapshot,           // P22.7: Take a PNG screenshot
        ToggleRecording     // P22.6: Start/stop video recording
    };
    Action action = Action::TriggerClip;

    // === Trigger Mode (P21) ===
    // Controls whether the binding fires on press only (Toggle) or press+release (Momentary/Piano).
    enum class TriggerMode : uint8_t
    {
        Toggle,     // Press = activate, press again = deactivate (default)
        Momentary   // Press = activate, release = deactivate (piano mode)
    };
    TriggerMode triggerMode = TriggerMode::Toggle;

    // === MIDI CC Mode (P21) ===
    // For CC bindings: Absolute (0-127 maps directly) or Relative (delta from 64).
    enum class CCMode : uint8_t
    {
        Absolute,   // 0-127 → 0.0-1.0 (default)
        Relative    // < 64 = decrement, > 64 = increment (endless encoders)
    };
    CCMode ccMode = CCMode::Absolute;
    float ccStepSize = 0.01f; // Step size for relative CC mode

    // === Targeting Mode (P21) ===
    // Controls what the binding targets.
    enum class TargetMode : uint8_t
    {
        ByPosition, // Targets the clip/layer at the specified index (survives reorder)
        ThisItem,   // Targets a specific clip by ID (follows the clip if moved)
        Selected    // Targets whatever is currently selected in the UI
    };
    TargetMode targetMode = TargetMode::ByPosition;
    uint32_t targetClipId = 0;  // For ThisItem mode — the specific clip ID to target

    // === MIDI Velocity (P21) ===
    bool velocityToOpacity = false; // If true, MIDI velocity maps to clip opacity on trigger

    // === Action Parameters ===
    int targetLayerIndex = 0;
    int targetColumn = 0;
    int targetDeckIndex = 0;
    int targetEffectIndex = 0;
    int targetMacroIndex = 0;

    bool enabled = true;
};
