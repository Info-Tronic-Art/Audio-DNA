#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <array>
#include <atomic>

struct Deck;

// MidiOutputHandler: Sends MIDI note messages to hardware controllers
// (Launchpad, APC40, etc.) to reflect clip state via pad colors.
//
// 5 clip states mapped to MIDI velocities:
//   Empty (no clip)      → Note Off (velocity 0)
//   Loaded but stopped   → dim amber (velocity 5)
//   Playing              → green (velocity 60)
//   Triggered/pending    → flashing green (velocity 52)
//   Active + effects     → bright yellow (velocity 62)
//
// Velocity values target Launchpad X/Mini MK3. Other controllers
// may interpret velocities differently.
//
// Called from MainComponent's timerCallback (~50ms) to poll deck state.
class MidiOutputHandler
{
public:
    MidiOutputHandler();
    ~MidiOutputHandler();

    // Open a MIDI output device by identifier.
    bool openDevice(const juce::String& deviceIdentifier);

    // Close the current output device.
    void closeDevice();

    // Get list of available MIDI output devices.
    static juce::Array<juce::MidiDeviceInfo> getAvailableDevices();

    // Get the currently opened device name.
    juce::String getDeviceName() const;
    bool isOpen() const { return outputDevice_ != nullptr; }

    // Update pad states from current deck. Call every ~50ms.
    // Sends MIDI only for cells whose state has changed.
    void updateFromDeck(const Deck* deck);

    // Send a raw MIDI message (for custom controller protocols).
    void sendMessage(const juce::MidiMessage& msg);

    // Clear all pads (send note-off to all tracked cells).
    void clearAllPads();

    // Pad state enum
    enum class PadState : uint8_t
    {
        Empty = 0,
        Loaded = 1,
        Playing = 2,
        Triggered = 3,
        ActiveWithFx = 4
    };

    // Velocity mapping for each state (Launchpad X/Mini MK3 defaults)
    static constexpr int kVelocityEmpty = 0;
    static constexpr int kVelocityLoaded = 5;      // dim amber
    static constexpr int kVelocityPlaying = 60;     // green
    static constexpr int kVelocityTriggered = 52;   // flashing green
    static constexpr int kVelocityActiveWithFx = 62; // bright yellow

    // Max grid size tracked for state change detection
    static constexpr int kMaxLayers = 10;
    static constexpr int kMaxColumns = 20;

    MidiOutputHandler(const MidiOutputHandler&) = delete;
    MidiOutputHandler& operator=(const MidiOutputHandler&) = delete;

private:
    int velocityForState(PadState state) const;
    int noteForCell(int layer, int column) const;

    std::unique_ptr<juce::MidiOutput> outputDevice_;

    // Cached pad states for change detection
    std::array<std::array<PadState, kMaxColumns>, kMaxLayers> padStates_{};
};
