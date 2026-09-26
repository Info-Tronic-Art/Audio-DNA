#include <catch2/catch_test_macros.hpp>
#include "render/ScratchPool.h"

// s-rta-0925 ms-white2: a SECOND, separate instance of the same GL
// feedback-loop aliasing class fixed by 52cd76c (see
// tests/test_compositor_opacity_alias.cpp for that fix) -- found while
// checking "do other call orders hit the same aliasing?" per
// .harmony/.reports/s-rta-0925/ms-white-diagnosis-2.md section 7/8.
//
// ORIGINAL bug: CompositorEngine::applyClipEffects hardcoded
// `int writeFBO = 0;` -- its ping-pong ALWAYS started its first enabled
// effect's write at effectFBO_A_/effectTex_A_, regardless of its own
// inputTex. compositeDeck() calls applyClipEffects TWICE per Opaque/
// Transparent layer, back to back with no intervening copy (per-clip
// effects, then -- if the layer has any enabled layer effects -- per-layer
// effects, with the first call's output as the second call's inputTex). An
// ODD enabled per-clip effect count left the first call's output on
// effectTex_A_, so the second call's hardcoded restart at effectFBO_A_
// aliased it -- the identical read/write-same-texture GL feedback-loop
// hazard as 52cd76c's mechanism, at a different call-site pairing.
//
// ORIGINAL fix: applyClipEffects picked its OWN starting write target from
// inputTex's identity (`writeFBO = (inputTex == effectTex_A_) ? 1 : 0`).
//
// s-rta-0926 xfade lane REPLACED that parity-based starting-index decision
// entirely: applyClipEffects no longer tracks a `writeFBO` index at all.
// Every enabled effect's pass -- the first AND every subsequent one, in
// EVERY call -- asks CompositorEngine::pickEffectTarget(currentInput,
// holdTex) fresh (CompositorEngine.cpp ~:284-352, render/ScratchPool.h's
// pick()), which never returns the texture that pass samples. Both
// back-to-back calls at compositeDeck's call sites (CompositorEngine.cpp
// ~:901 per-clip, ~:919 per-layer) pass holdTex=0 (its default), so the
// layer call's first pass reads inputTex=<per-clip call's output> and picks
// a target away from THAT -- automatically safe for ANY per-clip effect
// count, not just the odd/even cases the original parity fix special-cased.
//
// s-rta-0926 cleanup lane (this file): the OLD version of this test mirrored
// the REMOVED index-parity formula ((startWriteFBO + N - 1) % 2) with a
// hardcoded 2-member A/B model -- describing code that no longer exists (the
// pool is 3 members now, and there is no "starting index" concept left to
// mirror). Rewritten to drive the REAL selection function
// (ScratchPool::pick) through the actual per-pass sequence applyClipEffects
// now runs, across the same odd/even/zero per-clip effect counts the
// original diagnosis used (v12/v13 fixtures), to pin down that the
// generalized per-pass picker is alias-free for ANY count -- not just the
// ones the old hardcoded-then-patched code happened to get right.
//
// Driving the real CompositorEngine::applyClipEffects headlessly remains
// infeasible (real GL calls, no live context in this unit-test harness --
// see test_compositor_opacity_alias.cpp's identical note). Exhaustive
// coverage of pick() itself lives in test_scratch_pool.cpp; this file covers
// the scenario-level wiring it explicitly disclaims -- specifically, that
// applyClipEffects' per-pass loop, chained across the two real call sites
// with holdTex=0, never aliases regardless of effect-count parity.

namespace
{

using Tex = unsigned int; // GLuint-shaped handle, same convention as test_scratch_pool.cpp

// Stand-ins for effectTex_A_/B_/C_, matching test_compositor_opacity_alias.cpp.
constexpr Tex kPool[3] = { 101u, 102u, 103u };

// A clip's own persistent texture (image/video/source) -- never a pool member.
constexpr Tex kSrcTex = 100u;

Tex pick(Tex readTex, Tex holdTex)
{
    const int i = ScratchPool::pick(kPool, readTex, holdTex);
    REQUIRE(i >= 0);
    REQUIRE(i < 3);
    return kPool[static_cast<std::size_t>(i)];
}

// Mirrors applyClipEffects' per-pass loop (CompositorEngine.cpp ~:272-447):
// every enabled effect picks its target fresh from the CURRENT input, and
// writes there -- dry/wet, temporal, screen_split/frame_delay all still end
// each pass with the result in that pass's picked target (see
// EffectChain.cpp's dry/wet fix in this same lane for the analogous
// in-place-blend case). Zero enabled effects is the effects.empty() early
// return: inputTex passes straight through untouched, no pick() call at all.
Tex applyClipEffectsSim(Tex inputTex, int numEnabledEffects, Tex holdTex = 0)
{
    Tex currentInput = inputTex;
    for (int i = 0; i < numEnabledEffects; ++i)
        currentInput = pick(currentInput, holdTex);
    return currentInput;
}

} // namespace

TEST_CASE("odd per-clip effect count + a layer effect: the real picker never aliases across the two chained calls",
          "[compositor][effects][ms-white2][scratch_pool]")
{
    // v12_perclip_odd_plus_layereffect's shape: 1 (and, for good measure, 3)
    // enabled per-clip effects, then a second applyClipEffects call for 1
    // enabled layer effect, inputTex = the first call's output.
    for (int perClip : { 1, 3 })
    {
        INFO("perClip=" << perClip);
        Tex clipTexAfterPerClip = applyClipEffectsSim(kSrcTex, perClip);

        // What the layer call's first (and only) pass will pick, BEFORE
        // running it -- must differ from what it's about to sample.
        Tex firstLayerPassTarget = pick(clipTexAfterPerClip, 0);
        CHECK(firstLayerPassTarget != clipTexAfterPerClip);

        Tex final = applyClipEffectsSim(clipTexAfterPerClip, /*numEnabledEffects=*/1);
        CHECK(final == firstLayerPassTarget);
    }
}

TEST_CASE("even per-clip effect count + a layer effect: unaffected control, still safe",
          "[compositor][effects][ms-white2][scratch_pool]")
{
    // v13's control fixture shape (even count -- already safe under the
    // OLD hardcoded-writeFBO=0 code too, since the parity happened to land
    // the first call's output off effectTex_A_). Included at 2 and 4 to
    // show the new mechanism doesn't care about parity at all.
    for (int perClip : { 2, 4 })
    {
        INFO("perClip=" << perClip);
        Tex clipTexAfterPerClip = applyClipEffectsSim(kSrcTex, perClip);
        Tex firstLayerPassTarget = pick(clipTexAfterPerClip, 0);
        CHECK(firstLayerPassTarget != clipTexAfterPerClip);
    }
}

TEST_CASE("zero per-clip effects + a layer effect: clip's persistent texture passes straight through, still safe",
          "[compositor][effects][ms-white2][scratch_pool]")
{
    // No per-clip effects at all: the effects.empty() early return hands the
    // clip's own persistent texture straight through -- never a pool
    // texture, so there is no aliasing precondition in this case, fixed or
    // not.
    Tex clipTexAfterPerClip = applyClipEffectsSim(kSrcTex, /*numEnabledEffects=*/0);
    CHECK(clipTexAfterPerClip == kSrcTex);

    Tex firstLayerPassTarget = pick(clipTexAfterPerClip, 0);
    CHECK(firstLayerPassTarget != clipTexAfterPerClip);
}
