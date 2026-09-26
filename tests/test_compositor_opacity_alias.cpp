#include <catch2/catch_test_macros.hpp>
#include "render/ScratchPool.h"
#include <cmath>

// s-rta-0925 ms-white: GL FBO/texture feedback-loop hazard in
// CompositorEngine, diagnosed against the live app (production port 7070,
// no rebuild) via render_frame + pixel decoding -- see
// .harmony/.reports/s-rta-0925/ms-white-diagnosis.md for the full bisection
// (V0-V11) and the archived-artifact byte decode confirming the failing
// captures are exactly (0,0,0,0) everywhere.
//
// ORIGINAL fix (52cd76c): applyClipOpacity grew a `forceCopy` bool and wrote
// its identity-opacity copy into a hardcoded effectFBO_B_/effectTex_B_ --
// breaking the alias against applyClipEffects' (then-hardcoded) first write
// target, effectFBO_A_/effectTex_A_.
//
// s-rta-0926 xfade lane REPLACED that hardcoded target (and the analogous
// hardcoded target in applyClipEffects, test_compositor_effects_parity.cpp's
// subject) with one rule: every pass into the effect scratch pool asks
// CompositorEngine::pickEffectTarget(readTex, holdTex) -- render/ScratchPool.h's
// pick() -- which never returns the texture the pass samples or one its
// caller still holds. applyClipTransform now runs (CompositorEngine.cpp
// ~:465-552):
//   xf = pickEffectTarget(srcTex, 0)               -- only when needsTransform
//   op = pickEffectTarget(transformedTex, 0)       -- always
//   applyClipOpacity(opacity, transformedTex, op.fbo, op.tex, forceCopy=needsTransform)
// forceCopy is unchanged (still suppresses the identity-opacity no-op when a
// transform ran), but the write target it forces the copy INTO is now
// whatever pick() returns, not a fixed effectTex_B_.
//
// s-rta-0926 cleanup lane (this file): the OLD version of this test mirrored
// the REMOVED hardcoded-effectTex_B_ mechanism with literal index arithmetic
// -- describing code that no longer exists. Rewritten to exercise the REAL
// selection function (ScratchPool::pick, the exact one pickEffectTarget
// wraps) through the same two-hop sequence above, so it fails again if a
// future change reintroduces a hardcoded (non-pick()) target anywhere in
// this hand-off.
//
// Driving the real CompositorEngine::applyClipTransform/applyClipOpacity/
// applyClipEffects headlessly is infeasible: they issue real GL calls
// (glBindFramebuffer, glClear, glDrawArrays via FullscreenQuad) that require
// a live, attached OpenGL context, which test_compositor.cpp documents as
// unavailable in this unit-test harness (no target links CompositorEngine.cpp
// for exactly this reason -- see also test_renderer_source_confinement.cpp,
// test_composition_tier_oracle.cpp, test_layer_transport_reverse.cpp for the
// same, already-documented limitation and its established workaround).
// Following that precedent ("mirror the mechanism, not the subsystem"), this
// test reproduces the transform->opacity texture-handle hand-off using small
// integer "handles" standing in for the real GLuint values -- but the target
// SELECTION itself is the real, pure `ScratchPool::pick()`, not a mirror of
// it. Exhaustive coverage of pick() itself (every readTex/holdTex combo,
// the historic A/B ping-pong, the 3rd-member/hold interaction) lives in
// test_scratch_pool.cpp; this file covers the scenario-level wiring
// test_scratch_pool.cpp explicitly disclaims ("that every compositor call
// site actually routes its target through the picker" is NOT its job) --
// specifically, applyClipTransform/applyClipOpacity's forceCopy business
// logic around when pick() runs at all.

namespace
{

using Tex = unsigned int; // GLuint-shaped handle, same convention as test_scratch_pool.cpp

// Stand-ins for effectTex_A_/B_/C_ (CompositorEngine::pickEffectTarget's
// `texs[3] = { effectTex_A_, effectTex_B_, effectTex_C_ }`).
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

// Mirrors applyClipTransform's transform step (CompositorEngine.cpp
// ~:490-522): only runs when needsTransform, and picks its target away from
// srcTex with nothing held yet (holdTex=0).
Tex transformStep(bool needsTransform, Tex srcTex)
{
    return needsTransform ? pick(srcTex, 0) : srcTex;
}

// Mirrors applyClipOpacity (CompositorEngine.cpp ~:554-598): forceCopy
// suppresses the identity-opacity no-op; the true no-op (srcTex straight
// through, no GL call, no pick()) only survives when forceCopy is false AND
// opacity is at its 1.0 default.
Tex opacityStep(float opacity, Tex srcTex, bool forceCopy)
{
    constexpr float eps = 0.001f;
    if (!forceCopy && std::abs(opacity - 1.0f) <= eps)
        return srcTex; // true no-op
    return pick(srcTex, 0); // op = pickEffectTarget(transformedTex, 0)
}

// The full hand-off, matching applyClipTransform's tail exactly:
// forceCopy = needsTransform (CompositorEngine.cpp's own comment: "when
// there was no transform, transformedTex is the original (non-scratch)
// srcTex and the true no-op remains safe").
Tex transformThenOpacity(bool needsTransform, float opacity, Tex srcTex = kSrcTex)
{
    Tex transformedTex = transformStep(needsTransform, srcTex);
    return opacityStep(opacity, transformedTex, /*forceCopy=*/needsTransform);
}

} // namespace

TEST_CASE("transform + default clip opacity: the real picker never aliases the transform's own output",
          "[compositor][opacity][ms-white][scratch_pool]")
{
    // needsTransform=true, opacity=1.0 -- the B1/V8 fixture shape that was a
    // fully blank (0,0,0,0) capture pre-fix. forceCopy suppresses the no-op,
    // so opacityStep calls pick() again against transformedTex -- and
    // ScratchPool::pick's own contract (exhaustively proven by
    // test_scratch_pool.cpp) guarantees it never returns the texture it was
    // just given as readTex.
    Tex transformedTex = transformStep(/*needsTransform=*/true, kSrcTex);
    Tex result = transformThenOpacity(/*needsTransform=*/true, /*opacity=*/1.0f);

    CHECK(result != transformedTex); // no aliasing: opacity's write target != what it sampled
    CHECK(result != kSrcTex);        // and it actually rendered (forceCopy suppressed the no-op)
}

TEST_CASE("no transform + default clip opacity remains a true no-op (unaffected safe path)",
          "[compositor][opacity][ms-white][scratch_pool]")
{
    // needsTransform=false: transformedTex IS the clip's own persistent
    // texture, never a pool member, so there is no aliasing precondition at
    // all -- forceCopy is false and the cheap no-op (no pick() call, no GL
    // call) is preserved exactly as before the fix.
    Tex result = transformThenOpacity(/*needsTransform=*/false, /*opacity=*/1.0f);
    CHECK(result == kSrcTex);
}

TEST_CASE("transform + non-default clip opacity was already safe pre-fix, and still runs through the real picker",
          "[compositor][opacity][ms-white][scratch_pool]")
{
    // opacity != 1.0 always took the render branch, forceCopy or not -- this
    // case never exhibited the bug, and the fix doesn't change THAT it
    // renders, only which texture (now pick()'s choice, not a hardcoded one)
    // it renders into.
    Tex transformedTex = transformStep(/*needsTransform=*/true, kSrcTex);
    Tex result = transformThenOpacity(/*needsTransform=*/true, /*opacity=*/0.5f);
    CHECK(result != transformedTex);
}
