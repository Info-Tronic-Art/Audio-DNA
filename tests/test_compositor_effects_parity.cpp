#include <catch2/catch_test_macros.hpp>

// s-rta-0925 ms-white2: a SECOND, separate instance of the same GL
// feedback-loop aliasing class fixed by 52cd76c (see
// tests/test_compositor_opacity_alias.cpp for that fix) -- found while
// checking "do other call orders hit the same aliasing?" per
// .harmony/.reports/s-rta-0925/ms-white-diagnosis-2.md section 7/8.
//
// Root cause: CompositorEngine::applyClipEffects (CompositorEngine.cpp)
// hardcodes `int writeFBO = 0;` -- its ping-pong ALWAYS starts its first
// enabled effect's write at effectFBO_A_/effectTex_A_, regardless of what
// texture its own inputTex already is. compositeDeck() calls
// applyClipEffects TWICE per Opaque/Transparent layer, back to back with no
// intervening copy: once for the clip's per-clip effects
// (CompositorEngine.cpp ~line 864), then -- if the layer has any enabled
// layer effects -- again for the per-layer effects, with the FIRST call's
// output texture as the SECOND call's inputTex (~line 881).
//
// If the clip has an ODD number of enabled per-clip effects, the first
// call's own ping-pong exits with its output == effectTex_A_ (1 effect:
// writeFBO flips 0->1, so the sole write lands at index 0 = effectTex_A_;
// 3 effects: same parity). The second call then starts its OWN ping-pong
// at effectFBO_A_ again (hardcoded 0) -- so its first enabled layer effect
// samples effectTex_A_ as u_texture while simultaneously rendering into
// effectFBO_A_ in the same draw call: the identical read/write-same-texture
// GL feedback-loop hazard as 52cd76c's mechanism, just at a different
// call-site pairing. Confirmed live: 1 per-clip effect (Hue Shift) + 1
// layer effect (Saturation), no transform/feedback/crossfade -> fully
// blank (0,0,0,0) capture. Control: 2 per-clip effects (even) + same layer
// effect -> renders correctly -- isolates ping-pong PARITY, not effect
// identity, as the variable (diagnosis-2 section 8, v12/v13 fixtures).
//
// The fix (CompositorEngine.cpp applyClipEffects, near its
// `int writeFBO = 0;` initialization) has applyClipEffects decide its OWN
// starting write target from the identity of inputTex:
//     int writeFBO = (inputTex == effectTex_A_) ? 1 : 0;
// This protects EVERY current and future caller uniformly (the per-layer
// call site here, and any later addition), rather than threading a new
// forceCopy-style bool through each call site individually the way
// 52cd76c did for applyClipTransform's hand-off -- a materially different,
// better-generalizing shape of fix for a data-dependent (effect-count-
// parity) hazard, per diagnosis-2 section 8.
//
// Driving the real CompositorEngine::applyClipEffects headlessly is
// infeasible: it issues real GL calls (glBindFramebuffer, glClear,
// glDrawArrays via FullscreenQuad) that require a live, attached OpenGL
// context -- test_compositor.cpp documents this as unavailable in this
// unit-test harness (no target links CompositorEngine.cpp for exactly this
// reason -- see also test_compositor_opacity_alias.cpp,
// test_renderer_source_confinement.cpp, test_composition_tier_oracle.cpp,
// test_layer_transport_reverse.cpp for the same, already-documented
// limitation and its established workaround). Following that precedent
// ("mirror the mechanism, not the subsystem"), this test reproduces the
// exact texture-handle ping-pong decision chain across TWO chained
// applyClipEffects calls -- represented by small integer texture "handles"
// standing in for the real GLuint values -- rather than exercising the
// actual GL code.

