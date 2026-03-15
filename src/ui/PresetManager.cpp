#include "PresetManager.h"

// ── Enum string tables ──────────────────────────────────────────────

static const char* const kSourceNames[] = {
    "RMS", "Peak", "RmsDB",
    "LUFS", "DynamicRange", "TransientDensity",
    "SpectralCentroid", "SpectralFlux", "SpectralFlatness", "SpectralRolloff",
    "BandSub", "BandBass", "BandLowMid", "BandMid", "BandHighMid", "BandPresence", "BandBrilliance",
    "OnsetStrength", "BeatPhase", "BPM",
    "StructuralState",
    "DominantPitch", "PitchConfidence", "DetectedKey", "HarmonicChange",
    "MFCC0", "MFCC1", "MFCC2", "MFCC3", "MFCC4", "MFCC5", "MFCC6",
    "MFCC7", "MFCC8", "MFCC9", "MFCC10", "MFCC11", "MFCC12",
    "ChromaC", "ChromaCs", "ChromaD", "ChromaDs", "ChromaE", "ChromaF",
    "ChromaFs", "ChromaG", "ChromaGs", "ChromaA", "ChromaAs", "ChromaB"
};

static constexpr int kNumSources = static_cast<int>(MappingSource::Count);

static const char* const kCurveNames[] = {
    "Linear", "Exponential", "Logarithmic", "SCurve", "Stepped"
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
    root->setProperty("version", 1);

    // Serialize effects
    juce::Array<juce::var> effectsArray;
    for (int i = 0; i < chain.getNumEffects(); ++i)
    {
        const Effect* fx = const_cast<EffectChain&>(chain).getEffect(i);
        if (fx == nullptr) continue;

        auto* fxObj = new juce::DynamicObject();
        fxObj->setProperty("name", fx->getName());
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
        mObj->setProperty("targetEffect", static_cast<int>(m->targetEffectId));
        mObj->setProperty("targetParam", static_cast<int>(m->targetParamIndex));
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
                                MappingEngine& engine)
{
    if (!file.existsAsFile())
        return false;

    auto json = juce::JSON::parse(file.loadFileAsString());
    if (!json.isObject())
        return false;

    auto* root = json.getDynamicObject();
    if (root == nullptr)
        return false;

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

            // Match by name to handle reordered chains
            juce::String savedName = fxObj->getProperty("name").toString();
            if (savedName != fx->getName())
            {
                // Try to find the matching effect by name
                bool found = false;
                for (int j = 0; j < chain.getNumEffects(); ++j)
                {
                    Effect* candidate = chain.getEffect(j);
                    if (candidate && candidate->getName() == savedName)
                    {
                        fx = candidate;
                        found = true;
                        break;
                    }
                }
                if (!found) continue;
            }

            fx->setEnabled(fxObj->getProperty("enabled"));

            auto paramsVar = fxObj->getProperty("params");
            if (auto* paramsArray = paramsVar.getArray())
            {
                for (int p = 0; p < paramsArray->size() && p < fx->getNumParams(); ++p)
                {
                    auto pVar = (*paramsArray)[p];
                    auto* pObj = pVar.getDynamicObject();
                    if (pObj == nullptr) continue;

                    float val = static_cast<float>(static_cast<double>(pObj->getProperty("value")));
                    fx->setParamValue(p, val);
                }
            }
        }
    }

    // Rebuild mappings
    engine.clearAll();

    auto mappingsVar = root->getProperty("mappings");
    if (auto* mappingsArray = mappingsVar.getArray())
    {
        for (const auto& mVar : *mappingsArray)
        {
            auto* mObj = mVar.getDynamicObject();
            if (mObj == nullptr) continue;

            Mapping m;
            m.source         = stringToSource(mObj->getProperty("source").toString());
            m.targetEffectId = static_cast<uint32_t>(static_cast<int>(mObj->getProperty("targetEffect")));
            m.targetParamIndex = static_cast<uint32_t>(static_cast<int>(mObj->getProperty("targetParam")));
            m.curve          = stringToCurve(mObj->getProperty("curve").toString());
            m.inputMin       = static_cast<float>(static_cast<double>(mObj->getProperty("inputMin")));
            m.inputMax       = static_cast<float>(static_cast<double>(mObj->getProperty("inputMax")));
            m.outputMin      = static_cast<float>(static_cast<double>(mObj->getProperty("outputMin")));
            m.outputMax      = static_cast<float>(static_cast<double>(mObj->getProperty("outputMax")));
            m.smoothing      = static_cast<float>(static_cast<double>(mObj->getProperty("smoothing")));
            m.enabled        = mObj->getProperty("enabled");

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
                              MappingEngine& engine)
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
        loadPreset(tempFile, chain, engine);
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
