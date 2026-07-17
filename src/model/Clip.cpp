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
    obj->setProperty("hasAlpha", hasAlpha);

    // Image sequence
    if (!sequenceFiles.empty())
    {
        juce::Array<juce::var> seqArray;
        for (const auto& f : sequenceFiles)
            seqArray.add(f.getFullPathName());
        obj->setProperty("sequenceFiles", seqArray);
        obj->setProperty("sequenceFps", static_cast<double>(sequenceFps));
    }
    // BPM-sync timing — always persisted (matters for BPMSync clips without a sequence)
    obj->setProperty("beatDivision", static_cast<double>(beatDivision));
    obj->setProperty("videoBeats", static_cast<double>(videoBeats));

    // Source parameters
    juce::Array<juce::var> spArray;
    for (const auto& sp : sourceParams)
    {
        auto* spObj = new juce::DynamicObject();
        spObj->setProperty("name", juce::String(sp.name));
        spObj->setProperty("uniform", juce::String(sp.uniformName));
        spObj->setProperty("value", static_cast<double>(sp.value));
        spObj->setProperty("default", static_cast<double>(sp.defaultValue));
        spArray.add(juce::var(spObj));
    }
    obj->setProperty("sourceParams", spArray);

    obj->setProperty("transportMode", static_cast<int>(transportMode));
    obj->setProperty("loopMode", static_cast<int>(loopMode));
    obj->setProperty("speed", static_cast<double>(speed));
    obj->setProperty("reverse", reverse);
    obj->setProperty("startOffset", static_cast<double>(startOffset));
    obj->setProperty("inPoint", static_cast<double>(inPoint));
    obj->setProperty("outPoint", static_cast<double>(outPoint));
    obj->setProperty("beatSnap", beatSnap);
    obj->setProperty("beatSnapMode", static_cast<int>(beatSnapMode));
    obj->setProperty("autopilotAction", static_cast<int>(autopilotAction));
    obj->setProperty("autopilotDuration", static_cast<int>(autopilotDuration));
    obj->setProperty("autopilotCustomBeats", autopilotCustomBeats);
    obj->setProperty("autopilotSpecificCol", autopilotSpecificCol);

    // Video properties
    obj->setProperty("clipOpacity", static_cast<double>(clipOpacity));
    obj->setProperty("clipWidth", clipWidth);
    obj->setProperty("clipHeight", clipHeight);
    obj->setProperty("blendOverride", static_cast<int>(blendOverride));
    obj->setProperty("alphaType", static_cast<int>(alphaType));
    obj->setProperty("channelR", channelR);
    obj->setProperty("channelG", channelG);
    obj->setProperty("channelB", channelB);
    obj->setProperty("channelA", channelA);

    // Transform (per-clip)
    obj->setProperty("positionX", static_cast<double>(positionX));
    obj->setProperty("positionY", static_cast<double>(positionY));
    obj->setProperty("scale", static_cast<double>(scale));
    obj->setProperty("rotation", static_cast<double>(rotation));
    obj->setProperty("anchorX", static_cast<double>(anchorX));
    obj->setProperty("anchorY", static_cast<double>(anchorY));

    // Effects
    juce::Array<juce::var> fxArray;
    for (const auto& fx : effects)
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
    obj->setProperty("effects", fxArray);

    // Cuepoints
    juce::Array<juce::var> cpArray;
    for (int i = 0; i < numCuepoints; ++i)
        cpArray.add(static_cast<double>(cuepoints[i]));
    obj->setProperty("cuepoints", cpArray);

    // MilkDrop preset playlist (P20.5)
    juce::Array<juce::var> playlistArray;
    for (const auto& pe : presetPlaylist)
    {
        auto* peObj = new juce::DynamicObject();
        peObj->setProperty("path", juce::String(pe.presetPath));
        peObj->setProperty("name", juce::String(pe.presetName));
        peObj->setProperty("mood", juce::String(pe.mood));
        peObj->setProperty("energy", static_cast<double>(pe.energy));
        playlistArray.add(juce::var(peObj));
    }
    obj->setProperty("presetPlaylist", playlistArray);
    obj->setProperty("playlistCycleMode", static_cast<int>(playlistCycleMode));
    obj->setProperty("playlistTrigger", static_cast<int>(playlistTrigger));
    obj->setProperty("playlistTriggerBeats", playlistTriggerBeats);
    obj->setProperty("playlistBlendSeconds", static_cast<double>(playlistBlendSeconds));
    obj->setProperty("playlistEnabled", playlistEnabled);

    // P24.5: Content lock
    if (contentLocked)
        obj->setProperty("contentLocked", true);

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
        hasAlpha = static_cast<bool>(obj->getProperty("hasAlpha"));

        // Image sequence
        sequenceFiles.clear();
        if (auto* seqArray = obj->getProperty("sequenceFiles").getArray())
        {
            for (const auto& sf : *seqArray)
                sequenceFiles.push_back(juce::File(sf.toString()));
        }
        if (obj->hasProperty("sequenceFps"))
            sequenceFps = static_cast<float>(static_cast<double>(obj->getProperty("sequenceFps")));
        if (obj->hasProperty("beatDivision"))
            beatDivision = static_cast<float>(static_cast<double>(obj->getProperty("beatDivision")));
        if (obj->hasProperty("videoBeats"))
            videoBeats = static_cast<float>(static_cast<double>(obj->getProperty("videoBeats")));

        sourceParams.clear();
        if (auto* spArray = obj->getProperty("sourceParams").getArray())
        {
            for (const auto& spVar : *spArray)
            {
                if (auto* spObj = spVar.getDynamicObject())
                {
                    SourceParam sp;
                    sp.name = spObj->getProperty("name").toString().toStdString();
                    sp.uniformName = spObj->getProperty("uniform").toString().toStdString();
                    sp.value = static_cast<float>(static_cast<double>(spObj->getProperty("value")));
                    sp.defaultValue = static_cast<float>(static_cast<double>(spObj->getProperty("default")));
                    sourceParams.push_back(std::move(sp));
                }
            }
        }

        transportMode = static_cast<TransportMode>(static_cast<int>(obj->getProperty("transportMode")));
        loopMode = static_cast<LoopMode>(static_cast<int>(obj->getProperty("loopMode")));
        speed = static_cast<float>(static_cast<double>(obj->getProperty("speed")));
        reverse = static_cast<bool>(obj->getProperty("reverse"));
        startOffset = static_cast<float>(static_cast<double>(obj->getProperty("startOffset")));
        if (obj->hasProperty("inPoint"))
            inPoint = static_cast<float>(static_cast<double>(obj->getProperty("inPoint")));
        else
            inPoint = startOffset; // backwards compatibility
        if (obj->hasProperty("outPoint"))
            outPoint = static_cast<float>(static_cast<double>(obj->getProperty("outPoint")));
        else
            outPoint = 1.0f;
        beatSnap = static_cast<bool>(obj->getProperty("beatSnap"));
        beatSnapMode = static_cast<BeatSnapMode>(static_cast<int>(obj->getProperty("beatSnapMode")));
        // Upgrade legacy: if beatSnap is true but beatSnapMode is Off, set to Beat
        if (beatSnap && beatSnapMode == BeatSnapMode::Off)
            beatSnapMode = BeatSnapMode::Beat;
        autopilotAction = static_cast<AutopilotAction>(static_cast<int>(obj->getProperty("autopilotAction")));
        autopilotDuration = static_cast<AutopilotDuration>(static_cast<int>(obj->getProperty("autopilotDuration")));
        autopilotCustomBeats = static_cast<int>(obj->getProperty("autopilotCustomBeats"));
        autopilotSpecificCol = static_cast<int>(obj->getProperty("autopilotSpecificCol"));

        // Video properties (guarded for backward compatibility with old presets)
        if (obj->hasProperty("clipOpacity"))
            clipOpacity = static_cast<float>(static_cast<double>(obj->getProperty("clipOpacity")));
        if (obj->hasProperty("clipWidth"))
            clipWidth = static_cast<int>(obj->getProperty("clipWidth"));
        if (obj->hasProperty("clipHeight"))
            clipHeight = static_cast<int>(obj->getProperty("clipHeight"));
        if (obj->hasProperty("blendOverride"))
            blendOverride = static_cast<BlendOverride>(static_cast<int>(obj->getProperty("blendOverride")));
        if (obj->hasProperty("alphaType"))
            alphaType = static_cast<AlphaType>(static_cast<int>(obj->getProperty("alphaType")));
        if (obj->hasProperty("channelR"))
            channelR = static_cast<bool>(obj->getProperty("channelR"));
        if (obj->hasProperty("channelG"))
            channelG = static_cast<bool>(obj->getProperty("channelG"));
        if (obj->hasProperty("channelB"))
            channelB = static_cast<bool>(obj->getProperty("channelB"));
        if (obj->hasProperty("channelA"))
            channelA = static_cast<bool>(obj->getProperty("channelA"));

        // Transform (per-clip)
        if (obj->hasProperty("positionX"))
            positionX = static_cast<float>(static_cast<double>(obj->getProperty("positionX")));
        if (obj->hasProperty("positionY"))
            positionY = static_cast<float>(static_cast<double>(obj->getProperty("positionY")));
        if (obj->hasProperty("scale"))
            scale = static_cast<float>(static_cast<double>(obj->getProperty("scale")));
        if (obj->hasProperty("rotation"))
            rotation = static_cast<float>(static_cast<double>(obj->getProperty("rotation")));
        if (obj->hasProperty("anchorX"))
            anchorX = static_cast<float>(static_cast<double>(obj->getProperty("anchorX")));
        if (obj->hasProperty("anchorY"))
            anchorY = static_cast<float>(static_cast<double>(obj->getProperty("anchorY")));

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
                    if (fxObj->hasProperty("dryWet"))
                        slot.dryWet = static_cast<float>(static_cast<double>(fxObj->getProperty("dryWet")));
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

        // MilkDrop preset playlist (P20.5)
        presetPlaylist.clear();
        if (auto* playlistArray = obj->getProperty("presetPlaylist").getArray())
        {
            for (const auto& peVar : *playlistArray)
            {
                if (auto* peObj = peVar.getDynamicObject())
                {
                    PresetEntry pe;
                    pe.presetPath = peObj->getProperty("path").toString().toStdString();
                    pe.presetName = peObj->getProperty("name").toString().toStdString();
                    pe.mood = peObj->getProperty("mood").toString().toStdString();
                    pe.energy = static_cast<float>(static_cast<double>(peObj->getProperty("energy")));
                    presetPlaylist.push_back(std::move(pe));
                }
            }
        }
        if (obj->hasProperty("playlistCycleMode"))
            playlistCycleMode = static_cast<PlaylistCycleMode>(static_cast<int>(obj->getProperty("playlistCycleMode")));
        if (obj->hasProperty("playlistTrigger"))
            playlistTrigger = static_cast<PlaylistTrigger>(static_cast<int>(obj->getProperty("playlistTrigger")));
        if (obj->hasProperty("playlistTriggerBeats"))
            playlistTriggerBeats = static_cast<int>(obj->getProperty("playlistTriggerBeats"));
        if (obj->hasProperty("playlistBlendSeconds"))
            playlistBlendSeconds = static_cast<float>(static_cast<double>(obj->getProperty("playlistBlendSeconds")));
        if (obj->hasProperty("playlistEnabled"))
            playlistEnabled = static_cast<bool>(obj->getProperty("playlistEnabled"));

        // P24.5: Content lock
        if (obj->hasProperty("contentLocked"))
            contentLocked = static_cast<bool>(obj->getProperty("contentLocked"));
    }
}
