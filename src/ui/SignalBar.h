#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/SignalStrip.h"
#include "signal/SignalRegistry.h"
#include "features/FeatureBus.h"
#include "analysis/FeatureSnapshot.h"
#include "ui/LookAndFeel.h"
#include <vector>
#include <memory>

// SignalBar: horizontal strip of SignalStrip meters, full window width.
// Positioned below the TopBar. Displays all visible signals.
// Three size modes: minimized, normal, expanded (programming mode).
// Updates at ~30fps from the FeatureBus.
class SignalBar : public juce::Component, private juce::Timer
{
public:
    SignalBar(SignalRegistry& registry, FeatureBus& featureBus);

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Display size for all strips
    void setDisplaySize(SignalStrip::DisplaySize size);
    SignalStrip::DisplaySize getDisplaySize() const { return displaySize_; }

    // Rebuild strips from registry (call after adding/removing signals)
    void rebuildStrips();

    // Get preferred height based on display size
    int getPreferredHeight() const;

    // Signal selection callback (forwarded from strips)
    std::function<void(Signal&)> onSignalSelected;

    // Called when size changes so parent can re-layout
    std::function<void()> onSizeChanged;

    // Cycle to next/previous display size
    void shrink();
    void grow();

private:
    void timerCallback() override;

    SignalRegistry& registry_;
    FeatureBus& featureBus_;
    SignalStrip::DisplaySize displaySize_ = SignalStrip::DisplaySize::Normal;

    std::vector<std::unique_ptr<SignalStrip>> strips_;

    // Latest snapshot for display
    FeatureSnapshot displaySnap_{};

    // [+] button to add signals
    juce::TextButton addButton_{"+"};
    void showAddSignalMenu();

    // Size control buttons (top-right)
    juce::TextButton shrinkButton_;
    juce::TextButton growButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalBar)
};
