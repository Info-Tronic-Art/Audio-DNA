#include "Effect.h"
#include <algorithm>

Effect::Effect(const juce::String& name, const juce::String& category,
               const juce::String& shaderName)
    : name_(name), category_(category), shaderName_(shaderName)
{
}

void Effect::addParam(const std::string& name, const std::string& uniformName,
                      float defaultValue)
{
    params_.push_back({name, uniformName, defaultValue, defaultValue});
}

void Effect::setParamValue(int index, float value)
{
    if (index >= 0 && index < static_cast<int>(params_.size()))
        params_[static_cast<size_t>(index)].value = std::clamp(value, 0.0f, 1.0f);
}

void Effect::resetParams()
{
    for (auto& p : params_)
        p.value = p.defaultValue;
}
