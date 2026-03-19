#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "binding/BindingManager.h"
#include "model/Composition.h"
#include "ui/LookAndFeel.h"
#include "ui/BindingOverlay.h" // Reuse BindableTarget

// MidiLearnOverlay: semi-transparent overlay shown when MIDI learn mode is active.
// Same workflow as BindingOverlay, but listens for MIDI input instead of keyboard.
// Notes -> triggers (clips, columns), CCs -> continuous controls (sliders, knobs).
// Enter/exit via Shortcuts > Edit MIDI (Shift+Cmd+M).
class MidiLearnOverlay : public juce::Component,
                         public juce::KeyListener,
                         public juce::MidiInputCallback
{
public:
    MidiLearnOverlay(BindingManager& bindingManager, Composition& composition);
    ~MidiLearnOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    // KeyListener (for Escape to exit)
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

    // MidiInputCallback
    void handleIncomingMidiMessage(juce::MidiInput* source,
                                   const juce::MidiMessage& message) override;

    // Show/hide MIDI learn mode
    void enterLearnMode(juce::AudioDeviceManager* deviceManager);
    void exitLearnMode();
    bool isLearnModeActive() const { return active_; }

    // Called when learn mode ends
    std::function<void()> onLearnModeExit;

    // Provide bindable targets (same format as BindingOverlay)
    void setBindableTargets(const std::vector<BindingOverlay::BindableTarget>& targets);

private:
    BindingManager& bindingManager_;
    Composition& composition_;
    juce::AudioDeviceManager* deviceManager_ = nullptr;
    bool active_ = false;

    bool waitingForMidi_ = false;
    int selectedTargetIndex_ = -1;

    std::vector<BindingOverlay::BindableTarget> targets_;

    // Last received MIDI info for display
    juce::String lastMidiMessage_;

    int hitTestTarget(juce::Point<int> pos) const;
    const Binding* findExistingMidiBinding(const BindingOverlay::BindableTarget& target) const;

    // Enable/disable MIDI input listening
    void startListening();
    void stopListening();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnOverlay)
};
