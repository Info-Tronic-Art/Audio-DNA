#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "mapping/MappingEngine.h"
#include "effects/EffectChain.h"
#include "effects/EffectLibrary.h"
#include "ui/MappingEditor.h"
#include "ui/Knob.h"
#include <memory>
#include <vector>

// EffectsRackPanel: right-side panel showing the effect chain with
// per-parameter knobs and mapping controls.
//
// For each effect in the chain:
//   - Enable/disable toggle
//   - Rotary knobs for each parameter (with mapping indicator ring)
//   - Click knob to open mapping editor
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

    // Rebuild the UI from the current effect chain state (e.g. after preset load)
    void refreshFromChain();

    // Check if an effect is locked (protected from randomization)
    bool isEffectLocked(int effectIndex) const;

    // MappingEditor::Listener
    void mappingEditorChanged(MappingEditor* editor) override;
    void mappingEditorDeleteRequested(MappingEditor* editor) override;
    void mappingEditorCloseRequested(MappingEditor* editor) override;

private:
    // Per-parameter UI element
    struct ParamKnob
    {
        int effectIndex = 0;
        int paramIndex = 0;
        std::unique_ptr<Knob> knob;
        std::unique_ptr<juce::TextButton> mapButton;
    };

    // Per-effect UI section
    struct EffectSection
    {
        int effectIndex = 0;
        bool locked = false;  // Locked effects are skipped by randomize
        std::unique_ptr<juce::Label> nameLabel;
        std::unique_ptr<juce::ToggleButton> enableToggle;
        std::unique_ptr<juce::TextButton> lockButton;
        std::unique_ptr<juce::TextButton> randButton;
        std::vector<std::unique_ptr<ParamKnob>> paramKnobs;
    };

    // Category header label
    struct CategoryHeader
    {
        juce::String category;
        std::unique_ptr<juce::Label> label;
    };

    std::vector<std::unique_ptr<CategoryHeader>> categoryHeaders_;
    juce::TextButton randomizeButton_{"Random"};

    void rebuildUI();
    void openMappingEditor(int effectIndex, int paramIndex);
    void closeMappingEditor();
    int findMappingForParam(int effectIndex, int paramIndex) const;

    MappingEngine& mappingEngine_;
    EffectChain& effectChain_;

    int lastKnownEffectCount_ = 0;  // Detect when effects become available

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
