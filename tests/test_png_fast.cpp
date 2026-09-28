// test_png_fast -- s-rta-0928 renderleft R3: render_frame's fast PNG writer (src/render/PngWrite.h
// writeScanlinesReplacing + src/render/PixelConvert.h rgbaBottomUpToPngScanlines). The decoded picture must equal what
// the capture's JUCE writer produced (its row loop, copied verbatim from juce_PNGLoader.cpp writeImageToStream, is the
// oracle); only the file bytes may differ. [.bench] is hidden (run by name with -s): the ruling's evidence.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/PixelConvert.h"
#include "render/PngWrite.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

namespace
{
std::vector<uint8_t> everyAlphaGl(int& w, int& h)
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
    std::vector<uint8_t> px(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    for (auto& b : px)
    {
        seed = seed * 1664525u + 1013904223u;
        b = static_cast<uint8_t>(seed >> 24);
    }
    return px;
}

// The capture's image (Renderer::captureFrame, Archive): the GL rows through rgbaBottomUpToARGB.
juce::Image captureImage(const std::vector<uint8_t>& gl, int w, int h)
{
    juce::Image img(juce::Image::ARGB, w, h, false);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
    PixelConvert::rgbaBottomUpToARGB(gl.data(), w, h, bmp, false);
    return img;
}

// JUCE 8.0.4 juce_PNGLoader.cpp PNGImageFormat::writeImageToStream, the hasAlphaChannel() row loop, verbatim (with the
// filter byte PNG puts before each row), applied to the capture's image.
std::vector<uint8_t> juceWriterRows(const juce::Image& image)
{
    using namespace juce;
    auto width = image.getWidth();
    auto height = image.getHeight();
    std::vector<uint8_t> out;
    HeapBlock<uint8> rowData (width * 4);
    const Image::BitmapData srcData (image, Image::BitmapData::readOnly);
    for (int y = 0; y < height; ++y)
    {
        uint8* dst = rowData;
        const uint8* src = srcData.getLinePointer (y);
        for (int i = width; --i >= 0;)
        {
            PixelARGB p (*(const PixelARGB*) src);
            p.unpremultiply();

            *dst++ = p.getRed();
            *dst++ = p.getGreen();
            *dst++ = p.getBlue();
            *dst++ = p.getAlpha();
            src += srcData.pixelStride;
        }
        out.push_back(0);
        out.insert(out.end(), rowData.get(), rowData.get() + width * 4);
    }
    return out;
}

std::vector<uint8_t> scanlines(const std::vector<uint8_t>& gl, int w, int h)
{
    std::vector<uint8_t> s(static_cast<size_t>(h) * (1 + static_cast<size_t>(w) * 4), 0xCD);
    PixelConvert::rgbaBottomUpToPngScanlines(gl.data(), w, h, s.data());
    return s;
}

juce::File tmpFile(const char* name)
{
    return juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("test_png_fast_" + juce::Uuid().toString() + "_" + name);   // unique: ctest runs cases in parallel
}

uint32_t be32(const uint8_t* p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3]; }

// A bitwise CRC-32 (the independent oracle for the table CRC in PngWrite).
uint32_t crcBitwise(const uint8_t* d, size_t n)
{
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; ++i)
    {
        c ^= d[i];
        for (int k = 0; k < 8; ++k)
            c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return c ^ 0xFFFFFFFFu;
}
} // namespace

TEST_CASE("(1) the scanlines are the JUCE writer's row bytes (every alpha, odd sizes)", "[png_fast][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    int w = 0, h = 0;
    const auto px = everyAlphaGl(w, h);
    CHECK(scanlines(px, w, h) == juceWriterRows(captureImage(px, w, h)));
    const int sizes[][2] = { { 131, 77 }, { 1, 1 }, { 1, 9 }, { 9, 1 } };
    uint32_t seed = 928u;
    for (const auto& s : sizes)
    {
        INFO(s[0] << "x" << s[1]);
        const auto p = lcg(s[0], s[1], seed++);
        CHECK(scanlines(p, s[0], s[1]) == juceWriterRows(captureImage(p, s[0], s[1])));
    }
}

TEST_CASE("(2) the file parses: chunk CRCs check, IDAT inflates to exactly the scanlines", "[png_fast][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto p = lcg(131, 77, 5u);
    const auto scan = scanlines(p, 131, 77);
    const auto f = tmpFile("parse.png");
    REQUIRE(PngWrite::writeScanlinesReplacing(scan.data(), 131, 77, f, PngWrite::kFastPngLevel));
    juce::MemoryBlock mb;
    REQUIRE(f.loadFileAsData(mb));
    const auto* d = static_cast<const uint8_t*>(mb.getData());
    const size_t n = mb.getSize();
    static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    REQUIRE(n > 8);
    CHECK(std::memcmp(d, sig, 8) == 0);
    size_t pos = 8;
    std::vector<std::string> types;
    juce::MemoryBlock idat;
    while (pos + 12 <= n)
    {
        const uint32_t len = be32(d + pos);
        REQUIRE(pos + 12 + len <= n);
        const std::string type(reinterpret_cast<const char*>(d + pos + 4), 4);
        types.push_back(type);
        CHECK(be32(d + pos + 8 + len) == crcBitwise(d + pos + 4, 4 + len));
        if (type == "IHDR")
        {
            CHECK(len == 13u);
            CHECK(be32(d + pos + 8) == 131u); CHECK(be32(d + pos + 12) == 77u);
            CHECK(d[pos + 16] == 8); CHECK(d[pos + 17] == 6); CHECK(d[pos + 20] == 0);
        }
        if (type == "IDAT")
            idat.append(d + pos + 8, len);
        pos += 12 + len;
    }
    CHECK(pos == n);
    CHECK(types == std::vector<std::string>{ "IHDR", "IDAT", "IEND" });
    juce::MemoryInputStream zin(idat, false);
    juce::GZIPDecompressorInputStream inflate(&zin, false, juce::GZIPDecompressorInputStream::zlibFormat);
    juce::MemoryBlock raw;
    inflate.readIntoMemoryBlock(raw);
    REQUIRE(raw.getSize() == scan.size());
    CHECK(std::memcmp(raw.getData(), scan.data(), scan.size()) == 0);
    f.deleteFile();
}

