#pragma once
#include "DevicePolicy.h"
#include <vector>

// s-rta-0929b btguard (plan-btguard 4.2): every CoreAudio device with its transport type, UID, aggregate members and
// JUCE-identical per-direction names -- the same device order, the same channel-count gating per direction and the same
// StringArray::appendNumbersToDuplicates(false, true) as JUCE 8.0.4's CoreAudioIODeviceType::scanForDevices, so a JUCE
// device name maps to exactly one entry. Read-only HAL property queries; never opens a device. MESSAGE THREAD ONLY (it is
// called from JUCE's device-type scan and list-changed callbacks). Returns {} on a non-mac build.
std::vector<audiodna::devpolicy::DeviceInfo> enumerateCoreAudioDevices();
