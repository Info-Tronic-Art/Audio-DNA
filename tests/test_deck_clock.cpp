#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Deck.h"
#include "render/LayerClock.h"
#include "render/DeckClock.h"
#include "model/AutopilotBank.h"
#include <functional>
#include <vector>

using Catch::Approx;

// s-rta-0926b plan4 item 2: a deck that is not on screen keeps time (Boris 2026-09-26: "finish the fade. when we
// load a new deck that does not touch the clips playing in the layer"). DeckClock::tick is the inactive-deck
// clock (pure, no GL); Renderer::renderOpenGL calls it for every deck except the active one, inside the deckActive
// fence. LayerClock::advanceCrossfade is the ONE crossfade clock (CompositorEngine::advanceCrossfade forwards to
// it). These drive the real headers on real Deck / Layer / Clip objects.

namespace
{
Clip makeClip(uint32_t id, Clip::MediaType type)
{
    Clip c;
    c.id = id;
    c.name = "c" + std::to_string(id);
    c.mediaType = type;
    if (type == Clip::MediaType::Source)
        c.sourceType = "perlin_noise";
    return c;
}

// A deck whose layer `li` is mid-fade from col 0 to col 1 (progress 0, previousClipColumn 0).
void startFade(Deck& deck, int li, float duration,
               Clip::MediaType out = Clip::MediaType::Image, Clip::MediaType in = Clip::MediaType::Image)
{
    deck.setClip(li, 0, makeClip(static_cast<uint32_t>(100 + li * 10), out));
    deck.setClip(li, 1, makeClip(static_cast<uint32_t>(101 + li * 10), in));
    auto* layer = deck.getLayer(li);
    layer->transitionSpeed = duration;
    layer->triggerClip(0);          // first trigger: no fade
    layer->triggerClip(1);          // col 0 -> col 1 over `duration` s
}

struct Recorder
{
    std::vector<const Clip*> ticked;
    void operator()(const Clip* c, float) { ticked.push_back(c); }
};

auto noop = [](const Clip*, float) {};
} // namespace

TEST_CASE("(a) an inactive deck's non-persistent layer finishes its fade at the real rate", "[deck_clock]")
{
    Deck deck; deck.initDefault();
    startFade(deck, 0, 2.0f);
    auto* layer = deck.getLayer(0);
    REQUIRE(layer->runtime().crossfadeProgress == Approx(0.0f));
    REQUIRE(layer->runtime().previousClipColumn == 0);

    DeckClock::tick(deck, 0.5f, noop);
    CHECK(layer->runtime().crossfadeProgress == Approx(0.25f));
    CHECK(layer->runtime().previousClipColumn == 0);

    for (int i = 0; i < 3; ++i)
        DeckClock::tick(deck, 0.5f, noop);
    CHECK(layer->runtime().crossfadeProgress == Approx(1.0f));
    CHECK(layer->runtime().previousClipColumn == -1);       // transition complete
    CHECK(layer->runtime().activeClipColumn == 1);          // the clips are not touched
}

TEST_CASE("(b) persistent layers are owned by compositePersistentLayers: never advanced twice", "[deck_clock]")
{
    Deck deck; deck.initDefault();

    SECTION("a persistent Transparent layer's fade is untouched by the tick")
    {
        startFade(deck, 1, 2.0f, Clip::MediaType::Video, Clip::MediaType::Video);
        auto* layer = deck.getLayer(1);
        layer->type = Layer::Type::Transparent;
        layer->persistent = true;
        Recorder rec;
        DeckClock::tick(deck, 0.5f, std::ref(rec));
        CHECK(layer->runtime().crossfadeProgress == Approx(0.0f));
        CHECK(layer->runtime().previousClipColumn == 0);
        CHECK(rec.ticked.empty());               // its media is rendered (and clocked) by compositePersistentLayers
    }

    SECTION("a persistent Mask layer (loaded persistent, not persistable): fade untouched, media clocked here")
    {
        startFade(deck, 1, 2.0f, Clip::MediaType::Video, Clip::MediaType::Video);
        auto* layer = deck.getLayer(1);
        layer->type = Layer::Type::Mask;
        layer->persistent = true;
        REQUIRE_FALSE(Layer::canBePersistent(layer->type));
        Recorder rec;
        DeckClock::tick(deck, 0.5f, std::ref(rec));
        CHECK(layer->runtime().crossfadeProgress == Approx(0.0f));    // compositePersistentLayers advances it (before its type check)
        REQUIRE(rec.ticked.size() == 2);                     // ... but never renders its media
        CHECK(rec.ticked[0] == layer->getClipAt(1));
        CHECK(rec.ticked[1] == layer->getClipAt(0));
    }
}

