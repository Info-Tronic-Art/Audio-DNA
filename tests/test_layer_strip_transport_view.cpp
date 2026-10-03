// test_layer_strip_transport_view -- s-rta-0928b idlepaint (plan-idlepaint.md 2.5 / 3.1 test 3 + Harmony adoptions I2 /
// I3). The layer strip's 30 Hz timer used to repaint its transport rect and clip-name box every tick; together with the
// other always-animating widgets that made JUCE's mac peer repaint the whole window 30 times a second (Pitfall 57). Now:
// the transport rect repaints only when what it paints changes (LayerStrip::transportViewOf -- an Image clip's strip is
// silent), the playhead is read ONCE per tick and that one value is both compared and painted (I2), the routine band
// hairline repaints only when its painted width changes (I3), and syncFromModel() still runs every tick (Pitfall 41).
// Headless JUCE widgets under ScopedJuceInitialiser_GUI (the test_layer_strip_follows_model harness); the tick is
// LayerStrip::timerTick() (the timer's body), repaints are counted by uipaint::counters().
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LayerStrip.h"
#include "model/Composition.h"
#include "ShowFixture.h"
#include "ui/LookAndFeel.h"
#include "ui/UiPaintCounters.h"

using Catch::Approx;

namespace
{
const juce::Rectangle<int> kBar(0, 52, 78, 24);   // a 250 x 96 strip's transport rect (LayerStrip::resized)

Clip seqClip()
{
    Clip c;
    c.name = "seq";
    c.mediaType = Clip::MediaType::ImageSequence;
    return c;
}

// Lane bf9b: a strip shows a SHARED layer (Composition::layers[i]); its playing clip is the cell its ref names -- here
// deck 0's row-i column 0. A one-deck show of 3 layers x 2 columns, every cell empty.
Composition show() { return ShowFixture::makeShow(1, 3, 2, false); }

void activate(Composition& comp, int i, const Clip& clip)
{
    comp.decks[0].setClip(i, 0, clip);
    {
        LayerRuntimeSnapshot rt = comp.layers[static_cast<size_t>(i)].runtime();
        rt.activeClipColumn = 0;
        rt.activeDeckId = comp.decks[0].id;
        comp.layers[static_cast<size_t>(i)].setRuntime(rt);
    }
}

uint64_t transportRepaints() { return uipaint::counters().layerStripTransportRepaints.load(); }
uint64_t bandRepaints() { return uipaint::counters().layerStripBandRepaints.load(); }

// The x of the cyan playhead line inside the transport rect of a painted strip (-1 = none).
int playheadColumn(LayerStrip& strip)
{
    juce::Image img(juce::Image::ARGB, strip.getWidth(), strip.getHeight(), true);
    {
        juce::Graphics g(img);
        strip.paint(g);
    }
    const int y = kBar.getCentreY();
    for (int x = kBar.getX() + 1; x < kBar.getRight() - 1; ++x)
    {
        const auto c = img.getPixelAt(x, y);
        if (c.getRed() < 90 && c.getGreen() > 140 && c.getBlue() > 160)
            return x;
    }
    return -1;
}

RoutineDeckView::Band playing(float progress)
{
    RoutineDeckView::Band b;
    b.slot = 0;
    b.name = "Sweep";
    b.state = RoutineDeckView::State::Playing;
    b.progress01 = progress;
    return b;
}
}

TEST_CASE("LayerStrip::transportViewOf: equal views paint equal pixels", "[idlepaint][strip]")
{
    Composition comp = show();
    activate(comp, 0, seqClip());
    auto* clip = comp.playingClip(0);
    const auto v0 = LayerStrip::transportViewOf(comp.playingClip(0), kBar);
    CHECK(v0.showsClip);
    CHECK(v0.playheadX == 0);
    clip->playheadPosition = 0.005;                     // 0.39 px: the same pixel
    CHECK(LayerStrip::transportViewOf(comp.playingClip(0), kBar) == v0);
    clip->playheadPosition = 0.02;                      // 1.56 px: the next pixel
    const auto v1 = LayerStrip::transportViewOf(comp.playingClip(0), kBar);
    CHECK_FALSE(v1 == v0);
    CHECK(v1.playheadX == 1);
    clip->playheadPosition = 0.0;
    clip->inPoint = 0.01f;                              // an in-point edit moves an anti-aliased edge
    CHECK_FALSE(LayerStrip::transportViewOf(comp.playingClip(0), kBar) == v0);
}

