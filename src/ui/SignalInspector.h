#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "signal/Signal.h"
#include "signal/OscillatorSignal.h"
#include "signal/EnvelopeSignal.h"
#include "signal/SignalRegistry.h"
#include "ui/LookAndFeel.h"

// SignalInspector: shows full settings for the selected signal.
// - Audio signals: threshold, gain, falloff
// - Oscillators: wave shape, beat duration, amplitude, phase offset
// - Envelopes: curve editor with draggable points, phase, curve type
// - Routes list: where this signal connects
class SignalInspector : public juce::Component
{
public:
    SignalInspector();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setSignal(Signal* signal);
    Signal* getSignal() const { return signal_; }

    void refresh();
    int getPreferredHeight() const;

private:
    Signal* signal_ = nullptr;

    // --- Audio signal controls ---
    juce::Slider thresholdSlider_;
    juce::Slider gainSlider_;
    juce::Slider falloffSlider_;

    // --- Oscillator controls ---
    juce::ComboBox waveShapeSelector_;
    juce::ComboBox beatDurationSelector_;
    juce::Slider amplitudeSlider_;
    juce::Slider phaseOffsetSlider_;

    // --- Envelope controls ---
    juce::ComboBox curveTypeSelector_;
    juce::ComboBox envBeatDurationSelector_;
    juce::Slider envAmplitudeSlider_;
    juce::Slider envPhaseSlider_;
    juce::ToggleButton loopingToggle_{"Looping"};
    juce::ToggleButton oneShotToggle_{"One Shot"};

    // Envelope curve editor (simple rectangle for now)
    juce::Rectangle<int> curveEditorBounds_;

    void hideAllControls();
    void showAudioControls();
    void showOscillatorControls();
    void showEnvelopeControls();

    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title);
    void paintCurveEditor(juce::Graphics& g);

    static constexpr int kSectionHeaderHeight = 18;
    static constexpr int kRowHeight = 22;
    static constexpr int kSectionGap = 4;
    static constexpr int kCurveEditorHeight = 100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalInspector)
};
