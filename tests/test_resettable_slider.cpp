// tests/test_resettable_slider.cpp -- s-rta-0925 rclick2 lane (reconcile).
//
// Ports lane/0925-rclick's tests/test_resettable_slider.cpp (branch lane/0925-rclick,
// commit 622606b, never merged) onto main's ACTUAL ResettableSlider design, which
// landed via a different lane (opacity, commits a1be09b/41055a5) with a different
// shape: a public static ResettableSlider::childRightClickResets(event, owner)
// decision function + a private ChildRelay listener, instead of rclick's private
// ChildListener that forwards every non-Button child click into mouseDown() and
// decides there via an eventComponent-vs-this check. Per case:
//
//   1 "direct right-click resets, notifies once, no drag"      -> already GREEN on
//     main unmodified (ResettableSlider::mouseDown right-click path predates this
//     lane). Ported as a plain pin.
//   2 "un-armed right-click is swallowed (no jump/drag)"       -> GREEN once
//     hasDefaultValue() exists (main's resetToDefault() already no-ops when
//     un-armed; only the query accessor rclick added was missing). RED-before:
//     the ported assertion below fails to compile against ResettableSlider without
//     it -- see the same commit that adds it to UniversalParamControl.h.
//   3 "right-click from the IncDec text box resets"            -> already covered
//     on main by test_right_click_reset.cpp's childRightClickResets assertions;
//     ported here too (adapted to the gate idiom) as a same-file pin.
//   4 "right-click from a +/- Button does NOT reset"           -> named in the
//     rclick2 work packet as a case the opacity lane might not cover. Verified
//     GREEN already: childRightClickResets() explicitly excludes juce::Button
//     children. Ported as a value-level pin (not just the boolean decision).
//   5 "left-click from a child never starts a drag"            -> named in the
//     rclick2 work packet as a case the opacity lane might not cover. Verified
//     GREEN by construction: the ONLY path from a child event to the owning
//     slider is `if (childRightClickResets(e, owner)) owner.resetToDefault();`
//     (ChildRelay::mouseDown) -- childRightClickResets requires
//     isRightButtonDown(), so a left-click from a child can never reach
//     resetToDefault(), let alone Slider::mouseDown() (the only path that could
//     ever fire onDragStart). Ported as a dragStarts==0 regression pin exercising
//     the real gate idiom across every child/button-state combination.
//   6 "onResetToDefault fires once, after onValueChange"       -> same contract,
//     already covered by test_right_click_reset.cpp's own case of the same name;
//     not re-ported here (would duplicate, not port a missing behaviour).
//   7 "resetToDefault() on an un-armed slider is a no-op"      -> already GREEN
//     (hasDefault_ guard predates this lane). Ported as a plain pin.
//   8 "hook-less reset notifies once, not when already at default" -> already
//     GREEN (JUCE's Slider::setValue skips notification for an unchanged value).
//     Ported as a plain pin.
//
// Header-only class (src/ui/UniversalParamControl.h) -- no src/*.cpp is linked.
// Same per-TEST_CASE juce::ScopedJuceInitialiser_GUI local-variable pattern as
// tests/test_right_click_reset.cpp (construct-and-tear-down inside each
// TEST_CASE, never a function-local static): a function-local-static
// ScopedJuceInitialiser_GUI was found (lane/0925-rclick, see .harmony/notebook.md
// "juce_gui_basics ctest target: function-local-static ScopedJuceInitialiser_GUI
// corrupts the heap at exit") to abort inside juce::DeletedAtShutdown::deleteAll()
// when ctest's catch_discover_tests runs a single TEST_CASE alone in its own
// process. The per-TEST_CASE local pattern used here and in
// test_right_click_reset.cpp sidesteps that hazard entirely (deterministic
// teardown before the TEST_CASE returns, never via atexit) and needs no custom
// main() -- links Catch2::Catch2WithMain, consistent with the rest of the suite.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/UniversalParamControl.h"

using Catch::Approx;

namespace
{
// A synthesised mouse-down as JUCE's own dispatcher would build it
// (juce_Component.cpp HierarchyChecker::eventWithNearestParent): eventComponent
// = originator = the component the click landed on.
juce::MouseEvent clickOn(juce::Component& target, bool rightButton)
{
    const juce::ModifierKeys mods(rightButton ? juce::ModifierKeys::rightButtonModifier
                                              : juce::ModifierKeys::leftButtonModifier);
    const auto now = juce::Time::getCurrentTime();
    return { juce::Desktop::getInstance().getMainMouseSource(), {2.0f, 2.0f}, mods,
             0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, {2.0f, 2.0f}, now, 1, false };
}

template <typename T> T* firstChildOfType(juce::Component& parent)
{
    for (int i = 0; i < parent.getNumChildComponents(); ++i)
        if (auto* c = dynamic_cast<T*>(parent.getChildComponent(i))) return c;
    return nullptr;
}

// IncDecButtons + TextBoxLeft: the shape of the Composition-tab numeric sliders
// (CompositionInspector.cpp:50-51, 92-93). Children: one Label (text box), two
// Buttons ([-] and [+]).
struct IncDec
{
    ResettableSlider s;
    juce::Label* label = nullptr;
    juce::Button* button = nullptr;
    int notifies = 0, dragStarts = 0;

