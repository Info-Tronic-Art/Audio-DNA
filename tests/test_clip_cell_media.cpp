// test_clip_cell_media -- s-rta-1002b ui U2.2 / U2.3 (BF3 "Codec display for each video and easy access to that video
// in finder"; plan-ui.md U2.2-U2.3 as amended by ruling-ui.md AM10 / AM12). A clip cell's hover text and its right-click
// "Show in Finder" menu. Headless JUCE widgets under ScopedJuceInitialiser_GUI; the menu goes through an injected
// launcher (ClipCell::setMenuLauncherForTests), so no test ever opens a real PopupMenu; the video info comes from a fake
// VideoInfoSource. No file is touched: every path is made up.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/ClipCell.h"
#include "ui/DeckView.h"

#include <memory>
#include <vector>

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

Clip sourceClip()
{
    Clip c;
    c.name = "noise";
    c.mediaType = Clip::MediaType::Source;
    c.sourceType = "perlin_noise";
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

VideoInfo h264() { return { "H.264 High", 64, 64, 30.0 }; }

// A mouseDown at (10, 10) -- inside the thumbnail area -- with these modifiers.
juce::MouseEvent press(juce::Component& c, juce::ModifierKeys mods)
{
    const auto now = juce::Time::getCurrentTime();
    return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), { 10.0f, 10.0f }, mods,
                            juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation,
                            juce::MouseInputSource::defaultRotation, juce::MouseInputSource::defaultTiltX,
                            juce::MouseInputSource::defaultTiltY, &c, &c, now, { 10.0f, 10.0f }, now, 1, false);
}
const juce::ModifierKeys kLeft(juce::ModifierKeys::leftButtonModifier);
const juce::ModifierKeys kCtrlLeft(juce::ModifierKeys::leftButtonModifier | juce::ModifierKeys::ctrlModifier);
const juce::ModifierKeys kRight(juce::ModifierKeys::rightButtonModifier);

// A cell with recording callbacks and a recording launcher.
struct Rig
{
    VideoInfoSource source = [](const Clip&) -> std::optional<VideoInfo> { return h264(); };
    std::unique_ptr<ClipCell> cell = std::make_unique<ClipCell>();
    int triggers = 0, selects = 0;
    std::vector<std::pair<int, int>> reveals;
    struct Launch { juce::String header; juce::StringArray items; std::function<void(int)> done; };
    std::vector<Launch> launches;

    explicit Rig(Clip* clip)
    {
        cell->setSize(90, 96);
        cell->setGridPosition(2, 5);
        cell->setVideoInfoSource(&source);
        cell->setClip(clip);
        cell->onTrigger = [this](int, int) { ++triggers; };
        cell->onSelect = [this](int, int, bool) { ++selects; };
        cell->onRevealInFinder = [this](int l, int c) { reveals.emplace_back(l, c); };
        cell->setMenuLauncherForTests([this](const juce::String& h, const juce::StringArray& items,
                                             std::function<void(int)> done) {
            launches.push_back({ h, items, std::move(done) });
        });
    }
};
} // namespace

TEST_CASE("ClipCell media (a): a video cell's tooltip is the file name, the codec line and the menu hint",
          "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = videoClip();
    Rig r(&clip);
    CHECK(r.cell->getTooltip() == clipmedia::cellTooltip(clip, h264()));
    CHECK(r.cell->getTooltip()
          == "video_h264_64x64.mp4\nH.264 High, 64 x 64, 30 frames per second\nRight-click: Show in Finder");

    // The source is asked when JUCE asks (hover): a player that opens later shows up without a refresh.
    r.source = [](const Clip&) -> std::optional<VideoInfo> { return std::nullopt; };
    CHECK(r.cell->getTooltip() == "video_h264_64x64.mp4\nVideo file not loaded\nRight-click: Show in Finder");

    // A source cell has no tooltip.
    auto src = sourceClip();
    r.cell->setClip(&src);
    CHECK(r.cell->getTooltip().isEmpty());
}

TEST_CASE("ClipCell media (b): Ctrl+left on a VIDEO cell still triggers it (no menu)", "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = videoClip();
    Rig r(&clip);
    r.cell->mouseDown(press(*r.cell, kCtrlLeft));
    CHECK(r.triggers == 1);
    CHECK(r.launches.empty());
}

TEST_CASE("ClipCell media (b2): a right-click on a video cell opens the menu: header = the clip name, Show in Finder",
          "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = videoClip();
    Rig r(&clip);
    r.cell->mouseDown(press(*r.cell, kRight));
    REQUIRE(r.launches.size() == 1);
    CHECK(r.launches[0].header == "clip");
    CHECK(r.launches[0].items == juce::StringArray { "Show in Finder" });
    CHECK(r.triggers == 0);
    CHECK(r.selects == 0);
}

