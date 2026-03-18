#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "routing/MacroBank.h"
#include "effects/EffectLibrary.h"
#include "signal/SignalRegistry.h"
#include "ui/MacroPanel.h"
#include "ui/EffectStackView.h"
#include "ui/LookAndFeel.h"

// CompositionInspector: shows global composition properties.
// Sections: Global Macros, Global Effects, Master Opacity,
// Transition Speed, Output Settings.
class CompositionInspector : public juce::Component
{
public:
    CompositionInspector();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setComposition(Composition* comp);
    Composition* getComposition() const { return composition_; }

    void setEffectLibrary(EffectLibrary* lib);
    void setSignalRegistry(SignalRegistry* reg);
    void setMacroBank(MacroBank* bank);

    void refresh();
    int getPreferredHeight() const;

private:
    Composition* composition_ = nullptr;

    MacroPanel macroPanel_;
    EffectStackView effectStackView_;

    juce::Slider masterOpacitySlider_;
    juce::Slider transitionSpeedSlider_;

    // Output settings
    juce::ComboBox resolutionSelector_;

    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title);
    void syncFromComposition();

    static constexpr int kSectionHeaderHeight = 18;
    static constexpr int kSectionGap = 4;
    static constexpr int kRowHeight = 22;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompositionInspector)
};
