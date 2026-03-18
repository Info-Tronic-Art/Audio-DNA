#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/Deck.h"
#include "model/Autopilot.h"
#include "analysis/FeatureSnapshot.h"

using Catch::Matchers::WithinAbs;

// Note: Full OpenGL compositing tests require a GL context which is not
// available in unit tests. These tests verify the data model logic that
// drives compositing, not the GL rendering itself.

TEST_CASE("Deck layer compositing data model", "[compositor]")
{
    Deck deck;
    deck.initDefault();

    // Set up clips
    Clip clip1;
    clip1.name = "background";
    clip1.mediaType = Clip::MediaType::Image;
    deck.setClip(0, 0, clip1);

    Clip clip2;
    clip2.name = "overlay";
    clip2.mediaType = Clip::MediaType::Image;
    deck.setClip(1, 0, clip2);

    SECTION("Layer types are correct")
    {
        REQUIRE(deck.getLayer(0)->type == Layer::Type::Opaque);
        REQUIRE(deck.getLayer(1)->type == Layer::Type::Transparent);
    }

    SECTION("Active clips tracked correctly")
    {
        deck.getLayer(0)->triggerClip(0);
        deck.getLayer(1)->triggerClip(0);

        REQUIRE(deck.getLayer(0)->getActiveClip()->name == "background");
        REQUIRE(deck.getLayer(1)->getActiveClip()->name == "overlay");
    }

    SECTION("Layer visibility affects compositing")
    {
        deck.getLayer(1)->visible = false;
        deck.getLayer(0)->triggerClip(0);
        deck.getLayer(1)->triggerClip(0);

        // Layer 1 clip is active but not visible
        REQUIRE(deck.getLayer(1)->getActiveClip() != nullptr);
        REQUIRE_FALSE(deck.getLayer(1)->visible);
    }

    SECTION("Layer bypass")
    {
        deck.getLayer(1)->bypassed = true;
        REQUIRE(deck.getLayer(1)->bypassed);
    }
}

TEST_CASE("Autopilot clip advancement", "[autopilot]")
{
    Deck deck;
    deck.initDefault();

    // Put clips in columns 0, 1, 2 on layer 0
    for (int c = 0; c < 3; ++c)
    {
        Clip clip;
        clip.name = "clip_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Image;
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        clip.autopilotDuration = Clip::AutopilotDuration::Beat4;
        deck.setClip(0, c, clip);
    }

    auto* layer = deck.getLayer(0);
    layer->autopilotEnabled = true;
    layer->triggerClip(0);

    Autopilot autopilot;

    SECTION("No advance before duration reached")
    {
        // Simulate 3 beats (need 4 to advance)
        FeatureSnapshot snap;
        for (int beat = 0; beat < 3; ++beat)
        {
            // Simulate beat crossing: phase wraps from ~1.0 to ~0.0
            snap.beatPhase = 0.99f;
            autopilot.processFrame(deck, snap);
            snap.beatPhase = 0.01f;
            autopilot.processFrame(deck, snap);
        }
        REQUIRE(layer->activeClipColumn == 0); // Still on first clip
    }

    SECTION("Advance after duration reached")
    {
        FeatureSnapshot snap;
        for (int beat = 0; beat < 4; ++beat)
        {
            snap.beatPhase = 0.99f;
            autopilot.processFrame(deck, snap);
            snap.beatPhase = 0.01f;
            autopilot.processFrame(deck, snap);
        }
        // Should have advanced to column 1
        REQUIRE(layer->activeClipColumn == 1);
    }
}

TEST_CASE("Column management", "[deck]")
{
    Deck deck;
    deck.initDefault();

    SECTION("Add column")
    {
        int initial = deck.numColumns;
        deck.addColumn();
        REQUIRE(deck.numColumns == initial + 1);
    }

    SECTION("Remove column")
    {
        Clip clip;
        clip.name = "test";
        deck.setClip(0, 5, clip);

        REQUIRE(deck.removeColumn(5));
        REQUIRE(deck.numColumns == 11);
        // The clip at column 5 should be gone
    }

    SECTION("Cannot remove last column")
    {
        while (deck.numColumns > 1)
            deck.removeColumn(0);
        REQUIRE_FALSE(deck.removeColumn(0));
    }
}

TEST_CASE("Layer keying and blend properties", "[layer]")
{
    Layer layer;
    layer.type = Layer::Type::Transparent;

    SECTION("Default blend mode is Additive")
    {
        REQUIRE(layer.blendMode == Layer::MixMode::Additive);
    }

    SECTION("Keying properties serialize correctly")
    {
        layer.keyingMode = Layer::KeyingMode::ChromaKey;
        layer.chromaKeyR = 0.0f;
        layer.chromaKeyG = 1.0f;
        layer.chromaKeyB = 0.0f;
        layer.chromaKeyTolerance = 0.3f;

        auto var = layer.toVar();
        Layer loaded;
        loaded.fromVar(var);

        REQUIRE(loaded.keyingMode == Layer::KeyingMode::ChromaKey);
        REQUIRE_THAT(loaded.chromaKeyG, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(loaded.chromaKeyTolerance, WithinAbs(0.3f, 0.001f));
    }
}
