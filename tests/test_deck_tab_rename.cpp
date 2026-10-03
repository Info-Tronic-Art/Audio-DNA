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

// ===================================================================================================================
// U3.2 (ruling AM2-AM7 (c)-(j)): the rename box.
// ===================================================================================================================
namespace
{
const juce::ModifierKeys kLeft(juce::ModifierKeys::leftButtonModifier);
const juce::ModifierKeys kRight(juce::ModifierKeys::rightButtonModifier);
const juce::ModifierKeys kCtrlLeft(juce::ModifierKeys::leftButtonModifier | juce::ModifierKeys::ctrlModifier);

juce::MouseEvent mouseEvent(juce::Component* eventComp, juce::Component* original, juce::ModifierKeys mods, int clicks)
{
    const auto now = juce::Time::getCurrentTime();
    return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), {}, mods,
                            juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation,
                            juce::MouseInputSource::defaultRotation, juce::MouseInputSource::defaultTiltX,
                            juce::MouseInputSource::defaultTiltY, eventComp, original, now, {}, now, clicks, false);
}

// One press on a tab in JUCE's order: the mouseDown reaches DeckView's nested listener (after the tab's own
// mouseDown), then mouseUp fires the tab's onClick -- unless it is a popup click (DeckTabButton swallows those).
void press(DeckView& dv, const juce::String& tabName, int clicks, juce::ModifierKeys mods = kLeft)
{
    auto* tab = tabNamed(dv, tabName);
    REQUIRE(tab != nullptr);
    dv.tabRowMouseForTests().mouseDown(mouseEvent(tab, tab, mods, clicks));
    if (! mods.isPopupMenu())
        click(tab);
}

// A double-click on a tab in JUCE's order (ruling E-R3): press 1, press 2 (on the tab as rebuilt), then
// mouseDoubleClick -- to the tab if it survived its second onClick, else to DeckView.
void doubleClick(DeckView& dv, const juce::String& tabName, juce::ModifierKeys mods = kLeft)
{
    press(dv, tabName, 1, mods);
    auto* tab = tabNamed(dv, tabName);
    REQUIRE(tab != nullptr);
    juce::Component::SafePointer<juce::Component> second(tab);
    dv.tabRowMouseForTests().mouseDown(mouseEvent(tab, tab, mods, 2));
    if (! mods.isPopupMenu())
        click(tab);
    juce::Component* target = second != nullptr ? second.getComponent() : static_cast<juce::Component*>(&dv);
    dv.tabRowMouseForTests().mouseDoubleClick(mouseEvent(target, target, mods, 2));
}

// The VISIBLE TextEditor among DeckView's direct children (nullptr when none).
juce::TextEditor* visibleEditor(DeckView& dv)
{
    for (int i = 0; i < dv.getNumChildComponents(); ++i)
        if (auto* ed = dynamic_cast<juce::TextEditor*>(dv.getChildComponent(i)))
            if (ed->isVisible())
                return ed;
    return nullptr;
}

juce::Rectangle<int> expectedBox(DeckView& dv, const juce::String& tabName)
{
    const auto tab = tabNamed(dv, tabName)->getBounds();
    const auto r = DeckTabRow::editorRect({ tab.getX(), tab.getWidth() }, dv.getWidth());
    return { r.x, tab.getY(), r.w, tab.getHeight() };
}

ClipCell* firstClipCell(juce::Component& root)
{
    for (int i = 0; i < root.getNumChildComponents(); ++i)
    {
        auto* child = root.getChildComponent(i);
        if (auto* cell = dynamic_cast<ClipCell*>(child))
            return cell;
        if (auto* found = firstClipCell(*child))
            return found;
    }
    return nullptr;
}

