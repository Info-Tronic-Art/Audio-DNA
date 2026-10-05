#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/Composition.h"
#include "core/CompositionLoad.h"
#include "core/UndoManager.h"
#include "core/Command.h"
#include "core/CompositeCommand.h"
#include "ShowFixture.h"
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
    REQUIRE(comp.masterSignal == 1.0f);

    auto* deck = comp.getActiveDeck();
    REQUIRE(deck != nullptr);
    REQUIRE(comp.getNumLayers() == 3);   // lane bf9b: 3 shared layers; the deck holds a row of clips for each
    REQUIRE(deck->getNumRows() == 3);
    REQUIRE(deck->numColumns == 12);
}

TEST_CASE("Composition::initDefault resets masterSignal to 1.0 (s-rta-0925 mastersignal Step 1)",
         "[composition]")
{
    Composition comp;
    comp.initDefault();
    comp.masterSignal = 0.2f;

    comp.initDefault();
    REQUIRE(comp.masterSignal == 1.0f);
}

// Lane bf9b: layers are the show's (Composition::layers); a deck only holds a row of clips per layer.
TEST_CASE("Layer management on the shared stack", "[composition]")
{
    Composition show;
    show.initDefault();

    SECTION("Add layer")
    {
        int initialCount = show.getNumLayers();
        show.insertLayer(show.getNumLayers(), show.makeLayer(Layer::Type::Transparent));
        REQUIRE(show.getNumLayers() == initialCount + 1);
        REQUIRE(show.getLayer(initialCount)->type == Layer::Type::Transparent);
    }

    SECTION("Remove layer")
    {
        int initialCount = show.getNumLayers();
        REQUIRE(show.eraseLayer(1));
        REQUIRE(show.getNumLayers() == initialCount - 1);
    }

    SECTION("Cannot remove last layer")
    {
        while (show.getNumLayers() > 1)
            show.eraseLayer(0);
        REQUIRE_FALSE(show.eraseLayer(0));
        REQUIRE(show.getNumLayers() == 1);
    }
}

