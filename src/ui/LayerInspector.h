#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Layer.h"
#include "routing/MacroBank.h"
#include "effects/EffectLibrary.h"
#include "signal/SignalRegistry.h"
#include "ui/MacroPanel.h"
#include "ui/EffectStackView.h"
#include "ui/LookAndFeel.h"

// LayerInspector: shows properties for the selected layer.
// Sections: Macros, Blend Mode, Keying, Layer Effects,
// Autopilot Defaults, Transition Speed, 3D Controls.
// Dynamic sections based on layer type.
class LayerInspector : public juce::Component
{
public:
    LayerInspector();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setLayer(Layer* layer);
    Layer* getLayer() const { return layer_; }

    void setEffectLibrary(EffectLibrary* lib);
    void setSignalRegistry(SignalRegistry* reg);
    void setMacroBank(MacroBank* bank);

    void refresh();
    int getPreferredHeight() const;

private:
    Layer* layer_ = nullptr;

    MacroPanel macroPanel_;

    // Blend mode (Transparent type)
    juce::ComboBox blendModeSelector_;

    // Keying controls (Transparent type)
    juce::ComboBox keyingModeSelector_;
    juce::Slider keyThresholdSlider_;
    juce::Slider keySoftnessSlider_;

    // FX Only controls
    juce::Slider dryWetSlider_;

    // 3D controls
    juce::Slider rotXSlider_, rotYSlider_, rotZSlider_;
    juce::Slider rotSpeedSlider_;
    juce::Slider scale3DSlider_;

    // Layer effects
    EffectStackView effectStackView_;

    // Autopilot defaults
    juce::ComboBox defaultApActionSelector_;
    juce::ComboBox defaultApDurationSelector_;

    // Transition speed
    juce::Slider transitionSpeedSlider_;

    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title);
    void syncFromLayer();
    void populateBlendModes();
    void populateKeyingModes();

    static constexpr int kSectionHeaderHeight = 18;
    static constexpr int kSectionGap = 4;
    static constexpr int kRowHeight = 22;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayerInspector)
};
