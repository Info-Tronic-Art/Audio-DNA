#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/ClipFit.h"
#include "render/RenderGeometry.h"
#include <cstdlib>

// s-rta-0926b plan-fitmode: a clip whose picture is not the composition's shape is shown Stretch (default,
// today's output), Bars (own shape, centred, transparent bars) or Crop (own shape, covers, overflow cut).
// ClipFit.h is the pure half; CompositorEngine::applyClipTransform feeds scale() to the layer_transform
// shader. Every number below is re-derived in .harmony/.reports/s-rta-0926b/plan-fitmode.md section 2.8.
// The fixture shape 756x878 is media/P16_01_baseline.png (the live probe's picture); 1920x1080 = the
// default composition canvas.

using Catch::Matchers::WithinAbs;

namespace
{
bool same(const ClipFit::Rect& r, int x, int y, int w, int h)
{
    return r.x == x && r.y == y && r.w == w && r.h == h;
}
bool identity(const ClipFit::Scale& s)
{
    return s.x == 1.0f && s.y == 1.0f;   // exact, not approximate: the renderer skips the pass on {1,1}
}
} // namespace

TEST_CASE("scale: Stretch is the exact identity whatever the shapes", "[clip_fit]")
{
    CHECK(identity(ClipFit::scale(ClipFit::Mode::Stretch, 756, 878, 1920, 1080)));
    CHECK(identity(ClipFit::scale(ClipFit::Mode::Stretch, 1920, 1080, 1080, 1080)));
    CHECK(identity(ClipFit::scale(ClipFit::Mode::Stretch, 1, 4000, 4000, 1)));
}

TEST_CASE("scale: a portrait picture on a 16:9 canvas -- Bars narrows x, Crop narrows y", "[clip_fit]")
{
    const auto bars = ClipFit::scale(ClipFit::Mode::Bars, 756, 878, 1920, 1080);
    CHECK_THAT(bars.x, WithinAbs(1685760.0 / 816480.0, 1e-5));   // 2.0647
    CHECK(bars.y == 1.0f);
    const auto crop = ClipFit::scale(ClipFit::Mode::Crop, 756, 878, 1920, 1080);
    CHECK(crop.x == 1.0f);
    CHECK_THAT(crop.y, WithinAbs(816480.0 / 1685760.0, 1e-5));   // 0.48434
}

TEST_CASE("scale: a wide picture on a square canvas -- Bars narrows y, Crop narrows x", "[clip_fit]")
{
    const auto bars = ClipFit::scale(ClipFit::Mode::Bars, 1920, 1080, 1080, 1080);
    CHECK(bars.x == 1.0f);
    CHECK_THAT(bars.y, WithinAbs(1920.0 / 1080.0, 1e-5));        // 1.7778
    const auto crop = ClipFit::scale(ClipFit::Mode::Crop, 1920, 1080, 1080, 1080);
    CHECK_THAT(crop.x, WithinAbs(0.5625, 1e-6));
    CHECK(crop.y == 1.0f);
}

TEST_CASE("scale: the same aspect is the exact identity for Bars and Crop (no pass runs)", "[clip_fit]")
{
    for (auto m : { ClipFit::Mode::Bars, ClipFit::Mode::Crop })
    {
        CHECK(identity(ClipFit::scale(m, 1920, 1080, 1920, 1080)));
        CHECK(identity(ClipFit::scale(m, 3840, 2160, 1920, 1080)));
        CHECK(identity(ClipFit::scale(m, 1280, 720, 3840, 2160)));
    }
}

TEST_CASE("scale: any size <= 0 (an unreadable texture) is the exact identity", "[clip_fit]")
{
    for (auto m : { ClipFit::Mode::Bars, ClipFit::Mode::Crop })
    {
        CHECK(identity(ClipFit::scale(m, 0, 878, 1920, 1080)));
        CHECK(identity(ClipFit::scale(m, 756, 0, 1920, 1080)));
        CHECK(identity(ClipFit::scale(m, 756, 878, 0, 1080)));
        CHECK(identity(ClipFit::scale(m, 756, 878, 1920, -1)));
    }
}

