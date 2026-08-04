#include "PresetManager.h"
#include <iostream>

// ── Enum string tables ──────────────────────────────────────────────

static const char* const kSourceNames[] = {
    "RMS", "Peak", "RmsDB",
    "LUFS", "DynamicRange", "TransientDensity",
    "SpectralCentroid", "SpectralFlux", "SpectralFlatness", "SpectralRolloff",
    "BandSub", "BandBass", "BandLowMid", "BandMid", "BandHighMid", "BandPresence", "BandBrilliance",
    "OnsetStrength", "BeatPhase", "BPM", "BarPhase", "PhrasePhase", "BarCount",
    "StructuralState",
    "DominantPitch", "PitchConfidence", "DetectedKey", "HarmonicChange",
    "MFCC0", "MFCC1", "MFCC2", "MFCC3", "MFCC4", "MFCC5", "MFCC6",
    "MFCC7", "MFCC8", "MFCC9", "MFCC10", "MFCC11", "MFCC12",
    "ChromaC", "ChromaCs", "ChromaD", "ChromaDs", "ChromaE", "ChromaF",
    "ChromaFs", "ChromaG", "ChromaGs", "ChromaA", "ChromaAs", "ChromaB"
};

static constexpr int kNumSources = static_cast<int>(MappingSource::Count);

static const char* const kCurveNames[] = {
    "Linear", "Exponential", "Logarithmic", "SCurve", "Stepped",
    "CircularIn", "CircularOut", "CircularInOut",
    "BackIn", "BackOut", "BackInOut",
    "ElasticIn", "ElasticOut", "ElasticInOut",
    "BounceIn", "BounceOut", "BounceInOut",
    "CubicIn", "CubicOut", "CubicInOut",
    "SineIn", "SineOut", "SineInOut",
    "Hold"
};

static constexpr int kNumCurves = static_cast<int>(MappingCurve::Count);

juce::String PresetManager::sourceToString(MappingSource source)
{
    int idx = static_cast<int>(source);
    if (idx >= 0 && idx < kNumSources)
        return kSourceNames[idx];
    return "RMS";
}

MappingSource PresetManager::stringToSource(const juce::String& str)
{
    for (int i = 0; i < kNumSources; ++i)
        if (str == kSourceNames[i])
            return static_cast<MappingSource>(i);
    return MappingSource::RMS;
}

juce::String PresetManager::curveToString(MappingCurve curve)
{
    int idx = static_cast<int>(curve);
    if (idx >= 0 && idx < kNumCurves)
        return kCurveNames[idx];
    return "Linear";
}

MappingCurve PresetManager::stringToCurve(const juce::String& str)
{
    for (int i = 0; i < kNumCurves; ++i)
        if (str == kCurveNames[i])
            return static_cast<MappingCurve>(i);
    return MappingCurve::Linear;
}

// ── Save ─────────────────────────────────────────────────────────────

