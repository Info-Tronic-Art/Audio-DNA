#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/Composition.h"
#include "core/CompositionLoad.h"
#include "core/UndoManager.h"
#include "core/Command.h"
#include "core/CompositeCommand.h"
#include <algorithm>
#include <vector>

using Catch::Matchers::WithinAbs;

TEST_CASE("Composition default initialization", "[composition]")
{
    Composition comp;
    comp.initDefault();

    REQUIRE(comp.name == "Untitled");
    REQUIRE(comp.decks.size() == 1);
    REQUIRE(comp.activeDeckIndex == 0);
    REQUIRE(comp.masterOpacity == 1.0f);

    auto* deck = comp.getActiveDeck();
    REQUIRE(deck != nullptr);
    REQUIRE(deck->getNumLayers() == 3);
    REQUIRE(deck->numColumns == 12);
}

TEST_CASE("Deck layer management", "[composition]")
{
    Deck deck;
    deck.initDefault();

    SECTION("Add layer")
    {
        int initialCount = deck.getNumLayers();
        deck.addLayer(Layer::Type::Transparent);
        REQUIRE(deck.getNumLayers() == initialCount + 1);
        REQUIRE(deck.getLayer(initialCount)->type == Layer::Type::Transparent);
    }

    SECTION("Remove layer")
    {
        int initialCount = deck.getNumLayers();
        REQUIRE(deck.removeLayer(1));
        REQUIRE(deck.getNumLayers() == initialCount - 1);
    }

    SECTION("Cannot remove last layer")
    {
        while (deck.getNumLayers() > 1)
            deck.removeLayer(0);
        REQUIRE_FALSE(deck.removeLayer(0));
        REQUIRE(deck.getNumLayers() == 1);
    }
}

TEST_CASE("Clip placement and triggering", "[composition]")
{
    Deck deck;
    deck.initDefault();

    Clip clip;
    clip.name = "test_image";
    clip.mediaType = Clip::MediaType::Image;
    clip.mediaFile = juce::File("/path/to/test.png");

    deck.setClip(0, 0, clip);

    SECTION("Clip is accessible")
    {
        auto* retrieved = deck.getClip(0, 0);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->name == "test_image");
    }

    SECTION("Trigger clip activates it")
    {
        auto* layer = deck.getLayer(0);
        REQUIRE(layer != nullptr);
        layer->triggerClip(0);
        REQUIRE(layer->activeClipColumn == 0);
        REQUIRE(layer->getActiveClip() != nullptr);
        // Note: playing state is managed by MainComponent::handleClipTrigger,
        // not by triggerClipImmediate (which preserves existing playing state)
    }

    SECTION("Clear layer deactivates clip")
    {
        auto* layer = deck.getLayer(0);
        layer->triggerClip(0);
        REQUIRE(layer->activeClipColumn == 0);
        layer->clearActiveClip();
        REQUIRE(layer->activeClipColumn == -1);
    }

    SECTION("clearCell vacates a cell to genuinely empty (not a blank Clip{})")
    {
        // A2 fix (2026-07-30): setClip(..., Clip{}) leaves the cell
        // has_value()==true (a "blank" clip), which autopilot/getClipAt
        // consumers wrongly treat as occupied. clearCell() must report
        // unoccupied.
        REQUIRE(deck.getClip(0, 0) != nullptr);   // occupied by the setup clip
        deck.clearCell(0, 0);
        REQUIRE(deck.getClip(0, 0) == nullptr);
        REQUIRE(deck.getLayer(0)->getClipAt(0) == nullptr);

        // Out-of-range / already-empty clears are safe no-ops.
        deck.clearCell(0, 999);
        deck.clearCell(99, 0);
        deck.clearCell(0, 0);
    }

    SECTION("Retrigger resets playhead")
    {
        auto* layer = deck.getLayer(0);
        layer->triggerClip(0);
        auto* active = layer->getActiveClip();
        active->playheadPosition = 0.5;
        layer->triggerClip(0); // retrigger
        REQUIRE(active->playheadPosition == 0.0);
    }

    SECTION("Column trigger activates clips across layers")
    {
        Clip clip2;
        clip2.name = "layer2_clip";
        clip2.mediaType = Clip::MediaType::Image;
        deck.setClip(1, 0, clip2);

        deck.triggerColumn(0);
        REQUIRE(deck.getLayer(0)->activeClipColumn == 0);
        REQUIRE(deck.getLayer(1)->activeClipColumn == 0);
    }
}

