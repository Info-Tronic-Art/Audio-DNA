#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "keyboard/KeySlot.h"

// KeyboardPanel: visual 4×10 keyboard grid for the VJ clip launcher.
// Each key button shows its character, thumbnail (if media assigned),
// and glows when active. Keys can be clicked to toggle or receive
// drag-and-drop images.
class KeyboardPanel : public juce::Component
{
public:
    KeyboardPanel(KeyboardLayout& layout);

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Called by MainComponent when a physical key is pressed/released
    void keyActivated(KeySlot& key);
    void keyDeactivated(KeySlot& key);

    // Callback when a key is clicked (for opening the key editor)
    std::function<void(KeySlot& key)> onKeyClicked;

    // Force repaint of all key buttons
    void refresh();

private:
    KeyboardLayout& layout_;

    // One button per key
    struct KeyButton : public juce::Component
    {
        KeyButton(KeySlot& slot, KeyboardPanel& parent);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        void updateThumbnail();

        KeySlot& slot_;
        KeyboardPanel& parent_;
        juce::Image thumbnail_;
        juce::String lastMediaPath_;
    };

    std::array<std::unique_ptr<KeyButton>, KeyboardLayout::kNumKeys> buttons_;
};
