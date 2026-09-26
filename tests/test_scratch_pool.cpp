// test_scratch_pool.cpp -- s-rta-0926 xfade lane.
//
// Covers the REAL selection function the compositor uses for every write into
// its effect scratch pool (render/ScratchPool.h, called by
// CompositorEngine::pickEffectTarget). It is pure (no GL), so this is not a
// mirror of compositor logic: it exercises the one rule itself -- "a pass never
// renders into the texture it samples, nor into one its caller still holds".
//
// What it does NOT cover: that every compositor call site actually routes its
// target through the picker, or anything about GL/driver behaviour. That is
// covered live by .harmony/probe-crossfade.sh (pixel-decoded frames) and was
// traced with the env-gated FBO trace (.harmony/.reports/s-rta-0926/
// parity-trace.diff) -- see .harmony/.reports/s-rta-0926/xfade-report.md.

#include <catch2/catch_test_macros.hpp>
#include "render/ScratchPool.h"

#include <vector>

namespace
{
    using Tex = unsigned int;                 // GLuint-shaped handle
    constexpr Tex kPool[3] = { 3u, 4u, 11u }; // A, B, C (arbitrary non-zero ids)
    constexpr Tex kExternal[] = { 0u, 8u, 99u }; // 0 = "nothing held"; image/source ids
}

TEST_CASE("ScratchPool::pick never returns the sampled or the held texture", "[scratch_pool]")
{
    std::vector<Tex> candidates(std::begin(kPool), std::end(kPool));
    candidates.insert(candidates.end(), std::begin(kExternal), std::end(kExternal));

    for (Tex readTex : candidates)
    {
        for (Tex holdTex : candidates)
        {
            const int i = ScratchPool::pick(kPool, readTex, holdTex);
            INFO("readTex=" << readTex << " holdTex=" << holdTex);
            REQUIRE(i >= 0);
            REQUIRE(i < 3);
            REQUIRE(kPool[i] != readTex);
            REQUIRE(kPool[i] != holdTex);
        }
    }
}

TEST_CASE("ScratchPool::pick keeps the historic A/B ping-pong when nothing is held", "[scratch_pool]")
{
    // External input -> A, then each pass reads the previous result: A -> B -> A.
    Tex current = 8u;
    const Tex expected[] = { 3u, 4u, 3u, 4u };
    for (Tex want : expected)
    {
        const int i = ScratchPool::pick(kPool, current, Tex{ 0 });
        REQUIRE(kPool[i] == want);
        current = kPool[i];
    }
}

TEST_CASE("ScratchPool::pick uses the third member only while a pool texture is held", "[scratch_pool]")
{
    // The crossfade shape: incoming clip result held in A, outgoing clip's
    // chain starts from its own (external) texture.
    const Tex held = 3u;
    const int first = ScratchPool::pick(kPool, Tex{ 8u }, held);
    REQUIRE(kPool[first] == 4u);                        // B, not A
    const int second = ScratchPool::pick(kPool, kPool[first], held);
    REQUIRE(kPool[second] == 11u);                      // C: neither B (read) nor A (held)
    const int third = ScratchPool::pick(kPool, kPool[second], held);
    REQUIRE(kPool[third] == 4u);                        // back to B, A stays untouched
}

TEST_CASE("ScratchPool::pick with only two members cannot honour a pool hold", "[scratch_pool]")
{
    // Why the pool has three members: A/B alone has no free target for a pass
    // that reads B while its caller holds A (returns -1, never a clobber).
    constexpr Tex twoPool[2] = { 3u, 4u };
    REQUIRE(ScratchPool::pick(twoPool, Tex{ 4u }, Tex{ 3u }) == -1);
    REQUIRE(ScratchPool::pick(twoPool, Tex{ 8u }, Tex{ 3u }) == 1);
}
