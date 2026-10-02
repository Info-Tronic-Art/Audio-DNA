// test_deck_tab_rename -- s-rta-1002b ui U3 (BF8: "Need to be able to rename each deck with double click"; plan-ui.md
// U3.1-U3.2 as amended by ruling-ui.md AM2-AM7). Headless DeckView under ScopedJuceInitialiser_GUI (no window, no peer,
// no message loop): decks "A" "B" "C", deck "B" showing. Real JUCE dispatch cannot run off screen (ruling E-R1), so
// the mouse is driven here in JUCE's VERIFIED order (ruling E-R3): the tab's mouseDown reaches DeckView's nested
// listener, then mouseUp fires the tab's onClick, and a double-click is delivered after the second click's onClick --
// to the tab if it survived, else to DeckView. The real dispatch is probe_deck_tab_dispatch (never part of ctest).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/DeckView.h"

#include <vector>

namespace
{
// Decks "A" "B" "C" (ids 0, 1, 2); deck 1 ("B") showing.
void makeComposition(Composition& comp)
{
    comp.initDefault();
    comp.decks[0].name = "A";
    comp.addDeck("B");
    comp.addDeck("C");
    comp.activeDeckIndex = 1;
}

// The deck tab whose text is `name`: a direct child TextButton of DeckView (the column triggers are "1".."12").
juce::TextButton* tabNamed(DeckView& dv, const juce::String& name)
{
    for (int i = 0; i < dv.getNumChildComponents(); ++i)
        if (auto* b = dynamic_cast<juce::TextButton*>(dv.getChildComponent(i)))
            if (b->getButtonText() == name)
                return b;
    return nullptr;
}

int countTabs(DeckView& dv)
{
    int n = 0;
    for (const char* name : { "A", "B", "C" })
        if (tabNamed(dv, name) != nullptr)
            ++n;
    return n;
}

// What Button::mouseUp reaches. A COPY: a rebuilding handler destroys the button (and its std::function) mid-call.
void click(juce::TextButton* b)
{
    REQUIRE(b != nullptr);
    auto fn = b->onClick;
    REQUIRE(fn != nullptr);
    fn();
}
} // namespace

TEST_CASE("DeckView tab (a): a click on the SHOWING tab never calls onDeckSwitched", "[decktabs][rename]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    makeComposition(comp);
    DeckView dv;
    dv.setSize(1400, 600);
    dv.setComposition(&comp);

    std::vector<int> calls;
    dv.onDeckSwitched = [&](int i) { calls.push_back(i); };   // records only, never rebuilds
    click(tabNamed(dv, "B"));
    CHECK(calls.empty());
}

TEST_CASE("DeckView tab (a2): with a REBUILDING switch handler, the showing tab rebuilds nothing; another tab rebuilds once", "[decktabs][rename]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    makeComposition(comp);
    DeckView dv;
    dv.setSize(1400, 600);
    dv.setComposition(&comp);

    // MainComponent.cpp handleDeckSwitch: set the active deck, then rebuildGrid unconditionally.
    dv.onDeckSwitched = [&](int i) { comp.activeDeckIndex = i; dv.rebuildGrid(); };

    const int b0 = dv.tabRowBuilds();
    click(tabNamed(dv, "B"));
    CHECK(dv.tabRowBuilds() == b0);
    CHECK(countTabs(dv) == 3);
    CHECK(comp.activeDeckIndex == 1);

    click(tabNamed(dv, "C"));
    CHECK(dv.tabRowBuilds() == b0 + 1);
    CHECK(countTabs(dv) == 3);
    CHECK(comp.activeDeckIndex == 2);
}

TEST_CASE("DeckView tab (b): a click on another tab switches at once, exactly once, with its index", "[decktabs][rename]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    makeComposition(comp);
    DeckView dv;
    dv.setSize(1400, 600);
    dv.setComposition(&comp);

    std::vector<int> calls;
    dv.onDeckSwitched = [&](int i) { calls.push_back(i); };
    click(tabNamed(dv, "C"));
    REQUIRE(calls.size() == 1);
    CHECK(calls[0] == 2);
    calls.clear();
    click(tabNamed(dv, "A"));
    REQUIRE(calls.size() == 1);
    CHECK(calls[0] == 0);
}
