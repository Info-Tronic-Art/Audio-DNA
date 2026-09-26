// s-rta-0925 mastersignal Step 1: the TopBar Signal fader is a second
// widget-grip view of Composition::masterSignal / CompScalar::Signal, built
// exactly like TopBar's own Master fader (test_master_opacity_link.cpp's
// six cases, reproduced here against getMasterSignalSlider()/masterSignal/
// CompScalar::Signal), plus a check that the two faders are independent and
// a layout check that "Signal:" sits immediately left of "Master:" with no
// overlap. Headless JUCE widgets, same harness as test_right_click_reset.cpp;
// the timer read-back is exercised by calling
// TopBar::syncMasterSignalFromComposition() directly (no dispatch loop run).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/TopBar.h"
#include "features/FeatureBus.h"
#include "model/Composition.h"
#include "connect/ScalarParams.h"
#include <cmath>

using Catch::Approx;

static juce::MouseEvent clickOn(juce::Component& target, bool rightButton)
{
    const juce::ModifierKeys mods(rightButton ? juce::ModifierKeys::rightButtonModifier
                                              : juce::ModifierKeys::leftButtonModifier);
    const auto now = juce::Time::getCurrentTime();
    return { juce::Desktop::getInstance().getMainMouseSource(), {2.0f, 2.0f}, mods,
             0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, {2.0f, 2.0f}, now, 1, false };
}

namespace
{
    constexpr auto kSig = static_cast<size_t>(CompScalar::Signal);
}

TEST_CASE("Signal fader write lands on Composition::masterSignal", "[link][signal]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterSignalSlider();

    fader.setValue(0.3, juce::sendNotificationSync);

    REQUIRE(comp.masterSignal == Approx(0.3f).margin(0.006));
    REQUIRE(comp.scalarConns[kSig].grip.kind == ParamConnection::Grip::Kind::None);
}

TEST_CASE("Signal fader drag grips Held on CompScalar::Signal and releases on drag end", "[link][signal]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterSignalSlider();

    REQUIRE(bool(fader.onDragStart));
    REQUIRE(bool(fader.onDragEnd));

    fader.onDragStart();
    REQUIRE(comp.scalarConns[kSig].grip.kind == ParamConnection::Grip::Kind::Held);

    fader.onDragEnd();
    REQUIRE(comp.scalarConns[kSig].grip.kind == ParamConnection::Grip::Kind::None);
}

TEST_CASE("Signal sync: not connected -> fader shows the manual field", "[link][signal]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterSignalSlider();

    comp.masterSignal = 0.8f;
    bar.syncMasterSignalFromComposition();

    REQUIRE(fader.getValue() == Approx(0.8).margin(0.006));
    REQUIRE(comp.masterSignal == Approx(0.8f)); // sync never writes back
}

TEST_CASE("Signal sync: connected -> fader shows toNorm(eff()), not the manual field", "[link][signal]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterSignalSlider();

    comp.scalarConns[kSig].source.kind = ConnSource::Kind::Signal;
    comp.scalarLive[kSig].v.store(0.25f);
    comp.masterSignal = 1.0f;
    bar.syncMasterSignalFromComposition();
    REQUIRE(fader.getValue() == Approx(0.25).margin(0.006));

    // Gripped: the engine publishes NaN (twin), so eff() falls back to the
    // manual field -- the fader shows the manual value while gripped.
    comp.scalarConns[kSig].gripHeld();
    comp.scalarLive[kSig].v.store(std::nanf(""));
    comp.masterSignal = 0.6f;
    bar.syncMasterSignalFromComposition();
    REQUIRE(fader.getValue() == Approx(0.6).margin(0.006));
}

TEST_CASE("Signal sync: the thumb never moves while the fader is being dragged", "[link][signal]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterSignalSlider();

    fader.setValue(0.5, juce::sendNotificationSync);
    fader.onDragStart();
    comp.masterSignal = 0.1f;
    bar.syncMasterSignalFromComposition();
    REQUIRE(fader.getValue() == Approx(0.5).margin(0.006));

    fader.onDragEnd();
    bar.syncMasterSignalFromComposition();
    REQUIRE(fader.getValue() == Approx(0.1).margin(0.006));
}

TEST_CASE("right-click on the Signal fader resets to 1.0 on the model and touches the grip", "[link][signal]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterSignalSlider();

    fader.setValue(0.4, juce::sendNotificationSync);
    fader.mouseDown(clickOn(fader, true));

    REQUIRE(comp.masterSignal == Approx(1.0f));
    REQUIRE(comp.scalarConns[kSig].grip.kind == ParamConnection::Grip::Kind::Decaying);
}

TEST_CASE("the Signal and Master faders are independent", "[link][signal]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);

    bar.getMasterSignalSlider().setValue(0.3, juce::sendNotificationSync);

    REQUIRE(comp.masterSignal == Approx(0.3f).margin(0.006));
    REQUIRE(comp.masterOpacity == Approx(1.0f).margin(0.006));   // untouched
    REQUIRE(bar.getMasterLevelSlider().getValue() == Approx(1.0).margin(0.006));
}

TEST_CASE("layout: the Signal fader sits to the right of Fade with no overlap on Master",
         "[link][signal][layout]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    // A generous width so every widget in the bar gets positive space;
    // this is a same-row ordering/overlap check, not a pixel-budget test.
    bar.setSize(1728, 40);

    auto sig = bar.fadeSliderBoundsForTest();      // existing Fade slider (left neighbour)
    auto sigLabel = bar.masterSignalLabelBoundsForTest();
    auto& sigSlider = bar.getMasterSignalSlider();
    auto& masterSlider = bar.getMasterLevelSlider();

    REQUIRE(sigLabel.getX() >= sig.getRight());
    REQUIRE(sigSlider.getWidth() == 70);
    REQUIRE(sigSlider.getRight() <= masterSlider.getX());
    REQUIRE(!sigSlider.getBounds().intersects(masterSlider.getBounds()));
}