TEST_CASE("Composition JSON roundtrip", "[composition][serialization]")
{
    Composition comp;
    comp.initDefault();
    comp.name = "Test Composition";
    comp.masterOpacity = 0.85f;
    comp.bpmMultiplier = 2;
    comp.quantizeMode = Composition::QuantizeMode::NextBeat;

    // Add some clips
    Clip clip;
    clip.name = "my_clip";
    clip.mediaType = Clip::MediaType::Image;
    clip.speed = 1.5f;
    clip.beatSnap = true;

    Clip::EffectSlot fx;
    fx.effectName = "ripple";
    fx.paramValues = {0.5f, 0.7f, 0.3f};
    fx.enabled = true;
    clip.effects.push_back(fx);

    comp.decks[0].setClip(0, 0, clip);

    // Set layer properties
    auto* layer = comp.decks[0].getLayer(1);
    layer->type = Layer::Type::Transparent;
    layer->blendMode = Layer::MixMode::Screen;
    layer->opacity = 0.75f;

    // Serialize
    juce::var serialized = comp.toVar();
    juce::String json = juce::JSON::toString(serialized);

    // Deserialize
    Composition loaded;
    auto parsed = juce::JSON::parse(json);
    REQUIRE_FALSE(parsed.isVoid());
    loaded.fromVar(parsed);

    // Verify
    REQUIRE(loaded.name == "Test Composition");
    REQUIRE_THAT(loaded.masterOpacity, WithinAbs(0.85f, 0.001f));
    REQUIRE(loaded.bpmMultiplier == 2);
    REQUIRE(loaded.quantizeMode == Composition::QuantizeMode::NextBeat);
    REQUIRE(loaded.decks.size() == 1);
    REQUIRE(loaded.decks[0].getNumLayers() == 3);

    auto* loadedClip = loaded.decks[0].getClip(0, 0);
    REQUIRE(loadedClip != nullptr);
    REQUIRE(loadedClip->name == "my_clip");
    REQUIRE(loadedClip->mediaType == Clip::MediaType::Image);
    REQUIRE_THAT(loadedClip->speed, WithinAbs(1.5f, 0.001f));
    REQUIRE(loadedClip->beatSnap == true);
    REQUIRE(loadedClip->effects.size() == 1);
    REQUIRE(loadedClip->effects[0].effectName == "ripple");
    REQUIRE(loadedClip->effects[0].paramValues.size() == 3);

    auto* loadedLayer = loaded.decks[0].getLayer(1);
    REQUIRE(loadedLayer->type == Layer::Type::Transparent);
    REQUIRE(loadedLayer->blendMode == Layer::MixMode::Screen);
    REQUIRE_THAT(loadedLayer->opacity, WithinAbs(0.75f, 0.001f));
}

TEST_CASE("Clip full field serialization roundtrip", "[composition][serialization]")
{
    // Construct with NON-DEFAULT values in every newly-serialized field (Wave 1-C).
    Clip clip;
    clip.name = "full_clip";
    clip.mediaType = Clip::MediaType::Video;

    // BPM-sync timing (now always serialized, even without a sequence)
    clip.beatDivision = 8.0f;
    clip.videoBeats = 16.0f;

    // Video properties
    clip.clipOpacity = 0.42f;
    clip.clipWidth = 1280;
    clip.clipHeight = 720;
    clip.blendOverride = Clip::BlendOverride::Override;
    clip.alphaType = Clip::AlphaType::Straight;
    clip.channelR = false;
    clip.channelG = true;
    clip.channelB = false;
    clip.channelA = false;

    // Transform
    clip.positionX = 12.5f;
    clip.positionY = -8.0f;
    clip.scale = 2.5f;
    clip.rotation = 45.0f;
    clip.anchorX = 3.0f;
    clip.anchorY = -4.0f;

    // Effect with non-default dryWet
    Clip::EffectSlot fx;
    fx.effectName = "ripple";
    fx.dryWet = 0.33f;
    fx.paramValues = {0.1f, 0.2f};
    fx.enabled = true;
    fx.bypassed = false;
    clip.effects.push_back(fx);

    // MilkDrop preset playlist
    Clip::PresetEntry pe0; pe0.presetPath = "/a.milk"; pe0.presetName = "A"; pe0.mood = "dark"; pe0.energy = 0.2f;
    Clip::PresetEntry pe1; pe1.presetPath = "/b.milk"; pe1.presetName = "B"; pe1.mood = "bright"; pe1.energy = 0.9f;
    clip.presetPlaylist = {pe0, pe1};
    clip.playlistCycleMode = Clip::PlaylistCycleMode::PingPong;
    clip.playlistTrigger = Clip::PlaylistTrigger::OnDrop;
    clip.playlistTriggerBeats = 32;
    clip.playlistBlendSeconds = 3.25f;
    clip.playlistEnabled = true;

    // Roundtrip through JSON string
    juce::String json = juce::JSON::toString(clip.toVar());
    auto parsed = juce::JSON::parse(json);
    REQUIRE_FALSE(parsed.isVoid());
    Clip loaded;
    loaded.fromVar(parsed);

    // Field-by-field equality (explicit per field, so a future dropped field names itself)
    REQUIRE_THAT(loaded.beatDivision, WithinAbs(8.0f, 0.001f));
    REQUIRE_THAT(loaded.videoBeats, WithinAbs(16.0f, 0.001f));
    REQUIRE_THAT(loaded.clipOpacity, WithinAbs(0.42f, 0.001f));
    REQUIRE(loaded.clipWidth == 1280);
    REQUIRE(loaded.clipHeight == 720);
    REQUIRE(loaded.blendOverride == Clip::BlendOverride::Override);
    REQUIRE(loaded.alphaType == Clip::AlphaType::Straight);
    REQUIRE(loaded.channelR == false);
    REQUIRE(loaded.channelG == true);
    REQUIRE(loaded.channelB == false);
    REQUIRE(loaded.channelA == false);
    REQUIRE_THAT(loaded.positionX, WithinAbs(12.5f, 0.001f));
    REQUIRE_THAT(loaded.positionY, WithinAbs(-8.0f, 0.001f));
    REQUIRE_THAT(loaded.scale, WithinAbs(2.5f, 0.001f));
    REQUIRE_THAT(loaded.rotation, WithinAbs(45.0f, 0.001f));
    REQUIRE_THAT(loaded.anchorX, WithinAbs(3.0f, 0.001f));
    REQUIRE_THAT(loaded.anchorY, WithinAbs(-4.0f, 0.001f));
    REQUIRE(loaded.effects.size() == 1);
    REQUIRE_THAT(loaded.effects[0].dryWet, WithinAbs(0.33f, 0.001f));
    REQUIRE(loaded.presetPlaylist.size() == 2);
    REQUIRE(loaded.presetPlaylist[0].presetPath == "/a.milk");
    REQUIRE(loaded.presetPlaylist[0].presetName == "A");
    REQUIRE(loaded.presetPlaylist[0].mood == "dark");
    REQUIRE_THAT(loaded.presetPlaylist[0].energy, WithinAbs(0.2f, 0.001f));
    REQUIRE(loaded.presetPlaylist[1].presetPath == "/b.milk");
    REQUIRE_THAT(loaded.presetPlaylist[1].energy, WithinAbs(0.9f, 0.001f));
    REQUIRE(loaded.playlistCycleMode == Clip::PlaylistCycleMode::PingPong);
    REQUIRE(loaded.playlistTrigger == Clip::PlaylistTrigger::OnDrop);
    REQUIRE(loaded.playlistTriggerBeats == 32);
    REQUIRE_THAT(loaded.playlistBlendSeconds, WithinAbs(3.25f, 0.001f));
    REQUIRE(loaded.playlistEnabled == true);
}

