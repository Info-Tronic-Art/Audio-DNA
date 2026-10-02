// test_layer_strip_follows_model -- s-rta-0927 routine display, slice A (plan-routine-display-A.md 2.2 / 2.5 /
// 5.3). The layer strip's V fader used to pull layer->opacity ONCE (setLayer) and never again, so a routine,
// REST set_layer_opacity, MIDI or OSC write never moved it; LayerStrip::syncFromModel (the strip's 30 Hz
// timer) now re-reads opacity (eff() when connected) and the active clip's speed, and turns the V fill cyan
// while a lane-rank hand grips opacity. The routine bands' x takes the whole routine off (onRoutineRemove).
// Headless JUCE widgets under ScopedJuceInitialiser_GUI -- no window, no peer; syncFromModel() is called
// directly (no dispatch loop is run), mouse events are synthesised and delivered by calling mouseDown().
// NOT covered (disclosed): "skipped while the fader is being dragged" -- isMouseButtonDown() needs a real
// mouse source; the guard is checked by review.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LayerStrip.h"
#include "ui/LookAndFeel.h"
#include "connect/ManualWrite.h"

using Catch::Approx;

namespace
{
    juce::Slider* slider(LayerStrip& strip, const juce::String& id)
    {
        return dynamic_cast<juce::Slider*>(strip.findChildWithID(id));
    }

    juce::MouseEvent leftPressAt(juce::Component& target, juce::Point<float> at)
    {
        const juce::ModifierKeys mods(juce::ModifierKeys::leftButtonModifier);
        const auto now = juce::Time::getCurrentTime();
        return { juce::Desktop::getInstance().getMainMouseSource(), at, mods,
                 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, at, now, 1, false };
    }

    // LayerStrip::mouseDown is private; it overrides juce::Component::mouseDown (public, virtual).
    void press(LayerStrip& strip, const juce::MouseEvent& e) { static_cast<juce::Component&>(strip).mouseDown(e); }

    // The routine cue's hue family (s-rta-0927 fix round): 68..100 degrees, saturated -- chartreuse.
    bool isRoutineCueHue(juce::Colour c)
    {
        const float hueDeg = c.getHue() * 360.0f;
        return hueDeg >= 68.0f && hueDeg <= 100.0f && c.getSaturation() > 0.5f;
    }

    int cuePixels(const juce::Image& img, juce::Rectangle<int> area)
    {
        int n = 0;
        for (int y = area.getY(); y < area.getBottom(); ++y)
            for (int x = area.getX(); x < area.getRight(); ++x)
            {
                const auto c = img.getPixelAt(x, y);
                if (c.getAlpha() > 20 && c.getBrightness() > 0.4f && isRoutineCueHue(c))
                    ++n;
            }
        return n;
    }

    int accentCyanPixels(const juce::Image& img, juce::Rectangle<int> area)   // the app's kAccentCyan family
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

    RoutineDeckView::Band band(int slot, const char* name, RoutineDeckView::State st = RoutineDeckView::State::Playing)
    {
        RoutineDeckView::Band b;
        b.slot = slot;
        b.name = name;
        b.state = st;
        b.progress01 = 0.5f;
        return b;
    }
}

TEST_CASE("LayerStrip: the V and S faders carry component ids", "[layerstrip][follow]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    LayerStrip strip;
    strip.setLayer(&layer, 0);
    strip.setSize(250, 96);
    REQUIRE(strip.findChildWithID("layerOpacity") != nullptr);
    REQUIRE(strip.findChildWithID("layerSpeed") != nullptr);
}

TEST_CASE("LayerStrip: the V fader follows layer opacity written after setLayer", "[layerstrip][follow]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    LayerStrip strip;
    strip.setLayer(&layer, 0);
    strip.setSize(250, 96);
    auto* v = slider(strip, "layerOpacity");
    REQUIRE(v != nullptr);
    CHECK(v->getValue() == Approx(1.0));

    int writes = 0;
    v->onValueChange = [&writes] { ++writes; };   // a follow must never look like a user write
    layer.opacity = 0.3f;                           // a routine / REST / MIDI / OSC write lands in the model
    strip.syncFromModel();
    CHECK(v->getValue() == Approx(0.3));
    CHECK(writes == 0);
}

