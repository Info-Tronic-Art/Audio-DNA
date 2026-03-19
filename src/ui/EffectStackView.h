#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Clip.h"
#include "effects/EffectLibrary.h"
#include "ui/UniversalParamControl.h"
#include "routing/MacroBank.h"
#include "ui/LookAndFeel.h"
#include <vector>
#include <memory>
#include <functional>

// EffectStackView: vertical list of effects, each collapsible.
//
// Collapsed (single line):
//   [B] [icon] Ripple                  0.72
//
// Expanded (click to expand):
//   [B] [icon] Ripple
//     Frequency  0.50  [-][+] [═══╪══════]
//       ← Volume ▓▓▓▓▓░░░░░░ (source viz)
//     Amplitude  0.72  [-][+] [══════╪═══]
//       ← Beat Position ▓▓▓▓▓▓▓░░░ (viz)
//     Speed      0.30  [-][+] [══╪═══════]
//       ← Manual
class EffectStackView : public juce::Component
{
public:
    EffectStackView();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the effect list to display (clip or layer effects)
    void setEffects(std::vector<Clip::EffectSlot>* effects);

    // Set the effect library for parameter name lookup
    void setEffectLibrary(EffectLibrary* lib) { effectLibrary_ = lib; }

    // Set signal registry for source pickers
    void setSignalRegistry(SignalRegistry* reg) { signalRegistry_ = reg; }

    // Set macro bank for macro-driven parameters
    void setMacroBank(MacroBank* bank) { macroBank_ = bank; }

    // Refresh display from current effect data
    void refresh();

    // Get preferred height for layout
    int getPreferredHeight() const;

    // Callbacks
    std::function<void(int effectIndex, int paramIndex, float value)> onParamChanged;
    std::function<void(int effectIndex, bool bypassed)> onBypassChanged;

private:
    // One row per effect in the stack
    struct EffectRow
    {
        int effectIndex = 0;
        bool expanded = false;

        juce::TextButton bypassBtn{"B"};
        juce::Rectangle<int> headerBounds;

        std::vector<std::unique_ptr<UniversalParamControl>> paramControls;
    };

    std::vector<std::unique_ptr<EffectRow>> rows_;
    std::vector<Clip::EffectSlot>* effects_ = nullptr;
    EffectLibrary* effectLibrary_ = nullptr;
    SignalRegistry* signalRegistry_ = nullptr;
    MacroBank* macroBank_ = nullptr;

    static constexpr int kHeaderHeight = 26;
    static constexpr int kParamIndent = 12;

    void rebuildRows();
    void toggleExpand(int rowIndex);

    void mouseDown(const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectStackView)
};