TEST_CASE("Layer full field serialization roundtrip", "[composition][serialization]")
{
    Layer layer;
    layer.name = "full_layer";
    layer.type = Layer::Type::Transparent;

    // Video properties
    layer.layerWidth = 1280;
    layer.layerHeight = 720;
    layer.autoSize = Layer::AutoSizeMode::Fit;

    // Transition
    layer.transitionMode = Layer::MixMode::Cube;
    layer.transitionBlendMode = Layer::MixMode::Screen;

    // Transform
    layer.positionX = 15.0f;
    layer.positionY = -20.0f;
    layer.layerScale = 1.75f;
    layer.layerRotation = 90.0f;
    layer.layerAnchorX = 5.0f;
    layer.layerAnchorY = -6.0f;

    // Feedback (all fields)
    layer.feedback.enabled = true;
    layer.feedback.amount = 0.8f;
    layer.feedback.scaleX = 1.02f;
    layer.feedback.scaleY = 0.95f;
    layer.feedback.rotation = 3.5f;
    layer.feedback.offsetX = 0.1f;
    layer.feedback.offsetY = -0.2f;
    layer.feedback.lumaKey = 0.3f;
    layer.feedback.presetName = "Spiral";

    // Autopilot
    layer.autopilotLoops = 4;
    layer.autopilotEndOfVideo = true;

    // Layer effect with non-default dryWet
    Clip::EffectSlot fx;
    fx.effectName = "blur";
    fx.dryWet = 0.6f;
    fx.paramValues = {0.5f};
    layer.layerEffects.push_back(fx);

    // Roundtrip
    juce::String json = juce::JSON::toString(layer.toVar());
    auto parsed = juce::JSON::parse(json);
    REQUIRE_FALSE(parsed.isVoid());
    Layer loaded;
    loaded.fromVar(parsed);

    REQUIRE(loaded.layerWidth == 1280);
    REQUIRE(loaded.layerHeight == 720);
    REQUIRE(loaded.autoSize == Layer::AutoSizeMode::Fit);
    REQUIRE(loaded.transitionMode == Layer::MixMode::Cube);
    REQUIRE(loaded.transitionBlendMode == Layer::MixMode::Screen);
    REQUIRE_THAT(loaded.positionX, WithinAbs(15.0f, 0.001f));
    REQUIRE_THAT(loaded.positionY, WithinAbs(-20.0f, 0.001f));
    REQUIRE_THAT(loaded.layerScale, WithinAbs(1.75f, 0.001f));
    REQUIRE_THAT(loaded.layerRotation, WithinAbs(90.0f, 0.001f));
    REQUIRE_THAT(loaded.layerAnchorX, WithinAbs(5.0f, 0.001f));
    REQUIRE_THAT(loaded.layerAnchorY, WithinAbs(-6.0f, 0.001f));
    REQUIRE(loaded.feedback.enabled == true);
    REQUIRE_THAT(loaded.feedback.amount, WithinAbs(0.8f, 0.001f));
    REQUIRE_THAT(loaded.feedback.scaleX, WithinAbs(1.02f, 0.001f));
    REQUIRE_THAT(loaded.feedback.scaleY, WithinAbs(0.95f, 0.001f));
    REQUIRE_THAT(loaded.feedback.rotation, WithinAbs(3.5f, 0.001f));
    REQUIRE_THAT(loaded.feedback.offsetX, WithinAbs(0.1f, 0.001f));
    REQUIRE_THAT(loaded.feedback.offsetY, WithinAbs(-0.2f, 0.001f));
    REQUIRE_THAT(loaded.feedback.lumaKey, WithinAbs(0.3f, 0.001f));
    REQUIRE(loaded.feedback.presetName == "Spiral");
    REQUIRE(loaded.autopilotLoops == 4);
    REQUIRE(loaded.autopilotEndOfVideo == true);
    REQUIRE(loaded.layerEffects.size() == 1);
    REQUIRE_THAT(loaded.layerEffects[0].dryWet, WithinAbs(0.6f, 0.001f));
}