bool PresetManager::savePreset(const juce::File& file,
                                const juce::String& presetName,
                                const EffectChain& chain,
                                const MappingEngine& engine)
{
    auto* root = new juce::DynamicObject();

    root->setProperty("name", presetName);
    root->setProperty("version", 2);

    // Serialize effects
    juce::Array<juce::var> effectsArray;
    for (int i = 0; i < chain.getNumEffects(); ++i)
    {
        const Effect* fx = const_cast<EffectChain&>(chain).getEffect(i);
        if (fx == nullptr) continue;

        auto* fxObj = new juce::DynamicObject();
        fxObj->setProperty("name", fx->getName());
        fxObj->setProperty("shader", fx->getShaderName());
        fxObj->setProperty("enabled", fx->isEnabled());
        fxObj->setProperty("order", fx->getOrder());

        juce::Array<juce::var> paramsArray;
        for (int p = 0; p < fx->getNumParams(); ++p)
        {
            const auto& param = fx->getParam(p);
            auto* pObj = new juce::DynamicObject();
            pObj->setProperty("name", juce::String(param.name));
            pObj->setProperty("value", static_cast<double>(param.value));
            paramsArray.add(juce::var(pObj));
        }
        fxObj->setProperty("params", paramsArray);
        effectsArray.add(juce::var(fxObj));
    }
    root->setProperty("effects", effectsArray);

    // Serialize mappings
    juce::Array<juce::var> mappingsArray;
    for (int i = 0; i < engine.getNumMappings(); ++i)
    {
        const Mapping* m = engine.getMapping(i);
        if (m == nullptr) continue;

        auto* mObj = new juce::DynamicObject();
        mObj->setProperty("source", sourceToString(m->source));

        // Legacy raw-index fields — ALWAYS written, both for backward
        // compatibility with hand-edited/external tooling and as the
        // last-resort fallback for v1 loaders reading this v2 file.
        mObj->setProperty("targetEffect", static_cast<int>(m->targetEffectId));
        mObj->setProperty("targetParam", static_cast<int>(m->targetParamIndex));

        // D1 dual-key targeting: shaderName/uniformName (primary) survive an
        // EffectLibrary re-grouping that silently shifts raw chain indices —
        // the bug this fixes. displayName/paramName ride along as fallback.
        // Resolved off the LIVE chain, not the Mapping struct (which has no
        // name fields — MappingTypes.h).
        Effect* targetFx = const_cast<EffectChain&>(chain).getEffect(static_cast<int>(m->targetEffectId));
        if (targetFx != nullptr)
        {
            mObj->setProperty("targetEffectKey", targetFx->getShaderName());
            mObj->setProperty("targetEffectName", targetFx->getName());

            int paramIdx = static_cast<int>(m->targetParamIndex);
            if (paramIdx >= 0 && paramIdx < targetFx->getNumParams())
            {
                const auto& targetParam = targetFx->getParam(paramIdx);
                mObj->setProperty("targetParamKey", juce::String(targetParam.uniformName));
                mObj->setProperty("targetParamName", juce::String(targetParam.name));
            }
        }

        mObj->setProperty("curve", curveToString(m->curve));
        mObj->setProperty("inputMin", static_cast<double>(m->inputMin));
        mObj->setProperty("inputMax", static_cast<double>(m->inputMax));
        mObj->setProperty("outputMin", static_cast<double>(m->outputMin));
        mObj->setProperty("outputMax", static_cast<double>(m->outputMax));
        mObj->setProperty("smoothing", static_cast<double>(m->smoothing));
        mObj->setProperty("enabled", m->enabled);
        mappingsArray.add(juce::var(mObj));
    }
    root->setProperty("mappings", mappingsArray);

    // Write to file
    juce::var rootVar(root);
    juce::String json = juce::JSON::toString(rootVar);
    return file.replaceWithText(json);
}

// ── Load ─────────────────────────────────────────────────────────────

