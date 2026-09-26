// s-rta-0925 (lane "rclick"): right-click reset does nothing on ALL
// Composition-tab sliders (diag-opacity.md item 3 / plan-opacity.md Lane C).
// Written fail-first against ResettableSlider's HEAD behaviour plus the
// stubs added in this same commit (onResetToDefault/resetToDefault/
// childRightClickResets — no-ops). RC1 is the one PIN: it passes on HEAD
// already (ResettableSlider::mouseDown already resets an armed slider).
// Headless JUCE widgets under ScopedJuceInitialiser_GUI (no window, no
// peer) — mouse events are synthesised and delivered directly to
// mouseDown(), never through the real OS/event dispatch.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/UniversalParamControl.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ConnClock.h"

using Catch::Approx;

// A synthesised mouse-down as JUCE's own dispatcher would build it
// (juce_Component.cpp HierarchyChecker::eventWithNearestParent): eventComponent
// = originator = the component the click landed on.
static juce::MouseEvent clickOn(juce::Component& target, bool rightButton)
{
    const juce::ModifierKeys mods(rightButton ? juce::ModifierKeys::rightButtonModifier
                                              : juce::ModifierKeys::leftButtonModifier);
    const auto now = juce::Time::getCurrentTime();
    return { juce::Desktop::getInstance().getMainMouseSource(), {2.0f, 2.0f}, mods,
             0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, {2.0f, 2.0f}, now, 1, false };
}

template <typename T> static T* firstChildOfType(juce::Component& parent)
{
    for (int i = 0; i < parent.getNumChildComponents(); ++i)
        if (auto* c = dynamic_cast<T*>(parent.getChildComponent(i))) return c;
    return nullptr;
}

TEST_CASE("ResettableSlider: right-click on its own area resets to the armed default", "[rclick]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ResettableSlider s;
    s.setRange(0, 1, 0.01);
    s.setDefaultValue(0.7);
    s.setValue(0.2, juce::dontSendNotification);
    s.mouseDown(clickOn(s, true));
    REQUIRE(s.getValue() == Approx(0.7));
}

TEST_CASE("ResettableSlider: onResetToDefault fires once, after onValueChange", "[rclick]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ResettableSlider s;
    s.setRange(0, 1, 0.01);
    s.setDefaultValue(0.7);
    s.setValue(0.2, juce::dontSendNotification);

    int order = 0, seenValue = 0, seenReset = 0;
    s.onValueChange = [&] { seenValue = ++order; };
    s.onResetToDefault = [&] { seenReset = ++order; };

    s.mouseDown(clickOn(s, true));
    REQUIRE(seenValue == 1);
    REQUIRE(seenReset == 2);

    // Second right-click: value already at default, JUCE sends no value
    // notification, but onResetToDefault must still fire.
    int resetBefore = seenReset;
    int valueBefore = seenValue;
    s.mouseDown(clickOn(s, true));
    REQUIRE(seenReset > resetBefore);
    REQUIRE(seenValue == valueBefore);
}

TEST_CASE("ResettableSlider: a right-click relayed from the IncDec text box resets; from a +/- button or the slider itself it does not", "[rclick]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ResettableSlider s;
    s.setSliderStyle(juce::Slider::IncDecButtons);
    s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
    s.lookAndFeelChanged();
    s.setRange(1, 99, 1);
    s.setDefaultValue(1);
    s.setValue(5, juce::dontSendNotification);

    auto* box = firstChildOfType<juce::Label>(s);
    auto* btn = firstChildOfType<juce::Button>(s);
    REQUIRE(box != nullptr);
    REQUIRE(btn != nullptr);

    REQUIRE(ResettableSlider::childRightClickResets(clickOn(*box, true), s) == true);
    REQUIRE(ResettableSlider::childRightClickResets(clickOn(*btn, true), s) == false);
    REQUIRE(ResettableSlider::childRightClickResets(clickOn(s, true), s) == false);
    REQUIRE(ResettableSlider::childRightClickResets(clickOn(*box, false), s) == false);

    s.resetToDefault();
    REQUIRE(s.getValue() == Approx(1.0));
}

TEST_CASE("UniversalParamControl: the inner slider is armed without any setDefaultValue call", "[rclick]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    UniversalParamControl c;
    c.setParamValue(0.2f);
    auto* inner = firstChildOfType<ResettableSlider>(c);
    REQUIRE(inner != nullptr);
    inner->mouseDown(clickOn(*inner, true));
    REQUIRE(c.getParamValue() == Approx(0.5f));
}

TEST_CASE("UniversalParamControl: a right-click reset touches a bound connection (Decaying grip)", "[rclick]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Signal;
    LiveValue live;

    UniversalParamControl c;
    c.bindConnection(&conn, &live);
    c.setDefaultValue(1.0f);
    c.setParamValue(0.3f);

    auto* inner = firstChildOfType<ResettableSlider>(c);
    REQUIRE(inner != nullptr);
    inner->mouseDown(clickOn(*inner, true));

    REQUIRE(conn.grip.kind == ParamConnection::Grip::Kind::Decaying);
    REQUIRE(c.getParamValue() == Approx(1.0f));
}
