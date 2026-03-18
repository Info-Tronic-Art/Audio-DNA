#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include "signal/SignalRegistry.h"
#include "routing/MacroBank.h"
#include <functional>

// UniversalParamControl: the standard parameter widget used everywhere in the Inspector.
//
// Collapsed (default):
//   [Label]  [0.50]  [-] [+]  [════════╪════════]
//
// Expanded (click to expand):
//   Source picker button → dropdown: Manual / Signals / Macros
//   [Invert] checkbox
//   [Range] min/max sliders
//   [Dial Range] min/max sliders
//   If source drives value: mini meter visualization
class UniversalParamControl : public juce::Component
{
public:
    UniversalParamControl();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    // Set the parameter name and current value
    void setParamName(const juce::String& name);
    void setParamValue(float value);
    float getParamValue() const { return currentValue_; }

    // Source configuration
    enum class SourceMode : uint8_t { Manual, Signal, Macro };
    void setSourceMode(SourceMode mode) { sourceMode_ = mode; repaint(); }
    SourceMode getSourceMode() const { return sourceMode_; }
    void setSourceName(const juce::String& name) { sourceName_ = name; repaint(); }

    // Signal-driven visualization (0-1 value from source)
    void setSourceValue(float v) { sourceValue_ = v; repaint(); }

    // Expand/collapse
    void setExpanded(bool expanded);
    bool isExpanded() const { return expanded_; }

    // Height calculation
    int getPreferredHeight() const;
    static constexpr int kCollapsedHeight = 24;
    static constexpr int kExpandedHeight = 100;

    // Callbacks
    std::function<void(float)> onValueChanged;
    std::function<void()> onExpandToggled;
    std::function<void(SourceMode, const juce::String&)> onSourceChanged;
    std::function<void(float, float)> onRangeChanged;
    std::function<void(bool)> onInvertChanged;

    // Set the signal registry for source picker dropdown
    void setSignalRegistry(SignalRegistry* reg) { signalRegistry_ = reg; }

private:
    void showSourcePicker();
    void updateValueDisplay();

    juce::String paramName_ = "Parameter";
    float currentValue_ = 0.5f;
    bool expanded_ = false;

    // Source state
    SourceMode sourceMode_ = SourceMode::Manual;
    juce::String sourceName_;
    float sourceValue_ = 0.0f;

    // Range state
    float outputMin_ = 0.0f;
    float outputMax_ = 1.0f;
    float dialMin_ = 0.0f;
    float dialMax_ = 1.0f;
    bool inverted_ = false;

    // Collapsed row widgets
    juce::Slider valueSlider_;
    juce::TextButton decrementBtn_{"-"};
    juce::TextButton incrementBtn_{"+"};

    // Expanded widgets
    juce::TextButton sourceBtn_{"Manual"};
    juce::ToggleButton invertToggle_{"Invert"};
    juce::Slider rangeMinSlider_;
    juce::Slider rangeMaxSlider_;
    juce::Label rangeLabel_;

    SignalRegistry* signalRegistry_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UniversalParamControl)
};
