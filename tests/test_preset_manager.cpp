#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "ui/PresetManager.h"
#include "effects/Effect.h"
#include "effects/EffectChain.h"
#include "effects/EffectLibrary.h"
#include "mapping/MappingEngine.h"
#include "mapping/MappingTypes.h"
#include <juce_core/juce_core.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using Catch::Approx;

// ============================================================
// Helpers
// ============================================================

// Append an Effect with the given name/shaderName and (paramName, uniformName)
// pairs to chain, all params defaulting to 0.0f. Mirrors test_mapping_engine's
// makeChainWithEffect() but supports multiple named/keyed effects per chain,
// which the preset-retarget fix tests need (dual-key resolution operates
// across a whole chain, not one effect in isolation).
static void addTestEffect(EffectChain& chain,
                           const juce::String& name,
                           const juce::String& shaderName,
                           const std::vector<std::pair<std::string, std::string>>& params)
{
    auto fx = std::make_unique<Effect>(name, "test", shaderName);
    for (const auto& p : params)
        fx->addParam(p.first, p.second, 0.0f);
    chain.addEffect(std::move(fx));
}

static juce::File tempPresetFile(const juce::String& suffix)
{
    auto f = juce::File::getSpecialLocation(juce::File::tempDirectory)
                 .getChildFile("preset_manager_test_" + suffix + ".json");
    f.deleteFile();
    return f;
}

// ============================================================
// FAIL-FIRST: these must fail against unmodified PresetManager
// (raw-index mapping targets, positional param restore) and pass once the
// dual-key retarget fix (D1) and by-name param restore (N4) land.
// See .harmony/.work-packets/preset-retarget-fix.md T1/T8.
// ============================================================

TEST_CASE("FAIL-FIRST T1: mapping retargets by key after a library re-grouping shifts indices",
          "[preset][fail-first]")
{
    auto tempFile = tempPresetFile("t1");

    // Save: chain [Alpha, Beta, Gamma], one mapping -> Gamma (index 2), param 0.
    EffectChain saveChain;
    addTestEffect(saveChain, "Alpha", "alpha_shader", {{"amt", "u_alpha_amt"}});
    addTestEffect(saveChain, "Beta",  "beta_shader",  {{"amt", "u_beta_amt"}});
    addTestEffect(saveChain, "Gamma", "gamma_shader", {{"amt", "u_gamma_amt"}});

    MappingEngine saveEngine;
    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 2;   // Gamma
    m.targetParamIndex = 0;
    saveEngine.addMapping(m);

    REQUIRE(PresetManager::savePreset(tempFile, "t1", saveChain, saveEngine));

    // Load: chain re-grouped to [Alpha, Beta, Inserted, Gamma] — Gamma is now
    // at index 3. A raw-index load silently retargets "Inserted" instead.
    EffectChain loadChain;
    addTestEffect(loadChain, "Alpha",    "alpha_shader",    {{"amt", "u_alpha_amt"}});
    addTestEffect(loadChain, "Beta",     "beta_shader",     {{"amt", "u_beta_amt"}});
    addTestEffect(loadChain, "Inserted", "inserted_shader", {{"amt", "u_inserted_amt"}});
    addTestEffect(loadChain, "Gamma",    "gamma_shader",    {{"amt", "u_gamma_amt"}});

    MappingEngine loadEngine;
    REQUIRE(PresetManager::loadPreset(tempFile, loadChain, loadEngine));

    REQUIRE(loadEngine.getNumMappings() == 1);
    REQUIRE(loadEngine.getMapping(0)->targetEffectId == 3);  // Gamma's NEW index

    tempFile.deleteFile();
}

TEST_CASE("FAIL-FIRST T8: effect param values restore by name after a mid-list param insertion",
          "[preset][fail-first]")
{
    auto tempFile = tempPresetFile("t8");

    EffectChain saveChain;
    addTestEffect(saveChain, "Ripple", "ripple", {
        {"intensity", "u_ripple_intensity"},
        {"freq",      "u_ripple_freq"},
        {"speed",     "u_ripple_speed"}
    });
    saveChain.getEffect(0)->setParamValue(0, 0.1f);  // intensity
    saveChain.getEffect(0)->setParamValue(1, 0.2f);  // freq
    saveChain.getEffect(0)->setParamValue(2, 0.3f);  // speed

    MappingEngine saveEngine;
    REQUIRE(PresetManager::savePreset(tempFile, "t8", saveChain, saveEngine));

    // Load: same effect, but with a new param INSERTED between intensity and
    // freq — a positional restore lands freq's/speed's saved values one slot
    // too early.
    EffectChain loadChain;
    addTestEffect(loadChain, "Ripple", "ripple", {
        {"intensity", "u_ripple_intensity"},
        {"inserted",  "u_ripple_inserted"},
        {"freq",      "u_ripple_freq"},
        {"speed",     "u_ripple_speed"}
    });

    MappingEngine loadEngine;
    REQUIRE(PresetManager::loadPreset(tempFile, loadChain, loadEngine));

    Effect* fx = loadChain.getEffect(0);
    REQUIRE(fx->getParam(0).value == Approx(0.1f));  // intensity — unaffected by the insertion
    REQUIRE(fx->getParam(1).value == Approx(0.0f));  // inserted — untouched, stays default
    REQUIRE(fx->getParam(2).value == Approx(0.2f));  // freq — must land HERE by name
    REQUIRE(fx->getParam(3).value == Approx(0.3f));  // speed — must land HERE by name

    tempFile.deleteFile();
}

