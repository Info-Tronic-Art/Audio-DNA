#include "CoreAudioDeviceInfo.h"
#include <algorithm>

#if JUCE_MAC
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

namespace dp = audiodna::devpolicy;

// BG1 / attack M4: the mirrored fourccs must equal the SDK's -- a typo'd 'blue' would silently switch the guard off.
static_assert(dp::transport::Unknown            == kAudioDeviceTransportTypeUnknown);
static_assert(dp::transport::BuiltIn            == kAudioDeviceTransportTypeBuiltIn);
static_assert(dp::transport::Aggregate          == kAudioDeviceTransportTypeAggregate);
static_assert(dp::transport::AutoAggregate      == kAudioDeviceTransportTypeAutoAggregate);
static_assert(dp::transport::Virtual            == kAudioDeviceTransportTypeVirtual);
static_assert(dp::transport::PCI                == kAudioDeviceTransportTypePCI);
static_assert(dp::transport::USB                == kAudioDeviceTransportTypeUSB);
static_assert(dp::transport::FireWire           == kAudioDeviceTransportTypeFireWire);
static_assert(dp::transport::Bluetooth          == kAudioDeviceTransportTypeBluetooth);
static_assert(dp::transport::BluetoothLE        == kAudioDeviceTransportTypeBluetoothLE);
static_assert(dp::transport::HDMI               == kAudioDeviceTransportTypeHDMI);
static_assert(dp::transport::DisplayPort        == kAudioDeviceTransportTypeDisplayPort);
static_assert(dp::transport::AirPlay            == kAudioDeviceTransportTypeAirPlay);
static_assert(dp::transport::AVB                == kAudioDeviceTransportTypeAVB);
static_assert(dp::transport::Thunderbolt        == kAudioDeviceTransportTypeThunderbolt);
static_assert(dp::transport::ContinuityWired    == kAudioDeviceTransportTypeContinuityCaptureWired);
static_assert(dp::transport::ContinuityWireless == kAudioDeviceTransportTypeContinuityCaptureWireless);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static_assert(dp::transport::ContinuityCapture  == kAudioDeviceTransportTypeContinuityCapture);
#pragma clang diagnostic pop

namespace
{
AudioObjectPropertyAddress address(AudioObjectPropertySelector selector,
                                   AudioObjectPropertyScope scope = kAudioObjectPropertyScopeGlobal)
{
    return { selector, scope, kAudioObjectPropertyElementMain };
}

template <typename T>
bool readScalar(AudioObjectID id, const AudioObjectPropertyAddress& pa, T& out)
{
    UInt32 size = sizeof(T);
    return AudioObjectGetPropertyData(id, &pa, 0, nullptr, &size, &out) == noErr && size == sizeof(T);
}

bool readString(AudioObjectID id, const AudioObjectPropertyAddress& pa, juce::String& out)
{
    CFStringRef s = nullptr;
    UInt32 size = sizeof(s);
    if (AudioObjectGetPropertyData(id, &pa, 0, nullptr, &size, &s) != noErr || s == nullptr)
        return false;
    out = juce::String::fromCFString(s);
    CFRelease(s);
    return true;
}

// JUCE's getNumChannels: the sum of mNumberChannels over the stream configuration in that scope.
int numChannels(AudioObjectID id, bool input)
{
    const auto pa = address(kAudioDevicePropertyStreamConfiguration,
                            input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput);
    UInt32 size = 0;
    if (AudioObjectGetPropertyDataSize(id, &pa, 0, nullptr, &size) != noErr || size < sizeof(AudioBufferList))
        return 0;
    juce::HeapBlock<char> storage(size, true);
    auto* list = reinterpret_cast<AudioBufferList*>(storage.get());
    if (AudioObjectGetPropertyData(id, &pa, 0, nullptr, &size, list) != noErr)
        return 0;
    int total = 0;
    for (UInt32 i = 0; i < list->mNumberBuffers; ++i)
        total += static_cast<int>(list->mBuffers[i].mNumberChannels);
    return total;
}

bool readTransport(AudioObjectID id, uint32_t& out)
{
    UInt32 t = 0;
    if (!readScalar(id, address(kAudioDevicePropertyTransportType), t))
        return false;
    out = t;
    return true;
}

// kAudioAggregateDevicePropertyFullSubDeviceList (every member UID, present or not) -> each UID's device -> its transport.
bool readMembers(AudioObjectID id, std::vector<dp::DeviceInfo::Member>& members)
{
    const auto pa = address(kAudioAggregateDevicePropertyFullSubDeviceList);
    CFArrayRef uids = nullptr;
    UInt32 size = sizeof(uids);
    if (AudioObjectGetPropertyData(id, &pa, 0, nullptr, &size, &uids) != noErr || uids == nullptr)
        return false;

    const auto count = CFArrayGetCount(uids);
    for (CFIndex i = 0; i < count; ++i)
    {
        dp::DeviceInfo::Member m;
        auto uid = static_cast<CFStringRef>(CFArrayGetValueAtIndex(uids, i));
        if (uid == nullptr || CFGetTypeID(uid) != CFStringGetTypeID())
        {
            m.resolved = false;
            members.push_back(m);
            continue;
        }
        m.uid = juce::String::fromCFString(uid);

        AudioObjectID member = kAudioObjectUnknown;
        UInt32 memberSize = sizeof(member);
        const auto translate = address(kAudioHardwarePropertyTranslateUIDToDevice);
        const bool found = AudioObjectGetPropertyData(kAudioObjectSystemObject, &translate, sizeof(uid), &uid,
                                                      &memberSize, &member) == noErr
                        && member != kAudioObjectUnknown;
        m.resolved = found;
        if (found)
            m.transportReadOk = readTransport(member, m.transport);
        members.push_back(m);
    }
    CFRelease(uids);
    return true;
}
}