TEST_CASE("(3) JUCE decodes the fast file and the JUCE-written file to byte-equal images", "[png_fast][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    int w = 0, h = 0;
    const auto px = everyAlphaGl(w, h);
    const auto fast = tmpFile("fast.png"), slow = tmpFile("juce.png");
    const auto scan = scanlines(px, w, h);
    REQUIRE(PngWrite::writeScanlinesReplacing(scan.data(), w, h, fast, PngWrite::kFastPngLevel));
    REQUIRE(PngWrite::writeReplacing(captureImage(px, w, h), slow));
    const auto a = juce::ImageFileFormat::loadFrom(fast).convertedToFormat(juce::Image::ARGB);
    const auto b = juce::ImageFileFormat::loadFrom(slow).convertedToFormat(juce::Image::ARGB);
    REQUIRE(a.isValid()); REQUIRE(b.isValid());
    REQUIRE(a.getWidth() == w); REQUIRE(a.getHeight() == h);
    const juce::Image::BitmapData da(a, juce::Image::BitmapData::readOnly), db(b, juce::Image::BitmapData::readOnly);
    int bad = 0;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            if (da.getPixelColour(x, y) != db.getPixelColour(x, y)) ++bad;
    CHECK(bad == 0);
    fast.deleteFile(); slow.deleteFile();
}

TEST_CASE("(4) the fast writer replaces an existing (bigger) file, never appends", "[png_fast][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto f = tmpFile("replace.png");
    const auto big = lcg(300, 200, 1u);
    const auto small = lcg(20, 10, 2u);
    const auto sBig = scanlines(big, 300, 200), sSmall = scanlines(small, 20, 10);
    REQUIRE(PngWrite::writeScanlinesReplacing(sBig.data(), 300, 200, f, PngWrite::kFastPngLevel));
    const auto bigSize = f.getSize();
    REQUIRE(PngWrite::writeScanlinesReplacing(sSmall.data(), 20, 10, f, PngWrite::kFastPngLevel));
    CHECK(f.getSize() < bigSize);
    const auto img = juce::ImageFileFormat::loadFrom(f);
    REQUIRE(img.isValid());
    CHECK(img.getWidth() == 20);
    CHECK(img.getHeight() == 10);
    f.deleteFile();
}

TEST_CASE("[bench] JUCE writer vs fast levels 0/1/2 (+6 filter 0), 1080p / 4K, smooth / noise", "[.bench][png_fast]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    using Clock = std::chrono::steady_clock;
    for (auto [w, h] : { std::pair{ 1920, 1080 }, std::pair{ 3840, 2160 } })
        for (bool noise : { false, true })
        {
            std::vector<uint8_t> px(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
            uint32_t s = 7u;
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x)
                {
                    auto* p = px.data() + (static_cast<size_t>(y) * static_cast<size_t>(w) + static_cast<size_t>(x)) * 4;
                    s = s * 1664525u + 1013904223u;
                    const int n = noise ? static_cast<int>(s >> 26) - 32 : 0;
                    p[0] = static_cast<uint8_t>(std::clamp(x * 255 / w + n, 0, 255));
                    p[1] = static_cast<uint8_t>(std::clamp(y * 255 / h + n, 0, 255));
                    p[2] = static_cast<uint8_t>(128 + n);
                    p[3] = 255;
                }
            const auto f = tmpFile("bench.png");
            const auto t0 = Clock::now();
            PngWrite::writeReplacing(captureImage(px, w, h), f);
            const double juceMs = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
            const auto juceBytes = f.getSize();
            std::cout << w << "x" << h << (noise ? " noise" : " smooth") << ": JUCE writer (convert + libpng) "
                      << juceMs << " ms, " << juceBytes << " B" << std::endl;
            for (int level : { 0, 1, 2, 6 })
            {
                const auto t1 = Clock::now();
                const auto sc = scanlines(px, w, h);
                const double convMs = std::chrono::duration<double, std::milli>(Clock::now() - t1).count();
                const auto t2 = Clock::now();
                PngWrite::writeScanlinesReplacing(sc.data(), w, h, f, level);
                const double pngMs = std::chrono::duration<double, std::milli>(Clock::now() - t2).count();
                std::cout << "    fast level " << level << " (filter 0): scanlines " << convMs << " ms + png " << pngMs
                          << " ms, " << f.getSize() << " B" << std::endl;
            }
            f.deleteFile();
        }
    SUCCEED();
}
