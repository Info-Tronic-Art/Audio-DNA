#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "media/ImageSequence.h"
#include <vector>

// s-rta-0928b mediaopen (plan-mediaopen.md 4.5 / 4.9(3)): ImageSequence::open does NO file I/O -- no stat per file (a
// missing frame stays in the list and decodes Failed: the previous frame repeats, renderleft R-7), no frame-0 decode --
// and ImageSequence::seekTo is a REQUEST consumed at the top of advanceFrame on the GL thread (VideoPlayer's shape),
// before the playing check. Links the real ImageSequence.cpp; no GL call is reached (open / seekTo / advanceFrame only).
// Headless JUCE (juce_opengl for the GL symbols the .cpp names).

using Catch::Matchers::WithinAbs;

namespace
{
// Paths in a directory that does not exist: nothing can be read, and nothing is created.
std::vector<juce::File> missingPaths(int n)
{
    const auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getChildFile("adna-mediaopen-no-such-dir-" + juce::String(juce::Random::getSystemRandom().nextInt64()));
    std::vector<juce::File> files;
    for (int i = 0; i < n; ++i)
        files.push_back(dir.getChildFile("f" + juce::String(i).paddedLeft('0', 3) + ".png"));
    return files;
}

// n existing (non-image-content) files in a fresh temp dir -- the seek cases do not depend on open()'s stat.
struct RealFiles
{
    explicit RealFiles(int n)
        : dir(juce::File::getSpecialLocation(juce::File::tempDirectory)
                  .getChildFile("adna-mediaopen-seek-" + juce::String(juce::Random::getSystemRandom().nextInt64())))
    {
        REQUIRE(dir.createDirectory());
        for (int i = 0; i < n; ++i)
        {
            auto f = dir.getChildFile("f" + juce::String(i) + ".png");
            REQUIRE(f.replaceWithText("x"));
            files.push_back(f);
        }
    }
    ~RealFiles() { dir.deleteRecursively(); }
    juce::File dir;
    std::vector<juce::File> files;
};
} // namespace

TEST_CASE("ImageSequence::open keeps frames whose files do not exist (no stat)", "[mediaopen][imageseq][open]")
{
    SECTION("three missing paths open as three frames and touch nothing")
    {
        const auto files = missingPaths(3);
        ImageSequence seq;
        REQUIRE(seq.open(files));
        CHECK(seq.isOpen());
        CHECK(seq.getFrameCount() == 3);
        for (const auto& f : files)
            CHECK_FALSE(f.exists());
        CHECK_FALSE(files[0].getParentDirectory().exists());
    }

    SECTION("a 12-path list with path 5 missing keeps 12 frames, in natural order")
    {
        const auto tmp = juce::File::getSpecialLocation(juce::File::tempDirectory)
                             .getChildFile("adna-mediaopen-seq12-" + juce::String(juce::Random::getSystemRandom().nextInt64()));
        REQUIRE(tmp.createDirectory());
        std::vector<juce::File> files;
        for (int i = 0; i < 12; ++i)
        {
            auto f = tmp.getChildFile("f" + juce::String(i).paddedLeft('0', 2) + ".png");
            if (i != 5)
                REQUIRE(f.replaceWithText("x"));   // content never read by open()
            files.push_back(f);
        }
        std::vector<juce::File> shuffled(files.rbegin(), files.rend());
        ImageSequence seq;
        REQUIRE(seq.open(shuffled));
        CHECK(seq.getFrameCount() == 12);
        REQUIRE(seq.getFiles().size() == 12);
        CHECK(seq.getFiles()[5] == files[5]);   // the missing file holds its slot: later frames keep their timing
        CHECK(seq.getFiles()[6] == files[6]);
        tmp.deleteRecursively();
    }

    SECTION("a list with no image extension still refuses")
    {
        ImageSequence seq;
        CHECK_FALSE(seq.open({ juce::File("/tmp/adna-not-an-image.txt") }));
        CHECK_FALSE(seq.open({}));
    }
}

TEST_CASE("ImageSequence::seekTo is a request consumed by advanceFrame", "[mediaopen][imageseq][seek]")
{
    SECTION("the playhead moves only when the GL thread advances")
    {
        ImageSequence seq;
        RealFiles rf(3);
        REQUIRE(seq.open(rf.files));
        seq.seekTo(0.5);
        CHECK_THAT(seq.getPlayheadPosition(), WithinAbs(0.0, 1e-9));   // not applied on the calling thread
        seq.advanceFrame(0.0);
        CHECK_THAT(seq.getPlayheadPosition(), WithinAbs(0.5, 1e-9));
    }

    SECTION("a paused sequence takes the seek")
    {
        ImageSequence seq;
        RealFiles rf(3);
        REQUIRE(seq.open(rf.files));
        seq.setPlaying(false);
        seq.seekTo(1.0);
        seq.advanceFrame(0.0);
        CHECK_THAT(seq.getPlayheadPosition(), WithinAbs(1.0, 1e-9));
    }

    SECTION("two seeks before one advance: the last target lands")
    {
        ImageSequence seq;
        RealFiles rf(3);
        REQUIRE(seq.open(rf.files));
        seq.seekTo(0.25);
        seq.seekTo(0.75);
        seq.advanceFrame(0.0);
        CHECK_THAT(seq.getPlayheadPosition(), WithinAbs(0.75, 1e-9));
        seq.advanceFrame(0.0);   // consumed once: nothing re-applies
        CHECK_THAT(seq.getPlayheadPosition(), WithinAbs(0.75, 1e-9));
    }

    SECTION("a re-open drops a pending seek")
    {
        ImageSequence seq;
        RealFiles rf(3);
        REQUIRE(seq.open(rf.files));
        seq.seekTo(0.9);
        RealFiles rf4(4);
        REQUIRE(seq.open(rf4.files));
        seq.advanceFrame(0.0);
        CHECK_THAT(seq.getPlayheadPosition(), WithinAbs(0.0, 1e-9));
    }
}