std::vector<dp::DeviceInfo> enumerateCoreAudioDevices()
{
    std::vector<dp::DeviceInfo> out;

    // JUCE reads the device list with scope Wildcard, element Main (CoreAudioIODeviceType::scanForDevices).
    const auto devicesAddress = address(kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeWildcard);
    UInt32 size = 0;
    if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &devicesAddress, 0, nullptr, &size) != noErr)
        return out;
    std::vector<AudioObjectID> ids(size / sizeof(AudioObjectID));
    if (ids.empty()
        || AudioObjectGetPropertyData(kAudioObjectSystemObject, &devicesAddress, 0, nullptr, &size, ids.data()) != noErr)
        return out;
    ids.resize(size / sizeof(AudioObjectID));

    AudioObjectID defaultIn = kAudioObjectUnknown, defaultOut = kAudioObjectUnknown;
    readScalar(kAudioObjectSystemObject, address(kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeWildcard), defaultIn);
    readScalar(kAudioObjectSystemObject, address(kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeWildcard), defaultOut);

    juce::StringArray inputNames, outputNames;
    std::vector<size_t> inputOwner, outputOwner;   // index into `out` per list entry

    for (const auto id : ids)
    {
        juce::String name;
        // JUCE: kAudioDevicePropertyDeviceNameCFString, scope Wildcard; a device without a name is skipped.
        if (!readString(id, address(kAudioDevicePropertyDeviceNameCFString, kAudioObjectPropertyScopeWildcard), name))
            continue;

        dp::DeviceInfo d;
        readString(id, address(kAudioDevicePropertyDeviceUID), d.uid);
        d.transportReadOk = readTransport(id, d.transport);
        if (d.transportReadOk && (d.transport == dp::transport::Aggregate || d.transport == dp::transport::AutoAggregate))
            d.membersReadOk = readMembers(id, d.members);
        d.isDefaultInput = id == defaultIn;
        d.isDefaultOutput = id == defaultOut;

        const auto index = out.size();
        out.push_back(d);
        if (numChannels(id, true) > 0)
        {
            inputNames.add(name);
            inputOwner.push_back(index);
        }
        if (numChannels(id, false) > 0)
        {
            outputNames.add(name);
            outputOwner.push_back(index);
        }
    }

    // JUCE: appendNumbersToDuplicates(false, true) on each list -- the deduped name is the key JUCE opens by.
    inputNames.appendNumbersToDuplicates(false, true);
    outputNames.appendNumbersToDuplicates(false, true);
    for (size_t i = 0; i < inputOwner.size(); ++i)
        out[inputOwner[i]].inputName = inputNames[static_cast<int>(i)];
    for (size_t i = 0; i < outputOwner.size(); ++i)
        out[outputOwner[i]].outputName = outputNames[static_cast<int>(i)];

    // A device in neither list has no channels: JUCE never lists it, the policy never needs it.
    out.erase(std::remove_if(out.begin(), out.end(),
                             [](const dp::DeviceInfo& d) { return d.inputName.isEmpty() && d.outputName.isEmpty(); }),
              out.end());
    return out;
}

#else

std::vector<audiodna::devpolicy::DeviceInfo> enumerateCoreAudioDevices()
{
    return {};
}

#endif
