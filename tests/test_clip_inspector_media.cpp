// test_clip_inspector_media -- s-rta-1002b ui U2.4 (BF3 "Codec display for each video and easy access to that video in
// finder"; plan-ui.md U2.4 as amended by ruling-ui.md AM9). The Clip inspector shows, for a clip with a file, a "Show in
// Finder" button at the right of its name bar and one or two info rows under it (a video: codec, then size + rate).
// Headless JUCE widgets under ScopedJuceInitialiser_GUI; the video info comes from a fake VideoInfoSource. No file is
// touched: every path is made up.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/ClipInspector.h"
#include "ui/LookAndFeel.h"

#include <optional>

namespace
{
const juce::File kDir("/no/such/folder/bf3");

Clip videoClip()
{
    Clip c;
    c.name = "clip";
    c.mediaType = Clip::MediaType::Video;
    c.mediaFile = kDir.getChildFile("video_h264_64x64.mp4");
    return c;
}

Clip imageClip()
{
    Clip c;
    c.name = "picture";
    c.mediaType = Clip::MediaType::Image;
    c.mediaFile = kDir.getChildFile("a.png");
    return c;
}

Clip sourceClip()
{
    Clip c;
    c.name = "noise";
    c.mediaType = Clip::MediaType::Source;
    c.sourceType = "perlin_noise";        // no sourceParams: no Source section
    return c;
}

VideoInfo h264() { return { "H.264 High", 64, 64, 30.0 }; }

// The visible TextButton child whose text is `text`, else nullptr.
juce::TextButton* visibleButton(juce::Component& parent, const juce::String& text)
{
    for (int i = 0; i < parent.getNumChildComponents(); ++i)
        if (auto* b = dynamic_cast<juce::TextButton*>(parent.getChildComponent(i)))
            if (b->getButtonText() == text && b->isVisible())
                return b;
    return nullptr;
}

// An inspector with a switchable fake video source.
struct Rig
{
    std::optional<VideoInfo> info = h264();
    ClipInspector insp;
    Rig()
    {
        insp.setVideoInfoSource([this](const Clip&) { return info; });
        insp.setSize(303, 900);
    }
};
} // namespace

TEST_CASE("ClipInspector media (a): a video clip shows a visible Show in Finder button", "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    auto clip = videoClip();
    r.insp.setClip(&clip);
    auto* btn = visibleButton(r.insp, "Show in Finder");
    REQUIRE(btn != nullptr);
    CHECK(btn->getComponentID() == "revealClipFile");
    CHECK(r.insp.revealButtonShown());
}

TEST_CASE("ClipInspector media (b): the info rows add 18 px each to the preferred height", "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    // The same picture clip without and with its file: +18 (one row).
    auto pic = imageClip();
    pic.mediaFile = juce::File();
    r.insp.setClip(&pic);
    const int noFile = r.insp.getPreferredHeight();
    pic.mediaFile = kDir.getChildFile("a.png");
    r.insp.refresh();
    const int withFile = r.insp.getPreferredHeight();
    INFO("picture: no file " << noFile << ", with file " << withFile);
    CHECK(withFile - noFile == 18);

    // The same video clip: no file (0 rows), the player not answering (1 row), then answering (2 rows: 18 vs 36).
    auto vid = videoClip();
    vid.mediaFile = juce::File();
    r.info.reset();
    r.insp.setClip(&vid);
    const int base = r.insp.getPreferredHeight();
    vid.mediaFile = kDir.getChildFile("video_h264_64x64.mp4");
    r.insp.refresh();
    const int unknown = r.insp.getPreferredHeight();
    r.info = h264();
    r.insp.refresh();
    const int known = r.insp.getPreferredHeight();
    INFO("video: base " << base << ", unknown " << unknown << ", known " << known);
    CHECK(unknown - base == 18);
    CHECK(known - base == 36);
}

