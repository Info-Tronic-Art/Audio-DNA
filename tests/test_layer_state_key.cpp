// test_layer_state_key.cpp -- s-rta-0926b render lane (R2).
//
// Covers the REAL key function CompositorEngine uses for every layer's
// per-frame GL state (temporal buffer, Screen Split / Frame Stutter ring,
// feedback processor): render/LayerStateKey.h. It is pure (no GL), so this is
// not a mirror of compositor logic.
//
// The bug it pins: layer ids are per deck (Deck::initDefault numbers each
// deck's layers 0, 1, 2 ...), and the state used to be keyed by layer id alone,
// so a persistent layer of another deck shared state with the active deck's
// layer of the same id.
//
// What it does NOT cover: that every compositor call site passes the right key
// (GL call sites) -- covered live by .harmony/probe-render-state.sh rows
// R2-temporal and R2-ring (pixel-decoded frames).

#include <catch2/catch_test_macros.hpp>
#include "model/Deck.h"
#include "render/LayerStateKey.h"

#include <cstdint>
#include <set>

TEST_CASE("LayerStateKey: the same layer id in two decks gets two keys", "[layer_state_key]")
{
    // Two decks built the way the app builds them: both number their layers 0, 1, 2.
    Deck a; a.id = 0; a.initDefault();
    Deck b; b.id = 1; b.initDefault();
    REQUIRE(a.layers[0].id == b.layers[0].id);   // the collision precondition

    for (size_t i = 0; i < a.layers.size(); ++i)
    {
        INFO("layer index " << i);
        REQUIRE(LayerStateKey::clipChain(a.id, a.layers[i].id) != LayerStateKey::clipChain(b.id, b.layers[i].id));
        REQUIRE(LayerStateKey::layerChain(a.id, a.layers[i].id) != LayerStateKey::layerChain(b.id, b.layers[i].id));
    }
}

TEST_CASE("LayerStateKey: every (deck, layer, chain) is unique and never the Global Effects key",
          "[layer_state_key]")
{
    std::set<std::uint64_t> seen;
    const std::uint32_t decks[] = { 0u, 1u, 2u, 7u, 0xFFFFFFFEu };
    const std::uint32_t layers[] = { 0u, 1u, 2u, 3u, 15u, 1000u, 0x7FFFFFFEu };
    for (std::uint32_t d : decks)
    {
        for (std::uint32_t l : layers)
        {
            const std::uint64_t clip = LayerStateKey::clipChain(d, l);
            const std::uint64_t lay = LayerStateKey::layerChain(d, l);
            INFO("deck " << d << " layer " << l);
            REQUIRE(clip != lay);                                // a layer's two chains never share state
            REQUIRE(clip != LayerStateKey::kGlobalEffects);
            REQUIRE(lay != LayerStateKey::kGlobalEffects);
            REQUIRE(seen.insert(clip).second);
            REQUIRE(seen.insert(lay).second);
        }
    }
}

TEST_CASE("LayerStateKey: a deck's own keys are stable (same inputs, same key)", "[layer_state_key]")
{
    // A persistent layer is composited by compositeDeck while its deck is active
    // and by compositePersistentLayers otherwise; both must reach the SAME state.
    REQUIRE(LayerStateKey::clipChain(3u, 2u) == LayerStateKey::clipChain(3u, 2u));
    REQUIRE(LayerStateKey::layerChain(3u, 2u) == LayerStateKey::layerChain(3u, 2u));
    static_assert(LayerStateKey::clipChain(0u, 0u) == 0u, "deck 0 / layer 0 keeps key 0");
}