struct Rig
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    DeckView dv;
    std::vector<std::pair<int, juce::String>> renames;
    int closes = 0;
    int switches = 0;
    std::vector<bool> boxVisibleAtClose;   // onRenameClosed must run BEFORE the box hides (ruling E-R4, AM4)

    explicit Rig(bool rebuildingSwitch = true)
    {
        makeComposition(comp);
        dv.setSize(1400, 600);
        dv.setComposition(&comp);
        dv.onDeckRenamed = [this](int i, const juce::String& t) { renames.emplace_back(i, t); };
        dv.onRenameClosed = [this] { ++closes; boxVisibleAtClose.push_back(dv.renameEditorForTests()->isVisible()); };
        if (rebuildingSwitch)
            dv.onDeckSwitched = [this](int i) { ++switches; comp.activeDeckIndex = i; dv.rebuildGrid(); };
        else
            dv.onDeckSwitched = [this](int) { ++switches; };
    }

    void key(const juce::KeyPress& k) { dv.renameEditorForTests()->keyPressed(k); }
    // Every onRenameClosed so far ran while the box was still visible (focus handed home BEFORE the hide).
    bool closedBeforeHide() const
    {
        for (bool v : boxVisibleAtClose)
            if (! v)
                return false;
        return static_cast<int>(boxVisibleAtClose.size()) == closes;
    }
};

const juce::KeyPress kReturn(juce::KeyPress::returnKey);
const juce::KeyPress kTab(juce::KeyPress::tabKey);
const juce::KeyPress kShiftTab(juce::KeyPress::tabKey, juce::ModifierKeys::shiftModifier, 0);
const juce::KeyPress kEsc(juce::KeyPress::escapeKey);
} // namespace

TEST_CASE("DeckView rename (c): a left double-click on the SHOWING tab opens the box over it, its name all selected", "[decktabs][rename]")
{
    Rig r;
    doubleClick(r.dv, "B");
    auto* ed = visibleEditor(r.dv);
    REQUIRE(ed != nullptr);
    CHECK(ed->getText() == "B");
    CHECK(ed->getHighlightedRegion() == juce::Range<int>(0, 1));
    CHECK(ed->getBounds() == expectedBox(r.dv, "B"));
    CHECK(std::abs(ed->getFont().getHeight() - 14.0f) < 0.01f);
    CHECK(r.dv.renamingDeckIndex() == 1);
    CHECK(r.switches == 0);
}

TEST_CASE("DeckView rename (c2): a double-click on a tab NOT showing is one deck switch and no box (Boris Q1 default a)", "[decktabs][rename]")
{
    Rig r;
    doubleClick(r.dv, "C");
    CHECK(r.comp.activeDeckIndex == 2);
    CHECK(r.switches == 1);
    CHECK(visibleEditor(r.dv) == nullptr);
    CHECK_FALSE(r.dv.isRenaming());
}

TEST_CASE("DeckView rename (c3): a rebuild BETWEEN the two clicks on the showing tab still opens the box (clicks are counted per mouse source)", "[decktabs][rename]")
{
    Rig r;
    press(r.dv, "B", 1);
    r.dv.rebuildGrid();                                   // e.g. a REST / MIDI switch landing between the clicks
    auto* tab = tabNamed(r.dv, "B");
    r.dv.tabRowMouseForTests().mouseDown(mouseEvent(tab, tab, kLeft, 2));
    click(tab);
    r.dv.tabRowMouseForTests().mouseDoubleClick(mouseEvent(tab, tab, kLeft, 2));
    REQUIRE(visibleEditor(r.dv) != nullptr);
    CHECK(r.dv.renamingDeckIndex() == 1);
}

TEST_CASE("DeckView rename (c4): a double-click delivered to DeckView itself (the tab died) opens the box on the armed deck", "[decktabs][rename]")
{
    Rig r;
    press(r.dv, "B", 1);
    press(r.dv, "B", 2);
    r.dv.tabRowMouseForTests().mouseDoubleClick(mouseEvent(&r.dv, &r.dv, kLeft, 2));   // no position is read
    REQUIRE(visibleEditor(r.dv) != nullptr);
    CHECK(r.dv.renamingDeckIndex() == 1);
}

