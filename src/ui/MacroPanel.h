#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "routing/MacroBank.h"
#include "signal/SignalRegistry.h"
#include "ui/Knob.h"
#include "ui/LookAndFeel.h"
#include <array>
#include <memory>
#include <functional>

// MacroPanel: 8 dashboard link knobs with source picker buttons.
// Displayed in Clip/Layer/Composition inspector tabs as "Dashboard" section.
// Each knob is renameable and shows linked parameters.
// Layout: single row of 8 smaller knobs (matching Resolume's Dashboard).
class MacroPanel : public juce::Component
{
public:
    MacroPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the macro bank to display
    void setMacroBank(MacroBank* bank);

    // Set signal registry for source picking
    void setSignalRegistry(SignalRegistry* reg) { signalRegistry_ = reg; }

    // Refresh display from current macro state
    void refresh();

    // Preferred height — single row of 8 large knobs + header label
    static constexpr int kPreferredHeight = 110;

    // Callbacks
    std::function<void(int macroIndex, float value)> onMacroValueChanged;
    std::function<void(int macroIndex, uint32_t signalId)> onMacroSourceChanged;

private:
    struct MacroSlot
    {
        std::unique_ptr<Knob> knob;
        std::unique_ptr<juce::TextButton> sourceBtn;
    };

    std::array<MacroSlot, MacroBank::kNumMacros> slots_;
    MacroBank* macroBank_ = nullptr;
    SignalRegistry* signalRegistry_ = nullptr;

    void showSourcePicker(int macroIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroPanel)
};
