// test_pixel_convert -- s-rta-0927 plan-renderperf C2: a render_frame / snapshot capture converts the canvas's
// glReadPixels rows (RGBA8, bottom-up) into a juce::Image with PixelConvert::rgbaBottomUpToARGB instead of the old
// per-pixel bmp.setPixelColour(x, y, juce::Colour(r, g, b, a)) loop. The capture must not change by one byte: every
// case below runs the OLD loop (copied verbatim from Renderer.cpp's processPendingCapture at 1636785; the forceOpaque
// variant from TestServer.cpp's handleOutputProbe) and the new function on the same buffer and compares the image
// bytes, row by row, on both JUCE image backings -- and, in the last case, the PNG bytes the capture writes.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/PixelConvert.h"
#include <cstring>
#include <vector>

namespace
{
// The OLD loop, verbatim (Renderer.cpp processPendingCapture; forceOpaque = TestServer.cpp handleOutputProbe).
void referenceLoop(const std::vector<uint8_t>& pixels, int readW, int readH, juce::Image::BitmapData& bmp,
                   bool forceOpaque)
{
    for (int y = 0; y < readH; ++y)
    {
        const auto* srcRow = pixels.data() + static_cast<size_t>(readH - 1 - y) * static_cast<size_t>(readW) * 4;
        for (int x = 0; x < readW; ++x)
        {
            if (forceOpaque)
                bmp.setPixelColour(x, y, juce::Colour(srcRow[x * 4], srcRow[x * 4 + 1], srcRow[x * 4 + 2],
                                                      static_cast<uint8_t>(255)));
            else
                bmp.setPixelColour(x, y,
                    juce::Colour(srcRow[x * 4],     // R
                                 srcRow[x * 4 + 1], // G
                                 srcRow[x * 4 + 2], // B
                                 srcRow[x * 4 + 3]  // A
                    ));
        }
    }
}

juce::Image makeImage(int w, int h, bool software)
{
    return software ? juce::Image(juce::Image::ARGB, w, h, false, juce::SoftwareImageType())
                    : juce::Image(juce::Image::ARGB, w, h, false);   // the capture's ctor: NativeImageType
}

juce::Image oracle(const std::vector<uint8_t>& px, int w, int h, bool software, bool forceOpaque)
{
    auto img = makeImage(w, h, software);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
    referenceLoop(px, w, h, bmp, forceOpaque);
    return img;
}

juce::Image candidate(const std::vector<uint8_t>& px, int w, int h, bool software, bool forceOpaque)
{
    auto img = makeImage(w, h, software);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
    PixelConvert::rgbaBottomUpToARGB(px.data(), w, h, bmp, forceOpaque);
    return img;
}

// Equal strides, then every row's w*4 image bytes equal. Returns the number of differing rows.
int differingRows(const juce::Image& a, const juce::Image& b)
{
    juce::Image::BitmapData da(a, juce::Image::BitmapData::readOnly);
    juce::Image::BitmapData db(b, juce::Image::BitmapData::readOnly);
    REQUIRE(da.width == db.width);
    REQUIRE(da.height == db.height);
    REQUIRE(da.pixelStride == db.pixelStride);
    REQUIRE(da.lineStride == db.lineStride);
    int bad = 0;
    for (int y = 0; y < da.height; ++y)
        if (std::memcmp(da.getLinePointer(y), db.getLinePointer(y), static_cast<size_t>(da.width) * 4) != 0)
            ++bad;
    return bad;
}

// A 256x256 buffer with A = x, R = y, G = (7x + 3y) & 255, B = x ^ y: every alpha (0, 255 and all 254 premultiply
// values) against 256 red levels.
std::vector<uint8_t> everyAlpha(int& w, int& h)
{
    w = 256; h = 256;
    std::vector<uint8_t> px(static_cast<size_t>(w * h * 4));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            auto* p = px.data() + (static_cast<size_t>(y) * static_cast<size_t>(w) + static_cast<size_t>(x)) * 4;
            p[0] = static_cast<uint8_t>(y);
            p[1] = static_cast<uint8_t>((7 * x + 3 * y) & 255);
            p[2] = static_cast<uint8_t>(x ^ y);
            p[3] = static_cast<uint8_t>(x);
        }
    return px;
}

std::vector<uint8_t> lcg(int w, int h, uint32_t seed)
{
    std::vector<uint8_t> px(static_cast<size_t>(w * h * 4));
    for (auto& b : px)
    {
        seed = seed * 1664525u + 1013904223u;
        b = static_cast<uint8_t>(seed >> 24);
    }
    return px;
}
} // namespace

TEST_CASE("every alpha, every backing: the row conversion writes the setPixelColour loop's bytes",
          "[pixel_convert][s-rta-0927]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    int w = 0, h = 0;
    const auto px = everyAlpha(w, h);
    for (bool software : { false, true })
    {
        INFO((software ? "SoftwareImageType" : "NativeImageType"));
        CHECK(differingRows(oracle(px, w, h, software, false), candidate(px, w, h, software, false)) == 0);
    }
}

TEST_CASE("forceOpaque matches the output_probe loop", "[pixel_convert][s-rta-0927]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    int w = 0, h = 0;
    const auto px = everyAlpha(w, h);
    for (bool software : { false, true })
    {
        INFO((software ? "SoftwareImageType" : "NativeImageType"));
        CHECK(differingRows(oracle(px, w, h, software, true), candidate(px, w, h, software, true)) == 0);
    }
}

TEST_CASE("odd sizes: 131x77, 1x1, 1x9, 9x1", "[pixel_convert][s-rta-0927]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const int sizes[][2] = { { 131, 77 }, { 1, 1 }, { 1, 9 }, { 9, 1 } };
    uint32_t seed = 12345u;
    for (const auto& s : sizes)
    {
        const auto px = lcg(s[0], s[1], seed++);
        for (bool software : { false, true })
            for (bool opaque : { false, true })
            {
                INFO(s[0] << "x" << s[1] << (software ? " software" : " native") << (opaque ? " forceOpaque" : ""));
                CHECK(differingRows(oracle(px, s[0], s[1], software, opaque),
                                    candidate(px, s[0], s[1], software, opaque)) == 0);
            }
    }
}

TEST_CASE("PNG bytes identical", "[pixel_convert][s-rta-0927]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    int w = 0, h = 0;
    const auto px = everyAlpha(w, h);
    juce::MemoryOutputStream a, b;
    juce::PNGImageFormat png;
    REQUIRE(png.writeImageToStream(oracle(px, w, h, false, false), a));
    REQUIRE(png.writeImageToStream(candidate(px, w, h, false, false), b));
    INFO("oracle " << (int) a.getDataSize() << " B, candidate " << (int) b.getDataSize() << " B");
    REQUIRE(a.getDataSize() > 0);
    REQUIRE(a.getDataSize() == b.getDataSize());
    CHECK(std::memcmp(a.getData(), b.getData(), a.getDataSize()) == 0);
}
