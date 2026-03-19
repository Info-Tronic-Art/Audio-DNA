#pragma once
#include "model/Deck.h"
#include <juce_core/juce_core.h>
#include <string>
#include <vector>
#include <cstdint>

// Composition: the complete app state saved to disk.
// Contains all decks, global effects, global settings.
struct Composition
{
    // === Identity ===
    std::string name = "Untitled";
    juce::File filePath; // Where this composition is saved

    // === Decks ===
    std::vector<Deck> decks;
    int activeDeckIndex = 0;

    // === Global Effects (post-composite chain) ===
    std::vector<Clip::EffectSlot> globalEffects;

    // === Composition Master ===
    float masterOpacity = 1.0f;
    float masterSpeed = 1.0f;       // Global speed multiplier

    // === Video (Composition-level) ===
    float compOpacity = 1.0f;       // Composition video opacity

    // === CrossFader ===
    float crossfaderPhase = 0.5f;   // [0,1] A↔B
    enum class CrossfaderBlendMode : uint8_t { Alpha, Add, Multiply };
    CrossfaderBlendMode crossfaderBlendMode = CrossfaderBlendMode::Alpha;
    enum class CrossfaderBehaviour : uint8_t { Cut, Smooth };
    CrossfaderBehaviour crossfaderBehaviour = CrossfaderBehaviour::Cut;
    enum class CrossfaderCurve : uint8_t { Linear, EaseInOut, SCurve };
    CrossfaderCurve crossfaderCurve = CrossfaderCurve::Linear;

    // === Transform (composition-level, applied to final output) ===
    float compPositionX = 0.0f;
    float compPositionY = 0.0f;
    float compScale = 1.0f;         // 1.0 = 100%
    float compRotation = 0.0f;      // Degrees
    float compAnchorX = 0.0f;
    float compAnchorY = 0.0f;

    // === Global Settings ===
    float globalTransitionSpeed = 0.3f; // seconds
    int bpmMultiplier = 1; // -4 = ÷4, -2 = ÷2, 1 = ×1, 2 = ×2, 4 = ×4

    enum class QuantizeMode : uint8_t { Off, NextBeat, NextDownbeat };
    QuantizeMode quantizeMode = QuantizeMode::Off;

    // === Autopilot (composition-level) ===
    enum class AutopilotDirection : uint8_t { Rewind, Off, Forward, Random };
    AutopilotDirection autopilotDirection = AutopilotDirection::Off;
    enum class AutopilotDurationMode : uint8_t { LongestClip, ClipTransport, Custom };
    AutopilotDurationMode autopilotDurationMode = AutopilotDurationMode::LongestClip;
    int autopilotClipLoops = 1;
    bool autopilotLoop = false;
    int autopilotMasterLayer = -1;  // -1 = Off

    // === Output Settings ===
    int outputWidth = 1920;
    int outputHeight = 1080;
    int outputDisplay = -1; // -1 = no external output

    // === Initialization ===
    void initDefault()
    {
        name = "Untitled";
        filePath = juce::File();
        decks.clear();
        Deck deck;
        deck.name = "Deck 1";
        deck.id = 0;
        deck.initDefault();
        decks.push_back(std::move(deck));
        activeDeckIndex = 0;
        globalEffects.clear();
        masterOpacity = 1.0f;
        globalTransitionSpeed = 0.3f;
        bpmMultiplier = 1;
        quantizeMode = QuantizeMode::Off;
    }

    // === Active Deck Access ===
    Deck* getActiveDeck()
    {
        if (activeDeckIndex >= 0 && activeDeckIndex < static_cast<int>(decks.size()))
            return &decks[static_cast<size_t>(activeDeckIndex)];
        return nullptr;
    }

    const Deck* getActiveDeck() const
    {
        if (activeDeckIndex >= 0 && activeDeckIndex < static_cast<int>(decks.size()))
            return &decks[static_cast<size_t>(activeDeckIndex)];
        return nullptr;
    }

    // === Deck Management ===
    void addDeck(const std::string& deckName = "New Deck")
    {
        Deck deck;
        deck.name = deckName;
        deck.id = nextDeckId_++;
        deck.initDefault();
        decks.push_back(std::move(deck));
    }

    bool removeDeck(int index)
    {
        if (index < 0 || index >= static_cast<int>(decks.size()))
            return false;
        if (decks.size() <= 1)
            return false; // Must have at least one deck
        decks.erase(decks.begin() + index);
        if (activeDeckIndex >= static_cast<int>(decks.size()))
            activeDeckIndex = static_cast<int>(decks.size()) - 1;
        return true;
    }

    // === Serialization ===
    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", juce::String(name));
        obj->setProperty("activeDeckIndex", activeDeckIndex);
        obj->setProperty("masterOpacity", static_cast<double>(masterOpacity));
        obj->setProperty("globalTransitionSpeed", static_cast<double>(globalTransitionSpeed));
        obj->setProperty("bpmMultiplier", bpmMultiplier);
        obj->setProperty("quantizeMode", static_cast<int>(quantizeMode));
        obj->setProperty("outputWidth", outputWidth);
        obj->setProperty("outputHeight", outputHeight);
        obj->setProperty("outputDisplay", outputDisplay);

        // Decks
        juce::Array<juce::var> deckArray;
        for (const auto& deck : decks)
            deckArray.add(deck.toVar());
        obj->setProperty("decks", deckArray);

        // Global effects
        juce::Array<juce::var> fxArray;
        for (const auto& fx : globalEffects)
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
        obj->setProperty("globalEffects", fxArray);

        return juce::var(obj);
    }

    void fromVar(const juce::var& v)
    {
        if (auto* obj = v.getDynamicObject())
        {
            name = obj->getProperty("name").toString().toStdString();
            activeDeckIndex = static_cast<int>(obj->getProperty("activeDeckIndex"));
            masterOpacity = static_cast<float>(static_cast<double>(obj->getProperty("masterOpacity")));
            globalTransitionSpeed = static_cast<float>(static_cast<double>(obj->getProperty("globalTransitionSpeed")));
            bpmMultiplier = static_cast<int>(obj->getProperty("bpmMultiplier"));
            quantizeMode = static_cast<QuantizeMode>(static_cast<int>(obj->getProperty("quantizeMode")));
            outputWidth = static_cast<int>(obj->getProperty("outputWidth"));
            outputHeight = static_cast<int>(obj->getProperty("outputHeight"));
            outputDisplay = static_cast<int>(obj->getProperty("outputDisplay"));

            decks.clear();
            if (auto* deckArray = obj->getProperty("decks").getArray())
            {
                for (const auto& deckVar : *deckArray)
                {
                    Deck deck;
                    deck.fromVar(deckVar);
                    decks.push_back(std::move(deck));
                }
            }

            globalEffects.clear();
            if (auto* fxArray = obj->getProperty("globalEffects").getArray())
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
                        globalEffects.push_back(std::move(slot));
                    }
                }
            }
        }
    }

    // === File I/O ===
    bool saveToFile(const juce::File& file) const
    {
        auto json = juce::JSON::toString(toVar());
        return file.replaceWithText(json);
    }

    bool loadFromFile(const juce::File& file)
    {
        auto json = file.loadFileAsString();
        if (json.isEmpty()) return false;
        auto parsed = juce::JSON::parse(json);
        if (parsed.isVoid()) return false;
        fromVar(parsed);
        filePath = file;
        return true;
    }

private:
    uint32_t nextDeckId_ = 100;
};
