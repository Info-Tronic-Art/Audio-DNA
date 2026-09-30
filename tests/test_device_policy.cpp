// test_device_policy -- s-rta-0929b btguard (plan-btguard 4.9 + HARMONY ADOPTION BG1/BG2/BG4/BG5): the app never opens a
// Bluetooth (or other wireless) audio device.
//   P*  the pure policy (DevicePolicy.h): classify / filter / the default index.
//   D*  the decorator (GuardedDeviceType) over a mock inner device type.
//   M*  a REAL juce::AudioDeviceManager (GuardedAudioDeviceManager) over the mock: every manager case asserts, through
//       the inner SPY (every createDevice / open / start name, JUCE's temporary devices included), that no denied name
//       was ever created; the unguarded control M2 asserts the spy DID see it (the JUCE behaviour the guard exists for).
//   E*  the real HAL (macOS): read-only property queries, no device is opened.
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

// ======================================================================================================================
// D -- the decorator
// ======================================================================================================================

TEST_CASE("D1 the decorator lists only allowed devices and answers the built-in as the default", "[device_policy]")
{
    Spy spy;
    auto inner = std::make_unique<MockDeviceType>(spy);
    auto* mock = inner.get();
    addHeadsetAndBuiltIn(*mock);
    GuardedDeviceType guarded(std::move(inner), [mock] { return mock->infos(); }, {});
    guarded.scanForDevices();

    CHECK(guarded.getTypeName() == "CoreAudio");
    CHECK(guarded.getDeviceNames(true) == juce::StringArray("Built-in Mic"));
    CHECK(guarded.getDeviceNames(false) == juce::StringArray("Built-in Speakers"));
    CHECK(guarded.getDefaultDeviceIndex(true) == 0);
    CHECK(guarded.getDefaultDeviceIndex(false) == 0);
    CHECK(mock->getDefaultDeviceIndex(false) == 0);   // the inner default IS the headset (index 0 of its list)
    CHECK(mock->getDeviceNames(false)[0] == kDenied);
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

// AudioEngine's startup: initialiseWithDefaultDevices(2, 2), then setSourceMode(MicInput)'s re-open (kept, BG3).
void appOpenSequence(juce::AudioDeviceManager& adm)
{
    REQUIRE(adm.initialiseWithDefaultDevices(2, 2).isEmpty());
    auto setup = adm.getAudioDeviceSetup();
    setup.inputChannels.setRange(0, 2, true);
    adm.setAudioDeviceSetup(setup, true);
}

void removeDevice(MockDeviceType& m, const juce::String& name)
{
    m.devs.erase(std::remove_if(m.devs.begin(), m.devs.end(), [&](const auto& d) { return d.name == name; }), m.devs.end());
}
}

TEST_CASE("M5 the open output vanishes: JUCE's own re-init lands on an ALLOWED device, never the denied default",
          "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    // mono mic (the rig's built-in shape): the startup re-open records lastExplicitSettings (JUCE's XML branch).
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 1); });
    appOpenSequence(*rig.manager);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers A");

    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    pump(200);

    REQUIRE(rig.manager->getCurrentAudioDevice() != nullptr);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers B");
    CHECK(rig.manager->getAudioDeviceSetup().inputDeviceName == "Built-in Mic");
    CHECK_FALSE(rig.spy.saw(kDenied));
}

TEST_CASE("M5b the open output vanishes with a STEREO input (the startup re-open is a no-op there): JUCE alone ends with "
          "NO device; the DeviceReconciler re-applies the policy once onto an allowed device", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 2); });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    appOpenSequence(*rig.manager);
    pump(50);
    CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Speakers A");

    removeDevice(*rig.mock, "Speakers A");
    rig.mock->fireListChanged();
    // JUCE's own re-init (no explicit settings: the stereo re-open was an early return) re-opens the VANISHED name,
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

TEST_CASE("R1 a storm of 20 list changes after the open device vanished -> exactly one re-apply", "[device_policy]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    GuardedRig rig([](MockDeviceType& m) { addDeniedAndTwoBuiltIn(m, 2); });
    DeviceReconciler reconciler(*rig.manager, 2, 2);
    appOpenSequence(*rig.manager);
    pump(50);
    removeDevice(*rig.mock, "Speakers A");
    for (int i = 0; i < 20; ++i)
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
    appOpenSequence(*rig.manager);
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
    appOpenSequence(*rig.manager);
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
    appOpenSequence(*rig.manager);
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
#endif
