#include "DevicePolicy.h"

namespace audiodna::devpolicy
{
juce::String transport::toString(uint32_t t)
{
    if (t == Unknown)
        return "0";
    juce::String s;
    for (int shift = 24; shift >= 0; shift -= 8)
    {
        const auto c = static_cast<char>((t >> shift) & 0xffu);
        s += (c >= 0x20 && c < 0x7f) ? juce::String::charToString(static_cast<juce::juce_wchar>(c)) : juce::String("?");
    }
    return s;
}

namespace
{
// The wireless transports (BG1). Everything else -- built-in, USB, Thunderbolt, PCI, FireWire, HDMI, DisplayPort, AVB,
// wired Continuity, Virtual, a genuinely reported Unknown -- is allowed.
juce::String deniedTransportReason(uint32_t t)
{
    if (t == transport::Bluetooth)          return "bluetooth";
    if (t == transport::BluetoothLE)        return "bluetooth-le";
    if (t == transport::AirPlay)            return "airplay";
    if (t == transport::ContinuityWireless) return "wireless-continuity";
    if (t == transport::ContinuityCapture)  return "continuity-capture";
    return {};
}

bool isHex(juce::juce_wchar c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
}

bool hasBluetoothUidShape(const juce::String& uid)
{
    // XX-XX-XX-XX-XX-XX = 17 characters: six hex pairs, five separators.
    if (uid.length() < 17)
        return false;
    for (int i = 0; i < 17; ++i)
    {
        const auto c = uid[i];
        if (i % 3 == 2)
        {
            if (c != '-' && c != ':')
                return false;
        }
        else if (!isHex(c))
            return false;
    }
    const auto rest = uid.substring(17);
    return rest.isEmpty() || rest.equalsIgnoreCase(":input") || rest.equalsIgnoreCase(":output");
}

Verdict classify(const DeviceInfo& d, const Config& cfg)
{
#if AUDIODNA_TEST_SERVER
    if (cfg.testDeniedNames.contains("*")
        || (d.inputName.isNotEmpty() && cfg.testDeniedNames.contains(d.inputName))
        || (d.outputName.isNotEmpty() && cfg.testDeniedNames.contains(d.outputName)))
        return { false, "test-denied" };
#else
    juce::ignoreUnused(cfg);
#endif

    if (!d.transportReadOk)
        return { false, "transport-unreadable" };

    if (const auto r = deniedTransportReason(d.transport); r.isNotEmpty())
        return { false, r };

    const bool isAggregate = d.transport == transport::Aggregate || d.transport == transport::AutoAggregate
                          || !d.members.empty();
    if (isAggregate)
    {
        // BG2: denied if ANY member is denied, unresolvable, unreadable or has a Bluetooth-shaped UID.
        if (!d.membersReadOk)
            return { false, "aggregate:members-unreadable" };
        for (const auto& m : d.members)
        {
            if (hasBluetoothUidShape(m.uid))
                return { false, "aggregate:bluetooth-uid" };
            if (!m.resolved)
                return { false, "aggregate:member-unresolved" };
            if (!m.transportReadOk)
                return { false, "aggregate:transport-unreadable" };
            if (const auto r = deniedTransportReason(m.transport); r.isNotEmpty())
                return { false, "aggregate:" + r };
        }
    }
    return {};
}

Lists filter(const juce::StringArray& innerInputs, const juce::StringArray& innerOutputs,
             const std::vector<DeviceInfo>& scan, const Config& cfg)
{
    Lists out;

    const auto findInfo = [&scan](const juce::String& name, bool input) -> const DeviceInfo* {
        for (const auto& d : scan)
            if ((input ? d.inputName : d.outputName) == name)
                return &d;
        return nullptr;
    };

    const auto noteSkipped = [&out](const juce::String& name, const juce::String& reason, bool input) {
        for (auto& s : out.skipped)
            if (s.name == name && s.reason == reason)
            {
                (input ? s.input : s.output) = true;
                return;
            }
        Lists::Skipped s;
        s.name = name;
        s.reason = reason;
        (input ? s.input : s.output) = true;
        out.skipped.push_back(s);
    };

    const auto buildDirection = [&](const juce::StringArray& inner, bool input, juce::StringArray& names, int& defaultIndex) {
        std::vector<const DeviceInfo*> kept;
        for (const auto& name : inner)
        {
            const auto* info = findInfo(name, input);
            if (info == nullptr)
            {
                out.unmapped.addIfNotAlreadyThere(name);
                continue;
            }
            const auto v = classify(*info, cfg);
            if (!v.allowed)
            {
                noteSkipped(name, v.reason, input);
                continue;
            }
            names.add(name);
            kept.push_back(info);
        }

        defaultIndex = 0;
        const auto firstWhere = [&kept](auto pred) {
            for (size_t i = 0; i < kept.size(); ++i)
                if (pred(*kept[i]))
                    return static_cast<int>(i);
            return -1;
        };
        int i = firstWhere([input](const DeviceInfo& d) { return input ? d.isDefaultInput : d.isDefaultOutput; });
        if (i < 0) i = firstWhere([](const DeviceInfo& d) { return d.transport == transport::BuiltIn; });
        if (i < 0) i = firstWhere([](const DeviceInfo& d) { return d.transport == transport::USB; });
        if (i >= 0)
            defaultIndex = i;
    };

    buildDirection(innerInputs, true, out.inputs, out.defaultInput);
    buildDirection(innerOutputs, false, out.outputs, out.defaultOutput);
    return out;
}

Reapply reconcile(bool haveDevice, bool devicePlaying, const juce::String& openedInput, const Lists& lists)
{
    const bool anyAllowed = !(lists.inputs.isEmpty() && lists.outputs.isEmpty());
    if (!haveDevice)
        return anyAllowed ? Reapply::NoDevice : Reapply::None;
    if (openedInput.isNotEmpty() && !lists.inputs.contains(openedInput))
        return Reapply::InputLost;
    if (!devicePlaying)
        return anyAllowed ? Reapply::DeviceStopped : Reapply::None;
    if (openedInput.isEmpty() && !lists.inputs.isEmpty())
        return Reapply::AdoptInput;
    return Reapply::None;
}

juce::String toString(Reapply r)
{
    switch (r)
    {
        case Reapply::NoDevice:      return "no-device";
        case Reapply::AdoptInput:    return "adopt-input";
        case Reapply::InputLost:     return "input-lost";
        case Reapply::DeviceStopped: return "device-stopped";
        case Reapply::None:          break;
    }
    return {};
}
}