TEST_CASE("(c) hidden, bypassed and solo-excluded layers are left alone (compositeDeck's gate)", "[deck_clock]")
{
    Deck deck; deck.initDefault();
    startFade(deck, 0, 2.0f);
    startFade(deck, 1, 2.0f);
    startFade(deck, 2, 2.0f);

    SECTION("hidden")
    {
        deck.getLayer(0)->visible = false;
        DeckClock::tick(deck, 0.5f, noop);
        CHECK(deck.getLayer(0)->runtime().crossfadeProgress == Approx(0.0f));
        CHECK(deck.getLayer(1)->runtime().crossfadeProgress == Approx(0.25f));
    }
    SECTION("bypassed")
    {
        deck.getLayer(1)->bypassed = true;
        DeckClock::tick(deck, 0.5f, noop);
        CHECK(deck.getLayer(1)->runtime().crossfadeProgress == Approx(0.0f));
        CHECK(deck.getLayer(0)->runtime().crossfadeProgress == Approx(0.25f));
    }
    SECTION("solo on another layer")
    {
        deck.getLayer(2)->solo = true;
        DeckClock::tick(deck, 0.5f, noop);
        CHECK(deck.getLayer(0)->runtime().crossfadeProgress == Approx(0.0f));
        CHECK(deck.getLayer(1)->runtime().crossfadeProgress == Approx(0.0f));
        CHECK(deck.getLayer(2)->runtime().crossfadeProgress == Approx(0.25f));
    }
}

TEST_CASE("(d) the media clock ticks playable clips only: the active one, and the outgoing one during a fade", "[deck_clock]")
{
    Deck deck; deck.initDefault();
    startFade(deck, 0, 1.0f, Clip::MediaType::ImageSequence, Clip::MediaType::Video);   // out = sequence, in = video
    startFade(deck, 1, 1.0f, Clip::MediaType::Image, Clip::MediaType::Source);          // never ticked
    auto* layer = deck.getLayer(0);

    Recorder rec;
    DeckClock::tick(deck, 0.5f, std::ref(rec));
    REQUIRE(rec.ticked.size() == 2);
    CHECK(rec.ticked[0] == layer->getClipAt(1));   // active Video
    CHECK(rec.ticked[1] == layer->getClipAt(0));   // outgoing ImageSequence while fading

    rec.ticked.clear();
    DeckClock::tick(deck, 0.5f, std::ref(rec));    // this tick completes the fade
    REQUIRE(layer->runtime().previousClipColumn == -1);
    REQUIRE(rec.ticked.size() == 1);               // outgoing not ticked on the completing frame (applyTransition parity)
    CHECK(rec.ticked[0] == layer->getClipAt(1));

    rec.ticked.clear();
    DeckClock::tick(deck, 0.5f, std::ref(rec));
    REQUIRE(rec.ticked.size() == 1);
    CHECK(rec.ticked[0] == layer->getClipAt(1));
}

