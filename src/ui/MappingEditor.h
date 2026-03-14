#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "mapping/MappingTypes.h"

// MappingEditor: a JUCE Component for configuring a single Mapping.
//
// Shows:
//   - Source feature dropdown (ComboBox)
//   - Curve type dropdown (ComboBox)
//   - Input min/max sliders
//   - Output min/max sliders
//   - Smoothing slider
//   - Enable/disable toggle
//   - Delete button
//
// Uses a Listener interface so the parent can react to changes.
class MappingEditor : public juce::Component
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void mappingEditorChanged(MappingEditor* editor) = 0;
        virtual void mappingEditorDeleteRequested(MappingEditor* editor) = 0;
        virtual void mappingEditorCloseRequested(MappingEditor* editor) = 0;
    };

    MappingEditor();
    ~MappingEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the mapping to edit (copies values in)
    void setMapping(const Mapping& mapping, int mappingIndex);

    // Get the current mapping values
    Mapping getMapping() const;
    int getMappingIndex() const { return mappingIndex_; }

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    void populateSourceCombo();
    void populateCurveCombo();
    void notifyChanged();

    int mappingIndex_ = -1;

    juce::Label titleLabel_;
    juce::Label sourceLabel_;
    juce::Label curveLabel_;
    juce::Label inputMinLabel_;
    juce::Label inputMaxLabel_;
    juce::Label outputMinLabel_;
    juce::Label outputMaxLabel_;
    juce::Label smoothingLabel_;

    juce::ComboBox sourceCombo_;
    juce::ComboBox curveCombo_;

    juce::Slider inputMinSlider_;
    juce::Slider inputMaxSlider_;
    juce::Slider outputMinSlider_;
    juce::Slider outputMaxSlider_;
    juce::Slider smoothingSlider_;

    juce::ToggleButton enableToggle_{"Enabled"};
    juce::TextButton deleteButton_{"Delete Mapping"};
    juce::TextButton closeButton_{"X"};

    juce::ListenerList<Listener> listeners_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingEditor)
};
