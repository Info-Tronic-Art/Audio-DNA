// test_device_policy -- s-rta-0929b btguard (plan-btguard 4.9 + HARMONY ADOPTION BG1/BG2/BG4/BG5): the app never opens a
// Bluetooth (or other wireless) audio device.
//   P*  the pure policy (DevicePolicy.h): classify / filter / the default index.
//   D*  the decorator (GuardedDeviceType) over a mock inner device type.
//   M*  a REAL juce::AudioDeviceManager (GuardedAudioDeviceManager) over the mock: every manager case asserts, through
//       the inner SPY (every createDevice / open / start name, JUCE's temporary devices included), that no denied name
//       was ever created; the unguarded control M2 asserts the spy DID see it (the JUCE behaviour the guard exists for).
//   R*  the DeviceReconciler (BG4; s-rta-0930 bt2: adopt / input lost / device stopped, R6-R18) over the same manager.
//   E*  the real HAL (macOS): read-only property queries, no device is opened.
// Fix round (btguard-fix): D1 / D4 / M6 / M7 put a hidden device BEFORE the default so the inner and the filtered default
// index differ (JUCE moves getDefaultDeviceIndex to the front); every case added there was RED on a mutant of the code
// it guards (.harmony/.reports/s-rta-0929b/btguard.md "Fix round").
// Fixture names never name a real product: "Denied Wireless" stands for any Bluetooth headset.
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_events/juce_events.h>
#include "audio/DevicePolicy.h"
#include "audio/DeviceGuard.h"
#include "audio/CoreAudioDeviceInfo.h"
#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace dp = audiodna::devpolicy;
namespace tp = audiodna::devpolicy::transport;

namespace
{
dp::DeviceInfo dev(const juce::String& name, uint32_t transport, bool in, bool out,
                   bool defaultIn = false, bool defaultOut = false)
{
    dp::DeviceInfo d;
    d.inputName = in ? name : juce::String();
    d.outputName = out ? name : juce::String();
    d.uid = name + "-uid";
    d.transport = transport;
    d.isDefaultInput = defaultIn;
    d.isDefaultOutput = defaultOut;
    return d;
}

juce::StringArray namesOf(const std::vector<dp::DeviceInfo>& scan, bool input)
{
    juce::StringArray names;
    for (const auto& d : scan)
        if ((input ? d.inputName : d.outputName).isNotEmpty())
            names.add(input ? d.inputName : d.outputName);
    return names;
}

dp::Lists filterScan(const std::vector<dp::DeviceInfo>& scan, const dp::Config& cfg = {})
{
    return dp::filter(namesOf(scan, true), namesOf(scan, false), scan, cfg);
}

// ---- the inner spy ---------------------------------------------------------------------------------------------------
struct Spy
{
    juce::StringArray created, opened, started;   // every name, temporary devices included
    juce::StringArray createdPairs;                 // "<out> + <in>" per duplex createDevice (JUCE's probes are one-way)
    juce::StringArray failOpen;                     // names whose open() fails (an unopenable device)
    std::function<void()> onStart;                  // runs inside start() (a list change mid-open, attack S7)
    bool saw(const juce::String& name) const
    {
        return created.contains(name) || opened.contains(name) || started.contains(name);
    }
};

class MockDevice final : public juce::AudioIODevice
{
public:
    MockDevice(const juce::String& name, const juce::String& outName, const juce::String& inName, int ins, int outs, Spy& spy)
        : juce::AudioIODevice(name, "CoreAudio"), outName_(outName), inName_(inName), ins_(ins), outs_(outs), spy_(spy)
    {}
    juce::StringArray getOutputChannelNames() override { return channelNames(outs_, "Out"); }
    juce::StringArray getInputChannelNames() override { return channelNames(ins_, "In"); }
    juce::Array<double> getAvailableSampleRates() override { return { 44100.0, 48000.0 }; }
    juce::Array<int> getAvailableBufferSizes() override { return { 512 }; }
    int getDefaultBufferSize() override { return 512; }
    juce::String open(const juce::BigInteger& in, const juce::BigInteger& out, double rate, int) override
    {
        record(spy_.opened);
        if ((outName_.isNotEmpty() && spy_.failOpen.contains(outName_)) || (inName_.isNotEmpty() && spy_.failOpen.contains(inName_)))
            return "mock: cannot open";
        activeIns_ = in & mask(ins_);
        activeOuts_ = out & mask(outs_);
        rate_ = rate > 0.0 ? rate : 48000.0;
        open_ = true;
        return {};
    }
    void close() override { open_ = false; playing_ = false; }
    bool isOpen() override { return open_; }
    void start(juce::AudioIODeviceCallback* cb) override
    {
        record(spy_.started);
        playing_ = true;
        if (cb != nullptr)
            cb->audioDeviceAboutToStart(this);
        callback_ = cb;
        if (spy_.onStart)
        {
            auto f = std::move(spy_.onStart);   // once
            f();
        }
    }
    void stop() override
    {
        if (callback_ != nullptr && playing_)
            callback_->audioDeviceStopped();
        playing_ = false;
        callback_ = nullptr;
    }
    bool isPlaying() override { return playing_; }
    juce::String getLastError() override { return {}; }
    int getCurrentBufferSizeSamples() override { return 512; }
    double getCurrentSampleRate() override { return rate_; }
    int getCurrentBitDepth() override { return 32; }
    juce::BigInteger getActiveOutputChannels() const override { return activeOuts_; }
    juce::BigInteger getActiveInputChannels() const override { return activeIns_; }
    int getOutputLatencyInSamples() override { return 0; }
    int getInputLatencyInSamples() override { return 0; }
    ~MockDevice() override { stop(); }
    // TEST (bt2 AM13): JUCE's own restart of an open device -- stop (the manager is told), then start with the SAME callback.
    void stopKeepingCallback() { keptCallback_ = callback_; stop(); }
    void startWithKeptCallback() { start(keptCallback_); }

private:
    static juce::StringArray channelNames(int n, const juce::String& prefix)
    {
        juce::StringArray s;
        for (int i = 0; i < n; ++i)
            s.add(prefix + " " + juce::String(i + 1));
        return s;
    }
    static juce::BigInteger mask(int n)
    {
        juce::BigInteger b;
        if (n > 0)
            b.setRange(0, n, true);
        return b;
    }
    void record(juce::StringArray& into) const
    {
        if (outName_.isNotEmpty()) into.add(outName_);
        if (inName_.isNotEmpty()) into.add(inName_);
    }

