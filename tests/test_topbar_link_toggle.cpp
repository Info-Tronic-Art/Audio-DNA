// s-rta-0926b bpm2: the TopBar "Link" toggle in a DEFAULT build (AUDIODNA_BUILD_LINK
// OFF -- this test target never defines AUDIODNA_HAS_LINK, like the default app build).
// Ableton Link is not compiled in, so the toggle must be visibly disabled (not clickable,
// dimmed like the Record panel's disabled controls), explain why in its tooltip, and keep
// its exact place in the bar (no layout change). Pre-fix it was a live, clickable toggle
// whose tooltip promised network sync. Headless JUCE widgets, same harness as
// test_master_signal_link.cpp. The toggle is found as TopBar's child ToggleButton "Link",
// so the test needs no test seam.
//
// s-rta-0926b uitoggle: the dimming moved from TopBar's own Component::setAlpha(0.4)
// (removed -- it would now double-dim on top of the LookAndFeel's own dimming) into
// AudioDNALookAndFeel::drawToggleButton itself, so the pixel check below renders the
// toggle with the app's real LookAndFeel and samples the box-fill pixel's alpha channel
// instead of asking Component::getAlpha() (which the toggle itself no longer touches).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/TopBar.h"
#include "ui/LookAndFeel.h"
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
    AudioDNALookAndFeel laf;
    juce::LookAndFeel::setDefaultLookAndFeel(&laf);   // the app's real toggle dimming lives here

    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    bar.setSize(1728, 40);   // the maximized-window width used by test_master_signal_link.cpp

    auto* link = findLinkToggle(bar);
    REQUIRE(link != nullptr);

    // Box-fill pixel, unchecked toggle: drawToggleButton fills toggleBounds (local x:[4,20]
    // y:[10,26] for a 36-tall button) with kSurface -- fully opaque when enabled, multiplied
    // by kDisabledAlpha (0.4) when disabled. Sample its centre (12,18), away from the
    // anti-aliased border/corners, via a headless snapshot (Component::createComponentSnapshot,
    // same idiom as tool_routine_strip_snapshot.cpp).
    const auto snapshot = link->createComponentSnapshot(link->getLocalBounds());
    const float fillAlpha = snapshot.getPixelAt(12, 18).getFloatAlpha();
    INFO("bounds " << link->getBounds().toString() << ", enabled " << link->isEnabled()
         << ", fillAlpha " << fillAlpha << ", tooltip \"" << link->getTooltip() << "\"");

    CHECK(link->isVisible());
    CHECK_FALSE(link->isEnabled());                                               // RED pre-fix: enabled
    CHECK(fillAlpha < 0.6f);                    // RED pre-fix: ~1.0 (opaque -- pixel-identical to enabled)
    CHECK(fillAlpha > 0.05f);                   // sanity: still drawn, not invisible
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

    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}
