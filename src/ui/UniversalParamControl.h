#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include "signal/SignalRegistry.h"
#include "routing/MacroBank.h"
#include <functional>

// UniversalParamControl: the standard parameter widget used everywhere in the Inspector.
//
// Collapsed (default):
//   [▶] [Label]  [0.50]  [-] [+]  [════════╪════════]
//    ^-- signal connect triangle: grey=manual, cyan=connected
//        click opens source picker popup
//
// Expanded (click label/value to expand):
//   Source picker button → dropdown: Manual / Audio / BPM Sync / Oscillator / Envelope / Macro
//   [Invert] checkbox
//   [Range] min/max sliders
//   If source drives value: mini meter visualization
// Slider that resets to default on right-click
class ResettableSlider : public juce::Slider
{
public:
    using juce::Slider::Slider;

    void setDefaultValue(double val) { defaultVal_ = val; hasDefault_ = true; }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            if (hasDefault_)
                setValue(defaultVal_, juce::sendNotificationSync);
            return;
        }
        juce::Slider::mouseDown(e);
    }

private:
    double defaultVal_ = 0.0;
    bool hasDefault_ = false;
};

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
    void setDefaultValue(float value) { defaultValue_ = value; valueSlider_.setDefaultValue(static_cast<double>(value)); }
    float getDefaultValue() const { return defaultValue_; }

    // Source configuration — expanded to cover all Resolume-style source types
    enum class SourceMode : uint8_t {
        Manual,       // Direct slider control
        Signal,       // Audio feature signal (Volume, Bass, Beat Phase, etc.)
        BPMSync,      // Per-parameter BPM-synced oscillation (waveform + beat division)
        Oscillator,   // Free-running LFO from SignalRegistry
        Envelope,     // Custom envelope curve
        ClipPosition, // Driven by clip playhead position (0-1)
        Timeline,     // Per-parameter keyframed automation
        Macro         // Linked to a Macro knob
    };
    void setSourceMode(SourceMode mode) { sourceMode_ = mode; repaint(); }
    SourceMode getSourceMode() const { return sourceMode_; }
    void setSourceName(const juce::String& name) { sourceName_ = name; repaint(); }
    const juce::String& getSourceName() const { return sourceName_; }

    // Signal-driven visualization (0-1 value from source)
    void setSourceValue(float v) { sourceValue_ = v; repaint(); }

    // Expand/collapse
    void setExpanded(bool expanded);
    bool isExpanded() const { return expanded_; }

    // Height calculation
    int getPreferredHeight() const;
    static constexpr int kCollapsedHeight = 24;
    static constexpr int kExpandedHeight = 100;
    static constexpr int kTriangleSize = 14;  // Signal connect triangle clickable area

    // Callbacks
    std::function<void(float)> onValueChanged;
    std::function<void()> onExpandToggled;
    std::function<void(SourceMode, const juce::String&)> onSourceChanged;
    std::function<void(float, float)> onRangeChanged;
    std::function<void(bool)> onInvertChanged;

    // Set the signal registry for source picker dropdown
    void setSignalRegistry(SignalRegistry* reg) { signalRegistry_ = reg; }

    // Check if this parameter has any source connected (not Manual)
    bool isConnected() const { return sourceMode_ != SourceMode::Manual; }

private:
    void showSourcePicker();
    void showSourcePickerAtTriangle();
    void buildSourcePickerMenu(juce::PopupMenu& menu);
    void handleSourcePickerResult(int result);
    void updateValueDisplay();
    void drawSignalTriangle(juce::Graphics& g, juce::Rectangle<float> area, bool connected);

    juce::String paramName_ = "Parameter";
    float currentValue_ = 0.5f;
    float defaultValue_ = 0.5f;
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
    ResettableSlider valueSlider_;
    juce::TextButton decrementBtn_{"-"};
    juce::TextButton incrementBtn_{"+"};

    // Expanded widgets
    juce::TextButton sourceBtn_{"Manual"};
    juce::ToggleButton invertToggle_{"Invert"};
    ResettableSlider rangeMinSlider_;
    ResettableSlider rangeMaxSlider_;
    juce::Label rangeLabel_;

    SignalRegistry* signalRegistry_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UniversalParamControl)
};