TEST_CASE("LayerStrip::transportViewOf: an Image clip and no layer show no clip", "[idlepaint][strip]")
{
    Composition comp = show();
    Clip img;
    img.mediaType = Clip::MediaType::Image;
    activate(comp, 0, img);
    comp.playingClip(0)->playheadPosition = 0.7;
    const auto v = LayerStrip::transportViewOf(comp.playingClip(0), kBar);
    CHECK_FALSE(v.showsClip);
    CHECK(v == LayerStrip::TransportView {});
    CHECK(LayerStrip::transportViewOf(nullptr, kBar) == LayerStrip::TransportView {});
}

TEST_CASE("LayerStrip: the transport rect repaints only when its pixels change", "[idlepaint][strip]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp = show();
    activate(comp, 0, seqClip());
    LayerStrip strip;
    strip.setLayer(&comp.layers[0], 0, &comp);
    strip.setSize(250, 96);
    strip.timerTick();
    const auto base = transportRepaints();
    strip.timerTick();
    strip.timerTick();
    CHECK(transportRepaints() == base);                 // nothing moved: silent
    comp.playingClip(0)->playheadPosition = 0.005;
    strip.timerTick();
    CHECK(transportRepaints() == base);                 // moved less than a pixel: silent
    comp.playingClip(0)->playheadPosition = 0.3;
    strip.timerTick();
    CHECK(transportRepaints() == base + 1);
    strip.timerTick();
    CHECK(transportRepaints() == base + 1);

    Clip img;                                           // an Image clip's strip: silent at idle
    img.mediaType = Clip::MediaType::Image;
    activate(comp, 1, img);
    LayerStrip strip2;
    strip2.setLayer(&comp.layers[1], 1, &comp);
    strip2.setSize(250, 96);
    const auto b2 = transportRepaints();
    for (int i = 0; i < 30; ++i)
        strip2.timerTick();
    CHECK(transportRepaints() == b2);
}

TEST_CASE("LayerStrip (I2): the playhead read at the tick is the one painted", "[idlepaint][strip]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp = show();
    activate(comp, 0, seqClip());
    LayerStrip strip;
    strip.setLayer(&comp.layers[0], 0, &comp);
    strip.setSize(250, 96);
    comp.playingClip(0)->playheadPosition = 0.5;
    strip.timerTick();
    CHECK(playheadColumn(strip) == 39);                 // 0.5 x 78
    comp.playingClip(0)->playheadPosition = 0.9;       // the render thread moves on between two ticks...
    CHECK(playheadColumn(strip) == 39);                 // ...paint never re-reads the model
    strip.timerTick();
    CHECK(playheadColumn(strip) == 70);                 // 0.9 x 78 = 70.2
}

TEST_CASE("LayerStrip (I3): the routine band hairline repaints only when its painted width changes", "[idlepaint][strip]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp = show();
    activate(comp, 0, seqClip());
    LayerStrip strip;
    strip.setLayer(&comp.layers[0], 0, &comp);
    strip.setSize(250, 96);                             // thumbnail 76 x 76: bands shown
    strip.setRoutineBands({ playing(0.5f) });
    strip.timerTick();
    const auto base = bandRepaints();
    strip.timerTick();
    CHECK(bandRepaints() == base);                      // 38 px, unchanged
    strip.setRoutineBands({ playing(0.502f) });         // 38.15 px: the same painted width
    strip.timerTick();
    CHECK(bandRepaints() == base);
    strip.setRoutineBands({ playing(0.52f) });          // 39.5 -> 40 px
    strip.timerTick();
    CHECK(bandRepaints() == base + 1);
    auto waiting = playing(0.9f);
    waiting.state = RoutineDeckView::State::Waiting;    // a waiting band draws no hairline
    strip.setRoutineBands({ waiting });
    strip.timerTick();
    const auto afterWait = bandRepaints();
    strip.timerTick();
    CHECK(bandRepaints() == afterWait);
}

TEST_CASE("LayerStrip: the tick still pulls the faders from the model (Pitfall 41)", "[idlepaint][strip]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition comp = show();
    activate(comp, 0, seqClip());
    LayerStrip strip;
    strip.setLayer(&comp.layers[0], 0, &comp);
    strip.setSize(250, 96);
    auto* v = dynamic_cast<juce::Slider*>(strip.findChildWithID("layerOpacity"));
    REQUIRE(v != nullptr);
    comp.layers[0].opacity = 0.25f;
    strip.timerTick();
    CHECK(v->getValue() == Approx(0.25));
}
