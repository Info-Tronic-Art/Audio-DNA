// test_routine_pad_paint_key -- s-rta-0929 g4cpu (plan-g4cpu.md 2.3, Pitfall 57 rule 2): a ROUTINES pad repaints only
// when what it PAINTS changes. RoutinePad::paintKeyOf holds every field paintContent draws -- the sweep as a pixel
// width (never progress01, a fresh float every 30 Hz tick while a routine plays), the bar digits, the frame state, the
// marks -- and setSpec() repaints only when the key at the pad's width changed. (a)-(c) the pure key; (d) a real pad:
// the repaint counter moves only on a painted change; (e) the pixel proof -- two specs with equal keys paint the same
// image. Headless JUCE widgets under ScopedJuceInitialiser_GUI.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/RoutinePad.h"
#include "ui/UiPaintCounters.h"

namespace
{
    using State = RoutineDeckView::State;

    RoutineDeckView::Pad playing(float progress)
    {
        RoutineDeckView::Pad p;
        p.number = 1;
        p.name = "Sweep";
        p.state = State::Playing;
        p.loop = true;
        p.bar = 2;
        p.barsTotal = 4;
        p.progress01 = progress;
        p.tooltip = "Sweep";
        return p;
    }

    uint64_t padRepaints() { return uipaint::counters().routinePadRepaints.load(); }

    bool samePixels(const juce::Image& a, const juce::Image& b)
    {
        if (a.getWidth() != b.getWidth() || a.getHeight() != b.getHeight())
            return false;
        for (int y = 0; y < a.getHeight(); ++y)
            for (int x = 0; x < a.getWidth(); ++x)
                if (a.getPixelAt(x, y) != b.getPixelAt(x, y))
                    return false;
        return true;
    }
}

TEST_CASE("RoutinePad::paintKeyOf: the sweep is keyed as the painted pixel width, never the raw progress", "[g4cpu][pad]")
{
    // (a) 90 px: 0.300 -> 27.0 px, 0.304 -> 27.36 px (both 27); 0.311 -> 27.99 px (28)
    CHECK(RoutinePad::paintKeyOf(playing(0.300f), 90) == RoutinePad::paintKeyOf(playing(0.304f), 90));
    CHECK(RoutinePad::paintKeyOf(playing(0.300f), 90).sweepW == 27);
    CHECK_FALSE(RoutinePad::paintKeyOf(playing(0.300f), 90) == RoutinePad::paintKeyOf(playing(0.311f), 90));
    CHECK(RoutinePad::paintKeyOf(playing(0.311f), 90).sweepW == 28);
    // the width is part of the rule: the same two progresses at 200 px are different pixels (60 vs 61)
    CHECK_FALSE(RoutinePad::paintKeyOf(playing(0.300f), 200) == RoutinePad::paintKeyOf(playing(0.304f), 200));
}

TEST_CASE("RoutinePad::paintKeyOf: every other painted field is in the key", "[g4cpu][pad]")
{
    // (b)
    const auto base = playing(0.3f);
    const auto k0 = RoutinePad::paintKeyOf(base, 90);
    auto differs = [&](auto mutate) {
        auto p = base;
        mutate(p);
        return !(RoutinePad::paintKeyOf(p, 90) == k0);
    };
    CHECK(differs([](RoutineDeckView::Pad& p) { p.bar = 3; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.barsTotal = 8; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.name = "Drop"; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.number = 2; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.state = State::Waiting; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.warning = true; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.restartPending = true; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.onShownDeck = false; }));
    CHECK(differs([](RoutineDeckView::Pad& p) { p.loop = false; }));
    // the tooltip and the menu-only settings are not painted: not in the key (as samePad before)
    CHECK_FALSE(differs([](RoutineDeckView::Pad& p) { p.tooltip = "another tooltip"; }));
}

TEST_CASE("RoutinePad::paintKeyOf: no sweep is painted unless Playing", "[g4cpu][pad]")
{
    // (c)
    auto a = playing(0.9f), b = playing(0.1f);
    a.state = b.state = State::Waiting;
    CHECK(RoutinePad::paintKeyOf(a, 90).sweepW == 0);
    CHECK(RoutinePad::paintKeyOf(a, 90) == RoutinePad::paintKeyOf(b, 90));
}

TEST_CASE("RoutinePad::setSpec repaints only when the painted state changes", "[g4cpu][pad]")
{
    // (d)
    juce::ScopedJuceInitialiser_GUI gui;
    RoutinePad pad(0);
    pad.setBounds(0, 0, 90, 22);
    pad.setVisible(true);   // Pitfall 34
    pad.setSpec(playing(0.300f));

    auto n0 = padRepaints();
    pad.setSpec(playing(0.304f));                      // sub-pixel: the sweep stays 27 px
    CHECK(padRepaints() == n0);

    auto tip = playing(0.304f);
    tip.tooltip = "Sweep -- a new tooltip";
    pad.setSpec(tip);                                  // tooltip only: set, never painted
    CHECK(padRepaints() == n0);
    CHECK(pad.getTooltip() == "Sweep -- a new tooltip");

    pad.setSpec(playing(0.311f));                      // 27 -> 28 px
    CHECK(padRepaints() == n0 + 1);

    auto bar = playing(0.311f);
    bar.bar = 3;                                       // the digits
    pad.setSpec(bar);
    CHECK(padRepaints() == n0 + 2);
}

TEST_CASE("RoutinePad: two specs with equal paint keys paint the same pixels", "[g4cpu][pad]")
{
    // (e) the pixel proof of (a)
    juce::ScopedJuceInitialiser_GUI gui;
    RoutinePad pad(0);
    pad.setBounds(0, 0, 90, 22);
    pad.setVisible(true);
    pad.setSpec(playing(0.300f));
    const auto a = pad.createComponentSnapshot(pad.getLocalBounds(), true, 1.0f);
    pad.setSpec(playing(0.304f));
    const auto b = pad.createComponentSnapshot(pad.getLocalBounds(), true, 1.0f);
    pad.setSpec(playing(0.311f));
    const auto c = pad.createComponentSnapshot(pad.getLocalBounds(), true, 1.0f);
    CHECK(samePixels(a, b));
    CHECK_FALSE(samePixels(a, c));   // the comparator sees a 1-px sweep step
}
