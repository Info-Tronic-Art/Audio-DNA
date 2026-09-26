#pragma once

#include <cstddef>

// s-rta-0926 xfade: the compositor's ONE rule for its shared effect scratch
// targets (CompositorEngine's effectFBO_A_/B_/C_). Three earlier bugs were
// the same shape -- a pass rendered into the scratch texture that it, or its
// caller, was still reading:
//   52cd76c  clip transform -> clip effects (sampled + drew effectTex_A_)
//   4fca2c5  clip effects -> layer effects (sampled + drew effectTex_A_)
//   xfade    incoming clip effects -> the transition's outgoing clip effects
//            (overwrote effectTex_A_ while the incoming result was held in it)
// Each fix protected one call-site pairing. This picker replaces them: every
// pass that renders into the pool asks for a target that is neither the
// texture the pass samples (readTex) nor a texture its caller still holds
// (holdTex). With three members there is always a free one.
//
// Pure (no GL): Tex is a texture handle type; 0 is never a pool member, so
// passing 0 for "nothing held" never excludes anything.
namespace ScratchPool
{
    template <typename Tex, std::size_t N>
    int pick(const Tex (&pool)[N], Tex readTex, Tex holdTex)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if (pool[i] != readTex && pool[i] != holdTex)
                return static_cast<int>(i);
        }
        return -1; // only reachable with N < 3: every member excluded
    }
}
