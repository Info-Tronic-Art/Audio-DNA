#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "binding/BindingManager.h"
#include "model/Composition.h"
#include "ui/LookAndFeel.h"

// BindingOverlay: semi-transparent overlay shown when keyboard binding mode is active.
// Highlights bindable UI elements. User clicks an element, then presses a key to bind it.
// Enter/exit via Shortcuts > Edit Keyboard (Shift+Cmd+K).
class BindingOverlay : public juce::Component,
                       public juce::KeyListener
{
public:
    BindingOverlay(BindingManager& bindingManager, Composition& composition);
    ~BindingOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    // KeyListener
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

    // Show/hide binding mode
    void enterBindingMode();
    void exitBindingMode();
    bool isBindingModeActive() const { return active_; }

    // Called when binding mode ends
    std::function<void()> onBindingModeExit;

    // Provide the deck geometry so we can draw bindable targets
    struct BindableTarget
    {
        juce::Rectangle<int> bounds;  // In overlay coordinates
        juce::String label;           // Display label
        Binding::Action action = Binding::Action::TriggerClip;
        int layerIndex = 0;
        int column = 0;
        int deckIndex = 0;
        int effectIndex = 0;
        int macroIndex = 0;
    };
    void setBindableTargets(const std::vector<BindableTarget>& targets);

private:
    BindingManager& bindingManager_;
    Composition& composition_;
    bool active_ = false;

    // Waiting for key press after clicking a target
    bool waitingForKey_ = false;
    int selectedTargetIndex_ = -1;

    std::vector<BindableTarget> targets_;

    // Find which target was clicked
    int hitTestTarget(juce::Point<int> pos) const;

    // Get display string for a key binding
    static juce::String getKeyDescription(const Binding& b);

    // Find existing binding for a target
    const Binding* findExistingBinding(const BindableTarget& target) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BindingOverlay)
};
