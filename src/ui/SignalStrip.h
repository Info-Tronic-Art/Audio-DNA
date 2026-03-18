#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "signal/Signal.h"
#include "signal/SignalRegistry.h"
#include "analysis/FeatureSnapshot.h"
#include "ui/LookAndFeel.h"

// SignalStrip: a single vertical meter strip for one Signal.
// Three display sizes: minimized (~20px tall), normal (~80px tall), expanded.
// Click selects the signal for the inspector (callback).
class SignalStrip : public juce::Component
{
public:
    enum class DisplaySize { Minimized, Normal, Expanded };

    SignalStrip(Signal& signal, const SignalRegistry& registry);

    void paint(juce::Graphics& g) override;
    void resized() override {}
    void mouseDown(const juce::MouseEvent& event) override;

    // Update the displayed value (called by SignalBar timer)
    void updateValue(float newValue);

    // Display size
    void setDisplaySize(DisplaySize size);
    DisplaySize getDisplaySize() const { return displaySize_; }

    Signal& getSignal() { return signal_; }
    const Signal& getSignal() const { return signal_; }

    // Selection callback
    std::function<void(Signal&)> onSelected;

    // Preferred width based on display size
    int getPreferredWidth() const;
    int getPreferredHeight() const;

private:
    void paintMinimized(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintNormal(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintExpanded(juce::Graphics& g, juce::Rectangle<float> bounds);

    juce::String getFormattedValue() const;
    juce::Colour getSignalColour() const;

    Signal& signal_;
    const SignalRegistry& registry_;
    DisplaySize displaySize_ = DisplaySize::Normal;

    float displayValue_ = 0.0f;
    float smoothedValue_ = 0.0f;
    float peakValue_ = 0.0f;
    int peakHoldTimer_ = 0;
    float flashAlpha_ = 0.0f;

    static constexpr int kPeakHoldFrames = 30;  // ~1 second at 30fps
    static constexpr float kPeakDecay = 0.97f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalStrip)
};
