#pragma once
#include "model/Deck.h"
#include "model/Routine.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ScalarParams.h"
#include "connect/ConnSerialization.h"
#include <juce_core/juce_core.h>
#include <array>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>

struct Composition;

// The only place that names which Composition field backs each CompScalar
// (s166 spec section 2.2's exact phrasing). Forward-declared here (defined
// below the struct, where its fields are visible) so Composition::eff() --
// an inline member defined inside the class body -- can call it; ordinary
// name lookup for a free function needs the declaration to precede its use
// textually, unlike a class's own later-declared members.
RelaxedFloat& manualRef(Composition& c, CompScalar s);

// Composition: the complete app state saved to disk.
// Contains all decks, global effects, global settings.
struct Composition
{
    // === Identity ===
    std::string name = "Untitled";
    juce::File filePath; // Where this composition is saved

    // === Decks ===
    std::vector<Deck> decks;
    // Written by the message thread (deck switch, deck remove / undo, load); read by the httplib thread (/api/status,
    // /api/composition, /api/state): a relaxed atomic (Pitfall 63). The GL thread never reads it -- it derives its
    // index from the acquire-loaded deck pointer (Renderer::renderOpenGL).
    RelaxedInt activeDeckIndex = 0;

    // === Global Effects (post-composite chain) ===
    std::vector<Clip::EffectSlot> globalEffects;

    // === Routines (s-rta-0926 routines slice 1, plan-routines-s1-final.md 3.1-3.2) ===
    // Composition-owned routines (D9 "inside the thing it belongs to") and the
    // pad bank that fires them. The bank references a routine by uuid; a
    // routine no slot references is erased by the slot helpers below.
    static constexpr int kRoutineBankSize = 8;   // matches MacroBank::kNumMacros
    std::vector<Routine> routines;
    std::vector<RoutineSlot> routineBank;
    // Set by fromVar when a bank entry had to be dropped (its routine is not in
    // the file, its slot is out of range or taken twice) -- the app shows it
    // once; never silent. Not serialized.
    std::string routineLoadNote;

    // === Composition Master ===
    // Lane tsan (s-rta-1002; Pitfall 63): the 9 manualRef scalars (master opacity / speed / signal, the comp
    // transform) are RelaxedFloat -- message-thread writers, GL-thread reads through eff().
    RelaxedFloat masterOpacity = 1.0f;
    RelaxedFloat masterSpeed = 1.0f;       // Global speed multiplier
    // Master Signal depth (s-rta-0925 mastersignal Step 1): 1 = every
    // signal->parameter connection moves the controls it drives fully;
    // 0 = every one of those controls sits at its hand value. Backs
    // CompScalar::Signal; scaled in via SignalDepth.h::applyDepth at the
    // one point where a signal enters each chain (ConnectionEngine::
    // evaluate for non-Macro sources, MacroBank::updateValues, v1
    // MappingEngine::processFrame) -- never on any GL thread (Boris Q2:
    // effects/sources reading the beat clock or audio uniforms directly
    // keep pulsing at 0%).
    RelaxedFloat masterSignal = 1.0f;

    // === CrossFader ===
    float crossfaderPhase = 0.5f;   // [0,1] A↔B
    enum class CrossfaderBlendMode : uint8_t { Alpha, Add, Multiply };
    CrossfaderBlendMode crossfaderBlendMode = CrossfaderBlendMode::Alpha;
    enum class CrossfaderBehaviour : uint8_t { Cut, Smooth };
    CrossfaderBehaviour crossfaderBehaviour = CrossfaderBehaviour::Cut;
    enum class CrossfaderCurve : uint8_t { Linear, EaseInOut, SCurve };
    CrossfaderCurve crossfaderCurve = CrossfaderCurve::Linear;

    // === Transform (composition-level, applied to final output) ===
    RelaxedFloat compPositionX = 0.0f;
    RelaxedFloat compPositionY = 0.0f;
    RelaxedFloat compScale = 1.0f;         // 1.0 = 100%
    RelaxedFloat compRotation = 0.0f;      // Degrees
    RelaxedFloat compAnchorX = 0.0f;
    RelaxedFloat compAnchorY = 0.0f;

    // === Connections (s167-l2) ===
    // One ParamConnection + LiveValue twin per CompScalar. Opacity targets
    // masterOpacity -- an owner amendment received during this lane rules
    // Composition opacity is ONE knob (final = masterOpacity * layerOpacity
    // * clipOpacity); the model's old separate per-composition opacity
    // field merged into masterOpacity and was removed (s-rta-0923 lane 3
    // plan section 4.6; see ScalarParams.h's CompScalar comment).
    // eff()/manualRef() are the only places that name which struct field
    // backs each CompScalar.
    std::array<ParamConnection, static_cast<size_t>(CompScalar::Count)> scalarConns;
    std::array<LiveValue, static_cast<size_t>(CompScalar::Count)> scalarLive;
    float eff(CompScalar s) const
    {
        return scalarLive[static_cast<size_t>(s)].effective(manualRef(const_cast<Composition&>(*this), s));
    }