// ============================================================
// T2-T9 — remaining coverage from preset-retarget-fix.md's TEST PLAN.
// These reference PresetManager::LoadStats, which only exists once the D1
// fix lands, so they follow (rather than precede) the fail-first pair above.
// ============================================================

TEST_CASE("T2: mapping resolves by key when the display name changes but shaderName is kept",
          "[preset][retarget]")
{
    auto tempFile = tempPresetFile("t2");

    EffectChain saveChain;
    addTestEffect(saveChain, "Old Name", "fixed_shader", {{"amount", "u_fixed_amount"}});

    MappingEngine saveEngine;
    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    saveEngine.addMapping(m);
    REQUIRE(PresetManager::savePreset(tempFile, "t2", saveChain, saveEngine));

    // Same shaderName, relabeled display name.
    EffectChain loadChain;
    addTestEffect(loadChain, "New Name", "fixed_shader", {{"amount", "u_fixed_amount"}});

    MappingEngine loadEngine;
    PresetManager::LoadStats stats;
    REQUIRE(PresetManager::loadPreset(tempFile, loadChain, loadEngine, &stats));

    REQUIRE(loadEngine.getNumMappings() == 1);
    REQUIRE(loadEngine.getMapping(0)->targetEffectId == 0);
    REQUIRE(loadEngine.getMapping(0)->targetParamIndex == 0);
    REQUIRE(stats.resolvedByKey == 1);
    REQUIRE(stats.resolvedByName == 0);
    REQUIRE(stats.dropped == 0);

    tempFile.deleteFile();
}

TEST_CASE("T3: mapping resolves by display-name fallback when shaderName changes",
          "[preset][retarget]")
{
    auto tempFile = tempPresetFile("t3");

    EffectChain saveChain;
    addTestEffect(saveChain, "Fixed Name", "old_shader", {{"amount", "u_fixed_amount"}});

    MappingEngine saveEngine;
    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    saveEngine.addMapping(m);
    REQUIRE(PresetManager::savePreset(tempFile, "t3", saveChain, saveEngine));

    // shaderName changed, display name kept — key lookup misses, name hits.
    EffectChain loadChain;
    addTestEffect(loadChain, "Fixed Name", "new_shader", {{"amount", "u_fixed_amount"}});

    MappingEngine loadEngine;
    PresetManager::LoadStats stats;
    REQUIRE(PresetManager::loadPreset(tempFile, loadChain, loadEngine, &stats));

    REQUIRE(loadEngine.getNumMappings() == 1);
    REQUIRE(loadEngine.getMapping(0)->targetEffectId == 0);
    REQUIRE(loadEngine.getMapping(0)->targetParamIndex == 0);
    REQUIRE(stats.resolvedByKey == 0);
    REQUIRE(stats.resolvedByName == 1);
    REQUIRE(stats.dropped == 0);

    tempFile.deleteFile();
}

TEST_CASE("T4: mapping target absent from the chain is dropped, not silently mistargeted",
          "[preset][retarget]")
{
    auto tempFile = tempPresetFile("t4");

    EffectChain saveChain;
    addTestEffect(saveChain, "Ghost", "ghost_shader", {{"amount", "u_ghost_amount"}});

    MappingEngine saveEngine;
    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    saveEngine.addMapping(m);
    REQUIRE(PresetManager::savePreset(tempFile, "t4", saveChain, saveEngine));

    // Load chain never had "Ghost" — neither key nor display name resolves.
    EffectChain loadChain;
    addTestEffect(loadChain, "Unrelated", "unrelated_shader", {{"amount", "u_unrelated_amount"}});

    MappingEngine loadEngine;
    PresetManager::LoadStats stats;
    REQUIRE(PresetManager::loadPreset(tempFile, loadChain, loadEngine, &stats));

    REQUIRE(loadEngine.getNumMappings() == 0);
    REQUIRE(stats.dropped == 1);
    REQUIRE(stats.droppedDescriptions.size() == 1);
    REQUIRE(stats.droppedDescriptions[0].isNotEmpty());

    tempFile.deleteFile();
}

