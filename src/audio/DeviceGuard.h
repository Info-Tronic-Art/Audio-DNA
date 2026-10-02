#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "DevicePolicy.h"
#include "CoreAudioDeviceInfo.h"

// s-rta-0929b btguard (plan-btguard 4.3; docs/claude/pitfalls.md "never opens a Bluetooth audio device"): the app never opens a Bluetooth (or other wireless) audio device.
// JUCE's AudioDeviceManager learns devices ONLY through an AudioIODeviceType (names, the default index, createDevice) and
// re-reads the macOS DEFAULT device on every open path -- initialiseWithDefaultDevices, its device-list-changed re-init
// and that re-init's fallback -- so the guard is a decorator on the device TYPE, not a check at one open call.
//
// GuardedDeviceType wraps JUCE's CoreAudio type: getDeviceNames hides every device devpolicy::classify denies,
// getDefaultDeviceIndex answers the policy's default (the macOS default when allowed, else built-in), createDevice refuses a
// hidden name. It is the ONLY listener of the inner type, and JUCE's own listener is registered on the decorator, so the
// filtered lists are rebuilt BEFORE JUCE hears of a device-list change. All of it runs on the MESSAGE thread.
class GuardedDeviceType final : public juce::AudioIODeviceType, private juce::AudioIODeviceType::Listener
{
public:
    using Enumerate = std::function<std::vector<audiodna::devpolicy::DeviceInfo>()>;

    GuardedDeviceType(std::unique_ptr<juce::AudioIODeviceType> inner, Enumerate, audiodna::devpolicy::Config);
    ~GuardedDeviceType() override;

    void scanForDevices() override;
    juce::StringArray getDeviceNames(bool wantInputNames = false) const override;
    int getDefaultDeviceIndex(bool forInput) const override;
    int getIndexOfDevice(juce::AudioIODevice* device, bool asInput) const override;
    bool hasSeparateInputsAndOutputs() const override;
    juce::AudioIODevice* createDevice(const juce::String& outputDeviceName, const juce::String& inputDeviceName) override;

    struct Scan
    {
        audiodna::devpolicy::Lists lists;
        std::vector<audiodna::devpolicy::DeviceInfo> devices;
        uint64_t seq = 0;       // advances on every rebuild
        double enumerateMs = 0.0;
    };
    const Scan& lastScan() const noexcept { return scan_; }   // message thread

#if AUDIODNA_TEST_SERVER
    // TEST-ONLY (s-rta-0930 bt2, POST /api/debug/audio_deny): replaces the denied names, re-scans the inner type and runs
    // the device-list-change path (rebuild, then JUCE's listeners) -- the policy-level stand-in for a plug / unplug, not
    // the HAL's own notification. Message thread.
    void setTestDeniedNames(const juce::StringArray& names);
#endif

private:
    void audioDeviceListChanged() override;   // from the inner type: rebuild, then tell JUCE
    void rebuild();

    std::unique_ptr<juce::AudioIODeviceType> inner_;
    Enumerate enumerate_;
    audiodna::devpolicy::Config config_;
    Scan scan_;
    juce::String lastLogged_;

    JUCE_DECLARE_NON_COPYABLE(GuardedDeviceType)
};

// The app's AudioDeviceManager: createAudioDeviceTypes (virtual, "override this if your app needs to ... avoid using
// DirectSound devices") replaces JUCE's CoreAudio type by a GuardedDeviceType at the same index. Everything else is
// JUCE's manager unchanged.
class GuardedAudioDeviceManager final : public juce::AudioDeviceManager
{
public:
    explicit GuardedAudioDeviceManager(audiodna::devpolicy::Config,
                                       GuardedDeviceType::Enumerate = &enumerateCoreAudioDevices);

    void createAudioDeviceTypes(juce::OwnedArray<juce::AudioIODeviceType>& types) override;

    // The wrapped type, or nullptr (a platform without CoreAudio, or before the first scan).
    GuardedDeviceType* guardedType() const noexcept { return guarded_; }
    const audiodna::devpolicy::Config& policyConfig() const noexcept { return config_; }
#if AUDIODNA_TEST_SERVER
    // TEST-ONLY (s-rta-0930 bt2): replaces the denied names (policyConfig() follows) and forwards them to the guarded type.
    void setTestDeniedNames(const juce::StringArray& names);
#endif

    // TESTS ONLY: builds the inner types instead of JUCE's list (set BEFORE initialise; a mock names itself "CoreAudio").
    std::function<void(juce::OwnedArray<juce::AudioIODeviceType>&)> typeFactoryForTests;

private:
    audiodna::devpolicy::Config config_;
    GuardedDeviceType::Enumerate enumerate_;
    GuardedDeviceType* guarded_ = nullptr;   // owned by the manager's availableDeviceTypes
};

// HARMONY ADOPTION BG4: JUCE's own device-list-changed re-init runs through the guarded type, but when the OPEN device
// vanishes and the manager holds no explicit settings (lastExplicitSettings null -- e.g. a stereo input, where the startup
// setSourceMode re-open is a no-op), JUCE re-opens the VANISHED names, gets "No such device" and stays with NO device
// while an allowed one exists (tests/test_device_policy.cpp M5b). This re-applies the policy in exactly that case --
// "device gone" -- and nothing else (a working device is never switched away from; a newly plugged device is not
// adopted). Coalesced: re-evaluated `settleMs` after the LAST change message, at most one re-apply per `minIntervalMs`,
// and a failed re-apply is retried only after a NEW device-list change (never on its own change messages).
// It only ever acts when the manager has no device object, so no JUCE restart of a device can be pending.
// MESSAGE THREAD (ChangeListener + Timer).
class DeviceReconciler final : private juce::ChangeListener, private juce::Timer
{
public:
    DeviceReconciler(GuardedAudioDeviceManager&, int numInputChannels, int numOutputChannels,
                     int settleMs = 250, int minIntervalMs = 10000);
    ~DeviceReconciler() override;

    int reapplies() const noexcept { return reapplies_; }
    // After each re-apply (message thread): the error initialiseWithDefaultDevices returned ("" = none).
    std::function<void(const juce::String& error)> onReapplied;

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void timerCallback() override;

    GuardedAudioDeviceManager& manager_;
    const int numIns_, numOuts_, settleMs_, minIntervalMs_;
    bool hadDevice_ = false;   // a device was open when last observed: its disappearance is "device gone"
    double lastReapplyMs_ = 0.0;
    uint64_t lastReappliedSeq_ = 0;   // the device scan a re-apply ran on: a failed one waits for a newer scan
    int reapplies_ = 0;

    JUCE_DECLARE_NON_COPYABLE(DeviceReconciler)
};

namespace devguard
{
// The production policy config. In a TEST_SERVER build only, ADNA_AUDIO_DENY_DEVICES="<name>[;<name>...]" (exact JUCE
// device names, read once) lists devices to treat as denied -- it drives the HIDING branch (the lists, the default, the
// no-input / no-device states), not the transport branch. Absent from a build without AUDIODNA_BUILD_TEST_SERVER.
audiodna::devpolicy::Config productionConfig();

// One stderr line naming the opened input / output with their transports and every skipped device.
juce::String describeDevices(const juce::AudioDeviceManager&, const GuardedDeviceType*);
}
