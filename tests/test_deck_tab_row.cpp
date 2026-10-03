// test_deck_tab_row -- s-rta-0926b plan6 §6.1: the deck tab row's geometry (tabs, the "+" button) and its two menus.
// Pure header src/ui/DeckTabRow.h -- no JUCE; DeckView places its buttons from it.
#include <catch2/catch_test_macros.hpp>
#include "ui/DeckTabRow.h"
#include <string>

namespace
{
    bool rectIs(const DeckTabRow::Rect& r, int x, int w) { return r.x == x && r.w == w; }
}

TEST_CASE("DeckTabRow::layout -- 100-px tabs with 2-px gaps, the \"+\" right after the last tab", "[decktabrow]")
{
    using namespace DeckTabRow;

    const auto one = layout(1000, 1);
    REQUIRE(one.tabs.size() == 1);
    CHECK(rectIs(one.tabs[0], 0, 100));
    CHECK(rectIs(one.plus, 102, 24));

    const auto three = layout(1000, 3);
    REQUIRE(three.tabs.size() == 3);
    CHECK(rectIs(three.tabs[0], 0, 100));
    CHECK(rectIs(three.tabs[1], 102, 100));
    CHECK(rectIs(three.tabs[2], 204, 100));
    CHECK(rectIs(three.plus, 306, 24));

    const auto none = layout(1000, 0);
    CHECK(none.tabs.empty());
    CHECK(rectIs(none.plus, 0, 24));
}

TEST_CASE("DeckTabRow::layout -- tabs shrink evenly when the row is full, never below 60 px; the \"+\" never leaves the row", "[decktabrow]")
{
    using namespace DeckTabRow;

    // 12 * 102 + 24 = 1248 > 1000: w = (1000 - 24 - 12*2) / 12 = 79.
    const auto twelve = layout(1000, 12);
    REQUIRE(twelve.tabs.size() == 12);
    for (size_t i = 0; i < twelve.tabs.size(); ++i)
        CHECK(rectIs(twelve.tabs[i], static_cast<int>(i) * 81, 79));
    CHECK(rectIs(twelve.plus, 972, 24));

    // (1000 - 24 - 40) / 20 = 46 -> floored at 60; the "+" is clamped to rowWidth - 24.
    const auto twenty = layout(1000, 20);
    REQUIRE(twenty.tabs.size() == 20);
    for (const auto& t : twenty.tabs)
        CHECK(t.w == kMinTabWidth);
    CHECK(rectIs(twenty.plus, 976, 24));
}

TEST_CASE("DeckTabRow::tabMenu / plusMenu -- items, order, separators, Remove only with more than one deck", "[decktabrow]")
{
    using namespace DeckTabRow;

    const auto single = tabMenu(1);
    REQUIRE(single.size() == 5);
    CHECK(single[0].action == Action::SaveDeck);
    CHECK(std::string(single[0].label) == "Save Deck");
    CHECK(single[1].action == Action::SaveDeckAs);
    CHECK(std::string(single[1].label) == "Save Deck As...");
    CHECK(single[2].action == Action::Rename);
    CHECK(std::string(single[2].label) == "Rename Deck...");
    CHECK(single[3].action == Action::Duplicate);
    CHECK(std::string(single[3].label) == "Duplicate Deck");
    CHECK(single[4].action == Action::Remove);
    CHECK(std::string(single[4].label) == "Remove Deck");

    CHECK_FALSE(single[0].separatorBefore);
    CHECK_FALSE(single[1].separatorBefore);
    CHECK(single[2].separatorBefore);
    CHECK_FALSE(single[3].separatorBefore);
    CHECK(single[4].separatorBefore);

    for (int i = 0; i < 4; ++i)
        CHECK(single[static_cast<size_t>(i)].enabled);
    CHECK_FALSE(single[4].enabled);        // the last deck cannot be removed

    const auto two = tabMenu(2);
    REQUIRE(two.size() == 5);
    CHECK(two[4].action == Action::Remove);
    CHECK(two[4].enabled);

    const auto plus = plusMenu();
    REQUIRE(plus.size() == 2);
    CHECK(plus[0].action == Action::NewDeck);
    CHECK(std::string(plus[0].label) == "New Deck");
    CHECK(plus[1].action == Action::LoadDeck);
    CHECK(std::string(plus[1].label) == "Load Deck...");
    CHECK(plus[0].enabled);
    CHECK(plus[1].enabled);
    CHECK_FALSE(plus[0].separatorBefore);
    CHECK_FALSE(plus[1].separatorBefore);

    // Menu item ids are the Action values (a JUCE PopupMenu result of 0 means "dismissed").
    CHECK(static_cast<int>(Action::NewDeck) == 1);
}

TEST_CASE("DeckTabRow::editorRect -- the rename box is at least a full tab wide, covers its tab and never leaves the row", "[decktabrow]")
{
    using namespace DeckTabRow;
    // Every case: the box covers its tab, starts inside the row and ends inside it.
    auto covers = [](Rect box, Rect tab, int rowWidth) {
        INFO("box " << box.x << "+" << box.w << " tab " << tab.x << "+" << tab.w << " row " << rowWidth);
        CHECK(box.x <= tab.x);
        CHECK(box.x + box.w >= tab.x + tab.w);
        CHECK(box.x >= 0);
        CHECK(box.x + box.w <= rowWidth);
    };

    // A 100-px tab: the same rect.
    const auto three = layout(1000, 3);
    CHECK(rectIs(editorRect(three.tabs[1], 1000), 102, 100));
    covers(editorRect(three.tabs[1], 1000), three.tabs[1], 1000);

    // A 60-px tab (20 decks in 1000 px): 100 wide, from the tab's x.
    const auto twenty = layout(1000, 20);
    REQUIRE(twenty.tabs[3].w == kMinTabWidth);
    CHECK(rectIs(editorRect(twenty.tabs[3], 1000), 186, 100));
    covers(editorRect(twenty.tabs[3], 1000), twenty.tabs[3], 1000);

    // The LAST 60-px tab at the row end (12 decks in 12 * 62 + 24 = 768 px): shifted left inside the row.
    const auto full = layout(768, 12);
    REQUIRE(full.tabs[11].w == kMinTabWidth);
    REQUIRE(full.tabs[11].x == 682);
    CHECK(rectIs(editorRect(full.tabs[11], 768), 668, 100));
    covers(editorRect(full.tabs[11], 768), full.tabs[11], 768);

    // One tab in a 100-px row (the tab shrinks to 74): the box fills the row.
    const auto tiny = layout(100, 1);
    CHECK(rectIs(editorRect(tiny.tabs[0], 100), 0, 100));
    covers(editorRect(tiny.tabs[0], 100), tiny.tabs[0], 100);
}
