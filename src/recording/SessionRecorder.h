#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include <cstdint>
#include <atomic>

// SessionRecorder: records all parameter changes and clip triggers as timestamped events.
// Playback reproduces the performance exactly by replaying events at their timestamps.
//
// Event types:
//   - ParameterChange: effect/source/transform parameter changed
//   - ClipTrigger: clip triggered in deck (layer, column)
//   - ColumnTrigger: entire column triggered
//   - MacroChange: macro knob value changed
//   - TransportChange: play/pause/stop/speed/reverse
//
// Storage: JSON for readability, compact binary for large sessions.
// The undo/command system is separate — this records performance actions.
class SessionRecorder
{
public:
    SessionRecorder() = default;

    // === Event types ===
    enum class EventType : uint8_t
    {
        ParameterChange,   // effect param, source param, or transform param
        ClipTrigger,       // clip activated
        ColumnTrigger,     // column activated
        MacroChange,       // macro knob adjusted
        TransportChange,   // play/pause/speed/reverse
        EffectToggle,      // effect enabled/disabled/bypassed
        CuepointJump,      // jumped to cuepoint
    };

    struct Event
    {
        double timestamp = 0.0;      // seconds since recording started
        EventType type = EventType::ParameterChange;

        // For ParameterChange / MacroChange
        uint32_t targetId = 0;       // clip ID or layer index
        uint32_t paramIndex = 0;     // parameter index or macro index
        float value = 0.0f;          // new value

        // For ClipTrigger / ColumnTrigger
        int layerIndex = -1;
        int columnIndex = -1;

        // For TransportChange
        std::string action;          // "play", "pause", "stop", "speed", "reverse"

        // For EffectToggle
        std::string effectName;
        bool enabled = true;
    };

    // === Recording ===
    void startRecording();
    void stopRecording();
    bool isRecording() const { return recording_.load(std::memory_order_relaxed); }

    // Record an event (thread-safe via mutex)
    void recordParameterChange(uint32_t targetId, uint32_t paramIndex, float value);
    void recordClipTrigger(int layerIndex, int columnIndex);
    void recordColumnTrigger(int columnIndex);
    void recordMacroChange(uint32_t macroIndex, float value);
    void recordTransportChange(const std::string& action, float value = 0.0f);
    void recordEffectToggle(const std::string& effectName, bool enabled);
    void recordCuepointJump(uint32_t clipId, int cuepointIndex, float position);

    // === Playback ===
    void startPlayback();
    void stopPlayback();
    bool isPlaying() const { return playing_.load(std::memory_order_relaxed); }

    // Advance playback. Returns events that should fire this frame.
    // Call once per frame with the frame's delta time.
    std::vector<const Event*> advancePlayback(double dt);

    // === Session management ===
    int getNumEvents() const;
    double getDuration() const;

    // Save/load JSON
    bool saveToFile(const juce::File& file) const;
    bool loadFromFile(const juce::File& file);

    // Clear all recorded events
    void clear();

private:
    std::vector<Event> events_;
    std::atomic<bool> recording_{false};
    std::atomic<bool> playing_{false};

    double recordStartTime_ = 0.0;    // wall clock at recording start
    double playbackTime_ = 0.0;       // current playback position
    size_t playbackIndex_ = 0;        // next event to fire

    juce::CriticalSection lock_;      // protects events_ during recording

    double getCurrentTime() const;
};
