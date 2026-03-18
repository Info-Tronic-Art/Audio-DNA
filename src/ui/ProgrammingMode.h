#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/SignalBar.h"
#include "ui/LookAndFeel.h"

// ProgrammingMode: an expanded signal view that fills most of the screen.
// Toggled via View menu. When active, the SignalBar switches to Expanded
// display size and takes over the main content area.
// Each signal shows its meter + all connected parameters + inline controls.
class ProgrammingMode : public juce::Component
{
public:
    ProgrammingMode(SignalBar& signalBar);

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Toggle programming mode on/off
    void setActive(bool active);
    bool isActive() const { return active_; }

private:
    SignalBar& signalBar_;
    bool active_ = false;

    // Header
    juce::Label headerLabel_{"", "Programming Mode"};
    juce::TextButton closeButton_{"Exit"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgrammingMode)
};
