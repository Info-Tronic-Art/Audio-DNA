// SCRATCH (architect, plan-bt2): cases that use the NEW API (openDefaultDevices, dp::reconcile) -- guards, not RED-on-main.
#include "fixtures.inc"

namespace {
void rmDev2(MockDeviceType& m, const juce::String& name)
{
    m.devs.erase(std::remove_if(m.devs.begin(), m.devs.end(), [&](const auto& d) { return d.name == name; }), m.devs.end());
}
}

TEST_CASE("XP the pure reconcile table", "[bt2new]")
{
    dp::Lists l;
    CHECK(dp::reconcile(true, "Mic", l) == dp::Reapply::InputLost);
    l.inputs.add("Mic");
    CHECK(dp::reconcile(true, "Mic", l) == dp::Reapply::None);
    CHECK(dp::reconcile(true, "", l) == dp::Reapply::AdoptInput);
    CHECK(dp::reconcile(false, "", l) == dp::Reapply::NoDevice);
    dp::Lists empty;
    CHECK(dp::reconcile(true, "", empty) == dp::Reapply::None);
    CHECK(dp::reconcile(false, "", empty) == dp::Reapply::None);
    dp::Lists outOnly; outOnly.outputs.add("Spk");
    CHECK(dp::reconcile(false, "", outOnly) == dp::Reapply::NoDevice);
    CHECK(dp::reconcile(true, "", outOnly) == dp::Reapply::None);
}

TEST_CASE("XL the launch open failed while an allowed device is listed -> no re-apply until a NEW scan", "[bt2new]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { "Built-in Mic", tp::BuiltIn, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    rig.spy.failOpen.add("Built-in Speakers");
    CHECK(reconciler.openDefaultDevices().isNotEmpty());
    pump(800);
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);
    CHECK(reconciler.reapplies() == 0);          // the launch's own scan is never re-tried by itself
    rig.spy.failOpen.clear();
    rig.mock->fireListChanged();                 // a new scan
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getCurrentAudioDevice() != nullptr);
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

TEST_CASE("XS storm: 20 list changes while a mic appears (no input) -> exactly one adoption, never before the settle", "[bt2new]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(300);
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(100);
    CHECK(reconciler.reapplies() == 0);
    for (int i = 1; i < 20; ++i) { rig.mock->fireListChanged(); pump(20); }
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
}

TEST_CASE("XC the app's launch after C3 opens the device ONCE (one start) and records no explicit settings", "[bt2new]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { "Built-in Mic", tp::BuiltIn, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(600);
    int starts = 0;
    for (auto& x : rig.spy.started) if (x == "Built-in Speakers") ++starts;
    CHECK(starts == 1);
    CHECK(rig.manager->createStateXml() == nullptr);
    CHECK(reconciler.reapplies() == 0);
}

TEST_CASE("XR adoption round trip: input lost -> output-only; the mic returns -> adopted (Keep never fires while it works)", "[bt2new]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Mic", tp::USB, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Mic"; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(300);
    REQUIRE(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    rmDev2(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());   // no onboard mic here: output-only + notice
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 2);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}
