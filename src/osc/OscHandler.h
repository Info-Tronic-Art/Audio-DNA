#pragma once

#include <juce_osc/juce_osc.h>
#include <juce_core/juce_core.h>
#include <functional>
#include <atomic>

// Forward declarations
struct Composition;

// OscHandler: Receives OSC messages and routes them to application actions.
//
// Address patterns:
//   /audiodna/clip/{layer}/{column}     — trigger clip (float 0/1)
//   /audiodna/layer/{n}/opacity         — set layer opacity (float 0-1)
//   /audiodna/layer/{n}/bypass          — toggle layer bypass (float 0/1)
//   /audiodna/layer/{n}/solo            — toggle layer solo (float 0/1)
//   /audiodna/layer/{n}/mute            — toggle layer mute (float 0/1)
//   /audiodna/deck/{n}                  — switch to deck (float 0/1)
//   /audiodna/master                    — master opacity (float 0-1)
//   /audiodna/bpm                       — set manual BPM (float)
//   /audiodna/snapshot                  — take snapshot (any value)
//   /audiodna/effect/{name}/{param}     — set effect parameter (float 0-1)
//   /audiodna/macro/{n}                 — set macro value (float 0-1)
//
// Uses JUCE's OSCReceiver with MessageLoopCallback for thread safety.
class OscHandler : private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
    OscHandler();
    ~OscHandler() override;

    // Start listening on a UDP port. Returns true on success.
    bool startListening(int port);

    // Stop listening.
    void stopListening();

    bool isListening() const { return listening_.load(std::memory_order_relaxed); }
    int getPort() const { return port_; }

    // Callbacks wired by MainComponent
    std::function<void(int layer, int column)> onTriggerClip;
    std::function<void(int layer, float opacity)> onSetLayerOpacity;
    std::function<void(int layer, bool bypass)> onSetLayerBypass;
    std::function<void(int layer, bool solo)> onSetLayerSolo;
    std::function<void(int layer, bool mute)> onSetLayerMute;
    std::function<void(int deckIndex)> onSwitchDeck;
    std::function<void(float level)> onSetMaster;
    std::function<void(float bpm)> onSetBpm;
    std::function<void()> onSnapshot;
    std::function<void(const juce::String& effectName, const juce::String& paramName, float value)> onSetEffectParam;
    std::function<void(int macroIndex, float value)> onSetMacro;

    OscHandler(const OscHandler&) = delete;
    OscHandler& operator=(const OscHandler&) = delete;

private:
    void oscMessageReceived(const juce::OSCMessage& message) override;

    juce::OSCReceiver receiver_;
    std::atomic<bool> listening_{false};
    int port_ = 0;
};
