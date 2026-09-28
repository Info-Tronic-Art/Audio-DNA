// test_png_write -- s-rta-0927 follow-ups F2: a capture written to a path that already holds a file must REPLACE it.
// juce::FileOutputStream opens an existing file at its END (juce_SharedCode_posix.h openHandle: O_RDWR + lseek
// SEEK_END), so a bare stream write appends a second PNG after the old one and every decoder returns the OLD picture
// (renderperf found_not_fixed #4). PngWrite::writeReplacing is the one writer both PNG sites use.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/PngWrite.h"

namespace
{
juce::Image solid(int w, int h, juce::Colour c)
{
    juce::Image img(juce::Image::ARGB, w, h, false);
    img.clear(img.getBounds(), c);
    return img;
}

juce::File scratchDir()
{
    return juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("audiodna-png-write-" + juce::Uuid().toString());
}

// The raw sequence every PNG site used before F2 (Renderer.cpp captureFrame at 8c4c1a1): no delete, no truncate.
bool rawStreamWrite(const juce::Image& img, const juce::File& file)
{
    file.getParentDirectory().createDirectory();
    juce::FileOutputStream fos(file);
    return fos.openedOk() && juce::PNGImageFormat().writeImageToStream(img, fos);
}
}

TEST_CASE("writeReplacing over an existing PNG leaves exactly the new PNG", "[pngwrite][replace]")
{
    const auto dir = scratchDir();
    const auto p = dir.getChildFile("same.png");
    const auto q = dir.getChildFile("fresh.png");

    REQUIRE(PngWrite::writeReplacing(solid(64, 64, juce::Colours::red), p));
    REQUIRE(PngWrite::writeReplacing(solid(16, 16, juce::Colours::blue), p));
    REQUIRE(PngWrite::writeReplacing(solid(16, 16, juce::Colours::blue), q));

    const auto decoded = juce::ImageFileFormat::loadFrom(p);
    INFO("decoded " << decoded.getWidth() << "x" << decoded.getHeight() << ", file " << p.getSize()
                    << " bytes, a fresh write of the same image " << q.getSize() << " bytes");
    CHECK(decoded.getWidth() == 16);
    CHECK(decoded.getHeight() == 16);
    CHECK(decoded.isValid());
    if (decoded.isValid())
        CHECK(decoded.getPixelAt(0, 0) == juce::Colours::blue);

    juce::MemoryBlock pBytes, qBytes;
    REQUIRE(p.loadFileAsData(pBytes));
    REQUIRE(q.loadFileAsData(qBytes));
    CHECK(pBytes == qBytes);   // byte-identical to a fresh write: nothing appended, nothing left over

    dir.deleteRecursively();
}

TEST_CASE("teeth: JUCE's FileOutputStream appends to an existing file", "[pngwrite][juce-appends]")
{
    // Documents WHY PngWrite exists. If a JUCE bump stops appending, this fails and the helper's delete is redundant.
    const auto dir = scratchDir();
    const auto p = dir.getChildFile("twice.png");
    const auto a = dir.getChildFile("first.png");
    const auto b = dir.getChildFile("second.png");

    REQUIRE(rawStreamWrite(solid(64, 64, juce::Colours::red), a));
    REQUIRE(rawStreamWrite(solid(16, 16, juce::Colours::blue), b));
    REQUIRE(rawStreamWrite(solid(64, 64, juce::Colours::red), p));
    REQUIRE(rawStreamWrite(solid(16, 16, juce::Colours::blue), p));

    CHECK(p.getSize() == a.getSize() + b.getSize());

    dir.deleteRecursively();
}

TEST_CASE("writeReplacing to a fresh path in a missing directory", "[pngwrite][fresh]")
{
    const auto dir = scratchDir();
    const auto p = dir.getChildFile("sub").getChildFile("new.png");
    REQUIRE_FALSE(p.getParentDirectory().exists());

    REQUIRE(PngWrite::writeReplacing(solid(32, 24, juce::Colours::green), p));
    const auto decoded = juce::ImageFileFormat::loadFrom(p);
    CHECK(decoded.getWidth() == 32);
    CHECK(decoded.getHeight() == 24);

    dir.deleteRecursively();
}
