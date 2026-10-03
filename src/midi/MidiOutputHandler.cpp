#include "midi/MidiOutputHandler.h"
#include "model/Composition.h"
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

MidiOutputHandler::PadState MidiOutputHandler::padStateFor(const Composition& comp, int shownDeck, int row, int col)
{
    if (shownDeck < 0 || shownDeck >= static_cast<int>(comp.decks.size()))
        return PadState::Empty;
    const Deck& deck = comp.decks[static_cast<size_t>(shownDeck)];
    const Clip* clip = deck.getClip(row, col);
    if (clip == nullptr)
        return PadState::Empty;
    const Layer* layer = comp.getLayer(row);
    if (layer == nullptr || layer->runtime().activeRef() != ClipRef{ deck.id, col })
        return PadState::Loaded;
    if (!clip->playing)
        return PadState::Triggered;
    // Check if clip has active effects
    for (const auto& fx : clip->effects)
        if (!fx.bypassed)
            return PadState::ActiveWithFx;
    return PadState::Playing;
}

void MidiOutputHandler::updateFromDeck(const Composition& comp, int shownDeckIndex)
{
    if (!outputDevice_ || shownDeckIndex < 0 || shownDeckIndex >= static_cast<int>(comp.decks.size()))
        return;
    const Deck& deck = comp.decks[static_cast<size_t>(shownDeckIndex)];

    int numLayers = std::min(deck.getNumRows(), kMaxLayers);
    int numColumns = std::min(deck.numColumns, kMaxColumns);

    for (int li = 0; li < numLayers; ++li)
    {
        for (int ci = 0; ci < numColumns; ++ci)
        {
            const PadState newState = padStateFor(comp, shownDeckIndex, li, ci);

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
