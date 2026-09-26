// s-rta-0925 mastersignal Step 1: the TopBar Signal fader is a second
// widget-grip view of Composition::masterSignal / CompScalar::Signal, built
// exactly like TopBar's own Master fader (test_master_opacity_link.cpp's
// six cases, reproduced here against getMasterSignalSlider()/masterSignal/
// CompScalar::Signal), plus a check that the two faders are independent and
// a layout check that "Signal:" sits immediately left of "Master:" with no
// overlap. Headless JUCE widgets, same harness as test_right_click_reset.cpp;
// the timer read-back is exercised by calling
// TopBar::syncMasterSignalFromComposition() directly (no dispatch loop run).
//
// s-rta-0926 polish (label/readout/accent/tooltip, critic-flagged): cases
// below the original 8 cover the label reading the full "Master Signal"
// name, both faders exposing a numeric readout, the accent colour that
// tells them apart, tooltip text, and the layout budget at the two widths
// named in the task brief -- 1728 (this app's real maximized-window width
// on Boris's hardware, the width the critic screenshots were taken at) and
// 1280 (Main.cpp's setResizeLimits minimum). NOTE: at 1280 the WHOLE right
// side of TopBar (dsp/fps/output/master/signal) already overflows its
// budget pre-existingly (dsp+fps alone consume ~all of the ~105px left
// after the left-side controls) -- critics flagged this density as
// "pre-existing, out of scope" (critic-polish-ux-r1.md). This fix does not
// touch dsp/fps/output/left-side widths (out of fence), so full visibility
// at 1280 is not achievable here; the 1280 case instead guards the
// structural no-overlap invariant that removeFromRight already gives for
// free (widgets shrink toward zero width under a tight budget, they never
// overlap), and documents the width each control actually gets there.
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
    // 90 (s-rta-0926 polish): widened from 70 so the readout added below has
    // a real track left over instead of squeezing the thumb into nothing.
    REQUIRE(sigSlider.getWidth() == 90);
    REQUIRE(sigSlider.getRight() <= masterSlider.getX());
    REQUIRE(!sigSlider.getBounds().intersects(masterSlider.getBounds()));
}

// --- s-rta-0926 polish: whole-word label ------------------------------

TEST_CASE("the Signal fader's label reads the full \"Master Signal\" name, not the bare word",
         "[link][signal][polish]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);

    // CLAUDE.md, Composition::masterSignal, /api/set_master_signal, and OSC
    // /audiodna/signal all use "Master Signal" -- the on-screen label must
    // match so it never reads as an unrelated signal meter next to "Master:".
    REQUIRE(bar.masterSignalLabelTextForTest() == "Master Signal:");
}

// --- s-rta-0926 polish: numeric readout on both faders -----------------

TEST_CASE("Signal and Master both expose a numeric readout, not just thumb position",
         "[link][signal][polish]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& sigSlider = bar.getMasterSignalSlider();
    auto& masterSlider = bar.getMasterLevelSlider();

    REQUIRE(sigSlider.getTextBoxPosition() != juce::Slider::NoTextBox);
    REQUIRE(sigSlider.getTextBoxWidth() > 0);
    REQUIRE(masterSlider.getTextBoxPosition() != juce::Slider::NoTextBox);
    REQUIRE(masterSlider.getTextBoxWidth() > 0);
}

// --- s-rta-0926 polish: non-textual differentiator ----------------------

TEST_CASE("Signal carries a distinct thumb/track accent from Master, using an existing theme colour",
         "[link][signal][polish]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& sigSlider = bar.getMasterSignalSlider();
    auto& masterSlider = bar.getMasterLevelSlider();

    // Signal is painted with the app's existing magenta accent
    // (AudioDNALookAndFeel::kAccentMagenta) -- not a new palette entry, and
    // not the colour Master carries.
    REQUIRE(sigSlider.findColour(juce::Slider::thumbColourId)
            == juce::Colour(AudioDNALookAndFeel::kAccentMagenta));
    REQUIRE(sigSlider.findColour(juce::Slider::thumbColourId)
            != masterSlider.findColour(juce::Slider::thumbColourId));
}

// --- s-rta-0926 polish: tooltips --------------------------------------

