#pragma once
#include <juce_core/juce_core.h>
#include <string>
#include <tuple>
#include <cstdint>

// ControlPath: the lane key -- shared by the recorder (src/recording/*),
// REST, and later the connection test endpoints (s167 spec D2). A control
// is addressed by BOTH a positional path (what the app resolves by today,
// G7/G8) and the names that were there at capture time (what a human
// recognises and what re-binding matches on). Never keyed on Clip::id
// (G6/R4) -- `clipName` exists only for re-targeting, never identity.
//
// Program::compile (src/recording/Program.h) is the only place that turns
// a ControlPath into coordinates it trusts (D2's three-way resolution
// policy: resolved / rebound-by-position / rebound-by-name / unresolved);
// nothing else in src/recording/ resolves one mid-playback.
struct ControlPath
{
    enum class Scope : uint8_t { Clip, Layer, Comp, Macro, Routine };
    Scope scope = Scope::Comp;

    // === Deck ===
    int deck = -1;
    bool deckRelative = false;   // true = "active deck at fire time" (routines default, D2)
    std::string deckName;

    // === Layer (within deck) ===
    int layer = -1;
    uint32_t layerId = 0;        // tie-breaker only (Deck.h layer ids); never the primary key
    std::string layerName;

    // === Column / clip (within layer) ===
    int col = -1;
    std::string clipName;

    // === Effect slot (within the owner's stack -- clip, layer or comp) ===
    int fx = -1;
    std::string fxName;

    // === What on that object -- D2's control vocabulary. Free-form on
    // purpose: a future control this reader does not know about is still a
    // valid string and round-trips (D12 "ADD, never REDEFINE"); only the
    // dispatcher (later) needs to recognise it. ===
    std::string control;

    // control == "param" (index positional, key is the name check, G8)
    int param = -1;
    std::string paramKey;

    // control == "scalar" (ScalarDef::key -- itself the positional id, no
    // separate index exists for scalars)
    std::string scalar;

    // control == "macro" (ruling #10: layer/clip macro scopes are LATER,
    // additive; only Global is meaningful now)
    int macroScope = 0;
    int macro = -1;

    // Continuous lane kinds (D3's control-vocabulary paragraph).
    bool isContinuous() const
    {
        return control == "param" || control == "dryWet" || control == "scalar"
            || control == "macro" || control == "speed";
    }

    // Map key / equality: POSITIONAL fields only -- names are compared by
    // Program::compile (D2 resolution), never as part of a lane's identity.
    bool operator<(const ControlPath& o) const  { return asTuple() < o.asTuple(); }
    bool operator==(const ControlPath& o) const { return asTuple() == o.asTuple(); }
    bool operator!=(const ControlPath& o) const { return !(*this == o); }

    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("scope", scopeToString(scope));

        if (scope == Scope::Clip || scope == Scope::Layer)
        {
            auto* deckObj = new juce::DynamicObject();
            deckObj->setProperty("i", deck);
            if (deckRelative) deckObj->setProperty("rel", true);
            deckObj->setProperty("name", juce::String(deckName));
            obj->setProperty("deck", juce::var(deckObj));

            auto* layerObj = new juce::DynamicObject();
            layerObj->setProperty("i", layer);
            layerObj->setProperty("id", static_cast<int>(layerId));
            layerObj->setProperty("name", juce::String(layerName));
            obj->setProperty("layer", juce::var(layerObj));
        }

        if (scope == Scope::Clip)
        {
            auto* colObj = new juce::DynamicObject();
            colObj->setProperty("i", col);
            colObj->setProperty("clip", juce::String(clipName));
            obj->setProperty("col", juce::var(colObj));
        }

        if (fx >= 0 || !fxName.empty())
        {
            auto* fxObj = new juce::DynamicObject();
            fxObj->setProperty("i", fx);
            fxObj->setProperty("name", juce::String(fxName));
            obj->setProperty("fx", juce::var(fxObj));
        }

        obj->setProperty("control", juce::String(control));

        if (control == "param")
        {
            auto* paramObj = new juce::DynamicObject();
            paramObj->setProperty("i", param);
            paramObj->setProperty("key", juce::String(paramKey));
            obj->setProperty("param", juce::var(paramObj));
        }
        else if (control == "scalar")
        {
            obj->setProperty("scalar", juce::String(scalar));
        }
        else if (control == "macro")
        {
            auto* macroObj = new juce::DynamicObject();
            macroObj->setProperty("scope", macroScope);
            macroObj->setProperty("i", macro);
            obj->setProperty("macro", juce::var(macroObj));
        }

        return juce::var(obj);
    }

    static ControlPath fromVar(const juce::var& v)
    {
        ControlPath key;
        auto* obj = v.getDynamicObject();
        if (!obj) return key;

        key.scope = scopeFromString(obj->getProperty("scope").toString());

        if (auto* deckObj = obj->getProperty("deck").getDynamicObject())
        {
            key.deck = static_cast<int>(deckObj->getProperty("i"));
            key.deckRelative = static_cast<bool>(deckObj->getProperty("rel"));
            key.deckName = deckObj->getProperty("name").toString().toStdString();
        }
        if (auto* layerObj = obj->getProperty("layer").getDynamicObject())
        {
            key.layer = static_cast<int>(layerObj->getProperty("i"));
            key.layerId = static_cast<uint32_t>(static_cast<int>(layerObj->getProperty("id")));
            key.layerName = layerObj->getProperty("name").toString().toStdString();
        }
        if (auto* colObj = obj->getProperty("col").getDynamicObject())
        {
            key.col = static_cast<int>(colObj->getProperty("i"));
            key.clipName = colObj->getProperty("clip").toString().toStdString();
        }
        if (auto* fxObj = obj->getProperty("fx").getDynamicObject())
        {
            key.fx = static_cast<int>(fxObj->getProperty("i"));
            key.fxName = fxObj->getProperty("name").toString().toStdString();
        }

        key.control = obj->getProperty("control").toString().toStdString();

        if (auto* paramObj = obj->getProperty("param").getDynamicObject())
        {
            key.param = static_cast<int>(paramObj->getProperty("i"));
            key.paramKey = paramObj->getProperty("key").toString().toStdString();
        }
        if (obj->hasProperty("scalar"))
            key.scalar = obj->getProperty("scalar").toString().toStdString();
        if (auto* macroObj = obj->getProperty("macro").getDynamicObject())
        {
            key.macroScope = static_cast<int>(macroObj->getProperty("scope"));
            key.macro = static_cast<int>(macroObj->getProperty("i"));
        }

        return key;
    }

    static const char* scopeToString(Scope s)
    {
        switch (s)
        {
            case Scope::Clip:    return "clip";
            case Scope::Layer:   return "layer";
            case Scope::Comp:    return "comp";
            case Scope::Macro:   return "macro";
            case Scope::Routine: return "routine";
        }
        return "comp";
    }

    static Scope scopeFromString(const juce::String& s)
    {
        if (s == "clip")    return Scope::Clip;
        if (s == "layer")   return Scope::Layer;
        if (s == "macro")   return Scope::Macro;
        if (s == "routine") return Scope::Routine;
        return Scope::Comp;   // default/unknown -> Comp (D12: never silently misfiled as playable)
    }

private:
    using KeyTuple = std::tuple<uint8_t, int, bool, int, int, int, std::string, int, std::string, int, int>;
    KeyTuple asTuple() const
    {
        return { static_cast<uint8_t>(scope), deck, deckRelative, layer, col, fx,
                 control, param, scalar, macroScope, macro };
    }
};
