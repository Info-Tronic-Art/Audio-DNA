#include <catch2/catch_test_macros.hpp>
#include <cmath>

// s-rta-0925 ms-white: GL FBO/texture feedback-loop hazard in
// CompositorEngine, diagnosed against the live app (production port 7070,
// no rebuild) via render_frame + pixel decoding -- see
// .harmony/.reports/s-rta-0925/ms-white-diagnosis.md for the full bisection
// (V0-V11) and the archived-artifact byte decode confirming the failing
// captures are exactly (0,0,0,0) everywhere.
//
// Root cause: CompositorEngine::applyClipTransform (CompositorEngine.cpp)
// renders a clip's scale/position/rotation transform into
// effectFBO_A_/effectTex_A_ as scratch, then hands the result to
// applyClipOpacity. applyClipOpacity's identity-opacity (1.0, the default)
// path was a TRUE no-op that returned srcTex verbatim with no GL call --
// so when a transform ran, it handed back effectTex_A_ itself.
// applyClipEffects (called next, with that texture as inputTex) ALWAYS
// targets effectFBO_A_ for its first enabled effect's write (writeFBO=0).
// The fragment shader then samples effectTex_A_ as u_texture while it is
// simultaneously bound as that same FBO's color-attachment render target --
// a read/write-same-texture feedback loop, undefined behavior per the GL
// spec, which resolved to the glClear(0,0,0,0) value surviving with
// nothing valid drawn on top (a fully transparent frame) on the
// Metal-backed macOS GL driver used to diagnose this.
//
// The fix adds a `forceCopy` parameter to applyClipOpacity: when true, the
// identity-opacity early return is suppressed and the copy always renders
// through dstFBO/dstTex instead, breaking the alias. applyClipTransform
// passes forceCopy=needsTransform, since only the just-transformed branch
// produces a srcTex that aliases applyClipEffects' first write target.
//
// Driving the real CompositorEngine::applyClipTransform/applyClipOpacity/
// applyClipEffects headlessly is infeasible: they issue real GL calls
// (glBindFramebuffer, glClear, glDrawArrays via FullscreenQuad) that
// require a live, attached OpenGL context, which test_compositor.cpp
// documents as unavailable in this unit-test harness (no target links
// CompositorEngine.cpp for exactly this reason -- see also
// test_renderer_source_confinement.cpp, test_composition_tier_oracle.cpp,
// test_layer_transport_reverse.cpp for the same, already-documented
// limitation and its established workaround). Following that precedent
// ("mirror the mechanism, not the subsystem"), this test reproduces the
// exact texture-handle decision chain across the three functions --
// represented by small integer texture "handles" standing in for the real
// GLuint values -- rather than exercising the actual GL code.