    // === Connect settings (s167-l2) ===
    // Composition-level "connect" settings (owner D14/D15): how long a
    // release-less grip (MIDI/OSC/HTTP) survives after its last write before
    // the signal takes back over, and how long a hand-back glide runs after
    // any grip releases. Global to the whole composition, not per-connection
    // -- keeps ParamConnection to exactly the owner's per-connection list.
    float gripHoldMs = 250.0f;
    float handBackGlideMs = 120.0f;

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
        routines.clear();
        routineBank.clear();
        routineLoadNote.clear();
        masterOpacity = 1.0f;
        masterSignal = 1.0f;
        globalTransitionSpeed = 0.3f;
        bpmMultiplier = 1;
        quantizeMode = QuantizeMode::Off;
    }

    // === Active Deck Access ===
    // ONE load of the index: the range check and the subscript see the same value.
    Deck* getActiveDeck()
    {
        const int idx = activeDeckIndex.load();
        if (idx >= 0 && idx < static_cast<int>(decks.size()))
            return &decks[static_cast<size_t>(idx)];
        return nullptr;
    }

    const Deck* getActiveDeck() const
    {
        const int idx = activeDeckIndex.load();
        if (idx >= 0 && idx < static_cast<int>(decks.size()))
            return &decks[static_cast<size_t>(idx)];
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

    // Append a fully-formed deck (e.g. loaded from a deck file) under a fresh id.
    // Returns its index. Caller is responsible for the GL fence (push_back reallocates).
    int appendDeck(Deck deck)
    {
        deck.id = nextDeckId_++;
        decks.push_back(std::move(deck));
        return static_cast<int>(decks.size()) - 1;
    }

    // === Routine bank ===
    const Routine* routineInSlot(int slot) const
    {
        for (const auto& s : routineBank)
            if (s.slot == slot)
                for (const auto& r : routines)
                    if (r.uuid == s.uuid)
                        return &r;
        return nullptr;
    }

    Routine* routineInSlot(int slot)
    {
        return const_cast<Routine*>(static_cast<const Composition&>(*this).routineInSlot(slot));
    }

    // -1 when every pad is taken.
    int firstFreeRoutineSlot() const
    {
        for (int slot = 0; slot < kRoutineBankSize; ++slot)
            if (std::none_of(routineBank.begin(), routineBank.end(),
                             [slot](const RoutineSlot& s) { return s.slot == slot; }))
                return slot;
        return -1;
    }

    // Puts routine `uuid` (already in `routines`) on `slot`, replacing any occupant; the
    // replaced routine is erased unless another slot still references it.
    bool assignRoutineSlot(int slot, const std::string& uuid)
    {
        if (slot < 0 || slot >= kRoutineBankSize)
            return false;
        if (std::none_of(routines.begin(), routines.end(),
                         [&](const Routine& r) { return r.uuid == uuid; }))
            return false;

        std::string replaced;
        auto it = std::find_if(routineBank.begin(), routineBank.end(),
                               [slot](const RoutineSlot& s) { return s.slot == slot; });
        if (it != routineBank.end())
        {
            replaced = it->uuid;
            it->uuid = uuid;
        }
        else
        {
            routineBank.push_back(RoutineSlot{ slot, uuid });
        }
        if (!replaced.empty() && replaced != uuid)
            eraseRoutineIfUnreferenced(replaced);
        return true;
    }

    // Frees `slot`; its routine is erased unless another slot still references it.
    bool removeRoutineSlot(int slot)
    {
        auto it = std::find_if(routineBank.begin(), routineBank.end(),
                               [slot](const RoutineSlot& s) { return s.slot == slot; });
        if (it == routineBank.end())
            return false;
        const std::string uuid = it->uuid;
        routineBank.erase(it);
        eraseRoutineIfUnreferenced(uuid);
        return true;
    }

    // === Serialization ===
    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", juce::String(name));
        obj->setProperty("activeDeckIndex", activeDeckIndex.load());
        obj->setProperty("masterOpacity", static_cast<double>(masterOpacity));
        obj->setProperty("globalTransitionSpeed", static_cast<double>(globalTransitionSpeed));
        obj->setProperty("bpmMultiplier", bpmMultiplier);
        obj->setProperty("quantizeMode", static_cast<int>(quantizeMode));
        obj->setProperty("outputWidth", outputWidth);
        obj->setProperty("outputHeight", outputHeight);
        obj->setProperty("outputDisplay", outputDisplay);

        // Composition master + video
        obj->setProperty("masterSpeed", static_cast<double>(masterSpeed));
        obj->setProperty("masterSignal", static_cast<double>(masterSignal));

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

            juce::Array<juce::var> connsArray;
            for (size_t p = 0; p < fx.paramConns.size(); ++p)
            {
                if (!fx.paramConns[p].isConnected())
                    continue;
                auto connVar = ConnSerialization::toVar(fx.paramConns[p]);
                connVar.getDynamicObject()->setProperty("p", static_cast<int>(p));
                connsArray.add(connVar);
            }
            if (!connsArray.isEmpty())
                fxObj->setProperty("conns", connsArray);
            if (fx.dryWetConn.isConnected())
                fxObj->setProperty("dryWetConn", ConnSerialization::toVar(fx.dryWetConn));

            fxArray.add(juce::var(fxObj));
        }
        obj->setProperty("globalEffects", fxArray);

        // s167-l2: per-scalar connection map, sparse -- only written if
        // something is connected.
        auto scalarConnsVar = ConnSerialization::scalarsToVar<CompScalar>(scalarConns, compScalarDefs());
        if (!scalarConnsVar.isVoid())
            obj->setProperty("conns", scalarConnsVar);

        // Composition-level connect settings (owner D14/D15).
        auto* connectObj = new juce::DynamicObject();
        connectObj->setProperty("gripHoldMs", static_cast<double>(gripHoldMs));
        connectObj->setProperty("handBackGlideMs", static_cast<double>(handBackGlideMs));
        obj->setProperty("connect", juce::var(connectObj));

        // Routines (s-rta-0926): both keys ALWAYS written (an empty array is fine).
        juce::Array<juce::var> routineArray;
        for (const auto& r : routines)
            routineArray.add(r.toVar());
        obj->setProperty("routines", routineArray);
        juce::Array<juce::var> bankArray;
        for (const auto& s : routineBank)
        {
            auto* slotObj = new juce::DynamicObject();
            slotObj->setProperty("slot", s.slot);
            slotObj->setProperty("uuid", juce::String(s.uuid));
            bankArray.add(juce::var(slotObj));
        }
        obj->setProperty("routineBank", bankArray);

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
            // s-rta-0926b plan4 S4: guarded like masterSpeed below -- the canvas size is the render
            // size now, and a JSON without these keys used to load 0x0.
            if (obj->hasProperty("outputWidth"))
                outputWidth = static_cast<int>(obj->getProperty("outputWidth"));
            if (obj->hasProperty("outputHeight"))
                outputHeight = static_cast<int>(obj->getProperty("outputHeight"));
            if (outputWidth <= 0 || outputHeight <= 0)
            {
                outputWidth = 1920;
                outputHeight = 1080;
            }
            outputDisplay = static_cast<int>(obj->getProperty("outputDisplay"));

            // Composition master + video (guarded for backward compatibility with old presets)
            if (obj->hasProperty("masterSpeed"))
                masterSpeed = static_cast<float>(static_cast<double>(obj->getProperty("masterSpeed")));
            // Guarded the same way (not masterOpacity's unguarded read at
            // masterOpacity's assignment above): an old composition without
            // this key must load 1.0 (Boris Q3), which the field's own
            // default already gives -- this line simply doesn't touch it.
            if (obj->hasProperty("masterSignal"))
                masterSignal = static_cast<float>(static_cast<double>(obj->getProperty("masterSignal")));
            // The old separate per-composition opacity key (pre-lane-3
            // presets) is a known, deliberately unrecognized key now --
            // s-rta-0923 lane 3 plan section 4.6: it merged into
            // masterOpacity (ruling 11). hasProperty-guarded loads simply
            // ignore unrecognized keys, so old files still load.

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

            // L3: nextDeckId_ resets to its default on every Composition constructed
            // by fromVar; without this, a post-load addDeck()/appendDeck() re-mints an
            // id a loaded deck already holds.
            for (const auto& deck : decks)
                nextDeckId_ = std::max(nextDeckId_, deck.id + 1u);

            // F1 (s-rta-0926b plan6): a file saved by a build whose New Deck left every deck at id 0 carries
            // DUPLICATE deck ids; LayerStateKey keys per-layer GL history by (deckId, layerId), so two decks sharing an id
            // alias each other's temporal buffers. Re-mint any repeat (the bump above already put nextDeckId_ past every
            // loaded id).
            {
                std::vector<uint32_t> seen;
                for (auto& deck : decks)
                {
                    if (std::find(seen.begin(), seen.end(), deck.id) != seen.end())
                        deck.id = nextDeckId_++;
                    seen.push_back(deck.id);
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
                        slot.resizeParams(slot.paramValues.size());
                        if (auto* connsArray = fxObj->getProperty("conns").getArray())
                        {
                            for (const auto& cv : *connsArray)
                            {
                                if (auto* cvObj = cv.getDynamicObject())
                                {
                                    int p = static_cast<int>(cvObj->getProperty("p"));
                                    if (p >= 0 && static_cast<size_t>(p) < slot.paramConns.size())
                                        ConnSerialization::fromVar(slot.paramConns[static_cast<size_t>(p)], cv);
                                }
                            }
                        }
                        if (fxObj->hasProperty("dryWetConn"))
                            ConnSerialization::fromVar(slot.dryWetConn, fxObj->getProperty("dryWetConn"));
                        globalEffects.push_back(std::move(slot));
                    }
                }
            }

            if (obj->hasProperty("conns"))
                ConnSerialization::scalarsFromVar<CompScalar>(scalarConns, compScalarDefs(), obj->getProperty("conns"));

            if (auto* connectObj = obj->getProperty("connect").getDynamicObject())
            {
                if (connectObj->hasProperty("gripHoldMs"))
                    gripHoldMs = static_cast<float>(static_cast<double>(connectObj->getProperty("gripHoldMs")));
                if (connectObj->hasProperty("handBackGlideMs"))
                    handBackGlideMs = static_cast<float>(static_cast<double>(connectObj->getProperty("handBackGlideMs")));
            }

            // Routines (s-rta-0926): cleared first, then read guarded -- a file saved before
            // routines existed loads with none. A bank entry whose routine is not in the file
            // (or whose slot is out of range / already taken) is dropped and counted into
            // routineLoadNote, never silently.
            routines.clear();
            routineBank.clear();
            routineLoadNote.clear();
            if (auto* routineArray = obj->getProperty("routines").getArray())
                for (const auto& rv : *routineArray)
                    routines.push_back(Routine::fromVar(rv));
            int droppedSlots = 0;
            if (auto* bankArray = obj->getProperty("routineBank").getArray())
            {
                for (const auto& sv : *bankArray)
                {
                    auto* slotObj = sv.getDynamicObject();
                    if (!slotObj) { ++droppedSlots; continue; }
                    RoutineSlot s;
                    s.slot = static_cast<int>(slotObj->getProperty("slot"));
                    s.uuid = slotObj->getProperty("uuid").toString().toStdString();
                    const bool slotOk = s.slot >= 0 && s.slot < kRoutineBankSize
                        && std::none_of(routineBank.begin(), routineBank.end(),
                                        [&](const RoutineSlot& o) { return o.slot == s.slot; });
                    const bool routineOk = std::any_of(routines.begin(), routines.end(),
                                                       [&](const Routine& r) { return r.uuid == s.uuid; });
                    if (slotOk && routineOk)
                        routineBank.push_back(std::move(s));
                    else
                        ++droppedSlots;
                }
            }
            if (droppedSlots > 0)
                routineLoadNote = std::to_string(droppedSlots)
                    + (droppedSlots == 1 ? " routine pad was" : " routine pads were")
                    + " left empty: the routine it pointed at is not in this file";
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

    void eraseRoutineIfUnreferenced(const std::string& uuid)
    {
        if (std::any_of(routineBank.begin(), routineBank.end(),
                        [&](const RoutineSlot& s) { return s.uuid == uuid; }))
            return;
        routines.erase(std::remove_if(routines.begin(), routines.end(),
                                      [&](const Routine& r) { return r.uuid == uuid; }),
                       routines.end());
    }
};

inline RelaxedFloat& manualRef(Composition& c, CompScalar s)
{
    switch (s)
    {
        case CompScalar::Opacity:  return c.masterOpacity;   // see the CompScalar comment above
        case CompScalar::Speed:    return c.masterSpeed;
        case CompScalar::PosX:     return c.compPositionX;
        case CompScalar::PosY:     return c.compPositionY;
        case CompScalar::Scale:    return c.compScale;
        case CompScalar::Rotation: return c.compRotation;
        case CompScalar::AnchorX:  return c.compAnchorX;
        case CompScalar::AnchorY:  return c.compAnchorY;
        case CompScalar::Signal:   return c.masterSignal;
        case CompScalar::Count:    break;
    }
    static RelaxedFloat dummy = 0.0f;   // unreachable for a valid enumerator
    return dummy;
}