    juce::String outName_, inName_;
    int ins_, outs_;
    Spy& spy_;
    juce::BigInteger activeIns_, activeOuts_;
    double rate_ = 48000.0;
    bool open_ = false, playing_ = false;
    juce::AudioIODeviceCallback* callback_ = nullptr;
    juce::AudioIODeviceCallback* keptCallback_ = nullptr;
};

// The inner type: JUCE's CoreAudio shape (named "CoreAudio", separate in/out lists, default = the "macOS default").
class MockDeviceType final : public juce::AudioIODeviceType
{
public:
    struct Dev
    {
        juce::String name;
        uint32_t transport;
        int ins, outs;
    };

    explicit MockDeviceType(Spy& spy) : juce::AudioIODeviceType("CoreAudio"), spy_(spy) {}

    std::vector<Dev> devs;
    juce::String defaultIn, defaultOut;

    void scanForDevices() override
    {
        ins_.clear();
        outs_.clear();
        for (const auto& d : devs)
        {
            if (d.ins > 0) ins_.add(d.name);
            if (d.outs > 0) outs_.add(d.name);
        }
    }
    juce::StringArray getDeviceNames(bool wantInputNames) const override { return wantInputNames ? ins_ : outs_; }
    int getDefaultDeviceIndex(bool forInput) const override
    {
        return juce::jmax(0, (forInput ? ins_ : outs_).indexOf(forInput ? defaultIn : defaultOut));
    }
    int getIndexOfDevice(juce::AudioIODevice* d, bool asInput) const override
    {
        return d == nullptr ? -1 : (asInput ? ins_ : outs_).indexOf(d->getName());
    }
    bool hasSeparateInputsAndOutputs() const override { return true; }
    juce::AudioIODevice* createDevice(const juce::String& out, const juce::String& in) override
    {
        if (out.isNotEmpty()) spy_.created.add(out);
        if (in.isNotEmpty()) spy_.created.add(in);
        if (out.isNotEmpty() && in.isNotEmpty()) spy_.createdPairs.add(out + " + " + in);
        const Dev* o = find(out, false);
        const Dev* i = find(in, true);
        if (o == nullptr && i == nullptr)
            return nullptr;
        return new MockDevice(out.isEmpty() ? in : out, o != nullptr ? out : juce::String(), i != nullptr ? in : juce::String(),
                              i != nullptr ? i->ins : 0, o != nullptr ? o->outs : 0, spy_);
    }
    void fireListChanged()   // what CoreAudioIODeviceType::audioDeviceListChanged does
    {
        scanForDevices();
        callDeviceChangeListeners();
    }
    // The enumerator the decorator reads: the same devices with their transports (unique names in these fixtures).
    std::vector<dp::DeviceInfo> infos() const
    {
        std::vector<dp::DeviceInfo> v;
        for (const auto& d : devs)
            v.push_back(dev(d.name, d.transport, d.ins > 0, d.outs > 0, d.name == defaultIn, d.name == defaultOut));
        return v;
    }

private:
    const Dev* find(const juce::String& name, bool input) const
    {
        if (name.isEmpty() || !(input ? ins_ : outs_).contains(name))
            return nullptr;
        for (const auto& d : devs)
            if (d.name == name)
                return &d;
        return nullptr;
    }
    Spy& spy_;
    juce::StringArray ins_, outs_;
};

constexpr const char* kDenied = "Denied Wireless";

// A headset (in + out) that is the "macOS default" both ways, plus the built-in mic and speakers.
void addHeadsetAndBuiltIn(MockDeviceType& m)
{
    m.devs = { { kDenied, tp::Bluetooth, 1, 2 },
               { "Built-in Mic", tp::BuiltIn, 1, 0 },
               { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    m.defaultIn = kDenied;
    m.defaultOut = kDenied;
}

// A GuardedAudioDeviceManager whose inner type is the mock; `mock` is set when JUCE creates the types.
struct GuardedRig
{
    Spy spy;
    MockDeviceType* mock = nullptr;
    std::function<void(MockDeviceType&)> setup;
    std::unique_ptr<GuardedAudioDeviceManager> manager;

    explicit GuardedRig(std::function<void(MockDeviceType&)> s) : setup(std::move(s))
    {
        manager = std::make_unique<GuardedAudioDeviceManager>(
            dp::Config{}, [this] { return mock != nullptr ? mock->infos() : std::vector<dp::DeviceInfo>{}; });
        manager->typeFactoryForTests = [this](juce::OwnedArray<juce::AudioIODeviceType>& types) {
            auto m = std::make_unique<MockDeviceType>(spy);
            setup(*m);
            mock = m.get();
            types.add(m.release());
        };
    }
};

void pump(int ms)
{
    juce::MessageManager::getInstance()->runDispatchLoopUntil(ms);
}

void removeDevice(MockDeviceType& m, const juce::String& name)
{
    m.devs.erase(std::remove_if(m.devs.begin(), m.devs.end(), [&](const auto& d) { return d.name == name; }), m.devs.end());
}

int count(const juce::StringArray& a, const juce::String& n)
{
    int c = 0;
    for (auto& x : a)
        if (x == n)
            ++c;
    return c;
}

MockDevice* mockDevice(GuardedRig& rig)
{
    return dynamic_cast<MockDevice*>(rig.manager->getCurrentAudioDevice());
}
}

// ======================================================================================================================
// P -- the pure policy
// ======================================================================================================================

TEST_CASE("P1 a Bluetooth default in + out is hidden; the defaults point at the built-in devices", "[device_policy]")
{
    const std::vector<dp::DeviceInfo> scan = { dev(kDenied, tp::Bluetooth, true, true, true, true),
                                               dev("Built-in Mic", tp::BuiltIn, true, false),
                                               dev("Built-in Speakers", tp::BuiltIn, false, true) };
    const auto l = filterScan(scan);
    CHECK(l.inputs == juce::StringArray("Built-in Mic"));
    CHECK(l.outputs == juce::StringArray("Built-in Speakers"));
    CHECK(l.defaultInput == 0);
    CHECK(l.defaultOutput == 0);
    REQUIRE(l.skipped.size() == 1);
    CHECK(l.skipped[0].name == kDenied);
    CHECK(l.skipped[0].reason == "bluetooth");
    CHECK(l.skipped[0].input);
    CHECK(l.skipped[0].output);
    CHECK(l.unmapped.isEmpty());
}

TEST_CASE("P2 a wired USB macOS default stays the default (not the first allowed)", "[device_policy]")
{
    const std::vector<dp::DeviceInfo> scan = { dev("Built-in Mic", tp::BuiltIn, true, false),
                                               dev("Virtual Loop", tp::Virtual, true, true),
                                               dev("USB Interface", tp::USB, true, true, true, true) };
    const auto l = filterScan(scan);
    REQUIRE(l.inputs.size() == 3);
    CHECK(l.inputs[l.defaultInput] == "USB Interface");
    CHECK(l.outputs[l.defaultOutput] == "USB Interface");
    CHECK(l.skipped.empty());
}

TEST_CASE("P3 an aggregate is allowed only when every member is allowed", "[device_policy]")
{
    auto agg = [](const juce::String& name, std::vector<dp::DeviceInfo::Member> members) {
        auto d = dev(name, tp::Aggregate, true, true);
        d.members = std::move(members);
        return d;
    };
    const dp::DeviceInfo::Member usb { "usb-uid", tp::USB, true, true };
    const dp::DeviceInfo::Member builtIn { "BuiltInSpeakerDevice", tp::BuiltIn, true, true };
    const dp::DeviceInfo::Member bt { "bt-member-uid", tp::Bluetooth, true, true };

    CHECK(dp::classify(agg("Wired Agg", { usb, builtIn }), {}).allowed);
    const auto v = dp::classify(agg("Mixed Agg", { usb, bt }), {});
    CHECK_FALSE(v.allowed);
    CHECK(v.reason == "aggregate:bluetooth");

    auto autoAgg = agg("Auto Agg", { usb, bt });
    autoAgg.transport = tp::AutoAggregate;
    CHECK(dp::classify(autoAgg, {}).reason == "aggregate:bluetooth");
}

TEST_CASE("P3b an aggregate with an unresolvable or Bluetooth-shaped member UID is denied (fail closed)", "[device_policy]")
{
    auto d = dev("Multi-Output Device", tp::Aggregate, false, true);
    d.members = { { "usb-uid", tp::USB, true, true }, { "gone-uid", 0, false, true } };
    auto v = dp::classify(d, {});
    CHECK_FALSE(v.allowed);
    CHECK(v.reason == "aggregate:member-unresolved");

    d.members = { { "usb-uid", tp::USB, true, true }, { "AC-80-0A-3F-9B-12:output", 0, false, true } };
    v = dp::classify(d, {});
    CHECK_FALSE(v.allowed);
    CHECK(v.reason == "aggregate:bluetooth-uid");

    d.members = { { "usb-uid", tp::USB, true, true } };
    d.membersReadOk = false;
    CHECK(dp::classify(d, {}).reason == "aggregate:members-unreadable");

    CHECK(dp::hasBluetoothUidShape("ac:80:0a:3f:9b:12"));
    CHECK(dp::hasBluetoothUidShape("AC-80-0A-3F-9B-12:input"));
    CHECK_FALSE(dp::hasBluetoothUidShape("BuiltInSpeakerDevice"));
    CHECK_FALSE(dp::hasBluetoothUidShape("AC-80-0A-3F-9B-12-extra"));
}

TEST_CASE("P4 only Bluetooth devices -> empty lists, defaults 0", "[device_policy]")
{
    const std::vector<dp::DeviceInfo> scan = { dev(kDenied, tp::Bluetooth, true, true, true, true),
                                               dev("Denied Speaker", tp::Bluetooth, false, true) };
    const auto l = filterScan(scan);
    CHECK(l.inputs.isEmpty());
    CHECK(l.outputs.isEmpty());
    CHECK(l.defaultInput == 0);
    CHECK(l.defaultOutput == 0);
    CHECK(l.skipped.size() == 2);
}

TEST_CASE("P5 no devices -> empty lists", "[device_policy]")
{
    const auto l = filterScan({});
    CHECK(l.inputs.isEmpty());
    CHECK(l.outputs.isEmpty());
    CHECK(l.skipped.empty());
    CHECK(l.unmapped.isEmpty());
}

TEST_CASE("P6 AirPlay, wireless Continuity and Continuity Capture are denied; wired Continuity, Virtual, Unknown allowed",
          "[device_policy]")
{
    CHECK(dp::classify(dev("A", tp::AirPlay, false, true), {}).reason == "airplay");
    CHECK(dp::classify(dev("B", tp::ContinuityWireless, true, false), {}).reason == "wireless-continuity");
    CHECK(dp::classify(dev("C", tp::ContinuityCapture, true, false), {}).reason == "continuity-capture");
    CHECK(dp::classify(dev("D", tp::ContinuityWired, true, false), {}).allowed);
    CHECK(dp::classify(dev("E", tp::Virtual, true, true), {}).allowed);
    CHECK(dp::classify(dev("F", tp::Unknown, true, true), {}).allowed);
    for (auto t : { tp::BuiltIn, tp::USB, tp::Thunderbolt, tp::PCI, tp::FireWire, tp::HDMI, tp::DisplayPort, tp::AVB })
        CHECK(dp::classify(dev("W", t, true, true), {}).allowed);
}

TEST_CASE("P7 Bluetooth LE is denied", "[device_policy]")
{
    const auto v = dp::classify(dev("LE", tp::BluetoothLE, true, true), {});
    CHECK_FALSE(v.allowed);
    CHECK(v.reason == "bluetooth-le");
}

TEST_CASE("P8 TEST-ONLY testDeniedNames hides a built-in device by exact name", "[device_policy]")
{
    dp::Config cfg;
    cfg.testDeniedNames.add("Built-in Mic");
    const std::vector<dp::DeviceInfo> scan = { dev("Built-in Mic", tp::BuiltIn, true, false, true, false),
                                               dev("Built-in Speakers", tp::BuiltIn, false, true, false, true) };
    const auto l = filterScan(scan, cfg);
    CHECK(l.inputs.isEmpty());
    CHECK(l.outputs == juce::StringArray("Built-in Speakers"));
    REQUIRE(l.skipped.size() == 1);
    CHECK(l.skipped[0].reason == "test-denied");
    CHECK(l.skipped[0].input);
    CHECK_FALSE(l.skipped[0].output);
}

TEST_CASE("P9 duplicate names map by the deduped name: the USB 'Scarlett' stays, the Bluetooth 'Scarlett (2)' is hidden",
          "[device_policy]")
{
    const std::vector<dp::DeviceInfo> scan = { dev("Scarlett", tp::USB, true, true),
                                               dev("Scarlett (2)", tp::Bluetooth, true, true, true, true) };
    const auto l = filterScan(scan);
    CHECK(l.inputs == juce::StringArray("Scarlett"));
    CHECK(l.outputs == juce::StringArray("Scarlett"));
    REQUIRE(l.skipped.size() == 1);
    CHECK(l.skipped[0].name == "Scarlett (2)");
}

TEST_CASE("P10 an inner name with no DeviceInfo is unmapped and hidden (fail closed)", "[device_policy]")
{
    const std::vector<dp::DeviceInfo> scan = { dev("Built-in Mic", tp::BuiltIn, true, false) };
    const auto l = dp::filter(juce::StringArray("Built-in Mic", "Appeared Later"), {}, scan, {});
    CHECK(l.inputs == juce::StringArray("Built-in Mic"));
    CHECK(l.unmapped == juce::StringArray("Appeared Later"));
}

TEST_CASE("P11 a transport READ ERROR fails closed; a reported Unknown (0) stays allowed", "[device_policy]")
{
    auto broken = dev("Mid Teardown", tp::Unknown, true, true, true, true);
    broken.transportReadOk = false;
    const auto v = dp::classify(broken, {});
    CHECK_FALSE(v.allowed);
    CHECK(v.reason == "transport-unreadable");

    const std::vector<dp::DeviceInfo> scan = { broken, dev("Built-in Mic", tp::BuiltIn, true, false) };
    const auto l = filterScan(scan);
    CHECK(l.inputs == juce::StringArray("Built-in Mic"));
    CHECK(dp::classify(dev("Reported Unknown", tp::Unknown, true, true), {}).allowed);
}

TEST_CASE("P12 no allowed macOS default and no built-in: the first USB device is the default, never an earlier HDMI",
          "[device_policy]")
{
    const std::vector<dp::DeviceInfo> scan = { dev("Display Audio", tp::HDMI, false, true),
                                               dev("USB Speakers", tp::USB, false, true),
                                               dev(kDenied, tp::Bluetooth, false, true, false, true) };
    const auto l = filterScan(scan);
    REQUIRE(l.outputs.size() == 2);
    CHECK(l.outputs[l.defaultOutput] == "USB Speakers");
}

TEST_CASE("P13 the transport table spells the SDK fourccs", "[device_policy]")
{
    CHECK(tp::toString(tp::BuiltIn) == "bltn");
    CHECK(tp::toString(tp::Bluetooth) == "blue");
    CHECK(tp::toString(tp::USB) == "usb ");
    CHECK(tp::toString(tp::Unknown) == "0");
    CHECK(tp::Bluetooth == 0x626c7565u);   // 'blue' as the SDK spells it (AudioHardwareBase.h)
    CHECK(tp::BuiltIn == 0x626c746eu);     // 'bltn'
}

TEST_CASE("P15 the TEST-ONLY deny-all token hides every device", "[device_policy]")
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

TEST_CASE("P14 the reconcile table (liveness row included)", "[device_policy]")
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

// ======================================================================================================================
// D -- the decorator
// ======================================================================================================================

TEST_CASE("D1 the decorator lists only allowed devices and answers the built-in as the default", "[device_policy]")
{
    // The headset is the "macOS default" both ways and sits at a DIFFERENT inner index than the built-in's filtered index
    // (inner inputs {Built-in Mic, headset}: default 1; inner outputs {headset, USB, Built-in Speakers}: default 0), so
    // answering the inner index -- or skipping the built-in rung (the first allowed output is the USB) -- is caught.
    Spy spy;
    auto inner = std::make_unique<MockDeviceType>(spy);
    auto* mock = inner.get();
    mock->devs = { { "Built-in Mic", tp::BuiltIn, 1, 0 },
                   { kDenied, tp::Bluetooth, 1, 2 },
                   { "USB Speakers", tp::USB, 0, 2 },
                   { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    mock->defaultIn = kDenied;
    mock->defaultOut = kDenied;
    GuardedDeviceType guarded(std::move(inner), [mock] { return mock->infos(); }, {});
    guarded.scanForDevices();

    CHECK(guarded.getTypeName() == "CoreAudio");
    CHECK(guarded.getDeviceNames(true) == juce::StringArray("Built-in Mic"));
    CHECK(guarded.getDeviceNames(false) == juce::StringArray("USB Speakers", "Built-in Speakers"));
    CHECK(mock->getDefaultDeviceIndex(true) == 1);    // the inner default IS the headset
    CHECK(mock->getDefaultDeviceIndex(false) == 0);
    CHECK(guarded.getDefaultDeviceIndex(true) == 0);
    CHECK(guarded.getDefaultDeviceIndex(false) == 1);
    CHECK(guarded.getDeviceNames(false)[guarded.getDefaultDeviceIndex(false)] == "Built-in Speakers");
}

TEST_CASE("D2 createDevice refuses a hidden name before the inner type sees it", "[device_policy]")
{
    Spy spy;
    auto inner = std::make_unique<MockDeviceType>(spy);
    auto* mock = inner.get();
    addHeadsetAndBuiltIn(*mock);
    GuardedDeviceType guarded(std::move(inner), [mock] { return mock->infos(); }, {});
    guarded.scanForDevices();

    std::unique_ptr<juce::AudioIODevice> refused(guarded.createDevice("", kDenied));
    CHECK(refused == nullptr);
    std::unique_ptr<juce::AudioIODevice> refusedOut(guarded.createDevice(kDenied, "Built-in Mic"));
    CHECK(refusedOut == nullptr);
    CHECK_FALSE(spy.saw(kDenied));

    std::unique_ptr<juce::AudioIODevice> ok(guarded.createDevice("", "Built-in Mic"));
    CHECK(ok != nullptr);
}

TEST_CASE("D3 an inner list change reaches the decorator's listeners after the filtered lists were rebuilt", "[device_policy]")
{
    Spy spy;
    auto inner = std::make_unique<MockDeviceType>(spy);
    auto* mock = inner.get();
    addHeadsetAndBuiltIn(*mock);
    GuardedDeviceType guarded(std::move(inner), [mock] { return mock->infos(); }, {});
    guarded.scanForDevices();

    struct L : juce::AudioIODeviceType::Listener
    {
        GuardedDeviceType* g = nullptr;
        uint64_t seqSeen = 0;
        juce::StringArray outputsSeen;
        void audioDeviceListChanged() override
        {
            seqSeen = g->lastScan().seq;
            outputsSeen = g->getDeviceNames(false);
        }
    } listener;
    listener.g = &guarded;
    guarded.addListener(&listener);

    const auto before = guarded.lastScan().seq;
    mock->devs.push_back({ "USB Speakers", tp::USB, 0, 2 });
    mock->fireListChanged();
    CHECK(listener.seqSeen == before + 1);
    CHECK(listener.outputsSeen.contains("USB Speakers"));
    guarded.removeListener(&listener);
}

TEST_CASE("D4 an allowed USB macOS default listed after a hidden device: the default index is the FILTERED one",
          "[device_policy]")
{
    Spy spy;
    auto inner = std::make_unique<MockDeviceType>(spy);
    auto* mock = inner.get();
    mock->devs = { { kDenied, tp::Bluetooth, 1, 2 },
                   { "Built-in Mic", tp::BuiltIn, 1, 0 },
                   { "USB Interface", tp::USB, 2, 2 },
                   { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    mock->defaultIn = "USB Interface";
    mock->defaultOut = "USB Interface";
    GuardedDeviceType guarded(std::move(inner), [mock] { return mock->infos(); }, {});
    guarded.scanForDevices();

    CHECK(mock->getDefaultDeviceIndex(true) == 2);    // inner inputs {headset, Built-in Mic, USB Interface}
    CHECK(mock->getDefaultDeviceIndex(false) == 1);   // inner outputs {headset, USB Interface, Built-in Speakers}
    CHECK(guarded.getDeviceNames(true) == juce::StringArray("Built-in Mic", "USB Interface"));
    CHECK(guarded.getDeviceNames(false) == juce::StringArray("USB Interface", "Built-in Speakers"));
    CHECK(guarded.getDefaultDeviceIndex(true) == 1);
    CHECK(guarded.getDefaultDeviceIndex(false) == 0);
}

TEST_CASE("D5 a device that appears between the inner scan and the enumeration is mapped by the re-scan, not hidden",
          "[device_policy]")
{
    Spy spy;
    auto inner = std::make_unique<MockDeviceType>(spy);
    auto* mock = inner.get();
    addHeadsetAndBuiltIn(*mock);
    mock->devs.push_back({ "USB Speakers", tp::USB, 0, 2 });
    int calls = 0;
    // The HAL query race (attack S7): the first enumeration still misses the USB device the inner type already lists.
    GuardedDeviceType guarded(std::move(inner), [mock, &calls] {
        auto v = mock->infos();
        if (calls++ == 0)
            v.erase(std::remove_if(v.begin(), v.end(), [](const dp::DeviceInfo& d) { return d.outputName == "USB Speakers"; }),
                    v.end());
        return v;
    }, {});
    guarded.scanForDevices();

    CHECK(calls == 2);
    CHECK(guarded.lastScan().lists.unmapped.isEmpty());
    CHECK(guarded.getDeviceNames(false) == juce::StringArray("Built-in Speakers", "USB Speakers"));
}

TEST_CASE("D6 getIndexOfDevice answers the device's index in the FILTERED list (a hidden device precedes it)",
          "[device_policy]")
{
    Spy spy;
    auto inner = std::make_unique<MockDeviceType>(spy);
    auto* mock = inner.get();
    addHeadsetAndBuiltIn(*mock);   // inner inputs / outputs both start with the hidden headset
    GuardedDeviceType guarded(std::move(inner), [mock] { return mock->infos(); }, {});
    guarded.scanForDevices();

    std::unique_ptr<juce::AudioIODevice> out(guarded.createDevice("Built-in Speakers", ""));
    std::unique_ptr<juce::AudioIODevice> in(guarded.createDevice("", "Built-in Mic"));
    REQUIRE(out != nullptr);
    REQUIRE(in != nullptr);
    CHECK(mock->getIndexOfDevice(out.get(), false) == 1);
    CHECK(mock->getIndexOfDevice(in.get(), true) == 1);
    CHECK(guarded.getIndexOfDevice(out.get(), false) == 0);
    CHECK(guarded.getIndexOfDevice(in.get(), true) == 0);
    CHECK(guarded.getIndexOfDevice(nullptr, false) == -1);
}

// ======================================================================================================================
// M -- a real juce::AudioDeviceManager over the mock
// ======================================================================================================================

TEST_CASE("M1 the guarded manager opens the built-in devices while the headset is the macOS default", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig(addHeadsetAndBuiltIn);
    const auto err = rig.manager->initialiseWithDefaultDevices(2, 2);
    CHECK(err.isEmpty());
    const auto setup = rig.manager->getAudioDeviceSetup();
    CHECK(setup.inputDeviceName == "Built-in Mic");
    CHECK(setup.outputDeviceName == "Built-in Speakers");
    CHECK(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->guardedType() != nullptr);
    CHECK_FALSE(rig.spy.saw(kDenied));   // never created -- not even as JUCE's temporary sample-rate probe
    CHECK(rig.spy.started.contains("Built-in Speakers"));
}

TEST_CASE("M2 CONTROL: a plain juce::AudioDeviceManager opens the headset (the behaviour the guard exists for)",
          "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Spy spy;
    juce::AudioDeviceManager plain;
    auto m = std::make_unique<MockDeviceType>(spy);
    addHeadsetAndBuiltIn(*m);
    plain.addAudioDeviceType(std::move(m));
    const auto err = plain.initialiseWithDefaultDevices(2, 2);
    CHECK(err.isEmpty());
    CHECK(plain.getAudioDeviceSetup().outputDeviceName == kDenied);
    CHECK(spy.saw(kDenied));
    CHECK(spy.started.contains(kDenied));
}

TEST_CASE("M3 no allowed input: the device opens output-only", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied;
        m.defaultOut = kDenied;
    });
    CHECK(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    const auto setup = rig.manager->getAudioDeviceSetup();
    CHECK(setup.inputDeviceName.isEmpty());
    CHECK(setup.outputDeviceName == "Built-in Speakers");
    CHECK(rig.manager->getCurrentAudioDevice()->getActiveInputChannels().isZero());
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M4 nothing allowed: no device, no error", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 } };
        m.defaultIn = kDenied;
        m.defaultOut = kDenied;
    });
    CHECK(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M6 the headset is the macOS default and a USB output is listed before the built-in: the built-in opens "
          "(JUCE moves getDefaultDeviceIndex to the front)", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 },
                   { "USB Speakers", tp::USB, 0, 2 },
                   { "Built-in Mic", tp::BuiltIn, 1, 0 },
                   { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = kDenied;
        m.defaultOut = kDenied;
    });
    CHECK(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M7 an allowed USB macOS default listed after the hidden headset opens both ways", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 },
                   { "Built-in Mic", tp::BuiltIn, 1, 0 },
                   { "USB Interface", tp::USB, 2, 2 },
                   { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Interface";
        m.defaultOut = "USB Interface";
    });
    CHECK(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "USB Interface");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Interface");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M8 the launch with a hidden default opens the device ONCE, records no explicit settings, never the hidden one", "[device_policy]")
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

namespace
{
// {denied default, built-in A, built-in B}: the app's open sequence, then the OPEN output vanishes.
void addDeniedAndTwoBuiltIn(MockDeviceType& m, int micChannels)
{
    m.devs = { { kDenied, tp::Bluetooth, 1, 2 },
               { "Built-in Mic", tp::BuiltIn, micChannels, 0 },
               { "Speakers A", tp::BuiltIn, 0, 2 },
               { "Speakers B", tp::BuiltIn, 0, 2 } };
    m.defaultIn = kDenied;
    m.defaultOut = kDenied;
}

// AudioEngine's launch since bt2 C3: initialiseWithDefaultDevices(2, 2) only -- setSourceMode never re-opens the device,
// so the launch writes no explicit settings (lastExplicitSettings stays null).
void appLaunch(juce::AudioDeviceManager& adm)
{
    REQUIRE(adm.initialiseWithDefaultDevices(2, 2).isEmpty());
}
}

TEST_CASE("M5 the open output vanishes: JUCE's XML branch (explicit settings: a MIDI input enabled at launch -- "
          "MidiHandler::start -- or any treatAsChosenDevice setup) lands on an ALLOWED device, never the denied default",
          "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    // mono mic (the rig's built-in shape): the explicit-settings step records lastExplicitSettings (JUCE's XML branch).
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 1); });
    REQUIRE(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    auto setup = rig.manager->getAudioDeviceSetup();
    setup.inputChannels.setRange(0, 2, true);
    rig.manager->setAudioDeviceSetup(setup, true);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers A");

    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    pump(200);

    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M5b the open output vanishes with no explicit settings (the app's launch since C3): JUCE alone ends with "
          "NO device; the DeviceReconciler re-applies the policy once onto an allowed device", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 2); });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    appLaunch(*rig.manager);
    pump(50);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers A");

    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    // JUCE's own re-init (no explicit settings: the launch writes none) re-opens the VANISHED name,
    // gets "No such device" and leaves no device -- the case BG4 names.
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);
    pump(600);

    CHECK_FALSE(rig.spy.saw(kDenied));
    CHECK(reconciler.reapplies() == 1);
    const auto* device = rig.manager->getCurrentAudioDevice();
    INFO("after the list change: " << (device != nullptr ? rig.manager->getAudioDeviceSetup().outputDeviceName
                                                         : juce::String("no device")));
    REQUIRE(device != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
}

