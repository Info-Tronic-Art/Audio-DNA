#pragma once

#include <cstdint>

// s-rta-0926b render lane (R2): the key of a layer's per-frame GL state in
// CompositorEngine -- its temporal buffer (u_prev_frame), its Screen Split /
// Frame Stutter frame ring, and its feedback processor.
//
// Layer ids restart at 0 on every deck (Deck::initDefault numbers every deck's
// layers 0, 1, 2 ...), so keying that state by layer id alone would hand deck
// A's layer-0 history to deck B's layer 0 after a deck switch. The key carries
// the deck id in the high 32 bits.
//
// A layer has THREE state keys, one per effect chain that can run on it, and
// each chain keeps its own state (s-rta-0926 xfade: a clip chain and a layer
// chain that shared one temporal buffer each read the OTHER chain's output as
// "previous frame"):
//   clipChain      the active clip's chain (and the layer's feedback processor)
//   outgoingChain  the OUTGOING clip's chain during a clip-to-clip crossfade
//                  (s-rta-0926b R1: while a layer crossfades, two clip chains
//                  run on it every frame; sharing one key made each read the
//                  other's output -- measured 1/3 ghost at Freeze 0.5, the other
//                  clip outright with Frame Stutter). Sets kOutgoingChainBit.
//   layerChain     the layer-effects chain. Sets kLayerChainBit.
// Both bits live in the layer half. Real layer ids never reach bit 30 or 31
// (Deck::nextLayerId_ starts at 100 and counts up), so the three keys of a
// (deck, layer) never meet each other or another layer's keys.
//
// kGlobalEffects keys the composition-wide Global Effects chain; no real
// (deck, layer) pair produces it (it would need deck id 0xFFFFFFFF and layer id
// 0x7FFFFFFF on the layer chain).
//
// Pure (no GL), so tests/test_layer_state_key.cpp drives it directly.
namespace LayerStateKey
{
    inline constexpr std::uint32_t kLayerChainBit = 0x80000000u;
    inline constexpr std::uint32_t kOutgoingChainBit = 0x40000000u;
    inline constexpr std::uint64_t kGlobalEffects = ~std::uint64_t{ 0 };

    // State of the layer's CLIP chain (the active clip's effects) and of the
    // layer's feedback processor.
    constexpr std::uint64_t clipChain(std::uint32_t deckId, std::uint32_t layerId)
    {
        return (std::uint64_t{ deckId } << 32) | std::uint64_t{ layerId };
    }

    // State of the OUTGOING clip's chain while the layer crossfades (the
    // compositor hands the clip chain's history over to it at the first frame
    // of every crossfade -- CompositorEngine::handOverClipHistory).
    constexpr std::uint64_t outgoingChain(std::uint32_t deckId, std::uint32_t layerId)
    {
        return clipChain(deckId, layerId | kOutgoingChainBit);
    }

    // State of the layer's LAYER-effects chain.
    constexpr std::uint64_t layerChain(std::uint32_t deckId, std::uint32_t layerId)
    {
        return clipChain(deckId, layerId | kLayerChainBit);
    }
}
