#include "Clip.h"

juce::var Clip::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("name", juce::String(name));
    obj->setProperty("id", static_cast<int>(id));
    obj->setProperty("mediaType", static_cast<int>(mediaType));
    obj->setProperty("mediaFile", mediaFile.getFullPathName());
    obj->setProperty("cameraDeviceIndex", cameraDeviceIndex);
    obj->setProperty("sourceType", juce::String(sourceType));
    obj->setProperty("transportMode", static_cast<int>(transportMode));
    obj->setProperty("loopMode", static_cast<int>(loopMode));
    obj->setProperty("speed", static_cast<double>(speed));
    obj->setProperty("reverse", reverse);
    obj->setProperty("startOffset", static_cast<double>(startOffset));
    obj->setProperty("beatSnap", beatSnap);
    obj->setProperty("autopilotAction", static_cast<int>(autopilotAction));
    obj->setProperty("autopilotDuration", static_cast<int>(autopilotDuration));
    obj->setProperty("autopilotCustomBeats", autopilotCustomBeats);
    obj->setProperty("autopilotSpecificCol", autopilotSpecificCol);

    // Effects
    juce::Array<juce::var> fxArray;
    for (const auto& fx : effects)
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
    obj->setProperty("effects", fxArray);

    // Cuepoints
    juce::Array<juce::var> cpArray;
    for (int i = 0; i < numCuepoints; ++i)
        cpArray.add(static_cast<double>(cuepoints[i]));
    obj->setProperty("cuepoints", cpArray);

    return juce::var(obj);
}

void Clip::fromVar(const juce::var& v)
{
    if (auto* obj = v.getDynamicObject())
    {
        name = obj->getProperty("name").toString().toStdString();
        id = static_cast<uint32_t>(static_cast<int>(obj->getProperty("id")));
        mediaType = static_cast<MediaType>(static_cast<int>(obj->getProperty("mediaType")));
        mediaFile = juce::File(obj->getProperty("mediaFile").toString());
        cameraDeviceIndex = static_cast<int>(obj->getProperty("cameraDeviceIndex"));
        sourceType = obj->getProperty("sourceType").toString().toStdString();
        transportMode = static_cast<TransportMode>(static_cast<int>(obj->getProperty("transportMode")));
        loopMode = static_cast<LoopMode>(static_cast<int>(obj->getProperty("loopMode")));
        speed = static_cast<float>(static_cast<double>(obj->getProperty("speed")));
        reverse = static_cast<bool>(obj->getProperty("reverse"));
        startOffset = static_cast<float>(static_cast<double>(obj->getProperty("startOffset")));
        beatSnap = static_cast<bool>(obj->getProperty("beatSnap"));
        autopilotAction = static_cast<AutopilotAction>(static_cast<int>(obj->getProperty("autopilotAction")));
        autopilotDuration = static_cast<AutopilotDuration>(static_cast<int>(obj->getProperty("autopilotDuration")));
        autopilotCustomBeats = static_cast<int>(obj->getProperty("autopilotCustomBeats"));
        autopilotSpecificCol = static_cast<int>(obj->getProperty("autopilotSpecificCol"));

        effects.clear();
        if (auto* fxArray = obj->getProperty("effects").getArray())
        {
            for (const auto& fxVar : *fxArray)
            {
                if (auto* fxObj = fxVar.getDynamicObject())
                {
                    EffectSlot slot;
                    slot.effectName = fxObj->getProperty("name").toString().toStdString();
                    slot.enabled = static_cast<bool>(fxObj->getProperty("enabled"));
                    slot.bypassed = static_cast<bool>(fxObj->getProperty("bypassed"));
                    if (auto* paramArray = fxObj->getProperty("params").getArray())
                        for (const auto& p : *paramArray)
                            slot.paramValues.push_back(static_cast<float>(static_cast<double>(p)));
                    effects.push_back(std::move(slot));
                }
            }
        }

        numCuepoints = 0;
        if (auto* cpArray = obj->getProperty("cuepoints").getArray())
        {
            for (const auto& cp : *cpArray)
            {
                if (numCuepoints < kMaxCuepoints)
                    cuepoints[numCuepoints++] = static_cast<float>(static_cast<double>(cp));
            }
        }
    }
}