TEST_CASE("M5c MONO mic, the app's launch WITHOUT the setSourceMode re-open (C3): output vanishes -> JUCE alone ends with no device; the reconciler recovers once", "[device_policy]")
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
    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);   // JUCE's own re-init: "No such device"
    // ... and JUCE cleared the setup's names (deleteCurrentDevice): with no device the restore has nothing to put back,
    // so its `haveDevice` gate and its still-listed filter agree (fix round teeth, X6).
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName.isEmpty());
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M5r JUCE's own XML-branch re-init lands on an allowed device with its input -> the reconciler never re-applies on top of it", "[device_policy]")
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
    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    pump(800);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK(reconciler.reapplies() == 0);
    CHECK(rig.spy.started.size() == startsBefore + 2);   // JUCE ONE re-open (a combined pair logs two names), nothing on top
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R1 a storm of 20 list changes after the open device vanished -> exactly one re-apply", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 2); });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    appLaunch(*rig.manager);
    pump(50);
    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    pump(100);
    CHECK(reconciler.reapplies() == 0);   // not yet settled: coalesced, never acted on at the first change message
    for (int i = 1; i < 20; ++i)
    {
        rig.mock->fireListChanged();
        pump(20);
    }
    pump(600);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R2 a list change that keeps the open device -> no re-apply, the device is never switched away from",
          "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 2); });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    appLaunch(*rig.manager);
    pump(50);
    rig.mock->devs.push_back({ "USB Interface", tp::USB, 2, 2 });
    rig.mock->defaultOut = "USB Interface";   // macOS makes the new wired interface the default: kept on Speakers A
    rig.mock->fireListChanged();
    pump(600);
    CHECK(reconciler.reapplies() == 0);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers A");
}

