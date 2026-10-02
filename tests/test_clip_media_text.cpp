// test_clip_media_text -- s-rta-1002b ui U2.1 (BF3; plan-ui.md U2.1 + ruling-ui.md AM9 / AM12). clipmedia:: is the one
// place that words a clip's file for the cell tooltip, the cell menu, the Clip inspector's info rows and REST. Pure
// (juce_core + Clip + VideoInfo): no file is touched -- every path below is made up.
#include <catch2/catch_test_macros.hpp>
#include "ui/ClipMediaText.h"

namespace
{
const juce::File kDir("/no/such/folder/bf3");

Clip videoClip(const juce::String& file = "video_h264_64x64.mp4")
{
    Clip c;
    c.name = "clip";
    c.mediaType = Clip::MediaType::Video;
    c.mediaFile = kDir.getChildFile(file);
    return c;
}

Clip imageClip(const juce::String& file)
{
    Clip c;
    c.name = "picture";
    c.mediaType = Clip::MediaType::Image;
    c.mediaFile = kDir.getChildFile(file);
    return c;
}

Clip sequenceClip(int n)
{
    Clip c;
    c.name = "seq";
    c.mediaType = Clip::MediaType::ImageSequence;
    for (int i = 0; i < n; ++i)
        c.sequenceFiles.push_back(kDir.getChildFile("frame" + juce::String(i) + ".png"));
    c.sequenceFps = 2.5f;
    return c;
}

Clip typedClip(Clip::MediaType t)
{
    Clip c;
    c.name = "other";
    c.mediaType = t;
    c.sourceType = t == Clip::MediaType::Source ? "perlin_noise" : "";
    return c;
}

VideoInfo h264() { return { "H.264 High", 64, 64, 30.0 }; }
VideoInfo prores() { return { "ProRes 422 HQ", 1920, 1080, 30000.0 / 1001.0 }; }

std::vector<juce::String> v(std::initializer_list<const char*> l)
{
    std::vector<juce::String> out;
    for (auto* s : l) out.emplace_back(s);
    return out;
}

const juce::String kDash = juce::String(juce::CharPointer_UTF8("\xe2\x80\x94"));   // the em dash of today's sequence tooltip
}

TEST_CASE("clipmedia::revealTarget: the file a Show in Finder selects", "[clipmedia]")
{
    CHECK(clipmedia::revealTarget(videoClip()) == kDir.getChildFile("video_h264_64x64.mp4"));
    CHECK(clipmedia::revealTarget(imageClip("a.png")) == kDir.getChildFile("a.png"));
    CHECK(clipmedia::revealTarget(sequenceClip(3)) == kDir.getChildFile("frame0.png"));
    CHECK(clipmedia::revealTarget(sequenceClip(0)) == juce::File());          // an empty sequence has nothing to show
    CHECK(clipmedia::revealTarget(typedClip(Clip::MediaType::Source)) == juce::File());
    CHECK(clipmedia::revealTarget(typedClip(Clip::MediaType::Camera)) == juce::File());
    CHECK(clipmedia::revealTarget(typedClip(Clip::MediaType::None)) == juce::File());
    auto noFile = videoClip();
    noFile.mediaFile = juce::File();
    CHECK(clipmedia::revealTarget(noFile) == juce::File());
}

TEST_CASE("clipmedia::describe: a video whose player is open shows two lines, codec then size + rate", "[clipmedia]")
{
    const auto c = videoClip();
    const auto d = clipmedia::describe(c, h264());
    CHECK(d.fileBacked);
    CHECK_FALSE(d.missing);
    CHECK(d.lines == v({ "H.264 High", "64 x 64, 30 frames per second" }));
    CHECK(d.line == "H.264 High, 64 x 64, 30 frames per second");
    CHECK(d.revealTarget == c.mediaFile);
    CHECK(d.pathTip == c.mediaFile.getFullPathName());

    const auto p = clipmedia::describe(videoClip("x.mov"), prores());
    CHECK(p.lines == v({ "ProRes 422 HQ", "1920 x 1080, 29.97 frames per second" }));
    CHECK(p.line == "ProRes 422 HQ, 1920 x 1080, 29.97 frames per second");

    VideoInfo noRate { "HAP Q", 640, 360, 0.0 };
    CHECK(clipmedia::describe(c, noRate).lines == v({ "HAP Q", "640 x 360" }));
}

TEST_CASE("clipmedia::describe: a video with no open player, and a missing video", "[clipmedia]")
{
    const auto c = videoClip();
    const auto none = clipmedia::describe(c, std::nullopt);
    CHECK(none.fileBacked);
    CHECK(none.lines == v({ "Video file not loaded" }));
    CHECK(none.line == "Video file not loaded");
    CHECK(clipmedia::describe(c, VideoInfo {}).lines == v({ "Video file not loaded" }));   // an unknown codec

    auto gone = videoClip();
    gone.mediaMissing = true;
    const auto m = clipmedia::describe(gone, h264());                  // "missing" wins over a stale player
    CHECK(m.fileBacked);
    CHECK(m.missing);
    CHECK(m.lines == v({ "File missing" }));
    CHECK(m.line == "File missing");
    CHECK(m.revealTarget == gone.mediaFile);                           // Show in Finder then opens the folder
}

