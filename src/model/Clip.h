#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
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
    enum class MediaType : uint8_t { None, Image, Video, Camera, Source, ImageSequence };
    MediaType mediaType = MediaType::None;
    juce::File mediaFile;           // For Image or Video
    int cameraDeviceIndex = -1;     // For Camera
    std::string sourceType;         // For procedural Source (e.g., "perlin_noise")
    bool hasAlpha = false;          // True if media has an alpha channel

    // === Image Sequence (multi-image as video) ===
    std::vector<juce::File> sequenceFiles;  // Sorted image files
    float sequenceFps = 2.5f;               // Configurable frames per second

    // BPM Sync: how many beats to play back over
    // e.g., 4.0 = play content over 4 beats (1 bar), 1.0 = over 1 beat
    float beatDivision = 4.0f;

    // How many beats the source content contains (for exact timing).
    // If videoBeats=8 and beatDivision=8: plays at native speed at correct BPM.
    // If videoBeats=8 and beatDivision=4: plays at 2x (half the content per cycle).
    float videoBeats = 4.0f;

    // === Source Parameters (for procedural sources) ===
    struct SourceParam
    {
        std::string name;           // Display name
        std::string uniformName;    // GLSL uniform name
        float value = 0.5f;
        float defaultValue = 0.5f;
    };
    std::vector<SourceParam> sourceParams;  // Populated when sourceType is set

    // === Per-clip Effect Chain ===
    struct EffectSlot
    {
        std::string effectName;                // Registry name (e.g., "ripple")
        std::vector<float> paramValues;        // Parameter values [0,1]
        float dryWet = 1.0f;                   // 0 = fully dry, 1 = fully wet
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
    float startOffset = 0.0f;       // [0,1] normalized start position (legacy, use inPoint)

    // === In/Out Points ===
    float inPoint = 0.0f;           // [0,1] playback start position (draggable on timeline)
    float outPoint = 1.0f;          // [0,1] playback end position (draggable on timeline)

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

    // === Video Properties ===
    float clipOpacity = 1.0f;       // Per-clip opacity [0,1]
    int clipWidth = 1920;           // Video width (pixels)
    int clipHeight = 1080;          // Video height (pixels)
    enum class BlendOverride : uint8_t { LayerDetermined, Override };
    BlendOverride blendOverride = BlendOverride::LayerDetermined;
    enum class AlphaType : uint8_t { Premultiplied, Straight };
    AlphaType alphaType = AlphaType::Premultiplied;
    bool channelR = true, channelG = true, channelB = true, channelA = true;

    // === Transform (per-clip, applied before layer compositing) ===
    float positionX = 0.0f;         // Pixels offset from center
    float positionY = 0.0f;
    float scale = 1.0f;             // 1.0 = 100%
    float rotation = 0.0f;          // Degrees
    float anchorX = 0.0f;           // Anchor point offset from center
    float anchorY = 0.0f;

    // === MilkDrop Preset Playlist (P20.5) ===
    // When sourceType == "projectm_visualizer", this playlist cycles through presets.
    struct PresetEntry
    {
        std::string presetPath;       // Full path to .milk file
        std::string presetName;       // Display name
        std::string mood;             // Mood tag for smart cycling
        float energy = 0.5f;          // Energy level [0,1]
    };
    std::vector<PresetEntry> presetPlaylist;
    mutable int presetPlaylistIndex = 0;    // Current position in playlist
    mutable int presetBeatsPlayed = 0;      // Beat counter for playlist cycling

    enum class PlaylistCycleMode : uint8_t
    {
        Sequential, Reverse, RandomOther, RandomBag, PingPong
    };
    PlaylistCycleMode playlistCycleMode = PlaylistCycleMode::RandomBag;

    enum class PlaylistTrigger : uint8_t
    {
        Beats, Bars, Phrase, OnDrop, OnBreakdown, Manual
    };
    PlaylistTrigger playlistTrigger = PlaylistTrigger::Beats;
    int playlistTriggerBeats = 8;           // N beats/bars depending on trigger mode
    float playlistBlendSeconds = 1.5f;      // Crossfade duration between presets
    bool playlistEnabled = false;           // Master enable for playlist cycling

    bool hasPresetPlaylist() const { return !presetPlaylist.empty() && playlistEnabled; }

    // === Runtime State (not serialized) ===
    mutable bool playing = false; // mutable: render thread updates for OneShot/PingPong stop
    mutable double playheadPosition = 0.0; // [0,1] normalized — mutable for render-thread updates via const Clip*
    int beatsPlayed = 0;
    bool hasBeenTriggered = false; // true after first user trigger (used to auto-play on first click)
    juce::Image thumbnail;          // Cached thumbnail for UI display

    // === Helpers ===
    bool hasMedia() const { return mediaType != MediaType::None; }
    bool isPlayable() const { return mediaType == MediaType::Video || mediaType == MediaType::ImageSequence; }
    bool hasEffects() const { return !effects.empty(); }
    bool isEmpty() const { return !hasMedia() && !hasEffects(); }

    void clear()
    {
        name.clear();
        mediaType = MediaType::None;
        mediaFile = juce::File();
        cameraDeviceIndex = -1;
        sourceType.clear();
        sourceParams.clear();
        sequenceFiles.clear();
        sequenceFps = 2.5f;
        beatDivision = 4.0f;
        videoBeats = 4.0f;
        effects.clear();
        transportMode = TransportMode::Timeline;
        loopMode = LoopMode::Loop;
        speed = 1.0f;
        reverse = false;
        startOffset = 0.0f;
        inPoint = 0.0f;
        outPoint = 1.0f;
        beatSnap = false;
        numCuepoints = 0;
        autopilotAction = AutopilotAction::LayerDetermined;
        autopilotDuration = AutopilotDuration::LayerDetermined;
        presetPlaylist.clear();
        presetPlaylistIndex = 0;
        presetBeatsPlayed = 0;
        playlistCycleMode = PlaylistCycleMode::RandomBag;
        playlistTrigger = PlaylistTrigger::Beats;
        playlistTriggerBeats = 8;
        playlistBlendSeconds = 1.5f;
        playlistEnabled = false;
        playing = false;
        playheadPosition = 0.0;
        beatsPlayed = 0;
    }

    // === Serialization ===
    juce::var toVar() const;
    void fromVar(const juce::var& v);
};
