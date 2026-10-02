// SCRATCH (architect, ruling-bt2-seats): seat-amendment cases that use the NEW API (openDefaultDevices, dp::reconcile,
// lastAction, lostInput) -- guards on the lane, not RED-on-main.
#include "fixtures2.inc"

namespace {
void rmDev2(MockDeviceType& m, const juce::String& name)
{
    m.devs.erase(std::remove_if(m.devs.begin(), m.devs.end(), [&](const auto& d) { return d.name == name; }), m.devs.end());
}
}

TEST_CASE("P14s the reconcile table (liveness row included)", "[seatsnew]")
{
    dp::Lists l;
    l.outputs.add("Spk");
    CHECK(dp::reconcile(true, true, "Mic", l) == dp::Reapply::InputLost);
    CHECK(dp::reconcile(true, false, "Mic", l) == dp::Reapply::InputLost);   // name first: the probe's code
    l.inputs.add("Mic");
    CHECK(dp::reconcile(true, true, "Mic", l) == dp::Reapply::None);
    CHECK(dp::reconcile(true, false, "Mic", l) == dp::Reapply::DeviceStopped);
    CHECK(dp::reconcile(true, true, "", l) == dp::Reapply::AdoptInput);
    CHECK(dp::reconcile(true, false, "", l) == dp::Reapply::DeviceStopped);
    CHECK(dp::reconcile(false, false, "", l) == dp::Reapply::NoDevice);
    dp::Lists empty;
    CHECK(dp::reconcile(true, true, "", empty) == dp::Reapply::None);
    CHECK(dp::reconcile(true, false, "", empty) == dp::Reapply::None);
    CHECK(dp::reconcile(false, false, "", empty) == dp::Reapply::None);
    dp::Lists outOnly; outOnly.outputs.add("Spk");
    CHECK(dp::reconcile(false, false, "", outOnly) == dp::Reapply::NoDevice);
    CHECK(dp::reconcile(true, true, "", outOnly) == dp::Reapply::None);
    CHECK(dp::toString(dp::Reapply::None).isEmpty());
    CHECK(dp::toString(dp::Reapply::NoDevice) == "no-device");
    CHECK(dp::toString(dp::Reapply::AdoptInput) == "adopt-input");
    CHECK(dp::toString(dp::Reapply::InputLost) == "input-lost");
    CHECK(dp::toString(dp::Reapply::DeviceStopped) == "device-stopped");
}

TEST_CASE("P15 the TEST-ONLY deny-all token hides every device", "[seatsnew]")
{
    dp::Config cfg;
    cfg.testDeniedNames.add("*");
    const auto v = dp::classify(dev("Built-in Mic", tp::BuiltIn, true, false), cfg);
    CHECK_FALSE(v.allowed);
    CHECK(v.reason == "test-denied");
    const std::vector<dp::DeviceInfo> scan = { dev("Built-in Mic", tp::BuiltIn, true, false, true, false),
                                               dev("Built-in Speakers", tp::BuiltIn, false, true, false, true) };
    const auto lists = filterScan(scan, cfg);
    CHECK(lists.inputs.isEmpty());
    CHECK(lists.outputs.isEmpty());
}

TEST_CASE("R11s a mic that cannot open -> the output-only device is put back; the SAME inputs are not retried; re-plugged -> adopted", "[seatsnew]")
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
    CHECK(reconciler.reapplies() == 1);
    rig.spy.failOpen.clear();
    rig.mock->fireListChanged();          // same allowed inputs: not retried (the progress guard)
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    rmDev2(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(600);
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 2);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK(reconciler.lastAction() == dp::Reapply::AdoptInput);
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R18 the fallback is named: input lost -> the built-in mic; lostInput() names the lost mic and survives Keep", "[seatsnew]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Mic", tp::USB, 1, 0 },
                   { "Built-in Mic", tp::BuiltIn, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Mic"; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(300);
    CHECK(reconciler.lostInput().isEmpty());
    rmDev2(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(reconciler.lastAction() == dp::Reapply::InputLost);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK(reconciler.lostInput() == "USB Mic");
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });   // re-plugged: Keep (the working built-in mic stays)
    rig.mock->defaultIn = "USB Mic";
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK(reconciler.lostInput() == "USB Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R12s round trip: input lost -> output-only (lostInput empty: the no-input notice speaks); the mic returns -> adopted", "[seatsnew]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Mic", tp::USB, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Mic"; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(300);
    rmDev2(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    CHECK(reconciler.lostInput().isEmpty());
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 2);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK(reconciler.lostInput().isEmpty());
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M8s the launch with a hidden default opens the device ONCE, records no explicit settings, never the hidden one", "[seatsnew]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig(addHeadsetAndBuiltIn);
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(600);
    int starts = 0;
    for (auto& x : rig.spy.started) if (x == "Built-in Speakers") ++starts;
    CHECK(starts == 1);
    CHECK(rig.spy.started.size() == 2);   // one start: the combined pair's two names
    CHECK(rig.manager->createStateXml() == nullptr);
    CHECK(reconciler.reapplies() == 0);
    CHECK_FALSE(rig.spy.saw(kDenied));
}

#if JUCE_MAC
TEST_CASE("E3s the real CoreAudio type behind the decorator with the deny-all token lists NOTHING (read-only)", "[seatsnew]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    std::unique_ptr<juce::AudioIODeviceType> ca(juce::AudioIODeviceType::createAudioIODeviceType_CoreAudio());
    REQUIRE(ca != nullptr);
    dp::Config cfg;
    cfg.testDeniedNames.add("*");
    GuardedDeviceType guarded(std::move(ca), &enumerateCoreAudioDevices, cfg);
    guarded.scanForDevices();
    CHECK(guarded.getDeviceNames(true).isEmpty());
    CHECK(guarded.getDeviceNames(false).isEmpty());
    CHECK(guarded.createDevice("MacBook Pro Speakers", "") == nullptr);
}
#endif
