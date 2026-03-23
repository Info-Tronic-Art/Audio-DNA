#include "midi/MidiOutputHandler.h"
#include "model/Deck.h"
#include "model/Layer.h"
#include "model/Clip.h"
#include <iostream>

MidiOutputHandler::MidiOutputHandler()
{
    // Initialize all pad states to Empty
    for (auto& row : padStates_)
        row.fill(PadState::Empty);
}

MidiOutputHandler::~MidiOutputHandler()
{
    closeDevice();
}

bool MidiOutputHandler::openDevice(const juce::String& deviceIdentifier)
{
    closeDevice();

    auto devices = juce::MidiOutput::getAvailableDevices();
    for (const auto& dev : devices)
    {
        if (dev.identifier == deviceIdentifier)
        {
            outputDevice_ = juce::MidiOutput::openDevice(dev.identifier);
            if (outputDevice_)
            {
                std::cerr << "[MIDI Out] Opened: " << dev.name << std::endl;
                return true;
            }
        }
    }

    std::cerr << "[MIDI Out] Failed to open device: " << deviceIdentifier << std::endl;
    return false;
}

void MidiOutputHandler::closeDevice()
{
    if (outputDevice_)
    {
        clearAllPads();
        outputDevice_.reset();
        std::cerr << "[MIDI Out] Device closed" << std::endl;
    }
}

juce::Array<juce::MidiDeviceInfo> MidiOutputHandler::getAvailableDevices()
{
    return juce::MidiOutput::getAvailableDevices();
}

juce::String MidiOutputHandler::getDeviceName() const
{
    if (outputDevice_)
        return outputDevice_->getName();
    return {};
}

void MidiOutputHandler::updateFromDeck(const Deck* deck)
{
    if (!outputDevice_ || !deck)
        return;

    int numLayers = std::min(static_cast<int>(deck->layers.size()), kMaxLayers);
    int numColumns = std::min(deck->numColumns, kMaxColumns);

    for (int li = 0; li < numLayers; ++li)
    {
        const auto& layer = deck->layers[static_cast<size_t>(li)];

        for (int ci = 0; ci < numColumns; ++ci)
        {
            PadState newState = PadState::Empty;

            const Clip* clip = nullptr;
            if (ci >= 0 && ci < static_cast<int>(layer.clips.size()) && layer.clips[static_cast<size_t>(ci)].has_value())
                clip = &(*layer.clips[static_cast<size_t>(ci)]);
            if (clip)
            {
                if (layer.activeClipColumn == ci)
                {
                    if (clip->playing)
                    {
                        // Check if clip has active effects
                        bool hasFx = false;
                        for (const auto& fx : clip->effects)
                        {
                            if (!fx.bypassed)
                            {
                                hasFx = true;
                                break;
                            }
                        }
                        newState = hasFx ? PadState::ActiveWithFx : PadState::Playing;
                    }
                    else
                    {
                        newState = PadState::Triggered;
                    }
                }
                else
                {
                    newState = PadState::Loaded;
                }
            }

            // Only send MIDI if state changed
            if (newState != padStates_[static_cast<size_t>(li)][static_cast<size_t>(ci)])
            {
                padStates_[static_cast<size_t>(li)][static_cast<size_t>(ci)] = newState;

                int note = noteForCell(li, ci);
                int velocity = velocityForState(newState);

                if (velocity > 0)
                    outputDevice_->sendMessageNow(juce::MidiMessage::noteOn(1, note, static_cast<uint8_t>(velocity)));
                else
                    outputDevice_->sendMessageNow(juce::MidiMessage::noteOff(1, note));
            }
        }
    }
}

void MidiOutputHandler::sendMessage(const juce::MidiMessage& msg)
{
    if (outputDevice_)
        outputDevice_->sendMessageNow(msg);
}

void MidiOutputHandler::clearAllPads()
{
    if (!outputDevice_)
        return;

    for (int li = 0; li < kMaxLayers; ++li)
    {
        for (int ci = 0; ci < kMaxColumns; ++ci)
        {
            if (padStates_[static_cast<size_t>(li)][static_cast<size_t>(ci)] != PadState::Empty)
            {
                int note = noteForCell(li, ci);
                outputDevice_->sendMessageNow(juce::MidiMessage::noteOff(1, note));
                padStates_[static_cast<size_t>(li)][static_cast<size_t>(ci)] = PadState::Empty;
            }
        }
    }
}

int MidiOutputHandler::velocityForState(PadState state) const
{
    switch (state)
    {
        case PadState::Empty:        return kVelocityEmpty;
        case PadState::Loaded:       return kVelocityLoaded;
        case PadState::Playing:      return kVelocityPlaying;
        case PadState::Triggered:    return kVelocityTriggered;
        case PadState::ActiveWithFx: return kVelocityActiveWithFx;
    }
    return 0;
}

int MidiOutputHandler::noteForCell(int layer, int column) const
{
    // Launchpad X layout: rows from bottom (note 11-18, 21-28, ..., 81-88)
    // Map layer 0 = bottom row (notes 11-18), layer 1 = row above (21-28), etc.
    // Column 0 = leftmost pad (x1), column 7 = rightmost (x8)
    // For grids > 8 columns, wrap or use higher note banks

    int row = layer;  // 0 = bottom
    int col = column;

    // Launchpad X: note = (row + 1) * 10 + (col + 1)
    // This gives 11-18 for row 0, 21-28 for row 1, etc.
    return (row + 1) * 10 + (col + 1);
}