TEST_CASE("Signal and Master each carry a plain-words tooltip explaining what they do",
         "[link][signal][polish]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    auto& sigSlider = bar.getMasterSignalSlider();
    auto& masterSlider = bar.getMasterLevelSlider();

    auto sigTip = sigSlider.getTooltip();
    REQUIRE(sigTip.isNotEmpty());
    REQUIRE(sigTip.contains("Master Signal"));
    REQUIRE(sigTip.contains("pulsing")); // 0% still leaves beat effects pulsing

    auto masterTip = masterSlider.getTooltip();
    REQUIRE(masterTip.isNotEmpty());
    REQUIRE(masterTip.contains("Master"));
    REQUIRE(masterTip.contains("output level"));

    // Different controls get different explanations.
    REQUIRE(sigTip != masterTip);
}

// --- s-rta-0926 polish: layout budget at 1728 (the width the critic's
// screenshots were taken at, and this app's real maximized-window width) --

TEST_CASE("layout at 1728: the Master Signal label is not truncated and both readouts are visible",
         "[link][signal][polish][layout]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    bar.setSize(1728, 40);

    auto fade = bar.fadeSliderBoundsForTest();
    auto sigLabel = bar.masterSignalLabelBoundsForTest();
    auto masterLabel = bar.masterLabelBoundsForTest();
    auto& sigSlider = bar.getMasterSignalSlider();
    auto& masterSlider = bar.getMasterLevelSlider();

    // Label bounds do not overlap neighbours on either side.
    REQUIRE(sigLabel.getX() >= fade.getRight());
    REQUIRE(!sigLabel.intersects(fade));
    REQUIRE(sigLabel.getRight() <= sigSlider.getX());
    REQUIRE(!sigLabel.intersects(sigSlider.getBounds()));
    REQUIRE(!sigSlider.getBounds().intersects(masterSlider.getBounds()));
    REQUIRE(!masterLabel.intersects(masterSlider.getBounds()));

    // The label is wide enough to show "Master Signal:" in full -- never
    // truncated to fit a guessed magic-number width.
    auto sigFont = juce::Font(juce::FontOptions(11.0f));
    int textW = juce::GlyphArrangement::getStringWidthInt(sigFont, "Master Signal:");
    REQUIRE(sigLabel.getWidth() >= textW);

    // Readouts have real, nonzero pixel width at this window size.
    REQUIRE(sigSlider.getTextBoxWidth() > 0);
    REQUIRE(masterSlider.getTextBoxWidth() > 0);
    REQUIRE(sigSlider.getWidth() > sigSlider.getTextBoxWidth());   // room for track+thumb too
    REQUIRE(masterSlider.getWidth() > masterSlider.getTextBoxWidth());
}

// --- s-rta-0926 polish: layout budget at the minimum resizable width -----

TEST_CASE("layout at the minimum window width (1280): Signal/Master group never overlaps itself",
         "[link][signal][polish][layout]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp;
    comp.initDefault();
    FeatureBus bus;
    TopBar bar(bus, comp);
    // 1280 == Main.cpp's setResizeLimits(1280, ...) minimum width.
    bar.setSize(1280, 40);

    auto fade = bar.fadeSliderBoundsForTest();
    auto sigLabel = bar.masterSignalLabelBoundsForTest();
    auto masterLabel = bar.masterLabelBoundsForTest();
    auto& sigSlider = bar.getMasterSignalSlider();
    auto& masterSlider = bar.getMasterLevelSlider();

    // Structural invariant that must hold at ANY width: sequential
    // removeFromRight() never produces overlapping siblings, it only
    // shrinks them (toward zero) when the budget runs out. This is the
    // regression guard for the group this fix touched; it does not assert
    // the whole TopBar row fits at 1280 -- it doesn't, pre-existingly, well
    // before reaching this group (critic-polish-ux-r1.md: "TopBar is
    // visually dense... pre-existing, not something this fix touched").
    REQUIRE(!sigLabel.intersects(fade));
    REQUIRE(!sigLabel.intersects(sigSlider.getBounds()));
    REQUIRE(!sigSlider.getBounds().intersects(masterSlider.getBounds()));
    REQUIRE(!masterLabel.intersects(masterSlider.getBounds()));
    REQUIRE(sigLabel.getX() >= fade.getRight());
    REQUIRE(sigSlider.getX() >= sigLabel.getRight());
    REQUIRE(masterSlider.getX() >= masterLabel.getRight());
}
