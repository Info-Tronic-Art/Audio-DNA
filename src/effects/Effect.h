#pragma once
#include <juce_core/juce_core.h>
#include <string>
#include <vector>

// A single effect parameter with name, current value, and default.
// All values are normalized to [0, 1].
struct EffectParam
{
    std::string name;          // e.g., "intensity", "freq"
    std::string uniformName;   // e.g., "u_ripple_intensity"
    float value = 0.0f;
    float defaultValue = 0.0f;
};

// Effect: represents a single visual effect backed by a GLSL shader program.
// Owns a list of parameters that can be driven by the mapping engine.
class Effect
{
public:
    Effect(const juce::String& name, const juce::String& category,
           const juce::String& shaderName);

    // Add a parameter to this effect
    void addParam(const std::string& name, const std::string& uniformName,
                  float defaultValue = 0.0f);

    // Parameter access
    int getNumParams() const { return static_cast<int>(params_.size()); }
    EffectParam& getParam(int index) { return params_[static_cast<size_t>(index)]; }
    const EffectParam& getParam(int index) const { return params_[static_cast<size_t>(index)]; }

    // Set a parameter value by index
    void setParamValue(int index, float value);

    // Reset all parameters to defaults
    void resetParams();

    // Enable/disable
    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }

    // Metadata
    const juce::String& getName() const { return name_; }
    const juce::String& getCategory() const { return category_; }
    const juce::String& getShaderName() const { return shaderName_; }

    int getOrder() const { return order_; }
    void setOrder(int order) { order_ = order; }

private:
    juce::String name_;
    juce::String category_;
    juce::String shaderName_;  // Key into ShaderManager
    std::vector<EffectParam> params_;
    bool enabled_ = true;
    int order_ = 0;
};
