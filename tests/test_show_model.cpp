// Lane bf9b (s-rta-1002b), plan-bf9b S2.12 / ruling-bf9b amendments 2(c), 12: the show model -- ONE shared layer stack
// (Composition::layers) over deck boxes of clip rows (Deck::rows); a layer names its clip by ClipRef (deck id, column).
// S2a lands the first cases (T2, T3, T9, T10, T15 -- the model API S2a introduces); S2b adds the rest of plan S2.12.
// RED on STAGE_P_HEAD / S1 head: does not compile (no Composition::layers, no ClipRef trigger API).
#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "ShowFixture.h"
#include <vector>

using ShowFixture::makeShow;

TEST_CASE("T2 firing a cell of a deck that is not shown puts its ClipRef in that row's shared layer (bf9b)", "[show]")
{
    Composition c = makeShow(2, 3, 4);
    c.activeDeckIndex = 0;
    const uint32_t deck1 = c.decks[1].id;
    const auto t = c.fire(1, 1, 2);
    CHECK(t.changed());
    CHECK(c.layers[1].runtime().activeRef() == ClipRef{ deck1, 2 });
    REQUIRE(c.playingClip(1) != nullptr);
    CHECK(c.playingClip(1) == c.decks[1].getClip(1, 2));
    CHECK(c.playingClip(1)->id == 100u * 1 + 10u * 1 + 2 + 1);
    CHECK(c.playingClip(0) == nullptr);   // other layers untouched
    CHECK(c.activeDeckIndex == 0);        // a fire never moves the grid
}

TEST_CASE("T3 the same column from another deck is a new clip: a crossfade, not a retrigger (bf9b)", "[show]")
{
    Composition c = makeShow(2, 2, 4);
    c.layers[0].transitionSpeed = 1.0f;   // a real fade
    c.fire(0, 0, 2);
    const auto t = c.fire(0, 1, 2);
    const auto rt = c.layers[0].runtime();
    CHECK(t.changed());
    CHECK(rt.activeRef() == ClipRef{ c.decks[1].id, 2 });
    CHECK(rt.previousRef() == ClipRef{ c.decks[0].id, 2 });
    CHECK(rt.crossfadeProgress == 0.0f);
    // and the same ref again IS a retrigger: the tuple keeps its fade
    const auto again = c.fire(0, 1, 2);
    CHECK_FALSE(again.changed());
}

TEST_CASE("T9 C1 playing(i): one load, the incoming clip, nullptr when clear or empty, any deck incl. a retired one (bf9b)",
          "[show]")
{
    Composition c = makeShow(3, 2, 3);
    CHECK(c.playingClip(0) == nullptr);                   // clear
    c.fire(0, 2, 1);                                      // deck 2 (not shown)
    auto p = c.playing(0);
    REQUIRE(p.clip != nullptr);
    CHECK(p.ref == ClipRef{ c.decks[2].id, 1 });
    CHECK(p.deckIndex == 2);
    CHECK_FALSE(p.retired);
    CHECK(p.clip == c.decks[2].getClip(0, 1));

    // An empty target cell clears the layer (F12 / Layer::triggerClip's empty-cell branch) -> nullptr.
    c.decks[0].clearCell(1, 0);
    c.fire(1, 0, 0);
    CHECK(c.playingClip(1) == nullptr);

    // Retire deck 2 while layer 0 plays from it: the clip still resolves, at the same address.
    const Clip* before = p.clip;
    const uint32_t id2 = c.decks[2].id;
    CHECK(c.retireOrEraseDeck(2));
    REQUIRE(c.decks.size() == 2);
    CHECK(c.getNumRetiredDecks() == 1);
    p = c.playing(0);
    CHECK(p.clip == before);
    CHECK(p.retired);
    CHECK(p.deckIndex == -1);
    CHECK(p.ref.deckId == id2);
}

TEST_CASE("T10 C2 forEachLayer visits each shared layer once; forEachClip live decks by index, then retired, rows and "
          "columns ascending (bf9b)", "[show]")
{
    Composition c = makeShow(3, 2, 2);
    std::vector<int> seenLayers;
    c.forEachLayer([&](Layer&, int i) { seenLayers.push_back(i); });
    CHECK(seenLayers == std::vector<int>{ 0, 1 });

    c.fire(0, 1, 0);                   // deck 1 plays -> it is retired, not erased
    const uint32_t id1 = c.decks[1].id;
    REQUIRE(c.retireOrEraseDeck(1));

    std::vector<uint32_t> order;
    std::vector<int> deckIdx;
    std::vector<bool> retired;
    c.forEachClip([&](Clip& clip, const ClipSite& site) {
        order.push_back(clip.id);
        deckIdx.push_back(site.deckIndex);
        retired.push_back(site.retired);
        if (site.retired) CHECK(site.deckId == id1);
    });
    // live deck 0, live deck 2 (now index 1), then the retired deck 1; rows then columns ascending
    const std::vector<uint32_t> expected{ 1, 2, 11, 12, 201, 202, 211, 212, 101, 102, 111, 112 };
    CHECK(order == expected);
    CHECK(deckIdx == std::vector<int>{ 0, 0, 0, 0, 1, 1, 1, 1, -1, -1, -1, -1 });
    CHECK(retired.back());
}

TEST_CASE("T15 a ref the tuple cannot hold is refused at every tuple-writing entry: the tuple is byte-identical (bf9b, "
          "ruling-bf9b amendment 2(c))", "[show]")
{
    Composition c = makeShow(2, 2, 3);
    c.fire(0, 0, 1);
    const auto before0 = c.layers[0].runtime();
    const auto before1 = c.layers[1].runtime();
    auto unchanged = [&] {
        CHECK(c.layers[0].runtime() == before0);
        CHECK(c.layers[1].runtime() == before1);
    };
    const RowClips rows0 = c.rowClips(0);

    // column > kMaxColumn
    CHECK_FALSE(c.fire(0, 0, ClipRef::kMaxColumn + 1).changed());
    unchanged();
    CHECK_FALSE(c.layers[0].triggerClip(ClipRef{ c.decks[0].id, ClipRef::kMaxColumn + 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].triggerClipImmediate(ClipRef{ c.decks[0].id, ClipRef::kMaxColumn + 1 }, rows0).changed());
    unchanged();
    // deck > kMaxDeckId
    CHECK_FALSE(c.layers[0].triggerClip(ClipRef{ ClipRef::kMaxDeckId + 1, 0 }, rows0).changed());
    CHECK_FALSE(c.layers[0].releaseMomentary(ClipRef{ ClipRef::kMaxDeckId + 1, 1 }, rows0).changed());
    unchanged();
    // deck == kNoDeck with a column (the deck-less S1 form)
    CHECK_FALSE(c.layers[0].triggerClip(ClipRef{ ClipRef::kNoDeck, 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].triggerClipImmediate(ClipRef{ ClipRef::kNoDeck, 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].releaseMomentary(ClipRef{ ClipRef::kNoDeck, 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].clearActiveClip(rows0, ClipRef{ ClipRef::kNoDeck, 1 }).changed());
    unchanged();
    // an unknown deck index (Composition::fire / triggerColumn resolve a deck-less ref)
    CHECK_FALSE(c.fire(0, 7, 1).changed());
    std::vector<std::optional<LayerRuntimeTransition>> out;
    c.triggerColumn(7, 1, Clip::BeatSnapMode::Off, &out);
    unchanged();
    c.triggerColumn(0, ClipRef::kMaxColumn + 1, Clip::BeatSnapMode::Off, &out);
    unchanged();
}
