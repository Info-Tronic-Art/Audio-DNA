#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Knob: a rotary parameter control for the effects rack.
//
// Displays:
//   - A rotary slider (knob) using the LookAndFeel's drawRotarySlider
//   - A name label below the knob
//   - A value label inside/below the knob
//   - An optional mapping indicator ring (colored arc showing the mapped source)
//
// The mapping indicator is a second arc drawn behind the knob arc
// in a distinct color (magenta for mapped, dim when unmapped).
class Knob : public juce::Component
{
public:
    Knob(const juce::String& name = {});
    ~Knob() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Access the underlying slider
    juce::Slider& getSlider() { return slider_; }

    // Set the parameter name displayed below the knob
    void setParamName(const juce::String& name);

    // Set mapping indicator state
    // sourceName: empty string means unmapped; non-empty shows the colored ring
    void setMappingIndicator(const juce::String& sourceName);
    bool isMapped() const { return mapped_; }

    // Preferred size for layout
    static constexpr int kPreferredWidth = 64;
    static constexpr int kPreferredHeight = 80;

private:
    juce::Slider slider_;
    juce::Label nameLabel_;
    juce::Label valueLabel_;

    bool mapped_ = false;
    juce::String mappedSourceName_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob)
};