namespace
{

// Stand-ins for the real GLuint texture handles. Values are arbitrary but
// distinct, matching how CompositorEngine::initGL creates four separate
// FBO/texture pairs (accumulator, scratch, effect A, effect B).
constexpr int kSrcTex    = 100; // the clip's original (pre-transform) texture
constexpr int kEffectTexA = 101; // effectTex_A_
constexpr int kEffectTexB = 102; // effectTex_B_

// Mirrors CompositorEngine::applyClipOpacity's identity-opacity early
// return, AS IT SHIPPED before this lane's fix: a true no-op regardless of
// whether srcTex is about to be reused as a write target elsewhere.
int applyClipOpacity_PreFix(float opacity, int srcTex, int dstTex)
{
    constexpr float eps = 0.001f;
    if (std::abs(opacity - 1.0f) <= eps)
        return srcTex; // true no-op -- no GL call issued
    return dstTex; // (opacity != 1.0: renders into dstFBO/dstTex)
}

// Mirrors applyClipOpacity AS FIXED by this lane: forceCopy suppresses the
// no-op so the identity-opacity case still lands in dstTex.
int applyClipOpacity_Fixed(float opacity, int srcTex, int dstTex, bool forceCopy)
{
    constexpr float eps = 0.001f;
    if (!forceCopy && std::abs(opacity - 1.0f) <= eps)
        return srcTex; // true no-op -- no GL call issued
    return dstTex;
}

// Mirrors CompositorEngine::applyClipTransform's tail: transformedTex is
// kEffectTexA when needsTransform, else the original srcTex; the result is
// then passed through applyClipOpacity targeting effectTex_B_.
int applyClipTransform_PreFix(bool needsTransform, float clipOpacity)
{
    int transformedTex = needsTransform ? kEffectTexA : kSrcTex;
    return applyClipOpacity_PreFix(clipOpacity, transformedTex, kEffectTexB);
}

int applyClipTransform_Fixed(bool needsTransform, float clipOpacity)
{
    int transformedTex = needsTransform ? kEffectTexA : kSrcTex;
    return applyClipOpacity_Fixed(clipOpacity, transformedTex, kEffectTexB,
                                   /*forceCopy=*/needsTransform);
}

// Mirrors applyClipEffects: with at least one enabled, non-special-cased
// effect, the ping-pong loop's first pass ALWAYS targets effectFBO_A_
// (writeFBO starts at 0 -- CompositorEngine.cpp's applyClipEffects).
constexpr int kFirstEffectWriteTarget = kEffectTexA;

} // namespace

TEST_CASE("pre-fix: transform + default clip opacity aliases applyClipEffects' first write target (reproduces the bug)",
          "[compositor][opacity][ms-white]")
{
    // needsTransform=true (e.g. a non-default scale), clipOpacity at its
    // 1.0 default -- exactly the B1 fixture / V8 decisive-test shape from
    // the diagnosis.
    int clipTex = applyClipTransform_PreFix(/*needsTransform=*/true, /*clipOpacity=*/1.0f);

    // Without the fix, the identity-opacity no-op hands back effectTex_A_
    // verbatim -- the SAME texture applyClipEffects is about to sample AND
    // render into in one draw call.
    CHECK(clipTex == kEffectTexA);
    CHECK(clipTex == kFirstEffectWriteTarget); // aliasing hazard
}

TEST_CASE("fixed: transform + default clip opacity is routed through effectTex_B_, breaking the alias",
          "[compositor][opacity][ms-white]")
{
    int clipTex = applyClipTransform_Fixed(/*needsTransform=*/true, /*clipOpacity=*/1.0f);

    // forceCopy suppresses the no-op, so the result lands in effectTex_B_
    // instead of effectTex_A_.
    CHECK(clipTex == kEffectTexB);
    CHECK(clipTex != kFirstEffectWriteTarget); // no aliasing
}

TEST_CASE("fixed: no transform + default clip opacity remains a true no-op (unaffected safe path)",
          "[compositor][opacity][ms-white]")
{
    // needsTransform=false: transformedTex is the clip's original texture,
    // never effectTex_A_/effectFBO_A_ scratch -- no aliasing risk exists,
    // so forceCopy is false and the cheap no-op is preserved.
    int clipTex = applyClipTransform_Fixed(/*needsTransform=*/false, /*clipOpacity=*/1.0f);

    CHECK(clipTex == kSrcTex);
    CHECK(clipTex != kFirstEffectWriteTarget);
}

TEST_CASE("fixed: transform + non-default clip opacity was already safe pre-fix, and still renders into effectTex_B_",
          "[compositor][opacity][ms-white]")
{
    // clipOpacity != 1.0 always took the render branch (dstTex) even
    // pre-fix -- this case never exhibited the bug, and the fix doesn't
    // change its outcome.
    int preFix = applyClipTransform_PreFix(/*needsTransform=*/true, /*clipOpacity=*/0.5f);
    int fixed  = applyClipTransform_Fixed(/*needsTransform=*/true, /*clipOpacity=*/0.5f);

    CHECK(preFix == kEffectTexB);
    CHECK(fixed == kEffectTexB);
    CHECK(fixed != kFirstEffectWriteTarget);
}
