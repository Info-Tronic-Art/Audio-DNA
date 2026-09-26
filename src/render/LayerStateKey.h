#pragma once

#include <cstdint>

// s-rta-0926b render lane (R2): the key of a layer's per-frame GL state in
// CompositorEngine -- its temporal buffer (u_prev_frame), its Screen Split /
// Frame Stutter frame ring, and its feedback processor.
//
// Layer ids are per DECK (Deck::initDefault numbers every deck's layers
// 0, 1, 2 ...), so keying that state by layer id alone let a persistent layer
// from another deck share it with the active deck's layer of the same id
// (measured: deck 1's persistent Freeze read deck 0's frames -- 33% of the
// other deck's image in steady state; with Frame Stutter the persistent layer
// showed the other deck's image outright). The key carries the deck id in the
// high 32 bits.
//
// A layer has two effect chains and each keeps its own state (s-rta-0926
// xfade: a clip chain and a layer chain that shared one temporal buffer each
// read the OTHER chain's output as "previous frame"): the layer chain's key
// sets kLayerChainBit in the layer half. Real layer ids never reach that bit.
//
// kGlobalEffects keys the composition-wide Global Effects chain; no real
// (deck, layer) pair produces it (it would need deck id 0xFFFFFFFF and layer id
// 0x7FFFFFFF on the layer chain).
//
// Pure (no GL), so tests/test_layer_state_key.cpp drives it directly.
namespace LayerStateKey
{
    inline constexpr std::uint32_t kLayerChainBit = 0x80000000u;
    inline constexpr std::uint64_t kGlobalEffects = ~std::uint64_t{ 0 };

    // State of the layer's CLIP chain (clip effects, the transition's outgoing
    // clip chain) and of the layer's feedback processor.
    constexpr std::uint64_t clipChain(std::uint32_t deckId, std::uint32_t layerId)
    {
        return (std::uint64_t{ deckId } << 32) | std::uint64_t{ layerId };
    }

    // State of the layer's LAYER-effects chain.
    constexpr std::uint64_t layerChain(std::uint32_t deckId, std::uint32_t layerId)
    {
        return clipChain(deckId, layerId | kLayerChainBit);
    }
}
