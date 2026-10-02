// test_device_policy -- s-rta-0929b btguard (plan-btguard 4.9 + HARMONY ADOPTION BG1/BG2/BG4/BG5): the app never opens a
// Bluetooth (or other wireless) audio device.
//   P*  the pure policy (DevicePolicy.h): classify / filter / the default index.
//   D*  the decorator (GuardedDeviceType) over a mock inner device type.
//   M*  a REAL juce::AudioDeviceManager (GuardedAudioDeviceManager) over the mock: every manager case asserts, through
//       the inner SPY (every createDevice / open / start name, JUCE's temporary devices included), that no denied name
//       was ever created; the unguarded control M2 asserts the spy DID see it (the JUCE behaviour the guard exists for).
//   R*  the DeviceReconciler (BG4) over the same manager.
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
