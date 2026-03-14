#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "mapping/MappingEngine.h"
#include "effects/EffectChain.h"
#include "effects/EffectLibrary.h"
#include "ui/MappingEditor.h"
#include <memory>
#include <vector>

// EffectsRackPanel: right-side panel showing the effect chain with
// per-parameter sliders and mapping controls.
//
// For each effect in the chain:
//   - Enable/disable toggle
//   - Parameter sliders
//   - "Add Mapping" button per parameter
//   - Existing mapping indicators
//
// Owns the MappingEditor popup for configuring individual mappings.
class EffectsRackPanel : public juce::Component,
                         public juce::Timer,
                         public MappingEditor::Listener
{
public:
    EffectsRackPanel(MappingEngine& mappingEngine,
                     EffectChain& effectChain,
                     EffectLibrary& effectLibrary);
    ~EffectsRackPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // MappingEditor::Listener
    void mappingEditorChanged(MappingEditor* editor) override;
    void mappingEditorDeleteRequested(MappingEditor* editor) override;

private:
    // Per-parameter UI row
    struct ParamRow
    {
        int effectIndex = 0;
        int paramIndex = 0;
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> nameLabel;
        std::unique_ptr<juce::TextButton> mapButton;
    };

    // Per-effect UI section
    struct EffectSection
    {
        int effectIndex = 0;
        std::unique_ptr<juce::Label> nameLabel;
        std::unique_ptr<juce::ToggleButton> enableToggle;
        std::vector<std::unique_ptr<ParamRow>> paramRows;
    };

    void rebuildUI();
    void openMappingEditor(int effectIndex, int paramIndex);
    void closeMappingEditor();
    int findMappingForParam(int effectIndex, int paramIndex) const;

    MappingEngine& mappingEngine_;
    EffectChain& effectChain_;
    EffectLibrary& effectLibrary_;

    std::vector<std::unique_ptr<EffectSection>> sections_;

    // Scrollable viewport
    juce::Viewport viewport_;
    juce::Component contentComponent_;

    // Mapping editor popup
    std::unique_ptr<MappingEditor> activeMappingEditor_;
    int editingEffectIndex_ = -1;
    int editingParamIndex_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsRackPanel)
};
