#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/Composition.h"
#include "core/UndoManager.h"
#include "core/Command.h"

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