TEST_CASE("DeckView rename (d): a right or Ctrl double-click never opens the box; a popup-menu press clears the arm", "[decktabs][rename]")
{
    {
        Rig r;
        doubleClick(r.dv, "B", kRight);
        CHECK(visibleEditor(r.dv) == nullptr);
    }
    {
        Rig r;
        doubleClick(r.dv, "B", kCtrlLeft);
        CHECK(visibleEditor(r.dv) == nullptr);
    }
    {
        Rig r;
        press(r.dv, "B", 1);
        press(r.dv, "B", 2);                              // armed
        press(r.dv, "B", 1, kRight);                      // a right-click (the tab's menu) in between
        auto* tab = tabNamed(r.dv, "B");
        r.dv.tabRowMouseForTests().mouseDoubleClick(mouseEvent(tab, tab, kLeft, 2));
        CHECK(visibleEditor(r.dv) == nullptr);
    }
}

TEST_CASE("DeckView rename (d2): one click on A, then one on B, never opens a box (even if JUCE counted 2)", "[decktabs][rename]")
{
    Rig r;
    press(r.dv, "A", 1);                                  // switches to A (rebuild)
    press(r.dv, "B", 2);
    auto* tab = tabNamed(r.dv, "B");
    r.dv.tabRowMouseForTests().mouseDoubleClick(mouseEvent(tab, tab, kLeft, 2));
    CHECK(visibleEditor(r.dv) == nullptr);
}

TEST_CASE("DeckView rename (d3): a double-click INSIDE the open box neither commits nor re-opens it", "[decktabs][rename]")
{
    Rig r;
    doubleClick(r.dv, "B");
    auto* ed = r.dv.renameEditorForTests();
    REQUIRE(ed->isVisible());
    ed->setText("Intro", false);
    REQUIRE(ed->getNumChildComponents() > 0);
    auto* inner = ed->getChildComponent(0);
    for (auto* c : { static_cast<juce::Component*>(ed), inner })
    {
        r.dv.tabRowMouseForTests().mouseDown(mouseEvent(c, c, kLeft, 1));
        r.dv.tabRowMouseForTests().mouseDown(mouseEvent(c, c, kLeft, 2));
        r.dv.tabRowMouseForTests().mouseDoubleClick(mouseEvent(c, c, kLeft, 2));
    }
    CHECK(ed->isVisible());
    CHECK(ed->getText() == "Intro");
    CHECK(r.renames.empty());
    CHECK(r.closes == 0);
}

TEST_CASE("DeckView rename (e): Return / Tab / Shift+Tab / click-away / focus loss keep, Esc discards -- all synchronous", "[decktabs][rename]")
{
    SECTION("Return with \"  Intro  \" renames deck 1 to \"Intro\" once, closes, hands focus home once")
    {
        Rig r;
        doubleClick(r.dv, "B");
        r.dv.renameEditorForTests()->setText("  Intro  ", false);
        r.key(kReturn);
        REQUIRE(r.renames.size() == 1);
        CHECK(r.renames[0].first == 1);
        CHECK(r.renames[0].second == "Intro");
        CHECK_FALSE(r.dv.renameEditorForTests()->isVisible());
        CHECK_FALSE(r.dv.isRenaming());
        CHECK(r.closes == 1);
        CHECK(r.closedBeforeHide());
    }
    SECTION("Esc discards: no rename, closed, focus home once")
    {
        Rig r;
        doubleClick(r.dv, "B");
        r.dv.renameEditorForTests()->setText("Intro", false);
        r.key(kEsc);
        CHECK(r.renames.empty());
        CHECK_FALSE(r.dv.renameEditorForTests()->isVisible());
        CHECK(r.closes == 1);
        CHECK(r.closedBeforeHide());
    }
    SECTION("an empty name or the unchanged name keeps the old one (no rename)")
    {
        Rig r;
        doubleClick(r.dv, "B");
        r.dv.renameEditorForTests()->setText("   ", false);
        r.key(kReturn);
        doubleClick(r.dv, "B");
        r.dv.renameEditorForTests()->setText("B", false);
        r.key(kReturn);
        CHECK(r.renames.empty());
        CHECK(r.closes == 2);
        CHECK(r.closedBeforeHide());
    }
    SECTION("Tab and Shift+Tab keep")
    {
        Rig r;
        doubleClick(r.dv, "B");
        r.dv.renameEditorForTests()->setText("T1", false);
        r.key(kTab);
        doubleClick(r.dv, "B");
        r.dv.renameEditorForTests()->setText("T2", false);
        r.key(kShiftTab);
        REQUIRE(r.renames.size() == 2);
        CHECK(r.renames[0].second == "T1");
        CHECK(r.renames[1].second == "T2");
        CHECK(r.closes == 2);
        CHECK(r.closedBeforeHide());
    }
    SECTION("a press on a clip cell keeps; a press on the box or one of its children leaves it open")
    {
        Rig r;
        doubleClick(r.dv, "B");
        auto* ed = r.dv.renameEditorForTests();
        ed->setText("Cell", false);
        r.dv.tabRowMouseForTests().mouseDown(mouseEvent(ed, ed, kLeft, 1));
        REQUIRE(ed->getNumChildComponents() > 0);
        auto* inner = ed->getChildComponent(0);
        r.dv.tabRowMouseForTests().mouseDown(mouseEvent(inner, inner, kLeft, 1));
        CHECK(ed->isVisible());
        CHECK(r.renames.empty());
        auto* cell = firstClipCell(r.dv);
        REQUIRE(cell != nullptr);
        r.dv.tabRowMouseForTests().mouseDown(mouseEvent(cell, cell, kLeft, 1));
        CHECK_FALSE(ed->isVisible());
        REQUIRE(r.renames.size() == 1);
        CHECK(r.renames[0] == std::make_pair(1, juce::String("Cell")));
        CHECK(r.closes == 1);
        CHECK(r.closedBeforeHide());
    }
    SECTION("the box's focus-loss callback while it has no focus keeps")
    {
        Rig r;
        doubleClick(r.dv, "B");
        auto* ed = r.dv.renameEditorForTests();
        ed->setText("Lost", false);
        REQUIRE(ed->onFocusLost != nullptr);
        ed->onFocusLost();
        CHECK_FALSE(ed->isVisible());
        REQUIRE(r.renames.size() == 1);
        CHECK(r.renames[0].second == "Lost");
        CHECK(r.closes == 1);
        CHECK(r.closedBeforeHide());
    }
}

