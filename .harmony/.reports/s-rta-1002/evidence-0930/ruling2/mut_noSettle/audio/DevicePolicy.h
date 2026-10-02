#pragma once
#include <juce_core/juce_core.h>
#include <cstdint>
#include <vector>

// s-rta-0929b btguard (plan-btguard 4.1 + HARMONY ADOPTION BG1/BG2): the pure no-wireless device policy -- which audio
// devices the app may open, which one is the default, and nothing else. No CoreAudio, no device objects: the macOS
// enumeration (CoreAudioDeviceInfo) fills DeviceInfo, GuardedDeviceType (DeviceGuard) applies filter() to JUCE's lists.
//
// Boris's words (binding-decisions.md 2026-09-24): "we will never use bluetooth audio for any reason. it is slow and bad.
// never use it again." and "we will only use hard wired sound input or the onboard mic". Hiding AirPlay / wireless
// Continuity / the deprecated Continuity Capture as well, failing closed on a transport read error and on an aggregate
// with an unresolvable member, is the APP's rule (Harmony's reading of the second quote), not Boris's words.
namespace audiodna::devpolicy
{
// (a<<24)|(b<<16)|(c<<8)|d -- no multichar literals (-Wpedantic).
constexpr uint32_t fourcc(char a, char b, char c, char d) noexcept
{
    return (static_cast<uint32_t>(static_cast<unsigned char>(a)) << 24)
         | (static_cast<uint32_t>(static_cast<unsigned char>(b)) << 16)
         | (static_cast<uint32_t>(static_cast<unsigned char>(c)) << 8)
         |  static_cast<uint32_t>(static_cast<unsigned char>(d));
}

// kAudioDeviceTransportType* (AudioHardwareBase.h / AudioHardwareDeprecated.h), mirrored so the policy and its ctest need no
// CoreAudio header. CoreAudioDeviceInfo.cpp static_asserts every value against the SDK constant.
namespace transport
{
inline constexpr uint32_t Unknown            = 0;
inline constexpr uint32_t BuiltIn            = fourcc('b', 'l', 't', 'n');
inline constexpr uint32_t Aggregate          = fourcc('g', 'r', 'u', 'p');
inline constexpr uint32_t AutoAggregate      = fourcc('f', 'g', 'r', 'p');
inline constexpr uint32_t Virtual            = fourcc('v', 'i', 'r', 't');
inline constexpr uint32_t PCI                = fourcc('p', 'c', 'i', ' ');
inline constexpr uint32_t USB                = fourcc('u', 's', 'b', ' ');
inline constexpr uint32_t FireWire           = fourcc('1', '3', '9', '4');
inline constexpr uint32_t Bluetooth          = fourcc('b', 'l', 'u', 'e');
inline constexpr uint32_t BluetoothLE        = fourcc('b', 'l', 'e', 'a');
inline constexpr uint32_t HDMI               = fourcc('h', 'd', 'm', 'i');
inline constexpr uint32_t DisplayPort        = fourcc('d', 'p', 'r', 't');
inline constexpr uint32_t AirPlay            = fourcc('a', 'i', 'r', 'p');
inline constexpr uint32_t AVB                = fourcc('e', 'a', 'v', 'b');
inline constexpr uint32_t Thunderbolt        = fourcc('t', 'h', 'u', 'n');
inline constexpr uint32_t ContinuityWired    = fourcc('c', 'c', 'w', 'd');
inline constexpr uint32_t ContinuityWireless = fourcc('c', 'c', 'w', 'l');
inline constexpr uint32_t ContinuityCapture  = fourcc('c', 'c', 'a', 'p');   // deprecated macOS 13: wired-ness unknowable

juce::String toString(uint32_t t);   // "bltn", "usb " ...; "0" for Unknown
}

struct DeviceInfo
{
    struct Member   // one sub-device of an aggregate / multi-output device
    {
        juce::String uid;
        uint32_t transport = 0;
        bool resolved = true;          // false: the UID names no present device (e.g. earbuds not connected)
        bool transportReadOk = true;
    };

    juce::String inputName, outputName;   // JUCE's per-direction names AFTER appendNumbersToDuplicates; empty = not in that list
    juce::String uid;
    uint32_t transport = 0;
    bool transportReadOk = true;          // false: the property read failed -- NOT Unknown: fails CLOSED (BG1)
    std::vector<Member> members;          // aggregate members; empty otherwise
    bool membersReadOk = true;            // false: an aggregate whose member list could not be read (fails closed)
    bool isDefaultInput = false, isDefaultOutput = false;
};

struct Config
{
#if AUDIODNA_TEST_SERVER
    juce::StringArray testDeniedNames;   // TEST-ONLY (ADNA_AUDIO_DENY_DEVICES): exact JUCE names treated as denied
#endif
};

struct Verdict
{
    bool allowed = true;
    juce::String reason;   // "" when allowed; "bluetooth", "bluetooth-le", "airplay", "wireless-continuity",
                           // "continuity-capture", "transport-unreadable", "aggregate:<why>", "test-denied" (TEST-ONLY)
};

Verdict classify(const DeviceInfo&, const Config&);

// True when `uid` has the Bluetooth address shape XX-XX-XX-XX-XX-XX (':' also accepted as the separator), optionally
// followed by ":input" / ":output" -- how macOS names a Bluetooth device's UID (BG2).
bool hasBluetoothUidShape(const juce::String& uid);

struct Lists
{
    juce::StringArray inputs, outputs;        // the filtered lists JUCE sees: a subset of the inner lists, SAME order
    int defaultInput = 0, defaultOutput = 0;  // JUCE convention: 0 when unknown or empty
    struct Skipped
    {
        juce::String name, reason;
        bool input = false, output = false;
    };
    std::vector<Skipped> skipped;
    juce::StringArray unmapped;               // inner names with no DeviceInfo (hidden, fail closed)
};

// Default index per direction, over the ALLOWED devices: the macOS default if allowed; else the first built-in; else the
// first USB; else 0 (the first allowed).
Lists filter(const juce::StringArray& innerInputs, const juce::StringArray& innerOutputs,
             const std::vector<DeviceInfo>& scan, const Config&);

// bt2: what the reconciler does once the device list settled. A working (still allowed) input is never switched away
// from; everything else that leaves the app without an allowed input -- or without any device -- re-applies the policy.
enum class Reapply { None, NoDevice, AdoptInput, InputLost, DeviceStopped };
Reapply reconcile(bool haveDevice, bool devicePlaying, const juce::String& openedInput, const Lists&);
juce::String toString(Reapply);
}