TEST_CASE("R3 the open device vanished and nothing allowed is left -> no re-apply, no device, never the denied one",
          "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Interface", tp::USB, 2, 2 } };
        m.defaultIn = kDenied;
        m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    appLaunch(*rig.manager);
    pump(50);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "USB Interface");
    removeDevice(*rig.mock, "USB Interface");
    rig.mock->fireListChanged();
    pump(600);
    CHECK(reconciler.reapplies() == 0);
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R4 an allowed device that cannot open: one re-apply, retried only after a NEW list change and never within "
          "10 s", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 2); });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    appLaunch(*rig.manager);
    pump(50);
    rig.spy.failOpen.add("Speakers B");
    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    pump(600);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);
    pump(600);   // its own change messages never re-trigger it
    CHECK(reconciler.reapplies() == 1);
    for (int i = 0; i < 5; ++i)
    {
        rig.mock->fireListChanged();
        pump(100);
    }
    pump(600);
    CHECK(reconciler.reapplies() == 1);   // new list changes, but inside the 10 s bound
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R4b with a 50 ms bound: a failed re-apply waits for a NEW device scan, never its own change messages",
          "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 2); });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);   // only the scan gate can hold the retry back
    appLaunch(*rig.manager);
    pump(50);
    rig.spy.failOpen.add("Speakers B");
    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    pump(600);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getCurrentAudioDevice() == nullptr);
    pump(1000);   // its own change messages (close + a failed open) never re-trigger it
    CHECK(reconciler.reapplies() == 1);
    rig.mock->fireListChanged();   // a new device scan
    pump(600);
    CHECK(reconciler.reapplies() == 2);
    pump(1000);
    CHECK(reconciler.reapplies() == 2);
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R5 a device-list change fired from INSIDE the device's start() (JUCE's combiner rate reconcile) re-enters "
          "safely", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig(addHeadsetAndBuiltIn);
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    rig.spy.onStart = [&rig] { rig.mock->fireListChanged(); };
    CHECK(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
    pump(600);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
    CHECK(reconciler.reapplies() == 0);
    CHECK_FALSE(rig.spy.saw(kDenied));
}

