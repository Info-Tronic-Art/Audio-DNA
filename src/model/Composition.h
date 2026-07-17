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

    // === Per-Type Autopilot (P20) ===
    // Separate timers/settings for Opaque, Transparent, and FX layers.
    struct PerTypeAutopilotConfig
    {
        // Opaque layers
        int opaqueCycleBeats = 16;
        bool opaquePlayUntilEnd = false;

        // Transparent layers
        int transparentCycleBeats = 8;
        int transparentMaxLayers = 2;
        bool transparentRandomize = true;

        // Effect layers
        int effectCycleBeats = 4;
        int effectMaxLayers = 2;
        bool effectRandomize = true;

        // Global overrides
        bool perTypeEnabled = false;    // false = use existing per-layer autopilot
        bool globalRandomize = false;
        bool loopAutopilot = true;
    };
    PerTypeAutopilotConfig perTypeAutopilot;

    // === Genre-Aware Automation (P23) ===
    bool autoPresetOnGenre = false;         // Auto-switch visual preset on genre change
    bool smartAutopilotEnabled = false;     // Energy-aware clip selection
    bool structuralSceneEnabled = false;    // Auto-switch decks on structural transitions

    // Per-genre deck assignment: which deck to switch to when genre is detected
    // Index = genre ID (0-7), value = deck index (-1 = no switch)
    int genreDeckAssignment[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

    // Per-genre effect preset: name of FX preset to load when genre is detected
    std::string genrePresetNames[8] = {};

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

        // Composition master + video
        obj->setProperty("masterSpeed", static_cast<double>(masterSpeed));
        obj->setProperty("compOpacity", static_cast<double>(compOpacity));

        // Crossfader
        obj->setProperty("crossfaderPhase", static_cast<double>(crossfaderPhase));
        obj->setProperty("crossfaderBlendMode", static_cast<int>(crossfaderBlendMode));
        obj->setProperty("crossfaderBehaviour", static_cast<int>(crossfaderBehaviour));
        obj->setProperty("crossfaderCurve", static_cast<int>(crossfaderCurve));

        // Transform (composition-level)
        obj->setProperty("compPositionX", static_cast<double>(compPositionX));
        obj->setProperty("compPositionY", static_cast<double>(compPositionY));
        obj->setProperty("compScale", static_cast<double>(compScale));
        obj->setProperty("compRotation", static_cast<double>(compRotation));
        obj->setProperty("compAnchorX", static_cast<double>(compAnchorX));
        obj->setProperty("compAnchorY", static_cast<double>(compAnchorY));

        // Autopilot (composition-level)
        obj->setProperty("autopilotDirection", static_cast<int>(autopilotDirection));
        obj->setProperty("autopilotDurationMode", static_cast<int>(autopilotDurationMode));
        obj->setProperty("autopilotClipLoops", autopilotClipLoops);
        obj->setProperty("autopilotLoop", autopilotLoop);
        obj->setProperty("autopilotMasterLayer", autopilotMasterLayer);

        // Per-Type Autopilot (P20)
        obj->setProperty("ptaOpaqueCycleBeats", perTypeAutopilot.opaqueCycleBeats);
        obj->setProperty("ptaOpaquePlayUntilEnd", perTypeAutopilot.opaquePlayUntilEnd);
        obj->setProperty("ptaTransparentCycleBeats", perTypeAutopilot.transparentCycleBeats);
        obj->setProperty("ptaTransparentMaxLayers", perTypeAutopilot.transparentMaxLayers);
        obj->setProperty("ptaTransparentRandomize", perTypeAutopilot.transparentRandomize);
        obj->setProperty("ptaEffectCycleBeats", perTypeAutopilot.effectCycleBeats);
        obj->setProperty("ptaEffectMaxLayers", perTypeAutopilot.effectMaxLayers);
        obj->setProperty("ptaEffectRandomize", perTypeAutopilot.effectRandomize);
        obj->setProperty("ptaPerTypeEnabled", perTypeAutopilot.perTypeEnabled);
        obj->setProperty("ptaGlobalRandomize", perTypeAutopilot.globalRandomize);
        obj->setProperty("ptaLoopAutopilot", perTypeAutopilot.loopAutopilot);

        // Genre-aware automation (P23)
        obj->setProperty("autoPresetOnGenre", autoPresetOnGenre);
        obj->setProperty("smartAutopilotEnabled", smartAutopilotEnabled);
        obj->setProperty("structuralSceneEnabled", structuralSceneEnabled);
        juce::Array<juce::var> genreDeckArray;
        for (int i = 0; i < 8; ++i)
            genreDeckArray.add(genreDeckAssignment[i]);
        obj->setProperty("genreDeckAssignment", genreDeckArray);
        juce::Array<juce::var> genrePresetArray;
        for (int i = 0; i < 8; ++i)
            genrePresetArray.add(juce::String(genrePresetNames[i]));
        obj->setProperty("genrePresetNames", genrePresetArray);

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
            fxObj->setProperty("dryWet", static_cast<double>(fx.dryWet));
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

            // Composition master + video (guarded for backward compatibility with old presets)
            if (obj->hasProperty("masterSpeed"))
                masterSpeed = static_cast<float>(static_cast<double>(obj->getProperty("masterSpeed")));
            if (obj->hasProperty("compOpacity"))
                compOpacity = static_cast<float>(static_cast<double>(obj->getProperty("compOpacity")));

            // Crossfader
            if (obj->hasProperty("crossfaderPhase"))
                crossfaderPhase = static_cast<float>(static_cast<double>(obj->getProperty("crossfaderPhase")));
            if (obj->hasProperty("crossfaderBlendMode"))
                crossfaderBlendMode = static_cast<CrossfaderBlendMode>(static_cast<int>(obj->getProperty("crossfaderBlendMode")));
            if (obj->hasProperty("crossfaderBehaviour"))
                crossfaderBehaviour = static_cast<CrossfaderBehaviour>(static_cast<int>(obj->getProperty("crossfaderBehaviour")));
            if (obj->hasProperty("crossfaderCurve"))
                crossfaderCurve = static_cast<CrossfaderCurve>(static_cast<int>(obj->getProperty("crossfaderCurve")));

            // Transform (composition-level)
            if (obj->hasProperty("compPositionX"))
                compPositionX = static_cast<float>(static_cast<double>(obj->getProperty("compPositionX")));
            if (obj->hasProperty("compPositionY"))
                compPositionY = static_cast<float>(static_cast<double>(obj->getProperty("compPositionY")));
            if (obj->hasProperty("compScale"))
                compScale = static_cast<float>(static_cast<double>(obj->getProperty("compScale")));
            if (obj->hasProperty("compRotation"))
                compRotation = static_cast<float>(static_cast<double>(obj->getProperty("compRotation")));
            if (obj->hasProperty("compAnchorX"))
                compAnchorX = static_cast<float>(static_cast<double>(obj->getProperty("compAnchorX")));
            if (obj->hasProperty("compAnchorY"))
                compAnchorY = static_cast<float>(static_cast<double>(obj->getProperty("compAnchorY")));

            // Autopilot (composition-level)
            if (obj->hasProperty("autopilotDirection"))
                autopilotDirection = static_cast<AutopilotDirection>(static_cast<int>(obj->getProperty("autopilotDirection")));
            if (obj->hasProperty("autopilotDurationMode"))
                autopilotDurationMode = static_cast<AutopilotDurationMode>(static_cast<int>(obj->getProperty("autopilotDurationMode")));
            if (obj->hasProperty("autopilotClipLoops"))
                autopilotClipLoops = static_cast<int>(obj->getProperty("autopilotClipLoops"));
            if (obj->hasProperty("autopilotLoop"))
                autopilotLoop = static_cast<bool>(obj->getProperty("autopilotLoop"));
            if (obj->hasProperty("autopilotMasterLayer"))
                autopilotMasterLayer = static_cast<int>(obj->getProperty("autopilotMasterLayer"));

            // Per-Type Autopilot (P20)
            if (obj->hasProperty("ptaOpaqueCycleBeats"))
                perTypeAutopilot.opaqueCycleBeats = static_cast<int>(obj->getProperty("ptaOpaqueCycleBeats"));
            if (obj->hasProperty("ptaOpaquePlayUntilEnd"))
                perTypeAutopilot.opaquePlayUntilEnd = static_cast<bool>(obj->getProperty("ptaOpaquePlayUntilEnd"));
            if (obj->hasProperty("ptaTransparentCycleBeats"))
                perTypeAutopilot.transparentCycleBeats = static_cast<int>(obj->getProperty("ptaTransparentCycleBeats"));
            if (obj->hasProperty("ptaTransparentMaxLayers"))
                perTypeAutopilot.transparentMaxLayers = static_cast<int>(obj->getProperty("ptaTransparentMaxLayers"));
            if (obj->hasProperty("ptaTransparentRandomize"))
                perTypeAutopilot.transparentRandomize = static_cast<bool>(obj->getProperty("ptaTransparentRandomize"));
            if (obj->hasProperty("ptaEffectCycleBeats"))
                perTypeAutopilot.effectCycleBeats = static_cast<int>(obj->getProperty("ptaEffectCycleBeats"));
            if (obj->hasProperty("ptaEffectMaxLayers"))
                perTypeAutopilot.effectMaxLayers = static_cast<int>(obj->getProperty("ptaEffectMaxLayers"));
            if (obj->hasProperty("ptaEffectRandomize"))
                perTypeAutopilot.effectRandomize = static_cast<bool>(obj->getProperty("ptaEffectRandomize"));
            if (obj->hasProperty("ptaPerTypeEnabled"))
                perTypeAutopilot.perTypeEnabled = static_cast<bool>(obj->getProperty("ptaPerTypeEnabled"));
            if (obj->hasProperty("ptaGlobalRandomize"))
                perTypeAutopilot.globalRandomize = static_cast<bool>(obj->getProperty("ptaGlobalRandomize"));
            if (obj->hasProperty("ptaLoopAutopilot"))
                perTypeAutopilot.loopAutopilot = static_cast<bool>(obj->getProperty("ptaLoopAutopilot"));

            // Genre-aware automation (P23)
            if (obj->hasProperty("autoPresetOnGenre"))
                autoPresetOnGenre = static_cast<bool>(obj->getProperty("autoPresetOnGenre"));
            if (obj->hasProperty("smartAutopilotEnabled"))
                smartAutopilotEnabled = static_cast<bool>(obj->getProperty("smartAutopilotEnabled"));
            if (obj->hasProperty("structuralSceneEnabled"))
                structuralSceneEnabled = static_cast<bool>(obj->getProperty("structuralSceneEnabled"));
            if (auto* genreDeckArray = obj->getProperty("genreDeckAssignment").getArray())
            {
                int gi = 0;
                for (const auto& gd : *genreDeckArray)
                    if (gi < 8) genreDeckAssignment[gi++] = static_cast<int>(gd);
            }
            if (auto* genrePresetArray = obj->getProperty("genrePresetNames").getArray())
            {
                int gi = 0;
                for (const auto& gp : *genrePresetArray)
                    if (gi < 8) genrePresetNames[gi++] = gp.toString().toStdString();
            }

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
                        if (fxObj->hasProperty("dryWet"))
                            slot.dryWet = static_cast<float>(static_cast<double>(fxObj->getProperty("dryWet")));
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
