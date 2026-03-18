#pragma once
#include <juce_core/juce_core.h>
#include <string>
#include <vector>
#include <cstdint>

// Clip: media content + per-clip effects + transport + autopilot config.
// Placed in a deck cell (layer × column intersection).
struct Clip
{
    // === Identity ===
    std::string name;
    uint32_t id = 0; // Unique ID within composition

    // === Media ===
    enum class MediaType : uint8_t { None, Image, Video, Camera, Source };
    MediaType mediaType = MediaType::None;
    juce::File mediaFile;           // For Image or Video
    int cameraDeviceIndex = -1;     // For Camera
    std::string sourceType;         // For procedural Source (e.g., "perlin_noise")
    bool hasAlpha = false;          // True if media has an alpha channel

    // === Per-clip Effect Chain ===
    struct EffectSlot
    {
        std::string effectName;                // Registry name (e.g., "ripple")
        std::vector<float> paramValues;        // Parameter values [0,1]
        bool enabled = true;
        bool bypassed = false;
    };
    std::vector<EffectSlot> effects;

    // === Transport ===
    enum class TransportMode : uint8_t { Timeline, BPMSync };
    TransportMode transportMode = TransportMode::Timeline;
    enum class LoopMode : uint8_t { Loop, PingPong, OneShot };
    LoopMode loopMode = LoopMode::Loop;
    float speed = 1.0f;             // Playback speed multiplier
    bool reverse = false;
    float startOffset = 0.0f;       // [0,1] normalized start position

    // === Beat Snap ===
    bool beatSnap = false;          // Snap playhead to beat on trigger

    // === Cuepoints ===
    static constexpr int kMaxCuepoints = 8;
    float cuepoints[kMaxCuepoints] = {}; // Normalized positions [0,1]
    int numCuepoints = 0;

    // === Autopilot ===
    enum class AutopilotAction : uint8_t
    {
        LayerDetermined, DoNothing, PlayNext, PlayPrevious,
        PlayRandom, PlayFirst, PlayLast, PlaySpecific
    };
    AutopilotAction autopilotAction = AutopilotAction::LayerDetermined;
    int autopilotSpecificCol = -1;  // For PlaySpecific

    enum class AutopilotDuration : uint8_t
    {
        LayerDetermined, Beat1, Beat2, Beat4, Beat8, Beat16, Beat32, Custom
    };
    AutopilotDuration autopilotDuration = AutopilotDuration::LayerDetermined;
    int autopilotCustomBeats = 4;

    // === Runtime State (not serialized) ===
    bool playing = false;
    double playheadPosition = 0.0; // [0,1] normalized
    int beatsPlayed = 0;

    // === Helpers ===
    bool hasMedia() const { return mediaType != MediaType::None; }
    bool hasEffects() const { return !effects.empty(); }
    bool isEmpty() const { return !hasMedia() && !hasEffects(); }

    void clear()
    {
        name.clear();
        mediaType = MediaType::None;
        mediaFile = juce::File();
        cameraDeviceIndex = -1;
        sourceType.clear();
        effects.clear();
        transportMode = TransportMode::Timeline;
        loopMode = LoopMode::Loop;
        speed = 1.0f;
        reverse = false;
        startOffset = 0.0f;
        beatSnap = false;
        numCuepoints = 0;
        autopilotAction = AutopilotAction::LayerDetermined;
        autopilotDuration = AutopilotDuration::LayerDetermined;
        playing = false;
        playheadPosition = 0.0;
        beatsPlayed = 0;
    }

    // === Serialization ===
    juce::var toVar() const;
    void fromVar(const juce::var& v);
};