TEST_CASE("ClipInspector media (c): the rows are clipmedia::describe's lines; a source clip has no row and no button",
          "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    auto vid = videoClip();
    r.insp.setClip(&vid);
    CHECK(r.insp.mediaInfoLinesShown() == juce::StringArray { "H.264 High", "64 x 64, 30 frames per second" });

    auto pic = imageClip();
    r.insp.setClip(&pic);
    CHECK(r.insp.mediaInfoLinesShown() == juce::StringArray { "PNG image" });
    CHECK(r.insp.revealButtonShown());

    auto gone = videoClip();
    gone.mediaMissing = true;
    r.insp.setClip(&gone);
    CHECK(r.insp.mediaInfoLinesShown() == juce::StringArray { "File missing" });
    CHECK(r.insp.revealButtonShown());                       // Show in Finder then opens the folder it was in

    auto src = sourceClip();
    r.insp.setClip(&src);
    CHECK(r.insp.mediaInfoLinesShown().isEmpty());
    CHECK_FALSE(r.insp.revealButtonShown());
    CHECK(visibleButton(r.insp, "Show in Finder") == nullptr);

    r.insp.setClip(nullptr);
    CHECK(r.insp.mediaInfoLinesShown().isEmpty());
    CHECK(visibleButton(r.insp, "Show in Finder") == nullptr);
}

TEST_CASE("ClipInspector media (d): a player that answers later shows its codec after one refresh",
          "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    r.info.reset();
    auto vid = videoClip();
    r.insp.setClip(&vid);
    CHECK(r.insp.mediaInfoLinesShown() == juce::StringArray { "Video file not loaded" });
    r.insp.refresh();
    CHECK(r.insp.mediaInfoLinesShown() == juce::StringArray { "Video file not loaded" });
    r.info = h264();
    r.insp.refresh();
    CHECK(r.insp.mediaInfoLinesShown() == juce::StringArray { "H.264 High", "64 x 64, 30 frames per second" });
}

TEST_CASE("ClipInspector media (e): the button's click fires onRevealInFinder(clip) once", "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    auto vid = videoClip();
    r.insp.setClip(&vid);
    std::vector<Clip*> calls;
    r.insp.onRevealInFinder = [&](Clip* c) { calls.push_back(c); };
    auto* btn = visibleButton(r.insp, "Show in Finder");
    REQUIRE(btn != nullptr);
    REQUIRE(btn->onClick != nullptr);
    btn->onClick();
    REQUIRE(calls.size() == 1);
    CHECK(calls[0] == &vid);
}

TEST_CASE("ClipInspector media (g): every info line fits 200 px at the 10.5-px label font", "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;
    juce::LookAndFeel::setDefaultLookAndFeel(&laf);
    const juce::Font f(juce::FontOptions(10.5f));
    for (const char* s : { "H.264 Constrained Baseline", "ProRes 4444 XQ", "QuickTime Animation",
                           "4096 x 2160, 23.976 frames per second", "3840 x 2160, 59.94 frames per second" })
    {
        const float w = juce::GlyphArrangement::getStringWidth(f, s);
        INFO(s << " = " << w << " px");
        CHECK(w <= 200.0f);
    }
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

TEST_CASE("ClipInspector media (h): the button text fits the button", "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;
    juce::LookAndFeel::setDefaultLookAndFeel(&laf);
    const float w = juce::GlyphArrangement::getStringWidth(juce::Font(juce::FontOptions(14.0f)), "Show in Finder");
    INFO("Show in Finder = " << w << " px");
    CHECK(w + 16.0f <= static_cast<float>(ClipInspector::kRevealButtonWidth));
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

TEST_CASE("ClipInspector media (i): the name never runs under the button; without it the name bar is main's",
          "[clipinspector][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    auto vid = videoClip();
    r.insp.setClip(&vid);
    for (int w : { 200, 303, 400 })
    {
        r.insp.setSize(w, 900);
        auto* btn = visibleButton(r.insp, "Show in Finder");
        REQUIRE(btn != nullptr);
        const auto b = btn->getBounds();
        INFO("width " << w << ": button " << b.toString() << ", name " << ClipInspector::nameTextBounds(w, true).toString());
        CHECK(b == ClipInspector::revealButtonBounds(w));
        CHECK(b.getWidth() == ClipInspector::kRevealButtonWidth);
        CHECK(b.getY() >= 0);
        CHECK(b.getBottom() <= 28);                          // inside the 28-px name bar
        CHECK(b.getRight() <= w);
        CHECK(ClipInspector::nameTextBounds(w, true).getRight() <= b.getX() - 4);
        CHECK(ClipInspector::nameTextBounds(w, false)
              == juce::Rectangle<int>(0, 0, w, 28).withTrimmedLeft(4).withTrimmedRight(40));
    }
}
