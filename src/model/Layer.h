#pragma once
#include "model/Clip.h"
#include <juce_core/juce_core.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <cstdint>

// FeedbackConfig: per-layer feedback parameters.
// Feedback routes a layer's output back to its input with transformation.
struct FeedbackConfig
{
    bool enabled = false;
    float amount = 0.5f;      // [0,1] — how much of prev frame bleeds through
    float scaleX = 0.98f;     // Per-frame scale X (< 1 = zoom in, > 1 = zoom out)
    float scaleY = 0.98f;     // Per-frame scale Y
    float rotation = 0.0f;    // Per-frame rotation in degrees
    float offsetX = 0.0f;     // Per-frame horizontal drift [-0.5, 0.5]
    float offsetY = 0.0f;     // Per-frame vertical drift [-0.5, 0.5]
    float lumaKey = 0.0f;     // Fade out dark areas to prevent muddiness [0,1]
    std::string presetName;   // Preset name (empty = custom)
};

// Layer: a row in the deck. Contains clips across columns.
// One clip is active per layer at a time.
struct Layer
{
    // === Identity ===
    std::string name = "Layer";
    uint32_t id = 0;

    // === Layer Type ===
    enum class Type : uint8_t
    {
        Opaque,         // One clip at a time, replaces everything below
        Transparent,    // Composited over layers below with blend/keying
        FXOnly,         // Effects applied to accumulator (no media)
        ThreeD,         // 3D model/surface rendering
        Mask            // Content becomes alpha mask for layers below
    };
    Type type = Type::Opaque;

    // === Layer Controls ===
    float opacity = 1.0f;
    bool visible = true;
    bool bypassed = false;
    bool solo = false;
    bool muted = false;          // Audio mute
    bool autopilotEnabled = false;
    bool ignoreColumnTrigger = false;
    bool persistent = false;        // If true, this layer keeps rendering even when deck is not active
    bool folded = false;            // P24.12: If true, layer row is collapsed in DeckView

    // === Mix Mode — unified list for both layer blending and clip transitions ===
    // V dropdown picks a MixMode for persistent layer compositing.
    // F dropdown picks a MixMode for momentary clip-to-clip transitions.
    // Same list, different contexts: V = "the look", F = "the flash".
    enum class MixMode : uint8_t
    {
        // === Standard Compositing (persistent blend modes) ===
        // Basic
        Normal, Additive, Screen, Multiply, Overlay,
        // Light
        SoftLight, HardLight, VividLight, LinearLight, PinLight, HardMix,
        // Dark/Light compare
        Darken, Lighten, DarkerColor, LighterColor,
        // Dodge/Burn
        ColorDodge, ColorBurn,
        // Inversion
        Difference, Exclusion, Subtract,
        // Component (HSL)
        Hue, Saturation, Color, Luminosity,
        // Special blend
        Dissolve,

        // === Transitions (momentary clip changes, also usable as blend) ===
        // Instant
        Cut,
        // Directional wipes
        WipeLeft, WipeRight, WipeUp, WipeDown, WipeEllipse, WipeDiagonal,
        // Push (content slides in/out)
        PushLeft, PushRight, PushUp, PushDown,
        // Zoom
        ZoomIn, ZoomOut,
        // 3D rotation
        RotateX, RotateY, Spin, Cube, Flip, Fold,
        // Fade through color
        ToBlack, ToWhite,
        // Creative / VJ
        Pixelate, Blur, Noise, RGBSplit, GlitchBlocks, Strobe,
        Slide, Stretch, Displace
    };
    MixMode blendMode = MixMode::Additive;

    // === Keying Mode (Transparent type) ===
    enum class KeyingMode : uint8_t
    {
        Alpha, LumaKey, InvertedLumaKey, LumaIsAlpha, InvertedLumaIsAlpha,
        ChromaKey, MaxRGB, SaturationKey, EdgeDetection, ThresholdMask,
        ChannelR, ChannelG, ChannelB
    };
    KeyingMode keyingMode = KeyingMode::Alpha;
    float keyThreshold = 0.1f;
    float keySoftness = 0.1f;
    float chromaKeyR = 0.0f, chromaKeyG = 1.0f, chromaKeyB = 0.0f;
    float chromaKeyTolerance = 0.2f;

    // === FX Only ===
    float dryWetMix = 1.0f;