    IncDec()
    {
        s.setSliderStyle(juce::Slider::IncDecButtons);
        s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
        s.setRange(1, 64, 1);
        s.setValue(8, juce::dontSendNotification);
        s.setDefaultValue(16);
        s.onValueChange = [this] { ++notifies; };
        s.onDragStart = [this] { ++dragStarts; };
        for (int i = 0; i < s.getNumChildComponents(); ++i)
        {
            auto* c = s.getChildComponent(i);
            if (auto* l = dynamic_cast<juce::Label*>(c)) label = l;
            if (auto* b = dynamic_cast<juce::Button*>(c)) button = b;
        }
        REQUIRE(label != nullptr);
        REQUIRE(button != nullptr);
    }

    // The exact one-line consumer idiom ResettableSlider::ChildRelay::mouseDown
    // runs in production for a mouse-down relayed from a nested child. Used here
    // to exercise the real gate (a public static function) without needing
    // access to the private ChildRelay listener itself.
    void relay(const juce::MouseEvent& e)
    {
        if (ResettableSlider::childRightClickResets(e, s)) s.resetToDefault();
    }
}; // namespace
} // namespace

TEST_CASE("ResettableSlider rclick2-1: direct right-click resets an armed slider and notifies once", "[rclick2][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    IncDec f;
    f.s.mouseDown(clickOn(f.s, true));
    CHECK(f.s.getValue() == 16.0);
    CHECK(f.notifies == 1);
    CHECK(f.dragStarts == 0);
}

TEST_CASE("ResettableSlider rclick2-2: un-armed right-click is swallowed (no jump, no drag)", "[rclick2][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ResettableSlider u;   // never armed -- stock juce::Slider would drag-jump here
    u.setSliderStyle(juce::Slider::LinearHorizontal);
    u.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    u.setRange(0.0, 1.0, 0.001);
    u.setValue(0.3, juce::dontSendNotification);
    u.setBounds(0, 0, 200, 20);
    int drags = 0; u.onDragStart = [&] { ++drags; };
    u.mouseDown(clickOn(u, true));
    CHECK(u.getValue() == Approx(0.3));
    CHECK(drags == 0);
    CHECK_FALSE(u.hasDefaultValue());
}

TEST_CASE("ResettableSlider rclick2-3: right-click relayed from the IncDec text box resets", "[rclick2][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    IncDec f;
    f.relay(clickOn(*f.label, true));
    CHECK(f.s.getValue() == 16.0);
    CHECK(f.notifies == 1);
}

TEST_CASE("ResettableSlider rclick2-4: right-click relayed from a +/- Button does not reset", "[rclick2][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    IncDec f;
    f.relay(clickOn(*f.button, true));
    CHECK(f.s.getValue() == 8.0);   // unchanged -- Button children are excluded
    CHECK(f.notifies == 0);
}

TEST_CASE("ResettableSlider rclick2-5: a left-click relayed from any child never starts a drag", "[rclick2][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    IncDec f;
    // Every combination the real ChildRelay ever sees a mouse-down for: left and
    // right clicks on the text box and on the +/- button. Only 3-2 (right-click,
    // text box) resets; none of the four ever calls Slider::mouseDown(), so
    // dragStarts must stay 0 throughout.
    f.relay(clickOn(*f.label, false));
    CHECK(f.dragStarts == 0);
    f.relay(clickOn(*f.button, false));
    CHECK(f.dragStarts == 0);
    f.relay(clickOn(*f.button, true));
    CHECK(f.dragStarts == 0);
    CHECK(f.s.getValue() == 8.0);   // still unmoved by any of the above
}

TEST_CASE("ResettableSlider rclick2-7: resetToDefault() on an un-armed slider is a no-op", "[rclick2][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ResettableSlider u;
    u.setRange(0.0, 1.0, 0.001);
    u.setValue(0.3, juce::dontSendNotification);
    int hooks = 0; u.onResetToDefault = [&] { ++hooks; };
    u.resetToDefault();
    CHECK(u.getValue() == Approx(0.3));
    CHECK(hooks == 0);
}

TEST_CASE("ResettableSlider rclick2-8: hook-less reset notifies once, and not when already at default", "[rclick2][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    IncDec f;
    f.s.resetToDefault();
    CHECK(f.notifies == 1);
    f.s.resetToDefault();   // juce::Slider::setValue skips equal values -> no second notification
    CHECK(f.notifies == 1);
}