// ---- s-rta-0930 bt2 (plan-bt2 I3 / I4 + ruling-bt2.md AM1 / AM2 + ruling-bt2-seats AM12-AM17) -------------------------
// R6 / R7 / R8 / R9 / R13 / M5c / M5r / R14 / R15 / R16 / R17 use MAIN's API only (initialiseWithDefaultDevices,
// DeviceReconciler(manager, 2, 2[, settle, bound]), reapplies(), the spy) so they compile -- and are RED / guards -- on
// the lane's base (GATE-3); M8 / P14 / R10 / R11 / R12 / R18 use the new API (openDefaultDevices, dp::reconcile,
// lastAction, lostInput).

TEST_CASE("R6 no allowed input at launch (output-only); a wired mic appears -> adopted once", "[device_policy]")
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

TEST_CASE("R7 nothing allowed at launch; a built-in pair appears -> adopted once", "[device_policy]")
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

TEST_CASE("R8 the opened wired input vanishes while its output stays -> the policy is re-applied (built-in mic)", "[device_policy]")
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
    removeDevice(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(800);
    INFO("input after: " << rig.manager->getAudioDeviceSetup().inputDeviceName
         << " device " << (rig.manager->getCurrentAudioDevice() != nullptr ? "yes" : "no"));
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R9 a working input is never switched: a USB mic appears (and becomes the macOS default)", "[device_policy]")
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

TEST_CASE("R10 the launch open failed while an allowed device is listed -> no re-apply until a NEW scan", "[device_policy]")
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
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName.isEmpty());   // a failed open clears the names (X6)
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    CHECK(reconciler.reapplies() == 0);          // the launch's own scan is never re-tried by itself
    rig.spy.failOpen.clear();
    rig.mock->fireListChanged();                 // a new scan
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK(rig.manager->getCurrentAudioDevice() != nullptr);
}

