// test_param_control_routine_cue -- s-rta-0927 routine display, slice A C7 (plan-routine-display-A.md 5.6; design
// routine-ux/design-final.md 2.3.4, pulled forward from slice B): while a lane-rank hand (a routine -- or a take
// replay: the cue is rank-based) grips a bound control, its value digits are cyan and the collapsed hint slot reads
// "ROUTINE"; a human hand (HumanHeld / HumanDecaying) shows nothing new. The signal triangle is unchanged.
// Headless JUCE widgets under ScopedJuceInitialiser_GUI; the control is rendered with createComponentSnapshot and
// its pixels decoded (the test_topbar_link_toggle method) -- cue vs no-cue, never a hard-coded pixel colour alone.
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/UniversalParamControl.h"
#include "ui/LookAndFeel.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ManualWrite.h"

namespace
{
    // s-rta-0927 fix round (critic MUST): the routine cue is chartreuse -- 68..100 degrees, saturated; the
    // 60-120 degree band no other UI element uses -- never the accent cyan every mapped knob already wears.
    int cuePixels(const juce::Image& img, juce::Rectangle<int> area)
    {
        int n = 0;
        for (int y = area.getY(); y < area.getBottom(); ++y)
            for (int x = area.getX(); x < area.getRight(); ++x)
            {
                const auto c = img.getPixelAt(x, y);
                const float hueDeg = c.getHue() * 360.0f;
                if (c.getAlpha() > 20 && c.getBrightness() > 0.4f && c.getSaturation() > 0.5f
                    && hueDeg >= 68.0f && hueDeg <= 100.0f)
                    ++n;
            }
        return n;
    }
    // Pixels in `area` whose colour is cyan-dominant (the app's kAccentCyan #00e5ff family, any alpha > 0).
    int cyanPixels(const juce::Image& img, juce::Rectangle<int> area)
    {
        int n = 0;
        for (int y = area.getY(); y < area.getBottom(); ++y)
            for (int x = area.getX(); x < area.getRight(); ++x)
            {
                const auto c = img.getPixelAt(x, y);
                if (c.getAlpha() > 20 && c.getRed() < 90 && c.getGreen() > 140 && c.getBlue() > 160)
                    ++n;
            }
        return n;
    }
    // Pixels in `area` that are light grey text (kTextPrimary family: R, G, B all high and close).
    int greyTextPixels(const juce::Image& img, juce::Rectangle<int> area)
    {
        int n = 0;
        for (int y = area.getY(); y < area.getBottom(); ++y)
            for (int x = area.getX(); x < area.getRight(); ++x)
            {
                const auto c = img.getPixelAt(x, y);
                if (c.getAlpha() > 60 && c.getRed() > 150 && std::abs(c.getRed() - c.getBlue()) < 30)
                    ++n;
            }
        return n;
    }

