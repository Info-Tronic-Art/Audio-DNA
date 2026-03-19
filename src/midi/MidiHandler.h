#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include "binding/BindingManager.h"

// MidiHandler: receives MIDI messages from all enabled MIDI input devices
// and routes them through the BindingManager for action dispatch.
// Handles device selection and hot-plug.
class MidiHandler : public juce::MidiInputCallback
{
public:
    explicit MidiHandler(BindingManager& bindingManager);
    ~MidiHandler() override;

    // Start/stop listening on a device manager
    void start(juce::AudioDeviceManager& deviceManager);
    void stop();

    // Enable/disable a specific MIDI input device by identifier
    void enableDevice(const juce::String& deviceIdentifier, bool enabled);

    // Get list of available MIDI input devices
    static juce::Array<juce::MidiDeviceInfo> getAvailableDevices();

    // Check if a device is currently enabled
    bool isDeviceEnabled(const juce::String& deviceIdentifier) const;

    // MidiInputCallback
    void handleIncomingMidiMessage(juce::MidiInput* source,
                                   const juce::MidiMessage& message) override;

private:
    BindingManager& bindingManager_;
    juce::AudioDeviceManager* deviceManager_ = nullptr;
    juce::StringArray enabledDevices_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiHandler)
};