TEST_CASE("Composition full field serialization roundtrip", "[composition][serialization]")
{
    Composition comp;
    comp.initDefault();

    // Master + video
    comp.masterSpeed = 1.5f;
    comp.compOpacity = 0.7f;

    // Crossfader
    comp.crossfaderPhase = 0.25f;
    comp.crossfaderBlendMode = Composition::CrossfaderBlendMode::Multiply;
    comp.crossfaderBehaviour = Composition::CrossfaderBehaviour::Smooth;
    comp.crossfaderCurve = Composition::CrossfaderCurve::SCurve;

    // Transform
    comp.compPositionX = 30.0f;
    comp.compPositionY = -40.0f;
    comp.compScale = 0.85f;
    comp.compRotation = 180.0f;
    comp.compAnchorX = 7.0f;
    comp.compAnchorY = -9.0f;

    // Autopilot (composition-level)
    comp.autopilotDirection = Composition::AutopilotDirection::Forward;
    comp.autopilotDurationMode = Composition::AutopilotDurationMode::Custom;
    comp.autopilotClipLoops = 3;
    comp.autopilotLoop = true;
    comp.autopilotMasterLayer = 2;

    // Per-Type Autopilot (all fields non-default)
    comp.perTypeAutopilot.opaqueCycleBeats = 24;
    comp.perTypeAutopilot.opaquePlayUntilEnd = true;
    comp.perTypeAutopilot.transparentCycleBeats = 12;
    comp.perTypeAutopilot.transparentMaxLayers = 3;
    comp.perTypeAutopilot.transparentRandomize = false;
    comp.perTypeAutopilot.effectCycleBeats = 6;
    comp.perTypeAutopilot.effectMaxLayers = 4;
    comp.perTypeAutopilot.effectRandomize = false;
    comp.perTypeAutopilot.perTypeEnabled = true;
    comp.perTypeAutopilot.globalRandomize = true;
    comp.perTypeAutopilot.loopAutopilot = false;

    // Genre-aware automation
    comp.autoPresetOnGenre = true;
    comp.smartAutopilotEnabled = true;
    comp.structuralSceneEnabled = true;
    for (int i = 0; i < 8; ++i)
    {
        comp.genreDeckAssignment[i] = i;
        comp.genrePresetNames[i] = "genre_preset_" + std::to_string(i);
    }

    // Global effect with non-default dryWet
    Clip::EffectSlot fx;
    fx.effectName = "grade";
    fx.dryWet = 0.45f;
    fx.paramValues = {0.3f};
    comp.globalEffects.push_back(fx);

    // Roundtrip
    juce::String json = juce::JSON::toString(comp.toVar());
    auto parsed = juce::JSON::parse(json);
    REQUIRE_FALSE(parsed.isVoid());
    Composition loaded;
    loaded.fromVar(parsed);

    REQUIRE_THAT(loaded.masterSpeed, WithinAbs(1.5f, 0.001f));
    REQUIRE_THAT(loaded.compOpacity, WithinAbs(0.7f, 0.001f));
    REQUIRE_THAT(loaded.crossfaderPhase, WithinAbs(0.25f, 0.001f));
    REQUIRE(loaded.crossfaderBlendMode == Composition::CrossfaderBlendMode::Multiply);
    REQUIRE(loaded.crossfaderBehaviour == Composition::CrossfaderBehaviour::Smooth);
    REQUIRE(loaded.crossfaderCurve == Composition::CrossfaderCurve::SCurve);
    REQUIRE_THAT(loaded.compPositionX, WithinAbs(30.0f, 0.001f));
    REQUIRE_THAT(loaded.compPositionY, WithinAbs(-40.0f, 0.001f));
    REQUIRE_THAT(loaded.compScale, WithinAbs(0.85f, 0.001f));
    REQUIRE_THAT(loaded.compRotation, WithinAbs(180.0f, 0.001f));
    REQUIRE_THAT(loaded.compAnchorX, WithinAbs(7.0f, 0.001f));
    REQUIRE_THAT(loaded.compAnchorY, WithinAbs(-9.0f, 0.001f));
    REQUIRE(loaded.autopilotDirection == Composition::AutopilotDirection::Forward);
    REQUIRE(loaded.autopilotDurationMode == Composition::AutopilotDurationMode::Custom);
    REQUIRE(loaded.autopilotClipLoops == 3);
    REQUIRE(loaded.autopilotLoop == true);
    REQUIRE(loaded.autopilotMasterLayer == 2);
    REQUIRE(loaded.perTypeAutopilot.opaqueCycleBeats == 24);
    REQUIRE(loaded.perTypeAutopilot.opaquePlayUntilEnd == true);
    REQUIRE(loaded.perTypeAutopilot.transparentCycleBeats == 12);
    REQUIRE(loaded.perTypeAutopilot.transparentMaxLayers == 3);
    REQUIRE(loaded.perTypeAutopilot.transparentRandomize == false);
    REQUIRE(loaded.perTypeAutopilot.effectCycleBeats == 6);
    REQUIRE(loaded.perTypeAutopilot.effectMaxLayers == 4);
    REQUIRE(loaded.perTypeAutopilot.effectRandomize == false);
    REQUIRE(loaded.perTypeAutopilot.perTypeEnabled == true);
    REQUIRE(loaded.perTypeAutopilot.globalRandomize == true);
    REQUIRE(loaded.perTypeAutopilot.loopAutopilot == false);
    REQUIRE(loaded.autoPresetOnGenre == true);
    REQUIRE(loaded.smartAutopilotEnabled == true);
    REQUIRE(loaded.structuralSceneEnabled == true);
    for (int i = 0; i < 8; ++i)
    {
        REQUIRE(loaded.genreDeckAssignment[i] == i);
        REQUIRE(loaded.genrePresetNames[i] == "genre_preset_" + std::to_string(i));
    }
    REQUIRE(loaded.globalEffects.size() == 1);
    REQUIRE_THAT(loaded.globalEffects[0].dryWet, WithinAbs(0.45f, 0.001f));
}

