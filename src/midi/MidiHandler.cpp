#include "MidiHandler.h"

MidiHandler::MidiHandler(BindingManager& bindingManager)
    : bindingManager_(bindingManager)
{
}

MidiHandler::~MidiHandler()
{
    stop();
}

void MidiHandler::start(juce::AudioDeviceManager& deviceManager)
{
    stop(); // Clean up any previous state
    deviceManager_ = &deviceManager;

    // Enable all available MIDI inputs by default
    auto devices = juce::MidiInput::getAvailableDevices();
    for (const auto& device : devices)
    {
        if (!deviceManager_->isMidiInputDeviceEnabled(device.identifier))
            deviceManager_->setMidiInputDeviceEnabled(device.identifier, true);
        deviceManager_->addMidiInputDeviceCallback(device.identifier, this);
        enabledDevices_.add(device.identifier);
    }
}

void MidiHandler::stop()
{
    if (deviceManager_)
    {
        for (const auto& id : enabledDevices_)
            deviceManager_->removeMidiInputDeviceCallback(id, this);
        enabledDevices_.clear();
        deviceManager_ = nullptr;
    }
}

void MidiHandler::enableDevice(const juce::String& deviceIdentifier, bool enabled)
{
    if (!deviceManager_) return;

    if (enabled)
    {
        deviceManager_->setMidiInputDeviceEnabled(deviceIdentifier, true);
        if (!enabledDevices_.contains(deviceIdentifier))
        {
            deviceManager_->addMidiInputDeviceCallback(deviceIdentifier, this);
            enabledDevices_.add(deviceIdentifier);
        }
    }
    else
    {
        deviceManager_->removeMidiInputDeviceCallback(deviceIdentifier, this);
        enabledDevices_.removeString(deviceIdentifier);
    }
}

juce::Array<juce::MidiDeviceInfo> MidiHandler::getAvailableDevices()
{
    return juce::MidiInput::getAvailableDevices();
}

bool MidiHandler::isDeviceEnabled(const juce::String& deviceIdentifier) const
{
    return enabledDevices_.contains(deviceIdentifier);
}

void MidiHandler::handleIncomingMidiMessage(juce::MidiInput* /*source*/,
                                             const juce::MidiMessage& message)
{
    // Called on the MIDI thread — post to message thread for binding processing
    int channel = message.getChannel();

    if (message.isNoteOn())
    {
        int note = message.getNoteNumber();
        int velocity = message.getVelocity();
        juce::MessageManager::callAsync([this, channel, note, velocity]()
        {
            bindingManager_.processMidiNoteOn(channel, note, velocity);
        });
    }
    else if (message.isNoteOff())
    {
        int note = message.getNoteNumber();
        juce::MessageManager::callAsync([this, channel, note]()
        {
            bindingManager_.processMidiNoteOff(channel, note);
        });
    }
    else if (message.isController())
    {
        int cc = message.getControllerNumber();
        int value = message.getControllerValue();
        juce::MessageManager::callAsync([this, channel, cc, value]()
        {
            bindingManager_.processMidiCC(channel, cc, value);
        });
    }
}
