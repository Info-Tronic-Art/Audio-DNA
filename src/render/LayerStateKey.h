#pragma once

#include <cstdint>

// s-rta-0926b render lane (R2): the key of a layer's per-frame GL state in
// CompositorEngine -- its temporal buffer (u_prev_frame), its Screen Split /
// Frame Stutter frame ring, and its feedback processor.
//
// The high 32 bits are the stack half. Lane bf9b (s-rta-1002b): the show has ONE
// shared layer stack whose layer ids are unique per show, so every layer keys with
// kShowStackKey -- no GL key carries a deck, and a deck switch can never hand one
// layer's history to another (Pitfall 35).
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
// (Composition's layer-id counter starts at 100 and counts up), so the three keys
// of a layer never meet each other or another layer's keys.
//
// kGlobalEffects keys the composition-wide Global Effects chain; no real
// (stack, layer) pair produces it (it would need stack key 0xFFFFFFFF and layer id
// 0x7FFFFFFF on the layer chain).
//
// Pure (no GL), so tests/test_layer_state_key.cpp drives it directly.
namespace LayerStateKey
{
    inline constexpr std::uint32_t kLayerChainBit = 0x80000000u;
    inline constexpr std::uint32_t kOutgoingChainBit = 0x40000000u;
    inline constexpr std::uint64_t kGlobalEffects = ~std::uint64_t{ 0 };
    // The stack half of every shared layer's keys (lane bf9b).
    inline constexpr std::uint32_t kShowStackKey = 0;

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
