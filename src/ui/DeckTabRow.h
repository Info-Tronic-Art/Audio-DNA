#pragma once
#include <algorithm>
#include <string>
#include <vector>

// DeckTabRow -- s-rta-0926b plan6 §6.1. The deck tab row under the clip grid: one tab per deck, a square "+" button
// (New Deck / Load Deck...) right after the last tab, and -- for 10 s after a Remove Deck -- an "Undo Remove" button
// flush right. Pure (no JUCE, like TabBarLayout.h): DeckView measures the hint text and places its buttons from
// layout(); the two menus are listed here so tests/test_deck_tab_row.cpp pins items, order and enablement.
namespace DeckTabRow
{
inline constexpr int kTabWidth = 100, kTabGap = 2, kPlusWidth = 24, kMinTabWidth = 60, kHintGap = 8;   // kPlusWidth == DeckView::kDeckTabHeight (square)

struct Rect { int x; int w; };
struct Layout { std::vector<Rect> tabs; Rect plus; Rect hint; };   // hint.w == 0 => hidden

// Tabs keep 100 px while numDecks*(100+2)+24 fits rowWidth; otherwise every tab shrinks evenly (never below 60);
// the "+" follows the last tab but never leaves the row (x clamped to rowWidth-24; may overlap on absurd counts).
// The undo hint (hintWidth > 0) sits flush right and is hidden when it would come within kHintGap of the "+";
// tabs and the "+" NEVER move because a hint appears or disappears.
inline Layout layout(int rowWidth, int numDecks, int hintWidth = 0)
{
    Layout L;
    int w = kTabWidth;
    if (numDecks > 0 && numDecks * (kTabWidth + kTabGap) + kPlusWidth > rowWidth)
        w = std::max(kMinTabWidth, (rowWidth - kPlusWidth - numDecks * kTabGap) / numDecks);
    for (int i = 0; i < numDecks; ++i)
        L.tabs.push_back({ i * (w + kTabGap), w });
    L.plus = { std::min(std::max(0, numDecks) * (w + kTabGap), std::max(0, rowWidth - kPlusWidth)), kPlusWidth };
    L.hint = (hintWidth > 0 && rowWidth - hintWidth >= L.plus.x + kPlusWidth + kHintGap)
                 ? Rect{ rowWidth - hintWidth, hintWidth }
                 : Rect{ 0, 0 };
    return L;
}

// Lane bf9b S3.4 (ruling-bf9b 16(d)): the "Undo Remove" button's text. A deck removed while layers play its clips is
// kept (retired) and those clips keep playing until replaced -- the hint names the layers, so the removal never looks
// like it did nothing.
inline std::string undoRemoveHint(const std::string& deckName, const std::vector<std::string>& playingLayers)
{
    std::string s = "Undo Remove \"" + deckName + "\"";
    if (playingLayers.empty())
        return s;
    s += " -- ";
    for (size_t i = 0; i < playingLayers.size(); ++i)
        s += (i > 0 ? ", " : "") + playingLayers[i];
    s += playingLayers.size() == 1 ? " keeps playing its clip" : " keep playing their clips";
    return s;
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
