// s-rta-0926b bpm2: the TopBar "Link" toggle in a DEFAULT build (AUDIODNA_BUILD_LINK
// OFF -- this test target never defines AUDIODNA_HAS_LINK, like the default app build).
// Ableton Link is not compiled in, so the toggle must be visibly disabled (not clickable,
// dimmed like the Record panel's disabled controls), explain why in its tooltip, and keep
// its exact place in the bar (no layout change). Pre-fix it was a live, clickable toggle
// whose tooltip promised network sync. Headless JUCE widgets, same harness as
// test_master_signal_link.cpp. The toggle is found as TopBar's child ToggleButton "Link",
// so the test needs no test seam.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/TopBar.h"
#include "features/FeatureBus.h"
#include "model/Composition.h"

namespace
{
    juce::ToggleButton* findLinkToggle(juce::Component& bar)
    {
        for (auto* child : bar.getChildren())
            if (auto* toggle = dynamic_cast<juce::ToggleButton*>(child))
                if (toggle->getButtonText() == "Link")
                    return toggle;
        return nullptr;
    }

    juce::MouseEvent leftClickOn(juce::Component& target)
    {
        const juce::ModifierKeys mods(juce::ModifierKeys::leftButtonModifier);
        const auto now = juce::Time::getCurrentTime();
        return { juce::Desktop::getInstance().getMainMouseSource(), {8.0f, 8.0f}, mods,
                 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, {8.0f, 8.0f}, now, 1, false };
    }
}

TEST_CASE("default build: the TopBar Link toggle is disabled, dimmed, says Link is not in this "
          "build, and keeps its place in the bar",
          "[ableton-link][topbar][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    bar.setSize(1728, 40);   // the maximized-window width used by test_master_signal_link.cpp

    auto* link = findLinkToggle(bar);
    REQUIRE(link != nullptr);
    INFO("bounds " << link->getBounds().toString() << ", enabled " << link->isEnabled()
         << ", alpha " << link->getAlpha() << ", tooltip \"" << link->getTooltip() << "\"");

    CHECK(link->isVisible());
    CHECK_FALSE(link->isEnabled());                                               // RED pre-fix: enabled
    CHECK(link->getAlpha() < 1.0f);                                               // RED pre-fix: 1.0 (not dimmed)
    CHECK(link->getTooltip() == "Ableton Link is not included in this build");  // RED pre-fix: the sync promise
    REQUIRE_FALSE(link->getToggleState());
    // Same bounds as before the change: read from the pre-change TopBar at 1728 x 40
    // (s-rta-0926b bpm2 RED run on main 5f84899: "bounds 682 2 50 36").
    CHECK(link->getBounds() == juce::Rectangle<int>(682, 2, 50, 36));

    // A left click does nothing: no toggle, no onLinkToggled (which would reach LinkSync).
    int fired = 0;
    bar.onLinkToggled = [&fired](bool) { ++fired; };
    juce::Component& asComponent = *link;   // Button's mouse overrides are protected; Component's are public
    asComponent.mouseDown(leftClickOn(*link));
    asComponent.mouseUp(leftClickOn(*link));
    CHECK(fired == 0);                // RED pre-fix: 1 (the click toggled Link on)
    REQUIRE_FALSE(link->getToggleState());
}
