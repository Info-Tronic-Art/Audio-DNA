#pragma once
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cstddef>

// One shared descriptor table per scalar family so the connection engine,
// serialization, and any future UI never re-derive the model<->normalized
// conversion formulas independently (s166 spec section 2.2). Formulas are
// lifted verbatim from the existing inspectors and re-verified against the
// current source this session (ClipInspector.cpp:404-408,1251-1255;
// LayerInspector.cpp:399-403,918-920; CompositionInspector.cpp:173-178,
// 540-544) -- including the "builder verifies" compPositionY formula the
// architect flagged as ASSUMED: confirmed exact, (v-0.5)*2160.
struct ScalarDef
{
    const char* key;                 // JSON key (ConnSerialization's per-scalar sparse map)
    float (*toModel)(float norm);    // normalized [0,1] slider position -> real model units
    float (*toNorm)(float model);    // real model units -> normalized [0,1] slider position
    float defaultNorm;               // normalized position matching the field's struct default
};

enum class ClipScalar : uint8_t { Opacity, PosX, PosY, Scale, Rotation, AnchorX, AnchorY, Count };
enum class LayerScalar : uint8_t { Opacity, PosX, PosY, Scale, Rotation, AnchorX, AnchorY, Count };

// CompScalar has 8 entries, not the spec's original 9: an owner amendment
// received during this lane (after the s166 spec was written) rules
// Composition opacity is ONE knob -- final = masterOpacity * layerOpacity *
// clipOpacity. compOpacity (the spec's D7 "two separate opacities" reading)
// merges into masterOpacity and is therefore not a separately-connectable
// scalar here. See the s167-l2 report FINDINGS for the full amendment text.
enum class CompScalar : uint8_t { Opacity, Speed, PosX, PosY, Scale, Rotation, AnchorX, AnchorY, Count };

namespace ScalarMath
{
    inline float identity(float x) { return x; }
    inline float posX3840(float v) { return (v - 0.5f) * 3840.0f; }
    inline float normPosX3840(float m) { return m / 3840.0f + 0.5f; }
    inline float posY2160(float v) { return (v - 0.5f) * 2160.0f; }
    inline float normPosY2160(float m) { return m / 2160.0f + 0.5f; }
    // Clip/Layer scale is exponential: 2^((v-0.5)*2), so a mid-slider (0.5)
    // is 1.0x, one end is 0.25x, the other 4x.
    inline float expScale(float v) { return std::pow(2.0f, (v - 0.5f) * 2.0f); }
    inline float normExpScale(float m) { return std::log2(std::max(0.01f, m)) / 2.0f + 0.5f; }
    inline float rot720(float v) { return (v - 0.5f) * 720.0f; }
    inline float normRot720(float m) { return m / 720.0f + 0.5f; }
    // Composition's scale is linear (v*2), unlike clip/layer's exponential --
    // a pre-existing inconsistency (s166 spec section 6), preserved as-is.
    inline float linScale2(float v) { return v * 2.0f; }
    inline float normLinScale2(float m) { return m / 2.0f; }
    inline float speed4(float v) { return v * 4.0f; }
    inline float normSpeed4(float m) { return m / 4.0f; }
}

inline const std::array<ScalarDef, static_cast<size_t>(ClipScalar::Count)>& clipScalarDefs()
{
    static const std::array<ScalarDef, static_cast<size_t>(ClipScalar::Count)> defs{ {
        { "opacity",   ScalarMath::identity,  ScalarMath::identity,    1.0f },
        { "positionX", ScalarMath::posX3840,  ScalarMath::normPosX3840, 0.5f },
        { "positionY", ScalarMath::posY2160,  ScalarMath::normPosY2160, 0.5f },
        { "scale",     ScalarMath::expScale,  ScalarMath::normExpScale, 0.5f },
        { "rotation",  ScalarMath::rot720,    ScalarMath::normRot720,   0.5f },
        { "anchorX",   ScalarMath::posX3840,  ScalarMath::normPosX3840, 0.5f },
        { "anchorY",   ScalarMath::posY2160,  ScalarMath::normPosY2160, 0.5f },
    } };
    return defs;
}

inline const std::array<ScalarDef, static_cast<size_t>(LayerScalar::Count)>& layerScalarDefs()
{
    static const std::array<ScalarDef, static_cast<size_t>(LayerScalar::Count)> defs{ {
        { "opacity",   ScalarMath::identity,  ScalarMath::identity,    1.0f },
        { "positionX", ScalarMath::posX3840,  ScalarMath::normPosX3840, 0.5f },
        { "positionY", ScalarMath::posY2160,  ScalarMath::normPosY2160, 0.5f },
        { "scale",     ScalarMath::expScale,  ScalarMath::normExpScale, 0.5f },
        { "rotation",  ScalarMath::rot720,    ScalarMath::normRot720,   0.5f },
        { "anchorX",   ScalarMath::posX3840,  ScalarMath::normPosX3840, 0.5f },
        { "anchorY",   ScalarMath::posY2160,  ScalarMath::normPosY2160, 0.5f },
    } };
    return defs;
}

inline const std::array<ScalarDef, static_cast<size_t>(CompScalar::Count)>& compScalarDefs()
{
    static const std::array<ScalarDef, static_cast<size_t>(CompScalar::Count)> defs{ {
        { "opacity",   ScalarMath::identity,   ScalarMath::identity,    1.0f },   // -> masterOpacity
        { "speed",     ScalarMath::speed4,     ScalarMath::normSpeed4,  0.25f },  // -> masterSpeed
        { "positionX", ScalarMath::posX3840,   ScalarMath::normPosX3840, 0.5f },
        { "positionY", ScalarMath::posY2160,   ScalarMath::normPosY2160, 0.5f },
        { "scale",     ScalarMath::linScale2,  ScalarMath::normLinScale2, 0.5f },
        { "rotation",  ScalarMath::rot720,     ScalarMath::normRot720,   0.5f },
        { "anchorX",   ScalarMath::posX3840,   ScalarMath::normPosX3840, 0.5f },
        { "anchorY",   ScalarMath::posY2160,   ScalarMath::normPosY2160, 0.5f },
    } };
    return defs;
}
