#pragma once
#include "model/Clip.h"
#include <juce_core/juce_core.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <cstdint>

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

    // === Per-layer Effect Chain ===
    std::vector<Clip::EffectSlot> layerEffects;

    // === Autopilot Defaults ===
    Clip::AutopilotAction defaultAutopilotAction = Clip::AutopilotAction::PlayNext;
    Clip::AutopilotDuration defaultAutopilotDuration = Clip::AutopilotDuration::Beat4;
    int defaultAutopilotCustomBeats = 4;

    // === Clips (one per column) ===
    // Indexed by column. Use std::optional so empty cells are explicit.
    std::vector<std::optional<Clip>> clips;

    // === Runtime State ===
    int activeClipColumn = -1;  // -1 = no active clip
    int previousClipColumn = -1; // For crossfade
    float crossfadeProgress = 1.0f; // 1.0 = fully transitioned

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

    void triggerClip(int column)
    {
        if (column < 0 || column >= static_cast<int>(clips.size()))
            return;

        if (!clips[static_cast<size_t>(column)].has_value())
        {
            // Empty cell — clear the layer
            clearActiveClip();
            return;
        }

        if (column == activeClipColumn)
        {
            // Retrigger from in-point
            if (auto* clip = getActiveClip())
            {
                clip->playheadPosition = static_cast<double>(clip->inPoint);
                clip->beatsPlayed = 0;
                clip->playing = true;
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
            clip->playing = true;
        }
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
