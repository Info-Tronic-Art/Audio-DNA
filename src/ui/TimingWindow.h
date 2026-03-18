#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"

// TimingWindow: center-bottom panel with 3 tabs — BPM, Routing, Oscillators.
// Sits between the preview monitor and the inspector.
// Content is placeholder for now; will be fleshed out as features develop.
class TimingWindow : public juce::Component
{
public:
    TimingWindow();

    void paint(juce::Graphics& g) override;
    void resized() override;

    enum class Tab : int { BPM = 0, Routing = 1, Oscillators = 2 };
    void setActiveTab(Tab tab);
    Tab getActiveTab() const { return activeTab_; }

private:
    Tab activeTab_ = Tab::BPM;

    juce::TextButton bpmTabBtn_{"BPM"};
    juce::TextButton routingTabBtn_{"Routing"};
    juce::TextButton oscTabBtn_{"Oscillators"};

    static constexpr int kTabBarHeight = 26;

    void updateTabButtonColors();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimingWindow)
};