TEST_CASE("Clip placement and triggering", "[composition]")
{
    Composition show = ShowFixture::makeShow(1, 3, 12, false);   // lane bf9b: one deck box under the shared layers
    Deck& deck = show.decks[0];

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
        auto* layer = show.getLayer(0);
        REQUIRE(layer != nullptr);
        show.fire(0, 0, 0);
        REQUIRE(layer->runtime().activeClipColumn == 0);
        REQUIRE(show.playingClip(0) != nullptr);
        // Note: playing state is managed by MainComponent::handleClipTrigger,
        // not by triggerClipImmediate (which preserves existing playing state)
    }

    SECTION("Clear layer deactivates clip")
    {
        auto* layer = show.getLayer(0);
        show.fire(0, 0, 0);
        REQUIRE(layer->runtime().activeClipColumn == 0);
        layer->clearActiveClip(show.rowClips(0));
        REQUIRE(layer->runtime().activeClipColumn == -1);
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
        REQUIRE(deck.getRow(0)->getClipAt(0) == nullptr);

        // Out-of-range / already-empty clears are safe no-ops.
        deck.clearCell(0, 999);
        deck.clearCell(99, 0);
        deck.clearCell(0, 0);
    }

    SECTION("Retrigger resets playhead")
    {
        show.fire(0, 0, 0);
        auto* active = show.playingClip(0);
        active->playheadPosition = 0.5;
        show.fire(0, 0, 0); // retrigger
        REQUIRE(active->playheadPosition == 0.0);
    }

    SECTION("Column trigger activates clips across layers")
    {
        Clip clip2;
        clip2.name = "layer2_clip";
        clip2.mediaType = Clip::MediaType::Image;
        deck.setClip(1, 0, clip2);

        show.triggerColumn(0, 0);
        REQUIRE(show.getLayer(0)->runtime().activeClipColumn == 0);
        REQUIRE(show.getLayer(1)->runtime().activeClipColumn == 0);
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
    auto* layer = comp.getLayer(1);   // lane bf9b: the shared layer
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
    REQUIRE(loaded.getNumLayers() == 3);
    REQUIRE(loaded.decks[0].getNumRows() == 3);

    auto* loadedClip = loaded.decks[0].getClip(0, 0);
    REQUIRE(loadedClip != nullptr);
    REQUIRE(loadedClip->name == "my_clip");
    REQUIRE(loadedClip->mediaType == Clip::MediaType::Image);
    REQUIRE_THAT(loadedClip->speed, WithinAbs(1.5f, 0.001f));
    REQUIRE(loadedClip->beatSnap == true);
    REQUIRE(loadedClip->effects.size() == 1);
    REQUIRE(loadedClip->effects[0].effectName == "ripple");
    REQUIRE(loadedClip->effects[0].paramValues.size() == 3);

    auto* loadedLayer = loaded.getLayer(1);
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
    clip.fitMode = Clip::FitMode::Crop;   // s-rta-0926b plan-fitmode

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
    REQUIRE(loaded.fitMode == Clip::FitMode::Crop);
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
    comp.masterSignal = 0.35f;   // s-rta-0925 mastersignal Step 1

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
    REQUIRE_THAT(loaded.masterSignal, WithinAbs(0.35f, 0.001f));
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

TEST_CASE("Clip fitMode: out-of-range loads Stretch; replaceContent keeps it; clear() resets it", "[composition][serialization]")
{
    // s-rta-0926b plan-fitmode section 2.2.
    auto* obj = new juce::DynamicObject();
    obj->setProperty("name", "fit_clip");
    obj->setProperty("mediaType", static_cast<int>(Clip::MediaType::Image));
    obj->setProperty("fitMode", 7);
    Clip bad;
    bad.fitMode = Clip::FitMode::Bars;
    bad.fromVar(juce::var(obj));
    REQUIRE(bad.fitMode == Clip::FitMode::Stretch);

    Clip clip;
    clip.mediaType = Clip::MediaType::Image;
    clip.fitMode = Clip::FitMode::Bars;
    Clip incoming;
    incoming.mediaType = Clip::MediaType::Video;
    incoming.fitMode = Clip::FitMode::Crop;
    REQUIRE(clip.replaceContent(incoming));
    REQUIRE(clip.mediaType == Clip::MediaType::Video);
    REQUIRE(clip.fitMode == Clip::FitMode::Bars);   // "how this clip is shown" travels with the transform

    clip.clear();
    REQUIRE(clip.fitMode == Clip::FitMode::Stretch);
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
        REQUIRE(clip.fitMode == Clip::FitMode::Stretch);   // plan-fitmode: an old file shows as today
        REQUIRE_THAT(clip.scale, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(clip.rotation, WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(clip.beatDivision, WithinAbs(4.0f, 0.001f));
        REQUIRE_THAT(clip.videoBeats, WithinAbs(4.0f, 0.001f));
        REQUIRE(clip.presetPlaylist.empty());
        REQUIRE(clip.playlistEnabled == false);
        REQUIRE(clip.playlistCycleMode == Clip::PlaylistCycleMode::RandomBag);
        REQUIRE(clip.playlistTriggerBeats == 8);
        // s-rta-0928b mediaopen: a file without "speed" plays at the struct default, not frozen at 0.
        REQUIRE_THAT(clip.speed, WithinAbs(1.0f, 0.001f));
    }

    SECTION("Clip explicit speed")
    {
        // s-rta-0928b mediaopen: an explicit "speed" is kept -- 0.0 (a deliberately frozen clip) included.
        for (const double v : { 0.0, 2.0 })
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("name", "speed_clip");
            obj->setProperty("mediaType", static_cast<int>(Clip::MediaType::Video));
            obj->setProperty("speed", v);
            Clip clip;
            clip.fromVar(juce::var(obj));
            REQUIRE_THAT(clip.speed, WithinAbs(static_cast<float>(v), 0.001f));
        }
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
        // masterSignal has no key in this old-format object -- must load
        // 1.0 (Boris Q3: absent -> full signal reach), not 0.
        REQUIRE_THAT(comp.masterSignal, WithinAbs(1.0f, 0.001f));
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

// bf9 Stage P (s-rta-1002b; ruling-bf9 amendment 5): the Persistent layer feature is removed. A show saved with
// "persistent": true loads cleanly with the key ignored, saving never writes it back, and every other Layer field
// (Ignore Column Trigger, the one "keep this layer" control left, among them) round-trips unchanged.
TEST_CASE("Layer: a file's persistent key is ignored and never written back; every other field round-trips "
          "(bf9 Stage P)", "[composition][serialization][backcompat]")
{
    Layer layer;
    layer.ignoreColumnTrigger = true;
    layer.opacity = 0.4f;
    layer.blendMode = Layer::MixMode::Screen;
    REQUIRE(static_cast<int>(layer.blendMode) != 0);
    layer.transitionSpeed = 2.5f;

    juce::var v0 = layer.toVar();
    v0.getDynamicObject()->removeProperty("persistent");   // a no-op once the field is gone
    juce::var v1 = juce::JSON::parse(juce::JSON::toString(v0));
    REQUIRE(v1.getDynamicObject() != nullptr);
    v1.getDynamicObject()->setProperty("persistent", true);   // an older file's flag

    Layer fresh;
    fresh.fromVar(v1);
    const juce::var out = fresh.toVar();

    CHECK(fresh.ignoreColumnTrigger);
    CHECK_FALSE(out.getDynamicObject()->hasProperty("persistent"));
    CHECK(juce::JSON::toString(out) == juce::JSON::toString(v0));
}

// ============================================================
// L3 — Composition Persistence, Step 1: id-mint bumps, appendDeck,
// compload:: validate/remint/idsRetired helpers.
// ============================================================

// Lane bf9b: the layer-id mint is the show's (Composition::makeLayer); a bf9b file carries the layers top-level.
TEST_CASE("Composition::fromVar bumps the layer-id mint past every loaded id", "[composition][serialization]")
{
    // A file whose layers already hold ids at/past the private nextLayerId_
    // default (100) — without the bump, a post-load Add Layer re-mints an id
    // a loaded layer already holds, aliasing two layers onto one GL resource.
    Composition source;
    source.initDefault();
    juce::var v = source.toVar();

    juce::Array<juce::var> layerArray;
    for (uint32_t id : { 0u, 1u, 2u, 100u, 101u })
    {
        Layer layer;
        layer.id = id;
        layerArray.add(layer.toVar());
    }
    v.getDynamicObject()->setProperty("layers", layerArray);

    Composition comp;
    comp.fromVar(v);
    REQUIRE(comp.layers.size() == 5);

    comp.insertLayer(comp.getNumLayers(), comp.makeLayer(Layer::Type::Transparent));
    REQUIRE(comp.layers.back().id == 102);

    // No behavior change for a show that never loaded ids past the default mint.
    Composition fresh;
    fresh.initDefault();
    fresh.insertLayer(fresh.getNumLayers(), fresh.makeLayer(Layer::Type::Transparent));
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
        deck.rows.clear();   // lane bf9b: a deck's "layers" are its rows of clips
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
        ClipRow row;   // lane bf9b: one row of clips (the show gets its one shared layer from normalizeRows)
        row.clips.resize(5);
        deck.rows = { row };

        Composition comp;
        comp.decks = { deck };
        comp.activeDeckIndex = 0;

        auto reason = compload::validateComposition(comp);
        REQUIRE(reason.empty());
        REQUIRE(comp.decks[0].numColumns == 5);
        for (const auto& r : comp.decks[0].rows)
            REQUIRE(r.clips.size() == 5);
    }

    SECTION("An implausible numColumns is refused, not resized toward (crash-on-open guard)")
    {
        // Unlike the widest-layer case above, numColumns here is a single int
        // field with no clip data behind it — exactly what a hand-edited or
        // corrupted file could contain. Pre-fix, validateDeck had no upper
        // bound: it would set numColumns to this value and call
        // Layer::ensureColumns(50000) -> clips.resize(50000) on every layer,
        // an allocation wildly disproportionate to the 3 actual clips in the
        // file. Post-fix, kMaxNumColumns refuses before any resize is
        // attempted, so this SECTION's clips.size() check would FAIL against
        // the code as committed in b5a181c (validateDeck would resize to
        // 50000 and return "", so both REQUIRE_FALSE(reason.empty()) and the
        // clips.size() == 3 check below would fail on that commit).
        Deck deck;
        deck.name = "Huge Deck";
        deck.numColumns = 50000;
        ClipRow row;
        row.clips.resize(3);
        deck.rows = { row };

        Composition comp;
        comp.decks = { deck };
        comp.activeDeckIndex = 0;

        auto reason = compload::validateComposition(comp);
        REQUIRE_FALSE(reason.empty());
        // Refused before any resize attempt — the layer's clips vector must
        // be untouched, not grown toward the implausible count.
        REQUIRE(comp.decks[0].rows[0].clips.size() == 3);
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
        for (auto& row : deck.rows)
            for (auto& cell : row.clips)
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

    REQUIRE(comp.saveToFile(file, {}));

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

// plan6 §3 E2 (F1): a file saved by a build whose New Deck left every deck at id 0 carries
// DUPLICATE deck ids; LayerStateKey keys per-layer GL history by (deckId, layerId), so two
// decks sharing an id alias each other's temporal buffers. fromVar re-mints every repeat
// (the first holder keeps its id) and leaves distinct ids untouched.
TEST_CASE("Composition::fromVar re-mints duplicate deck ids", "[composition][serialization][ids]")
{
    Composition source;
    source.initDefault();
    juce::var v = source.toVar();

    juce::Array<juce::var> deckArray;
    const char* names[] = { "A", "B", "C" };
    const uint32_t ids[] = { 0u, 0u, 7u };
    for (int i = 0; i < 3; ++i)
    {
        Deck deck;
        deck.name = names[i];
        deck.id = ids[i];
        deck.initDefault();
        deckArray.add(deck.toVar());
    }
    v.getDynamicObject()->setProperty("decks", deckArray);

    Composition comp;
    comp.fromVar(v);
    REQUIRE(comp.decks.size() == 3);
    REQUIRE(comp.decks[0].id != comp.decks[1].id);
    REQUIRE(comp.decks[0].id != comp.decks[2].id);
    REQUIRE(comp.decks[1].id != comp.decks[2].id);
    REQUIRE(comp.decks[0].id == 0u);        // the first holder keeps its id
    REQUIRE(comp.decks[2].id == 7u);        // a distinct id is untouched
    REQUIRE(comp.decks[1].id >= 100u);      // the repeat is re-minted from the mint

    comp.addDeck();
    REQUIRE(comp.decks.size() == 4);
    REQUIRE(comp.decks[3].id != comp.decks[0].id);
    REQUIRE(comp.decks[3].id != comp.decks[1].id);
    REQUIRE(comp.decks[3].id != comp.decks[2].id);
}

// plan6 §5 A1-e: Duplicate Deck = a value copy under "<name> copy" with the library link dropped and
// every clip re-minted (media is closed BY CLIP ID, so a copy must never share one with its source).
// Row count, columns and clip content are kept. id 0 = re-minted by Composition::appendDeck.
// Lane bf9b (plan-bf9b S2.3): a deck is a box of clip rows -- it holds no layer ids and no trigger tuple, so there
// is no queued trigger to clear any more (the layer-id and pending-trigger checks went with them; "no ref names the
// copy" is T8 in test_show_model.cpp).
TEST_CASE("compload::duplicateDeck copies under \"<name> copy\" with every clip re-minted (a deck holds no tuple)", "[composition][compload]")
{
    Deck src;
    src.name = "A";
    src.id = 42;
    src.numColumns = 6;
    src.sourceFile = juce::File("/tmp/plan6-A.json");
    src.initDefault(2);   // two rows of 6 empty cells

    Clip video;  video.id = 11; video.name = "vid"; video.mediaType = Clip::MediaType::Video;
    video.mediaFile = juce::File("/tmp/plan6-vid.mp4");
    Clip image;  image.id = 12; image.name = "img"; image.mediaType = Clip::MediaType::Image;
    Clip source; source.id = 13; source.name = "src"; source.mediaType = Clip::MediaType::Source;
    source.sourceType = "plasma";
    src.rows[0].clips[0] = video;
    src.rows[0].clips[2] = image;
    src.rows[1].clips[1] = source;

    uint32_t nextClipId = 500;
    const Deck copy = compload::duplicateDeck(src, nextClipId);

    REQUIRE(copy.name == "A copy");
    REQUIRE(copy.id == 0u);
    REQUIRE(copy.sourceFile == juce::File());
    REQUIRE(copy.numColumns == src.numColumns);
    REQUIRE(copy.rows.size() == 2);
    REQUIRE(copy.rows[0].clips[0].has_value());
    REQUIRE(copy.rows[0].clips[0]->name == "vid");
    REQUIRE(copy.rows[0].clips[0]->mediaFile == video.mediaFile);
    REQUIRE(copy.rows[0].clips[2]->name == "img");
    REQUIRE(copy.rows[1].clips[1]->name == "src");

    std::vector<uint32_t> copyIds;
    for (const auto& row : copy.rows)
        for (const auto& cell : row.clips)
            if (cell.has_value()) copyIds.push_back(cell->id);
    REQUIRE(copyIds.size() == 3);
    for (auto id : copyIds)
        REQUIRE((id != 11u && id != 12u && id != 13u));   // disjoint from the source's clip ids
    REQUIRE(nextClipId == 503u);

    // The source is untouched.
    REQUIRE(src.name == "A");
    REQUIRE(src.rows[0].clips[0]->id == 11u);
}

// s-rta-0927 source-defects (plan-source-defects.md A2): a composition saved by an older build carries source params
// the registry no longer has (dead torus controls) and old defaults (julia_set's black-interior default). It must
// still LOAD, and after the load-time reconcile the clip's params are the registry's current list: dead ones gone,
// kept values + connections intact, defaults refreshed (the right-click reset target, Pitfall 8), new ones added,
// registry order; an unknown source type untouched; a second pass changes nothing.
TEST_CASE("compload::reconcileSourceParams brings an old file's source clips to the registry's current params", "[composition][compload][source-params]")
{
    auto sp = [](const char* name, const char* uni, float v, float d) {
        Clip::SourceParam p; p.name = name; p.uniformName = uni; p.value = v; p.defaultValue = d; return p;
    };
    Composition old;
    old.initDefault();
    Deck& deck = *old.getActiveDeck();
    Clip torus; torus.id = 1; torus.name = "tt"; torus.mediaType = Clip::MediaType::Source; torus.sourceType = "twisted_torus";
    torus.sourceParams = { sp("Twist", "u_src_twist", 0.3f, 0.3f), sp("Stripe Count", "u_src_stripe_count", 0.9f, 0.3f),
                           sp("Orbit", "u_src_orbit", 0.7f, 0.0f), sp("Tilt", "u_src_tilt", 0.49f, 0.49f),
                           sp("Speed", "u_src_speed", 0.3f, 0.3f), sp("Zoom", "u_src_zoom", 0.4f, 0.4f),
                           sp("Tube Radius", "u_src_tube_radius", 0.32f, 0.32f), sp("Pinch", "u_src_pinch", 0.5f, 0.5f),
                           sp("Color Shift", "u_src_color_shift", 0.25f, 0.0f) };
    torus.sourceParams.back().conn.source.kind = ConnSource::Kind::Macro;   // a connection on a KEPT param
    torus.sourceParams.back().conn.source.macroIndex = 2;
    Clip julia; julia.id = 2; julia.name = "js"; julia.mediaType = Clip::MediaType::Source; julia.sourceType = "julia_set";
    julia.sourceParams = { sp("C Real", "u_src_cx", 0.35f, 0.35f), sp("Zoom", "u_src_zoom", 0.25f, 0.25f) };
    Clip mystery; mystery.id = 3; mystery.name = "mx"; mystery.mediaType = Clip::MediaType::Source; mystery.sourceType = "gone_source";
    mystery.sourceParams = { sp("A", "u_src_a", 0.1f, 0.2f) };
    Clip image; image.id = 4; image.name = "img"; image.mediaType = Clip::MediaType::Image;
    deck.setClip(0, 0, torus);
    deck.setClip(0, 1, julia);
    deck.setClip(1, 0, mystery);
    deck.setClip(1, 1, image);

    // Round-trip through a real file: the old file must still load and validate.
    const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory)
                          .getChildFile("reconcile-old-" + juce::String(juce::Time::getMillisecondCounter()) + ".json");
    REQUIRE(old.saveToFile(file, {}));
    Composition incoming;
    REQUIRE(incoming.loadFromFile(file));
    file.deleteFile();
    REQUIRE(compload::validateComposition(incoming).empty());
    REQUIRE(incoming.getActiveDeck()->getClip(0, 0)->sourceParams.size() == 9);

    using RP = compload::RegisteredSourceParam;
    const std::map<std::string, std::vector<RP>> registry = {
        { "twisted_torus", { { "Twist", "u_src_twist", 0.3f }, { "Stripe Count", "u_src_stripe_count", 0.3f },
                             { "Speed", "u_src_speed", 0.3f }, { "Tube Radius", "u_src_tube_radius", 0.32f },
                             { "Color Shift", "u_src_color_shift", 0.0f } } },
        { "julia_set", { { "Dive Speed", "u_src_dive_speed", 0.0f }, { "C Real", "u_src_cx", 0.2f },
                         { "Zoom", "u_src_zoom", 0.1f } } },
    };
    int lookups = 0;
    const compload::SourceParamLookup lookup = [&](const std::string& type) -> std::optional<std::vector<RP>> {
        ++lookups;
        auto it = registry.find(type);
        if (it == registry.end()) return std::nullopt;
        return it->second;
    };

    REQUIRE(compload::reconcileSourceParams(incoming, lookup) == 2);
    Deck& d = *incoming.getActiveDeck();

    const auto& tp = d.getClip(0, 0)->sourceParams;
    REQUIRE(tp.size() == 5);
    const char* want[] = { "u_src_twist", "u_src_stripe_count", "u_src_speed", "u_src_tube_radius", "u_src_color_shift" };
    for (size_t i = 0; i < 5; ++i) REQUIRE(tp[i].uniformName == want[i]);
    REQUIRE_THAT(tp[1].value, WithinAbs(0.9, 1e-6));          // kept value
    REQUIRE_THAT(tp[4].value, WithinAbs(0.25, 1e-6));
    REQUIRE(tp[4].conn.source.kind == ConnSource::Kind::Macro);  // kept connection
    REQUIRE(tp[4].conn.source.macroIndex == 2);

    const auto& jp = d.getClip(0, 1)->sourceParams;
    REQUIRE(jp.size() == 3);
    REQUIRE(jp[0].uniformName == "u_src_dive_speed");          // added at its default, in registry order
    REQUIRE_THAT(jp[0].value, WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(jp[1].value, WithinAbs(0.35, 1e-6));          // the user's value survives ...
    REQUIRE_THAT(jp[1].defaultValue, WithinAbs(0.2, 1e-6));    // ... but right-click now resets to the NEW default
    REQUIRE_THAT(jp[2].defaultValue, WithinAbs(0.1, 1e-6));

    const auto& mp = d.getClip(1, 0)->sourceParams;            // unknown type: untouched
    REQUIRE(mp.size() == 1);
    REQUIRE(mp[0].uniformName == "u_src_a");
    REQUIRE_THAT(mp[0].defaultValue, WithinAbs(0.2, 1e-6));
    REQUIRE(d.getClip(1, 1)->sourceParams.empty());            // non-source clip untouched

    REQUIRE(lookups == 3);                                     // once per source TYPE, not per clip
    REQUIRE(compload::reconcileSourceParams(incoming, lookup) == 0);   // idempotent
}

// Lane bf9b (plan-bf9b 4.B, test_composition.cpp:1322): the clips the shared layers PLAY come first, from any deck,
// then the shown deck's other cells, then the other decks.
TEST_CASE("compload::imagePaths: the playing clips first (any deck), then the shown deck, then the other decks; "
          "deduplicated", "[composition][compload][s-rta-0928]")
{
    auto img = [](const char* path) {
        Clip c;
        c.mediaType = Clip::MediaType::Image;
        c.mediaFile = juce::File(path);
        return c;
    };
    Composition comp;
    comp.initDefault();
    while (comp.getNumLayers() > 2)
        comp.eraseLayer(comp.getNumLayers() - 1);
    Deck d0, d1;
    d0.name = "D0"; d1.name = "D1";
    ClipRow a, b, c;
    a.clips = { img("/i/a0.png"), img("/i/a1.png"), img("/i/a2.png") };
    b.clips = { std::nullopt, img("/i/b1.png"), img("/i/a0.png") };    // a duplicate of a0
    Clip src; src.mediaType = Clip::MediaType::Source; src.sourceType = "plasma";
    Clip none; none.mediaType = Clip::MediaType::Image;                 // no file: skipped
    c.clips = { img("/i/c0.png"), src, none };
    d0.rows = { c, ClipRow{} };
    d1.rows = { a, b };
    d0.numColumns = d1.numColumns = 3;
    comp.decks.clear();
    REQUIRE(comp.appendDeck(d0) == 0);
    REQUIRE(comp.appendDeck(d1) == 1);
    comp.activeDeckIndex = 1;

    SECTION("the shown deck's playing clips")
    {
        comp.fire(0, 1, 2);   // layer 0 plays D1 a2
        comp.fire(1, 1, 1);   // layer 1 plays D1 b1
        const auto p = compload::imagePaths(comp);
        const std::vector<std::string> expect = { "/i/a2.png", "/i/b1.png", "/i/a0.png", "/i/a1.png", "/i/c0.png" };
        CHECK(p == expect);
    }

    SECTION("a clip playing from a deck that is not shown still comes first")
    {
        comp.fire(0, 0, 0);   // layer 0 plays D0 c0 while D1 is shown
        const auto p = compload::imagePaths(comp);
        const std::vector<std::string> expect = { "/i/c0.png", "/i/a0.png", "/i/a1.png", "/i/a2.png", "/i/b1.png" };
        CHECK(p == expect);
    }
}
