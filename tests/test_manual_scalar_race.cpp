// test_manual_scalar_race -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T0 R3, ruling
// ruling-tsan.md amendment 2). The manual scalars (the 23 fields manualRef names) are written on the message thread
// through the real funnel (resolveControl + manualWriteCore) while the GL thread reads them through the real eff()
// (Layer::eff / Clip::eff / Composition::eff). Family D of the s-rta-0929b TSan sweep (Layer.cpp eff vs
// ManualWrite.cpp manualWriteCore).
// Registered with LABELS tsan + FAIL_REGULAR_EXPRESSION "WARNING: ThreadSanitizer" (tests/CMakeLists.txt): a single
// data race fails the case in a -DADNA_SANITIZE=thread build; in a normal build only the value checks bite. Run under
// TSan: .harmony/probe-tsan-unit.sh.
#include <catch2/catch_test_macros.hpp>
#include "connect/ManualWrite.h"
#include "connect/ScalarParams.h"
#include "model/Composition.h"
#include "model/ControlPath.h"
#include "routing/MacroBank.h"
#include <atomic>
#include <cmath>
#include <optional>
#include <thread>
#include <vector>

namespace
{
ControlPath compScalar(const std::string& key)
{
    ControlPath p; p.scope = ControlPath::Scope::Comp; p.control = "scalar"; p.scalar = key; return p;
}
ControlPath layerScalar(int deck, int layer, const std::string& key)
{
    ControlPath p; p.scope = ControlPath::Scope::Layer; p.deck = deck; p.layer = layer;
    p.control = "scalar"; p.scalar = key; return p;
}
ControlPath clipScalar(int deck, int layer, int col, const std::string& key)
{
    ControlPath p; p.scope = ControlPath::Scope::Clip; p.deck = deck; p.layer = layer; p.col = col;
    p.control = "scalar"; p.scalar = key; return p;
}
} // namespace

TEST_CASE("R3 manual scalar writes vs eff() reads", "[tsan][manual_scalar]")
{
    Composition comp;
    comp.initDefault();   // one deck, 3 layers
    comp.decks[0].setClip(0, 0, Clip{});
    MacroBank bank;
    const std::vector<ControlPath> paths = { layerScalar(0, 0, "opacity"), layerScalar(0, 0, "positionX"),
                                             clipScalar(0, 0, 0, "opacity"), compScalar("opacity"),
                                             compScalar("speed") };
    for (const auto& p : paths)
        REQUIRE(resolveControl(comp, bank, p).has_value());

    Layer& layer = comp.decks[0].layers[0];
    const Clip* clip = comp.decks[0].getClip(0, 0);
    REQUIRE(clip != nullptr);

    std::atomic<bool> go{ false }, stop{ false };
    std::atomic<long> renderFrames{ 0 };
    std::atomic<int> nonFinite{ 0 };

    std::thread render([&] {
        while (!go.load()) std::this_thread::yield();
        while (!stop.load())
        {
            const float values[] = { layer.eff(LayerScalar::Opacity), layer.eff(LayerScalar::PosX),
                                     clip->eff(ClipScalar::Opacity), comp.eff(CompScalar::Opacity),
                                     comp.eff(CompScalar::Speed) };
            for (float v : values)
                if (!std::isfinite(v)) nonFinite.fetch_add(1);
            renderFrames.fetch_add(1);
        }
    });

    go.store(true);
    constexpr int kIterations = 40000;
    int refused = 0;
    for (int i = 0; i < kIterations; ++i)
    {
        const float norm = static_cast<float>(i % 101) / 100.0f;
        for (const auto& p : paths)
        {
            auto ref = resolveControl(comp, bank, p);
            if (!ref || !manualWriteCore(*ref, norm, Hand::HumanDecaying, ParamConnection::Grip::Kind::Decaying,
                                         static_cast<double>(i), 250.0f))
                ++refused;
        }
    }
    stop.store(true);
    render.join();

    INFO("render frames " << renderFrames.load() << ", refused writes " << refused);
    CHECK(renderFrames.load() > 0);
    CHECK(refused == 0);
    CHECK(nonFinite.load() == 0);
}
