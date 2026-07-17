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
    obj->setProperty("autopilotLoops", autopilotLoops);
    obj->setProperty("autopilotEndOfVideo", autopilotEndOfVideo);

    // Video properties (per-layer)
    obj->setProperty("layerWidth", layerWidth);
    obj->setProperty("layerHeight", layerHeight);
    obj->setProperty("autoSize", static_cast<int>(autoSize));

    // Transition
    obj->setProperty("transitionMode", static_cast<int>(transitionMode));
    obj->setProperty("transitionBlendMode", static_cast<int>(transitionBlendMode));

    // Transform (per-layer)
    obj->setProperty("positionX", static_cast<double>(positionX));
    obj->setProperty("positionY", static_cast<double>(positionY));
    obj->setProperty("layerScale", static_cast<double>(layerScale));
    obj->setProperty("layerRotation", static_cast<double>(layerRotation));
    obj->setProperty("layerAnchorX", static_cast<double>(layerAnchorX));
    obj->setProperty("layerAnchorY", static_cast<double>(layerAnchorY));

    // Feedback (Larsen loop)
    obj->setProperty("feedbackEnabled", feedback.enabled);
    obj->setProperty("feedbackAmount", static_cast<double>(feedback.amount));
    obj->setProperty("feedbackScaleX", static_cast<double>(feedback.scaleX));
    obj->setProperty("feedbackScaleY", static_cast<double>(feedback.scaleY));
    obj->setProperty("feedbackRotation", static_cast<double>(feedback.rotation));
    obj->setProperty("feedbackOffsetX", static_cast<double>(feedback.offsetX));
    obj->setProperty("feedbackOffsetY", static_cast<double>(feedback.offsetY));
    obj->setProperty("feedbackLumaKey", static_cast<double>(feedback.lumaKey));
    obj->setProperty("feedbackPreset", juce::String(feedback.presetName));

    // Layer effects
    juce::Array<juce::var> fxArray;
    for (const auto& fx : layerEffects)
    {
        auto* fxObj = new juce::DynamicObject();
        fxObj->setProperty("name", juce::String(fx.effectName));
        fxObj->setProperty("enabled", fx.enabled);
        fxObj->setProperty("bypassed", fx.bypassed);
        fxObj->setProperty("dryWet", static_cast<double>(fx.dryWet));
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
        // Guarded for backward compatibility with old presets (missing keys keep struct defaults)
        if (obj->hasProperty("autopilotLoops"))
            autopilotLoops = static_cast<int>(obj->getProperty("autopilotLoops"));
        if (obj->hasProperty("autopilotEndOfVideo"))
            autopilotEndOfVideo = static_cast<bool>(obj->getProperty("autopilotEndOfVideo"));

        // Video properties (per-layer)
        if (obj->hasProperty("layerWidth"))
            layerWidth = static_cast<int>(obj->getProperty("layerWidth"));
        if (obj->hasProperty("layerHeight"))
            layerHeight = static_cast<int>(obj->getProperty("layerHeight"));
        if (obj->hasProperty("autoSize"))
            autoSize = static_cast<AutoSizeMode>(static_cast<int>(obj->getProperty("autoSize")));

        // Transition
        if (obj->hasProperty("transitionMode"))
            transitionMode = static_cast<MixMode>(static_cast<int>(obj->getProperty("transitionMode")));
        if (obj->hasProperty("transitionBlendMode"))
            transitionBlendMode = static_cast<MixMode>(static_cast<int>(obj->getProperty("transitionBlendMode")));

        // Transform (per-layer)
        if (obj->hasProperty("positionX"))
            positionX = static_cast<float>(static_cast<double>(obj->getProperty("positionX")));
        if (obj->hasProperty("positionY"))
            positionY = static_cast<float>(static_cast<double>(obj->getProperty("positionY")));
        if (obj->hasProperty("layerScale"))
            layerScale = static_cast<float>(static_cast<double>(obj->getProperty("layerScale")));
        if (obj->hasProperty("layerRotation"))
            layerRotation = static_cast<float>(static_cast<double>(obj->getProperty("layerRotation")));
        if (obj->hasProperty("layerAnchorX"))
            layerAnchorX = static_cast<float>(static_cast<double>(obj->getProperty("layerAnchorX")));
        if (obj->hasProperty("layerAnchorY"))
            layerAnchorY = static_cast<float>(static_cast<double>(obj->getProperty("layerAnchorY")));

        // Feedback (Larsen loop)
        if (obj->hasProperty("feedbackEnabled"))
            feedback.enabled = static_cast<bool>(obj->getProperty("feedbackEnabled"));
        if (obj->hasProperty("feedbackAmount"))
            feedback.amount = static_cast<float>(static_cast<double>(obj->getProperty("feedbackAmount")));
        if (obj->hasProperty("feedbackScaleX"))
            feedback.scaleX = static_cast<float>(static_cast<double>(obj->getProperty("feedbackScaleX")));
        if (obj->hasProperty("feedbackScaleY"))
            feedback.scaleY = static_cast<float>(static_cast<double>(obj->getProperty("feedbackScaleY")));
        if (obj->hasProperty("feedbackRotation"))
            feedback.rotation = static_cast<float>(static_cast<double>(obj->getProperty("feedbackRotation")));
        if (obj->hasProperty("feedbackOffsetX"))
            feedback.offsetX = static_cast<float>(static_cast<double>(obj->getProperty("feedbackOffsetX")));
        if (obj->hasProperty("feedbackOffsetY"))
            feedback.offsetY = static_cast<float>(static_cast<double>(obj->getProperty("feedbackOffsetY")));
        if (obj->hasProperty("feedbackLumaKey"))
            feedback.lumaKey = static_cast<float>(static_cast<double>(obj->getProperty("feedbackLumaKey")));
        if (obj->hasProperty("feedbackPreset"))
            feedback.presetName = obj->getProperty("feedbackPreset").toString().toStdString();

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
                    if (fxObj->hasProperty("dryWet"))
                        slot.dryWet = static_cast<float>(static_cast<double>(fxObj->getProperty("dryWet")));
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
