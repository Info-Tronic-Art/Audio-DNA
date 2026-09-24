#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "connect/ScalarParams.h"

using Catch::Approx;

// Units fix for the comp-transform black-screen bug:
// Renderer::applyCompTransform (src/render/Renderer.cpp) feeds
// composition_->eff(CompScalar::PosX/PosY/AnchorX/AnchorY) -- pixel offsets
// in ScalarMath::posX3840/posY2160 space (a 3840x2160 reference canvas,
// range approx +/-1920 / +/-1080) -- into the comp_transform shader's
// u_comp_position/u_comp_anchor uniforms. The shader (EmbeddedShaders.h,
// compTransform) applies those uniforms as `uv -= u_comp_position * 0.5`,
// i.e. a uniform value of 1.0 shifts the sampled UV by 0.5 (half the
// canvas, in normalized [0,1] UV space); the shader samples black outside
// [0,1] UV. Before this fix, the raw pixel value was passed straight to
// glUniform2f (an implicit identity "conversion"), so ANY non-zero
// composition position pushed the sample far outside [0,1] and rendered
// solid black. These pin the correct pixel -> uniform conversion:
// ScalarMath::posPxToCompUniformX/Y (ScalarParams.h), used for both
// PosX/PosY and AnchorX/AnchorY (the shader applies the same `* 0.5`
// convention to u_comp_anchor).

TEST_CASE("posPxToCompUniformX/Y: zero pixel offset is zero", "[comp_transform_units]")
{
    REQUIRE(ScalarMath::posPxToCompUniformX(0.0f) == Approx(0.0f));
    REQUIRE(ScalarMath::posPxToCompUniformY(0.0f) == Approx(0.0f));
}

TEST_CASE("posPxToCompUniformX: +1920px (half the 3840-wide reference canvas) maps to a uniform of 1.0", "[comp_transform_units]")
{
    // Matches the shader's own convention: uv -= u_comp_position * 0.5, so a
    // uniform of 1.0 shifts the sample by 0.5 (half the canvas) in
    // normalized UV -- exactly what a 1920px (half of 3840) offset should
    // produce.
    REQUIRE(ScalarMath::posPxToCompUniformX(1920.0f) == Approx(1.0f));
}

TEST_CASE("posPxToCompUniformY: +1080px (half the 2160-tall reference canvas) maps to a uniform of 1.0", "[comp_transform_units]")
{
    REQUIRE(ScalarMath::posPxToCompUniformY(1080.0f) == Approx(1.0f));
}

TEST_CASE("posPxToCompUniformX/Y: sign is preserved (round trip through pixel space)", "[comp_transform_units]")
{
    REQUIRE(ScalarMath::posPxToCompUniformX(-1920.0f) == Approx(-1.0f));
    REQUIRE(ScalarMath::posPxToCompUniformY(-1080.0f) == Approx(-1.0f));

    for (float px : { -960.0f, -1.0f, 200.0f, 960.0f })
    {
        REQUIRE(ScalarMath::posPxToCompUniformX(px) == Approx(px / 1920.0f));
        // Same reasoning applies to the Y axis at its own half-canvas scale.
        REQUIRE(ScalarMath::posPxToCompUniformY(px) == Approx(px / 1080.0f));
    }
}

TEST_CASE("posPxToCompUniformX: demonstrates why the pre-fix identity 'conversion' was broken", "[comp_transform_units]")
{
    // Before this fix, Renderer::applyCompTransform passed the raw pixel
    // value straight to glUniform2f -- an implicit identity conversion. Even
    // a modest 200px nudge, fed raw, is already outside the shader's
    // expected [-1, 1] uniform range (u_comp_position documented as
    // "Normalized offset (-1 to 1)"), while the fixed conversion keeps it
    // inside range.
    float rawPixelOffset = 200.0f;
    float oldIdentityConversion = rawPixelOffset;              // pre-fix behavior
    float newConversion = ScalarMath::posPxToCompUniformX(rawPixelOffset);

    REQUIRE(oldIdentityConversion > 1.0f);                     // pre-fix: already out of range -> black screen
    REQUIRE(newConversion < 1.0f);                             // post-fix: still in range
    REQUIRE(newConversion == Approx(200.0f / 1920.0f));
}
