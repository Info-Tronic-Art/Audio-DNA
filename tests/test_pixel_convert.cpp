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

// ---- s-rta-0928 renderleft R1.1: the reverse direction, image -> GL rows (PixelConvert::argbToGlRgbaBottomUp) ----
// A decoded image's pixels become GL RGBA8 rows (bottom-up) with the row function instead of the three old loops. The
// uploaded bytes must not change by one byte: each case runs the OLD loop, copied verbatim, as the oracle.
namespace
{
// The OLD loop of CompositorEngine::loadKeyImage (6db8d67), verbatim: getPixelColour -> straight RGBA, top-down, then
// the flip copy. ImageSequence::loadImageToTexture writes the same bytes (the same getPixelColour loop, flipped as it
// writes).
std::vector<uint8_t> oldLoadKeyImageLoop(const juce::Image& img)
{
    int w = img.getWidth();
    int h = img.getHeight();
    std::vector<uint8_t> rgba(static_cast<size_t>(w * h * 4));

    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            auto pixel = bmp.getPixelColour(x, y);
            size_t idx = static_cast<size_t>((y * w + x) * 4);
            rgba[idx + 0] = pixel.getRed();
            rgba[idx + 1] = pixel.getGreen();
            rgba[idx + 2] = pixel.getBlue();
            rgba[idx + 3] = pixel.getAlpha();
        }
    }

    // Flip Y for OpenGL (bottom-up)
    std::vector<uint8_t> flipped(rgba.size());
    size_t rowBytes = static_cast<size_t>(w * 4);
    for (int y = 0; y < h; ++y)
        std::memcpy(flipped.data() + static_cast<size_t>(y) * rowBytes,
                     rgba.data() + static_cast<size_t>((h - 1 - y)) * rowBytes,
                     rowBytes);
    return flipped;
}

// The OLD loop of TextureManager::uploadImage (6db8d67), verbatim: the raw premultiplied bytes swizzled B,G,R,A ->
// R,G,B,A, flipped.
std::vector<uint8_t> oldUploadImageLoop(const juce::Image& image)
{
    int w = image.getWidth();
    int h = image.getHeight();
    auto argbImage = image.convertedToFormat(juce::Image::ARGB);
    juce::Image::BitmapData bitmapData(argbImage, juce::Image::BitmapData::readOnly);

    std::vector<uint8_t> rgbaPixels(static_cast<size_t>(w * h * 4));

    for (int y = 0; y < h; ++y)
    {
        // Flip Y: OpenGL texture origin is bottom-left, image is top-left
        auto* srcRow = bitmapData.getLinePointer(h - 1 - y);
        auto* dstRow = &rgbaPixels[static_cast<size_t>(y * w * 4)];

        for (int x = 0; x < w; ++x)
        {
            auto* srcPixel = srcRow + x * 4;
            auto* dstPixel = dstRow + x * 4;
            dstPixel[0] = srcPixel[2]; // R
            dstPixel[1] = srcPixel[1]; // G
            dstPixel[2] = srcPixel[0]; // B
            dstPixel[3] = srcPixel[3]; // A
        }
    }
    return rgbaPixels;
}

// An image holding `px` (straight RGBA, top-down) as JUCE stores it (premultiplied), written with setPixelColour.
juce::Image imageOf(const std::vector<uint8_t>& px, int w, int h, bool software)
{
    auto img = makeImage(w, h, software);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            const auto* p = px.data() + (static_cast<size_t>(y) * static_cast<size_t>(w) + static_cast<size_t>(x)) * 4;
            bmp.setPixelColour(x, y, juce::Colour(p[0], p[1], p[2], p[3]));
        }
    return img;
}

std::vector<uint8_t> newRows(const juce::Image& img, bool unpremultiply)
{
    std::vector<uint8_t> out(static_cast<size_t>(img.getWidth()) * static_cast<size_t>(img.getHeight()) * 4, 0xAB);
    const juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
    PixelConvert::argbToGlRgbaBottomUp(bmp, out.data(), unpremultiply);
    return out;
}
} // namespace

TEST_CASE("straight == loadKeyImage's getPixelColour loop + flip (and ImageSequence's)", "[pixel_convert][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    int w = 0, h = 0;
    const auto px = everyAlpha(w, h);
    for (bool software : { false, true })
    {
        INFO((software ? "SoftwareImageType" : "NativeImageType"));
        const auto img = imageOf(px, w, h, software);
        CHECK(newRows(img, true) == oldLoadKeyImageLoop(img));
    }
}

TEST_CASE("premultiplied == TextureManager::uploadImage's swizzle", "[pixel_convert][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    int w = 0, h = 0;
    const auto px = everyAlpha(w, h);
    for (bool software : { false, true })
    {
        INFO((software ? "SoftwareImageType" : "NativeImageType"));
        const auto img = imageOf(px, w, h, software);
        CHECK(newRows(img, false) == oldUploadImageLoop(img));
    }
}

TEST_CASE("image -> GL rows, odd sizes 131x77, 1x1, 1x9, 9x1, both layouts", "[pixel_convert][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const int sizes[][2] = { { 131, 77 }, { 1, 1 }, { 1, 9 }, { 9, 1 } };
    uint32_t seed = 928u;
    for (const auto& s : sizes)
    {
        const auto px = lcg(s[0], s[1], seed++);
        for (bool software : { false, true })
        {
            INFO(s[0] << "x" << s[1] << (software ? " software" : " native"));
            const auto img = imageOf(px, s[0], s[1], software);
            CHECK(newRows(img, true) == oldLoadKeyImageLoop(img));
            CHECK(newRows(img, false) == oldUploadImageLoop(img));
        }
    }
}