TEST_CASE("ClipCell media (b3): choosing Show in Finder calls onRevealInFinder(layer, column) once; dismissing does not",
          "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = videoClip();
    Rig r(&clip);
    r.cell->mouseDown(press(*r.cell, kRight));
    REQUIRE(r.launches.size() == 1);
    r.launches[0].done(0);                                   // dismissed
    CHECK(r.reveals.empty());
    r.launches[0].done(1);
    REQUIRE(r.reveals.size() == 1);
    CHECK(r.reveals[0] == std::make_pair(2, 5));

    // The clip lost its file while the menu was open: nothing to show, no call.
    auto src = sourceClip();
    r.cell->setClip(&src);
    r.cell->menuChosen(1);
    CHECK(r.reveals.size() == 1);
}

TEST_CASE("ClipCell media (b4): the cell is destroyed before the menu closes -> no call, no crash", "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = videoClip();
    Rig r(&clip);
    r.cell->mouseDown(press(*r.cell, kRight));
    REQUIRE(r.launches.size() == 1);
    r.cell.reset();                                          // a rebuildGrid while the menu is up
    r.launches[0].done(1);
    CHECK(r.reveals.empty());
}

TEST_CASE("ClipCell media (c): Ctrl+left on a SOURCE cell still triggers it", "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = sourceClip();
    Rig r(&clip);
    r.cell->mouseDown(press(*r.cell, kCtrlLeft));
    CHECK(r.triggers == 1);
    CHECK(r.launches.empty());
}

TEST_CASE("ClipCell media (d): a right-click on a source cell or an empty cell does nothing", "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = sourceClip();
    Rig r(&clip);
    r.cell->mouseDown(press(*r.cell, kRight));
    r.cell->setClip(nullptr);
    r.cell->mouseDown(press(*r.cell, kRight));
    auto empty = sequenceClip(0);                            // a sequence with no image: no menu either
    r.cell->setClip(&empty);
    r.cell->mouseDown(press(*r.cell, kRight));
    CHECK(r.launches.empty());
    CHECK(r.triggers == 0);
    CHECK(r.selects == 0);
    CHECK(r.reveals.empty());
}

TEST_CASE("ClipCell media (e): a sequence cell's tooltip says images per second, then the menu hint",
          "[clipcell][clipmedia]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto clip = sequenceClip(3);
    Rig r(&clip);
    const auto lines = juce::StringArray::fromLines(r.cell->getTooltip());
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == juce::String(juce::CharPointer_UTF8("Image sequence \xe2\x80\x94 3 images at 2.5 images per second")));
    CHECK(lines[1] == "Right-click: Show in Finder");
    // A left click still triggers a sequence cell.
    r.cell->mouseDown(press(*r.cell, kLeft));
    CHECK(r.triggers == 1);
}

TEST_CASE("DeckView fan-out (U2.3): every cell reads DeckView's video source; a cell's Show in Finder reaches "
          "DeckView::onRevealInFinder(layer, column) exactly once", "[clipcell][clipmedia][deckview]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    auto* deck = comp.getActiveDeck();
    REQUIRE(deck != nullptr);
    REQUIRE(deck->getRow(0) != nullptr);
    deck->getRow(0)->clips[3] = videoClip();
    deck->getRow(0)->clips[4] = sourceClip();

    DeckView dv;
    dv.setSize(1400, 600);
    dv.setComposition(&comp);
    // Set AFTER the grid exists: the cells hold the source's address, so they see it at once.
    dv.setVideoInfoSource([](const Clip&) -> std::optional<VideoInfo> { return h264(); });
    std::vector<std::pair<int, int>> reveals;
    dv.onRevealInFinder = [&](int l, int c) { reveals.emplace_back(l, c); };

    auto* cell = dv.cellForTests(0, 3);
    REQUIRE(cell != nullptr);
    CHECK(cell->getClip() == deck->getRow(0)->getClipAt(3));
    CHECK(cell->getTooltip()
          == "video_h264_64x64.mp4\nH.264 High, 64 x 64, 30 frames per second\nRight-click: Show in Finder");

    dv.revealCellForTests(0, 3);                             // the REST path: the cell's menuChosen(1)
    REQUIRE(reveals.size() == 1);
    CHECK(reveals[0] == std::make_pair(0, 3));

    // The real right-click path through the same cell, with an injected launcher.
    std::function<void(int)> done;
    cell->setMenuLauncherForTests([&](const juce::String&, const juce::StringArray&, std::function<void(int)> d) {
        done = std::move(d);
    });
    cell->mouseDown(press(*cell, kRight));
    REQUIRE(done != nullptr);
    done(1);
    REQUIRE(reveals.size() == 2);
    CHECK(reveals[1] == std::make_pair(0, 3));

    dv.revealCellForTests(0, 4);                             // a source: nothing to show
    dv.revealCellForTests(0, 5);                             // an empty cell
    dv.revealCellForTests(99, 0);                            // no such cell
    CHECK(reveals.size() == 2);

    // A rebuilt grid wires its fresh cells the same way.
    dv.rebuildGrid();
    auto* fresh = dv.cellForTests(0, 3);
    REQUIRE(fresh != nullptr);
    CHECK(fresh->getTooltip().contains("H.264 High"));
    dv.revealCellForTests(0, 3);
    CHECK(reveals.size() == 3);
}