TEST_CASE("DeckView rename (f): a rebuild keeps the box on its deck, on top; the deck removed -> closed, no rename", "[decktabs][rename]")
{
    Rig r(false);
    r.dv.beginRename(2);
    auto* ed = r.dv.renameEditorForTests();
    REQUIRE(ed->isVisible());
    ed->setText("Zed", false);

    r.comp.activeDeckIndex = 0;                            // a non-UI switch (REST / MIDI / genre) rebuilds the row
    r.dv.rebuildGrid();
    CHECK(r.dv.isRenaming());
    CHECK(r.dv.renamingDeckIndex() == 2);
    CHECK(ed->isVisible());
    CHECK(ed->getText() == "Zed");
    CHECK(ed->getX() == tabNamed(r.dv, "C")->getX());
    CHECK(ed->getBounds() == expectedBox(r.dv, "C"));
    CHECK(r.dv.getChildComponent(r.dv.getNumChildComponents() - 1) == ed);

    CHECK_FALSE(r.comp.retireOrEraseDeck(2));              // erased, not retired: no layer plays deck C (lane bf9b)
    REQUIRE(r.comp.decks.size() == 2);
    r.dv.rebuildGrid();
    CHECK_FALSE(r.dv.isRenaming());
    CHECK_FALSE(ed->isVisible());
    CHECK(r.renames.empty());
    CHECK(r.closes == 1);
    CHECK(r.closedBeforeHide());
}

TEST_CASE("DeckView rename (g): key-ups are swallowed; the output keys pass through and leave the box open; plain Esc discards", "[decktabs][rename]")
{
    Rig r;
    doubleClick(r.dv, "B");
    auto* ed = r.dv.renameEditorForTests();
    REQUIRE(ed->isVisible());
    CHECK(ed->keyStateChanged(false));
    const juce::KeyPress panic(juce::KeyPress::escapeKey,
                               juce::ModifierKeys(juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier), 0);
    const juce::KeyPress raise('`', juce::ModifierKeys::commandModifier, 0);
    const juce::KeyPress mainOut('F', juce::ModifierKeys::commandModifier, 0);
    CHECK_FALSE(ed->keyPressed(panic));
    CHECK_FALSE(ed->keyPressed(raise));
    CHECK_FALSE(ed->keyPressed(mainOut));
    CHECK(ed->isVisible());
    CHECK(r.dv.isRenaming());
    CHECK(ed->keyPressed(kEsc));
    CHECK_FALSE(ed->isVisible());
    CHECK(r.renames.empty());
}

