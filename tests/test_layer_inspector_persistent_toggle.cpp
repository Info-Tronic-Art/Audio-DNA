// s-rta-0926b uitoggle: LayerInspector's Persistent toggle enable rule. Pre-fix, the rule
// was simply `enabled = Layer::canBePersistent(type)` -- so a Mask (or 3D) layer loaded from
// a composition file with persistent=true showed the box checked-but-permanently-disabled,
// with no way to clear the (inert) flag. The fix widens the rule to
// `enabled = canBePersistent(type) || layer->persistent`, so a stale flag can be cleared --
// and once cleared, canBePersistent is still false and persistent is now false too, so the
// next sync disables the toggle again (it cannot be re-set). Headless JUCE widgets, same
// idiom as test_topbar_link_toggle.cpp / test_effect_stack_binding.cpp -- no test seam,
// LayerInspector's real setLayer()/syncFromLayer() path drives the toggle.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LayerInspector.h"
#include "model/Layer.h"

namespace
{
    juce::ToggleButton* findPersistentToggle(juce::Component& inspector)
    {
        for (auto* child : inspector.getChildren())
            if (auto* toggle = dynamic_cast<juce::ToggleButton*>(child))
                if (toggle->getButtonText() == "Persistent")
                    return toggle;
        return nullptr;
    }
}

TEST_CASE("LayerInspector Persistent toggle: enabled iff the type supports it or the flag is "
          "already set, and a cleared stale flag cannot be re-set",
          "[layer-inspector][persistent][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    LayerInspector inspector;
    inspector.setSize(300, 900);

    auto* toggle = findPersistentToggle(inspector);
    REQUIRE(toggle != nullptr);

    SECTION("Opaque layer, flag off: enabled, normal tooltip")
    {
        Layer layer;
        layer.type = Layer::Type::Opaque;
        layer.persistent = false;
        inspector.setLayer(&layer);

        CHECK(toggle->isEnabled());
        CHECK(toggle->getTooltip() == "Keep this layer rendering when switching to another deck");
    }

    SECTION("Mask layer, flag off: disabled, explains why")
    {
        Layer layer;
        layer.type = Layer::Type::Mask;
        layer.persistent = false;
        inspector.setLayer(&layer);

        CHECK_FALSE(toggle->isEnabled());
        CHECK(toggle->getTooltip() == "Persistent is available for Opaque, Transparent and FX Only layers");
    }

    SECTION("Mask layer loaded with a stale persistent=true flag: enabled so it can be cleared")
    {
        Layer layer;
        layer.type = Layer::Type::Mask;
        layer.persistent = true;   // e.g. loaded from a composition file saved before R4-types
        inspector.setLayer(&layer);

        CHECK(toggle->isEnabled());                 // RED pre-fix: disabled (checked-but-inert)
        REQUIRE(toggle->getToggleState());
        // RED pre-fix-round: tooltip read the same "working" string as a real
        // persistable layer, even though canBePersistent is still false here.
        CHECK(toggle->getTooltip() ==
              "This layer type doesn't support Persistent -- this box is enabled only so you can clear the leftover flag");

        // The user clears the stale flag (a real click, same path syncFromLayer's onClick
        // handler wires) -- and once cleared it cannot be re-set: the next sync sees
        // canBePersistent()==false AND persistent==false, so the toggle disables itself.
        toggle->setToggleState(false, juce::sendNotification);
        CHECK_FALSE(layer.persistent);
        inspector.refresh();

        CHECK_FALSE(toggle->isEnabled());            // RED pre-fix (rule never re-evaluated this way)
        CHECK(toggle->getTooltip() == "Persistent is available for Opaque, Transparent and FX Only layers");
        CHECK_FALSE(toggle->getToggleState());
    }
}