TEST_CASE("Backward compatibility: old-format presets load with struct defaults", "[composition][serialization][backcompat]")
{
    // Old presets predate Wave 1-C and lack the new keys. fromVar must fall back to
    // struct defaults for every missing key while still loading the old keys, no crash.

    SECTION("Clip old-format")
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", "old_clip");
        obj->setProperty("mediaType", static_cast<int>(Clip::MediaType::Image));
        juce::var clipVar(obj);

        Clip clip;
        clip.fromVar(clipVar);

        REQUIRE(clip.name == "old_clip");
        REQUIRE(clip.mediaType == Clip::MediaType::Image);
        REQUIRE_THAT(clip.clipOpacity, WithinAbs(1.0f, 0.001f));
        REQUIRE(clip.clipWidth == 1920);
        REQUIRE(clip.clipHeight == 1080);
        REQUIRE(clip.blendOverride == Clip::BlendOverride::LayerDetermined);
        REQUIRE(clip.alphaType == Clip::AlphaType::Premultiplied);
        REQUIRE(clip.channelR);
        REQUIRE(clip.channelG);
        REQUIRE(clip.channelB);
        REQUIRE(clip.channelA);
        REQUIRE_THAT(clip.scale, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(clip.rotation, WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(clip.beatDivision, WithinAbs(4.0f, 0.001f));
        REQUIRE_THAT(clip.videoBeats, WithinAbs(4.0f, 0.001f));
        REQUIRE(clip.presetPlaylist.empty());
        REQUIRE(clip.playlistEnabled == false);
        REQUIRE(clip.playlistCycleMode == Clip::PlaylistCycleMode::RandomBag);
        REQUIRE(clip.playlistTriggerBeats == 8);
    }

    SECTION("Layer old-format")
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", "old_layer");
        obj->setProperty("type", static_cast<int>(Layer::Type::Opaque));
        juce::var layerVar(obj);

        Layer layer;
        layer.fromVar(layerVar);

        REQUIRE(layer.name == "old_layer");
        REQUIRE(layer.type == Layer::Type::Opaque);
        REQUIRE(layer.feedback.enabled == false);
        REQUIRE_THAT(layer.feedback.amount, WithinAbs(0.5f, 0.001f));
        REQUIRE_THAT(layer.feedback.scaleX, WithinAbs(0.98f, 0.001f));
        REQUIRE(layer.feedback.presetName.empty());
        REQUIRE(layer.autopilotLoops == 1);
        REQUIRE(layer.autopilotEndOfVideo == false);
        REQUIRE(layer.layerWidth == 1920);
        REQUIRE(layer.layerHeight == 1080);
        REQUIRE(layer.autoSize == Layer::AutoSizeMode::Off);
        REQUIRE(layer.transitionMode == Layer::MixMode::Dissolve);
        REQUIRE(layer.transitionBlendMode == Layer::MixMode::Normal);
        REQUIRE_THAT(layer.layerScale, WithinAbs(1.0f, 0.001f));
    }

    SECTION("Composition old-format")
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", "old_comp");
        obj->setProperty("masterOpacity", 0.9);   // an old field — must still load
        juce::var compVar(obj);

        Composition comp;
        comp.fromVar(compVar);

        REQUIRE(comp.name == "old_comp");
        REQUIRE_THAT(comp.masterOpacity, WithinAbs(0.9f, 0.001f));
        REQUIRE_THAT(comp.masterSpeed, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(comp.compOpacity, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(comp.crossfaderPhase, WithinAbs(0.5f, 0.001f));
        REQUIRE(comp.crossfaderBlendMode == Composition::CrossfaderBlendMode::Alpha);
        REQUIRE(comp.crossfaderBehaviour == Composition::CrossfaderBehaviour::Cut);
        REQUIRE(comp.crossfaderCurve == Composition::CrossfaderCurve::Linear);
        REQUIRE(comp.autopilotDirection == Composition::AutopilotDirection::Off);
        REQUIRE(comp.autopilotMasterLayer == -1);
        REQUIRE(comp.perTypeAutopilot.opaqueCycleBeats == 16);
        REQUIRE(comp.perTypeAutopilot.perTypeEnabled == false);
        REQUIRE(comp.perTypeAutopilot.loopAutopilot == true);
        REQUIRE(comp.autoPresetOnGenre == false);
        REQUIRE(comp.genreDeckAssignment[0] == -1);
        REQUIRE(comp.genrePresetNames[0].empty());
    }
}

// ============================================================
// L3 — Composition Persistence, Step 1: id-mint bumps, appendDeck,
// compload:: validate/remint/idsRetired helpers.
// ============================================================

TEST_CASE("Deck::fromVar bumps the layer-id mint past every loaded id", "[composition][serialization]")
{
    // A file whose layers already hold ids at/past the private nextLayerId_
    // default (100) — without the bump, a post-load addLayer() re-mints an id
    // a loaded layer already holds, aliasing two layers onto one GL resource.
    auto* deckObj = new juce::DynamicObject();
    deckObj->setProperty("name", "Loaded Deck");
    deckObj->setProperty("id", 0);
    deckObj->setProperty("numColumns", 12);

    juce::Array<juce::var> layerArray;
    for (uint32_t id : { 0u, 1u, 2u, 100u, 101u })
    {
        Layer layer;
        layer.id = id;
        layerArray.add(layer.toVar());
    }
    deckObj->setProperty("layers", layerArray);

    Deck deck;
    deck.fromVar(juce::var(deckObj));
    REQUIRE(deck.layers.size() == 5);

    deck.addLayer(Layer::Type::Transparent);
    REQUIRE(deck.layers.back().id == 102);

    // No behavior change for a deck that never loaded ids past the default mint.
    Deck fresh;
    fresh.initDefault();
    fresh.addLayer(Layer::Type::Transparent);
    REQUIRE(fresh.layers.back().id == 100);
}

TEST_CASE("Composition::fromVar bumps the deck-id mint past every loaded id", "[composition][serialization]")
{
    auto* compObj = new juce::DynamicObject();
    compObj->setProperty("name", "Loaded Comp");

    juce::Array<juce::var> deckArray;
    for (uint32_t id : { 0u, 100u, 250u })
    {
        Deck deck;
        deck.id = id;
        deck.initDefault();
        deckArray.add(deck.toVar());
    }
    compObj->setProperty("decks", deckArray);

    Composition comp;
    comp.fromVar(juce::var(compObj));
    REQUIRE(comp.decks.size() == 3);

    comp.addDeck();
    REQUIRE(comp.decks.back().id == 251);
}

TEST_CASE("Composition::appendDeck assigns a fresh id, keeps contents, returns the index", "[composition]")
{
    Composition comp;
    comp.initDefault(); // one deck, default id 0, mint still at its default (100)

    Deck incoming;
    incoming.name = "Appended Deck";
    incoming.id = 999; // stale id from a loaded file — must be overwritten, not kept
    incoming.initDefault();

    int index = comp.appendDeck(incoming);
    REQUIRE(index == 1);
    REQUIRE(comp.decks.size() == 2);
    REQUIRE(comp.decks[1].name == "Appended Deck");
    REQUIRE(comp.decks[1].id != 999);
    REQUIRE(comp.decks[1].id == 100); // mint's default — comp was never loaded via fromVar

    // A second append keeps minting distinct, incrementing ids.
    Deck another;
    another.name = "Second Appended";
    another.initDefault();
    int index2 = comp.appendDeck(another);
    REQUIRE(index2 == 2);
    REQUIRE(comp.decks[2].id == 101);
    REQUIRE(comp.decks[2].id != comp.decks[1].id);
}

TEST_CASE("compload::validateComposition refuses structurally-empty files and repairs indices/columns", "[composition][compload]")
{
    SECTION("No decks refused")
    {
        Composition comp;
        comp.decks.clear();
        auto reason = compload::validateComposition(comp);
        REQUIRE_FALSE(reason.empty());
    }

    SECTION("A deck with zero layers is refused")
    {
        Composition comp;
        Deck deck;
        deck.name = "Empty Deck";
        deck.layers.clear();
        comp.decks = { deck };
        auto reason = compload::validateComposition(comp);
        REQUIRE_FALSE(reason.empty());
    }

    SECTION("Out-of-range activeDeckIndex is repaired to 0")
    {
        Composition comp;
        comp.initDefault(); // one deck
        comp.activeDeckIndex = 7;
        auto reason = compload::validateComposition(comp);
        REQUIRE(reason.empty());
        REQUIRE(comp.activeDeckIndex == 0);
    }

    SECTION("numColumns is repaired to fit the widest layer")
    {
        // A single layer with 5 clips and no others — the widest layer sets
        // numColumns, and every layer (there's only the one) gets padded to it.
        Deck deck;
        deck.name = "Deck";
        deck.numColumns = 0;
        Layer layer;
        layer.clips.resize(5);
        deck.layers = { layer };

        Composition comp;
        comp.decks = { deck };
        comp.activeDeckIndex = 0;

        auto reason = compload::validateComposition(comp);
        REQUIRE(reason.empty());
        REQUIRE(comp.decks[0].numColumns == 5);
        for (const auto& l : comp.decks[0].layers)
            REQUIRE(l.clips.size() == 5);
    }
}

TEST_CASE("compload::remintClipIds gives every clip a unique monotonic id and advances the mint", "[composition][compload]")
{
    Composition comp;
    comp.initDefault();
    comp.addDeck("Deck 2");

    // All-zero / duplicate ids across both decks, as a hand-edited or
    // older-build file (or a deck appended into a live composition) could carry.
    Clip a; a.id = 0; a.mediaType = Clip::MediaType::Image;
    Clip b; b.id = 0; b.mediaType = Clip::MediaType::Image;
    Clip c; c.id = 5; c.mediaType = Clip::MediaType::Image;
    Clip d; d.id = 5; d.mediaType = Clip::MediaType::Image;

    comp.decks[0].setClip(0, 0, a);
    comp.decks[0].setClip(0, 1, b);
    comp.decks[1].setClip(0, 0, c);
    comp.decks[1].setClip(0, 1, d);

    uint32_t nextId = 1000;
    int n = compload::remintClipIds(comp, nextId);
    REQUIRE(n == 4);
    REQUIRE(nextId == 1000u + 4u);

    std::vector<uint32_t> ids;
    for (auto& deck : comp.decks)
        for (auto& layer : deck.layers)
            for (auto& cell : layer.clips)
                if (cell.has_value()) ids.push_back(cell->id);

    REQUIRE(ids.size() == 4);
    for (auto id : ids)
        REQUIRE(id >= 1000u);
    std::sort(ids.begin(), ids.end());
    REQUIRE(std::adjacent_find(ids.begin(), ids.end()) == ids.end()); // all unique
}

TEST_CASE("compload::idsRetired is before minus after", "[composition][compload]")
{
    using Ids = std::vector<uint32_t>;

    REQUIRE(compload::idsRetired(Ids{ 1, 2, 3 }, Ids{ 2, 3, 4 }) == Ids{ 1 });
    REQUIRE(compload::idsRetired(Ids{ 1, 2, 3 }, Ids{ 1, 2, 3, 4 }) == Ids{}); // superset: nothing retired
    REQUIRE(compload::idsRetired(Ids{ 1, 2, 3 }, Ids{ 4, 5, 6 }) == Ids{ 1, 2, 3 }); // disjoint: all retired
    REQUIRE(compload::idsRetired(Ids{}, Ids{}) == Ids{});
}

TEST_CASE("Composition saveToFile/loadFromFile round-trips through a real file and sets filePath", "[composition][serialization]")
{
    auto file = juce::File::createTempFile(".json");

    Composition comp;
    comp.initDefault();
    comp.name = "File RoundTrip";
    comp.decks[0].name = "Deck A";
    comp.addDeck("Deck B");

    Clip clip;
    clip.name = "file_clip";
    clip.mediaType = Clip::MediaType::Image;
    comp.decks[0].setClip(0, 0, clip);

    REQUIRE(comp.saveToFile(file));

    Composition loaded;
    REQUIRE(loaded.loadFromFile(file));

    REQUIRE(loaded.decks.size() == 2);
    REQUIRE(loaded.decks[0].name == "Deck A");
    REQUIRE(loaded.decks[1].name == "Deck B");
    auto* loadedClip = loaded.decks[0].getClip(0, 0);
    REQUIRE(loadedClip != nullptr);
    REQUIRE(loadedClip->name == "file_clip");
    REQUIRE(loaded.filePath == file);

    file.deleteFile();
}

TEST_CASE("Composition::loadFromFile fails closed on a missing or non-JSON file", "[composition][serialization]")
{
    SECTION("Nonexistent file")
    {
        auto missing = juce::File::getSpecialLocation(juce::File::tempDirectory)
                           .getChildFile("l3-gate-does-not-exist.json");
        missing.deleteFile(); // ensure absence, no leftover from a prior run

        Composition comp;
        comp.initDefault();
        auto decksBefore = comp.decks.size();

        REQUIRE_FALSE(comp.loadFromFile(missing));
        REQUIRE(comp.decks.size() == decksBefore);
    }

    SECTION("Non-JSON content")
    {
        auto file = juce::File::createTempFile(".json");
        file.replaceWithText("not json");

        Composition comp;
        comp.initDefault();
        auto decksBefore = comp.decks.size();

        REQUIRE_FALSE(comp.loadFromFile(file));
        REQUIRE(comp.decks.size() == decksBefore);

        file.deleteFile();
    }
}

TEST_CASE("UndoManager basic operations", "[undo]")
{
    UndoManager mgr;
    int value = 0;

    // Simple command that sets a value
    struct SetValueCmd : Command
    {
        int& ref;
        int newVal, oldVal;
        SetValueCmd(int& r, int nv) : ref(r), newVal(nv), oldVal(r) {}
        void execute() override { ref = newVal; }
        void undo() override { ref = oldVal; }
        std::string description() const override { return "Set value"; }
    };

    SECTION("Perform and undo")
    {
        mgr.perform(std::make_unique<SetValueCmd>(value, 42));
        REQUIRE(value == 42);
        REQUIRE(mgr.canUndo());

        mgr.undo();
        REQUIRE(value == 0);
        REQUIRE_FALSE(mgr.canUndo());
    }

    SECTION("Redo after undo")
    {
        mgr.perform(std::make_unique<SetValueCmd>(value, 42));
        mgr.undo();
        REQUIRE(mgr.canRedo());

        mgr.redo();
        REQUIRE(value == 42);
        REQUIRE_FALSE(mgr.canRedo());
    }

    SECTION("New command clears redo history")
    {
        mgr.perform(std::make_unique<SetValueCmd>(value, 10));
        mgr.perform(std::make_unique<SetValueCmd>(value, 20));
        mgr.undo(); // value = 10
        REQUIRE(mgr.canRedo());

        mgr.perform(std::make_unique<SetValueCmd>(value, 30));
        REQUIRE_FALSE(mgr.canRedo());
        REQUIRE(value == 30);
    }

    SECTION("Multiple undo/redo")
    {
        mgr.perform(std::make_unique<SetValueCmd>(value, 1));
        mgr.perform(std::make_unique<SetValueCmd>(value, 2));
        mgr.perform(std::make_unique<SetValueCmd>(value, 3));
        REQUIRE(value == 3);

        mgr.undo();
        REQUIRE(value == 2);
        mgr.undo();
        REQUIRE(value == 1);
        mgr.undo();
        REQUIRE(value == 0);
        REQUIRE_FALSE(mgr.canUndo());

        mgr.redo();
        REQUIRE(value == 1);
        mgr.redo();
        REQUIRE(value == 2);
    }
}

namespace
{
    // Records execution/undo order into a shared log (positive id on execute,
    // negative on undo) so ordering can be asserted.
    struct LogCmd : Command
    {
        std::vector<int>& log;
        int id;
        LogCmd(std::vector<int>& l, int i) : log(l), id(i) {}
        void execute() override { log.push_back(id); }
        void undo() override { log.push_back(-id); }
        std::string description() const override { return "log"; }
    };

    // Plain value-setter, no merging.
    struct SetCmd : Command
    {
        int& ref;
        int newVal, oldVal;
        SetCmd(int& r, int v) : ref(r), newVal(v), oldVal(r) {}
        void execute() override { ref = newVal; }
        void undo() override { ref = oldVal; }
        std::string description() const override { return "set"; }
    };

    // Mergeable setter: keeps its original before-state, adopts the latest
    // after-state on merge (models consecutive triggers on one layer).
    struct MergeCmd : Command
    {
        int& ref;
        int before, after;
        MergeCmd(int& r, int v) : ref(r), before(r), after(v) {}
        void execute() override { ref = after; }
        void undo() override { ref = before; }
        std::string description() const override { return "merge"; }
        bool canMergeWith(const Command& /*other*/) const override { return true; }
        void mergeWith(const Command& other) override
        {
            after = static_cast<const MergeCmd&>(other).after;
        }
    };
}

TEST_CASE("CompositeCommand executes in order, undoes in reverse", "[undo][composite]")
{
    std::vector<int> log;

    CompositeCommand comp("Group");
    comp.add(std::make_unique<LogCmd>(log, 1));
    comp.add(std::make_unique<LogCmd>(log, 2));
    comp.add(std::make_unique<LogCmd>(log, 3));

    REQUIRE(comp.size() == 3);
    REQUIRE_FALSE(comp.isEmpty());
    REQUIRE(comp.description() == "Group");

    comp.execute();
    REQUIRE(log == std::vector<int>{ 1, 2, 3 });

    log.clear();
    comp.undo();
    REQUIRE(log == std::vector<int>{ -3, -2, -1 });

    log.clear();
    comp.execute(); // redo path
    REQUIRE(log == std::vector<int>{ 1, 2, 3 });
}

TEST_CASE("CompositeCommand is one undo unit through UndoManager", "[undo][composite]")
{
    UndoManager mgr;
    int a = 0, b = 0;

    auto comp = std::make_unique<CompositeCommand>("Set A and B");
    comp->add(std::make_unique<SetCmd>(a, 5));
    comp->add(std::make_unique<SetCmd>(b, 7));
    mgr.perform(std::move(comp));

    REQUIRE(a == 5);
    REQUIRE(b == 7);
    REQUIRE(mgr.historySize() == 1);            // one slot for the whole group
    REQUIRE(mgr.undoDescription() == "Set A and B");

    mgr.undo();
    REQUIRE(a == 0);
    REQUIRE(b == 0);

    mgr.redo();
    REQUIRE(a == 5);
    REQUIRE(b == 7);
}

TEST_CASE("UndoManager merges consecutive mergeable commands", "[undo][merge]")
{
    UndoManager mgr;
    int value = 0;

    mgr.perform(std::make_unique<MergeCmd>(value, 10));
    REQUIRE(value == 10);
    REQUIRE(mgr.historySize() == 1);

    mgr.perform(std::make_unique<MergeCmd>(value, 20));
    REQUIRE(value == 20);
    REQUIRE(mgr.historySize() == 1);            // merged, not a new slot

    mgr.perform(std::make_unique<MergeCmd>(value, 30));
    REQUIRE(value == 30);
    REQUIRE(mgr.historySize() == 1);

    // One undo reverts to the ORIGINAL before-state, proving the merge kept the
    // first command's before while adopting the last after.
    mgr.undo();
    REQUIRE(value == 0);
    REQUIRE_FALSE(mgr.canUndo());
}

TEST_CASE("UndoManager caps history at kMaxHistory (100)", "[undo][cap]")
{
    UndoManager mgr;
    int value = 0;

    // 150 distinct, non-merging commands.
    for (int i = 1; i <= 150; ++i)
        mgr.perform(std::make_unique<SetCmd>(value, i));

    REQUIRE(value == 150);
    REQUIRE(mgr.historySize() == 100);          // oldest 50 evicted

    int undos = 0;
    while (mgr.canUndo())
    {
        mgr.undo();
        ++undos;
    }
    REQUIRE(undos == 100);                       // exactly cap-many undos remain
    // Undoing #51 last restores value to its before-state (50).
    REQUIRE(value == 50);
}
