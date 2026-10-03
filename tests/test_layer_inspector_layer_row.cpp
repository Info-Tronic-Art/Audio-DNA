// bf9 Stage P (s-rta-1002b; ruling-bf9 amendment 11): the Layer tab's "Layer" section after the Persistent layer
// feature was removed. Boris (2026-10-02): "I want to remove the persistent. The only thing remotely persistent should
// be to ignore column controls". So: (1) no "Persistent" toggle is left; (2) Ignore Column Trigger keeps exactly its
// behaviour for every layer type; (3) it takes the full row under Master (no hole where Persistent was).
// Headless JUCE widgets, the idiom of the file this replaced (test_layer_inspector_persistent_toggle.cpp): children
// found by button text through getChildren + dynamic_cast; a click = setToggleState(x, juce::sendNotification), the
// same path the onClick handler wires. LayerInspector's real setLayer() / refresh() / resized() drive the widgets.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LayerInspector.h"
#include "ui/UniversalParamControl.h"
#include "model/Layer.h"

namespace
{
    juce::ToggleButton* findToggle(juce::Component& inspector, const juce::String& text)
    {
        for (auto* child : inspector.getChildren())
            if (auto* toggle = dynamic_cast<juce::ToggleButton*>(child))
                if (toggle->getButtonText() == text)
                    return toggle;
        return nullptr;
    }
}

TEST_CASE("LayerInspector Layer section: Ignore Column Trigger is there and no Persistent toggle is left (bf9 Stage P)",
          "[layer-inspector][bf9]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    LayerInspector inspector;
    inspector.setSize(300, 900);
    Layer layer;
    layer.type = Layer::Type::Opaque;
    inspector.setLayer(&layer);

    REQUIRE(findToggle(inspector, "Ignore Column Trigger") != nullptr);   // positive control: the walk finds toggles
    CHECK(findToggle(inspector, "Persistent") == nullptr);
}

TEST_CASE("LayerInspector Ignore Column Trigger: for every layer type it is enabled, mirrors the field, a click writes "
          "it, and its tooltip is unchanged (bf9 Stage P)",
          "[layer-inspector][bf9]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    LayerInspector inspector;
    inspector.setSize(300, 900);

    auto* toggle = findToggle(inspector, "Ignore Column Trigger");
    REQUIRE(toggle != nullptr);

    const Layer::Type types[] = { Layer::Type::Opaque, Layer::Type::Transparent, Layer::Type::FXOnly,
                                  Layer::Type::ThreeD, Layer::Type::Mask };
    for (const auto type : types)
    {
        INFO("Layer::Type " << static_cast<int>(type));
        for (const bool initial : { false, true })
        {
            INFO("ignoreColumnTrigger initially " << initial);
            Layer layer;
            layer.type = type;
            layer.ignoreColumnTrigger = initial;
            inspector.setLayer(&layer);

            CHECK(toggle->isEnabled());
            CHECK(toggle->getToggleState() == initial);
            CHECK(toggle->getTooltip() == "This layer ignores column trigger buttons");

            toggle->setToggleState(!initial, juce::sendNotification);   // a click
            CHECK(layer.ignoreColumnTrigger == !initial);
            inspector.refresh();
            CHECK(toggle->getToggleState() == !initial);

            inspector.setLayer(nullptr);
        }
    }
}

TEST_CASE("LayerInspector Ignore Column Trigger takes the full row under Master (bf9 Stage P)",
          "[layer-inspector][bf9]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    LayerInspector inspector;
    inspector.setSize(300, 900);
    Layer layer;
    layer.type = Layer::Type::Opaque;
    inspector.setLayer(&layer);
    inspector.resized();

    auto* toggle = findToggle(inspector, "Ignore Column Trigger");
    REQUIRE(toggle != nullptr);

    // The Master control from the same layout pass: the UniversalParamControl whose bottom edge is the toggle's top.
    UniversalParamControl* master = nullptr;
    for (auto* child : inspector.getChildren())
        if (auto* upc = dynamic_cast<UniversalParamControl*>(child))
            if (upc->getBottom() == toggle->getY())
                master = upc;
    REQUIRE(master != nullptr);

    INFO("toggle " << toggle->getBounds().toString() << ", master " << master->getBounds().toString());
    CHECK(toggle->getX() == 4);        // getLocalBounds().reduced(4, 0) at width 300
    CHECK(toggle->getWidth() == 292);
    CHECK(toggle->getX() == master->getX());
    CHECK(toggle->getWidth() == master->getWidth());
}