TEST_CASE("R11 a mic that cannot open -> the output-only device is put back; the SAME inputs are not retried; re-plugged -> adopted", "[device_policy]")
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
    CHECK(rig.manager->createStateXml() == nullptr);   // the put-back is no explicit choice: JUCE's XML branch stays off (X5)
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    rig.spy.failOpen.clear();
    rig.mock->fireListChanged();          // same allowed inputs: not retried (the progress guard)
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    removeDevice(*rig.mock, "USB Mic");
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

TEST_CASE("R12 round trip: input lost -> output-only (lostInput empty: the no-input notice speaks); the mic returns -> adopted", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) {
        m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Mic", tp::USB, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
        m.defaultIn = "USB Mic"; m.defaultOut = kDenied;
    });
    DeviceReconciler reconciler(*rig.manager, 2, 2, 250, 50);
    REQUIRE(reconciler.openDefaultDevices().isEmpty());
    pump(300);
    removeDevice(*rig.mock, "USB Mic");
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

TEST_CASE("R13 storm: 20 list changes while a mic appears (output-only) -> exactly one adoption, never before the settle", "[device_policy]")
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

TEST_CASE("R14 input lost and the fallback cannot open -> only the still-listed output is put back; the lost input is never re-opened", "[device_policy]")
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
    removeDevice(*rig.mock, "USB Mic");
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName.isEmpty());
    CHECK(rig.manager->createStateXml() == nullptr);   // the put-back is no explicit choice (X5)
    CHECK(count(rig.spy.opened, "USB Mic") == 1);   // the launch only
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R15 a listed mic that never opens: ONE adoption attempt; list changes that leave the allowed inputs unchanged never retry; re-plugged -> tried again", "[device_policy]")
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
        removeDevice(*rig.mock, "Other Wireless");
        rig.mock->fireListChanged();
        pump(400);
    }
    CHECK(reconciler.reapplies() == 1);
    rig.spy.failOpen.clear();
    removeDevice(*rig.mock, "USB Mic");        // the performer re-plugs it
    rig.mock->fireListChanged();
    pump(600);
    CHECK(reconciler.reapplies() == 1);
    rig.mock->devs.push_back({ "USB Mic", tp::USB, 1, 0 });
    rig.mock->fireListChanged();
    pump(800);
    CHECK(reconciler.reapplies() == 2);
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "USB Mic");
    CHECK_FALSE(rig.spy.saw("Other Wireless"));
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R16 a cable jiggle: the open mic's device stops (JUCE's combiner shut down) and the mic is listed again inside the settle -> one re-apply; a later stop with no new scan waits", "[device_policy]")
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
    removeDevice(*rig.mock, "USB Mic");
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
    CHECK(count(rig.spy.createdPairs, "Built-in Speakers + USB Mic") == 2);   // launch + re-apply: closed and re-created (X2)
    CHECK(reconciler.lostInput().isEmpty());   // re-applied onto the SAME mic: no mic was lost (X1)
    rig.manager->getCurrentAudioDevice()->stop();   // dies again with NO device-list change: waits for a new scan
    pump(800);
    CHECK(reconciler.reapplies() == 1);
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R17 JUCE restarts the open device itself (stop, 100 ms, start) -> no re-apply", "[device_policy]")
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
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("R18 the fallback is named: input lost -> the built-in mic; lostInput() names the lost mic and survives Keep and a stop re-apply onto the same mic", "[device_policy]")
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
    removeDevice(*rig.mock, "USB Mic");
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
    removeDevice(*rig.mock, "USB Mic");   // pulled again while the device stops: re-applied onto the SAME built-in mic (X1)
    rig.mock->fireListChanged();
    pump(50);
    rig.manager->getCurrentAudioDevice()->stop();
    pump(800);
    CHECK(reconciler.reapplies() == 2);
    CHECK(reconciler.lastAction() == dp::Reapply::DeviceStopped);
    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getCurrentAudioDevice()->isPlaying());
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK(reconciler.lostInput() == "USB Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

// ======================================================================================================================
// E -- the real HAL (read-only; no device opened)
// ======================================================================================================================

#if JUCE_MAC
TEST_CASE("E1 the enumerator reproduces JUCE's CoreAudio names exactly and reads every transport", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    std::unique_ptr<juce::AudioIODeviceType> ca(juce::AudioIODeviceType::createAudioIODeviceType_CoreAudio());
    REQUIRE(ca != nullptr);
    ca->scanForDevices();
    const auto devices = enumerateCoreAudioDevices();

    for (const bool input : { true, false })
        for (const auto& name : ca->getDeviceNames(input))
        {
            int matches = 0;
            for (const auto& d : devices)
                if ((input ? d.inputName : d.outputName) == name)
                    ++matches;
            INFO((input ? "input " : "output ") << name);
            CHECK(matches == 1);
        }

    const std::vector<uint32_t> known = { tp::BuiltIn, tp::Aggregate, tp::AutoAggregate, tp::Virtual, tp::PCI, tp::USB,
                                          tp::FireWire, tp::Bluetooth, tp::BluetoothLE, tp::HDMI, tp::DisplayPort,
                                          tp::AirPlay, tp::AVB, tp::Thunderbolt, tp::ContinuityWired,
                                          tp::ContinuityWireless, tp::ContinuityCapture };
    int builtIn = 0;
    for (const auto& d : devices)
    {
        INFO(d.inputName << " / " << d.outputName << " uid " << d.uid << " transport " << tp::toString(d.transport));
        CHECK(d.transportReadOk);
        CHECK(d.transport != tp::Unknown);
        CHECK(std::find(known.begin(), known.end(), d.transport) != known.end());
        if (d.uid.startsWith("BuiltIn"))
        {
            CHECK(d.transport == tp::BuiltIn);   // the rig's built-in mic / speakers read 'bltn' EXACTLY
            ++builtIn;
        }
    }
    CHECK(builtIn >= 1);

    // The macOS default flags drive "default = the macOS default if allowed": exactly one device per direction carries
    // it, and it is the device JUCE's own CoreAudio type answers as its default on the same scan.
    for (const bool input : { true, false })
    {
        const auto names = ca->getDeviceNames(input);
        if (names.isEmpty())
            continue;
        int flagged = 0;
        juce::String flaggedName;
        for (const auto& d : devices)
            if (input ? d.isDefaultInput : d.isDefaultOutput)
            {
                ++flagged;
                flaggedName = input ? d.inputName : d.outputName;
            }
        INFO((input ? "input" : "output") << " default: JUCE \"" << names[ca->getDefaultDeviceIndex(input)]
                                           << "\", enumerator \"" << flaggedName << "\"");
        CHECK(flagged == 1);
        CHECK(flaggedName == names[ca->getDefaultDeviceIndex(input)]);
    }
}

TEST_CASE("E2 the real CoreAudio type behind the decorator: every JUCE name is listed or skipped with a reason", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    std::unique_ptr<juce::AudioIODeviceType> ca(juce::AudioIODeviceType::createAudioIODeviceType_CoreAudio());
    REQUIRE(ca != nullptr);
    auto* raw = ca.get();
    GuardedDeviceType guarded(std::move(ca), &enumerateCoreAudioDevices, {});
    guarded.scanForDevices();
    const auto& scan = guarded.lastScan();
    CHECK(scan.lists.unmapped.isEmpty());
    for (const bool input : { true, false })
        for (const auto& name : raw->getDeviceNames(input))
        {
            const bool listed = guarded.getDeviceNames(input).contains(name);
            bool skipped = false;
            for (const auto& s : scan.lists.skipped)
                if (s.name == name && (input ? s.input : s.output) && s.reason.isNotEmpty())
                    skipped = true;
            INFO(name);
            CHECK(listed != skipped);
        }
}

TEST_CASE("E3 the real CoreAudio type behind the decorator with the deny-all token lists NOTHING (read-only)", "[device_policy]")
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