    // s-rta-0927 fix round 2 (graphic-design critic): WCAG relative luminance and the brightest pixel's contrast in
    // `area` against `bg` -- the peak-sample method the critic measured the hint with (4.54:1 at 50 % alpha).
    double relLum(juce::Colour c)
    {
        const auto ch = [](juce::uint8 v) {
            const double s = v / 255.0;
            return s <= 0.04045 ? s / 12.92 : std::pow((s + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * ch(c.getRed()) + 0.7152 * ch(c.getGreen()) + 0.0722 * ch(c.getBlue());
    }
    double peakContrast(const juce::Image& img, juce::Rectangle<int> area, juce::Colour bg, juce::Colour& peak)
    {
        double best = 0.0;
        for (int y = area.getY(); y < area.getBottom(); ++y)
            for (int x = area.getX(); x < area.getRight(); ++x)
                if (const auto c = img.getPixelAt(x, y); relLum(c) > best)
                {
                    best = relLum(c);
                    peak = c;
                }
        return (best + 0.05) / (relLum(bg) + 0.05);
    }
    // The control's OWN paint() (the triangle, name, digits and hint -- no child slider or buttons, whose thumb in
    // the cue colour would otherwise sit in the hint slot's pixels) on the Layer inspector's own background
    // (LayerInspector::paint fills #1a1a1a), under a routine's (lane-rank) hand.
    juce::Image paintOnPanel(ParamConnection& conn, LiveValue& live, juce::Colour bg)
    {
        UniversalParamControl c;
        c.setParamName("Opacity");
        c.setSize(260, 24);
        c.setVisible(true);   // Pitfall 34
        c.bindConnection(&conn, &live);
        conn.grip.kind = ParamConnection::Grip::Kind::Held;
        conn.grip.rank = static_cast<uint8_t>(Hand::Lane);
        c.setParamValue(0.62f);
        juce::Image out(juce::Image::ARGB, 260, 24, true);
        {
            juce::Graphics g(out);
            g.fillAll(bg);
            c.paint(g);
        }
        conn.grip.kind = ParamConnection::Grip::Kind::None;
        conn.grip.rank = 0;
        return out;
    }

    juce::Image render(ParamConnection& conn, LiveValue& live, uint8_t rank, ParamConnection::Grip::Kind kind)
    {
        UniversalParamControl c;
        c.setParamName("Opacity");
        c.setSize(260, 24);
        c.setVisible(true);   // Pitfall 34
        c.bindConnection(&conn, &live);
        conn.grip.kind = kind;
        conn.grip.rank = rank;
        c.setParamValue(0.62f);   // the inspector's 10 Hz refresh, after the hand took the control
        c.repaint();
        auto img = c.createComponentSnapshot(c.getLocalBounds(), true, 1.0f);
        conn.grip.kind = ParamConnection::Grip::Kind::None;   // never leave a grip for the destructor's release
        conn.grip.rank = 0;
        return img;
    }
}

TEST_CASE("UniversalParamControl: a routine's (lane-rank) hand paints the digits in the routine cue and ROUTINE in the hint slot", "[paramcontrol][routine][cue]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ParamConnection conn;
    LiveValue live;
    const auto digits = juce::Rectangle<int>(UniversalParamControl::kTriangleSize + 72, 0, 36, 24);
    const auto hint = juce::Rectangle<int>(UniversalParamControl::kTriangleSize + 72 + 36 + 46, 0, 260 - 168, 10);

    const auto lane = render(conn, live, static_cast<uint8_t>(Hand::Lane), ParamConnection::Grip::Kind::Held);
    const auto human = render(conn, live, static_cast<uint8_t>(Hand::HumanHeld), ParamConnection::Grip::Kind::Held);
    const auto none = render(conn, live, 0, ParamConnection::Grip::Kind::None);

    const auto whole = lane.getBounds();
    INFO("digits cue: lane " << cuePixels(lane, digits) << ", human " << cuePixels(human, digits)
         << ", none " << cuePixels(none, digits) << "; hint cue: lane " << cuePixels(lane, hint)
         << ", human " << cuePixels(human, hint) << ", none " << cuePixels(none, hint)
         << "; whole-row accent cyan: lane " << cyanPixels(lane, whole) << ", none " << cyanPixels(none, whole)
         << "; whole-row cue: lane " << cuePixels(lane, whole));
    CHECK(cuePixels(lane, digits) >= 10);         // the value in the routine cue
    CHECK(cyanPixels(lane, digits) == 0);         // ... never the accent cyan a mapped knob wears
    CHECK(cuePixels(human, digits) == 0);         // a human hand: the digits stay kTextPrimary
    CHECK(greyTextPixels(human, digits) >= 10);
    CHECK(cuePixels(none, digits) == 0);
    CHECK(cuePixels(lane, hint) > cuePixels(none, hint) + 5);   // "ROUTINE" in the hint slot
    CHECK(cuePixels(human, hint) == cuePixels(none, hint));
    // The whole row under a routine's hand carries no accent cyan at all (the slider's thumb follows the cue);
    // without one the thumb is the ordinary accent cyan.
    CHECK(cyanPixels(none, whole) > 0);
    CHECK(cyanPixels(lane, whole) == 0);
    CHECK(cuePixels(lane, whole) > cuePixels(lane, digits) + cuePixels(lane, hint));

    SECTION("a decaying lane grip lights it too; a decaying human grip does not")
    {
        const auto laneDecay = render(conn, live, static_cast<uint8_t>(Hand::Lane), ParamConnection::Grip::Kind::Decaying);
        const auto humanDecay = render(conn, live, static_cast<uint8_t>(Hand::HumanDecaying), ParamConnection::Grip::Kind::Decaying);
        CHECK(cuePixels(laneDecay, digits) >= 10);
        CHECK(cuePixels(humanDecay, digits) == 0);
    }
}

TEST_CASE("UniversalParamControl: the ROUTINE hint reads at 7:1 or more on the Layer inspector's background", "[paramcontrol][routine][cue][contrast]")
{
    // s-rta-0927 fix round 2 (graphic-design critic SHOULD): the hint was the cue at 50 % alpha -- peak #6F8C37 on
    // #1A1A1A, 4.54:1, the dimmest text in its cluster though it is the one word that says WHY the fader turned
    // chartreuse. It is lifted to WCAG AAA (7:1) in the kRoutineCue family.
    juce::ScopedJuceInitialiser_GUI gui;
    ParamConnection conn;
    LiveValue live;
    const auto panel = juce::Colour(0xff1a1a1a);
    const auto hint = juce::Rectangle<int>(UniversalParamControl::kTriangleSize + 72 + 36 + 46, 0, 260 - 168, 10);

    const auto lane = paintOnPanel(conn, live, panel);
    juce::Colour peak;
    const double contrast = peakContrast(lane, hint, panel, peak);
    INFO("ROUTINE hint peak #" << peak.toDisplayString(false) << " on #1A1A1A: " << contrast << ":1; hue "
         << peak.getHue() * 360.0f << " deg, cue pixels " << cuePixels(lane, hint));
    CHECK(contrast >= 7.0);
    CHECK(cuePixels(lane, hint) > 5);   // still the routine cue's hue, not a grey or a new colour
}
