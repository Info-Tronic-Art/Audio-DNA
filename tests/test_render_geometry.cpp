#include <catch2/catch_test_macros.hpp>
#include "render/RenderGeometry.h"

// s-rta-0926b plan4 item 1: the composition canvas drives the picture's shape. RenderGeometry.h is the
// pure half of it -- which size the canvas is (resolveCanvas), where the panel presents it (fitCanvas),
// how small a Screen Split / Frame Stutter ring cell is stored (ringDownscale) and how many box-filter
// taps the present pass averages (presentTaps). Renderer (GL) only consumes these; the numbers below are
// re-derived by hand in .harmony/.reports/s-rta-0926b/plan4-final.md section 2.5.

namespace
{
bool same(const RenderGeometry::Rect& r, int x, int y, int w, int h)
{
    return r.x == x && r.y == y && r.w == w && r.h == h;
}
bool same(const RenderGeometry::Size& s, int w, int h)
{
    return s.w == w && s.h == h;
}
} // namespace

TEST_CASE("fitCanvas: a 16:9 canvas in the real 756x852 preview GL host is letterboxed, never stretched", "[render_geometry]")
{
    // The lower-left panel's GL host (panel minus the 26 px tab bar) measured 756x852 in the render lanes.
    CHECK(same(RenderGeometry::fitCanvas(1920, 1080, 756, 852), 0, 213, 756, 425));
    CHECK(same(RenderGeometry::fitCanvas(2560, 1440, 756, 852), 0, 213, 756, 425));
    CHECK(same(RenderGeometry::fitCanvas(3840, 2160, 1000, 1000), 0, 219, 1000, 562));
}

TEST_CASE("fitCanvas: an exact-aspect view is filled edge to edge", "[render_geometry]")
{
    CHECK(same(RenderGeometry::fitCanvas(1920, 1080, 1920, 1080), 0, 0, 1920, 1080));
    CHECK(same(RenderGeometry::fitCanvas(1280, 720, 3840, 2160), 0, 0, 3840, 2160));
}

TEST_CASE("fitCanvas: square and 4:3 canvases are pillarboxed or letterboxed by their own shape", "[render_geometry]")
{
    CHECK(same(RenderGeometry::fitCanvas(1000, 1000, 756, 852), 0, 48, 756, 756));
    CHECK(same(RenderGeometry::fitCanvas(1024, 768, 1920, 1080), 240, 0, 1440, 1080));   // pillarbox
    CHECK(same(RenderGeometry::fitCanvas(600, 600, 1920, 1080), 420, 0, 1080, 1080));    // the legacy-image fit
}

TEST_CASE("fitCanvas: any non-positive input gives an empty rect", "[render_geometry]")
{
    CHECK(same(RenderGeometry::fitCanvas(0, 1080, 756, 852), 0, 0, 0, 0));
    CHECK(same(RenderGeometry::fitCanvas(1920, 0, 756, 852), 0, 0, 0, 0));
    CHECK(same(RenderGeometry::fitCanvas(1920, 1080, 0, 852), 0, 0, 0, 0));
    CHECK(same(RenderGeometry::fitCanvas(1920, 1080, 756, -1), 0, 0, 0, 0));
}

TEST_CASE("resolveCanvas: the test lock wins, then the composition, then 1920x1080", "[render_geometry]")
{
    CHECK(same(RenderGeometry::resolveCanvas(0, 0, 0, 0), 1920, 1080));          // a JSON without the keys
    CHECK(same(RenderGeometry::resolveCanvas(256, 256, 3840, 2160), 256, 256));  // TestServer lock wins
    CHECK(same(RenderGeometry::resolveCanvas(0, 0, 3840, 2160), 3840, 2160));
    CHECK(same(RenderGeometry::resolveCanvas(0, 0, -5, 10), 1920, 1080));        // a half pair is ignored
    CHECK(same(RenderGeometry::resolveCanvas(100, 0, 1280, 720), 1280, 720));    // a half lock is ignored
}

TEST_CASE("ringDownscale: a ring cell is never stored wider than 480 px, never finer than 1/4", "[render_geometry]")
{
    CHECK(RenderGeometry::ringDownscale(756) == 4);
    CHECK(RenderGeometry::ringDownscale(1920) == 4);
    CHECK(RenderGeometry::ringDownscale(2560) == 6);
    CHECK(RenderGeometry::ringDownscale(3840) == 8);
    CHECK(RenderGeometry::ringDownscale(4000) == 9);
    CHECK(RenderGeometry::ringDownscale(480) == 4);
}

TEST_CASE("presentTaps: box-filter taps per axis for the panel downscale", "[render_geometry]")
{
    CHECK(RenderGeometry::presentTaps(1920, 756) == 3);
    CHECK(RenderGeometry::presentTaps(3840, 756) == 6);
    CHECK(RenderGeometry::presentTaps(1920, 1920) == 1);
    CHECK(RenderGeometry::presentTaps(256, 756) == 1);     // an upscale is plain bilinear
    CHECK(RenderGeometry::presentTaps(7680, 100) == 8);    // clamped
    CHECK(RenderGeometry::presentTaps(1920, 0) == 1);      // no panel: no divide by zero
}