TEST_CASE("T5: hand-written v1 JSON (no keys) validates the raw index against the live chain",
          "[preset][backcompat]")
{
    SECTION("In-range raw index loads and flags legacyFile")
    {
        auto tempFile = tempPresetFile("t5_inrange");
        juce::String v1Json =
            R"({"name":"v1","version":1,"effects":[],"mappings":[)"
            R"({"source":"RMS","targetEffect":1,"targetParam":0,"curve":"Linear",)"
            R"("inputMin":0.0,"inputMax":1.0,"outputMin":0.0,"outputMax":1.0,)"
            R"("smoothing":0.15,"enabled":true}]})";
        tempFile.replaceWithText(v1Json);

        EffectChain loadChain;
        addTestEffect(loadChain, "Alpha", "alpha_shader", {{"amt", "u_alpha_amt"}});
        addTestEffect(loadChain, "Beta",  "beta_shader",  {{"amt", "u_beta_amt"}});

        MappingEngine loadEngine;
        PresetManager::LoadStats stats;
        REQUIRE(PresetManager::loadPreset(tempFile, loadChain, loadEngine, &stats));

        REQUIRE(loadEngine.getNumMappings() == 1);
        REQUIRE(loadEngine.getMapping(0)->targetEffectId == 1);
        REQUIRE(stats.legacyFile == true);
        REQUIRE(stats.legacyIndex == 1);
        REQUIRE(stats.dropped == 0);

        tempFile.deleteFile();
    }

    SECTION("Out-of-range raw index is dropped, not silently mistargeted")
    {
        auto tempFile = tempPresetFile("t5_oor");
        juce::String v1Json =
            R"({"name":"v1","version":1,"effects":[],"mappings":[)"
            R"({"source":"RMS","targetEffect":5,"targetParam":0,"curve":"Linear",)"
            R"("inputMin":0.0,"inputMax":1.0,"outputMin":0.0,"outputMax":1.0,)"
            R"("smoothing":0.15,"enabled":true}]})";
        tempFile.replaceWithText(v1Json);

        EffectChain loadChain;
        addTestEffect(loadChain, "Alpha", "alpha_shader", {{"amt", "u_alpha_amt"}});
        addTestEffect(loadChain, "Beta",  "beta_shader",  {{"amt", "u_beta_amt"}});

        MappingEngine loadEngine;
        PresetManager::LoadStats stats;
        REQUIRE(PresetManager::loadPreset(tempFile, loadChain, loadEngine, &stats));

        REQUIRE(loadEngine.getNumMappings() == 0);
        REQUIRE(stats.legacyFile == true);
        REQUIRE(stats.dropped == 1);

        tempFile.deleteFile();
    }
}

TEST_CASE("T6: premise gate — built-in effect defs have unique names/shaderNames/param+uniform names",
          "[preset][premise]")
{
    // Guards the resolution invariant D1 depends on: if this ever fails, a
    // built-in effect addition introduced a targeting-key collision.
    EffectLibrary lib;
    lib.registerDefaults();

    REQUIRE(lib.getNumEffects() > 0);

    juce::StringArray seenNames;
    juce::StringArray seenShaders;
    auto names = lib.getEffectNames();
    REQUIRE(names.size() == lib.getNumEffects());

    for (const auto& name : names)
    {
        INFO("effect name: " << name);
        REQUIRE(!seenNames.contains(name));
        seenNames.add(name);

        const auto* def = lib.getEffectDef(name);
        REQUIRE(def != nullptr);
        REQUIRE(!seenShaders.contains(def->shaderName));
        seenShaders.add(def->shaderName);

        for (size_t i = 0; i < def->params.size(); ++i)
        {
            for (size_t j = i + 1; j < def->params.size(); ++j)
            {
                INFO("params[" << i << "] vs params[" << j << "]");
                REQUIRE(def->params[i].name != def->params[j].name);
                REQUIRE(def->params[i].uniformName != def->params[j].uniformName);
            }
        }
    }
}