TEST_CASE("LayerStrip: a connected opacity shows its effective value", "[layerstrip][follow]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    LayerStrip strip;
    strip.setLayer(&layer, 0);
    strip.setSize(250, 96);
    auto& conn = layer.scalarConns[static_cast<size_t>(LayerScalar::Opacity)];
    conn.source.kind = ConnSource::Kind::Signal;
    conn.source.signalName = "rms";
    REQUIRE(conn.isConnected());
    layer.scalarLive[static_cast<size_t>(LayerScalar::Opacity)].v = 0.7f;
    layer.opacity = 0.2f;
    REQUIRE(layer.eff(LayerScalar::Opacity) == Approx(0.7f));
    strip.syncFromModel();
    CHECK(slider(strip, "layerOpacity")->getValue() == Approx(0.7));
}

TEST_CASE("LayerStrip: the V fill is cyan only while a lane-rank hand grips opacity", "[layerstrip][follow][cue]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    LayerStrip strip;
    strip.setLayer(&layer, 0);
    strip.setSize(250, 96);
    auto* v = slider(strip, "layerOpacity");
    REQUIRE(v != nullptr);
    auto& grip = layer.scalarConns[static_cast<size_t>(LayerScalar::Opacity)].grip;

    strip.syncFromModel();
    CHECK_FALSE(v->isColourSpecified(juce::Slider::trackColourId));

    grip.kind = ParamConnection::Grip::Kind::Held;
    grip.rank = static_cast<uint8_t>(Hand::Lane);   // a routine's (or a replay's) hand
    strip.syncFromModel();
    REQUIRE(v->isColourSpecified(juce::Slider::trackColourId));
    // s-rta-0927 fix round (critic MUST): the routine cue is its own hue -- chartreuse, the 60-120 degree band no
    // other UI element uses -- never the app-wide accent cyan every mapped knob and Mod macro already wears.
    CHECK(isRoutineCueHue(v->findColour(juce::Slider::trackColourId)));
    CHECK(v->findColour(juce::Slider::trackColourId) != juce::Colour(AudioDNALookAndFeel::kAccentCyan));

    grip.rank = static_cast<uint8_t>(Hand::HumanHeld);   // a human hand: no routine cue
    strip.syncFromModel();
    CHECK_FALSE(v->isColourSpecified(juce::Slider::trackColourId));

    grip.rank = static_cast<uint8_t>(Hand::Lane);
    strip.syncFromModel();
    CHECK(v->isColourSpecified(juce::Slider::trackColourId));
    grip.kind = ParamConnection::Grip::Kind::None;       // released
    grip.rank = 0;
    strip.syncFromModel();
    CHECK_FALSE(v->isColourSpecified(juce::Slider::trackColourId));
}

TEST_CASE("LayerStrip: the S fader follows the active clip's speed", "[layerstrip][follow]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    layer.ensureColumns(2);
    Clip clip;
    clip.speed = 1.0f;
    layer.clips[0] = clip;
    {
        LayerRuntimeSnapshot rt = layer.runtime();
        rt.activeClipColumn = 0;
        layer.setRuntime(rt);
    }
    REQUIRE(layer.getActiveClip() != nullptr);
    LayerStrip strip;
    strip.setLayer(&layer, 0);
    strip.setSize(250, 96);
    auto* s = slider(strip, "layerSpeed");
    REQUIRE(s != nullptr);
    CHECK(s->getValue() == Approx(0.25));
    layer.getActiveClip()->speed = 2.0f;   // a routine can drive speed
    strip.syncFromModel();
    CHECK(s->getValue() == Approx(0.5));
}

