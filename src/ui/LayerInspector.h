#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Layer.h"
#include "render/FeedbackProcessor.h"
#include "routing/MacroBank.h"
#include "effects/EffectLibrary.h"
#include "signal/SignalRegistry.h"
#include "ui/MacroPanel.h"
#include "ui/EffectStackView.h"
#include "ui/UniversalParamControl.h"
#include "ui/LookAndFeel.h"

// LayerInspector: Resolume-style layer properties panel.
//
// Sections (matching Resolume Layer tab):
//   [Name]                    "Layer 1"  search + gear icons
//   [Dashboard]               8 link knobs
//   [Autopilot]               Direction, Duration, Clip Loops, Loop
//   [Layer]                   ▶ Master % slider
//   [Video]                   Blend Mode, ▶ Opacity %, Width, Height, Auto Size
//   [Transition]              Blend Mode, Duration
//   [Keying]                  (Transparent only) Mode, Threshold, Softness
//   [DryWet]                  (FX Only only) Mix slider
//   [3D Controls]             (ThreeD only) Rotation X/Y/Z, Speed, Scale
//   [Transform]               Position X/Y, Scale %, Rotation °, Anchor
//   [Layer Effects]           Effect stack
//   [Autopilot Defaults]      Default Action, Default Duration
class LayerInspector : public juce::Component,
                       public juce::DragAndDropTarget
{
public:
    LayerInspector();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    void setLayer(Layer* layer);
    Layer* getLayer() const { return layer_; }

    void setEffectLibrary(EffectLibrary* lib);
    void setSignalRegistry(SignalRegistry* reg);
    void setMacroBank(MacroBank* bank);

    void refresh();
    int getPreferredHeight() const;

    // Callback when layer name is changed by the user
    std::function<void()> onLayerNameChanged;

private:
    Layer* layer_ = nullptr;
    SignalRegistry* signalRegistry_ = nullptr;

    // Name bar — editable label
    juce::Label nameLabel_;

    MacroPanel macroPanel_;

    // --- Autopilot ---
    juce::TextButton apRewindBtn_;
    juce::TextButton apOffBtn_{"OFF"};
    juce::TextButton apForwardBtn_;
    juce::TextButton apRandomBtn_;
    juce::ComboBox apTriggerModeSelector_;   // "End of Video" or "On Beat"
    juce::ComboBox apBeatCountSelector_;     // 1/2/4/8/16/32 beats (visible in On Beat mode)
    juce::Label apLoopsLabel_{"", "Loops:"};
    ResettableSlider apLoopsSlider_;             // Number of loops before advancing

    // --- Layer (Master) ---
    UniversalParamControl masterControl_;

    // --- Video ---
    juce::ComboBox blendModeSelector_;
    UniversalParamControl opacityControl_;
    ResettableSlider widthSlider_;
    ResettableSlider heightSlider_;
    juce::ComboBox autoSizeSelector_;

    // --- Transition ---
    juce::ComboBox transitionBlendSelector_;
    ResettableSlider transitionDurationSlider_;

    // --- Keying (Transparent type) ---
    juce::ComboBox keyingModeSelector_;
    ResettableSlider keyThresholdSlider_;
    ResettableSlider keySoftnessSlider_;

    // --- FX Only ---
    ResettableSlider dryWetSlider_;

    // --- 3D Controls ---
    ResettableSlider rotXSlider_, rotYSlider_, rotZSlider_;
    ResettableSlider rotSpeedSlider_;
    ResettableSlider scale3DSlider_;

    // --- Transform ---
    UniversalParamControl posXControl_;
    UniversalParamControl posYControl_;
    UniversalParamControl scaleControl_;
    UniversalParamControl rotationControl_;
    UniversalParamControl anchorControl_;

    // --- Feedback ---
    juce::ToggleButton feedbackEnableBtn_{"Enable"};
    juce::ComboBox feedbackPresetSelector_;
    ResettableSlider feedbackAmountSlider_;
    ResettableSlider feedbackScaleXSlider_;
    ResettableSlider feedbackScaleYSlider_;
    ResettableSlider feedbackRotationSlider_;
    ResettableSlider feedbackOffsetXSlider_;
    ResettableSlider feedbackOffsetYSlider_;
    ResettableSlider feedbackLumaKeySlider_;

    // --- Layer Effects ---
    EffectStackView effectStackView_;

    // DragAndDropTarget for FX drops
    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;
    bool fxDropHighlight_ = false;

    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title, bool hasPButton = false);
    void syncFromLayer();
    void updateAutopilotButtons();
    void populateBlendModes();
    void populateKeyingModes();

    static constexpr int kSectionHeaderHeight = 18;
    static constexpr int kSectionGap = 4;
    static constexpr int kRowHeight = 22;
    static constexpr int kNameBarHeight = 26;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayerInspector)
};