bool PresetManager::loadPreset(const juce::File& file,
                                EffectChain& chain,
                                MappingEngine& engine,
                                LoadStats* stats)
{
    LoadStats localStats;
    if (stats == nullptr)
        stats = &localStats;
    *stats = LoadStats{};

    if (!file.existsAsFile())
        return false;

    auto json = juce::JSON::parse(file.loadFileAsString());
    if (!json.isObject())
        return false;

    auto* root = json.getDynamicObject();
    if (root == nullptr)
        return false;

    // D4: version is read for diagnostics/forward-tolerance only — nothing
    // below branches on it except the legacyFile flag (no keys written <
    // version 2). A newer-than-supported file still gets a best-effort load
    // since DynamicObject carries unknown properties inertly.
    int fileVersion = root->hasProperty("version")
                     ? static_cast<int>(root->getProperty("version"))
                     : 1;
    if (fileVersion > 2)
        std::cerr << "[PresetManager] Preset \"" << file.getFileName()
                   << "\" has version " << fileVersion
                   << ", newer than supported version 2 — loading best-effort." << std::endl;
    stats->legacyFile = fileVersion < 2;

    // Apply effect states
    auto effectsVar = root->getProperty("effects");
    if (auto* effectsArray = effectsVar.getArray())
    {
        for (int i = 0; i < effectsArray->size() && i < chain.getNumEffects(); ++i)
        {
            auto fxVar = (*effectsArray)[i];
            auto* fxObj = fxVar.getDynamicObject();
            if (fxObj == nullptr) continue;

            Effect* fx = chain.getEffect(i);
            if (fx == nullptr) continue;

            // Match by shaderName first (stable across display-name
            // relabeling), falling back to display name — legacy files have
            // no "shader" field, so savedShader is empty and this collapses
            // to the original name-only match. Handles reordered/re-grouped
            // chains (D1, preset-retarget-fix).
            juce::String savedShader = fxObj->getProperty("shader").toString();
            juce::String savedName = fxObj->getProperty("name").toString();
            bool matches = (!savedShader.isEmpty() && fx->getShaderName() == savedShader)
                        || (savedShader.isEmpty() && savedName == fx->getName());

            if (!matches)
            {
                bool found = false;
                if (!savedShader.isEmpty())
                {
                    for (int j = 0; j < chain.getNumEffects() && !found; ++j)
                    {
                        Effect* candidate = chain.getEffect(j);
                        if (candidate != nullptr && candidate->getShaderName() == savedShader)
                        {
                            fx = candidate;
                            found = true;
                        }
                    }
                }
                if (!found)
                {
                    for (int j = 0; j < chain.getNumEffects() && !found; ++j)
                    {
                        Effect* candidate = chain.getEffect(j);
                        if (candidate != nullptr && candidate->getName() == savedName)
                        {
                            fx = candidate;
                            found = true;
                        }
                    }
                }
                if (!found) continue;
            }

            fx->setEnabled(fxObj->getProperty("enabled"));

            auto paramsVar = fxObj->getProperty("params");
            if (auto* paramsArray = paramsVar.getArray())
            {
                for (int p = 0; p < paramsArray->size(); ++p)
                {
                    auto pVar = (*paramsArray)[p];
                    auto* pObj = pVar.getDynamicObject();
                    if (pObj == nullptr) continue;

                    float val = static_cast<float>(static_cast<double>(pObj->getProperty("value")));

                    // N4: restore by name first — survives a param-list
                    // reordering/insertion, which the old positional
                    // restore silently mis-targeted. Falls back to position
                    // for legacy files (no "name") or an unrecognized name.
                    juce::String savedParamName = pObj->getProperty("name").toString();
                    int targetIdx = -1;
                    if (!savedParamName.isEmpty())
                    {
                        for (int q = 0; q < fx->getNumParams(); ++q)
                        {
                            if (juce::String(fx->getParam(q).name) == savedParamName)
                            {
                                targetIdx = q;
                                break;
                            }
                        }
                    }
                    if (targetIdx < 0 && p < fx->getNumParams())
                        targetIdx = p;

                    if (targetIdx >= 0)
                        fx->setParamValue(targetIdx, val);
                }
            }
        }
    }

    // Rebuild mappings
    engine.clearAll();

    auto mappingsVar = root->getProperty("mappings");
    if (auto* mappingsArray = mappingsVar.getArray())
    {
        stats->mappingsTotal = mappingsArray->size();

        for (const auto& mVar : *mappingsArray)
        {
            auto* mObj = mVar.getDynamicObject();
            if (mObj == nullptr) continue;

            Mapping m;
            m.source    = stringToSource(mObj->getProperty("source").toString());
            m.curve     = stringToCurve(mObj->getProperty("curve").toString());
            m.inputMin  = static_cast<float>(static_cast<double>(mObj->getProperty("inputMin")));
            m.inputMax  = static_cast<float>(static_cast<double>(mObj->getProperty("inputMax")));
            m.outputMin = static_cast<float>(static_cast<double>(mObj->getProperty("outputMin")));
            m.outputMax = static_cast<float>(static_cast<double>(mObj->getProperty("outputMax")));
            m.smoothing = static_cast<float>(static_cast<double>(mObj->getProperty("smoothing")));
            m.enabled   = mObj->getProperty("enabled");

            // D1: key fields present means this is a v2 mapping — resolve
            // key-first, name-fallback, and DROP (do NOT fall back to the
            // raw ints) if neither resolves. A stale-but-in-range raw index
            // is exactly the silent-mistarget bug this fixes; falling back
            // to it here would reintroduce it for every v2 file whose
            // target effect/param was renamed AND moved.
            bool hasKeyFields = mObj->hasProperty("targetEffectKey")
                              || mObj->hasProperty("targetEffectName");
            bool resolved = false;

            if (hasKeyFields)
            {
                juce::String effKey    = mObj->getProperty("targetEffectKey").toString();
                juce::String effName   = mObj->getProperty("targetEffectName").toString();
                juce::String paramKey  = mObj->getProperty("targetParamKey").toString();
                juce::String paramName = mObj->getProperty("targetParamName").toString();

                int effIdx = -1;
                bool effByKey = false;
                if (!effKey.isEmpty())
                {
                    for (int j = 0; j < chain.getNumEffects(); ++j)
                    {
                        Effect* candidate = chain.getEffect(j);
                        if (candidate != nullptr && candidate->getShaderName() == effKey)
                        {
                            effIdx = j;
                            effByKey = true;
                            break;
                        }
                    }
                }
                if (effIdx < 0 && !effName.isEmpty())
                {
                    for (int j = 0; j < chain.getNumEffects(); ++j)
                    {
                        Effect* candidate = chain.getEffect(j);
                        if (candidate != nullptr && candidate->getName() == effName)
                        {
                            effIdx = j;
                            break;
                        }
                    }
                }

                if (effIdx >= 0)
                {
                    Effect* fx = chain.getEffect(effIdx);
                    int paramIdx = -1;
                    bool paramByKey = false;
                    if (fx != nullptr && !paramKey.isEmpty())
                    {
                        for (int q = 0; q < fx->getNumParams(); ++q)
                        {
                            if (juce::String(fx->getParam(q).uniformName) == paramKey)
                            {
                                paramIdx = q;
                                paramByKey = true;
                                break;
                            }
                        }
                    }
                    if (fx != nullptr && paramIdx < 0 && !paramName.isEmpty())
                    {
                        for (int q = 0; q < fx->getNumParams(); ++q)
                        {
                            if (juce::String(fx->getParam(q).name) == paramName)
                            {
                                paramIdx = q;
                                break;
                            }
                        }
                    }

                    if (paramIdx >= 0)
                    {
                        m.targetEffectId = static_cast<uint32_t>(effIdx);
                        m.targetParamIndex = static_cast<uint32_t>(paramIdx);
                        resolved = true;
                        if (effByKey && paramByKey)
                            stats->resolvedByKey++;
                        else
                            stats->resolvedByName++;
                    }
                }

                if (!resolved)
                {
                    stats->dropped++;
                    stats->droppedDescriptions.add(
                        "Mapping (" + sourceToString(m.source) + ") target \""
                        + (effName.isEmpty() ? effKey : effName)
                        + "\" could not be resolved — dropped.");
                    continue;
                }
            }
            else
            {
                // D6: v1 file, no keys — no automatic remap. Validate the
                // raw index against the live chain: in-range loads exactly
                // as v1 always did (pre-existing behavior, R1 accepted
                // residual), out-of-range is now dropped instead of
                // silently mis-targeting whatever effect landed there.
                int rawEffect = static_cast<int>(mObj->getProperty("targetEffect"));
                int rawParam  = static_cast<int>(mObj->getProperty("targetParam"));

                Effect* fx = chain.getEffect(rawEffect);
                if (fx != nullptr && rawParam >= 0 && rawParam < fx->getNumParams())
                {
                    m.targetEffectId = static_cast<uint32_t>(rawEffect);
                    m.targetParamIndex = static_cast<uint32_t>(rawParam);
                    resolved = true;
                    stats->legacyIndex++;
                }
                else
                {
                    stats->dropped++;
                    stats->droppedDescriptions.add(
                        "Legacy mapping (" + sourceToString(m.source) + ") target index "
                        + juce::String(rawEffect) + "/" + juce::String(rawParam)
                        + " out of range — dropped.");
                    continue;
                }
            }

            engine.addMapping(m);
        }
    }

    return true;
}

