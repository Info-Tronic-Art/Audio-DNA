#pragma once
#include <algorithm>
#include <vector>

// DeckTabRow -- s-rta-0926b plan6 §6.1. The deck tab row under the clip grid: one tab per deck and a square "+" button
// (New Deck / Load Deck...) right after the last tab. Pure (no JUCE, like TabBarLayout.h): DeckView places its buttons
// from layout(); the two menus are listed here so tests/test_deck_tab_row.cpp pins items, order and enablement.
namespace DeckTabRow
{
inline constexpr int kTabWidth = 100, kTabGap = 2, kPlusWidth = 24, kMinTabWidth = 60;   // kPlusWidth == DeckView::kDeckTabHeight (square)

struct Rect { int x; int w; };
struct Layout { std::vector<Rect> tabs; Rect plus; };

// Tabs keep 100 px while numDecks*(100+2)+24 fits rowWidth; otherwise every tab shrinks evenly (never below 60);
// the "+" follows the last tab but never leaves the row (x clamped to rowWidth-24; may overlap on absurd counts).
inline Layout layout(int rowWidth, int numDecks)
{
    Layout L;
    int w = kTabWidth;
    if (numDecks > 0 && numDecks * (kTabWidth + kTabGap) + kPlusWidth > rowWidth)
        w = std::max(kMinTabWidth, (rowWidth - kPlusWidth - numDecks * kTabGap) / numDecks);
    for (int i = 0; i < numDecks; ++i)
        L.tabs.push_back({ i * (w + kTabGap), w });
    L.plus = { std::min(std::max(0, numDecks) * (w + kTabGap), std::max(0, rowWidth - kPlusWidth)), kPlusWidth };
    return L;
}

// s-rta-1002b ui U3.2 (BF8): the in-place rename box over a tab (x relative to the row, like layout()). At least
// kTabWidth wide -- a 60-px box shows about 6 characters at the tab's 14-px font -- starting at the tab, shifted left so
// it never leaves the row: it always covers its own tab and may cover the next one while it is open.
inline Rect editorRect(Rect tab, int rowWidth)
{
    const int w = std::max(tab.w, kTabWidth);
    return { std::max(0, std::min(tab.x, rowWidth - w)), w };
}

// Menu item ids ARE these values (a JUCE PopupMenu result of 0 means "dismissed").
enum class Action : int { NewDeck = 1, LoadDeck, SaveDeck, SaveDeckAs, Rename, Duplicate, Remove };

struct Item { Action action; const char* label; bool enabled; bool separatorBefore; };

// Right-click on a tab: Save Deck | Save Deck As... | -- | Rename Deck... | Duplicate Deck | -- | Remove Deck
// (Remove enabled iff more than one deck -- a composition keeps at least one).
inline std::vector<Item> tabMenu(int numDecks)
{
    return {
        { Action::SaveDeck,   "Save Deck",       true,         false },
        { Action::SaveDeckAs, "Save Deck As...", true,         false },
        { Action::Rename,     "Rename Deck...",  true,         true  },
        { Action::Duplicate,  "Duplicate Deck",  true,         false },
        { Action::Remove,     "Remove Deck",     numDecks > 1, true  },
    };
}

// The "+" button: New Deck | Load Deck...
inline std::vector<Item> plusMenu()
{
    return {
        { Action::NewDeck,  "New Deck",     true, false },
        { Action::LoadDeck, "Load Deck...", true, false },
    };
}
} // namespace DeckTabRow