namespace
{

// Stand-ins for the real GLuint texture handles, matching
// test_compositor_opacity_alias.cpp's convention.
constexpr int kSrcTex     = 100; // a clip's persistent texture (image/video/source) -- never effectTex_A_
constexpr int kEffectTexA = 101; // effectTex_A_
constexpr int kEffectTexB = 102; // effectTex_B_

// Mirrors applyClipEffects' ping-pong loop for N >= 1 enabled,
// non-special-cased effects at dryWet==1.0 (the common/default case; the
// dry/wet blend branch and the screen_split/frame_delay special cases
// don't change this ping-pong's parity math and are out of scope here).
// Returns the FBO index (0=effectFBO_A_/effectTex_A_, 1=effectFBO_B_/
// effectTex_B_) that ends up holding the chain's final output, given the
// STARTING writeFBO decision and the effect count.
//
// Per-iteration: targetFBO = writeFBO; render; currentInput = that FBO's
// texture; writeFBO = 1 - writeFBO. After N iterations starting at W0, the
// final output's FBO index is (W0 + N - 1) mod 2.
int pingPongFinalIndex(int startWriteFBO, int numEnabledEffects)
{
    return (startWriteFBO + numEnabledEffects - 1) % 2;
}

int indexToTex(int fboIndex) { return fboIndex == 0 ? kEffectTexA : kEffectTexB; }

// Mirrors applyClipEffects AS IT SHIPPED before this lane's fix: the
// ping-pong always starts at effectFBO_A_ (index 0), regardless of inputTex.
int applyClipEffects_PreFix(int inputTex, int numEnabledEffects)
{
    if (numEnabledEffects == 0)
        return inputTex; // effects.empty() early return -- unchanged either way
    constexpr int startWriteFBO = 0; // hardcoded, ignores inputTex
    return indexToTex(pingPongFinalIndex(startWriteFBO, numEnabledEffects));
}

// Mirrors applyClipEffects AS FIXED by this lane: the ping-pong starts at
// whichever FBO inputTex does NOT already alias.
int applyClipEffects_Fixed(int inputTex, int numEnabledEffects)
{
    if (numEnabledEffects == 0)
        return inputTex;
    int startWriteFBO = (inputTex == kEffectTexA) ? 1 : 0;
    return indexToTex(pingPongFinalIndex(startWriteFBO, numEnabledEffects));
}

// The hazard itself: a second applyClipEffects call whose inputTex equals
// the FIRST write target its OWN ping-pong is about to target (index 0,
// effectTex_A_) is a read/write-same-texture aliasing hazard on its first
// enabled effect's draw call.
bool secondCallAliasesPreFix(int secondCallInputTex)
{
    constexpr int secondCallStartWriteFBO = 0; // hardcoded pre-fix
    return secondCallInputTex == indexToTex(secondCallStartWriteFBO);
}

bool secondCallAliasesFixed(int secondCallInputTex)
{
    int secondCallStartWriteFBO = (secondCallInputTex == kEffectTexA) ? 1 : 0;
    return secondCallInputTex == indexToTex(secondCallStartWriteFBO);
}

} // namespace

TEST_CASE("pre-fix: odd per-clip effect count + a layer effect aliases the second call's first write target (reproduces the bug)",
          "[compositor][effects][ms-white2]")
{
    // 1 enabled per-clip effect (Hue Shift), no transform, clip's own
    // persistent texture as the very first inputTex -- exactly the
    // v12_perclip_odd_plus_layereffect fixture from diagnosis-2.
    int clipTexAfterPerClip = applyClipEffects_PreFix(kSrcTex, /*numEnabledEffects=*/1);
    CHECK(clipTexAfterPerClip == kEffectTexA); // odd count -> lands on effectTex_A_

    // Layer has >= 1 enabled layer effect -- compositeDeck calls
    // applyClipEffects a second time, with the first call's output as input.
    CHECK(secondCallAliasesPreFix(clipTexAfterPerClip)); // aliasing hazard: blank frame
}

TEST_CASE("fixed: odd per-clip effect count + a layer effect no longer aliases (bug is broken)",
          "[compositor][effects][ms-white2]")
{
    int clipTexAfterPerClip = applyClipEffects_Fixed(kSrcTex, /*numEnabledEffects=*/1);
    CHECK(clipTexAfterPerClip == kEffectTexA); // first call's own output is unaffected (inputTex was kSrcTex, never aliased)

    // Second call now sees inputTex == effectTex_A_ and starts its OWN
    // ping-pong at effectFBO_B_ instead -- no alias.
    CHECK_FALSE(secondCallAliasesFixed(clipTexAfterPerClip));
}

TEST_CASE("fixed: even per-clip effect count + a layer effect was already safe pre-fix, and stays safe (unaffected control)",
          "[compositor][effects][ms-white2]")
{
    // 2 enabled per-clip effects (even) -- diagnosis-2's v13 control fixture.
    int preFixOutput = applyClipEffects_PreFix(kSrcTex, /*numEnabledEffects=*/2);
    int fixedOutput  = applyClipEffects_Fixed(kSrcTex, /*numEnabledEffects=*/2);

    CHECK(preFixOutput == kEffectTexB); // even count -> lands on effectTex_B_, not the FBO the second call starts at
    CHECK(fixedOutput == kEffectTexB);  // same result -- inputTex (kSrcTex) never equals effectTex_A_, so the fix is a no-op here

    CHECK_FALSE(secondCallAliasesPreFix(preFixOutput)); // already safe pre-fix
    CHECK_FALSE(secondCallAliasesFixed(fixedOutput));   // still safe post-fix -- no regression on the safe control
}

TEST_CASE("fixed: zero per-clip effects + a layer effect was already safe pre-fix (inputTex is never effectTex_A_), and stays safe",
          "[compositor][effects][ms-white2]")
{
    // No per-clip effects at all: applyClipEffects' effects.empty() early
    // return hands the clip's own persistent texture straight through --
    // never effectTex_A_/B_, so there is no aliasing precondition in this
    // case at all, fixed or not.
    int preFixOutput = applyClipEffects_PreFix(kSrcTex, /*numEnabledEffects=*/0);
    int fixedOutput  = applyClipEffects_Fixed(kSrcTex, /*numEnabledEffects=*/0);

    CHECK(preFixOutput == kSrcTex);
    CHECK(fixedOutput == kSrcTex);

    CHECK_FALSE(secondCallAliasesPreFix(preFixOutput));
    CHECK_FALSE(secondCallAliasesFixed(fixedOutput));
}
