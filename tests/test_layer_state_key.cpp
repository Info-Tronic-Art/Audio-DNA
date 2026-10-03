// test_layer_state_key.cpp -- s-rta-0926b render lane (R2).
//
// Covers the REAL key function CompositorEngine uses for every layer's
// per-frame GL state (temporal buffer, Screen Split / Frame Stutter ring,
// feedback processor): render/LayerStateKey.h. It is pure (no GL), so this is
// not a mirror of compositor logic.
//
// The bug it pinned: layer ids repeated across decks, so keying the state by layer
// id alone would hand deck A's layer-0 history to deck B's layer 0 after a deck
// switch. Lane bf9b: one shared stack whose layer ids are unique per show keys
// every layer with kShowStackKey; the two-half key function stays pinned here.
//
// What it does NOT cover: that every compositor call site passes the right key
// (GL call sites). The live per-deck call-site rows r2_* were retired with
// Persistent (bf9 Stage P); successor: bf9b gate K1t.

#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "render/LayerStateKey.h"

#include <cstdint>
#include <set>

TEST_CASE("LayerStateKey: the same layer id in two decks gets two keys", "[layer_state_key]")
{
    // Lane bf9b: decks hold no layers any more (one shared stack, keyed with kShowStackKey), but the key function
    // keeps its (stack, layer) halves: the same layer ids under two stack halves must still give two keys.
    Composition show; show.initDefault();
    const std::uint32_t a = LayerStateKey::kShowStackKey, b = a + 1;

    for (size_t i = 0; i < show.layers.size(); ++i)
    {
        INFO("layer index " << i);
        const std::uint32_t id = show.layers[i].id;
        REQUIRE(LayerStateKey::clipChain(a, id) != LayerStateKey::clipChain(b, id));
        REQUIRE(LayerStateKey::layerChain(a, id) != LayerStateKey::layerChain(b, id));
    }
}

TEST_CASE("LayerStateKey: every (deck, layer, chain) is unique and never the Global Effects key",
          "[layer_state_key]")
{
    std::set<std::uint64_t> seen;
    const std::uint32_t decks[] = { 0u, 1u, 2u, 7u, 0xFFFFFFFEu };
    // s-rta-0926b R1: the largest layer id is 0x3FFFFFFF (was 0x7FFFFFFE) -- the
    // outgoing-chain bit is bit 30, and real layer ids never reach it
    // (Deck::nextLayerId_ starts at 100).
    const std::uint32_t layers[] = { 0u, 1u, 2u, 3u, 15u, 1000u, 0x3FFFFFFFu };
    for (std::uint32_t d : decks)
    {
        for (std::uint32_t l : layers)
        {
            const std::uint64_t clip = LayerStateKey::clipChain(d, l);
            const std::uint64_t lay = LayerStateKey::layerChain(d, l);
            const std::uint64_t outgoing = LayerStateKey::outgoingChain(d, l);
            INFO("deck " << d << " layer " << l);
            REQUIRE(clip != lay);                                // a layer's chains never share state
            REQUIRE(outgoing != clip);                           // s-rta-0926b R1: the two clip chains of
            REQUIRE(outgoing != lay);                            // a crossfade never share state either
            REQUIRE(clip != LayerStateKey::kGlobalEffects);
            REQUIRE(lay != LayerStateKey::kGlobalEffects);
            REQUIRE(outgoing != LayerStateKey::kGlobalEffects);
            REQUIRE(seen.insert(clip).second);
            REQUIRE(seen.insert(lay).second);
            REQUIRE(seen.insert(outgoing).second);
        }
    }
}

TEST_CASE("LayerStateKey: a deck's own keys are stable (same inputs, same key)", "[layer_state_key]")
{
    // A layer reaches its state through the key every frame its deck is shown;
    // the same inputs must reach the SAME state.
    REQUIRE(LayerStateKey::clipChain(3u, 2u) == LayerStateKey::clipChain(3u, 2u));
    REQUIRE(LayerStateKey::layerChain(3u, 2u) == LayerStateKey::layerChain(3u, 2u));
    REQUIRE(LayerStateKey::outgoingChain(3u, 2u) == LayerStateKey::outgoingChain(3u, 2u));
    static_assert(LayerStateKey::clipChain(0u, 0u) == 0u, "deck 0 / layer 0 keeps key 0");
}
