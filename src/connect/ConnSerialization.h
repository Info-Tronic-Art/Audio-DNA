#pragma once
#include "connect/ParamConnection.h"
#include "connect/ScalarParams.h"
#include <juce_core/juce_core.h>
#include <array>
#include <cstddef>

// Sparse, additive, forward-tolerant connection JSON (s166 spec section
// 2.7). Only CONNECTED entries are ever written; a file saved by an older
// build simply lacks these keys and loads with every connection defaulted
// to None. An unrecognized source kind loads as None and is counted (never
// silently misread, never throws) -- the same forward-compat rule that
// covers a future ConnSource::Kind this session does not implement (see
// ParamConnection.h's Kind-level comment).
namespace ConnSerialization
{
    // Curve names, matching MappingCurve's enum order. Currently duplicates
    // PresetManager.cpp's kCurveNames (PresetManager.cpp:22-31) rather than
    // reusing it directly -- src/ui/PresetManager.cpp is outside this lane's
    // fence (src/ui/* is FORBIDDEN), and the spec's "moved to
    // src/connect/ConnSerialization" consolidation is Lane 7 work (v1 preset
    // conversion). Keep the two tables in sync until then.
    const char* curveName(uint8_t curve);
    uint8_t curveFromName(const juce::String& name);

    // One connection: {"src":{...},"shape":{...},"enabled":true}. Callers
    // that need a "p" (param index) or scalar key alongside add it to the
    // returned DynamicObject directly (see Clip.cpp/Layer.cpp/model/
    // Composition.h for the effect-param and scalar-map shapes).
    juce::var toVar(const ParamConnection& c);

    // Loads `v` into `c`, always starting from a fresh default (this is also
    // where "undo/redo and preset load clear all grips" is honored for the
    // preset-load half -- s166 spec section 2.3). An absent/malformed `v`
    // leaves `c` at ParamConnection{} (disconnected). An unrecognized source
    // kind loads as None and increments *unknownKindCount if non-null.
    void fromVar(ParamConnection& c, const juce::var& v, int* unknownKindCount = nullptr);

    // Per-scalar sparse map: {"opacity": {...}, "positionX": {...}, ...} --
    // only connected scalars are written; returns a void juce::var (never a
    // DynamicObject) when nothing is connected, so callers can skip adding
    // an empty "conns" key entirely.
    template <typename ScalarEnum, size_t N>
    juce::var scalarsToVar(const std::array<ParamConnection, N>& conns,
                           const std::array<ScalarDef, N>& defs)
    {
        bool any = false;
        for (const auto& c : conns)
            if (c.isConnected()) { any = true; break; }
        if (!any)
            return juce::var();

        auto* obj = new juce::DynamicObject();
        for (size_t i = 0; i < N; ++i)
            if (conns[i].isConnected())
                obj->setProperty(juce::String(defs[i].key), toVar(conns[i]));
        return juce::var(obj);
    }

    template <typename ScalarEnum, size_t N>
    void scalarsFromVar(std::array<ParamConnection, N>& conns, const std::array<ScalarDef, N>& defs,
                        const juce::var& v, int* unknownKindCount = nullptr)
    {
        if (auto* obj = v.getDynamicObject())
        {
            for (size_t i = 0; i < N; ++i)
            {
                juce::String key(defs[i].key);
                if (obj->hasProperty(key))
                    fromVar(conns[i], obj->getProperty(key), unknownKindCount);
            }
        }
    }
}
