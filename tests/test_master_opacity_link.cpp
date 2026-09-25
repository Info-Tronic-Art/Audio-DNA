// s-rta-0925 (lane "link"): the TopBar master fader must be a second
// widget-grip view of Composition::masterOpacity / CompScalar::Opacity, not
// a second independent variable (diag-opacity.md; plan-opacity.md Lane B).
// Written fail-first against HEAD's disconnected TopBar fader (writes the
// now-deleted Renderer::masterLevel_) plus this commit's stubs
// (TopBar::syncMasterFromComposition() is a no-op; getMasterLevelSlider()
// narrowed to ResettableSlider&). Headless JUCE widgets, same harness as
// test_right_click_reset.cpp; the timer read-back is exercised by calling
// TopBar::syncMasterFromComposition() directly (no dispatch loop is run).
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
    constexpr auto kOp = static_cast<size_t>(CompScalar::Opacity);
}

TEST_CASE("fader write lands on Composition::masterOpacity", "[link]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterLevelSlider();

    fader.setValue(0.3, juce::sendNotificationSync);

    REQUIRE(comp.masterOpacity == Approx(0.3f).margin(0.006));
    REQUIRE(comp.scalarConns[kOp].grip.kind == ParamConnection::Grip::Kind::None);
}

TEST_CASE("fader drag grips Held on CompScalar::Opacity and releases on drag end", "[link]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterLevelSlider();

    REQUIRE(bool(fader.onDragStart));
    REQUIRE(bool(fader.onDragEnd));

    fader.onDragStart();
    REQUIRE(comp.scalarConns[kOp].grip.kind == ParamConnection::Grip::Kind::Held);

    fader.onDragEnd();
    REQUIRE(comp.scalarConns[kOp].grip.kind == ParamConnection::Grip::Kind::None);
}

TEST_CASE("sync: not connected -> fader shows the manual field", "[link]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterLevelSlider();

    comp.masterOpacity = 0.8f;
    bar.syncMasterFromComposition();

    REQUIRE(fader.getValue() == Approx(0.8).margin(0.006));
    REQUIRE(comp.masterOpacity == Approx(0.8f)); // sync never writes back
}

TEST_CASE("sync: connected -> fader shows toNorm(eff()), not the manual field", "[link]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterLevelSlider();

    comp.scalarConns[kOp].source.kind = ConnSource::Kind::Signal;
    comp.scalarLive[kOp].v.store(0.25f);
    comp.masterOpacity = 1.0f;
    bar.syncMasterFromComposition();
    REQUIRE(fader.getValue() == Approx(0.25).margin(0.006));

    // Gripped: the engine publishes NaN (twin), so eff() falls back to the
    // manual field — the fader shows the manual value while gripped.
    comp.scalarConns[kOp].gripHeld();
    comp.scalarLive[kOp].v.store(std::nanf(""));
    comp.masterOpacity = 0.6f;
    bar.syncMasterFromComposition();
    REQUIRE(fader.getValue() == Approx(0.6).margin(0.006));
}

TEST_CASE("sync: the thumb never moves while the fader is being dragged", "[link]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterLevelSlider();

    fader.setValue(0.5, juce::sendNotificationSync);
    fader.onDragStart();
    comp.masterOpacity = 0.1f;
    bar.syncMasterFromComposition();
    REQUIRE(fader.getValue() == Approx(0.5).margin(0.006));

    fader.onDragEnd();
    bar.syncMasterFromComposition();
    REQUIRE(fader.getValue() == Approx(0.1).margin(0.006));
}

TEST_CASE("right-click on the fader resets to 1.0 on the model and touches the grip", "[link]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& fader = bar.getMasterLevelSlider();

    fader.setValue(0.4, juce::sendNotificationSync);
    fader.mouseDown(clickOn(fader, true));

    REQUIRE(comp.masterOpacity == Approx(1.0f));
    REQUIRE(comp.scalarConns[kOp].grip.kind == ParamConnection::Grip::Kind::Decaying);
}
