#include "recording/PerfState.h"

namespace
{
    juce::var mapIntFloatToVar(const std::map<int, float>& m)
    {
        juce::Array<juce::var> arr;
        for (const auto& [i, v] : m)
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("i", i);
            obj->setProperty("v", static_cast<double>(v));
            arr.add(juce::var(obj));
        }
        return arr;
    }

    std::map<int, float> mapIntFloatFromVar(const juce::var& v)
    {
        std::map<int, float> m;
        if (auto* arr = v.getArray())
            for (const auto& e : *arr)
                if (auto* obj = e.getDynamicObject())
                    m[static_cast<int>(obj->getProperty("i"))] =
                        static_cast<float>(static_cast<double>(obj->getProperty("v")));
        return m;
    }

    juce::var mapStringFloatToVar(const std::map<std::string, float>& m)
    {
        juce::Array<juce::var> arr;
        for (const auto& [key, v] : m)
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("key", juce::String(key));
            obj->setProperty("v", static_cast<double>(v));
            arr.add(juce::var(obj));
        }
        return arr;
    }

    std::map<std::string, float> mapStringFloatFromVar(const juce::var& v)
    {
        std::map<std::string, float> m;
        if (auto* arr = v.getArray())
            for (const auto& e : *arr)
                if (auto* obj = e.getDynamicObject())
                    m[obj->getProperty("key").toString().toStdString()] =
                        static_cast<float>(static_cast<double>(obj->getProperty("v")));
        return m;
    }
}

juce::var PerfState::ClipRuntime::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("clip", juce::String(clip));
    obj->setProperty("playing", playing);
    obj->setProperty("playhead", playheadPosition);
    obj->setProperty("effectParams", mapIntFloatToVar(effectParams));
    obj->setProperty("scalars", mapStringFloatToVar(scalars));
    return juce::var(obj);
}

PerfState::ClipRuntime PerfState::ClipRuntime::fromVar(const juce::var& v)
{
    ClipRuntime c;
    if (auto* obj = v.getDynamicObject())
    {
        c.clip = obj->getProperty("clip").toString().toStdString();
        c.playing = static_cast<bool>(obj->getProperty("playing"));
        c.playheadPosition = static_cast<double>(obj->getProperty("playhead"));
        c.effectParams = mapIntFloatFromVar(obj->getProperty("effectParams"));
        c.scalars = mapStringFloatFromVar(obj->getProperty("scalars"));
    }
    return c;
}

juce::var PerfState::LayerRuntime::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("layer", juce::String(layer));
    obj->setProperty("activeClipColumn", activeClipColumn);
    obj->setProperty("previousClipColumn", previousClipColumn);
    obj->setProperty("crossfadeProgress", static_cast<double>(crossfadeProgress));
    obj->setProperty("pendingTriggerColumn", pendingTriggerColumn);
    obj->setProperty("pendingTriggerSnapOverride", pendingTriggerSnapOverride);
    obj->setProperty("opacity", static_cast<double>(opacity));
    obj->setProperty("visible", visible);
    obj->setProperty("bypassed", bypassed);
    obj->setProperty("solo", solo);
    obj->setProperty("muted", muted);
    obj->setProperty("autopilotEnabled", autopilotEnabled);
    obj->setProperty("effectParams", mapIntFloatToVar(effectParams));

    juce::Array<juce::var> clipArr;
    for (const auto& [col, clip] : clips)
    {
        juce::var cv = clip.toVar();
        cv.getDynamicObject()->setProperty("col", col);
        clipArr.add(cv);
    }
    obj->setProperty("clips", clipArr);
    return juce::var(obj);
}