TEST_CASE("barsRect: the picture's own shape, centred -- agrees with plan4's fitCanvas", "[clip_fit]")
{
    CHECK(same(ClipFit::barsRect(756, 878, 1920, 1080), 495, 0, 929, 1080));
    CHECK(same(ClipFit::barsRect(600, 600, 1920, 1080), 420, 0, 1080, 1080));
    CHECK(same(ClipFit::barsRect(1024, 768, 1920, 1080), 240, 0, 1440, 1080));
    CHECK(same(ClipFit::barsRect(1920, 1080, 1080, 1080), 0, 236, 1080, 607));   // letterbox
    CHECK(same(ClipFit::barsRect(1920, 1080, 1920, 1080), 0, 0, 1920, 1080));
    CHECK(same(ClipFit::barsRect(0, 1080, 1920, 1080), 0, 0, 0, 0));

    // The two headers are one geometry: same integer math on every shared vector.
    const int vecs[][4] = { { 756, 878, 1920, 1080 }, { 600, 600, 1920, 1080 }, { 1024, 768, 1920, 1080 },
                            { 1920, 1080, 1080, 1080 }, { 1920, 1080, 1920, 1080 }, { 1080, 1920, 1920, 1080 } };
    for (const auto& v : vecs)
    {
        const auto a = ClipFit::barsRect(v[0], v[1], v[2], v[3]);
        const auto b = RenderGeometry::fitCanvas(v[0], v[1], v[2], v[3]);
        CHECK(same(a, b.x, b.y, b.w, b.h));
    }
}

TEST_CASE("cropRect: the part of the picture that stays visible, centred", "[clip_fit]")
{
    CHECK(same(ClipFit::cropRect(756, 878, 1920, 1080), 0, 226, 756, 425));
    CHECK(same(ClipFit::cropRect(1920, 1080, 1080, 1080), 420, 0, 1080, 1080));
    CHECK(same(ClipFit::cropRect(1920, 1080, 1920, 1080), 0, 0, 1920, 1080));
    CHECK(same(ClipFit::cropRect(756, 878, 0, 1080), 0, 0, 0, 0));
}

TEST_CASE("scale and barsRect describe the same picture (the shader's width = the rect's width)", "[clip_fit]")
{
    // Bars stretches the UV by s: the picture covers canvas/s of the canvas. canvas/s must be the rect's
    // width (portrait) or height (landscape) within 2 px of integer truncation.
    const int portrait[][4] = { { 756, 878, 1920, 1080 }, { 600, 600, 1920, 1080 }, { 1024, 768, 1920, 1080 } };
    for (const auto& v : portrait)
    {
        const auto s = ClipFit::scale(ClipFit::Mode::Bars, v[0], v[1], v[2], v[3]);
        const auto r = ClipFit::barsRect(v[0], v[1], v[2], v[3]);
        CHECK(std::abs(r.w * s.x - static_cast<float>(v[2])) <= 2.0f);
        CHECK(s.y == 1.0f);
    }
    const auto s = ClipFit::scale(ClipFit::Mode::Bars, 1920, 1080, 1080, 1080);
    const auto r = ClipFit::barsRect(1920, 1080, 1080, 1080);
    CHECK(std::abs(r.h * s.y - 1080.0f) <= 2.0f);
}

TEST_CASE("clampMode: 0..2 map to the modes, anything else is Stretch", "[clip_fit]")
{
    CHECK(ClipFit::clampMode(0) == ClipFit::Mode::Stretch);
    CHECK(ClipFit::clampMode(1) == ClipFit::Mode::Bars);
    CHECK(ClipFit::clampMode(2) == ClipFit::Mode::Crop);
    CHECK(ClipFit::clampMode(-1) == ClipFit::Mode::Stretch);
    CHECK(ClipFit::clampMode(3) == ClipFit::Mode::Stretch);
    CHECK(ClipFit::clampMode(255) == ClipFit::Mode::Stretch);
}