    // === 3D Controls ===
    float rotationX = 0.0f, rotationY = 0.0f, rotationZ = 0.0f;
    float rotationSpeed = 0.0f;
    float scale3D = 1.0f;

    // === Video Properties (per-layer) ===
    int layerWidth = 1920;
    int layerHeight = 1080;
    enum class AutoSizeMode : uint8_t { Off, Fill, Fit, Stretch, Original };
    AutoSizeMode autoSize = AutoSizeMode::Off;

    // === Transition ===
    MixMode transitionMode = MixMode::Dissolve;  // F dropdown — momentary clip change style
    MixMode transitionBlendMode = MixMode::Normal; // Transition blend method
    float transitionSpeed = -1.0f; // -1 = use global default

    // === Transform (per-layer, applied after clip compositing) ===
    float positionX = 0.0f;
    float positionY = 0.0f;
    float layerScale = 1.0f;        // 1.0 = 100%
    float layerRotation = 0.0f;     // Degrees
    float layerAnchorX = 0.0f;
    float layerAnchorY = 0.0f;

    // === Feedback (Larsen loop) ===
    FeedbackConfig feedback;

    // === Per-layer Effect Chain ===
    std::vector<Clip::EffectSlot> layerEffects;

    // === Autopilot Defaults ===
    Clip::AutopilotAction defaultAutopilotAction = Clip::AutopilotAction::PlayNext;
    Clip::AutopilotDuration defaultAutopilotDuration = Clip::AutopilotDuration::Beat4;
    int defaultAutopilotCustomBeats = 4;
    int autopilotLoops = 1;  // Number of clip loops before advancing (1 = advance after first play)
    bool autopilotEndOfVideo = false;  // true = advance when video playhead reaches outPoint

    // === Clips (one per column) ===
    // Indexed by column. Use std::optional so empty cells are explicit.
    std::vector<std::optional<Clip>> clips;

    // === Runtime State ===
    int activeClipColumn = -1;  // -1 = no active clip
    int previousClipColumn = -1; // For crossfade
    float crossfadeProgress = 1.0f; // 1.0 = fully transitioned
    int pendingTriggerColumn = -1;  // Beat snap: queued trigger awaiting next beat
    // Set alongside pendingTriggerColumn whenever a trigger is queued. Off means
    // "derive granularity from the target clip's own beatSnapMode" (today's
    // behavior, unchanged). Non-Off means a caller (global Quantize) is FORCING
    // a granularity for this one queued trigger, overriding the clip's own field
    // for this trigger only — the clip's own beatSnapMode is never mutated.
    Clip::BeatSnapMode pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;

    // === Helpers ===
    Clip* getActiveClip()
    {
        if (activeClipColumn >= 0 && activeClipColumn < static_cast<int>(clips.size()))
        {
            if (clips[static_cast<size_t>(activeClipColumn)].has_value())
                return &clips[static_cast<size_t>(activeClipColumn)].value();
        }
        return nullptr;
    }

    const Clip* getActiveClip() const
    {
        if (activeClipColumn >= 0 && activeClipColumn < static_cast<int>(clips.size()))
        {
            if (clips[static_cast<size_t>(activeClipColumn)].has_value())
                return &clips[static_cast<size_t>(activeClipColumn)].value();
        }
        return nullptr;
    }

    Clip* getClipAt(int column)
    {
        if (column >= 0 && column < static_cast<int>(clips.size()))
        {
            if (clips[static_cast<size_t>(column)].has_value())
                return &clips[static_cast<size_t>(column)].value();
        }
        return nullptr;
    }

    void triggerClip(int column, Clip::BeatSnapMode forcedSnap = Clip::BeatSnapMode::Off)
    {
        if (column < 0 || column >= static_cast<int>(clips.size()))
            return;

        if (!clips[static_cast<size_t>(column)].has_value())
        {
            // Empty cell — clear the layer. clearActiveClip() itself now cancels
            // any pending trigger too (L5 Quantize fix), so no separate reset
            // needed here.
            clearActiveClip();
            return;
        }

        // Check beat snap: if the target clip has beat snap enabled, or a caller is
        // forcing a granularity (global Quantize), queue for next beat/bar.
        auto& clipOpt = clips[static_cast<size_t>(column)];
        bool snapEnabled = forcedSnap != Clip::BeatSnapMode::Off ||
                          (clipOpt.has_value() &&
                          (clipOpt->beatSnapMode != Clip::BeatSnapMode::Off || clipOpt->beatSnap));
        if (snapEnabled && column != activeClipColumn)
        {
            pendingTriggerColumn = column;
            pendingTriggerSnapOverride = forcedSnap;
            return;
        }

        triggerClipImmediate(column);
    }

