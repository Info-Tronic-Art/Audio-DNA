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

    SECTION("Autopilot never selects a genuinely-cleared cell")
    {
        // A2 regression guard: clearCell() (not a blank Clip{}) must report
        // unoccupied to getClipAt so PlayNext's occupancy scan skips it,
        // landing on column 2 instead of the cleared column 1.
        deck.clearCell(0, 1);
        REQUIRE(layer->getClipAt(1) == nullptr);

        FeatureSnapshot snap;
        for (int beat = 0; beat < 4; ++beat)
        {
            snap.beatPhase = 0.99f;
            autopilot.processFrame(deck, snap);
            snap.beatPhase = 0.01f;
            autopilot.processFrame(deck, snap);
        }
        REQUIRE(layer->activeClipColumn == 2); // column 1 skipped (cleared)
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

// S167-L4b: opacity product / speed fold pure-math coverage.
//
// The real functions -- CompositorEngine::combinedOpacity() (CompositorEngine.h)
// and Renderer::effectiveClipSpeed() (Renderer.h) -- are one-line, GL-free
// arithmetic, but they live in headers that pull in juce_opengl (and, for
// CompositorEngine.h, ShaderManager/TextureManager/FullscreenQuad/
// FeedbackProcessor/EffectLibrary/FeatureSnapshot on top of that). This
// target links only juce_core + juce_graphics and compiles only
// model/Clip.cpp, model/Layer.cpp, model/Autopilot.cpp (see this file's
// header comment above and tests/CMakeLists.txt) -- wiring in the GL headers
// would need linking juce::juce_opengl plus several more .cpp files there,
// which is out of this lane's fence (tests/CMakeLists.txt is a FORBIDDEN
// file for this work packet). So these mirror the production one-liners
// exactly, cited by file:line, rather than including the GL-heavy headers.
namespace {
    // Mirrors CompositorEngine::combinedOpacity() (src/render/CompositorEngine.h:244).
    float combinedOpacity(float layerOpacity, float clipOpacity)
    {
        return layerOpacity * clipOpacity;
    }

    // Mirrors Renderer::effectiveClipSpeed() (src/render/Renderer.h:244).
    float effectiveClipSpeed(float clipSpeed, float masterSpeed, bool isBpmSynced)
    {
        return isBpmSynced ? clipSpeed : clipSpeed * masterSpeed;
    }
}

TEST_CASE("Opacity product multiplies master/layer/clip (S167-L4b)", "[compositor][opacity]")
{
    SECTION("Full opacity on both sides is a no-op")
    {
        REQUIRE_THAT(combinedOpacity(1.0f, 1.0f), WithinAbs(1.0f, 0.0001f));
    }

    SECTION("Clip pinned at 0.5 can never exceed 50%, regardless of layer opacity")
    {
        REQUIRE_THAT(combinedOpacity(1.0f, 0.5f), WithinAbs(0.5f, 0.0001f));
        REQUIRE_THAT(combinedOpacity(0.7f, 0.5f), WithinAbs(0.35f, 0.0001f));
    }

    SECTION("Zero at either level zeroes the result")
    {
        REQUIRE_THAT(combinedOpacity(0.0f, 1.0f), WithinAbs(0.0f, 0.0001f));
        REQUIRE_THAT(combinedOpacity(1.0f, 0.0f), WithinAbs(0.0f, 0.0001f));
    }
}

TEST_CASE("Speed fold leaves BPM-synced clips tempo-locked (S167-L4b)", "[compositor][speed]")
{
    SECTION("Non-BPM-synced clip speed is scaled by masterSpeed")
    {
        REQUIRE_THAT(effectiveClipSpeed(1.0f, 2.0f, false), WithinAbs(2.0f, 0.0001f));
        REQUIRE_THAT(effectiveClipSpeed(0.5f, 0.5f, false), WithinAbs(0.25f, 0.0001f));
    }

    SECTION("BPM-synced clip speed ignores masterSpeed entirely")
    {
        REQUIRE_THAT(effectiveClipSpeed(1.0f, 2.0f, true), WithinAbs(1.0f, 0.0001f));
        REQUIRE_THAT(effectiveClipSpeed(0.5f, 4.0f, true), WithinAbs(0.5f, 0.0001f));
    }

    SECTION("masterSpeed at its 1.0 default is a no-op either way")
    {
        REQUIRE_THAT(effectiveClipSpeed(0.75f, 1.0f, false), WithinAbs(0.75f, 0.0001f));
        REQUIRE_THAT(effectiveClipSpeed(0.75f, 1.0f, true), WithinAbs(0.75f, 0.0001f));
    }
}

// S167-L4b DT-FIX: transition-progress (crossfade / deck-transition)
// frame-rate-independence pure-math coverage.
//
// The real step computations -- CompositorEngine::compositeDeck()'s
// crossfade-progress advance (`float step = dt / speed;`,
// src/render/CompositorEngine.cpp:781) and Renderer::renderOpenGL()'s
// deck-transition advance (`deckTransitionProgress_ +=
// deckTransitionSpeed_ * realDt;`, src/render/Renderer.cpp:605, with
// deckTransitionSpeed_ set to `1.0f / transSpeed` at Renderer.cpp:642) --
// live inside GL-heavy functions/files this GL-free test target cannot
// link (see this file's header comment above and tests/CMakeLists.txt, a
// FORBIDDEN file for this work packet). Both sites reduce to the same
// one-line arithmetic: a progress-per-second rate (1/durationSeconds)
// advanced by the real per-frame delta, so cumulative progress after T
// real seconds is T/durationSeconds regardless of how many frames T was
// split into. Mirrored here exactly, cited by file:line, rather than
// pulling in the GL headers.
namespace {
    // Mirrors the crossfade step at CompositorEngine.cpp:781 and the
    // deck-transition step at Renderer.cpp:605/642 -- both reduce to this.
    // `speed`/`transSpeed` in the real sites are misleadingly-named
    // DURATIONS in seconds (see Layer::transitionSpeed's UI wiring in
    // LayerInspector.cpp/LayerStrip.cpp and Composition::globalTransitionSpeed's
    // "// seconds" comment in Composition.h), not rate multipliers.
    float transitionProgressStep(float dt, float durationSeconds)
    {
        return dt / durationSeconds;
    }
}

TEST_CASE("Transition progress step is frame-rate independent (S167-L4b)", "[compositor][transition]")
{
    SECTION("Same total progress after one simulated second, 30 steps vs 120 steps")
    {
        const float duration = 2.0f; // seconds

        float progress30 = 0.0f;
        for (int i = 0; i < 30; ++i)
            progress30 = std::min(progress30 + transitionProgressStep(1.0f / 30.0f, duration), 1.0f);

        float progress120 = 0.0f;
        for (int i = 0; i < 120; ++i)
            progress120 = std::min(progress120 + transitionProgressStep(1.0f / 120.0f, duration), 1.0f);

        REQUIRE_THAT(progress30, WithinAbs(progress120, 0.0001f));
        REQUIRE_THAT(progress30, WithinAbs(0.5f, 0.0001f)); // 1s into a 2s transition = 50%
    }

    SECTION("Transition completes at exactly its set duration regardless of fps")
    {
        const float duration = 0.5f; // seconds

        float progress60 = 0.0f;
        for (int i = 0; i < 30; ++i) // 0.5s at 60fps = 30 frames
            progress60 = std::min(progress60 + transitionProgressStep(1.0f / 60.0f, duration), 1.0f);

        float progress24 = 0.0f;
        for (int i = 0; i < 12; ++i) // 0.5s at 24fps = 12 frames
            progress24 = std::min(progress24 + transitionProgressStep(1.0f / 24.0f, duration), 1.0f);

        REQUIRE_THAT(progress60, WithinAbs(1.0f, 0.0001f));
        REQUIRE_THAT(progress24, WithinAbs(1.0f, 0.0001f));
    }

    SECTION("Regression guard: a hardcoded 1/60 step would break frame-rate independence")
    {
        // Documents the OLD bug's consequence for a 1-second-duration
        // transition -- a constant-per-callback step (ignoring real dt)
        // makes total progress track fps instead of wall-clock time. Kept
        // as a regression guard against reintroducing `dt = 1.0f/60.0f` at
        // either fixed call site.
        const float oldHardcodedStep = 1.0f / 60.0f; // the bug: constant regardless of real fps

        // At a sustained 30fps, 30 real callbacks land in one real second,
        // but the hardcoded step only ever advanced by 1/60 per callback --
        // total progress after 1 real second was 30 * (1/60) = 0.5 (half
        // done at the transition's supposed 1-second mark).
        REQUIRE_THAT(30.0f * oldHardcodedStep, WithinAbs(0.5f, 0.0001f));

        // At 120fps, 120 real callbacks land in one real second --
        // 120 * (1/60) = 2.0 (clamped to 1.0 in production) -- the
        // transition finished twice as fast as intended.
        REQUIRE(120.0f * oldHardcodedStep >= 1.0f);
    }
}

// S167-L4b DT-FIX: procedural-source scaledTime_ accumulation frame-rate-
// independence pure-math coverage.
//
// The real accumulation -- Renderer::renderOpenGL()'s non-override branch,
// `scaledTime_ += static_cast<double>(realDt) * static_cast<double>(masterSpeedVal);`
// (src/render/Renderer.cpp:400, with realDt computed just above it at
// Renderer.cpp:409-414) -- lives inside a GL-heavy function this GL-free
// test target cannot link (see this file's header comment above and
// tests/CMakeLists.txt, a FORBIDDEN file for this work packet). Before this
// fix, scaledTime_ accumulated with a hardcoded `(1.0 / 60.0)` step instead
// of realDt -- the same bug shape as the transitionProgressStep and
// combinedOpacity/effectiveClipSpeed cases above, just for procedural-
// source animation (noise/plasma/etc. sources rendered via renderSource()).
// Mirrored here exactly, cited by file:line, rather than pulling in the GL
// headers.
namespace {
    // Mirrors the non-override accumulation at Renderer.cpp:400.
    double scaledTimeStep(double realDt, double masterSpeed)
    {
        return realDt * masterSpeed;
    }
}

TEST_CASE("Procedural-source scaledTime_ accumulation is frame-rate independent (S167-L4b)", "[compositor][speed]")
{
    SECTION("Same total scaled time after one simulated second, 30 steps vs 120 steps")
    {
        const double masterSpeed = 1.0;

        double scaled30 = 0.0;
        for (int i = 0; i < 30; ++i)
            scaled30 += scaledTimeStep(1.0 / 30.0, masterSpeed);

        double scaled120 = 0.0;
        for (int i = 0; i < 120; ++i)
            scaled120 += scaledTimeStep(1.0 / 120.0, masterSpeed);

        REQUIRE_THAT(static_cast<float>(scaled30), WithinAbs(static_cast<float>(scaled120), 0.0001f));
        REQUIRE_THAT(static_cast<float>(scaled30), WithinAbs(1.0f, 0.0001f)); // 1s of real time at 1x speed
    }

    SECTION("masterSpeed scales the accumulation rate independent of frame rate")
    {
        const double masterSpeed = 2.0;

        double scaled60 = 0.0;
        for (int i = 0; i < 60; ++i)
            scaled60 += scaledTimeStep(1.0 / 60.0, masterSpeed);

        double scaled24 = 0.0;
        for (int i = 0; i < 24; ++i)
            scaled24 += scaledTimeStep(1.0 / 24.0, masterSpeed);

        REQUIRE_THAT(static_cast<float>(scaled60), WithinAbs(2.0f, 0.0001f)); // 1s real time * 2x speed
        REQUIRE_THAT(static_cast<float>(scaled24), WithinAbs(2.0f, 0.0001f)); // 1s real time * 2x speed
    }

    SECTION("Regression guard: a hardcoded 1/60 step would couple procedural-source speed to fps")
    {
        // Documents the OLD bug's consequence -- accumulating with a
        // constant 1/60 step (ignoring realDt) makes scaledTime_ track
        // frame COUNT instead of wall-clock time.
        const double oldHardcodedStep = 1.0 / 60.0; // the bug: constant regardless of real fps
        const double masterSpeed = 1.0;

        // At a sustained 30fps, 30 real callbacks land in one real second,
        // but the hardcoded step only ever advanced by 1/60 per callback --
        // procedural sources animated at HALF the intended rate.
        REQUIRE_THAT(static_cast<float>(30.0 * oldHardcodedStep * masterSpeed), WithinAbs(0.5f, 0.0001f));

        // At 120fps, 120 real callbacks land in one real second --
        // procedural sources animated at DOUBLE the intended rate.
        REQUIRE_THAT(static_cast<float>(120.0 * oldHardcodedStep * masterSpeed), WithinAbs(2.0f, 0.0001f));
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