PerfState::LayerRuntime PerfState::LayerRuntime::fromVar(const juce::var& v)
{
    LayerRuntime l;
    if (auto* obj = v.getDynamicObject())
    {
        l.layer = obj->getProperty("layer").toString().toStdString();
        l.activeClipColumn = static_cast<int>(obj->getProperty("activeClipColumn"));
        l.previousClipColumn = static_cast<int>(obj->getProperty("previousClipColumn"));
        l.crossfadeProgress = static_cast<float>(static_cast<double>(obj->getProperty("crossfadeProgress")));
        l.pendingTriggerColumn = static_cast<int>(obj->getProperty("pendingTriggerColumn"));
        l.pendingTriggerSnapOverride = static_cast<int>(obj->getProperty("pendingTriggerSnapOverride"));
        l.opacity = static_cast<float>(static_cast<double>(obj->getProperty("opacity")));
        l.visible = static_cast<bool>(obj->getProperty("visible"));
        l.bypassed = static_cast<bool>(obj->getProperty("bypassed"));
        l.solo = static_cast<bool>(obj->getProperty("solo"));
        l.muted = static_cast<bool>(obj->getProperty("muted"));
        l.autopilotEnabled = static_cast<bool>(obj->getProperty("autopilotEnabled"));
        l.effectParams = mapIntFloatFromVar(obj->getProperty("effectParams"));

        if (auto* clipArr = obj->getProperty("clips").getArray())
        {
            for (const auto& cv : *clipArr)
            {
                if (auto* cObj = cv.getDynamicObject())
                {
                    const int col = static_cast<int>(cObj->getProperty("col"));
                    l.clips[col] = ClipRuntime::fromVar(cv);
                }
            }
        }
    }
    return l;
}

juce::var PerfState::DeckRuntime::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("deck", juce::String(deck));

    juce::Array<juce::var> layerArr;
    for (const auto& [i, layer] : layers)
    {
        juce::var lv = layer.toVar();
        lv.getDynamicObject()->setProperty("i", i);
        layerArr.add(lv);
    }
    obj->setProperty("layers", layerArr);
    return juce::var(obj);
}

PerfState::DeckRuntime PerfState::DeckRuntime::fromVar(const juce::var& v)
{
    DeckRuntime d;
    if (auto* obj = v.getDynamicObject())
    {
        d.deck = obj->getProperty("deck").toString().toStdString();
        if (auto* layerArr = obj->getProperty("layers").getArray())
        {
            for (const auto& lv : *layerArr)
            {
                if (auto* lObj = lv.getDynamicObject())
                {
                    const int i = static_cast<int>(lObj->getProperty("i"));
                    d.layers[i] = LayerRuntime::fromVar(lv);
                }
            }
        }
    }
    return d;
}

juce::var PerfState::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("activeDeckIndex", activeDeckIndex);
    obj->setProperty("quantizeMode", quantizeMode);
    obj->setProperty("bpm", static_cast<double>(bpm));
    obj->setProperty("audioAction", juce::String(audioAction));

    juce::Array<juce::var> deckArr;
    for (const auto& [i, deck] : decks)
    {
        juce::var dv = deck.toVar();
        dv.getDynamicObject()->setProperty("i", i);
        deckArr.add(dv);
    }
    obj->setProperty("decks", deckArr);
    return juce::var(obj);
}

PerfState PerfState::fromVar(const juce::var& v)
{
    PerfState p;
    if (auto* obj = v.getDynamicObject())
    {
        p.activeDeckIndex = static_cast<int>(obj->getProperty("activeDeckIndex"));
        p.quantizeMode = static_cast<int>(obj->getProperty("quantizeMode"));
        p.bpm = static_cast<float>(static_cast<double>(obj->getProperty("bpm")));
        p.audioAction = obj->getProperty("audioAction").toString().toStdString();

        if (auto* deckArr = obj->getProperty("decks").getArray())
        {
            for (const auto& dv : *deckArr)
            {
                if (auto* dObj = dv.getDynamicObject())
                {
                    const int i = static_cast<int>(dObj->getProperty("i"));
                    p.decks[i] = DeckRuntime::fromVar(dv);
                }
            }
        }
    }
    return p;
}
