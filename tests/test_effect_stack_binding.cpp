// s-rta-0925 mastersignal Step 0: EffectStackView rows bind to the model's
// per-param/dry-wet ParamConnection/LiveValue (S0-T5a/b), tickModulation()
// becomes display-only (S0-T5b), and rebuilding rows after the backing
// effects vector is cleared/reallocated must never dereference a freed
// ParamConnection (S0-T5c, PIN -- forget-before-rebuild). Headless JUCE
// widgets under ScopedJuceInitialiser_GUI, same harness as
// tests/test_right_click_reset.cpp (no window, no peer; mouse events are
// synthesised and delivered by calling mouseDown() directly).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/EffectStackView.h"
#include "model/Clip.h"
#include "connect/ParamConnection.h"

using Catch::Approx;

TEST_CASE("EffectStackView::setEffects binds every param row and the dry/wet row to the slot's connection",
         "[connection][effectstack]")
{
    juce::ScopedJuceInitialiser_GUI gui;

    Clip clip;
    clip.effects.push_back(Clip::EffectSlot{});
    auto& fx = clip.effects[0];
    fx.effectName = "Ripple";
    fx.addParam(0.5f);
    fx.addParam(0.2f);
    fx.addParam(0.9f);

    EffectStackView view;
    view.setEffects(&clip.effects);

    REQUIRE(view.dryWetControlForTest(0) != nullptr);
    REQUIRE(view.dryWetControlForTest(0)->boundConnection() == &clip.effects[0].dryWetConn);

    for (int p = 0; p < 3; ++p)
    {
        auto* pc = view.paramControlForTest(0, p);
        REQUIRE(pc != nullptr);
        REQUIRE(pc->boundConnection() == &clip.effects[0].paramConns[static_cast<size_t>(p)]);
    }
}

TEST_CASE("EffectStackView::tickModulation never writes the manual field and the thumb shows fx.effParam",
         "[connection][effectstack]")
{
    juce::ScopedJuceInitialiser_GUI gui;

    Clip clip;
    clip.effects.push_back(Clip::EffectSlot{});
    auto& fx = clip.effects[0];
    fx.effectName = "Ripple";
    fx.addParam(0.2f);

    EffectStackView view;
    view.setEffects(&clip.effects);  // rebuildRows() binds row 0's param 0

    // Connect param 0 (source kind alone is enough for isConnected()==true;
    // the engine would normally publish paramLive[0] via evaluate(), but this
    // test only exercises the DISPLAY path, so the twin is set directly).
    fx.paramConns[0].source.kind = ConnSource::Kind::Lfo;
    fx.paramLive[0].v.store(0.9f, std::memory_order_relaxed);

    // rebuildRows() adds every control via addChildComponent(), which -- like
    // every fresh juce::Component -- starts invisible (componentFlags(0));
    // resized()'s collapsed-row branch would also hide it explicitly. Force
    // it visible directly (the same effect an actual header-click expand
    // would have) so the display-push gate (`changed && pc.isVisible()`, the
    // L9 cost-trap guard) doesn't suppress this tick's assertion.
    auto* pcForVisibility = view.paramControlForTest(0, 0);
    REQUIRE(pcForVisibility != nullptr);
    pcForVisibility->setVisible(true);

    view.tickModulation();

    REQUIRE(fx.paramValues[0] == Approx(0.2f));   // manual field untouched
    auto* pc = view.paramControlForTest(0, 0);
    REQUIRE(pc != nullptr);
    REQUIRE(pc->getParamValue() == Approx(0.9f)); // thumb shows the live twin
}

TEST_CASE("EffectStackView: rebuilding rows after the effects vector is cleared/reallocated touches no freed memory",
         "[connection][effectstack][pin]")
{
    juce::ScopedJuceInitialiser_GUI gui;

    Clip clip;
    clip.effects.push_back(Clip::EffectSlot{});
    clip.effects[0].effectName = "Ripple";
    clip.effects[0].addParam(0.5f);

    EffectStackView view;
    view.setEffects(&clip.effects);

    auto* pc = view.paramControlForTest(0, 0);
    REQUIRE(pc != nullptr);
    // Bind explicitly (bindConnection() is pre-existing API regardless of
    // whether rebuildRows() also does this internally) and simulate the Held
    // grip a real onDragStart would leave (mouse-down on the slider, never
    // released) -- the exact state ~UniversalParamControl()/bindConnection()
    // would otherwise dereference.
    pc->bindConnection(&clip.effects[0].paramConns[0], &clip.effects[0].paramLive[0]);
    clip.effects[0].paramConns[0].gripHeld();

    // The connection pc is bound to is now gone.
    clip.effects.clear();
    // Must not dereference the freed ParamConnection/LiveValue -- forgetting
    // every row's binding BEFORE removeChildComponent/rows_.clear() is what
    // makes this safe under ASan.
    view.setEffects(&clip.effects);

    // Regrow -- no crash, no stale pointer reused.
    clip.effects.push_back(Clip::EffectSlot{});
    clip.effects[0].effectName = "Ripple";
    clip.effects[0].addParam(0.3f);
    view.setEffects(&clip.effects);

    REQUIRE(view.paramControlForTest(0, 0) != nullptr);
}
