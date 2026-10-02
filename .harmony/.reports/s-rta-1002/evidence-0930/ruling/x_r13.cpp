// SCRATCH (architect ruling bt2): R13 adoption storm written in MAIN's API only (initialiseWithDefaultDevices, reapplies()).
#include "fixtures.inc"
TEST_CASE("R13 storm: 20 list changes while a mic appears (output-only) -> exactly one adoption, never before the settle", "[bt2]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    REQUIRE(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(100);
    CHECK(reconciler.reapplies() == 0);
    for (int i = 1; i < 20; ++i) { rig.mock->fireListChanged(); pump(20); }
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}
