// SCRATCH (architect, plan-bt2): cases that use the NEW API (openDefaultDevices, dp::reconcile) -- guards, not RED-on-main.
#include "fixtures2.inc"

namespace {
void rmDev2(MockDeviceType& m, const juce::String& name)
{
    m.devs.erase(std::remove_if(m.devs.begin(), m.devs.end(), [&](const auto& d) { return d.name == name; }), m.devs.end());
}
}

TEST_CASE("XA1f the appeared mic cannot open -> the output-only device is put back; retried only on a new scan", "[bt2new]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(300);
    rig.spy.failOpen.add("USB Mic");
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    pump(800);
    CHECK(reconciler.reapplies() == 1);          // its own change messages never re-trigger it
    rig.spy.failOpen.clear();
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 2);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

