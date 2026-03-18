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
        REQUIRE(layer->getActiveClip()->playing == true);
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