TEST_CASE("LayerStrip: a routine band's x removes that routine; the rest of the band selects the layer", "[layerstrip][routine]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    LayerStrip strip;
    strip.setLayer(&layer, 2);
    strip.setSize(250, 96);
    strip.setVisible(true);   // Pitfall 34
    strip.setRoutineBands({ band(1, "Build"), band(0, "Drop") });

    std::vector<int> removed;
    int selected = 0;
    strip.onRoutineRemove = [&removed](int slot) { removed.push_back(slot); };
    strip.onSelect = [&selected](int) { ++selected; };

    // Thumbnail (144, 0, 76, 76): band 0 = y 0..16, band 1 = y 16..32, the x = the rightmost 16x16 (x 204..220).
    press(strip, leftPressAt(strip, { 212.0f, 8.0f }));
    REQUIRE(removed.size() == 1);
    CHECK(removed[0] == 1);
    press(strip, leftPressAt(strip, { 212.0f, 24.0f }));
    REQUIRE(removed.size() == 2);
    CHECK(removed[1] == 0);
    CHECK(selected == 0);

    press(strip, leftPressAt(strip, { 160.0f, 8.0f }));   // on the name: no removal, the strip selects
    CHECK(removed.size() == 2);
    CHECK(selected == 1);

    press(strip, leftPressAt(strip, { 212.0f, 40.0f }));  // below the bands: nothing to remove
    CHECK(removed.size() == 2);

    SECTION("a folded row draws no bands and has no x")
    {
        strip.setSize(250, 22);
        press(strip, leftPressAt(strip, { 212.0f, 8.0f }));
        CHECK(removed.size() == 2);
    }
}

TEST_CASE("LayerStrip: a playing band's name is painted in the routine cue hue, never the accent cyan", "[layerstrip][routine][cue]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    LayerStrip strip;
    strip.setLayer(&layer, 0);
    strip.setSize(250, 96);
    strip.setVisible(true);   // Pitfall 34
    const auto nameArea = juce::Rectangle<int>(147, 0, 57, 16);   // band 0, left of its x (thumbnail x 144..220)

    const auto without = strip.createComponentSnapshot(strip.getLocalBounds(), true, 1.0f);
    strip.setRoutineBands({ band(0, "Drop") });
    const auto with = strip.createComponentSnapshot(strip.getLocalBounds(), true, 1.0f);

    INFO("band name: cue " << cuePixels(with, nameArea) << " (without " << cuePixels(without, nameArea)
         << "), accent cyan " << accentCyanPixels(with, nameArea));
    CHECK(cuePixels(with, nameArea) >= 10);
    CHECK(cuePixels(without, nameArea) == 0);
    CHECK(accentCyanPixels(with, nameArea) == 0);
}

// s-rta-0927 fix round (critic SHOULDs): the two controls that now stop routines say so before they are pressed --
// a band's x (the WHOLE routine, every layer it plays on) and the layer's familiar X (it clears the clip and now
// also stops every routine on the layer). The routine stop cannot be undone; the tooltips say that too.
TEST_CASE("LayerStrip: the band x and the layer X carry tooltips that name the routine stop", "[layerstrip][routine][tooltip]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Layer layer;
    LayerStrip strip;
    strip.setLayer(&layer, 0);
    strip.setSize(250, 96);
    strip.setVisible(true);   // Pitfall 34

    juce::String clearTip;
    for (auto* child : strip.getChildren())
        if (auto* b = dynamic_cast<juce::Button*>(child); b != nullptr && b->getButtonText() == "X")
            clearTip = b->getTooltip();
    INFO("layer X tooltip: '" << clearTip << "'");
    CHECK(clearTip.containsIgnoreCase("clip"));
    CHECK(clearTip.containsIgnoreCase("routine"));
    CHECK(clearTip.containsIgnoreCase("undone"));

    strip.setRoutineBands({ band(1, "Build"), band(0, "Drop") });
    const auto xTip = strip.tooltipAt({ 212, 8 });
    INFO("band x tooltip: '" << xTip << "'");
    CHECK(xTip.containsIgnoreCase("every layer"));
    CHECK(xTip.containsIgnoreCase("undone"));
    CHECK(strip.tooltipAt({ 212, 24 }) == xTip);   // the second band's x says the same
    CHECK(strip.tooltipAt({ 160, 8 }).isEmpty());  // the name: no tip
    CHECK(strip.tooltipAt({ 212, 40 }).isEmpty()); // below the bands
    strip.setRoutineBands({});
    CHECK(strip.tooltipAt({ 212, 8 }).isEmpty());  // no band, no x
}