    // Execute a clip trigger immediately (bypasses beat snap check).
    // Called directly or from beat snap queue processing.
    void triggerClipImmediate(int column)
    {
        if (column < 0 || column >= static_cast<int>(clips.size()))
            return;

        pendingTriggerColumn = -1;
        pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;

        if (column == activeClipColumn)
        {
            // Retrigger from in-point — preserve current playing state
            if (auto* clip = getActiveClip())
            {
                clip->playheadPosition = static_cast<double>(clip->inPoint);
                clip->beatsPlayed = 0;
                // Don't change clip->playing — keep paused if paused, playing if playing
            }
            return;
        }

        // Start transition to new clip
        previousClipColumn = activeClipColumn;
        activeClipColumn = column;
        crossfadeProgress = (transitionSpeed <= 0.0f) ? 1.0f : 0.0f;

        if (auto* clip = getActiveClip())
        {
            clip->playheadPosition = static_cast<double>(clip->inPoint);
            clip->beatsPlayed = 0;
            // Only auto-play on first activation; returning clips keep their state
            if (!clip->hasBeenTriggered)
                clip->playing = true;
        }
    }

    // Process pending beat-snapped triggers. Call on each beat detection.
    // beatInBar: which beat within the bar (0-3). barCount: total bars elapsed.
    void processPendingTrigger(int beatInBar = 0, int barCount = 0)
    {
        if (pendingTriggerColumn < 0)
            return;

        // Determine the required snap granularity: a forced override (global
        // Quantize) wins outright; otherwise fall back to the pending clip's own
        // beatSnapMode (unchanged pre-existing behavior).
        auto snapMode = Clip::BeatSnapMode::Beat; // default
        auto& clipOpt = clips[static_cast<size_t>(pendingTriggerColumn)];
        if (pendingTriggerSnapOverride != Clip::BeatSnapMode::Off)
            snapMode = pendingTriggerSnapOverride;               // global quantize forced this
        else if (clipOpt.has_value())
            snapMode = (clipOpt->beatSnapMode != Clip::BeatSnapMode::Off)
                       ? clipOpt->beatSnapMode : Clip::BeatSnapMode::Beat;   // unchanged fallback

        bool shouldTrigger = false;
        switch (snapMode)
        {
            case Clip::BeatSnapMode::Off:
            case Clip::BeatSnapMode::Beat:
                shouldTrigger = true; // Every beat
                break;
            case Clip::BeatSnapMode::Bar:
                shouldTrigger = (beatInBar == 0); // First beat of bar
                break;
            case Clip::BeatSnapMode::TwoBar:
                shouldTrigger = (beatInBar == 0 && (barCount % 2) == 0);
                break;
            case Clip::BeatSnapMode::FourBar:
                shouldTrigger = (beatInBar == 0 && (barCount % 4) == 0);
                break;
        }

        if (shouldTrigger)
            triggerClipImmediate(pendingTriggerColumn);
    }

    void clearActiveClip()
    {
        if (activeClipColumn >= 0)
        {
            if (auto* clip = getActiveClip())
                clip->playing = false;
        }
        previousClipColumn = activeClipColumn;
        activeClipColumn = -1;
        crossfadeProgress = 1.0f;

        // L5 Quantize fix: a clear also cancels any quantized trigger still
        // queued on this layer. Without this, a pending trigger silently
        // outlives the clear and fires on the next beat/bar crossing,
        // reactivating a layer the caller just deactivated — e.g. a momentary
        // MIDI pad released before the beat lands (handleBindingAction's
        // Momentary release path calls clearActiveClip() directly). Lives here,
        // not at individual call sites, so every caller inherits it the same
        // way the queue decision itself lives in triggerClip rather than at
        // each trigger call site.
        pendingTriggerColumn = -1;
        pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
    }

    void ensureColumns(int count)
    {
        if (static_cast<int>(clips.size()) < count)
            clips.resize(static_cast<size_t>(count));
    }

    // === Serialization ===
    juce::var toVar() const;
    void fromVar(const juce::var& v);
};
