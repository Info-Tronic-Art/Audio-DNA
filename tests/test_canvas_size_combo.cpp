// test_canvas_size_combo -- s-rta-0926b canvas fix round: the Composition inspector's
// Resolution dropdown showed "1920x1080" for a 1080x1920 or 1024x768 canvas (it matched on
// width only and had no fallback), and offered no way to pick a non-16:9 shape.
// CanvasSizeCombo (src/ui/CanvasSizeCombo.h) is what CompositionInspector runs; this drives
// it on a real juce::ComboBox, headless under ScopedJuceInitialiser_GUI (no window, no peer).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/CanvasSizeCombo.h"

TEST_CASE("CanvasSizeCombo: the dropdown never shows a size the canvas is not", "[canvassizecombo]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    juce::ComboBox combo;

    SECTION("the default 16:9 canvas selects its preset (ids 1..4 unchanged)")
    {
        CanvasSizeCombo::show(combo, 1920, 1080);
        CHECK(combo.getSelectedId() == 1);
        CHECK(combo.getText() == "1920x1080");
        CanvasSizeCombo::show(combo, 3840, 2160);
        CHECK(combo.getSelectedId() == 4);
        CHECK(combo.getText() == "3840x2160");
        CHECK(combo.indexOfItemId(CanvasSizeCombo::kCustomId) < 0);
    }

    SECTION("portrait and 4:3 canvases show themselves, never 1920x1080")
    {
        CanvasSizeCombo::show(combo, 1080, 1920);
        CHECK(combo.getText() == "1080x1920 (portrait)");
        CanvasSizeCombo::show(combo, 1024, 768);
        CHECK(combo.getText() == "1024x768 (4:3)");
        CanvasSizeCombo::show(combo, 1080, 1080);
        CHECK(combo.getText() == "1080x1080 (square)");
    }

    SECTION("the old width-only match: 1920 x 1200 is NOT the 1920x1080 preset")
    {
        CanvasSizeCombo::show(combo, 1920, 1200);
        CHECK(combo.getSelectedId() == CanvasSizeCombo::kCustomId);
        CHECK(combo.getText() == "Custom (1920 x 1200)");
    }

    SECTION("any other size shows a Custom item with the live size; it goes away on a preset")
    {
        CanvasSizeCombo::show(combo, 1080, 1350);
        CHECK(combo.getSelectedId() == CanvasSizeCombo::kCustomId);
        CHECK(combo.getText() == "Custom (1080 x 1350)");
        CHECK(combo.getNumItems() == CanvasSizeCombo::kNumPresets + 1);

        CanvasSizeCombo::show(combo, 720, 1280);
        CHECK(combo.getText() == "Custom (720 x 1280)");
        CHECK(combo.getNumItems() == CanvasSizeCombo::kNumPresets + 1);

        CanvasSizeCombo::show(combo, 1280, 720);
        CHECK(combo.getSelectedId() == 2);
        CHECK(combo.getNumItems() == CanvasSizeCombo::kNumPresets);
        CHECK(combo.indexOfItemId(CanvasSizeCombo::kCustomId) < 0);
    }
}

TEST_CASE("CanvasSizeCombo: a selection maps back to the size it names", "[canvassizecombo]")
{
    int w = -1, h = -1;
    for (int id = 1; id <= CanvasSizeCombo::kNumPresets; ++id)
    {
        REQUIRE(CanvasSizeCombo::sizeFor(id, w, h));
        CHECK(CanvasSizeCombo::idFor(w, h) == id);   // round trip
    }
    CHECK(CanvasSizeCombo::sizeFor(5, w, h));
    CHECK((w == 1080 && h == 1920));
    CHECK(CanvasSizeCombo::sizeFor(7, w, h));
    CHECK((w == 1024 && h == 768));

    // Custom (and "nothing selected") ask for no change.
    w = h = -1;
    CHECK_FALSE(CanvasSizeCombo::sizeFor(CanvasSizeCombo::kCustomId, w, h));
    CHECK_FALSE(CanvasSizeCombo::sizeFor(0, w, h));
    CHECK((w == -1 && h == -1));

    // Every preset is a distinct size.
    for (int i = 0; i < CanvasSizeCombo::kNumPresets; ++i)
        for (int j = i + 1; j < CanvasSizeCombo::kNumPresets; ++j)
            CHECK_FALSE((CanvasSizeCombo::kPresets[i].w == CanvasSizeCombo::kPresets[j].w
                         && CanvasSizeCombo::kPresets[i].h == CanvasSizeCombo::kPresets[j].h));
}
