// SCRATCH (architect, ruling-bt2-seats): seat-amendment cases in MAIN's API only (initialiseWithDefaultDevices,
// DeviceReconciler(manager, 2, 2[, settle, bound]), reapplies(), getAudioDeviceSetup, the spy).
#include "fixtures2.inc"

namespace {
void rmDev(MockDeviceType& m, const juce::String& name)
{
    m.devs.erase(std::remove_if(m.devs.begin(), m.devs.end(), [&](const auto& d) { return d.name == name; }), m.devs.end());
}
int count(const juce::StringArray& a, const juce::String& n) { int c = 0; for (auto& x : a) if (x == n) ++c; return c; }
MockDevice* mockDevice(GuardedRig& rig) { return dynamic_cast<MockDevice*>(rig.manager->getCurrentAudioDevice()); }
}

TEST_CASE("M5r JUCE's own XML-branch re-init lands on an allowed device with its input -> the reconciler never re-applies on top of it", "[seats]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Mic", tp::BuiltIn, 1, 0 },
                   { "Speakers A", tp::BuiltIn, 0, 2 }, { "Speakers B", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    auto setup = rig.manager->getAudioDeviceSetup();   // explicit settings (JUCE's XML branch), as M5
    setup.inputChannels.setRange(0, 2, true);
    rig.manager->setAudioDeviceSetup(setup, true);
    pump(300);
    REQUIRE(rig.manager->createStateXml() != nullptr);
    const auto startsBefore = rig.spy.started.size();
    rmDev(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    pump(800);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK(reconciler.reapplies() == 0);
    CHECK(rig.spy.started.size() == startsBefore + 2);   // JUCE ONE re-open (a combined pair logs two names), nothing on top
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R14 input lost and the fallback cannot open -> only the still-listed output is put back; the lost input is never re-opened", "[seats]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Mic", tp::USB, 1, 0 },
                   { "Built-in Mic", tp::BuiltIn, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Mic"; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    REQUIRE(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    rig.spy.failOpen.add("Built-in Mic");
    rmDev(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    CHECK(count(rig.spy.opened, "USB Mic") == 1);   // the launch only
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R15 a listed mic that never opens: ONE adoption attempt; list changes that leave the allowed inputs unchanged never retry; re-plugged -> tried again", "[seats]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);   // only the progress guard can hold a retry back
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    REQUIRE(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    rig.spy.failOpen.add("USB Mic");
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    for (int i = 0; i < 5; ++i)   // a hidden device comes and goes (a display, a phone): the allowed inputs never change
    {
        rig.mock->devs.push_back({ "Other Wireless", tp::Bluetooth, 1, 2 });
        rig.mock->fireListChanged();
        pump(400);
        rmDev(*rig.mock, "Other Wireless");
        rig.mock->fireListChanged();
        pump(400);
    }
    CHECK(reconciler.reapplies() == 1);
    rig.spy.failOpen.clear();
    rmDev(*rig.mock, "USB Mic");        // the performer re-plugs it
    rig.mock->fireListChanged();
    pump(600);
    CHECK(reconciler.reapplies() == 1);
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 2);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
    CHECK_FALSE(rig.spy.saw("Other Wireless"));
}

TEST_CASE("R16 a cable jiggle: the open mic's device stops (JUCE's combiner shut down) and the mic is listed again inside the settle -> one re-apply; a later stop with no new scan waits", "[seats]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Mic", tp::USB, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Mic"; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    REQUIRE(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    rmDev(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(50);
    rig.manager->getCurrentAudioDevice()->stop();   // CA:1118-1127 -> the combiner's shutdown; the manager keeps it
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getCurrentAudioDevice()->isPlaying());
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
    rig.manager->getCurrentAudioDevice()->stop();   // dies again with NO device-list change: waits for a new scan
    pump(800);
    CHECK(reconciler.reapplies() == 1);
}

TEST_CASE("R17 JUCE restarts the open device itself (stop, 100 ms, start) -> no re-apply", "[seats]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Mic", tp::USB, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Mic"; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    rig.mock->fireListChanged();   // a new scan, so only the settle can hold an evaluation back
    pump(50);
    auto* d = mockDevice(rig);
    REQUIRE(d != nullptr);
    d->stopKeepingCallback();
    pump(100);
    d->startWithKeptCallback();
    pump(800);
    CHECK(reconciler.reapplies() == 0);
    CHECK(rig.manager->getCurrentAudioDevice() == d);
    CHECK(d->isPlaying());
}