TEST_CASE("T7: registerDynamic rejects duplicate name/shaderName/intra-def param collisions",
          "[preset][effectlibrary]")
{
    EffectLibrary lib;
    lib.registerDefaults();

    EffectLibrary::EffectDef dupName;
    dupName.name = "Ripple";  // collides with a built-in display name
    dupName.category = "isf";
    dupName.shaderName = "unique_shader_dup_name";
    dupName.params.push_back({"amount", "u_dup_name_amount", 0.5f});
    REQUIRE(lib.registerDynamic(dupName) == false);

    EffectLibrary::EffectDef dupShader;
    dupShader.name = "Unique Display Name 1";
    dupShader.category = "isf";
    dupShader.shaderName = "ripple";  // collides with a built-in shaderName
    dupShader.params.push_back({"amount", "u_dup_shader_amount", 0.5f});
    REQUIRE(lib.registerDynamic(dupShader) == false);

    EffectLibrary::EffectDef dupParamName;
    dupParamName.name = "Dup Param Name Effect";
    dupParamName.category = "isf";
    dupParamName.shaderName = "dup_param_name_shader";
    dupParamName.params.push_back({"amount", "u_dup_a", 0.0f});
    dupParamName.params.push_back({"amount", "u_dup_b", 0.0f});  // dup param name
    REQUIRE(lib.registerDynamic(dupParamName) == false);

    EffectLibrary::EffectDef dupUniform;
    dupUniform.name = "Dup Uniform Effect";
    dupUniform.category = "isf";
    dupUniform.shaderName = "dup_uniform_shader";
    dupUniform.params.push_back({"a", "u_shared", 0.0f});
    dupUniform.params.push_back({"b", "u_shared", 0.0f});  // dup uniform name
    REQUIRE(lib.registerDynamic(dupUniform) == false);

    EffectLibrary::EffectDef clean;
    clean.name = "Totally New Effect";
    clean.category = "isf";
    clean.shaderName = "totally_new_shader";
    clean.params.push_back({"amount", "u_totally_new_amount", 0.5f});
    REQUIRE(lib.registerDynamic(clean) == true);
    REQUIRE(lib.getEffectDef("Totally New Effect") != nullptr);
}

TEST_CASE("T9: save writes version 2 and keeps the legacy int fields equal to the live indices",
          "[preset][backcompat]")
{
    auto tempFile = tempPresetFile("t9");

    EffectChain chain;
    addTestEffect(chain, "Alpha", "alpha_shader", {{"amt", "u_alpha_amt"}});
    addTestEffect(chain, "Beta",  "beta_shader",  {{"a", "u_beta_a"}, {"b", "u_beta_b"}});

    MappingEngine engine;
    Mapping m;
    m.source = MappingSource::Peak;
    m.targetEffectId = 1;      // Beta
    m.targetParamIndex = 1;    // "b"
    engine.addMapping(m);

    REQUIRE(PresetManager::savePreset(tempFile, "t9", chain, engine));

    auto parsed = juce::JSON::parse(tempFile.loadFileAsString());
    auto* root = parsed.getDynamicObject();
    REQUIRE(root != nullptr);
    REQUIRE(static_cast<int>(root->getProperty("version")) == 2);

    auto* mappingsArray = root->getProperty("mappings").getArray();
    REQUIRE(mappingsArray != nullptr);
    REQUIRE(mappingsArray->size() == 1);
    auto* mObj = (*mappingsArray)[0].getDynamicObject();
    REQUIRE(mObj != nullptr);
    REQUIRE(static_cast<int>(mObj->getProperty("targetEffect")) == 1);
    REQUIRE(static_cast<int>(mObj->getProperty("targetParam")) == 1);
    REQUIRE(mObj->getProperty("targetEffectKey").toString() == "beta_shader");
    REQUIRE(mObj->getProperty("targetParamKey").toString() == "u_beta_b");

    tempFile.deleteFile();
}

TEST_CASE("Deck round-trip: mapping resolution survives the embedded-fx save/load path",
          "[preset][deck]")
{
    auto tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getChildFile("preset_manager_test_deck.adna");
    tempFile.deleteFile();

    EffectChain saveChain;
    addTestEffect(saveChain, "Alpha", "alpha_shader", {{"amt", "u_alpha_amt"}});
    addTestEffect(saveChain, "Gamma", "gamma_shader", {{"amt", "u_gamma_amt"}});

    MappingEngine saveEngine;
    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 1;   // Gamma
    m.targetParamIndex = 0;
    saveEngine.addMapping(m);

    PresetManager::DeckState deckState;
    REQUIRE(PresetManager::saveDeck(tempFile, deckState, saveChain, saveEngine));

    // Load into a re-grouped chain — Gamma shifted from index 1 to index 2.
    EffectChain loadChain;
    addTestEffect(loadChain, "Alpha",    "alpha_shader",    {{"amt", "u_alpha_amt"}});
    addTestEffect(loadChain, "Inserted", "inserted_shader", {{"amt", "u_inserted_amt"}});
    addTestEffect(loadChain, "Gamma",    "gamma_shader",    {{"amt", "u_gamma_amt"}});

    MappingEngine loadEngine;
    PresetManager::DeckState loadedDeck;
    PresetManager::LoadStats stats;
    REQUIRE(PresetManager::loadDeck(tempFile, loadedDeck, loadChain, loadEngine, &stats));

    REQUIRE(loadEngine.getNumMappings() == 1);
    REQUIRE(loadEngine.getMapping(0)->targetEffectId == 2);
    REQUIRE(stats.resolvedByKey == 1);

    tempFile.deleteFile();
}
