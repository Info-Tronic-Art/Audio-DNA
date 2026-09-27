// test_deck_tab_row -- s-rta-0926b plan6 §6.1: the deck tab row's geometry (tabs, the "+" button, the Remove-Deck
// undo hint) and its two menus. Pure header src/ui/DeckTabRow.h -- no JUCE; DeckView places its buttons from it.
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

TEST_CASE("DeckTabRow::layout -- the undo hint sits flush right and hides when it would crowd the \"+\"", "[decktabrow]")
{
    using namespace DeckTabRow;

    CHECK(rectIs(layout(1000, 3, 120).hint, 880, 120));
    CHECK(rectIs(layout(1000, 12, 120).hint, 0, 0));   // 972 + 24 + 8 > 880
    CHECK(rectIs(layout(1000, 3, 0).hint, 0, 0));      // no hint asked for

    // Tabs and the "+" never move because a hint appears or disappears.
    const auto without = layout(1000, 3);
    const auto with = layout(1000, 3, 120);
    for (size_t i = 0; i < without.tabs.size(); ++i)
        CHECK(rectIs(with.tabs[i], without.tabs[i].x, without.tabs[i].w));
    CHECK(rectIs(with.plus, without.plus.x, without.plus.w));
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
