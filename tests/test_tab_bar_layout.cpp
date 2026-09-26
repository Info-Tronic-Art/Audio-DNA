// test_tab_bar_layout -- s-rta-0925 step-4 polish D2: the Browser's tab widths follow the measured label
// widths so the longest tab label is never clipped. Pure function in src/ui/TabBarLayout.h -- no JUCE.
#include <catch2/catch_test_macros.hpp>
#include "ui/TabBarLayout.h"

TEST_CASE("TabBarLayout -- the Browser's six tabs at the crop width: Compositions gets its whole label", "[tabbarlayout]")
{
    // 14 px approximations for Files/FX/Sources/Compositions/Record/MilkDrop (the arithmetic is the pin,
    // not the font; "Compositions" replaced the abbreviated "Comp/Decks" -- s-rta-0925 visual gate MUST,
    // critic-ux.md). Pre-fix rule: 428 / 6 = 71 < 94 -- the clip seen in every step-4 crop.
    const std::vector<int> labelWidths = { 33, 18, 52, 94, 44, 58 };
    const auto widths = tabWidthsFor(labelWidths, 8, 428);
    const std::vector<int> expected = { 55, 40, 74, 115, 65, 79 };
    CHECK(widths == expected);
    int sum = 0;
    for (auto w : widths) sum += w;
    CHECK(sum == 428);
    CHECK(widths[3] >= 94 + 16);
}

TEST_CASE("TabBarLayout -- exact sum and every minimum whenever the minimums fit", "[tabbarlayout]")
{
    const std::vector<std::vector<int>> cases = { {10, 20, 30}, {5}, {40, 40, 40, 40} };
    const std::vector<int> totals = { 100, 101, 333 };
    for (const auto& labels : cases)
    {
        int minSum = 0;
        for (auto l : labels) minSum += l + 16;
        for (auto total : totals)
        {
            if (minSum > total)
                continue;
            const auto widths = tabWidthsFor(labels, 8, total);
            int sum = 0;
            for (size_t i = 0; i < widths.size(); ++i)
            {
                CHECK(widths[i] >= labels[i] + 16);
                sum += widths[i];
            }
            CHECK(sum == total);
        }
    }
}

TEST_CASE("TabBarLayout -- the leftover pixels go to the leftmost tabs, one each", "[tabbarlayout]")
{
    const std::vector<int> labelWidths = { 10, 20, 30 };
    const auto widths = tabWidthsFor(labelWidths, 8, 200);
    const std::vector<int> expected = { 57, 67, 76 };
    CHECK(widths == expected);
}

TEST_CASE("TabBarLayout -- equal labels give equal widths", "[tabbarlayout]")
{
    const std::vector<int> labelWidths(6, 20);
    {
        const auto widths = tabWidthsFor(labelWidths, 8, 216);
        for (auto w : widths) CHECK(w == 36);
    }
    {
        const auto widths = tabWidthsFor(labelWidths, 8, 300);
        for (auto w : widths) CHECK(w == 50);
    }
}

TEST_CASE("TabBarLayout -- too narrow: proportional, exact sum, the longest label keeps the largest share", "[tabbarlayout]")
{
    const std::vector<int> labelWidths = { 10, 20, 30 };
    {
        const auto widths = tabWidthsFor(labelWidths, 8, 54);
        const std::vector<int> expected = { 13, 18, 23 };
        CHECK(widths == expected);
    }
    {
        const auto widths = tabWidthsFor(labelWidths, 8, 55);
        const std::vector<int> expected = { 14, 18, 23 };
        CHECK(widths == expected);
    }
    {
        const auto widths = tabWidthsFor(labelWidths, 8, 1);
        const std::vector<int> expected = { 1, 0, 0 };
        CHECK(widths == expected);
    }
}

TEST_CASE("TabBarLayout -- edges", "[tabbarlayout]")
{
    CHECK(tabWidthsFor({}, 8, 100).empty());
    {
        const std::vector<int> labelWidths = { 10, 20 };
        const auto widths = tabWidthsFor(labelWidths, 8, 0);
        for (auto w : widths) CHECK(w == 0);
    }
    {
        const std::vector<int> labelWidths = { 42 };
        const auto widths = tabWidthsFor(labelWidths, 8, 100);
        CHECK(widths == std::vector<int>{ 100 });
    }
    {
        // A negative label width counts as 0.
        const std::vector<int> labelWidths = { -5 };
        const auto widths = tabWidthsFor(labelWidths, 8, 100);
        CHECK(widths == std::vector<int>{ 100 });
    }
    {
        // Negative padding counts as 0.
        const std::vector<int> labelWidths = { 20 };
        const auto widths = tabWidthsFor(labelWidths, -8, 100);
        CHECK(widths == std::vector<int>{ 100 });
    }
}
