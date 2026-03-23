#include "Layer.h"

juce::var Layer::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("name", juce::String(name));
    obj->setProperty("id", static_cast<int>(id));
    obj->setProperty("type", static_cast<int>(type));
    obj->setProperty("opacity", static_cast<double>(opacity));
    obj->setProperty("visible", visible);
    obj->setProperty("bypassed", bypassed);
    obj->setProperty("solo", solo);
    obj->setProperty("muted", muted);
    obj->setProperty("autopilotEnabled", autopilotEnabled);
    obj->setProperty("ignoreColumnTrigger", ignoreColumnTrigger);
    obj->setProperty("persistent", persistent);
    obj->setProperty("folded", folded);
    obj->setProperty("blendMode", static_cast<int>(blendMode));
    obj->setProperty("keyingMode", static_cast<int>(keyingMode));
    obj->setProperty("keyThreshold", static_cast<double>(keyThreshold));
    obj->setProperty("keySoftness", static_cast<double>(keySoftness));
    obj->setProperty("chromaKeyR", static_cast<double>(chromaKeyR));
    obj->setProperty("chromaKeyG", static_cast<double>(chromaKeyG));
    obj->setProperty("chromaKeyB", static_cast<double>(chromaKeyB));
    obj->setProperty("chromaKeyTolerance", static_cast<double>(chromaKeyTolerance));
    obj->setProperty("dryWetMix", static_cast<double>(dryWetMix));
    obj->setProperty("rotationX", static_cast<double>(rotationX));
    obj->setProperty("rotationY", static_cast<double>(rotationY));
    obj->setProperty("rotationZ", static_cast<double>(rotationZ));
    obj->setProperty("rotationSpeed", static_cast<double>(rotationSpeed));
    obj->setProperty("scale3D", static_cast<double>(scale3D));
    obj->setProperty("transitionSpeed", static_cast<double>(transitionSpeed));
    obj->setProperty("defaultAutopilotAction", static_cast<int>(defaultAutopilotAction));
    obj->setProperty("defaultAutopilotDuration", static_cast<int>(defaultAutopilotDuration));
    obj->setProperty("defaultAutopilotCustomBeats", defaultAutopilotCustomBeats);

    // Layer effects
    juce::Array<juce::var> fxArray;
    for (const auto& fx : layerEffects)
    {
        auto* fxObj = new juce::DynamicObject();
        fxObj->setProperty("name", juce::String(fx.effectName));
        fxObj->setProperty("enabled", fx.enabled);
        fxObj->setProperty("bypassed", fx.bypassed);
        juce::Array<juce::var> paramArray;
        for (float p : fx.paramValues)
            paramArray.add(static_cast<double>(p));
        fxObj->setProperty("params", paramArray);
        fxArray.add(juce::var(fxObj));
    }
    obj->setProperty("layerEffects", fxArray);

    // Clips
    juce::Array<juce::var> clipArray;
    for (const auto& clipOpt : clips)
    {
        if (clipOpt.has_value())
            clipArray.add(clipOpt->toVar());
        else
            clipArray.add(juce::var()); // null for empty cells
    }
    obj->setProperty("clips", clipArray);

    return juce::var(obj);
}

void Layer::fromVar(const juce::var& v)
{
    if (auto* obj = v.getDynamicObject())
    {
        name = obj->getProperty("name").toString().toStdString();
        id = static_cast<uint32_t>(static_cast<int>(obj->getProperty("id")));
        type = static_cast<Type>(static_cast<int>(obj->getProperty("type")));
        opacity = static_cast<float>(static_cast<double>(obj->getProperty("opacity")));
        visible = static_cast<bool>(obj->getProperty("visible"));
        bypassed = static_cast<bool>(obj->getProperty("bypassed"));
        solo = static_cast<bool>(obj->getProperty("solo"));
        muted = static_cast<bool>(obj->getProperty("muted"));
        autopilotEnabled = static_cast<bool>(obj->getProperty("autopilotEnabled"));
        ignoreColumnTrigger = static_cast<bool>(obj->getProperty("ignoreColumnTrigger"));
        persistent = static_cast<bool>(obj->getProperty("persistent"));
        if (obj->hasProperty("folded"))
            folded = static_cast<bool>(obj->getProperty("folded"));
        blendMode = static_cast<MixMode>(static_cast<int>(obj->getProperty("blendMode")));
        keyingMode = static_cast<KeyingMode>(static_cast<int>(obj->getProperty("keyingMode")));
        keyThreshold = static_cast<float>(static_cast<double>(obj->getProperty("keyThreshold")));
        keySoftness = static_cast<float>(static_cast<double>(obj->getProperty("keySoftness")));
        chromaKeyR = static_cast<float>(static_cast<double>(obj->getProperty("chromaKeyR")));
        chromaKeyG = static_cast<float>(static_cast<double>(obj->getProperty("chromaKeyG")));
        chromaKeyB = static_cast<float>(static_cast<double>(obj->getProperty("chromaKeyB")));
        chromaKeyTolerance = static_cast<float>(static_cast<double>(obj->getProperty("chromaKeyTolerance")));
        dryWetMix = static_cast<float>(static_cast<double>(obj->getProperty("dryWetMix")));
        rotationX = static_cast<float>(static_cast<double>(obj->getProperty("rotationX")));
        rotationY = static_cast<float>(static_cast<double>(obj->getProperty("rotationY")));
        rotationZ = static_cast<float>(static_cast<double>(obj->getProperty("rotationZ")));
        rotationSpeed = static_cast<float>(static_cast<double>(obj->getProperty("rotationSpeed")));
        scale3D = static_cast<float>(static_cast<double>(obj->getProperty("scale3D")));
        transitionSpeed = static_cast<float>(static_cast<double>(obj->getProperty("transitionSpeed")));
        defaultAutopilotAction = static_cast<Clip::AutopilotAction>(static_cast<int>(obj->getProperty("defaultAutopilotAction")));
        defaultAutopilotDuration = static_cast<Clip::AutopilotDuration>(static_cast<int>(obj->getProperty("defaultAutopilotDuration")));
        defaultAutopilotCustomBeats = static_cast<int>(obj->getProperty("defaultAutopilotCustomBeats"));

        layerEffects.clear();
        if (auto* fxArray = obj->getProperty("layerEffects").getArray())
        {
            for (const auto& fxVar : *fxArray)
            {
                if (auto* fxObj = fxVar.getDynamicObject())
                {
                    Clip::EffectSlot slot;
                    slot.effectName = fxObj->getProperty("name").toString().toStdString();
                    slot.enabled = static_cast<bool>(fxObj->getProperty("enabled"));
                    slot.bypassed = static_cast<bool>(fxObj->getProperty("bypassed"));
                    if (auto* paramArray = fxObj->getProperty("params").getArray())
                        for (const auto& p : *paramArray)
                            slot.paramValues.push_back(static_cast<float>(static_cast<double>(p)));
                    layerEffects.push_back(std::move(slot));
                }
            }
        }

        clips.clear();
        if (auto* clipArray = obj->getProperty("clips").getArray())
        {
            for (const auto& clipVar : *clipArray)
            {
                if (clipVar.isVoid() || clipVar.isUndefined())
                {
                    clips.push_back(std::nullopt);
                }
                else
                {
                    Clip clip;
                    clip.fromVar(clipVar);
                    clips.push_back(std::move(clip));
                }
            }
        }
    }
}
