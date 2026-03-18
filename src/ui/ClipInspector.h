#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Clip.h"
#include "routing/MacroBank.h"
#include "effects/EffectLibrary.h"
#include "signal/SignalRegistry.h"
#include "ui/MacroPanel.h"
#include "ui/EffectStackView.h"
#include "ui/LookAndFeel.h"

// ClipInspector: shows properties for the selected clip.
// Sections: Macros, Transport, Autopilot, Beat Snap, Effect Stack, Cuepoints.
class ClipInspector : public juce::Component
{
public:
    ClipInspector();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the clip to inspect (nullptr clears)
    void setClip(Clip* clip);
    Clip* getClip() const { return clip_; }

    // Dependencies
    void setEffectLibrary(EffectLibrary* lib);
    void setSignalRegistry(SignalRegistry* reg);
    void setMacroBank(MacroBank* bank);

    // Refresh from current clip state
    void refresh();

    // Get preferred content height for scrollable container
    int getPreferredHeight() const;

private:
    Clip* clip_ = nullptr;

    // --- Macros section ---
    MacroPanel macroPanel_;

    // --- Transport section ---
    juce::ComboBox transportModeSelector_;
    juce::ComboBox loopModeSelector_;
    juce::Slider speedSlider_;
    juce::TextButton reverseBtn_{"Reverse"};
    juce::TextButton halfSpeedBtn_{juce::String(juce::CharPointer_UTF8("\xc3\xb7")) + "2"};
    juce::TextButton doubleSpeedBtn_{juce::String(juce::CharPointer_UTF8("\xc3\x97")) + "2"};

    // --- Autopilot section ---
    juce::ComboBox autopilotActionSelector_;
    juce::ComboBox autopilotDurationSelector_;

    // --- Beat Snap ---
    juce::ToggleButton beatSnapToggle_{"Beat Snap"};

    // --- Effect Stack ---
    EffectStackView effectStackView_;

    // --- Cuepoints ---
    static constexpr int kNumCuepoints = 8;
    std::array<std::unique_ptr<juce::TextButton>, kNumCuepoints> cuepointBtns_;

    // Section helpers
    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title);
    void populateDropdowns();
    void syncFromClip();

    static constexpr int kSectionHeaderHeight = 18;
    static constexpr int kSectionGap = 4;
    static constexpr int kRowHeight = 22;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipInspector)
};
