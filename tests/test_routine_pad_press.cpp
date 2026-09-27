// test_routine_pad_press -- s-rta-0927 routine display, slice A (plan-routine-display-A.md 2.1 / 5.4): a routine
// pad in the deck's ROUTINES row has ONE press action, Fire -- a left press fires (the engine restarts a playing
// routine and ignores a waiting one), a right-click (isPopupMenu) opens the settings menu and never fires, an
// empty pad does nothing. The old Record-tab grammar (press a playing pad = stop) cannot come back: RoutinePad
// has no stop callback (a compile-level pin). Headless JUCE widgets under ScopedJuceInitialiser_GUI; mouse
// events are synthesised and delivered by calling mouseDown() directly.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/RoutinePad.h"

// (a dependent requires-expression: a non-dependent one would be a hard error, not `false`)
template <typename T> concept HasStopCallback = requires(T& p) { p.onStop; };
static_assert(!HasStopCallback<RoutinePad>, "a routine pad has one press action: Fire");

namespace
{
    juce::MouseEvent pressOn(juce::Component& target, juce::ModifierKeys mods)
    {
        const auto now = juce::Time::getCurrentTime();
        return { juce::Desktop::getInstance().getMainMouseSource(), {8.0f, 8.0f}, mods,
                 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, {8.0f, 8.0f}, now, 1, false };
    }

    RoutineDeckView::Pad spec(RoutineDeckView::State st)
    {
        RoutineDeckView::Pad p;
        p.number = 2;
        p.name = "Build";
        p.state = st;
        p.bar = 1;
        p.barsTotal = 4;
        p.progress01 = 0.1f;
        return p;
    }

    struct Counts { int fire = 0, menu = 0, lastSlot = -1; };

    void wire(RoutinePad& pad, Counts& c)
    {
        pad.onFire = [&c](int slot) { ++c.fire; c.lastSlot = slot; };
        pad.onContextMenu = [&c](int slot) { ++c.menu; c.lastSlot = slot; };
    }
}

TEST_CASE("RoutinePad: a left press on a playing pad fires (restart) and never opens the menu", "[routine][pad]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    RoutinePad pad(1);
    pad.setSize(90, 22);
    pad.setVisible(true);   // Pitfall 34
    pad.setSpec(spec(RoutineDeckView::State::Playing));
    Counts c;
    wire(pad, c);
    pad.mouseDown(pressOn(pad, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier)));
    CHECK(c.fire == 1);
    CHECK(c.menu == 0);
    CHECK(c.lastSlot == 1);
}

TEST_CASE("RoutinePad: a right-click opens the settings menu and never fires", "[routine][pad]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    RoutinePad pad(1);
    pad.setSize(90, 22);
    pad.setVisible(true);
    pad.setSpec(spec(RoutineDeckView::State::Playing));
    Counts c;
    wire(pad, c);
    pad.mouseDown(pressOn(pad, juce::ModifierKeys(juce::ModifierKeys::popupMenuClickModifier)));
    CHECK(c.menu == 1);
    CHECK(c.fire == 0);
}

TEST_CASE("RoutinePad: an empty pad does nothing on a press or a right-click", "[routine][pad]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    RoutinePad pad(4);
    pad.setSize(90, 22);
    pad.setVisible(true);
    pad.setSpec(spec(RoutineDeckView::State::Empty));
    Counts c;
    wire(pad, c);
    pad.mouseDown(pressOn(pad, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier)));
    pad.mouseDown(pressOn(pad, juce::ModifierKeys(juce::ModifierKeys::popupMenuClickModifier)));
    CHECK(c.fire == 0);
    CHECK(c.menu == 0);
}

TEST_CASE("RoutinePad: a press on a waiting pad fires too (the engine makes it a no-op)", "[routine][pad]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    RoutinePad pad(0);
    pad.setSize(90, 22);
    pad.setVisible(true);
    pad.setSpec(spec(RoutineDeckView::State::Waiting));
    Counts c;
    wire(pad, c);
    pad.mouseDown(pressOn(pad, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier)));
    CHECK(c.fire == 1);
    CHECK(c.menu == 0);
}

TEST_CASE("RoutinePad: an idle pad fires; its tooltip is the view's", "[routine][pad]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    RoutinePad pad(2);
    pad.setSize(90, 22);
    pad.setVisible(true);
    auto s = spec(RoutineDeckView::State::Idle);
    s.tooltip = "Press to play. Right-click for settings.";
    pad.setSpec(s);
    CHECK(pad.getTooltip() == "Press to play. Right-click for settings.");
    CHECK(pad.getComponentID() == "routinePad2");
    Counts c;
    wire(pad, c);
    pad.mouseDown(pressOn(pad, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier)));
    CHECK(c.fire == 1);
}