TEST_CASE("DeckView rename (h): only the showing tab's tooltip says \"Double-click: rename\"; every tab keeps the right-click line", "[decktabs][rename]")
{
    Rig r;
    CHECK(tabNamed(r.dv, "B")->getTooltip().contains("Double-click: rename"));
    CHECK_FALSE(tabNamed(r.dv, "A")->getTooltip().contains("Double-click: rename"));
    CHECK_FALSE(tabNamed(r.dv, "C")->getTooltip().contains("Double-click: rename"));
    for (const char* name : { "A", "B", "C" })
        CHECK(tabNamed(r.dv, name)->getTooltip().endsWith("Right-click: Save / Rename / Duplicate / Remove"));

    r.comp.activeDeckIndex = 0;   // refresh() follows the showing deck too
    r.dv.refresh();
    CHECK(tabNamed(r.dv, "A")->getTooltip().contains("Double-click: rename"));
    CHECK_FALSE(tabNamed(r.dv, "B")->getTooltip().contains("Double-click: rename"));
}

TEST_CASE("DeckView rename (i): cancelDeckRename while open closes, renames nothing, hands focus home once", "[decktabs][rename]")
{
    Rig r;
    doubleClick(r.dv, "B");
    r.dv.renameEditorForTests()->setText("Gone", false);
    r.dv.cancelDeckRename();
    CHECK_FALSE(r.dv.isRenaming());
    CHECK_FALSE(r.dv.renameEditorForTests()->isVisible());
    CHECK(r.renames.empty());
    CHECK(r.closes == 1);
    CHECK(r.closedBeforeHide());
    r.dv.cancelDeckRename();                              // closed already: nothing
    CHECK(r.closes == 1);
}

TEST_CASE("DeckView rename (j): the box's tooltip names its keys, and it has no popup menu", "[decktabs][rename]")
{
    Rig r;
    auto* ed = r.dv.renameEditorForTests();
    CHECK(ed->getTooltip() == "Enter keeps the new name, Esc cancels");
    CHECK_FALSE(ed->isPopupMenuEnabled());
}

TEST_CASE("DeckView rename (k): focus held OUTSIDE DeckView stays there (no onRenameClosed); inside DeckView or nowhere goes home", "[decktabs][rename]")
{
    // Headless, nothing can hold keyboard focus, so the close reads it through the test seam. The real case: Boris
    // clicks into the BPM field or a browser search box while the box is open -> the box's (posted) focus loss keeps
    // the name, and focus must be LEFT in that field (ruling AM4: "If focus is elsewhere ... it is left there").
    Rig r;
    juce::Component elsewhere;                            // not inside DeckView (e.g. the TopBar BPM field)
    juce::Component* focus = &elsewhere;
    r.dv.setFocusedComponentForTests([&focus] { return focus; });

    doubleClick(r.dv, "B");
    auto* ed = r.dv.renameEditorForTests();
    ed->setText("Kept", false);
    REQUIRE(ed->onFocusLost != nullptr);
    ed->onFocusLost();
    CHECK_FALSE(ed->isVisible());
    REQUIRE(r.renames.size() == 1);
    CHECK(r.renames[0].second == "Kept");
    CHECK(r.closes == 0);

    focus = ed;                                           // inside DeckView: the box itself
    doubleClick(r.dv, "B");
    r.key(kEsc);
    CHECK(r.closes == 1);

    focus = tabNamed(r.dv, "A");                          // inside DeckView: a tab button
    doubleClick(r.dv, "B");
    r.key(kEsc);
    CHECK(r.closes == 2);

    focus = nullptr;                                      // nowhere
    doubleClick(r.dv, "B");
    r.key(kEsc);
    CHECK(r.closes == 3);
    CHECK(r.closedBeforeHide());
}