TEST_CASE("(e) LayerClock::advanceCrossfade: step = dt / duration, 0.5 s default, clamps, clears previousClipColumn", "[deck_clock]")
{
    Layer layer;
    layer.ensureColumns(2);
    {
        LayerRuntimeSnapshot rt = layer.runtime();
        rt.previousClipColumn = 0;
        rt.activeClipColumn = 1;
        rt.crossfadeProgress = 0.0f;
        layer.setRuntime(rt);
    }

    SECTION("step = dt / duration")
    {
        layer.transitionSpeed = 4.0f;
        LayerClock::advanceCrossfade(layer, 1.0f);
        CHECK(layer.runtime().crossfadeProgress == Approx(0.25f));
        CHECK(layer.runtime().previousClipColumn == 0);
    }
    SECTION("duration <= 0 uses 0.5 s")
    {
        layer.transitionSpeed = 0.0f;
        LayerClock::advanceCrossfade(layer, 0.125f);
        CHECK(layer.runtime().crossfadeProgress == Approx(0.25f));
    }
    SECTION("clamps at 1.0 and clears previousClipColumn")
    {
        layer.transitionSpeed = 1.0f;
        LayerClock::advanceCrossfade(layer, 5.0f);
        CHECK(layer.runtime().crossfadeProgress == Approx(1.0f));
        CHECK(layer.runtime().previousClipColumn == -1);
    }
    SECTION("no fade in progress: untouched")
    {
        {
            LayerRuntimeSnapshot rt = layer.runtime();
            rt.previousClipColumn = -1;
            layer.setRuntime(rt);
        }
        layer.transitionSpeed = 1.0f;
        LayerClock::advanceCrossfade(layer, 0.5f);
        CHECK(layer.runtime().crossfadeProgress == Approx(0.0f));
    }
}

TEST_CASE("(f) AutopilotBank: every deck keeps its own beat-crossing baseline (Pitfall 38)", "[deck_clock][autopilot]")
{
    // Two decks, layer 0 of each on autopilot Beat4 / PlayNext over three columns (test_autopilot.cpp's setup).
    auto makeDeck = []() {
        Deck deck; deck.initDefault();
        for (int c = 0; c < 3; ++c)
        {
            Clip clip = makeClip(static_cast<uint32_t>(200 + c), Clip::MediaType::Image);
            clip.autopilotAction = Clip::AutopilotAction::PlayNext;
            clip.autopilotDuration = Clip::AutopilotDuration::Beat4;
            deck.setClip(0, c, clip);
        }
        deck.getLayer(0)->autopilotEnabled = true;
        deck.getLayer(0)->triggerClip(0);       // first trigger sets clip->playing
        return deck;
    };
    Deck d0 = makeDeck(), d1 = makeDeck();
    FeatureSnapshot snap;

    SECTION("one Autopilot per deck index: both decks advance on the same crossings")
    {
        AutopilotBank bank;
        for (int beat = 0; beat < 4; ++beat)
        {
            snap.beatPhase = 0.99f;
            bank.forIndex(0).processFrame(d0, snap); bank.forIndex(1).processFrame(d1, snap);
            snap.beatPhase = 0.01f; snap.totalBeatCount++;
            bank.forIndex(0).processFrame(d0, snap); bank.forIndex(1).processFrame(d1, snap);
        }
        CHECK(bank.size() == 2);
        CHECK(d0.getLayer(0)->runtime().activeClipColumn == 1);
        CHECK(d1.getLayer(0)->runtime().activeClipColumn == 1);
    }

    SECTION("teeth: ONE shared Autopilot driven for both decks lets only the first see each crossing")
    {
        Autopilot shared;
        for (int beat = 0; beat < 4; ++beat)
        {
            snap.beatPhase = 0.99f;
            shared.processFrame(d0, snap); shared.processFrame(d1, snap);
            snap.beatPhase = 0.01f; snap.totalBeatCount++;
            shared.processFrame(d0, snap); shared.processFrame(d1, snap);
        }
        CHECK(d0.getLayer(0)->runtime().activeClipColumn == 1);
        CHECK(d1.getLayer(0)->runtime().activeClipColumn == 0);   // the second call never sees a crossing
    }

    SECTION("config set from any thread reaches every instance handed out")
    {
        AutopilotBank bank;
        Composition::PerTypeAutopilotConfig cfg;
        cfg.perTypeEnabled = true;
        cfg.opaqueCycleBeats = 1;                // Opaque layer 0: every beat
        bank.setPerTypeConfig(&cfg);
        snap.beatPhase = 0.99f; bank.forIndex(1).processFrame(d1, snap);
        snap.beatPhase = 0.01f; snap.totalBeatCount++; bank.forIndex(1).processFrame(d1, snap);
        CHECK(d1.getLayer(0)->runtime().activeClipColumn == 1);
    }
}