// ── Directory ────────────────────────────────────────────────────────

juce::File PresetManager::getPresetsDirectory()
{
    auto appData = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto dir = appData.getChildFile("AudioDNA").getChildFile("Presets");
    dir.createDirectory();
    return dir;
}

juce::File PresetManager::getFxSaveDirectory()
{
    auto appData = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto dir = appData.getChildFile("AudioDNA").getChildFile("FX Saves");
    dir.createDirectory();
    return dir;
}

juce::File PresetManager::getDeckDirectory()
{
    auto appData = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto dir = appData.getChildFile("AudioDNA").getChildFile("Decks");
    dir.createDirectory();
    return dir;
}

juce::Array<juce::File> PresetManager::getAvailablePresets()
{
    auto dir = getPresetsDirectory();
    return dir.findChildFiles(juce::File::findFiles, false, "*.json");
}

// ── Deck save/load ────────────────────────────────────────────────

bool PresetManager::saveDeck(const juce::File& file,
                              const DeckState& deck,
                              const EffectChain& chain,
                              const MappingEngine& engine)
{
    // Save FX preset to a temp string first
    auto fxFile = file.getSiblingFile("_temp_fx_.json");
    savePreset(fxFile, "deck_fx", chain, engine);
    auto fxJson = fxFile.loadFileAsString();
    fxFile.deleteFile();

    // Parse the FX JSON and embed it in the deck
    auto fxVar = juce::JSON::parse(fxJson);

    auto deckObj = std::make_unique<juce::DynamicObject>();
    deckObj->setProperty("type", "deck");
    deckObj->setProperty("audioFile", deck.audioFile.getFullPathName());
    deckObj->setProperty("imageFile", deck.imageFile.getFullPathName());
    deckObj->setProperty("imageFolderPath", deck.imageFolderPath.getFullPathName());
    deckObj->setProperty("slideshowBeatsPerImage", deck.slideshowBeatsPerImage);
    deckObj->setProperty("beatRandomCount", deck.beatRandomCount);
    deckObj->setProperty("beatRandomEnabled", deck.beatRandomEnabled);
    deckObj->setProperty("audioSourceMode", deck.audioSourceMode);
    deckObj->setProperty("viewportResolution", deck.viewportResolution);
    deckObj->setProperty("outputDisplay", deck.outputDisplay);
    deckObj->setProperty("inputGain", static_cast<double>(deck.inputGain));
    deckObj->setProperty("masterVideoLevel", static_cast<double>(deck.masterVideoLevel));
    deckObj->setProperty("showAudioPanel", deck.showAudioPanel);
    deckObj->setProperty("showFxPanel", deck.showFxPanel);
    deckObj->setProperty("showWavePanel", deck.showWavePanel);
    deckObj->setProperty("showKeysPanel", deck.showKeysPanel);
    deckObj->setProperty("showPresetsPanel", deck.showPresetsPanel);

    // Slot assignments
    juce::Array<juce::var> slotsArray;
    for (const auto& s : deck.slotFiles)
        slotsArray.add(s);
    deckObj->setProperty("slots", slotsArray);

    // Embed the full FX preset
    deckObj->setProperty("fx", fxVar);

    // Keyboard layout
    if (!deck.keyboardKeys.isEmpty())
        deckObj->setProperty("keyboard", deck.keyboardKeys);

    auto json = juce::JSON::toString(juce::var(deckObj.release()));
    return file.replaceWithText(json);
}

