// SCRATCH (architect, plan-bt2): RED predictions on main HEAD 655d232's DeviceGuard / DevicePolicy objects.
// Uses ONLY main's API (GuardedRig, MockDeviceType, DeviceReconciler(manager, 2, 2), reapplies()).
#include "fixtures.inc"

namespace {
void rmDev(MockDeviceType& m, const juce::String& name)
{
    m.devs.erase(std::remove_if(m.devs.begin(), m.devs.end(), [&](const auto& d) { return d.name == name; }), m.devs.end());
}
}

TEST_CASE("XA1 no allowed input at launch (output-only); a wired mic appears -> adopted once", "[bt2]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    REQUIRE(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("XA0 nothing allowed at launch; a built-in pair appears -> adopted once", "[bt2]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    REQUIRE(rig.manager->getCurrentAudioDevice() == nullptr);
    rig.mock->devs.push_back({ "Built-in Mic", tp::BuiltIn, 1, 0 });
    rig.mock->devs.push_back({ "Built-in Speakers", tp::BuiltIn, 0, 2 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("XA2 the opened wired input vanishes while its output stays -> the policy is re-applied (built-in mic)", "[bt2]")
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
    rmDev(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(800);
    INFO("input after: " << rig.manager->getAudioDeviceSetup().inputDeviceName
         << " device " << (rig.manager->getCurrentAudioDevice() != nullptr ? "yes" : "no"));
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("XK1 a working input is never switched: a USB mic appears (and becomes the macOS default)", "[bt2]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { "Built-in Mic", tp::BuiltIn, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    const auto startsBefore = rig.spy.started.size();
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->defaultIn = "USB Mic";
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 0);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK(rig.spy.started.size() == startsBefore);
}

TEST_CASE("XM MONO mic, the app's launch WITHOUT the setSourceMode re-open (C3): output vanishes -> JUCE alone ends with no device; the reconciler recovers once", "[bt2]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Mic", tp::BuiltIn, 1, 0 },
                   { "Speakers A", tp::BuiltIn, 0, 2 }, { "Speakers B", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(300);
    CHECK(rig.manager->createStateXml() == nullptr);
    REQUIRE(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers A");
    rmDev(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);   // JUCE's own re-init: "No such device"
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("XO opens: the app's launch = 1 start; the setSourceMode re-open replica = a 2nd start of the same device", "[bt2]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { "Built-in Mic", tp::BuiltIn, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    });
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    auto n = [&rig] { int c = 0; for (auto& x : rig.spy.started) if (x == "Built-in Speakers") ++c; return c; };
    const auto afterLaunch = n();
    auto setup = rig.manager->getAudioDeviceSetup();
    setup.inputChannels.setRange(0, 2, true);
    rig.manager->setAudioDeviceSetup(setup, true);          // AudioEngine.cpp:139-141 replica (mic mode)
    const auto afterMic = n();
    auto s2 = rig.manager->getAudioDeviceSetup();
    s2.inputChannels.clear();
    rig.manager->setAudioDeviceSetup(s2, true);             // AudioEngine.cpp:151-153 replica (file mode)
    const auto afterFile = n();
    auto s3 = rig.manager->getAudioDeviceSetup();
    s3.inputChannels.setRange(0, 2, true);
    rig.manager->setAudioDeviceSetup(s3, true);
    const auto afterMic2 = n();
    INFO("started: launch " << (int) afterLaunch << " mic " << (int) afterMic << " file " << (int) afterFile << " mic2 " << (int) afterMic2
         << " active ins after file " << rig.manager->getCurrentAudioDevice()->getActiveInputChannels().toString(2));
    CHECK(afterLaunch == 1);
    CHECK(afterMic == 2);
    CHECK(afterFile == 3);
    CHECK(afterMic2 == 4);
    CHECK(rig.manager->createStateXml() != nullptr);
}
