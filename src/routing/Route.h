#pragma once
#include "mapping/CurveTransforms.h"
#include <cstdint>
#include <string>

// Route: connects a Signal or Macro to a target parameter.
// Replaces the v1 Mapping struct with richer configuration.
struct Route
{
    uint32_t id = 0;

    // === Source ===
    enum class SourceType : uint8_t { Signal, Macro };
    SourceType sourceType = SourceType::Signal;
    uint32_t sourceId = 0; // Signal ID or Macro ID

    // === Target ===
    // Targets are identified by a scope + effect index + param index.
    enum class TargetScope : uint8_t { Clip, Layer, Global };
    TargetScope targetScope = TargetScope::Clip;
    uint32_t targetLayerId = 0;
    uint32_t targetClipId = 0;
    int targetEffectIndex = 0;
    int targetParamIndex = 0;

    // === Transform ===
    float outputMin = 0.0f;
    float outputMax = 1.0f;
    bool inverted = false;

    // Dial range: input sensitivity (what portion of source range drives output)
    float dialRangeMin = 0.0f;
    float dialRangeMax = 1.0f;

    // Per-route gain and threshold
    float threshold = 0.0f;   // Source must exceed this to have any effect
    float gain = 1.0f;        // Post-threshold multiplier
    float falloff = 0.1f;     // How quickly value falls when source drops below threshold

    bool enabled = true;
};

// RouteTarget: identifies a specific parameter in the system.
// Used for quick lookup during per-frame processing.
struct RouteTarget
{
    Route::TargetScope scope;
    uint32_t layerId;
    uint32_t clipId;
    int effectIndex;
    int paramIndex;

    bool operator==(const RouteTarget& other) const
    {
        return scope == other.scope
            && layerId == other.layerId
            && clipId == other.clipId
            && effectIndex == other.effectIndex
            && paramIndex == other.paramIndex;
    }
};