TEST_CASE("clipmedia::describe: a picture is its kind, from the extension", "[clipmedia]")
{
    CHECK(clipmedia::describe(imageClip("a.png"), std::nullopt).lines == v({ "PNG image" }));
    CHECK(clipmedia::describe(imageClip("a.png"), std::nullopt).line == "PNG image");
    CHECK(clipmedia::describe(imageClip("b.JPG"), std::nullopt).line == "JPEG image");
    CHECK(clipmedia::describe(imageClip("c.jpeg"), std::nullopt).line == "JPEG image");
    CHECK(clipmedia::describe(imageClip("d.tif"), std::nullopt).line == "TIFF image");
    CHECK(clipmedia::describe(imageClip("e.tiff"), std::nullopt).line == "TIFF image");
    CHECK(clipmedia::describe(imageClip("f.gif"), std::nullopt).line == "GIF image");
    CHECK(clipmedia::describe(imageClip("g.bmp"), std::nullopt).line == "BMP image");
    CHECK(clipmedia::describe(imageClip("a.png"), std::nullopt).fileBacked);

    auto gone = imageClip("a.png");
    gone.mediaMissing = true;
    const auto m = clipmedia::describe(gone, std::nullopt);
    CHECK(m.missing);
    CHECK(m.lines == v({ "File missing" }));
}

TEST_CASE("clipmedia::describe: an image sequence is its image count; an empty one has no row", "[clipmedia]")
{
    const auto s = clipmedia::describe(sequenceClip(3), std::nullopt);
    CHECK(s.fileBacked);
    CHECK_FALSE(s.missing);
    CHECK(s.lines == v({ "Image sequence, 3 images" }));
    CHECK(s.line == "Image sequence, 3 images");
    CHECK(s.pathTip == kDir.getChildFile("frame0.png").getFullPathName());

    const auto e = clipmedia::describe(sequenceClip(0), std::nullopt);
    CHECK_FALSE(e.fileBacked);
    CHECK(e.lines.empty());
    CHECK(e.line.isEmpty());
}

TEST_CASE("clipmedia::describe: sources, cameras, effects-only and file-less clips have no row", "[clipmedia]")
{
    for (auto t : { Clip::MediaType::Source, Clip::MediaType::Camera, Clip::MediaType::None })
    {
        const auto d = clipmedia::describe(typedClip(t), std::nullopt);
        CHECK_FALSE(d.fileBacked);
        CHECK_FALSE(d.missing);
        CHECK(d.lines.empty());
        CHECK(d.line.isEmpty());
        CHECK(d.pathTip.isEmpty());
        CHECK(d.revealTarget == juce::File());
    }
    auto noFile = videoClip();
    noFile.mediaFile = juce::File();
    CHECK_FALSE(clipmedia::describe(noFile, h264()).fileBacked);
    CHECK(clipmedia::describe(noFile, h264()).lines.empty());
}

TEST_CASE("clipmedia::cellTooltip: file name, the one-line info, and where the menu is", "[clipmedia]")
{
    CHECK(clipmedia::cellTooltip(videoClip(), h264())
          == "video_h264_64x64.mp4\nH.264 High, 64 x 64, 30 frames per second\nRight-click: Show in Finder");
    CHECK(clipmedia::cellTooltip(videoClip(), std::nullopt)
          == "video_h264_64x64.mp4\nVideo file not loaded\nRight-click: Show in Finder");
    CHECK(clipmedia::cellTooltip(imageClip("a.png"), std::nullopt) == "a.png\nPNG image\nRight-click: Show in Finder");
    auto gone = videoClip();
    gone.mediaMissing = true;
    CHECK(clipmedia::cellTooltip(gone, h264()) == "video_h264_64x64.mp4\nFile missing\nRight-click: Show in Finder");

    // AM12: today's sequence line, with "images per second" in whole words (UI Text Rules), then the menu line.
    CHECK(clipmedia::cellTooltip(sequenceClip(3), std::nullopt)
          == "Image sequence " + kDash + " 3 images at 2.5 images per second\nRight-click: Show in Finder");
    // An empty sequence has nothing to show in Finder: the first line only.
    CHECK(clipmedia::cellTooltip(sequenceClip(0), std::nullopt)
          == "Image sequence " + kDash + " 0 images at 2.5 images per second");

    CHECK(clipmedia::cellTooltip(typedClip(Clip::MediaType::Source), std::nullopt).isEmpty());
    CHECK(clipmedia::cellTooltip(typedClip(Clip::MediaType::Camera), std::nullopt).isEmpty());
    CHECK(clipmedia::cellTooltip(typedClip(Clip::MediaType::None), std::nullopt).isEmpty());
}

TEST_CASE("clipmedia::menuItems: Show in Finder only for a clip with a file", "[clipmedia]")
{
    const juce::StringArray show { "Show in Finder" };
    CHECK(clipmedia::menuItems(nullptr).isEmpty());
    const auto vc = videoClip();
    CHECK(clipmedia::menuItems(&vc) == show);
    const auto ic = imageClip("a.png");
    CHECK(clipmedia::menuItems(&ic) == show);
    const auto sc = sequenceClip(3);
    CHECK(clipmedia::menuItems(&sc) == show);
    auto gone = videoClip();
    gone.mediaMissing = true;
    CHECK(clipmedia::menuItems(&gone) == show);                        // a missing file: its folder opens
    const auto empty = sequenceClip(0);
    CHECK(clipmedia::menuItems(&empty).isEmpty());
    const auto src = typedClip(Clip::MediaType::Source);
    CHECK(clipmedia::menuItems(&src).isEmpty());
    const auto none = typedClip(Clip::MediaType::None);
    CHECK(clipmedia::menuItems(&none).isEmpty());
}