bool PresetManager::loadDeck(const juce::File& file,
                              DeckState& deck,
                              EffectChain& chain,
                              MappingEngine& engine,
                              LoadStats* stats)
{
    auto json = file.loadFileAsString();
    auto parsed = juce::JSON::parse(json);

    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr)
        return false;

    // Check it's a deck file
    if (obj->getProperty("type").toString() != "deck")
        return false;

    deck.audioFile = juce::File(obj->getProperty("audioFile").toString());
    deck.imageFile = juce::File(obj->getProperty("imageFile").toString());
    deck.imageFolderPath = juce::File(obj->getProperty("imageFolderPath").toString());
    deck.slideshowBeatsPerImage = static_cast<int>(obj->getProperty("slideshowBeatsPerImage"));
    deck.beatRandomCount = static_cast<int>(obj->getProperty("beatRandomCount"));
    deck.beatRandomEnabled = static_cast<bool>(obj->getProperty("beatRandomEnabled"));
    deck.audioSourceMode = static_cast<int>(obj->getProperty("audioSourceMode"));
    deck.viewportResolution = static_cast<int>(obj->getProperty("viewportResolution"));
    deck.outputDisplay = static_cast<int>(obj->getProperty("outputDisplay"));
    deck.inputGain = static_cast<float>(static_cast<double>(obj->getProperty("inputGain")));
    deck.masterVideoLevel = static_cast<float>(static_cast<double>(obj->getProperty("masterVideoLevel")));
    deck.showAudioPanel = obj->hasProperty("showAudioPanel") ? static_cast<bool>(obj->getProperty("showAudioPanel")) : true;
    deck.showFxPanel = obj->hasProperty("showFxPanel") ? static_cast<bool>(obj->getProperty("showFxPanel")) : true;
    deck.showWavePanel = obj->hasProperty("showWavePanel") ? static_cast<bool>(obj->getProperty("showWavePanel")) : true;
    deck.showKeysPanel = obj->hasProperty("showKeysPanel") ? static_cast<bool>(obj->getProperty("showKeysPanel")) : true;
    deck.showPresetsPanel = obj->hasProperty("showPresetsPanel") ? static_cast<bool>(obj->getProperty("showPresetsPanel")) : true;

    // Slot assignments
    deck.slotFiles.clear();
    auto* slotsArray = obj->getProperty("slots").getArray();
    if (slotsArray)
    {
        for (const auto& s : *slotsArray)
            deck.slotFiles.add(s.toString());
    }

    // Load embedded FX
    auto fxVar = obj->getProperty("fx");
    if (fxVar.isObject())
    {
        // Write to temp file and load via existing loadPreset
        auto tempFile = file.getSiblingFile("_temp_load_fx_.json");
        tempFile.replaceWithText(juce::JSON::toString(fxVar));
        loadPreset(tempFile, chain, engine, stats);
        tempFile.deleteFile();
    }

    // Keyboard layout
    deck.keyboardKeys.clear();
    if (auto* kbArray = obj->getProperty("keyboard").getArray())
    {
        for (const auto& v : *kbArray)
            deck.keyboardKeys.add(v);
    }

    return true;
}
