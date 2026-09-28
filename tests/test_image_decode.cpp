// test_image_decode -- s-rta-0928 renderleft R1.2: image files decoded + converted OFF the GL thread
// (src/render/ImageDecode.h). Runs with NO GL context: the decoder never makes a GL call. The bytes it delivers are
// the bytes the old GL-thread loops uploaded (their loops, copied verbatim, are the oracle).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/ImageDecode.h"
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

namespace
{
using Clock = std::chrono::steady_clock;

// The OLD loop of CompositorEngine::loadKeyImage (6db8d67), verbatim, applied to a decoded image (straight RGBA).
std::vector<uint8_t> oldStraight(juce::Image img)
{
    img = img.convertedToFormat(juce::Image::ARGB);
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
    std::vector<uint8_t> flipped(rgba.size());
    size_t rowBytes = static_cast<size_t>(w * 4);
    for (int y = 0; y < h; ++y)
        std::memcpy(flipped.data() + static_cast<size_t>(y) * rowBytes,
                     rgba.data() + static_cast<size_t>((h - 1 - y)) * rowBytes,
                     rowBytes);
    return flipped;
}

// The OLD loop of TextureManager::uploadImage (6db8d67), verbatim (premultiplied RGBA).
std::vector<uint8_t> oldPremultiplied(const juce::Image& image)
{
    int w = image.getWidth();
    int h = image.getHeight();
    auto argbImage = image.convertedToFormat(juce::Image::ARGB);
    juce::Image::BitmapData bitmapData(argbImage, juce::Image::BitmapData::readOnly);
    std::vector<uint8_t> rgbaPixels(static_cast<size_t>(w * h * 4));
    for (int y = 0; y < h; ++y)
    {
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

juce::File tempDir()
{
    auto d = juce::File::getSpecialLocation(juce::File::tempDirectory)
                 .getChildFile("test_image_decode_" + juce::Uuid().toString());   // unique: ctest runs cases in parallel
    d.createDirectory();
    return d;
}

void writePng(const juce::Image& img, const juce::File& f)
{
    f.deleteFile();
    juce::FileOutputStream fos(f);
    REQUIRE(fos.openedOk());
    REQUIRE(juce::PNGImageFormat().writeImageToStream(img, fos));
}

juce::Image variedAlpha(int w, int h, int seed)
{
    juce::Image img(juce::Image::ARGB, w, h, false, juce::SoftwareImageType());
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
    uint32_t s = static_cast<uint32_t>(seed);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            s = s * 1664525u + 1013904223u;
            bmp.setPixelColour(x, y, juce::Colour(static_cast<uint8_t>(s >> 24), static_cast<uint8_t>(s >> 16),
                                                  static_cast<uint8_t>(x * 3 + y), static_cast<uint8_t>((x * 7 + y * 5) & 255)));
        }
    return img;
}

// Waits (<= 10 s) until the mailbox holds n results.
std::vector<ImageDecode::Result> waitFor(ImageDecode::Mailbox& box, size_t n)
{
    std::vector<ImageDecode::Result> out;
    const auto t0 = Clock::now();
    while (out.size() < n && Clock::now() - t0 < std::chrono::seconds(10))
    {
        box.tryDrain(out);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return out;
}
} // namespace

TEST_CASE("(1) a decoded PNG with varied alpha: both layouts == the old loops on the same file", "[image_decode][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto dir = tempDir();
    const auto f = dir.getChildFile("a.png");
    writePng(variedAlpha(97, 61, 7), f);
    const auto reference = juce::ImageFileFormat::loadFrom(f);
    REQUIRE(reference.isValid());

    ImageDecode::Decoder dec(2);
    auto box = std::make_shared<ImageDecode::Mailbox>();
    dec.request(f, ImageDecode::Layout::StraightRGBA, 11u, std::nullopt, box);
    dec.request(f, ImageDecode::Layout::PremultipliedRGBA, 12u, std::nullopt, box);
    auto rs = waitFor(*box, 2);
    REQUIRE(rs.size() == 2);
    for (auto& r : rs)
    {
        INFO("tag " << r.tag);
        CHECK(r.kind == ImageTexCache::Kind::Decoded);
        CHECK(r.path == f.getFullPathName().toStdString());
        CHECK(r.w == 97); CHECK(r.h == 61);
        CHECK(r.stamp == ImageDecode::stampOf(f));
        if (r.tag == 11u) CHECK(r.rgba == oldStraight(reference));
        else { CHECK(r.tag == 12u); CHECK(r.rgba == oldPremultiplied(reference)); }
    }
    dir.deleteRecursively();
}

TEST_CASE("(2) a known stamp equal to the file's -> Unchanged, no decode; a rewritten file -> Decoded",
          "[image_decode][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto dir = tempDir();
    const auto f = dir.getChildFile("b.png");
    writePng(variedAlpha(33, 20, 1), f);
    const auto s = ImageDecode::stampOf(f);
    auto r = ImageDecode::decodeFile(f, ImageDecode::Layout::StraightRGBA, 1u, s);
    CHECK(r.kind == ImageTexCache::Kind::Unchanged);
    CHECK(r.rgba.empty());
    writePng(variedAlpha(34, 20, 2), f);   // a different size: the stamp changes even within one mtime tick
    r = ImageDecode::decodeFile(f, ImageDecode::Layout::StraightRGBA, 1u, s);
    CHECK(r.kind == ImageTexCache::Kind::Decoded);
    CHECK(r.w == 34);
    dir.deleteRecursively();
}

TEST_CASE("(3) a file that is not an image -> Failed", "[image_decode][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto dir = tempDir();
    const auto f = dir.getChildFile("not.png");
    REQUIRE(f.replaceWithText("this is not a png"));
    ImageDecode::Decoder dec(1);
    auto box = std::make_shared<ImageDecode::Mailbox>();
    dec.request(f, ImageDecode::Layout::StraightRGBA, 3u, std::nullopt, box);
    auto rs = waitFor(*box, 1);
    REQUIRE(rs.size() == 1);
    CHECK(rs[0].kind == ImageTexCache::Kind::Failed);
    CHECK(rs[0].tag == 3u);
    CHECK(rs[0].rgba.empty());
    dir.deleteRecursively();
}

TEST_CASE("(4) the consumer's mailbox dies while a job runs: no crash, the destructor returns",
          "[image_decode][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto dir = tempDir();
    const auto f = dir.getChildFile("c.png");
    writePng(variedAlpha(256, 256, 3), f);
    {
        ImageDecode::Decoder dec(1);
        auto box = std::make_shared<ImageDecode::Mailbox>();
        std::weak_ptr<ImageDecode::Mailbox> weak = box;
        for (int i = 0; i < 4; ++i)
            dec.request(f, ImageDecode::Layout::StraightRGBA, static_cast<uint64_t>(i), std::nullopt, box);
        box.reset();   // the only owner goes (a retired sequence / a released compositor)
        CHECK(weak.expired());
    }                  // ~Decoder: queued jobs dropped, the running one finishes and drops its result
    SUCCEED("destroyed with jobs queued and the mailbox gone");
    dir.deleteRecursively();
}

TEST_CASE("(5) destroying a busy decoder drops its queued jobs (quit time is bounded)", "[image_decode][s-rta-0928]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto dir = tempDir();
    const auto f = dir.getChildFile("noise.png");
    {
        juce::Image img(juce::Image::ARGB, 1024, 1024, false, juce::SoftwareImageType());
        juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
        uint32_t s = 928u;
        for (int y = 0; y < 1024; ++y)
            for (int x = 0; x < 1024; ++x)
            {
                s = s * 1664525u + 1013904223u;
                bmp.setPixelColour(x, y, juce::Colour(static_cast<uint8_t>(s >> 24), static_cast<uint8_t>(s >> 16),
                                                      static_cast<uint8_t>(s >> 8), static_cast<uint8_t>(255)));
            }
        writePng(img, f);
    }
    // One decode, timed.
    const auto t1 = Clock::now();
    const auto one = ImageDecode::decodeFile(f, ImageDecode::Layout::StraightRGBA, 0u, std::nullopt);
    const double singleMs = std::chrono::duration<double, std::milli>(Clock::now() - t1).count();
    REQUIRE(one.kind == ImageTexCache::Kind::Decoded);

    auto box = std::make_shared<ImageDecode::Mailbox>();
    const auto t0 = Clock::now();
    {
        ImageDecode::Decoder dec(1);
        for (int i = 0; i < 30; ++i)
            dec.request(f, ImageDecode::Layout::StraightRGBA, static_cast<uint64_t>(i), std::nullopt, box);
    }
    const double elapsedMs = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
    const int delivered = box->pending();
    INFO("single decode " << singleMs << " ms; 30 requests, destroyed at once: " << elapsedMs << " ms, delivered "
                          << delivered);
    CHECK(elapsedMs < 0.5 * 30.0 * singleMs);
    CHECK(delivered < 30);
    dir.deleteRecursively();
}
