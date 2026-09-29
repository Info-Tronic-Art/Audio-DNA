#include <catch2/catch_test_macros.hpp>
#include "core/MediaPresence.h"
#include <algorithm>

// s-rta-0928b mediaopen (plan-mediaopen.md 4.6 / 4.9(2)): the pure half of MediaPresence (src/core/MediaPresence.h) --
// which paths a sweep stats, how its answers become Clip::mediaMissing, the load-time seed, and the flag's life in a Clip
// (clear() resets it, replaceContent() carries the new content's). The sweeper's timer / pool / callAsync is proven live
// (.harmony/probe-media-open.sh m7_presence). Linked like test_composition (Clip.cpp / Layer.cpp).

namespace
{
Clip mediaClip(uint32_t id, Clip::MediaType type, const juce::File& f)
{
    Clip c;
    c.id = id;
    c.mediaType = type;
    c.mediaFile = f;
    return c;
}

// One deck, one layer, columns = the given clips (nullopt = an empty cell).
Composition compOf(std::vector<std::optional<Clip>> cells)
{
    Composition comp;
    Deck d;
    Layer l;
    l.clips = std::move(cells);
    d.layers.push_back(std::move(l));
    d.numColumns = static_cast<int>(d.layers[0].clips.size());
    comp.decks.push_back(std::move(d));
    return comp;
}

Clip& cellAt(Composition& c, int col) { return *c.decks[0].layers[0].clips[static_cast<size_t>(col)]; }
} // namespace

TEST_CASE("presence::mediaPaths lists Image and Video paths once each", "[mediaopen][presence]")
{
    const juce::File a("/tmp/adna-presence/a.png"), b("/tmp/adna-presence/b.mp4");
    Clip seq; seq.mediaType = Clip::MediaType::ImageSequence; seq.sequenceFiles = { juce::File("/tmp/adna-presence/s0.png") };
    Clip src; src.mediaType = Clip::MediaType::Source; src.sourceType = "plasma";
    Clip emptyImage; emptyImage.mediaType = Clip::MediaType::Image;
    auto comp = compOf({ mediaClip(1, Clip::MediaType::Image, a), mediaClip(2, Clip::MediaType::Video, b),
                         mediaClip(3, Clip::MediaType::Image, a), seq, src, emptyImage, std::nullopt });
    const auto paths = presence::mediaPaths(comp);
    REQUIRE(paths.size() == 2);
    CHECK(std::count(paths.begin(), paths.end(), a.getFullPathName()) == 1);
    CHECK(std::count(paths.begin(), paths.end(), b.getFullPathName()) == 1);
}

TEST_CASE("presence::apply flips exactly the matching clips and counts changes", "[mediaopen][presence]")
{
    const juce::File a("/tmp/adna-presence/a.png"), b("/tmp/adna-presence/b.mp4"), c("/tmp/adna-presence/c.png");
    auto comp = compOf({ mediaClip(1, Clip::MediaType::Image, a), mediaClip(2, Clip::MediaType::Video, b),
                         mediaClip(3, Clip::MediaType::Image, a), mediaClip(4, Clip::MediaType::Image, c) });
    const std::vector<presence::Seen> seen = { { a.getFullPathName(), false }, { b.getFullPathName(), true } };
    CHECK(presence::apply(comp, seen) == 2);            // both clips of a flip; b was already present
    CHECK(cellAt(comp, 0).mediaMissing);
    CHECK_FALSE(cellAt(comp, 1).mediaMissing);
    CHECK(cellAt(comp, 2).mediaMissing);
    CHECK_FALSE(cellAt(comp, 3).mediaMissing);          // c not in this sweep: untouched
    CHECK(presence::apply(comp, seen) == 0);            // an identical sweep changes nothing
    CHECK(presence::apply(comp, { { a.getFullPathName(), true } }) == 2);   // a comes back
    CHECK_FALSE(cellAt(comp, 0).mediaMissing);
}

TEST_CASE("Clip::mediaMissing: clear() resets it, replaceContent() carries the new content's", "[mediaopen][presence]")
{
    Clip clip = mediaClip(1, Clip::MediaType::Image, juce::File("/tmp/adna-presence/a.png"));
    clip.mediaMissing = true;
    clip.clear();
    CHECK_FALSE(clip.mediaMissing);

    Clip incoming = mediaClip(2, Clip::MediaType::Image, juce::File("/tmp/adna-presence/b.png"));
    incoming.mediaMissing = true;
    REQUIRE(clip.replaceContent(incoming));
    CHECK(clip.mediaMissing);
    Clip present = mediaClip(3, Clip::MediaType::Image, juce::File("/tmp/adna-presence/c.png"));
    REQUIRE(clip.replaceContent(present));
    CHECK_FALSE(clip.mediaMissing);
}

TEST_CASE("presence::seed stats a staged deck's clips", "[mediaopen][presence]")
{
    const auto tmp = juce::File::createTempFile(".png");
    REQUIRE(tmp.replaceWithText("x"));
    const juce::File gone("/tmp/adna-presence-no-such-dir/gone.png");
    auto comp = compOf({ mediaClip(1, Clip::MediaType::Image, tmp), mediaClip(2, Clip::MediaType::Video, gone) });
    cellAt(comp, 0).mediaMissing = true;                // a stale flag is overwritten
    presence::seed(comp.decks[0]);
    CHECK_FALSE(cellAt(comp, 0).mediaMissing);
    CHECK(cellAt(comp, 1).mediaMissing);
    tmp.deleteFile();
}
