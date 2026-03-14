#pragma once
#include "effects/Effect.h"
#include <juce_core/juce_core.h>
#include <memory>
#include <vector>

// EffectLibrary: registry of effect definitions.
// Each entry defines a name, category, shader key, and parameter list.
// Use createEffect() to instantiate a configured Effect object.
//
// All 17 effects from the architecture are registered via registerDefaults().
class EffectLibrary
{
public:
    // Definition of a single effect parameter
    struct ParamDef
    {
        std::string name;
        std::string uniformName;
        float defaultValue;
    };

    // Definition of a single effect type
    struct EffectDef
    {
        juce::String name;
        juce::String category;     // "warp", "color", "glitch", "blur"
        juce::String shaderName;   // Key into ShaderManager
        std::vector<ParamDef> params;
    };

    EffectLibrary() = default;

    // Register all 17 built-in effects
    void registerDefaults();

    // Create an Effect instance by name. Returns nullptr if not found.
    std::unique_ptr<Effect> createEffect(const juce::String& name) const;

    // Get all registered effect names
    juce::StringArray getEffectNames() const;

    // Get effect names filtered by category
    juce::StringArray getEffectsByCategory(const juce::String& category) const;

    // Get the definition for an effect by name. Returns nullptr if not found.
    const EffectDef* getEffectDef(const juce::String& name) const;

    int getNumEffects() const { return static_cast<int>(defs_.size()); }

private:
    void registerEffect(const EffectDef& def);

    std::vector<EffectDef> defs_;
};
